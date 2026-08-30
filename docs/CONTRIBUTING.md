# Contributing

Thank you for helping decompile *Mickey's Speedway USA*. Read the
[clean-room policy](CLEANROOM.md) and the
[architecture decisions](adr/README.md) before changing canonical source.

## Setup

Provide a legally obtained US ROM at `baseroms/mickey.us.z64`, then run:

```sh
gmake setup
gmake -j$(sysctl -n hw.ncpu)
gmake verify
```

Setup installs Python dependencies, initializes required submodules, prepares
the toolchain, checks and splits the ROM, and enables the repository hooks.

## Worktrees and branches

Use a separate worktree for independent matching work:

```sh
tools/new_lane.sh <name>
```

The command creates a `lane/<name>` branch with its own `build/` and `asm/`
directories. Do not edit another contributor's worktree. Commit small,
coherent changes as they are completed. `tools/merge_lane.sh` checks and merges
a completed branch. On macOS, the helper also adds a git-ignored
`.metadata_never_index` marker before extraction so several new lanes do not
trigger simultaneous Spotlight indexing of duplicate build trees.

## Matching a function

1. Confirm the function boundary, translation unit, compiler flags, callers,
   padding, and relocations.
2. Check the current source, target assembly, and the documented reference
   results. A reference can guide naming and structure, but Mickey's bytes are
   authoritative.
3. Compile the unchanged baseline and record its size, first mismatch, and
   relocation differences.
4. Change C source only. Preserve types, signedness, control flow, and call
   order.
5. Compare the compiled function and its relocations with the target.
6. Run `gmake verify` before marking the function as matched.
7. Update the symbol file and any affected generated reports in the same
   commit.

A match requires exact owned bytes, exact relocation records, correct linked
placement, and a byte-identical ROM. Similar assembly, equal size, or semantic
agreement is not a match. Post-compile edits to instruction words are
prohibited by [ADR 0002](adr/0002-no-post-compile-instruction-editing.md).

If a useful C body does not match, keep it under `#ifdef NON_MATCHING` and keep
the `#pragma GLOBAL_ASM` fallback. Do not count it as matched. After a bounded
set of attempts, record the best result and the remaining mismatch instead of
making speculative source changes.

`NON_MATCHING=1` compiles guarded candidates into the separate
`build_non_matching/` tree. As a second defense against a manual full-TU probe
that writes directly into `build/`, every successful verification receipts the
canonical hashes of candidate-bearing objects. The next `gmake verify`
forcibly rebuilds only objects that changed since that receipt; a missing or
malformed receipt safely rebuilds the complete candidate-bearing set once.

Before sweeping flags, run `tools/function_preflight.py <symbol>`. It accepts
either a friendly or generated overlay name and fails closed unless it can
prove one source, one owned range, one padding boundary, and stable relocation
identities. It automatically selects the ordinary or `NON_MATCHING` full-TU
build and reports callers, exports, the candidate ABI context, overlay runtime
records or authenticated resident static relocation tuples, and the current
workbench score and first mismatch without printing instruction text or ROM
bytes. Sparse resident startup-table records are reported separately and may
legitimately be absent. See [`tools.md`](tools.md) for details.

After an exact promotion removes the extracted fallback, preflight admits a
separate `post_promotion` route only when one unconditional C definition, its
tracked symbol or overlay-atlas ownership, and the linked value and size all
agree. It uses the ordinary object and fully relocated ROM comparison, while
requiring exact relocation count/offset/type shape. A missing fallback never
promotes guarded `NON_MATCHING` C.

For allocator investigations, use the fail-closed procedure mapping and
fidelity receipt in
[`allocator-trace-receipts.md`](allocator-trace-receipts.md). Raw traces and
objects remain untracked evidence; only compact findings belong in ledgers.

### Safe plateau finalization

`tools/finalize_plateau.py` preserves one bounded attempt without weakening
its assembly fallback:

```sh
tools/finalize_plateau.py overlay40FadeRecords \
  src/overlays/o040/overlay40FadeRecords.c \
  --score "98/101 words" --frame 0x8 --relocations 10 \
  --first-mismatch +0xC --summary "one allocator web remains"
```

The exact function must already be C under `#ifdef NON_MATCHING`, followed by
one `#else` / `#pragma GLOBAL_ASM(...)` fallback. The fallback filename must
either match the C symbol or use splat's canonical generated overlay-function
form (`func_overlay_NNN_F...s`), which is how friendly overlay names retain
their original assembly identity. Balanced declaration-only `NON_MATCHING`
guards elsewhere in the same translation unit are ignored; validation is tied
to the requested symbol's own top-level guard and fallback. The command refuses
an unguarded, nested, unterminated, or ambiguous target body, any other
mismatched fallback, an untracked source, or
worktree/index dirt outside that source and an optional `--handoff-doc
docs/<file>.md`. It records only the supplied score, frame, relocation count,
first mismatch, and one-line summary in a symbol-keyed metadata comment at
the end of the source file (and, when requested, a bounded Markdown block).
Appending the source metadata preserves every pre-existing byte and physical
source line, including the measured guarded function. Re-running the command
updates only that symbol's EOF block, and multiple symbols may share a source
file. The command refuses legacy inline handoff comments because moving one
would itself require a fresh compile and byte-comparison proof. It never
records instruction rows or claims exactness.

The result remains uncommitted by default. After reviewing the diff, pass
`--commit` to stage and commit only the named source and optional tracked
`--handoff-doc docs/<file>.md`.

### Report-only m2c sweep

