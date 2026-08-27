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
a completed branch.

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

## Checks

Run the checks that apply to the change. Source changes that affect matching
must pass `gmake verify`.

| Command | Needs a build | Purpose |
|---|:---:|---|
| `gmake verify` | yes | Compare the rebuilt ROM with the target |
| `gmake cleanroom` | no | Reject prohibited tracked content |
| `gmake check-docs` | no | Recompute documented numeric claims |
| `gmake check-scoreboard` | yes | Recompute the README progress section |
| `gmake check-scoreboard-partial` | no | Check source counts and table arithmetic |
| `gmake overlay-atlas` | no | Check the overlay manifest and YAML projection |
| `gmake overlay-donors` | no | Check the recorded reference-result table |
| `gmake overlay-donors-scan-check` | no | Repeat the object scan against local references |
| `gmake postprocess-audit` | no | Check the recorded post-compile operations |
| `gmake check-reference-builds` | no | Compare local reference objects with their lock |

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
