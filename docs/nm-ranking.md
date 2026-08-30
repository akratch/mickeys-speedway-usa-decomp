# tools/nm_ranking.py: ranking the NON_MATCHING queue by closeness

Hundreds of kilobytes of code across many overlays and several `src/main` TUs sit behind
`#ifdef NON_MATCHING` (the compile-only escape hatch documented in the
Makefile and `docs/acceleration-survey.md` sec.13.2): a real C body compiled
under `gmake NON_MATCHING=1`, but not yet byte-identical, so the tree still
ships the `#pragma GLOBAL_ASM` fallback under normal `gmake`. Some of those
candidates are one register swap from matching; others are structurally
wrong. This tool ranks every queued function by how close its candidate
already is, so a fleet of workers can spend its time on near-misses first
instead of triaging the whole queue by hand.

## Method

### Two build paths, and why only one covers the whole queue

The natural approach -- `gmake NON_MATCHING=1 -j`, diff the resulting
`build_non_matching/` tree against a verified `expected/build/` snapshot
with objdiff-cli -- works, but not for the whole queue. 590 of this
project's C objects carry a Makefile `POSTPROCESS` override, overwhelmingly
`trim_elf_section.py`: it shortens a `.text` section header after the fact
so a function that ends on a 4-byte boundary inside a larger overlay module
doesn't drag the next subsegment's start with it. That trim step asserts
the post-compile object is *exactly* the size it was proved to be at the
matched target's size. Under `NON_MATCHING=1`, a queued function in the
same translation unit as a trimmed one compiles to its real (larger, still
wrong) size, and the trim step fails outright:

```
overlay_055/func_overlay_055_F000031C_18A1E34.c.o: cannot grow .text from 0x900 to 0x914
```

A full `gmake NON_MATCHING=1 -k -j` run hits this on most overlay TUs that
still have a queued function (~100 distinct `Error 1`s in this run). It
doesn't stop the build -- `-k` keeps going -- but every such object is
absent or stale under `build_non_matching/`, and this repository's Makefile,
`src/`, and `docs/modules.md` are out of this lane's scope, so the trim
step itself isn't something this tool can fix.

So the ranking's primary source is a **per-function isolated compile**,
which sidesteps the Makefile path entirely: for each queued function,
`tools/permute_batch.py`'s own machinery (`write_settings_toml`,
`prepare_target_asm`, `run_import`) is reused to run the same
asm-processor + IDO invocation the permuter itself runs, producing two
function-comparable objects:

- `target.o` -- the function's own `asm/nonmatchings/**/<f>.s`, assembled
  directly. This *is* the ROM's bytes; splat only stops emitting a
  function's nonmatchings `.s` once C actually implements it, so as long as
  the tree is still in `#ifdef NON_MATCHING` form the `.s` is authoritative
  (the same ordering constraint `tools/wb_compare.sh` documents).
- `base.o` -- normally the `#ifdef NON_MATCHING` C body isolated by
  permuter import and compiled with the Makefile-expanded recipe recovered by
  `tools/permute_batch.py`'s `build_recipe_for`, including per-file optimizer,
  ISA, and backend overrides. Static flag groups are only the explicit
  fail-loud fallback when that recipe cannot be recovered.

If import cannot compile the pruned source, the tool writes an untracked TU
copy that selects only this C body and retains every other function's ASM
fallback. It dry-runs the real `NON_MATCHING=1` Makefile rule and executes
only its raw asm-processor/IDO command, preserving TU-specific flags while
skipping POSTPROCESS. Consolidated candidates whose private typed externs
were lost use their last tracked pre-consolidation declaration wrapper, but
only after the current and historical candidate bodies compare identical.
Function offsets and relocations are then normalized out of that larger
object before comparison.

The failure classes fixed in this pass were: nested conditional directives;
atlas rows that name a consolidated TU rather than a function; a function
name spelled through an object-like macro; auxiliary text/data labels and
jump tables in target assembly; import pruning failures on complex C, static
symbols and local rodata; missing private declaration context after TU
consolidation; exact per-TU flag overrides; and a concurrent-import race that
removed the shared scratch parent directory.

The isolated compile is also, on its own terms, a closer proxy to "how much
work is left" than a whole-TU diff would be: it isolates exactly the
function under test, so a mismatch can't be masked or amplified by drift
elsewhere in a large multi-function overlay module.

### objdiff-cli, as supplementary context

`tools/gen_objdiff_config.py` gained a `--base-dir` option so its unit list
can point at `build_non_matching/` instead of `build/`, target side
unchanged (`expected/build/`, from a `gmake verify`-clean normal build --
the two trees mirror each other's relative paths one-for-one below the top
directory, so no other change was needed). Run against whatever fraction of
`build_non_matching/` actually built, objdiff-cli's `report generate -d`
returns a `fuzzy_match_percent` per function within the units it can parse,
which this tool reads into `objdiff_match_pct` by name.

Coverage here is partial in both directions this run covers:

- Only units that survived the `NON_MATCHING=1 -k` build at all (see
  above) can be in the report.
- objdiff-cli's ELF reader aborts its whole batch on the first
  POSTPROCESS-trimmed object it can't parse (`Section symbol without
  section`, or an unattributed `Symbol data out of bounds` with no file
  name to key a targeted exclusion off of -- `docs/tools.md`'s own
  documented scope limit). The brief for this lane suggested most overlay
  POSTPROCESS steps are metadata-only now and worth trying to include; a
  bisection over the ~590 POSTPROCESS-tagged objects (recursively
  re-including halves and retrying) was attempted to find the actual
  parseable subset rather than excluding all of them by blanket rule, but
  it did not converge in the time budget for this run and was killed. This
  run falls back to the same blanket exclusion `tools/objdiff_report.sh`
  itself uses (every Makefile `POSTPROCESS`-tagged object, ~590 names,
  regenerated from the Makefile, never committed -- see that script's own
  comment on why). A future run with more time, or a narrower per-overlay
  bisection, could recover more of the difference; the mechanism
  (`--base-dir`) is already in place for it.

When a refresh omits `--objdiff-report`, its `objdiff_match_pct` values are
`null`. Since that field is supplementary context (it reflects the function
in its real linked/whole-TU context) and `differing_words`/`category` come from
the isolated compile regardless, every resolved function is ranked either
way. The generated region below states coverage for the current snapshot.

### Word diff and categorization

For each resolved function, both `.text` sections are read as big-endian
32-bit words (raw, in memory only -- see "Clean-room note" below) up to
each side's own symbol size:

- **`size_bytes`**: the target (ROM) function's size.
- **`size_delta`**: `base size - target size`.
- **`differing_words`**: word positions that disagree over the shared
  prefix, plus every word past the shorter side's end.
- **`first_mismatch_offset`**: the byte offset of the first disagreeing
  word (or the shared length, if one side is a truncated/extended prefix
  of the other).
- **`category`**, in this precedence order:
  1. **`size-mismatch`** -- `size_delta != 0`. Nothing else is checked;  a
     size mismatch means the two objects aren't even comparable word-for-word
     past the point of divergence.
  2. *(equal size, no differing words at all)* -- reported as `other` with
     `differing_words: 0`: the isolated candidate is already
     byte-identical to the target in isolation. This can happen even
     though the function is still `NON_MATCHING` in the full build, when
     whatever's blocking the real match is TU-level (surrounding
     function order, a flag difference the Makefile applies only in
     context, etc.) rather than in the function's own body.
  3. **`schedule-only`** -- equal size, and the target's words are a
     permutation of the candidate's (`Counter(base_words) ==
     Counter(target_words)`, order differs). A same-instructions,
     different-order mismatch is exactly the shape IDO's scheduler is
     sensitive to (`docs/tools.md`'s own permuter case study describes a
     `perm_sameline` fix of this shape).
  4. **`register-only`** -- equal size, and every differing word decodes
     to the same opcode/function/immediate once register-select fields
     are masked out (R-type rs/rt/rd, or I-type rs/rt; J-type is left
     unmasked since it has no register fields to begin with). This is a
     coarse approximation -- COP1/COP0 fields share the R-type layout and
     are masked the same way, undistinguished from GPR swaps -- but it's
     conservative in the direction that matters: it only ever
     under-counts `register-only`, never mis-labels a real semantic
     difference as one.
  5. **`reloc-mismatch`** -- equal size, and every differing word's offset
     carries a relocation entry (`objdump -r .text`) on the base or the
     target side.
  6. **`other`** -- equal size, everything else. This is the largest
     bucket in this run and is exactly the "needs a human/model
     look, no cheap mechanical explanation available" category.

Sort order for the table and the JSON: category rank (`register-only` <
`schedule-only` < `other` < `reloc-mismatch` < `size-mismatch`), then
`differing_words` ascending within a category. Register-only and
schedule-only mismatches are typically single flag-lattice or `mips_to_c`
tweaks; size and reloc mismatches usually mean a structural rewrite.

### Clean-room note

No instruction word, mnemonic, or hex byte from a decoded `.text` section
is ever written to `config/nonmatching-ranking.us.json`, printed, or
otherwise leaves the running process -- every decode exists only long
enough to produce a count (`differing_words`) or an offset
(`first_mismatch_offset`) before being discarded. `objdiff_match_pct` is a
float already computed by objdiff-cli, not ROM content.

## Reproducing this run

```sh
gmake extract
gmake -j$(sysctl -n hw.ncpu)
gmake verify
./tools/make_expected.sh
gmake NON_MATCHING=1 -k -j$(sysctl -n hw.ncpu)   # -k: some POSTPROCESS objects will fail, see above

# objdiff-cli report against build_non_matching/, with the standard
# POSTPROCESS exclusion (tools/objdiff_report.sh's own approach, pointed at
# the NON_MATCHING tree instead of build/):
grep -oE '^\$\(BUILD_DIR\)/\$\(SRC_DIR\)/[A-Za-z0-9_/]+\.c\.o: POSTPROCESS' \
    Makefile | sed -E 's#\$\(BUILD_DIR\)/\$\(SRC_DIR\)/#src/#; s/: POSTPROCESS$//' \
    | sort -u > tools/objdiff_exclude.txt
.venv/bin/python tools/gen_objdiff_config.py --base-dir build_non_matching > objdiff.json
tools/objdiff/objdiff-cli report generate -p . -o /tmp/nm_report.json -f json -d

# restore objdiff.json to the normal build/ tree afterward:
.venv/bin/python tools/gen_objdiff_config.py > objdiff.json

# the ranking itself:
.venv/bin/python tools/nm_ranking.py --jobs 12 \
    --out config/nonmatching-ranking.us.json --no-table
# Optional linked/whole-TU context:
.venv/bin/python tools/nm_ranking.py --objdiff-report /tmp/nm_report.json --jobs 12
.venv/bin/python tools/nm_ranking.py --top 20 --markdown --no-table  # fleet-prompt excerpt

# Fast maintenance between full compile passes: remove snapshot rows whose
# exact source/symbol identity is no longer guarded by NON_MATCHING.
.venv/bin/python tools/nm_ranking.py --prune-stale

# Regenerate or verify the marked human-readable snapshot without compiling.
.venv/bin/python tools/nm_ranking.py --write-doc
.venv/bin/python tools/nm_ranking.py --check-doc
```

A complete pass can run long enough for another lane to promote functions
that were queued at startup. Immediately before publishing, the ranking tool
re-scans the canonical source and drops resolved and unresolved rows whose
`#ifdef NON_MATCHING` block no longer exists. The checked-in JSON is still a
historical snapshot after the ranking process exits; consumers that need the
current queue must intersect it with a fresh source scan. `--prune-stale`
performs that intersection in place without invoking the compiler or requiring
decomp-permuter. It keys every row by the exact `(source file, symbol)` pair,
validates that resolved and unresolved identities are unique, writes the JSON
atomically, and refuses malformed unresolved rows rather than guessing which
function they describe. It only removes stale rows and normalizes the retained
counts: newly added `NON_MATCHING` functions remain unranked and are reported,
so a full ranking pass is still required to measure them.

