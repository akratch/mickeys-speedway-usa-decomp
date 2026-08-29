# `tools/flag_sweep.py`: sweep the flag lattice before hand permutation

Several compiler-flag families occur in this tree: `-Wab,-r4300_mul`, `-O1`,
phase-specific optimizer overrides, `-O2 -g3`, and overlay 5's
`-O3 -mips2`. This tool measures those known families systematically before
any hand permutation.

Run it against a translation unit and a function name; it compiles that TU
under every flag combination this project has ever needed (plus a few
adjacent ones), scores each candidate against the real target bytes, and
prints a ranked table plus the winning flags in Makefile-paste form. The
contributor must then *confirm* the winner with `tools/wb_compare.sh` (and
land it under `tools/wb_compare.sh --rom` once it's actually matched), not to
grope through `-O1` vs `-O2` vs `-mips2` combinations one at a time.

```sh
.venv/bin/python tools/flag_sweep.py src/libultra/sinf.c --function __sinf
```

## What it sweeps

The lattice lives as a small set of tables at the top of the script (`OPT_GROUP`,
`ISA_GROUP`, `EXTRAS_GROUP`, `PHASE_VARIANTS`) so a new flag family is one row,
not a new code path:

| axis | values |
|---|---|
| `OPT_GROUP` | bare (no `-O`), `-O0`, `-O1`, `-O2`, `-O3`, `-g`, `-g3`, `-O2 -g3` |
| `ISA_GROUP` | `-mips1 -32`, `-mips2 -32`, `-mips3 -32` |
| `EXTRAS_GROUP` | none, `-Wab,-r4300_mul`, `-Wo,-loopunroll,{0,2,4}`, `-woff 835` |

The cross product is pruned once: a loop-unroll hint under an `OPT_GROUP`
entry with no `-On` token at all (bare/`-g`/`-g3`) is dropped, since nothing
in the tree pairs the two and there is no optimizing pass for the hint to
act on. That leaves 117 combos.

`PHASE_VARIANTS` adds two more, driven through `tools/ido-phases.py` instead
of `tools/ido/cc` directly: `uopt` forced to run at `-O1` while everything
else stays `-O2` (`setglobalintmask.c`'s flag group), and all four IDO phases
at `-O3` (`ll.c`/`ldiv.c`'s). These are not crossed against `ISA_GROUP` /
`EXTRAS_GROUP` — the project has exactly two documented uses of this family
and both pin their own ISA — so add a row to `PHASE_VARIANTS` for a third one
rather than folding it into the cross product.

119 combos total. Each compiles through `asm-processor` exactly like the
Makefile's `%.c.o` rule (`Makefile` ~415-424): same `CFLAGS`/`ASFLAGS`, same
`asm_processor_prelude.inc`, same two-stage `asm-processor build.py <cc> --
<as> -- <compile args>` shape. `BASE_CFLAGS`/`ASFLAGS` are copied by hand at
the top of the script (there is no way to `include` the Makefile's variables
into a driver script that calls IDO's phases directly) — if the Makefile's
own values move, this file needs the matching edit.

Compiles run in parallel (`ncpu - 2` workers by default, `--jobs` to
override) into a content-addressed cache under
`build/flag_sweep/cache/<compile-key>/<combo-id>/`, which is gitignored by
the tree's blanket `build/` rule. Objects, logs, and failed-row records are
retained by default; `--keep` remains only as a no-op compatibility option.

Relative translation-unit, `--target-asm`, and `--elf` paths are resolved
against the repository root, independent of the caller's current directory.

## Target resolution

Pass `--function NAME` (the symbol as it appears in the *compiled candidate*
object) and, if the ROM/asm side uses a different name, `--target-symbol
NAME` for that side. The two differ for exactly one reason: splat names an
unmatched function `func_<VRAM>` until it's matched, so a `NON_MATCHING`
draft can be named for real (`ProcessRelocationEntry`) while its ROM target
is still `func_80031A30`.

Before reading bytes, the tool resolves one canonical owner. An overlay
target must map to exactly one `text_ownership` row in
`config/overlays.us.json`; an encoded `func_overlay_NNN_Fxxxxxxx_*` offset,
the overlay number, and the input TU's canonical source path must agree with
that row. A resident target must have exactly one sized record in
`symbol_addrs.us.txt`. Missing, overlapping, or contradictory ownership is a
hard error, not a reason to guess from a candidate object's size.

Target bytes are then resolved in this order (first that resolves wins; the
CLI never asks the caller to pick a mode):

1. `--target-asm PATH` — assemble this `.s` file directly.
2. `asm/nonmatchings/**/<target-symbol>.s` — the same file
   `tools/wb_compare.sh` uses, found the same way (`find asm/nonmatchings
   -name '<target-symbol>.s'`), for a function that is still `#pragma
   GLOBAL_ASM` or sits under `#ifdef NON_MATCHING`. Assembled with the
   project's `AS`/`ASFLAGS` plus the same `.set noat` / `macro.inc` header
   `wb_compare.sh` prepends. The tool extracts the named ELF function symbol,
   not the assembled file's whole `.text` section, and verifies that symbol
   lies inside its atlas owner. Zero alignment or padding words after
   `endlabel` therefore cannot inflate the target extent.
3. `<target-symbol>` in `build/mickey.us.elf` — the function is already
   matched, so the linked ELF's bytes for it already *are* the ROM's bytes.
   Read straight from `baseroms/mickey.us.z64` at the symbol's ROM offset.

Mode 3's ROM offset is **not** the resident segment's fixed `vram -
0x7FFFF400`. It is derived per section from `objdump -h`'s VMA and LMA:
`rom_offset = LMA + (symbol_vram - section_VMA)`. For resident code VMA and
LMA agree up to exactly that fixed offset, so the two formulas coincide —
but every overlay function's ELF address is a splat *synthetic* VMA
(`0xF0000000`-based, shared across all 107 overlays, since they all load at
the same runtime address) while its LMA is that overlay's real, distinct ROM
location. Deriving the offset from LMA-VMA per symbol's own section, rather
than assuming the resident constant, is what makes mode 3 work for overlay
functions at all; the three worked examples below include one.

## Scoring: masked word comparison, not "are the bytes equal"

A candidate is an *unlinked* object. A word carrying a relocation (`jal`,
a `%hi`/`%lo` pair, ...) holds a placeholder or an addend, not a resolved
address, so comparing it literally against linked ROM bytes (mode 3) or
against an also-unlinked assembled target (mode 2) is meaningless at that
word. Both are handled the same way: gather every relocation `objdump -r`
reports for the candidate's own object (mode 2 also unions in the target
object's own relocations) and mask each flagged word to the bits a linker
would still have to fill in before comparing — 26 bits for `R_MIPS_26`
(`jal`/`j`), 16 for `R_MIPS_HI16`/`R_MIPS_LO16`/`R_MIPS_GPREL16`/
`R_MIPS_GOT16`/`R_MIPS_CALL16`, all 32 for `R_MIPS_32`, 16 as the fallback
for anything else. In mode 3 the linked ROM has no relocation table of its
own to consult, so only the candidate's relocations are available — the
mask is best-effort there, not authoritative, which is exactly why an
"exact" verdict from this tool is a lead for `tools/wb_compare.sh` to
confirm, not a substitute for it.

`score_words()` (`tools/flag_sweep.py`), the function `tests/test_flag_sweep.py`
exercises directly with synthetic word arrays, turns two masked word arrays
into:

- **exact** — same length, zero masked differences.
- **size_delta** — candidate bytes minus target bytes.
- **diff_words** — masked-mismatching words over the shared length, plus one
  per extra/missing word beyond it (so a pure length regression still shows
  up here even when every shared word agrees).
- **first_mismatch** — byte offset of the first differing word, or `None`
  when there isn't one.

Ranking sorts `exact` first, then fewer `diff_words`, then smaller
`|size_delta|`, then a later `first_mismatch` (agreeing for longer before
diverging beats diverging immediately, as a last tiebreak).

`--objdiff` additionally shells out to `tools/objdiff/objdiff-cli` for the
top row's match percentage. This is entirely best-effort: the sweep does not
install or require `objdiff`, and the flag is silently a no-op if the binary
is absent.

## Reusing and rescoring the compile cache

The compile key covers the TU and every recursively resolved literal include,
the defines and complete flag lattice, Python's major/minor version, and the
compiler, assembler, IDO phase driver, and asm-processor tool files. Computed
or unresolved includes fail closed because their dependencies cannot be bound
soundly. Target assembly, the atlas, linked ELF, and baserom are deliberately
not compile inputs: they change scoring geometry, not the candidate objects.

A normal invocation reuses every complete cache row and compiles only missing
rows. To guarantee that no compiler process runs, add `--rescore`:

```sh
.venv/bin/python tools/flag_sweep.py \
    src/overlays/o022/overlay22RemoveObject.c \
    --function func_overlay_022_F0000D30_1878E38 \
    --jobs 2 --rescore
```

`--rescore` loads all 119 retained results, resolves current ownership and
target bytes, and recomputes the ranking. It fails if the current compile key
has even one missing row. This makes an atlas/range correction cheap without
allowing stale source, header, flag, or tool inputs to masquerade as a valid
cache hit.

## Nothing ROM-derived is ever written to a tracked file

Compiled objects, logs, cache manifests, assembled `.s` targets, and their
`objcopy`-dumped section bytes all live under `build/flag_sweep/`, which the
tree's `build/` rule already gitignores. The ranked table prints only counts
and byte offsets — never a mnemonic, an opcode, or a raw word — so a terminal
transcript of a run is not itself ROM-derived content under
`docs/CLEANROOM.md`'s rules, the same way `gmake progress`'s output isn't.

## Three worked rankings

All three below reproduce this tree's already-documented answer as the
top-ranked (or, for the third, correctly-not-exact) row — the point of
running the sweep on cases with a known answer first.

### 1. Sanity: a matched libultra TU (`src/libultra/sinf.c`, `__sinf`)

```
.venv/bin/python tools/flag_sweep.py src/libultra/sinf.c --function __sinf
```

Top two rows, tied, both exact: `-O2 -mips2 -32 -Wab,-r4300_mul` and
`-O2 -g3 -mips2 -32 -Wab,-r4300_mul`. That is exactly the flag group the
file's own header comment documents ("The reference SDK build enables the
R4300 multiply hazard pass") and exactly what `-Wab,-r4300_mul`'s use on 69
TUs across the tree already establishes as the project's default multiply-
hazard group. Every other combo in the top 15 is off by 16-91 masked words.
119 combos, ~2-4s wall time on 12 workers.

### 2. Blocked on instruction scheduling: `__osContRamRead`

`src/libultra/contramread.c` is currently `#pragma GLOBAL_ASM` outright (no
C body committed — see its header comment), so there was nothing to point
the sweep at directly. The comment's own diagnosis — "the SDK body
reproduces every word except five, all inside one four-times-unrolled
byte-fill loop" at `-O2 -g3 -mips2 -32` — was reproduced by feeding the
sweep the JFG source (`$JFG_ROOT/libultra/src/io/contramread.c`, permitted per
`docs/CLEANROOM.md`) built against this tree's
own headers (`include/PRinternal/controller.h` already carries the
`addrh`/`addrl`-split struct layout this source expects) from a scratch
path outside the repo:

```
.venv/bin/python tools/flag_sweep.py /path/to/contramread_demo.c \
    --function __osContRamRead
```

Top row: `-O2 -g3 -mips2 -32`, size delta -12 bytes (3 words short of the
ROM's 140), 3 masked-diff words, first mismatch at the tail end (byte
0x224) — every combo in the tree's lattice agrees for the entire function
body up to that point, and none closes the remaining gap, which is the
expected outcome for a scheduling disagreement this tool cannot fix: it
finds the best *flags*, and this file's problem, per its own comment, is not
flags. `docs/CLEANROOM.md`'s adoption rule applies to landing this source in
`src/`, not to using it as a scratch input for a flag ranking that is never
committed; nothing from it appears in this file or in the demo TU's path,
which was not tracked.

### 3. An overlay function needing more than flags: `overlay1FindNextAngle`

```
.venv/bin/python tools/flag_sweep.py \
    src/overlays/o001/overlay_001_middle.c --function overlay1FindNextAngle
```

Top row: `-O2 -mips2 -32` (the overlay directory's project default,
`Makefile` ~615) with a correct size (delta 0) and exactly 4 masked-diff
words, first mismatch at byte 0x3c. That is not a flags gap: the file's own
header comment says two operand pairs are swapped at the shipped object's
natural scheduling points at offsets 0x3c, 0x40, 0x6c and 0x70. Current
`POSTPROCESS` only renames fallback symbols and trims zero alignment; it does
not and must not rewrite instructions. This is the sweep correctly reporting
"flags alone don't reach this one." Eight bounded source families have also
missed, so the current next action is an unchanged, fully annotated reproof,
not normalization or another flag combination.

## Runtime

~1-4 seconds wall time per sweep (119 combos, `ncpu - 2` = 12 workers on the
machine these were measured on) for these three small single/few-function
TUs — almost entirely IDO invocation overhead, not compile time. A larger
TU with many functions in it will cost more per combo since the whole file
is recompiled each time; there is no per-function isolation.
