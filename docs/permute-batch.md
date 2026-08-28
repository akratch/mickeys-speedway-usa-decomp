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

# overlay functions only, with the relocation-annotated target that makes
# their score mean something (see "Overlay targets carry no relocations")
tools/permute_batch.py --overlays-only --order ranking --minutes 15 --apply
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
`master`, extract, warm build, verify, sweep, extract again so the
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

## Overlay targets carry no relocations, and what to do about it (2026-08-28)

Until this section was written the runner shipped `--resident-only`, because
for an overlay function the permuter's score was not a measurement of
anything. `overlay18Load` differs from the shipped ROM by **two words** and
scored **700**.

### Why

`docs/reloc-surface.md` §1: an overlay module ships *unrelocated*. What the
ROM stores at a relocation site is the record's **stored addend**, and every
`SYMBOL` `R_MIPS_26` record stores immediate zero. So splat's disassembly of
an overlay function assembles into a target object with **no relocations at
all**: each cross-module call reads back as a `jal` at the module's own
synthetic base (i.e. against whatever symbol happens to sit at module offset
0 -- often the function itself), and each address materialization as a bare
`lui`/`addiu` pair carrying the stored addend as a literal.

The candidate object, compiled from C, carries the honest thing: an
`R_MIPS_26` against `overlayNNCommonReloc`, a `%hi`/`%lo` pair against
`gOverlayNNSomething`.

decomp-permuter's scorer ignores a symbol-name difference only when the
candidate's field looks like a symbol **and the target line also carries a
relocation** (`scorer.py`: `field_matches_any_symbol(nf) and
old_line.has_symbol`). The target line carries none, so every relocation site
in the function was scored as a mnemonic-level `replace` -- one insertion and
one deletion penalty, 200 points a site. The score measured the *number of
relocation sites*, not the distance to the ROM.

### The fix: annotate the target with the module's own relocation table

`tools/reloc_surface.py`'s `permuter_annotation()` gives the scratch target
the relocations the shipped module says are there, and renames the
candidate's placeholders to match, so both sides render **identically** at
every corroborated site. `permute_batch.py` calls it from
`annotate_overlay_scratch()` right after `import.py`, for overlay functions
only.

For each relocation the candidate's own base object carries inside the
function, the site's object offset maps to a module offset; a site the
module's `reloc1`/`reloc2` tables do **not** name is not a relocation site in
the shipped image and is left alone (`docs/reloc-surface.md` §2). A site the
tables do name gets a canonical, ROM-derived identity:

| site | canonical name | why |
|---|---|---|
| `HI16`+`LO16` pair | `__ovval_<link value>` | the value `synthesize()` derives at that site -- the ROM's stored words minus the object's own addend. Two placeholders the surface values identically produce the same linked words, so they are the same symbol as far as the link is concerned |
| `R_MIPS_26`, `SYMBOL` record | `__ovcall_o<overlay>_<offset>` | the stored immediate is always zero and carries no identity, so the record's own `overlayRomTable` entry -- the callee's overlay and offset -- names it |
| `R_MIPS_26`, `JUMP` record | `__ovjump_<module offset>` | an intra-module call; the stored immediate *is* the target's offset |

The target `.s` copy is rewritten to spell those names symbolically
(`jal __ovcall_o0_26934`, `lui $t0,%hi(__ovval_00000080)`,
`addiu $t0,$t0,%lo(__ovval_00000080)`), which makes the assembler emit real
relocations; the candidate's placeholder symbols are renamed to the same
names by `objcopy --redefine-sym` steps appended to the scratch's
`compile.sh`, alongside the ones `replicate_objcopy` already writes there.

Three properties are worth stating because they are what make this a
measurement rather than a fudge:

- **It is not a relaxed scorer.** The canonical name comes from the ROM's
  own record, not from the candidate's symbol table, so a candidate that
  calls the *wrong* placeholder at a site still scores a penalty.
- **A symbol whose sites disagree is left alone entirely.** If one site wants
  `__ovval_...` and another `__ovcall_...`, the symbol has no canonical
  identity; annotating half its sites would make the target disagree with the
  candidate at the other half, silently. Those are reported in
  `build/permuter/<fn>/annotation.txt`.