Full canonical ranking writes and canonical `--prune-stale` runs update the
marked region below in the same invocation. `--write-doc` is the source-only
repair command; `--check-doc` validates the complete JSON schema and exact
`(file, symbol)` identity uniqueness before comparing the rendered region.
`gmake check-docs` runs that check, so edited prose, category summaries, ranked
rows, and unresolved tables cannot silently diverge from the persisted JSON.
Text outside the markers remains authored and is preserved byte-for-byte.

<!-- NM_RANKING_GENERATED_BEGIN -->
## Current generated ranking snapshot

> Generated by `tools/nm_ranking.py --write-doc` from
> `config/nonmatching-ranking.us.json`. Do not edit this region by hand;
> `--check-doc` and `gmake check-docs` fail on any drift.

The snapshot contains **392 queued identities**: **392 resolved measurements** and **0 unresolved identities**. Resolved target size totals **400,992 bytes (391.6 KiB)**.

Resolved rows span **73 overlays** and **1 resident TU group** (`main`).

A supplementary objdiff report was not supplied; `objdiff_match_pct` covers **0 / 392** resolved rows.

Persisted selective-TU source evidence covers **392 / 392** resolved rows. Rows without it are retained legacy or bounded-refresh measurements and must be treated as requiring reproof.

Relocation-masked mismatch evidence covers **392 / 392** resolved rows. The raw count preserves literal object differences; the masked count removes only known linker-owned fields to expose the remaining code-generation mismatch. Neither replaces linked byte-identity proof.

### Category distribution

| Category | Count | Share of resolved |
|---|---:|---:|
| `register-only` | 17 | 4.3% |
| `schedule-only` | 1 | 0.3% |
| `other` | 201 | 51.3% |
| `size-mismatch` | 173 | 44.1% |

### Differing-word thresholds

| Threshold | Count |
|---|---:|
| raw `differing_words <= 5` | 10 |
| relocation-masked `differing_words <= 5` | 15 |
| raw `differing_words <= 10` | 32 |
| relocation-masked `differing_words <= 10` | 37 |
| raw `differing_words <= 20` | 79 |
| relocation-masked `differing_words <= 20` | 82 |

### Complete ranked queue

Rank is the persisted `functions` array order. The exact identity is
(`file`, `symbol`), so repeated symbol spellings in different translation
units remain distinct.