`tools/m2c_sweep.py` inventories every `GLOBAL_ASM` fallback and attempts only
bare fallbacks that do not already have a `NON_MATCHING` or `NON_EQUIVALENT`
C candidate. It gives m2c the owning translation unit as context, inserts each
draft into a scratch copy of that complete unit, compiles with the effective
Makefile flags, and compares function bytes and relocations. It runs one
low-priority compiler process at a time.

```sh
nice -n 15 .venv/bin/python tools/m2c_sweep.py --inventory-only --fresh
nice -n 15 .venv/bin/python tools/m2c_sweep.py --fresh
```

Output stays under the ignored `build/m2c_sweep/` directory. The tool does not
edit source or promote a result. A scratch exact result still needs the normal
configured object, linked range, relocation, and full-ROM checks before it can
be counted as matched.

## Overlay work

Overlay virtual addresses are reused. Identify a function by `(overlay,
section, offset)` and confirm its ROM range in `config/overlays.us.json`.

Before adopting an overlay name or body, check the recorded Diddy Kong Racing
and Jet Force Gemini object results:

```sh
gmake overlay-donors
gmake overlay-donors-scan-check   # requires the local reference builds
```

A generated placeholder name is not evidence. State whether a name comes from
byte identity, a call graph, a string correspondence, or structural analysis.

### Overlay build flow

Mickey uses a runtime overlay linker from the same Rare engine lineage as JFG,
but it does not copy a JFG host-side overlay build. There is one build graph and
one final linker invocation:

1. `config/overlays.us.json` records each module's measured ROM ranges and
   ownership. `tools/overlay_atlas.py` checks it against the generated overlay
   block in `mickey.us.yaml`.
2. Splat produces ordinary inputs under `src/overlays/oNNN/`,
   `asm/overlays/oNNN/`, and `assets/overlays/oNNN/`, plus `mickey.us.ld`.
3. The root Makefile's normal C, assembly, and binary-wrapper rules produce
   objects. `mk/overlays.mk` contains measured per-overlay-object compiler
   flags and reviewed ELF normalization; it is an included policy table, not
   another build graph or linker. Pure `objcopy --redefine-sym`-only rules may
   instead be declared in
   `config/normalizations/overlay-symbol-aliases.us.json`; the checked-in
   `mk/overlay_aliases.generated.mk` include is its deterministic projection.
   Rules that also trim sections or filter/rebind relocations remain explicit
   in `mk/overlays.mk` so their ordered command chains stay visible.
4. `build/mickey.us.elf` links all objects once. Splat's script places each
   module's text, data, and original relocation-table blobs in its ROM range.

`src/main/runlink.c` is game code that loads and relocates modules on the
console. JFG is disclosed evidence for parts of that runtime lineage; Mickey's
tables, atlas, generated linker script, and exact ROM comparison define this
repository's host build.

For build debugging, start in the roughly 1,100-line root Makefile: source
discovery, generic recipes, and the sole final link are all there. Consult
`mk/overlays.mk` only when one overlay object needs a measured flag, trim, or
symbol/relocation normalization.

After changing the pure alias manifest, refresh and verify its projection:

```sh
tools/render_overlay_aliases.py --write
tools/render_overlay_aliases.py --check
```

The renderer rejects unknown schema fields, malformed source or destination
symbols, duplicate object targets, duplicate sources or destinations within a
target, and order-dependent alias chains. Do not edit the generated include
by hand or add trim/filter/rebind commands to the manifest. `gmake check-docs`
runs the render check, and `gmake check-tooling` includes the focused renderer
tests.

## Checks

Run the checks that apply to the change. Source changes that affect matching
must pass `gmake verify`.

| Command | Needs a build | Purpose |
|---|:---:|---|
| `gmake verify` | yes | Compare the rebuilt ROM with the target |
| `gmake cleanroom` | no | Reject prohibited tracked content |
| `gmake check-docs` | no | Recompute documented numeric claims |
| `gmake check-scoreboard` | yes | Recompute the README progress section |
| `gmake overlay-atlas` | no | Check the overlay manifest and YAML projection |
| `gmake overlay-donors` | no | Check the recorded reference-result table |
| `gmake overlay-donors-scan-check` | no | Repeat the object scan against local references |
| `tools/postprocess_audit.py --check` | no | Check the recorded post-compile operations |
| `gmake check-reference-builds` | no | Compare local reference objects with their lock |
| `gmake public-release PUBLIC_RELEASE_ARGS="--remote public --branch master"` | yes | Dry-run exact deltas, outgoing-tree scans, and all public release gates; never pushes |

Public CI cannot run checks that require a retail ROM or local reference ROMs.
Run those checks locally before submitting a matching change.

## Progress reporting

Run `gmake scoreboard` after a matching change. The generated README table
reports resident functions, resident and overlay C bytes, total resolved bytes,
and whole-game code status. Do not edit its numbers by hand.

Function counts apply only to the resident segment because complete overlay
function boundaries are not yet available. Overlay and whole-game progress use
byte counts.

## Provenance and documentation

Every borrowed body or group of borrowed names needs a point-of-use
`PROVENANCE` note. Every name claim uses one of the evidence levels in
[The module map](modules.md). Derived counts must come from the current tree or
a checked generator.

Update documentation when a source-layout, build, or policy change makes it
wrong. Record a policy change in a new ADR; do not rewrite an accepted decision
without a replacement record.

## Commit rules

- Keep hooks enabled. Never use `--no-verify`.
- Commit one function and its related symbol and progress updates together.
- Do not commit generated ROM data, local logs, caches, or comparison output.
- Do not change global flags or shared data layout to improve one function
  without checking every affected object.
- Include the commands run and the exact match result in the commit message or
  review description.
