# tools/permute_batch.py: batch-running the permuter over the NON_MATCHING queue

`docs/adr/0007-matching-tools.md` requires decomp-permuter to run only as a
**bounded batch job**, separate from the interactive matching loop. This tool
is that job runner: it enumerates every queued `NON_MATCHING` function,
imports and permutes each one under a wall-clock cap, and reports what came
back -- score, whether a zero-diff candidate turned up, and (opt-in) whether
that candidate survived promotion into a real, verified match. See
`docs/tools.md` for the underlying decomp-permuter/objdiff setup this
builds on, and `tools/permute.sh` for the single-function version this
generalizes (this script does not shell out to `permute.sh`; it reimplements
the same import/permute steps inline so it can handle the overlay naming
quirk below and the `#ifdef NON_MATCHING` queue shape, which `permute.sh`
was not written for).

## Usage

```sh
# see what's queued, without running anything
tools/permute_batch.py --list

# one function, report-only (default: no source is touched)
tools/permute_batch.py --function overlay1GetEntry --minutes 12

# one function, and splice + verify a zero-score candidate if found
tools/permute_batch.py --function overlay1GetEntry --minutes 12 --apply

# a whole overlay, 3 functions at a time, capped at 15 min each
tools/permute_batch.py --overlay 9 --jobs 3 --minutes 15 --apply

# the first 20 queued functions project-wide
tools/permute_batch.py --limit 20 --jobs 4 --minutes 15 --apply

# extra permuter.py flags go after --
tools/permute_batch.py --function overlay1GetEntry -- --best-only
```

`--jobs` is concurrent *functions*; each function's own `permuter.py`
still gets its own `-j <threads>` (`--permuter-threads`, default: split
`ncpu - 2` evenly across `--jobs` so the total thread count stays around
`ncpu - 2` regardless of how many functions run at once). `--build-jobs`
is the separate `gmake -jN` used only when `--apply` promotes a zero-score
result (default: `ncpu`, since a promotion rebuild is not concurrent with
another function's permuter run within the same `--jobs` slot... though it
*can* overlap with another slot's permuter run, so don't set `--build-jobs`
to the full core count if also running `--jobs > 1` with `--apply` -- the
two searches don't currently coordinate CPU budget with each other's
promotion rebuilds).

Every run (even a `--list`-free normal run) writes `build/permuter/summary.json`
and `build/permuter/summary.txt` incrementally, one function at a time, so a
killed batch still leaves a readable partial result.

## Queue discovery

Two sources, unioned:

1. `config/overlays.us.json`'s per-module `text_ownership` rows (written by
   `tools/overlay_atlas.py`), each carrying a mechanically-derived
   `"nonmatching"` flag -- true iff the source file for that byte range still
   has an `#ifdef NON_MATCHING` guard around the named function
   (`overlay_atlas.is_nonmatching_source`). This is authoritative for
   overlay functions once the atlas has been regenerated after a conversion.
2. A direct scan of `src/**/*.c` for `#ifdef NON_MATCHING` blocks, matched
   by function-definition shape. This catches anything not yet reflected in
   the atlas -- in practice, at the time this was written, two `src/main/`
   functions (`MatrixMultiplyVec4`, `ProcessRelocationEntry`) that predate
   the atlas's `nonmatching` flag and aren't overlay code at all, so the
   atlas will never carry a row for them.

`--overlay N` filters on the atlas's `overlay` number (source-scan-only
items outside `src/overlays/` have no overlay number and are excluded by
this filter); `--function NAME` and `--limit K` apply after that.

## The overlay naming quirk