| Rank | File | Symbol | Overlay/TU | Category | Target bytes | Raw diff | Masked diff | Raw first | Masked first | Size delta | Objdiff% |
|---:|---|---|---|---|---:|---:|---:|---:|---:|---:|---:|
| 1 | `src/main/menu.c` | `func_8003A2C8` | `main` | `register-only` | 128 | 5 | 5 | 12 | 12 | 0 | — |
| 2 | `src/main/charControl.c` | `func_8001C114` | `main` | `register-only` | 432 | 8 | 8 | 244 | 244 | 0 | — |
| 3 | `src/overlays/o020/overlay20UpdateObjectResource.c` | `overlay20UpdateObjectResource` | `o020` | `register-only` | 392 | 8 | 8 | 176 | 176 | 0 | — |
| 4 | `src/overlays/o019/overlay19ClassifyEdge.c` | `overlay19ClassifyEdge` | `o019` | `register-only` | 480 | 10 | 10 | 312 | 312 | 0 | — |
| 5 | `src/main/models_5B300.c` | `func_8005A948` | `main` | `register-only` | 376 | 11 | 11 | 64 | 64 | 0 | — |
| 6 | `src/overlays/o079/func_overlay_079_F0001290_18CE230.c` | `func_overlay_079_F0001290_18CE230` | `o079` | `register-only` | 492 | 12 | 12 | 200 | 200 | 0 | — |
| 7 | `src/main/anim.c` | `func_80050E9C` | `main` | `register-only` | 360 | 14 | 14 | 100 | 100 | 0 | — |
| 8 | `src/overlays/o040/overlay40FadeRecords.c` | `overlay40FadeRecords` | `o040` | `register-only` | 404 | 16 | 16 | 12 | 12 | 0 | — |
| 9 | `src/main/models.c` | `func_80020D8C` | `main` | `register-only` | 192 | 17 | 17 | 56 | 56 | 0 | — |
| 10 | `src/overlays/o001/overlay_001.c` | `overlay1FindType5ByKey` | `o001` | `register-only` | 156 | 17 | 17 | 28 | 28 | 0 | — |
| 11 | `src/main/saves.c` | `func_8002C69C` | `main` | `register-only` | 112 | 18 | 18 | 16 | 16 | 0 | — |
| 12 | `src/overlays/o020/overlay20RemoveEntry.c` | `overlay20RemoveEntry` | `o020` | `register-only` | 212 | 18 | 18 | 76 | 76 | 0 | — |
| 13 | `src/main/particles.c` | `func_80041CE4` | `main` | `register-only` | 612 | 27 | 27 | 72 | 72 | 0 | — |
| 14 | `src/overlays/o022/overlay22RemoveObject.c` | `func_overlay_022_F0000D30_1878E38` | `o022` | `register-only` | 364 | 43 | 43 | 16 | 16 | 0 | — |
| 15 | `src/overlays/o016/overlay_016.c` | `overlay16ApplyGradient` | `o016` | `register-only` | 580 | 64 | 64 | 60 | 60 | 0 | — |
| 16 | `src/overlays/o057/overlay57UpdateModeState.c` | `overlay57UpdateModeState` | `o057` | `register-only` | 1,416 | 87 | 87 | 220 | 220 | 0 | — |
| 17 | `src/main/level.c` | `levelInit` | `main` | `register-only` | 2,064 | 122 | 122 | 568 | 568 | 0 | — |
| 18 | `src/overlays/o001/overlay_001_head.c` | `overlay1MeasureCurves` | `o001` | `schedule-only` | 316 | 27 | 27 | 12 | 12 | 0 | — |
| 19 | `src/overlays/o097/overlay97InitScale.c` | `overlay97InitScale` | `o097` | `other` | 576 | 1 | 1 | 208 | 208 | 0 | — |
| 20 | `src/overlays/o007/overlay_007_tail.c` | `overlay7DispatchModes` | `o007` | `other` | 524 | 3 | 2 | 68 | 68 | 0 | — |
| 21 | `src/overlays/o099/overlay99RenderSegments.c` | `overlay99RenderSegments` | `o099` | `other` | 568 | 4 | 2 | 8 | 124 | 0 | — |
| 22 | `src/overlays/o007/overlay_007_tail.c` | `overlay7DispatchSelection` | `o007` | `other` | 240 | 5 | 2 | 4 | 4 | 0 | — |
| 23 | `src/main/diCpu.c` | `func_80045BBC` | `main` | `other` | 240 | 8 | 2 | 24 | 188 | 0 | — |
| 24 | `src/overlays/o011/overlay11UpdateMenu.c` | `overlay11UpdateMenu` | `o011` | `other` | 1,204 | 16 | 2 | 20 | 312 | 0 | — |
| 25 | `src/overlays/o026/func_overlay_026_F0000B18_187AF10.c` | `func_overlay_026_F0000B18_187AF10` | `o026` | `other` | 524 | 6 | 3 | 44 | 120 | 0 | — |
| 26 | `src/overlays/o001/overlay_001_middle.c` | `overlay1FindNextAngle` | `o001` | `other` | 200 | 5 | 4 | 56 | 60 | 0 | — |
| 27 | `src/overlays/o001/overlay_001_middle.c` | `overlay1FindPreviousAngle` | `o001` | `other` | 200 | 5 | 4 | 56 | 60 | 0 | — |
| 28 | `src/overlays/o009/overlay_009.c` | `func_overlay_009_F0000CE4_186735C` | `o009` | `other` | 648 | 5 | 4 | 200 | 200 | 0 | — |
| 29 | `src/overlays/o034/overlay34InitStorage.c` | `overlay34InitStorage` | `o034` | `other` | 200 | 5 | 4 | 36 | 36 | 0 | — |
| 30 | `src/overlays/o014/func_overlay_014_F0000000_186F8D8.c` | `func_overlay_014_F0000000_186F8D8` | `o014` | `other` | 316 | 23 | 4 | 36 | 112 | 0 | — |
| 31 | `src/overlays/o022/overlay22InitializeObject.c` | `func_overlay_022_F0000000_1878108` | `o022` | `other` | 688 | 5 | 5 | 204 | 204 | 0 | — |
| 32 | `src/overlays/o014/func_overlay_014_F0001830_1871108.c` | `func_overlay_014_F0001830_1871108` | `o014` | `other` | 804 | 8 | 5 | 152 | 500 | 0 | — |
| 33 | `src/overlays/o041/overlay41AddSlot.c` | `func_overlay_041_F0001650_1888988` | `o041` | `other` | 220 | 6 | 6 | 16 | 16 | 0 | — |
| 34 | `src/main/diprint.c` | `debug_text_width` | `main` | `other` | 264 | 7 | 7 | 56 | 56 | 0 | — |
| 35 | `src/main/track.c` | `func_80012574` | `main` | `other` | 228 | 7 | 7 | 80 | 80 | 0 | — |
| 36 | `src/overlays/o022/overlay22ResolvePlane.c` | `func_overlay_022_F0000A7C_1878B84` | `o022` | `other` | 692 | 7 | 7 | 116 | 116 | 0 | — |
| 37 | `src/overlays/o038/func_overlay_038_F0000000_1885D10.c` | `func_overlay_038_F0000000_1885D10` | `o038` | `other` | 340 | 7 | 7 | 72 | 72 | 0 | — |
| 38 | `src/main/level.c` | `levelGetCounts` | `main` | `other` | 1,036 | 8 | 7 | 80 | 88 | 0 | — |
| 39 | `src/overlays/o014/overlay14ResetMode.c` | `overlay14ResetMode` | `o014` | `other` | 224 | 11 | 7 | 20 | 20 | 0 | — |
| 40 | `src/main/font.c` | `func_8004BA8C` | `main` | `other` | 184 | 8 | 8 | 48 | 48 | 0 | — |
| 41 | `src/overlays/o001/overlay_001_tail.c` | `overlay1AssignRecordIndex` | `o001` | `other` | 176 | 8 | 8 | 28 | 28 | 0 | — |
| 42 | `src/overlays/o009/overlay_009.c` | `func_overlay_009_F0000540_1866BB8` | `o009` | `other` | 516 | 9 | 8 | 76 | 76 | 0 | — |
| 43 | `src/overlays/o001/overlay_001_end.c` | `overlay1ResolvePathPoint` | `o001` | `other` | 608 | 10 | 8 | 144 | 144 | 0 | — |
| 44 | `src/overlays/o073/overlay73Initialize.c` | `func_overlay_073_F0000000_18CAAC0` | `o073` | `other` | 400 | 10 | 8 | 64 | 64 | 0 | — |
| 45 | `src/main/particles.c` | `func_8003E8D8` | `main` | `other` | 560 | 9 | 9 | 56 | 56 | 0 | — |
| 46 | `src/overlays/o001/overlay_001_tail.c` | `overlay1AllocateRecord` | `o001` | `other` | 160 | 10 | 9 | 80 | 108 | 0 | — |
| 47 | `src/overlays/o059/overlay59PrepareEntry.c` | `overlay59PrepareEntry` | `o059` | `other` | 248 | 10 | 9 | 16 | 84 | 0 | — |
| 48 | `src/main/main.c` | `func_80028FCC` | `main` | `other` | 108 | 10 | 10 | 28 | 28 | 0 | — |
| 49 | `src/main/models_5B300.c` | `func_8005A7A0` | `main` | `other` | 424 | 10 | 10 | 0 | 0 | 0 | — |
| 50 | `src/overlays/o062/overlay62Update.c` | `overlay62Update` | `o062` | `other` | 1,176 | 11 | 10 | 68 | 68 | 0 | — |
| 51 | `src/overlays/o047/overlay47ReleaseResources.c` | `func_overlay_047_F00009D0_18917E8` | `o047` | `other` | 352 | 18 | 10 | 24 | 84 | 0 | — |
| 52 | `src/main/rcpFast3d.c` | `rcpClearZBuffer` | `main` | `other` | 428 | 11 | 11 | 116 | 116 | 0 | — |
| 53 | `src/overlays/o001/overlay_001_tail.c` | `overlay1FindBestRecord` | `o001` | `other` | 120 | 12 | 12 | 4 | 4 | 0 | — |
| 54 | `src/overlays/o034/overlay34RemoveRecord.c` | `overlay34RemoveRecord` | `o034` | `other` | 176 | 12 | 12 | 20 | 20 | 0 | — |
| 55 | `src/overlays/o098/overlay98CollectAccepted.c` | `overlay98CollectAccepted` | `o098` | `other` | 240 | 12 | 12 | 0 | 0 | 0 | — |
| 56 | `src/overlays/o001/overlay_001.c` | `overlay1FindPreviousUsable` | `o001` | `other` | 160 | 14 | 12 | 4 | 20 | 0 | — |
| 57 | `src/overlays/o001/overlay_001_head.c` | `overlay1InterpolatePath` | `o001` | `other` | 332 | 16 | 12 | 8 | 76 | 0 | — |
| 58 | `src/overlays/o015/overlay_015.c` | `overlay15DrawScreenStars` | `o015` | `other` | 420 | 14 | 13 | 52 | 56 | 0 | — |
| 59 | `src/overlays/o031/overlay31CreatePool.c` | `overlay31CreatePool` | `o031` | `other` | 200 | 14 | 13 | 0 | 0 | 0 | — |
| 60 | `src/overlays/o036/overlay36CheckNearbyHeight.c` | `func_overlay_036_F0000818_1883CD0` | `o036` | `other` | 252 | 14 | 13 | 0 | 0 | 0 | — |
| 61 | `src/overlays/o057/overlay57HandleModeInput.c` | `overlay57HandleModeInput` | `o057` | `other` | 868 | 17 | 13 | 20 | 32 | 0 | — |
| 62 | `src/overlays/o015/overlay_015.c` | `overlay15DrawRain` | `o015` | `other` | 216 | 18 | 13 | 8 | 116 | 0 | — |
| 63 | `src/overlays/o094/overlay94UpdateController.c` | `overlay94UpdateController` | `o094` | `other` | 1,100 | 24 | 13 | 28 | 56 | 0 | — |
| 64 | `src/main/memory.c` | `func_8002B524` | `main` | `other` | 464 | 14 | 14 | 224 | 224 | 0 | — |
| 65 | `src/overlays/o033/overlay33InitializeBuffers.c` | `overlay33InitializeBuffers` | `o033` | `other` | 324 | 16 | 14 | 4 | 116 | 0 | — |
| 66 | `src/main/anim.c` | `func_80050BF4` | `main` | `other` | 348 | 15 | 15 | 52 | 52 | 0 | — |
| 67 | `src/overlays/o004/overlay_004.c` | `overlay4UpdateObjectMotion` | `o004` | `other` | 920 | 15 | 15 | 68 | 68 | 0 | — |
| 68 | `src/overlays/o029/overlay29InitializeObject.c` | `func_overlay_029_F000042C_187D6DC` | `o029` | `other` | 408 | 15 | 15 | 20 | 20 | 0 | — |
| 69 | `src/overlays/o070/func_overlay_070_F00000D8_18C92A0.c` | `func_overlay_070_F00000D8_18C92A0` | `o070` | `other` | 684 | 15 | 15 | 196 | 196 | 0 | — |
| 70 | `src/overlays/o099/overlay99RenderSortedEntries.c` | `overlay99RenderSortedEntries` | `o099` | `other` | 932 | 17 | 15 | 100 | 100 | 0 | — |
| 71 | `src/overlays/o001/overlay_001_tail.c` | `overlay1StartTimerCallbacks` | `o001` | `other` | 224 | 20 | 15 | 44 | 80 | 0 | — |
| 72 | `src/overlays/o041/overlay41UpdateColorRecords.c` | `func_overlay_041_F0000124_188745C` | `o041` | `other` | 392 | 20 | 15 | 48 | 64 | 0 | — |
| 73 | `src/main/fx.c` | `func_8004ACC4` | `main` | `other` | 112 | 16 | 16 | 20 | 20 | 0 | — |
| 74 | `src/overlays/o043/overlay43ComputeMotion.c` | `func_overlay_043_F00010A8_188B078` | `o043` | `other` | 220 | 16 | 16 | 116 | 116 | 0 | — |
| 75 | `src/overlays/o007/overlay_007.c` | `overlay7AcquireEntry` | `o007` | `other` | 384 | 17 | 16 | 8 | 260 | 0 | — |
| 76 | `src/overlays/o014/overlay14CreateValue.c` | `overlay14CreateValue` | `o014` | `other` | 384 | 17 | 16 | 24 | 24 | 0 | — |
| 77 | `src/overlays/o041/overlay41SampleCurve.c` | `func_overlay_041_F00002AC_18875E4` | `o041` | `other` | 1,360 | 17 | 17 | 1,180 | 1,180 | 0 | — |
| 78 | `src/main/anim.c` | `func_8005716C` | `main` | `other` | 320 | 18 | 18 | 84 | 84 | 0 | — |
| 79 | `src/overlays/o001/overlay_001_tail.c` | `overlay1InitializeGaugeObjects` | `o001` | `other` | 296 | 18 | 18 | 44 | 44 | 0 | — |
| 80 | `src/overlays/o068/overlay68CheckKind.c` | `overlay68CheckKind` | `o068` | `other` | 320 | 18 | 18 | 20 | 20 | 0 | — |
| 81 | `src/main/fx.c` | `func_8004ADE8` | `main` | `other` | 384 | 19 | 19 | 96 | 96 | 0 | — |
| 82 | `src/main/level.c` | `levelFreeAll` | `main` | `other` | 468 | 19 | 19 | 224 | 224 | 0 | — |
| 83 | `src/overlays/o027/overlay_027.c` | `overlay27UpdateCoordinates` | `o027` | `other` | 260 | 19 | 19 | 0 | 0 | 0 | — |
| 84 | `src/overlays/o001/overlay_001.c` | `overlay1FindType47ByAngle` | `o001` | `other` | 296 | 20 | 19 | 8 | 8 | 0 | — |
| 85 | `src/overlays/o075/overlay75UpdateMovingObject.c` | `overlay75UpdateMovingObject` | `o075` | `other` | 1,216 | 20 | 19 | 84 | 84 | 0 | — |
| 86 | `src/overlays/o059/overlay59Advance.c` | `overlay59Advance` | `o059` | `other` | 1,048 | 22 | 19 | 4 | 4 | 0 | — |
| 87 | `src/main/fx.c` | `func_80047CD8` | `main` | `other` | 936 | 20 | 20 | 64 | 64 | 0 | — |
| 88 | `src/overlays/o001/overlay_001_tail.c` | `overlay1AppendPathPoint` | `o001` | `other` | 432 | 22 | 21 | 32 | 32 | 0 | — |
| 89 | `src/overlays/o033/overlay33PresentAndSwap.c` | `overlay33PresentAndSwap` | `o033` | `other` | 156 | 22 | 21 | 16 | 16 | 0 | — |
| 90 | `src/overlays/o060/overlay60Initialize.c` | `func_overlay_060_F0000000_18B9DD8` | `o060` | `other` | 820 | 46 | 21 | 124 | 124 | 0 | — |
| 91 | `src/overlays/o003/overlay3SelectScoredObject.c` | `overlay3SelectScoredObject` | `o003` | `other` | 472 | 22 | 22 | 68 | 68 | 0 | — |
| 92 | `src/overlays/o005/overlay_005.c` | `overlay5InitializeAudio` | `o005` | `other` | 932 | 22 | 22 | 156 | 156 | 0 | — |
| 93 | `src/overlays/o017/overlay17DrawStrip.c` | `overlay17DrawStrip` | `o017` | `other` | 476 | 22 | 22 | 0 | 0 | 0 | — |
| 94 | `src/overlays/o063/overlay63Initialize.c` | `overlay63Initialize` | `o063` | `other` | 468 | 28 | 22 | 76 | 184 | 0 | — |
| 95 | `src/main/models.c` | `func_80020E4C` | `main` | `other` | 452 | 25 | 25 | 12 | 12 | 0 | — |
| 96 | `src/overlays/o001/overlay_001_tail.c` | `overlay1BendPathPoint` | `o001` | `other` | 428 | 25 | 25 | 12 | 12 | 0 | — |
| 97 | `src/overlays/o096/overlay96Unregister.c` | `overlay96Unregister` | `o096` | `other` | 136 | 25 | 25 | 0 | 0 | 0 | — |
| 98 | `src/overlays/o057/overlay57UpdateModeTrigger.c` | `overlay57UpdateModeTrigger` | `o057` | `other` | 376 | 36 | 25 | 16 | 24 | 0 | — |
| 99 | `src/main/fx.c` | `func_8004AF68` | `main` | `other` | 208 | 26 | 26 | 16 | 16 | 0 | — |
| 100 | `src/overlays/o001/overlay_001_middle.c` | `overlay1AdvanceGauge` | `o001` | `other` | 168 | 26 | 26 | 20 | 20 | 0 | — |
| 101 | `src/overlays/o092/overlay92FindNearestCourse.c` | `func_overlay_092_F0000068_18D5F88` | `o092` | `other` | 672 | 26 | 26 | 4 | 4 | 0 | — |
| 102 | `src/overlays/o002/func_overlay_002_F0001A94_185888C.c` | `func_overlay_002_F0001A94_185888C` | `o002` | `other` | 868 | 51 | 27 | 76 | 312 | 0 | — |
| 103 | `src/overlays/o071/func_overlay_071_F0000278_18C9D98.c` | `func_overlay_071_F0000278_18C9D98` | `o071` | `other` | 1,328 | 28 | 28 | 168 | 168 | 0 | — |
| 104 | `src/overlays/o001/overlay_001_tail.c` | `overlay1DispatchMode` | `o001` | `other` | 796 | 39 | 28 | 52 | 108 | 0 | — |
| 105 | `src/main/charControl.c` | `func_8001D880` | `main` | `other` | 144 | 29 | 29 | 4 | 4 | 0 | — |
| 106 | `src/overlays/o041/overlay41EnqueueTransition.c` | `func_overlay_041_F000195C_1888C94` | `o041` | `other` | 420 | 29 | 29 | 8 | 8 | 0 | — |
| 107 | `src/overlays/o029/overlay29ProjectPoint.c` | `func_overlay_029_F0000EE0_187E190` | `o029` | `other` | 484 | 30 | 29 | 0 | 0 | 0 | — |
| 108 | `src/main/particles.c` | `func_8003F154` | `main` | `other` | 1,188 | 30 | 30 | 524 | 524 | 0 | — |
| 109 | `src/overlays/o098/overlay98CollectUniqueY.c` | `overlay98CollectUniqueY` | `o098` | `other` | 324 | 32 | 32 | 104 | 104 | 0 | — |
| 110 | `src/overlays/o029/overlay29HandleEffects.c` | `func_overlay_029_F00010C4_187E374` | `o029` | `other` | 1,028 | 33 | 33 | 124 | 124 | 0 | — |
| 111 | `src/overlays/o071/func_overlay_071_F0000870_18CA390.c` | `func_overlay_071_F0000870_18CA390` | `o071` | `other` | 728 | 33 | 33 | 112 | 112 | 0 | — |
| 112 | `src/overlays/o098/overlay98CheckObject.c` | `overlay98CheckObject` | `o098` | `other` | 444 | 33 | 33 | 0 | 0 | 0 | — |
| 113 | `src/main/saves.c` | `packInit` | `main` | `other` | 460 | 34 | 34 | 160 | 160 | 0 | — |
| 114 | `src/overlays/o001/overlay_001_tail.c` | `overlay1UpdateRangeFlags` | `o001` | `other` | 480 | 34 | 34 | 0 | 0 | 0 | — |
| 115 | `src/overlays/o003/overlay3RunCachedModeAction.c` | `overlay3RunCachedModeAction` | `o003` | `other` | 452 | 34 | 34 | 132 | 132 | 0 | — |
| 116 | `src/overlays/o041/overlay41UpdateProgress.c` | `func_overlay_041_F0001298_18885D0` | `o041` | `other` | 460 | 34 | 34 | 232 | 232 | 0 | — |
| 117 | `src/overlays/o043/overlay43FilterImage.c` | `overlay43FilterImage` | `o043` | `other` | 172 | 35 | 35 | 4 | 4 | 0 | — |
| 118 | `src/overlays/o060/overlay60ReassignChoiceSlots.c` | `overlay60ReassignChoiceSlots` | `o060` | `other` | 212 | 35 | 35 | 4 | 4 | 0 | — |
| 119 | `src/overlays/o034/overlay34CreateRecord.c` | `overlay34CreateRecord` | `o034` | `other` | 500 | 38 | 36 | 0 | 0 | 0 | — |
| 120 | `src/overlays/o001/overlay_001_head.c` | `overlay1ResolveMotionPoint` | `o001` | `other` | 400 | 37 | 37 | 44 | 44 | 0 | — |
| 121 | `src/overlays/o010/overlay10Initialize.c` | `overlay10Initialize` | `o010` | `other` | 688 | 37 | 37 | 0 | 0 | 0 | — |
| 122 | `src/main/charControl.c` | `func_8001BBB4` | `main` | `other` | 600 | 38 | 38 | 0 | 0 | 0 | — |
| 123 | `src/overlays/o101/overlay101BuildBorder.c` | `overlay101BuildBorder` | `o101` | `other` | 316 | 38 | 38 | 68 | 68 | 0 | — |
| 124 | `src/main/main.c` | `func_80029274` | `main` | `other` | 348 | 39 | 39 | 8 | 8 | 0 | — |
| 125 | `src/overlays/o026/overlay26HandleEffects.c` | `func_overlay_026_F0000D24_187B11C` | `o026` | `other` | 1,076 | 39 | 39 | 80 | 80 | 0 | — |
| 126 | `src/overlays/o074/overlay74Update.c` | `overlay74Update` | `o074` | `other` | 400 | 39 | 39 | 0 | 0 | 0 | — |
| 127 | `src/overlays/o002/overlay2QueryNode.c` | `overlay2QueryNode` | `o002` | `other` | 1,012 | 52 | 40 | 64 | 88 | 0 | — |
| 128 | `src/overlays/o019/overlay19BuildAdjacency.c` | `overlay19BuildAdjacency` | `o019` | `other` | 492 | 41 | 41 | 124 | 124 | 0 | — |
| 129 | `src/main/track.c` | `func_8000FAE0` | `main` | `other` | 248 | 43 | 43 | 28 | 28 | 0 | — |
| 130 | `src/overlays/o008/overlay_008.c` | `func_overlay_008_F0004CF0_1862A48` | `o008` | `other` | 1,080 | 48 | 43 | 376 | 376 | 0 | — |
| 131 | `src/main/matrix.c` | `MatrixMultiplyVec4` | `main` | `other` | 212 | 47 | 47 | 0 | 0 | 0 | — |
| 132 | `src/overlays/o073/overlay73Draw.c` | `func_overlay_073_F0000D70_18CB830` | `o073` | `other` | 312 | 47 | 47 | 28 | 28 | 0 | — |
| 133 | `src/overlays/o001/overlay_001.c` | `overlay1ActivateObject` | `o001` | `other` | 352 | 50 | 47 | 0 | 0 | 0 | — |
| 134 | `src/overlays/o029/overlay29DrawGroups.c` | `overlay29DrawGroups` | `o029` | `other` | 516 | 48 | 48 | 80 | 80 | 0 | — |
| 135 | `src/main/joy.c` | `joyRead` | `main` | `other` | 636 | 49 | 48 | 24 | 24 | 0 | — |
| 136 | `src/overlays/o008/overlay_008.c` | `func_overlay_008_F0000894_185E5EC` | `o008` | `other` | 1,524 | 57 | 48 | 204 | 1,064 | 0 | — |
| 137 | `src/overlays/o033/overlay33BuildDisplayList.c` | `overlay33BuildDisplayList` | `o033` | `other` | 1,232 | 50 | 50 | 448 | 448 | 0 | — |
| 138 | `src/main/anim.c` | `func_8005776C` | `main` | `other` | 420 | 51 | 51 | 36 | 36 | 0 | — |
| 139 | `src/overlays/o017/overlay17AdvanceChain.c` | `overlay17AdvanceChain` | `o017` | `other` | 588 | 51 | 51 | 24 | 24 | 0 | — |
| 140 | `src/overlays/o014/overlay14LoadRelocatedValue.c` | `overlay14LoadRelocatedValue` | `o014` | `other` | 376 | 53 | 52 | 4 | 8 | 0 | — |
| 141 | `src/main/audio_manager_36D0.c` | `func_80003480` | `main` | `other` | 376 | 53 | 53 | 0 | 0 | 0 | — |
| 142 | `src/overlays/o046/overlay46UpdateSequence.c` | `func_overlay_046_F0000120_188E518` | `o046` | `other` | 1,268 | 70 | 54 | 4 | 120 | 0 | — |
| 143 | `src/overlays/o008/overlay_008.c` | `func_overlay_008_F000291C_1860674` | `o008` | `other` | 1,444 | 60 | 56 | 0 | 0 | 0 | — |
| 144 | `src/overlays/o001/overlay_001_tail.c` | `overlay1UpdateAimedTransient` | `o001` | `other` | 996 | 64 | 57 | 0 | 0 | 0 | — |
| 145 | `src/main/fx.c` | `func_80048760` | `main` | `other` | 484 | 58 | 58 | 8 | 8 | 0 | — |
| 146 | `src/overlays/o089/overlay89InitializeEffect.c` | `overlay89InitializeEffect` | `o089` | `other` | 820 | 58 | 58 | 64 | 64 | 0 | — |
| 147 | `src/overlays/o035/overlay35BuildGridMasks.c` | `func_overlay_035_F0000770_1882450` | `o035` | `other` | 976 | 59 | 59 | 0 | 0 | 0 | — |
| 148 | `src/overlays/o057/overlay57Draw32A0.c` | `overlay57Draw32A0` | `o057` | `other` | 832 | 63 | 60 | 0 | 0 | 0 | — |
| 149 | `src/overlays/o101/overlay101DrawTransformed.c` | `overlay101DrawTransformed` | `o101` | `other` | 664 | 62 | 62 | 124 | 124 | 0 | — |
| 150 | `src/overlays/o008/overlay_008.c` | `func_overlay_008_F0002640_1860398` | `o008` | `other` | 732 | 63 | 62 | 184 | 184 | 0 | — |
| 151 | `src/main/saves.c` | `rumbleTick` | `main` | `other` | 1,372 | 64 | 64 | 244 | 244 | 0 | — |
| 152 | `src/main/spranim.c` | `effectboxControl` | `main` | `other` | 772 | 65 | 65 | 0 | 0 | 0 | — |
| 153 | `src/overlays/o026/overlay26DrawGroups.c` | `func_overlay_026_F0001158_187B550` | `o026` | `other` | 536 | 65 | 65 | 60 | 60 | 0 | — |
| 154 | `src/overlays/o014/overlay14PrepareInputState.c` | `overlay14PrepareInputState` | `o014` | `other` | 524 | 67 | 65 | 52 | 52 | 0 | — |
| 155 | `src/overlays/o019/overlay19BuildSpatialMasks.c` | `overlay19BuildSpatialMasks` | `o019` | `other` | 908 | 66 | 66 | 88 | 88 | 0 | — |
| 156 | `src/overlays/o027/overlay_027.c` | `func_overlay_027_F0000064_187BA3C` | `o027` | `other` | 1,472 | 67 | 66 | 24 | 24 | 0 | — |
| 157 | `src/main/particles.c` | `func_8003D25C` | `main` | `other` | 672 | 70 | 70 | 80 | 80 | 0 | — |
| 158 | `src/overlays/o058/overlay58DrawLargePointQuad.c` | `overlay58DrawLargePointQuad` | `o058` | `other` | 416 | 70 | 70 | 48 | 48 | 0 | — |
| 159 | `src/overlays/o058/overlay58DrawPointQuad.c` | `overlay58DrawPointQuad` | `o058` | `other` | 416 | 70 | 70 | 48 | 48 | 0 | — |
| 160 | `src/main/gameVi.c` | `func_800336A8` | `main` | `other` | 780 | 71 | 71 | 0 | 0 | 0 | — |
| 161 | `src/overlays/o036/overlay36ChooseWeightedState.c` | `func_overlay_036_F0000A60_1883F18` | `o036` | `other` | 680 | 71 | 71 | 60 | 60 | 0 | — |
| 162 | `src/overlays/o084/overlay84InitializeAndUpdate.c` | `overlay84InitializeAndUpdate` | `o084` | `other` | 716 | 71 | 71 | 0 | 0 | 0 | — |
| 163 | `src/overlays/o083/overlay83DrawStrip.c` | `overlay83DrawStrip` | `o083` | `other` | 308 | 73 | 73 | 4 | 4 | 0 | — |
| 164 | `src/overlays/o086/func_overlay_086_F0000474_18D22AC.c` | `func_overlay_086_F0000474_18D22AC` | `o086` | `other` | 2,648 | 73 | 73 | 112 | 112 | 0 | — |
| 165 | `src/overlays/o058/overlay58DrawSegmentStrip.c` | `overlay58DrawSegmentStrip` | `o058` | `other` | 804 | 74 | 74 | 164 | 164 | 0 | — |
| 166 | `src/overlays/o040/overlay40BuildFrame.c` | `overlay40BuildFrame` | `o040` | `other` | 324 | 75 | 75 | 4 | 4 | 0 | — |
| 167 | `src/overlays/o101/overlay101DrawPanel.c` | `overlay101DrawPanel` | `o101` | `other` | 1,072 | 79 | 79 | 180 | 180 | 0 | — |
| 168 | `src/overlays/o009/overlay_009.c` | `func_overlay_009_F00010B4_186772C` | `o009` | `other` | 1,128 | 86 | 79 | 136 | 136 | 0 | — |
| 169 | `src/overlays/o007/overlay_007_tail.c` | `overlay7UpdateOwnerMode` | `o007` | `other` | 556 | 81 | 81 | 16 | 16 | 0 | — |
| 170 | `src/overlays/o060/func_overlay_060_F0002F54_18BCD2C.c` | `func_overlay_060_F0002F54_18BCD2C` | `o060` | `other` | 888 | 84 | 84 | 100 | 100 | 0 | — |
| 171 | `src/overlays/o014/func_overlay_014_F0001540_1870E18.c` | `func_overlay_014_F0001540_1870E18` | `o014` | `other` | 752 | 89 | 88 | 136 | 136 | 0 | — |
| 172 | `src/overlays/o057/overlay57SmoothAndCheckDistance.c` | `overlay57SmoothAndCheckDistance` | `o057` | `other` | 800 | 97 | 92 | 36 | 36 | 0 | — |
| 173 | `src/overlays/o057/overlay57EaseAndLatch.c` | `overlay57EaseAndLatch` | `o057` | `other` | 884 | 101 | 92 | 20 | 44 | 0 | — |
| 174 | `src/overlays/o013/overlay13ProcessRecord.c` | `overlay13UpdateRecord` | `o013` | `other` | 644 | 96 | 96 | 44 | 44 | 0 | — |
| 175 | `src/overlays/o083/overlay83BuildBatch.c` | `overlay83BuildBatch` | `o083` | `other` | 672 | 97 | 97 | 0 | 0 | 0 | — |
| 176 | `src/overlays/o002/func_overlay_002_F0000C90_1857A88.c` | `func_overlay_002_F0000C90_1857A88` | `o002` | `other` | 1,420 | 108 | 98 | 0 | 0 | 0 | — |
| 177 | `src/overlays/o031/overlay31BuildLookupTables.c` | `func_overlay_031_F0000000_187F520` | `o031` | `other` | 744 | 104 | 104 | 52 | 52 | 0 | — |
| 178 | `src/overlays/o050/overlay50Initialize.c` | `func_overlay_050_F0000000_1896970` | `o050` | `other` | 740 | 112 | 104 | 60 | 192 | 0 | — |
| 179 | `src/overlays/o013/overlay13DrawRecord.c` | `overlay13DrawRecord` | `o013` | `other` | 756 | 108 | 105 | 0 | 0 | 0 | — |
| 180 | `src/overlays/o052/overlay52Initialize.c` | `func_overlay_052_F0000000_189A670` | `o052` | `other` | 1,264 | 121 | 117 | 44 | 44 | 0 | — |
| 181 | `src/overlays/o002/func_overlay_002_F0001364_185815C.c` | `func_overlay_002_F0001364_185815C` | `o002` | `other` | 756 | 125 | 125 | 8 | 8 | 0 | — |
| 182 | `src/overlays/o101/overlay101TailC6E8.c` | `func_overlay_101_F000C6E8_18E7F08` | `o101` | `other` | 1,268 | 131 | 125 | 52 | 52 | 0 | — |
| 183 | `src/overlays/o025/overlay_025.c` | `overlay25UpdateEffect` | `o025` | `other` | 1,036 | 127 | 127 | 0 | 0 | 0 | — |
| 184 | `src/overlays/o017/overlay17CreateChain.c` | `overlay17CreateChain` | `o017` | `other` | 784 | 130 | 130 | 0 | 0 | 0 | — |
| 185 | `src/overlays/o017/overlay17CalculateEndpoints.c` | `overlay17CalculateEndpoints` | `o017` | `other` | 792 | 133 | 133 | 36 | 36 | 0 | — |
| 186 | `src/overlays/o009/overlay_009.c` | `func_overlay_009_F0000000_1866678` | `o009` | `other` | 1,344 | 139 | 133 | 0 | 0 | 0 | — |
| 187 | `src/overlays/o063/overlay63UpdateEffects.c` | `overlay63UpdateEffects` | `o063` | `other` | 1,400 | 142 | 139 | 88 | 364 | 0 | — |
| 188 | `src/overlays/o079/func_overlay_079_F0000FA0_18CDF40.c` | `func_overlay_079_F0000FA0_18CDF40` | `o079` | `other` | 736 | 144 | 143 | 56 | 60 | 0 | — |
| 189 | `src/overlays/o068/overlay68DrawSortedEntries.c` | `overlay68DrawSortedEntries` | `o068` | `other` | 852 | 148 | 148 | 0 | 0 | 0 | — |
| 190 | `src/overlays/o101/overlay101BuildPresentationD.c` | `overlay101BuildPresentationD` | `o101` | `other` | 824 | 157 | 156 | 16 | 16 | 0 | — |
| 191 | `src/overlays/o092/func_overlay_092_F0000308_18D6228.c` | `func_overlay_092_F0000308_18D6228` | `o092` | `other` | 1,832 | 158 | 157 | 148 | 148 | 0 | — |
| 192 | `src/overlays/o100/overlay100DrawMotion.c` | `overlay100DrawMotion` | `o100` | `other` | 972 | 159 | 159 | 0 | 0 | 0 | — |
| 193 | `src/overlays/o038/overlay38UpdateParticles.c` | `func_overlay_038_F0000154_1885E64` | `o038` | `other` | 808 | 160 | 159 | 32 | 32 | 0 | — |
| 194 | `src/main/particles.c` | `func_80040B88` | `main` | `other` | 1,208 | 160 | 160 | 0 | 0 | 0 | — |
| 195 | `src/overlays/o057/overlay57UpdateSelection.c` | `overlay57UpdateSelection` | `o057` | `other` | 1,132 | 166 | 162 | 48 | 76 | 0 | — |
| 196 | `src/overlays/o058/overlay58FinalizePackedStatus.c` | `overlay58FinalizePackedStatus` | `o058` | `other` | 1,216 | 178 | 177 | 24 | 24 | 0 | — |
| 197 | `src/main/anim.c` | `func_80051364` | `main` | `other` | 1,148 | 192 | 192 | 0 | 0 | 0 | — |
| 198 | `src/main/track.c` | `func_800140CC` | `main` | `other` | 868 | 195 | 195 | 0 | 0 | 0 | — |
| 199 | `src/overlays/o027/overlay_027.c` | `func_overlay_027_F0000624_187BFFC` | `o027` | `other` | 1,016 | 196 | 195 | 8 | 8 | 0 | — |
| 200 | `src/overlays/o068/overlay68UpdateAnimation.c` | `overlay68UpdateAnimation` | `o068` | `other` | 1,424 | 208 | 208 | 28 | 28 | 0 | — |
| 201 | `src/main/rcpFast3d.c` | `func_8002EBE0` | `main` | `other` | 1,020 | 218 | 218 | 0 | 0 | 0 | — |
| 202 | `src/overlays/o041/overlay41UpdateCurveObject.c` | `func_overlay_041_F0000854_1887B8C` | `o041` | `other` | 2,552 | 225 | 222 | 120 | 120 | 0 | — |
| 203 | `src/overlays/o101/overlay101TailB544.c` | `func_overlay_101_F000B544_18E6D64` | `o101` | `other` | 1,264 | 225 | 225 | 44 | 44 | 0 | — |
| 204 | `src/overlays/o011/func_overlay_011_F00022E8_186AB30.c` | `func_overlay_011_F00022E8_186AB30` | `o011` | `other` | 1,068 | 231 | 227 | 4 | 4 | 0 | — |
| 205 | `src/overlays/o007/func_overlay_007_F0000324_185C1AC.c` | `func_overlay_007_F0000324_185C1AC` | `o007` | `other` | 1,392 | 242 | 238 | 0 | 0 | 0 | — |
| 206 | `src/overlays/o101/overlay101TailC144.c` | `func_overlay_101_F000C144_18E7964` | `o101` | `other` | 1,444 | 249 | 246 | 16 | 16 | 0 | — |
| 207 | `src/overlays/o002/overlay2ChooseBoundary.c` | `overlay2ChooseBoundary` | `o002` | `other` | 1,168 | 249 | 247 | 0 | 0 | 0 | — |
| 208 | `src/main/menu.c` | `func_80038E1C` | `main` | `other` | 1,116 | 248 | 248 | 36 | 36 | 0 | — |
| 209 | `src/overlays/o036/overlay36UpdateInteractiveEntity.c` | `overlay36UpdateInteractiveEntity` | `o036` | `other` | 1,220 | 251 | 248 | 0 | 0 | 0 | — |
| 210 | `src/overlays/o066/overlay66SmoothAndDraw.c` | `func_overlay_066_F0000040_18C64A8` | `o066` | `other` | 1,184 | 251 | 249 | 4 | 132 | 0 | — |
| 211 | `src/main/models.c` | `func_8002057C` | `main` | `other` | 1,368 | 251 | 251 | 0 | 0 | 0 | — |
| 212 | `src/overlays/o101/func_overlay_101_F000571C_18E0F3C.c` | `func_overlay_101_F000571C_18E0F3C` | `o101` | `other` | 1,772 | 293 | 293 | 44 | 44 | 0 | — |
| 213 | `src/overlays/o058/func_overlay_058_F0000000_18AF1E8.c` | `func_overlay_058_F0000000_18AF1E8` | `o058` | `other` | 1,472 | 306 | 296 | 0 | 0 | 0 | — |
| 214 | `src/overlays/o101/func_overlay_101_F00063F8_18E1C18.c` | `func_overlay_101_F00063F8_18E1C18` | `o101` | `other` | 1,520 | 304 | 304 | 44 | 44 | 0 | — |
| 215 | `src/overlays/o101/func_overlay_101_F000512C_18E094C.c` | `func_overlay_101_F000512C_18E094C` | `o101` | `other` | 1,520 | 305 | 305 | 44 | 44 | 0 | — |
| 216 | `src/overlays/o101/func_overlay_101_F0005E08_18E1628.c` | `func_overlay_101_F0005E08_18E1628` | `o101` | `other` | 1,520 | 305 | 305 | 44 | 44 | 0 | — |
| 217 | `src/overlays/o098/overlay98RenderReflections.c` | `overlay98RenderReflections` | `o098` | `other` | 1,556 | 335 | 334 | 0 | 0 | 0 | — |
| 218 | `src/overlays/o101/overlay101TailBA34.c` | `func_overlay_101_F000BA34_18E7254` | `o101` | `other` | 1,808 | 401 | 400 | 0 | 0 | 0 | — |
| 219 | `src/overlays/o064/overlay64GenerateTexture.c` | `func_overlay_064_F0000000_18C3B28` | `o064` | `other` | 1,680 | 401 | 401 | 0 | 0 | 0 | — |
| 220 | `src/main/vehicle_sounds.c` | `func_80058250` | `main` | `size-mismatch` | 88 | 19 | 19 | 0 | 0 | 16 | — |
| 221 | `src/overlays/o015/overlay_015.c` | `overlay15MoveStars` | `o015` | `size-mismatch` | 216 | 33 | 30 | 48 | 48 | 16 | — |
| 222 | `src/overlays/o001/overlay_001_tail.c` | `overlay1HandleCachedMode` | `o001` | `size-mismatch` | 128 | 32 | 31 | 4 | 20 | 16 | — |
| 223 | `src/main/fx.c` | `func_800498FC` | `main` | `size-mismatch` | 400 | 33 | 33 | 24 | 24 | -8 | — |
| 224 | `src/main/menu.c` | `func_8003968C` | `main` | `size-mismatch` | 148 | 33 | 33 | 0 | 0 | -36 | — |
| 225 | `src/overlays/o007/overlay_007_tail.c` | `overlay7CommitSelection` | `o007` | `size-mismatch` | 288 | 37 | 35 | 4 | 20 | -4 | — |
| 226 | `src/main/charControl.c` | `func_8001D2A0` | `main` | `size-mismatch` | 380 | 40 | 40 | 224 | 224 | 4 | — |
| 227 | `src/overlays/o040/overlay40UpdateEntries.c` | `overlay40UpdateEntries` | `o040` | `size-mismatch` | 184 | 44 | 44 | 8 | 8 | 4 | — |
| 228 | `src/main/lights.c` | `func_80019DE8` | `main` | `size-mismatch` | 252 | 45 | 45 | 68 | 68 | 4 | — |
| 229 | `src/overlays/o048/overlay48InitializeState.c` | `overlay48InitializeState` | `o048` | `size-mismatch` | 228 | 47 | 47 | 0 | 0 | -16 | — |
| 230 | `src/overlays/o001/overlay_001_tail.c` | `overlay1ConsumeNearbyPending` | `o001` | `size-mismatch` | 276 | 51 | 51 | 64 | 64 | 4 | — |
| 231 | `src/main/menu.c` | `func_80038878` | `main` | `size-mismatch` | 340 | 54 | 54 | 20 | 20 | 4 | — |
| 232 | `src/main/fx.c` | `func_80046EC4` | `main` | `size-mismatch` | 440 | 61 | 61 | 44 | 44 | 4 | — |
| 233 | `src/overlays/o002/overlay2ClassifyBoundary.c` | `overlay2ClassifyBoundary` | `o002` | `size-mismatch` | 316 | 62 | 61 | 4 | 20 | 4 | — |
| 234 | `src/main/memory.c` | `func_8002B7AC` | `main` | `size-mismatch` | 252 | 62 | 62 | 4 | 4 | -4 | — |
| 235 | `src/main/runlink.c` | `runlinkInit` | `main` | `size-mismatch` | 584 | 64 | 64 | 8 | 8 | -16 | — |
| 236 | `src/main/camera.c` | `func_80024978` | `main` | `size-mismatch` | 332 | 67 | 66 | 8 | 8 | 4 | — |
| 237 | `src/main/saves.c` | `func_8002CF6C` | `main` | `size-mismatch` | 352 | 77 | 77 | 8 | 8 | -12 | — |
| 238 | `src/main/matrix.c` | `func_8002AB78` | `main` | `size-mismatch` | 268 | 83 | 83 | 0 | 0 | 68 | — |
| 239 | `src/main/sched.c` | `__scHandleRetrace` | `main` | `size-mismatch` | 1,636 | 84 | 84 | 984 | 984 | -4 | — |
| 240 | `src/main/track.c` | `func_8000D820` | `main` | `size-mismatch` | 344 | 84 | 84 | 0 | 0 | -8 | — |
| 241 | `src/overlays/o015/overlay_015.c` | `overlay15UpdateMovingStars` | `o015` | `size-mismatch` | 412 | 87 | 84 | 36 | 48 | 28 | — |
| 242 | `src/main/fx.c` | `func_80048080` | `main` | `size-mismatch` | 356 | 85 | 85 | 0 | 0 | -20 | — |
| 243 | `src/main/diCpu.c` | `func_80046BCC` | `main` | `size-mismatch` | 424 | 89 | 89 | 44 | 44 | 4 | — |
| 244 | `src/main/matrix.c` | `func_8002AA50` | `main` | `size-mismatch` | 296 | 92 | 92 | 0 | 0 | 76 | — |
| 245 | `src/overlays/o001/overlay_001_tail.c` | `overlay1SolveAngleCandidates` | `o001` | `size-mismatch` | 556 | 92 | 92 | 168 | 168 | -8 | — |
| 246 | `src/main/models_5B300.c` | `func_8005ABA8` | `main` | `size-mismatch` | 444 | 97 | 97 | 56 | 56 | -4 | — |
| 247 | `src/main/track.c` | `func_800133FC` | `main` | `size-mismatch` | 384 | 98 | 98 | 0 | 0 | 12 | — |
| 248 | `src/overlays/o089/overlay89UpdateStateAndParticles.c` | `overlay89UpdateStateAndParticles` | `o089` | `size-mismatch` | 544 | 98 | 98 | 0 | 0 | 4 | — |
| 249 | `src/main/particles.c` | `func_8004054C` | `main` | `size-mismatch` | 500 | 101 | 101 | 44 | 44 | -4 | — |
| 250 | `src/overlays/o063/overlay63UpdateSequence.c` | `overlay63UpdateSequence` | `o063` | `size-mismatch` | 428 | 102 | 102 | 20 | 20 | -4 | — |
| 251 | `src/overlays/o099/overlay99BuildHeightGrid.c` | `overlay99BuildHeightGrid` | `o099` | `size-mismatch` | 456 | 104 | 104 | 44 | 44 | 4 | — |
| 252 | `src/main/font.c` | `func_8004C690` | `main` | `size-mismatch` | 584 | 105 | 105 | 0 | 0 | -8 | — |
| 253 | `src/overlays/o068/overlay68RebuildSecondaryEntry.c` | `overlay68RebuildSecondaryEntry` | `o068` | `size-mismatch` | 488 | 105 | 105 | 0 | 0 | -8 | — |
| 254 | `src/main/weather_tail.c` | `func_8003C80C` | `main` | `size-mismatch` | 472 | 106 | 106 | 16 | 16 | -4 | — |
| 255 | `src/overlays/o001/overlay_001_head.c` | `overlay1BuildObjectMappings` | `o001` | `size-mismatch` | 592 | 114 | 114 | 0 | 0 | -16 | — |
| 256 | `src/main/runlink.c` | `runlinkFreeCode` | `main` | `size-mismatch` | 736 | 117 | 117 | 0 | 0 | -4 | — |
| 257 | `src/main/matrix.c` | `func_8002AC84` | `main` | `size-mismatch` | 396 | 118 | 118 | 0 | 0 | 80 | — |
| 258 | `src/overlays/o041/overlay41ProcessEntry.c` | `func_overlay_041_F0001464_188879C` | `o041` | `size-mismatch` | 492 | 120 | 120 | 16 | 16 | 20 | — |
| 259 | `src/main/fx.c` | `func_800470B0` | `main` | `size-mismatch` | 596 | 121 | 121 | 68 | 68 | 4 | — |
| 260 | `src/main/track.c` | `func_8000D1B8` | `main` | `size-mismatch` | 512 | 121 | 121 | 4 | 4 | -16 | — |
| 261 | `src/overlays/o031/overlay31InitializeParticleAssets.c` | `func_overlay_031_F00002E8_187F808` | `o031` | `size-mismatch` | 528 | 123 | 123 | 16 | 16 | 4 | — |
| 262 | `src/overlays/o001/overlay_001_tail.c` | `overlay1AdvancePath` | `o001` | `size-mismatch` | 648 | 124 | 124 | 16 | 16 | 12 | — |
| 263 | `src/main/fx.c` | `func_80049000` | `main` | `size-mismatch` | 596 | 125 | 125 | 0 | 0 | 4 | — |
| 264 | `src/main/runlink.c` | `ProcessRelocationEntry` | `main` | `size-mismatch` | 584 | 126 | 126 | 0 | 0 | 4 | — |
| 265 | `src/overlays/o054/overlay54Initialize.c` | `func_overlay_054_F0000000_189ECA0` | `o054` | `size-mismatch` | 972 | 144 | 127 | 84 | 240 | 4 | — |
| 266 | `src/main/spranim.c` | `func_8001B798` | `main` | `size-mismatch` | 700 | 131 | 131 | 0 | 0 | -16 | — |
| 267 | `src/overlays/o044/overlay44UpdateFrameCache.c` | `overlay44UpdateFrameCache` | `o044` | `size-mismatch` | 748 | 133 | 133 | 0 | 0 | -4 | — |
| 268 | `src/main/matrix.c` | `func_8002AE10` | `main` | `size-mismatch` | 348 | 138 | 138 | 0 | 0 | 208 | — |
| 269 | `src/overlays/o037/overlay37Render.c` | `overlay37RenderEffect` | `o037` | `size-mismatch` | 856 | 139 | 139 | 0 | 0 | -4 | — |
| 270 | `src/main/track.c` | `func_80010900` | `main` | `size-mismatch` | 588 | 143 | 143 | 4 | 4 | -8 | — |
| 271 | `src/main/track.c` | `func_8000D3B8` | `main` | `size-mismatch` | 440 | 144 | 144 | 4 | 4 | 160 | — |
| 272 | `src/overlays/o015/overlay_015.c` | `overlay15InitStars` | `o015` | `size-mismatch` | 760 | 144 | 144 | 100 | 100 | -12 | — |
| 273 | `src/main/sched.c` | `func_80030610` | `main` | `size-mismatch` | 768 | 148 | 148 | 0 | 0 | 8 | — |
| 274 | `src/main/fx.c` | `func_8004A10C` | `main` | `size-mismatch` | 628 | 155 | 155 | 0 | 0 | -4 | — |
| 275 | `src/main/models.c` | `func_80020B10` | `main` | `size-mismatch` | 636 | 156 | 156 | 0 | 0 | 4 | — |
| 276 | `src/main/track.c` | `func_80012658` | `main` | `size-mismatch` | 708 | 157 | 157 | 0 | 0 | -12 | — |
| 277 | `src/main/fx.c` | `func_80049E4C` | `main` | `size-mismatch` | 676 | 159 | 159 | 8 | 8 | -8 | — |
| 278 | `src/overlays/o008/overlay_008.c` | `func_overlay_008_F0001000_185ED58` | `o008` | `size-mismatch` | 660 | 159 | 159 | 8 | 8 | -4 | — |
| 279 | `src/overlays/o001/overlay_001_tail.c` | `func_overlay_001_F0003FD8_18503B8` | `o001` | `size-mismatch` | 948 | 162 | 162 | 32 | 32 | -12 | — |
| 280 | `src/overlays/o013/overlay13DrawActive.c` | `overlay13DrawActive` | `o013` | `size-mismatch` | 664 | 163 | 162 | 0 | 0 | -4 | — |
| 281 | `src/main/lights.c` | `func_80018F08` | `main` | `size-mismatch` | 820 | 166 | 166 | 0 | 0 | -24 | — |
| 282 | `src/overlays/o034/overlay34SortAndDraw.c` | `overlay34SortAndDraw` | `o034` | `size-mismatch` | 760 | 168 | 168 | 0 | 0 | -4 | — |
| 283 | `src/overlays/o101/overlay101BuildPresentationA.c` | `overlay101BuildPresentationA` | `o101` | `size-mismatch` | 832 | 169 | 168 | 16 | 16 | 4 | — |
| 284 | `src/overlays/o101/overlay101BuildPresentationB.c` | `overlay101BuildPresentationB` | `o101` | `size-mismatch` | 832 | 169 | 168 | 16 | 16 | 4 | — |
| 285 | `src/overlays/o101/overlay101BuildPresentationC.c` | `overlay101BuildPresentationC` | `o101` | `size-mismatch` | 832 | 169 | 168 | 16 | 16 | 4 | — |
| 286 | `src/main/track.c` | `func_80010654` | `main` | `size-mismatch` | 684 | 172 | 172 | 0 | 0 | 12 | — |
| 287 | `src/main/fx.c` | `func_80047304` | `main` | `size-mismatch` | 740 | 176 | 176 | 4 | 4 | -8 | — |
| 288 | `src/main/fx.c` | `func_800479D4` | `main` | `size-mismatch` | 772 | 176 | 176 | 56 | 56 | -56 | — |
| 289 | `src/main/fx.c` | `wakeDraw` | `main` | `size-mismatch` | 708 | 177 | 177 | 0 | 0 | -56 | — |
| 290 | `src/main/track.c` | `func_800103D4` | `main` | `size-mismatch` | 640 | 177 | 177 | 0 | 0 | 68 | — |
| 291 | `src/main/lights.c` | `func_80019AB8` | `main` | `size-mismatch` | 736 | 178 | 178 | 0 | 0 | 28 | — |
| 292 | `src/overlays/o038/func_overlay_038_F000047C_188618C.c` | `func_overlay_038_F000047C_188618C` | `o038` | `size-mismatch` | 876 | 178 | 178 | 0 | 0 | -4 | — |
| 293 | `src/overlays/o020/func_overlay_020_F0001148_1877720.c` | `func_overlay_020_F0001148_1877720` | `o020` | `size-mismatch` | 828 | 180 | 178 | 32 | 32 | -8 | — |
| 294 | `src/main/shadows.c` | `func_800180B4` | `main` | `size-mismatch` | 824 | 179 | 179 | 52 | 52 | -8 | — |
| 295 | `src/overlays/o066/func_overlay_066_F00004E0_18C6948.c` | `func_overlay_066_F00004E0_18C6948` | `o066` | `size-mismatch` | 816 | 181 | 181 | 0 | 0 | -12 | — |
| 296 | `src/overlays/o020/overlay20UpdateGrid.c` | `overlay20UpdateGrid` | `o020` | `size-mismatch` | 860 | 184 | 184 | 0 | 0 | 16 | — |
| 297 | `src/overlays/o011/func_overlay_011_F0001E4C_186A694.c` | `func_overlay_011_F0001E4C_186A694` | `o011` | `size-mismatch` | 1,180 | 189 | 186 | 216 | 216 | -4 | — |
| 298 | `src/main/menu.c` | `func_80039E34` | `main` | `size-mismatch` | 1,048 | 190 | 190 | 20 | 20 | 8 | — |
| 299 | `src/overlays/o087/func_overlay_087_F0000128_18D3090.c` | `func_overlay_087_F0000128_18D3090` | `o087` | `size-mismatch` | 1,896 | 195 | 193 | 44 | 44 | -16 | — |
| 300 | `src/main/anim.c` | `func_800573C8` | `main` | `size-mismatch` | 932 | 203 | 203 | 44 | 44 | -8 | — |
| 301 | `src/overlays/o099/overlay99ApplySegment.c` | `overlay99ApplySegment` | `o099` | `size-mismatch` | 920 | 206 | 205 | 0 | 0 | -4 | — |
| 302 | `src/main/main.c` | `func_80026FB4` | `main` | `size-mismatch` | 1,652 | 207 | 207 | 56 | 56 | 24 | — |
| 303 | `src/main/charControl.c` | `func_8001D960` | `main` | `size-mismatch` | 880 | 208 | 208 | 0 | 0 | 4 | — |
| 304 | `src/main/track.c` | `func_80011980` | `main` | `size-mismatch` | 860 | 208 | 208 | 0 | 0 | -4 | — |
| 305 | `src/main/track.c` | `func_80012234` | `main` | `size-mismatch` | 832 | 208 | 208 | 0 | 0 | -132 | — |
| 306 | `src/main/anim.c` | `func_80056DD8` | `main` | `size-mismatch` | 916 | 214 | 214 | 0 | 0 | -12 | — |
| 307 | `src/main/camera.c` | `func_80022FD4` | `main` | `size-mismatch` | 1,476 | 216 | 216 | 44 | 44 | -16 | — |
| 308 | `src/main/fx.c` | `func_80049B14` | `main` | `size-mismatch` | 824 | 216 | 216 | 8 | 8 | 52 | — |
| 309 | `src/main/fx.c` | `fxSPDPRipple` | `main` | `size-mismatch` | 928 | 226 | 226 | 8 | 8 | 16 | — |
| 310 | `src/main/track.c` | `func_8000F198` | `main` | `size-mismatch` | 996 | 228 | 228 | 0 | 0 | -8 | — |
| 311 | `src/overlays/o015/overlay_015.c` | `overlay15InitStarsAndPalette` | `o015` | `size-mismatch` | 988 | 231 | 230 | 4 | 4 | -4 | — |
| 312 | `src/main/track.c` | `func_800115E4` | `main` | `size-mismatch` | 924 | 232 | 232 | 0 | 0 | 20 | — |
| 313 | `src/main/charControl.c` | `func_8001EC44` | `main` | `size-mismatch` | 952 | 236 | 236 | 0 | 0 | 8 | — |
| 314 | `src/overlays/o020/func_overlay_020_F000038C_1876964.c` | `func_overlay_020_F000038C_1876964` | `o020` | `size-mismatch` | 1,080 | 256 | 256 | 0 | 0 | -24 | — |
| 315 | `src/main/camera.c` | `func_80023598` | `main` | `size-mismatch` | 1,136 | 263 | 263 | 0 | 0 | 8 | — |
| 316 | `src/overlays/o057/func_overlay_057_F00060F8_18A9CF0.c` | `func_overlay_057_F00060F8_18A9CF0` | `o057` | `size-mismatch` | 1,764 | 278 | 263 | 0 | 0 | -16 | — |
| 317 | `src/main/lights.c` | `func_8001953C` | `main` | `size-mismatch` | 1,016 | 266 | 266 | 0 | 0 | 188 | — |
| 318 | `src/main/track.c` | `func_8000DB34` | `main` | `size-mismatch` | 688 | 267 | 267 | 0 | 0 | 380 | — |
| 319 | `src/overlays/o101/overlay101TailA6BC.c` | `overlay101TailA6BC` | `o101` | `size-mismatch` | 1,168 | 268 | 267 | 0 | 0 | -8 | — |
| 320 | `src/overlays/o101/func_overlay_101_F0002510_18DDD30.c` | `func_overlay_101_F0002510_18DDD30` | `o101` | `size-mismatch` | 1,172 | 276 | 276 | 0 | 0 | -52 | — |
| 321 | `src/main/particles.c` | `func_80041530` | `main` | `size-mismatch` | 1,824 | 280 | 280 | 0 | 0 | 4 | — |
| 322 | `src/main/track.c` | `func_8000E5EC` | `main` | `size-mismatch` | 820 | 283 | 283 | 0 | 0 | 312 | — |
| 323 | `src/overlays/o035/func_overlay_035_F00001E0_1881EC0.c` | `func_overlay_035_F00001E0_1881EC0` | `o035` | `size-mismatch` | 1,424 | 283 | 283 | 120 | 120 | -12 | — |
| 324 | `src/main/models.c` | `func_8001FC50` | `main` | `size-mismatch` | 1,332 | 300 | 300 | 0 | 0 | -12 | — |
| 325 | `src/overlays/o043/func_overlay_043_F0000BE4_188ABB4.c` | `func_overlay_043_F0000BE4_188ABB4` | `o043` | `size-mismatch` | 1,220 | 307 | 307 | 4 | 4 | 28 | — |
| 326 | `src/main/rcpFast3d.c` | `func_8002F618` | `main` | `size-mismatch` | 1,308 | 312 | 312 | 52 | 52 | -156 | — |
| 327 | `src/main/shadows.c` | `func_80017BCC` | `main` | `size-mismatch` | 1,256 | 313 | 312 | 0 | 0 | 48 | — |
| 328 | `src/main/track.c` | `func_8001357C` | `main` | `size-mismatch` | 1,040 | 315 | 315 | 0 | 0 | 280 | — |
| 329 | `src/main/diCpu.c` | `render_epc_lock_up_display` | `main` | `size-mismatch` | 1,376 | 321 | 321 | 60 | 60 | 12 | — |
| 330 | `src/overlays/o044/func_overlay_044_F0000580_188BDE0.c` | `func_overlay_044_F0000580_188BDE0` | `o044` | `size-mismatch` | 1,396 | 329 | 329 | 0 | 0 | -8 | — |
| 331 | `src/main/shadows.c` | `func_80017140` | `main` | `size-mismatch` | 1,312 | 337 | 337 | 0 | 0 | 64 | — |
| 332 | `src/main/track.c` | `func_80011CDC` | `main` | `size-mismatch` | 1,368 | 339 | 339 | 0 | 0 | -56 | — |
| 333 | `src/main/fx.c` | `wakeAllocate` | `main` | `size-mismatch` | 1,404 | 340 | 340 | 12 | 12 | -232 | — |
| 334 | `src/overlays/o012/func_overlay_012_F00003A8_186D628.c` | `func_overlay_012_F00003A8_186D628` | `o012` | `size-mismatch` | 1,384 | 340 | 340 | 16 | 16 | 12 | — |
| 335 | `src/main/track.c` | `func_8000BDB4` | `main` | `size-mismatch` | 1,612 | 342 | 342 | 48 | 48 | -40 | — |
| 336 | `src/overlays/o057/func_overlay_057_F0000000_18A3BF8.c` | `func_overlay_057_F0000000_18A3BF8` | `o057` | `size-mismatch` | 2,388 | 371 | 345 | 8 | 8 | -12 | — |
| 337 | `src/main/rcpFast3d.c` | `func_8002FB34` | `main` | `size-mismatch` | 1,436 | 358 | 358 | 0 | 0 | -16 | — |
| 338 | `src/overlays/o001/overlay_001_tail.c` | `func_overlay_001_F0003750_184FB30` | `o001` | `size-mismatch` | 1,784 | 364 | 360 | 12 | 12 | 4 | — |
| 339 | `src/overlays/o046/func_overlay_046_F0000874_188EC6C.c` | `func_overlay_046_F0000874_188EC6C` | `o046` | `size-mismatch` | 1,800 | 364 | 363 | 0 | 0 | 12 | — |
| 340 | `src/main/track.c` | `func_8001398C` | `main` | `size-mismatch` | 1,320 | 365 | 365 | 0 | 0 | 160 | — |
| 341 | `src/main/anim.c` | `func_80054B3C` | `main` | `size-mismatch` | 1,480 | 368 | 368 | 0 | 0 | -112 | — |
| 342 | `src/main/shadows.c` | `func_80017660` | `main` | `size-mismatch` | 1,388 | 368 | 368 | 0 | 0 | 92 | — |
| 343 | `src/main/track.c` | `func_8000DFBC` | `main` | `size-mismatch` | 1,584 | 380 | 380 | 0 | 0 | -164 | — |
| 344 | `src/overlays/o008/overlay_008.c` | `func_overlay_008_F00042A8_1862000` | `o008` | `size-mismatch` | 1,788 | 384 | 384 | 68 | 68 | -68 | — |
| 345 | `src/main/fx.c` | `wakeUpdate` | `main` | `size-mismatch` | 1,592 | 386 | 386 | 0 | 0 | -72 | — |
| 346 | `src/main/fx.c` | `func_800475E8` | `main` | `size-mismatch` | 1,004 | 390 | 390 | 0 | 0 | 780 | — |
| 347 | `src/main/models.c` | `func_8001F520` | `main` | `size-mismatch` | 1,604 | 394 | 394 | 8 | 8 | -228 | — |
| 348 | `src/overlays/o046/func_overlay_046_F0001228_188F620.c` | `func_overlay_046_F0001228_188F620` | `o046` | `size-mismatch` | 1,844 | 416 | 415 | 92 | 92 | -96 | — |
| 349 | `src/main/main.c` | `func_80028564` | `main` | `size-mismatch` | 1,956 | 427 | 426 | 4 | 4 | 12 | — |
| 350 | `src/overlays/o022/func_overlay_022_F00002B0_18783B8.c` | `func_overlay_022_F00002B0_18783B8` | `o022` | `size-mismatch` | 1,996 | 431 | 430 | 4 | 4 | 8 | — |
| 351 | `src/overlays/o029/func_overlay_029_F00005C4_187D874.c` | `func_overlay_029_F00005C4_187D874` | `o029` | `size-mismatch` | 2,332 | 438 | 438 | 4 | 4 | -4 | — |
| 352 | `src/main/diCpu.c` | `func_80045D34` | `main` | `size-mismatch` | 1,836 | 440 | 439 | 0 | 0 | -28 | — |
| 353 | `src/overlays/o055/func_overlay_055_F000031C_18A1E34.c` | `func_overlay_055_F000031C_18A1E34` | `o055` | `size-mismatch` | 2,324 | 445 | 445 | 60 | 60 | -24 | — |
| 354 | `src/overlays/o002/func_overlay_002_F0001DF8_1858BF0.c` | `func_overlay_002_F0001DF8_1858BF0` | `o002` | `size-mismatch` | 1,840 | 448 | 448 | 4 | 4 | -20 | — |
| 355 | `src/overlays/o057/func_overlay_057_F0004460_18A8058.c` | `func_overlay_057_F0004460_18A8058` | `o057` | `size-mismatch` | 1,976 | 450 | 449 | 4 | 4 | -144 | — |
| 356 | `src/overlays/o084/func_overlay_084_F0000314_18D07F4.c` | `func_overlay_084_F0000314_18D07F4` | `o084` | `size-mismatch` | 1,856 | 450 | 449 | 60 | 60 | 16 | — |
| 357 | `src/overlays/o101/func_overlay_101_F0008128_18E3948.c` | `func_overlay_101_F0008128_18E3948` | `o101` | `size-mismatch` | 2,100 | 472 | 469 | 4 | 4 | -4 | — |
| 358 | `src/overlays/o101/func_overlay_101_F000895C_18E417C.c` | `func_overlay_101_F000895C_18E417C` | `o101` | `size-mismatch` | 2,100 | 472 | 469 | 4 | 4 | -4 | — |
| 359 | `src/overlays/o101/func_overlay_101_F0009190_18E49B0.c` | `func_overlay_101_F0009190_18E49B0` | `o101` | `size-mismatch` | 2,100 | 472 | 469 | 4 | 4 | -4 | — |
| 360 | `src/overlays/o001/overlay_001_head.c` | `overlay1LoadBuildRecords` | `o001` | `size-mismatch` | 2,288 | 491 | 470 | 52 | 52 | -92 | — |
| 361 | `src/overlays/o101/func_overlay_101_F00078F4_18E3114.c` | `func_overlay_101_F00078F4_18E3114` | `o101` | `size-mismatch` | 2,100 | 480 | 477 | 4 | 4 | -4 | — |
| 362 | `src/overlays/o008/overlay_008.c` | `func_overlay_008_F0000058_185DDB0` | `o008` | `size-mismatch` | 2,108 | 482 | 477 | 0 | 0 | -76 | — |
| 363 | `src/overlays/o035/func_overlay_035_F0000B40_1882820.c` | `func_overlay_035_F0000B40_1882820` | `o035` | `size-mismatch` | 2,112 | 486 | 486 | 4 | 4 | -48 | — |
| 364 | `src/main/shadows.c` | `shadowGenerate` | `main` | `size-mismatch` | 2,040 | 490 | 490 | 12 | 12 | -220 | — |
| 365 | `src/main/font.c` | `func_8004B1DC` | `main` | `size-mismatch` | 2,224 | 509 | 509 | 48 | 48 | -112 | — |
| 366 | `src/overlays/o026/func_overlay_026_F00001A0_187A598.c` | `func_overlay_026_F00001A0_187A598` | `o026` | `size-mismatch` | 2,424 | 510 | 510 | 156 | 156 | -16 | — |
| 367 | `src/overlays/o019/overlay19BuildPlanes.c` | `overlay19BuildPlanes` | `o019` | `size-mismatch` | 2,128 | 514 | 514 | 72 | 72 | -8 | — |
| 368 | `src/overlays/o043/func_overlay_043_F0000324_188A2F4.c` | `func_overlay_043_F0000324_188A2F4` | `o043` | `size-mismatch` | 2,240 | 529 | 529 | 64 | 64 | -12 | — |
| 369 | `src/overlays/o011/func_overlay_011_F0000150_1868998.c` | `func_overlay_011_F0000150_1868998` | `o011` | `size-mismatch` | 2,248 | 538 | 535 | 0 | 0 | -120 | — |
| 370 | `src/main/track.c` | `func_8000E920` | `main` | `size-mismatch` | 2,168 | 536 | 536 | 56 | 56 | 48 | — |
| 371 | `src/main/track.c` | `func_8001291C` | `main` | `size-mismatch` | 2,192 | 545 | 545 | 0 | 0 | -124 | — |
| 372 | `src/overlays/o047/func_overlay_047_F0000000_1890E18.c` | `func_overlay_047_F0000000_1890E18` | `o047` | `size-mismatch` | 2,512 | 553 | 548 | 104 | 200 | 16 | — |
| 373 | `src/main/shadows.c` | `func_80016890` | `main` | `size-mismatch` | 2,224 | 556 | 556 | 0 | 0 | -188 | — |
| 374 | `src/overlays/o090/overlay_090.c` | `func_overlay_090_F00000FC_18D4BF4` | `o090` | `size-mismatch` | 2,592 | 575 | 575 | 0 | 0 | -36 | — |
| 375 | `src/overlays/o012/func_overlay_012_F0000910_186DB90.c` | `func_overlay_012_F0000910_186DB90` | `o012` | `size-mismatch` | 2,444 | 582 | 582 | 0 | 0 | -20 | — |
| 376 | `src/overlays/o057/func_overlay_057_F0001020_18A4C18.c` | `func_overlay_057_F0001020_18A4C18` | `o057` | `size-mismatch` | 2,392 | 585 | 585 | 0 | 0 | -32 | — |
| 377 | `src/overlays/o045/func_overlay_045_F0001158_188D5B0.c` | `func_overlay_045_F0001158_188D5B0` | `o045` | `size-mismatch` | 2,696 | 593 | 593 | 0 | 0 | 28 | — |
| 378 | `src/overlays/o101/overlay101TailAB4C.c` | `func_overlay_101_F000AB4C_18E636C` | `o101` | `size-mismatch` | 2,552 | 636 | 636 | 4 | 4 | 108 | — |
| 379 | `src/main/track.c` | `func_80010B4C` | `main` | `size-mismatch` | 2,712 | 677 | 677 | 4 | 4 | -136 | — |
| 380 | `src/overlays/o065/overlay65UpdateParticles.c` | `overlay65UpdateParticles` | `o065` | `size-mismatch` | 2,880 | 696 | 689 | 12 | 12 | -40 | — |
| 381 | `src/main/vehicle_sounds.c` | `func_8005830C` | `main` | `size-mismatch` | 3,048 | 700 | 699 | 0 | 0 | -16 | — |
| 382 | `src/overlays/o053/func_overlay_053_F0000240_189DBE8.c` | `func_overlay_053_F0000240_189DBE8` | `o053` | `size-mismatch` | 2,544 | 710 | 709 | 0 | 0 | 308 | — |
| 383 | `src/overlays/o058/func_overlay_058_F00005FC_18AF7E4.c` | `func_overlay_058_F00005FC_18AF7E4` | `o058` | `size-mismatch` | 3,316 | 731 | 724 | 0 | 0 | -12 | — |
| 384 | `src/overlays/o065/func_overlay_065_F0000C38_18C4EA0.c` | `func_overlay_065_F0000C38_18C4EA0` | `o065` | `size-mismatch` | 3,548 | 824 | 824 | 0 | 0 | -88 | — |
| 385 | `src/overlays/o079/func_overlay_079_F0000134_18CD0D4.c` | `func_overlay_079_F0000134_18CD0D4` | `o079` | `size-mismatch` | 3,528 | 848 | 848 | 4 | 4 | -48 | — |
| 386 | `src/overlays/o101/func_overlay_101_F00069E8_18E2208.c` | `func_overlay_101_F00069E8_18E2208` | `o101` | `size-mismatch` | 3,852 | 850 | 849 | 44 | 44 | 16 | — |
| 387 | `src/overlays/o008/overlay_008.c` | `func_overlay_008_F00034A0_18611F8` | `o008` | `size-mismatch` | 3,592 | 991 | 991 | 0 | 0 | 448 | — |
| 388 | `src/overlays/o101/func_overlay_101_F0003A58_18DF278.c` | `func_overlay_101_F0003A58_18DF278` | `o101` | `size-mismatch` | 5,844 | 1,396 | 1,396 | 52 | 52 | 8 | — |
| 389 | `src/overlays/o001/overlay_001_tail.c` | `func_overlay_001_F000438C_185076C` | `o001` | `size-mismatch` | 6,168 | 1,522 | 1,519 | 0 | 0 | 216 | — |
| 390 | `src/overlays/o050/func_overlay_050_F0000334_1896CA4.c` | `func_overlay_050_F0000334_1896CA4` | `o050` | `size-mismatch` | 6,300 | 1,544 | 1,544 | 0 | 0 | -12 | — |
| 391 | `src/overlays/o047/func_overlay_047_F0000B30_1891948.c` | `func_overlay_047_F0000B30_1891948` | `o047` | `size-mismatch` | 8,672 | 2,701 | 2,701 | 0 | 0 | 2,176 | — |
| 392 | `src/overlays/o058/func_overlay_058_F000138C_18B0574.c` | `func_overlay_058_F000138C_18B0574` | `o058` | `size-mismatch` | 14,456 | 3,547 | 3,544 | 0 | 0 | -472 | — |

