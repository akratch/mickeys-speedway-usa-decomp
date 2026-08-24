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

Measured against the one function queued at the time this tool was built,
`overlay1GetEntry` (o001, 0x30 bytes / 12 instructions):

| | |
|---|---|
| base score | 330 |
| best score reached, 12-minute cap, `-j 6` | 190 (a 42% score reduction; still nonzero) |
| zero-diff found | no |
| promoted (verified match) | no -- stayed queued, `#ifdef NON_MATCHING` unchanged |
| wall-clock for the whole run (import + permute, capped; no promote/build phase since no zero-diff was found) | 721s (~12 min -- ran to the wall-clock cap, `--stop-on-zero` never triggered) |

One data point is not a match-rate estimate. `overlay1GetEntry` was
originally reached only through `normalize_elf_instructions.py` (three
register-field edits, disqualified by `docs/adr/0002`), so a genuine
compiler-only match existing at all was not guaranteed going in -- a
useful floor case, not a representative one. The ~310-function queue this
tool is built for will produce the real rate; rerun this section's numbers
(`tools/permute_batch.py --limit 20 --jobs 4 --minutes 15 --apply` is a
reasonable first real batch) once a meaningful slice has gone through and
replace this table rather than layering a second one on top of it, per
`CLAUDE.md`'s "derived numbers are recomputed, never remembered."

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