splat auto-names every overlay function it disassembles from the ROM
`func_overlay_MMM_FOOOOOOO_ROMADDR` -- every overlay module shares one
synthetic VMA (`tools/overlay_atlas.py`'s `SYNTHETIC_VMA`), so spimdisasm
can't derive a unique name from the address alone. A converted function's
`#ifdef NON_MATCHING` branch already carries the friendly name a human (or
`TEXT_SUBSEGMENTS`) gave it, e.g. `overlay1GetEntry`; its `#else` branch's
`#pragma GLOBAL_ASM(...)` still points at the auto-named `.s` file, whose
`glabel` is the auto name, not the friendly one.

decomp-permuter's `import.py` derives the function name it hunts for in the
C source from the target `.s` file's `glabel` line. Handed the auto-named
`.s` directly, it looks for `func_overlay_...` in the (preprocessed) C and
fails with `"...not found in base.c"` even though the function is right
there under a different name. `permute_batch.py` works around this without
touching `import.py` (out of this lane's ownership) or `asm/` itself
(gitignored, never written to in place): it copies the target `.s` into the
function's own `build/permuter/<fn>/target.s` scratch file and renames the
`glabel`/`endlabel` pair to the friendly name before handing that copy to
`import.py`. That is a label rename -- metadata, not an instruction word --
exactly the class of edit `docs/adr/0002-no-post-compile-instruction-editing.md`
already permits for the project's own `objcopy --redefine-sym` POSTPROCESS
steps.

Some queue items don't hit this at all: `overlay16ApplyGradient` (overlay
16) does, but `func_overlay_028_F00004D8_187CDA8` (overlay 28) is still
carrying its auto name as the *friendly* name too (no human name assigned
yet), so the rename is a no-op there. The script handles both uniformly --
it always renames to whatever the `#ifdef NON_MATCHING` branch's own
function name is, whether or not that differs from the `.s` label.

## The `-DNON_MATCHING` define, not source surgery

A queued function's `.c` file still has both branches
(`#ifdef NON_MATCHING <candidate> #else <pragma> #endif`); the project's own
Makefile already has an escape hatch for selecting the candidate branch,
`gmake NON_MATCHING=1` (`-DNON_MATCHING` in `DEFINES`). `permute_batch.py`'s
generated per-function `permuter_settings.toml` adds that same define to
`compiler_command` and hands `import.py` the **unmodified** `.c` file --
never a hand-edited copy. The real C preprocessor resolves every
`#ifdef NON_MATCHING` block in the translation unit consistently, which
matters for a `.c` file with more than one queued function in it (several
of the lane `cx-nm-*` conversions batch multiple functions per overlay into
one file).

Flag group (`-mips1`/`-mips2`, `-O1`/`-O2`, `-g3`) is chosen per file from
its path, mirroring `tools/permuter_settings.toml`'s own documented groups
and the Makefile's per-directory `OPT_FLAGS`/`MIPSISET` overrides:
`src/main/**` and `src/overlays/**` get `-O2 -mips2 -32` (this covers the
whole current and expected queue, which is overlay conversions);
`src/libultra/**` falls back to the `-O2 -mips1 -32` project default unless
the file is in the Makefile's `LIBULTRA_O2_G3_TUS` list (parsed from the
Makefile directly, not hand-copied), in which case it gets `-O2 -g3 -mips2
-32`. The `LIBULTRA_O1_MIPS2_TUS`-equivalent group and any `IDO_PHASES`
uopt-only overrides are **not** auto-detected (no single Makefile variable
enumerates them at the time of writing) -- a libultra function needing one
of those groups needs a `--settings` override edited by hand until that gap
is closed; see `tools/permuter_settings.toml`'s own header comment for how.

## Promotion (`--apply`)

Every reference project and this project's own `docs/adr/0007` are explicit
that a permuter score of 0 is a *candidate*, not a match, until it is
compiled by the real toolchain, linked at its real address, and
byte-compared. Without `--apply`, that's all this script reports: base
score, best score, whether zero was reached. With `--apply`, a zero score
triggers:

1. **Splice**: the winning `output-0-*/source.c` is the whole pruned
   translation unit import.py built around the candidate, not just the
   function -- `permute_batch.py` extracts just the function definition
   (balanced-brace scan from the signature, the same technique
   `tools/permute.sh` already uses to recover a function body) and replaces
   the *entire* `#ifdef NON_MATCHING ... #else ... #endif` block for that
   function with it, dropping the guard, the stale candidate, and the
   `#pragma GLOBAL_ASM` fallback.
2. **Rebuild**: `gmake -j<build-jobs>`.
3. **Verify**: `gmake verify` (byte-identical ROM rebuild against the
   pinned SHA1).