### Unresolved identities

None.
<!-- NM_RANKING_GENERATED_END -->

## Recommended batching for the fleet

1. **`register-only` and `schedule-only` first** -- the
   cheapest category by construction: the candidate already has the right
   instructions, just the wrong register allocation or schedule. These are
   flag-lattice/`mips_to_c` tweaks, not rewrites. Assign these individually;
   a single worker can plausibly clear several per session.
2. **`other` with `differing_words <= 10` next.** These are structurally
   close (same size, no mechanical explanation available) but small enough
   that a worker can read the whole diff by hand quickly. Batch by overlay
   where more than one queued function shares a TU, since the worker
   already has that file's context loaded.
3. **`reloc-mismatch`.** Usually a symbol/addend binding
   question (wrong `%hi`/`%lo` target, or a static vs. extern reference) --
   narrow enough to hand off as its own small batch, distinct skill from
   the register/schedule fixes.
4. **`size-mismatch` last, sorted by `differing_words` ascending within
   the category.** The best `size-mismatch` candidates (`differing_words`
   near the low end of that bucket) are worth a look before the worst
   `other` candidates, but the category itself signals a structural
   rewrite (an inlined helper, a duplicated/dropped instruction, a
   different loop shape) rather than a tweak -- expect these to need more
   than one attempt, and route them to workers/models with more budget per
   function (`docs/adr/0009` model-routing).

