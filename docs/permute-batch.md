# tools/permute_batch.py: batch-running the permuter over the NON_MATCHING queue

`docs/adr/0007-matching-tools.md` requires decomp-permuter to run only as a
**bounded batch job**, never inside an agent's own reasoning loop. This tool
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

## Scratch fidelity (2026-08-28)

A permuter score-0 is only a match if the scratch object it was scored
against is bit-identical to the object the project build produces for that
TU. Three fidelity faults were found in `tools/permute.sh` on 2026-08-27
(`docs/matching-triage.md`) and are now applied by this runner too:

| Fault | Effect before | Fix in `permute_batch.py` |
|---|---|---|
| importer default `-mips1`, static flag groups | searched the wrong ISA; per-file `-Wab,-r4300_mul` / `-Wo,-loopunroll,0` dropped | `build_recipe_for()` touches the source and reads the real cc line from `gmake -n <obj>` (falls back to the static group with a loud warning) |
| no post-compile `objcopy --redefine-sym` | track.c results never transferred | the TU's objcopy chain is appended to the scratch `compile.sh`, retargeted at `$OUTPUT`; digest-guarded `.py` passes are skipped and listed in `build/permuter/<fn>/recipe.txt` |
| scorer normalises stack offsets | false 0 on a spill at the wrong slot | `--stack-diffs` is always passed |

The 2026-08-25 farm result in "Cost and match rate" below (0 hits in 38
searches) predates all three fixes and is not evidence about the queue.

Other runner behaviour added at the same time: `--order ranking` (default)
runs the closest functions first by `config/nonmatching-ranking.us.json`
`differing_words`; `--resume` skips functions already in
`build/permuter/summary.json` and carries their rows forward;
`--extend-minutes N` re-seeds from the best candidate and runs once more when
a capped search was still descending (best result in the last third of the
window); `--load-threshold L` (default 9) waits for headroom before every
permuter launch and promotion build; `--commit` (with `--apply`) commits each
verified promotion as `Match <fn> (permuter)`, staging only that C file. The
permuter is niced. `tools/permute_sweep.sh` wraps all of it: resync a lane to
the integration branch, extract, warm build, verify, sweep, extract again so the
scoreboard counts the promotions.

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

**Cross-lane sample**, three functions from one earlier batch.s overlay 2
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
table rather than layering a second one on top of it, per `docs/CONTRIBUTING.md`'s
"derived numbers are recomputed, never remembered.""

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