- **Any failure falls back to the previous behaviour.** If the annotated `.s`
  does not assemble, the unannotated target is restored and reassembled; the
  run continues with the old, pessimistic score. `--no-overlay-annotate`
  forces that path for before/after measurement.

Nothing ROM-derived is written anywhere tracked: the rewritten `.s` lives in
the gitignored permuter scratch, exactly like the label rename above.

### Measured (2026-08-28)

Base score before and after annotation, against
`tools/promotion_trial.py`'s in-range word count for the same function:

| function | ov | trial words | base before | base after | differing rows |
|---|---:|---:|---:|---:|---|
| `overlay18Load` | 18 | 2 | 700 | **400** | 62 -> 2 |
| `overlay7DispatchSelection` | 7 | 2 | 75 | **10** | 14 -> 2 |
| `overlay97InitScale` | 97 | 1 | 10 | 10 | 1 -> 1 |
| `overlay84AdvanceCurrent` | 84 | 2 | 41 | **16** | 7 -> 2 |
| `overlay40FadeRecords` | 40 | 3 | 75 | **25** | 11 -> 3 |

`overlay97InitScale` has no site the module's table names inside it, so
nothing is annotated and nothing changes -- which is the honest answer, and
it already scored its one real word.

The scores that remain are ordinary permuter penalties over the rows that
actually differ (`overlay18Load`'s 400 is one `li`/`move` pair scored as a
replace; `overlay84AdvanceCurrent`'s 16 is a stack-offset difference). What
changed is that they are now *about the function's codegen*.

**Zero means zero.** The check that matters is the other direction: a
candidate whose C is already exact must score 0. `overlay62Initialize`
(o062, matched, 15 placeholder symbols, 30 relocation sites) was temporarily
re-wrapped as a `NON_MATCHING` candidate around its own exact C and
re-split. Unannotated it scored **150** over 30 differing rows; annotated it
scored **0** over 0. The wrapper was reverted afterwards.

### Promotion is still the linked ROM

A score of 0 on the annotated scratch says the candidate's instruction
schedule matches the target. For an overlay that is necessary and not
sufficient, and `--apply` does not treat it as sufficient. The promotion path
splices, rebuilds the object, regenerates the relocation surface
(`gmake overlay-syms`, since a promoted body may reference placeholders whose
values are synthesized from the objects), rebuilds, and accepts only on
`gmake verify` -- byte-identical ROM -- followed by
`tools/wb_compare.sh --rom`.

The splice also regenerates `config/overlays.us.json`: a spliced candidate
flips that TU's mechanically-derived `nonmatching` flag, and
`overlay_atlas.py --check` is a prerequisite of `build/.splat-stamp` and so of
everything, which means a stale atlas kills the promotion build before it
compiles anything. A rejected candidate has both the atlas and
`overlay_undefined_syms.us.txt` restored along with its `.c` file.

A score-0 overlay candidate can still fail `verify`. The module's data is
placed by the runtime, not by this link, so a promotion can move a word the
scratch never modelled: a datum landing at a different module offset, an
alias the surface now resolves elsewhere, a digest-guarded POSTPROCESS pass
the scratch could not replicate. When that happens the C file is reverted and
the function stays a **candidate**, and the reported reason carries
`tools/promotion_trial.py`'s own class for it
(`text-differs` / `text-size-differs` / a named `build-error` cause) read
from `build/promotion-trial.json`, or the command to produce it. "Scored 0
but did not verify" is a trial result to route, not a failure to hide.

### Running the overlay pool

```sh
# overlay functions only, closest first, one at a time, machine-safe
tools/permute_batch.py --overlays-only --order ranking --jobs 1 \
    --permuter-threads 4 --minutes 15 --flat-minutes 5 \
    --load-threshold 12 --limit 25 --apply --commit
```

`--overlays-only` is the complement of the older `--resident-only`, which
existed only because overlay scores were meaningless. It no longer is.

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

**Cross-lane sample**, three functions from an earlier overlay 2 worker's
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