## Authored campaign notes

The observations below are retained campaign context, not generated ranking
rows. Current queue membership and measurements live only in the marked region
above and its JSON source.

At the time of its recorded investigation, `overlay1FindBestRecord` was a
size-exact register-allocation near miss that the snapshot classified as
`other`: the extracted target
object omits the selected-type HI16/LO16 pair that the shipped runtime table
and candidate both retain. Current configured full-TU V0 is frameless and
18/30 words with 12 `a1`/`a3` pool-register differences from `+0x04`; it
belongs beside the 12-word rows operationally. All 119 flag configurations
were attempted, seven O2/MIPS-II rows tie V0, and none is exact. A
fidelity-clean proc-38 allocator trace plus all three permitted natural
declaration/scope forms leave the same object, so the fallback is parked
pending a new allocator-order mechanism. The earlier exact claim depended on
prohibited post-compile field edits and remains inadmissible.

Nine rows from that historical run were subsequently promoted and ceased to
be search candidates: `overlay3FindClosestObject`, `overlay40AddEntry`,
`overlay43SubmitChildren`, `func_80038750`, `partUpdateTriggers`,
`func_8001A154`, `overlay1UpdateValueCache`,
`func_overlay_041_F0000000_1887338`, and `overlay80UpdateContact`. Their
canonical source and function-specific ledgers carry the exact proofs; stale
generated ranking entries must not put them back into the ready queue.