4. **Linked-range check**: `tools/wb_compare.sh --rom <function>` -- the
   `--rom` mode specifically, since splat stops emitting a function's
   `asm/nonmatchings/**/*.s` the moment its C matches, so the ordinary
   target-object comparison mode has nothing left to diff against.

Any of the three failing reverts the `.c` file to its pre-splice text and
reports the function as `zero-found` but not `promoted` -- still useful
signal (the permuter found *an* exact-diff candidate; something else about
the tree, or the extraction, didn't hold up), never left half-applied.

**What this does not do automatically**: remove a now-dead
`POSTPROCESS = ... objcopy --redefine-sym func_overlay_...=<friendly>`
Makefile rule. Once the real C function (already named the friendly name)
compiles directly, that rule's rename becomes a no-op at best and a
reference to a symbol the object no longer emits at worst -- but whether a
given object's `POSTPROCESS` line is *only* that rename, or bundles it with
something still load-bearing (a second `--redefine-sym` for an unrelated
symbol, a `trim_elf_section.py` step, per §"Makefile context" in the pilot
commit), is a per-object judgment call this script does not make for you.
Check the object's `POSTPROCESS` line by hand after a promotion and drop it
if -- and only if -- the whole line was that one now-redundant rename.

## Cost and match rate (measured)

**Pilot, this lane's own queue item**, `overlay1GetEntry` (o001, 0x30 bytes
/ 12 instructions), 12-minute cap, `-j 6`, `--apply`:

| function | base | best | zero-diff | promoted | wall-clock |
|---|---|---|---|---|---|
| `overlay1GetEntry` | 330 | 190 (-42%) | no | no | 721s (ran to the cap) |

**Cross-lane sample**, three functions from `lane/cx-nm-2`'s overlay 2
conversion (not yet merged into this lane's own queue -- checked out onto a
throwaway branch reset to that lane's `HEAD`, tested, then discarded; no
commit of theirs was kept), 8-minute cap, `-j 4`, report-only (`--apply`
omitted deliberately -- these functions are not this lane's to promote):

| function | base | best | zero-diff | promoted | wall-clock |
|---|---|---|---|---|---|
| `func_overlay_002_F0001A94_185888C` | 570 | 530 (-7%) | no | no | 482s (ran to the cap) |
| `overlay2ClassifyBoundary` | 2540 | 1320 (-48%) | no | no | 482s (ran to the cap) |
| `overlay2ChooseBoundary` | 3295 | 1995 (-39%) | no | no | 482s (ran to the cap) |

This also proved the queue-discovery and naming-quirk handling against a
tree with several other lanes' conversions merged in at once:
`--list` against the `cx-nm-2` checkout correctly surfaced 19 queued
functions across 7 overlay/`main` sources without duplicates, including
`func_overlay_002_F0001A94_185888C` -- a function whose `#ifdef
NON_MATCHING` branch is itself still named with splat's auto name (no
friendly name assigned yet), the other shape the naming-quirk rename needs
to handle correctly (a no-op rename, rather than skipping the rename step).