`func_80021504` is unguarded matched C and no longer a ranking candidate.
Retained configured C owns 133 words with frame `0x28` and 43 candidate
relocation tuples. Its linked range, complete camera TU, and resident `.main`
section are byte-identical to ROM. The retained full `.bin` predates the
object and independent target relocation metadata is absent, so this is a
reproof-only integrity target rather than living search.

`func_80021718` is already canonical C: retained configured C owns 37
words, frame `0x28`, and 14 candidate relocation tuples. Those tuples
reproduce all 37 linked ELF words, and the post-object linked range and
complete camera TU are byte-identical to ROM. No caller is proven;
ROM-table row 453 is an unreferenced export, not inbound evidence. The
full `.bin` predates the object and independent target relocation metadata
is absent, so this remains a reproof-only target rather than living search.

`func_800219D0` is a reproof-only matched function, not a ranking candidate.
The retained pre-comment configured object owns 104 words with frame `0x8`
and eight HI16/LO16 records. Its raw function agrees with the retained
`NON_MATCHING=1` object, and its linked range, complete camera TU, and resident
`.main` section are byte-identical to ROM. The later canonical change added
comments only, so no source/codegen search is warranted. The exact retained
whole `.bin` predates the object and is historical rather than causal proof;
fresh current-source object, link, and full-bin comparisons remain due.

`func_800320F0` (the function formerly routed under the JFG donor alias
`runlinkEnsureJumpIsValid`) has since been promoted: retained canonical C is
101 words with 21 relocations, and its linked ROM range
`0x32CF0..0x32E84` is byte-identical. It is no longer an unmatched ranking
candidate.