**Combined: 4 functions run, 0 zero-score (0%), 0 promoted (0%), 0
errored.** Every run reduced the score (7-48%) but none reached zero within
its cap. Four data points is not a reliable match-rate estimate for a
~310-function queue, but it is enough to report honestly: at these caps
(8-12 min, 4-6 permuter threads), the permuter is closing part of the gap
on functions of this size (roughly a hundred bytes to half a kilobyte) but
not resolving them outright, on this small a sample. `overlay1GetEntry`
specifically was originally reached only through
`normalize_elf_instructions.py` (three register-field edits, disqualified
by `docs/adr/0002`), so a genuine compiler-only match existing for it at
all was not guaranteed going in. Re-run this section's numbers once a
meaningful slice of the real queue has gone through
(`tools/permute_batch.py --limit 20 --jobs 4 --minutes 15 --apply` against
this lane's own merged queue is a reasonable next batch) and replace this
table rather than layering a second one on top of it, per
`docs/CONTRIBUTING.md`'s
"derived numbers are recomputed, never remembered."

## Scratch fidelity: when a permuter zero is not a match

A permuter score of 0 is only trustworthy if the scratch object it searches is
bit-identical to the real per-TU object *and* the scorer counts every byte
`gmake verify` counts. Two false-ceiling causes are handled in
`tools/permute.sh`; a third is documented but not yet fixed. Rule of thumb: **a
permuter 0 that has not passed `gmake verify` is a hypothesis, not a match.**

1. **Post-compile `objcopy` not replicated (handled).** Some TUs apply an
   `objcopy --redefine-sym A=B` after `cc` via the Makefile's per-file
   `POSTPROCESS` (e.g. `src/main/track.c`: `trackCamPosTrap=TrapDanglingJump`).
   The importer's scratch runs `cc` only. `permute.sh` recovers that step from
   the same `gmake -n <obj>` dry-run it already uses for flags and appends it to
   the scratch `compile.sh`, retargeted to the scratch output.
   Digest-guarded ELF surgery (`add_elf_relocations.py`, `trim_elf_section.py`)
   is deliberately *not* replicated — it is tied to the matched bytes and would
   abort on a permuted object — so such TUs get a warning instead.
2. **Scorer normalizes stack offsets (handled).** decomp-permuter's scorer
   defaults to `stack_differences=False`, which strips every `sp`-relative
   offset before diffing. A candidate whose only residual is a spill or local at
   the wrong `sp` offset (`sw v1,0x18(sp)` vs `0x1C(sp)`) then scores 0 while
   its bytes still differ and `gmake verify` fails. `permute.sh` always passes
   `--stack-diffs` so the score reflects real bytes.
3. **Injected gfx-macro expansion (documented, not fixed).** The importer keeps
   a TU's `g[DS]P*`/`_SHIFTL`/`gDma*` macros as preserved-macro entries and
   restores them at candidate-compile time. The bodies are correct, but for a
   display-list function the pruned/round-tripped scratch can still reproduce a
   different frame than the full TU (a preserved macro expanding to a sub-macro
   outside the preserve set, or an AST round-trip of the macro-call arguments).
   Until this is closed, treat any gfx-heavy permuter 0 as unverified: byte-diff
   the function between the scratch object and its real `build/` object before
   trusting it.

## Routing a walled function to the right tool

Not every unmatched function is a permuter job. Sending one to the wrong tool
burns effort — a permuter cannot fix a structural gap, and hand analysis cannot
out-search the register allocator. The useful wall classes:

- **Permuter-tractable** — opcode multiset and frame already exact; only which
  register or stack slot differs. The winning spelling exists but is not
  derivable by reasoning, so the batch search finds it. Frame-exact with a small
  register/schedule word-diff is the signature.
- **Permuter-stuck** — same shape, but the search plateaus above 0 within the
  cap. Re-seed, extend the cap, or hand-write a scratch variant before escalating.
- **Import-blocked** — the candidate does not isolate cleanly or does not compile
  under `-DNON_MATCHING` in-TU; fix the base first, since the search never
  actually runs until it compiles standalone. A "finds nothing instantly" result
  is almost always a setup or flag fault, not a hard function.
- **Structural** — genuinely wrong shape (missing or extra code, wrong control
  flow or types). Not an allocation problem; needs real decompilation, guided by
  `mips_to_c` and reference donors with `PROVENANCE` notes.
- **Hard wall** — a list-scheduler slot-fill or interference-forbidden colour
  with no source lever. The permuter is the last resort; if it cannot move it,
  record the wall.

## Recommended default cap

Start at **`--minutes 15 --jobs <ncpu/6 to ncpu/8>`** (leaving
`--permuter-threads` at its default split, so total permuter threads stay
around `ncpu - 2`): most of these functions are the small end of overlay
code (tens to a few hundred bytes; `overlay1GetEntry` itself is 0x30 bytes),
where a permuter search either finds a small perturbation quickly or is
unlikely to converge within any reasonable cap because the mismatch is
structural (register allocation shape, scheduling) rather than a `perm_*`
knob can reach -- in which case 15 minutes and 60 rarely differ in outcome,
per the caution `docs/adr/0007` records from the Snowboard Kids 2 project
about not mistaking permuter search time for the fix. Raise the cap for a
specific function only after confirming from its `permuter.log` that the
score is still trending down near the cap, not plateaued.
