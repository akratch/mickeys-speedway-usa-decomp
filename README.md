# Mickey's Speedway USA

This repo contains a decompilation of *Mickey's Speedway USA* for the N64. You
need your own legally dumped copy of the ROM; nothing ROM-derived is committed
here.

The US ROM (SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548`) is the matching
target and rebuilds byte-identically today. PAL and JPN splat configs exist
from the original stub but have not been modernized or verified.

<!-- SCOREBOARD_BEGIN -->
### Progress

[![functions](https://img.shields.io/badge/functions_matched-167_of_1461_(11.43%25)-blue)](#progress) [![bytes](https://img.shields.io/badge/code_bytes_resolved-325260_of_950256_(34.23%25)-blue)](#progress) [![names](https://img.shields.io/badge/symbols_named-487_adopted-blue)](#progress)

```
functions      167 / 1461    11.43%   matched to C, byte-identical
.text bytes  31604 / 480992   6.57%   matched C in the resident segment
verified asm  17104 / 480992   3.56%   original hand-written assembly (83 functions)
overlay C   276552 / 469264  58.93%   matched C keyed by overlay and offset
whole resolved 325260 / 950256  34.23%   resident C + verified asm + overlay C
named          408 / 1461    27.93%   functions carrying an adopted name
symbols        487                    adopted in symbol_addrs.us.txt
```

DKR-style report (docs/acceleration-survey.md sec.13.1: NON_MATCHING and NON_EQUIVALENT count as unmatched, exactly like extracted assembly):

```
decompiled              308156 / 950256  (32.43%)
handwritten asm          17104 / 950256  ( 1.80%)
GLOBAL_ASM remaining    624948 / 950256  (65.77%)
NON_MATCHING                48 / 950256  ( 0.01%)
NON_EQUIVALENT               0 / 950256  ( 0.00%)
```

| Area | Functions | Matched to C | Named, still asm | Unnamed | Identified |
| :--- | ---: | ---: | ---: | ---: | :--- |
| libultra corridor | 299 | 160 | 111 | 28 | `███████████▓▓▓▓▓▓▓░░` 90.6% |
| game code, TU identified | 114 | 7 | 73 | 34 | `█▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░` 70.2% |
| game code, not yet split | 1048 | 0 | 57 | 991 | `▓░░░░░░░░░░░░░░░░░░░` 5.4% |
| **total** | 1461 | 167 | 241 | 1053 | `██▓▓▓▓░░░░░░░░░░░░░░` 27.9% |

`█` matched to C · `▓` named but still assembly · `░` neither. Naming runs ahead of matching: a function is decompiled against an already-identified translation unit.

**Source organization**: 855 fully-C translation units and 12 C scaffolds that still include assembly. 118 under `src/libultra/`; 2 under `src/main/`, including `matrix.c` (matrix/vector maths) and `runlink.c` (the runtime overlay linker core); 75 under `src/overlays/o001/`; 14 under `src/overlays/o002/`; 9 under `src/overlays/o003/`; 8 under `src/overlays/o004/`; 9 under `src/overlays/o005/`; 1 under `src/overlays/o006/`; 9 under `src/overlays/o007/`; 17 under `src/overlays/o008/`; 8 under `src/overlays/o009/`; 1 under `src/overlays/o010/`; 25 under `src/overlays/o011/`; 4 under `src/overlays/o012/`; 8 under `src/overlays/o013/`; 28 under `src/overlays/o014/`; 12 under `src/overlays/o015/`; 4 under `src/overlays/o016/`; 5 under `src/overlays/o017/`; 4 under `src/overlays/o018/`; 7 under `src/overlays/o019/`; 14 under `src/overlays/o020/`; 2 under `src/overlays/o021/`; 3 under `src/overlays/o022/`; 4 under `src/overlays/o023/`; 3 under `src/overlays/o024/`; 3 under `src/overlays/o025/`; 3 under `src/overlays/o026/`; 6 under `src/overlays/o027/`; 5 under `src/overlays/o028/`; 10 under `src/overlays/o029/`; 2 under `src/overlays/o030/`; 7 under `src/overlays/o031/`; 7 under `src/overlays/o033/`; 8 under `src/overlays/o034/`; 3 under `src/overlays/o035/`; 21 under `src/overlays/o036/`; 5 under `src/overlays/o037/`; 3 under `src/overlays/o038/`; 2 under `src/overlays/o039/`; 9 under `src/overlays/o040/`; 14 under `src/overlays/o041/`; 5 under `src/overlays/o042/`; 7 under `src/overlays/o043/`; 4 under `src/overlays/o044/`; 9 under `src/overlays/o045/`; 9 under `src/overlays/o046/`; 2 under `src/overlays/o047/`; 4 under `src/overlays/o048/`; 3 under `src/overlays/o049/`; 4 under `src/overlays/o050/`; 3 under `src/overlays/o051/`; 4 under `src/overlays/o052/`; 4 under `src/overlays/o053/`; 5 under `src/overlays/o054/`; 5 under `src/overlays/o055/`; 6 under `src/overlays/o056/`; 18 under `src/overlays/o057/`; 8 under `src/overlays/o058/`; 10 under `src/overlays/o059/`; 6 under `src/overlays/o060/`; 12 under `src/overlays/o061/`; 3 under `src/overlays/o062/`; 4 under `src/overlays/o063/`; 1 under `src/overlays/o064/`; 5 under `src/overlays/o065/`; 3 under `src/overlays/o066/`; 1 under `src/overlays/o067/`; 18 under `src/overlays/o068/`; 3 under `src/overlays/o069/`; 3 under `src/overlays/o070/`; 4 under `src/overlays/o071/`; 2 under `src/overlays/o072/`; 2 under `src/overlays/o073/`; 2 under `src/overlays/o074/`; 3 under `src/overlays/o075/`; 1 under `src/overlays/o076/`; 3 under `src/overlays/o077/`; 1 under `src/overlays/o078/`; 6 under `src/overlays/o079/`; 2 under `src/overlays/o080/`; 4 under `src/overlays/o081/`; 3 under `src/overlays/o082/`; 9 under `src/overlays/o083/`; 21 under `src/overlays/o084/`; 2 under `src/overlays/o085/`; 6 under `src/overlays/o086/`; 3 under `src/overlays/o087/`; 3 under `src/overlays/o088/`; 5 under `src/overlays/o089/`; 1 under `src/overlays/o090/`; 3 under `src/overlays/o091/`; 2 under `src/overlays/o092/`; 1 under `src/overlays/o093/`; 3 under `src/overlays/o094/`; 2 under `src/overlays/o095/`; 6 under `src/overlays/o096/`; 11 under `src/overlays/o097/`; 4 under `src/overlays/o098/`; 8 under `src/overlays/o099/`; 7 under `src/overlays/o100/`; 60 under `src/overlays/o101/`; 1 under `src/overlays/o102/`; 1 under `src/overlays/o103/`; 1 under `src/overlays/o104/`; 1 under `src/overlays/o105/`; 1 under `src/overlays/o106/`; 1 under `src/overlays/o107/`.

Generated by `gmake scoreboard` from the built ELF, the splat config, the `asm/` tree and `symbol_addrs.us.txt`; `gmake check-scoreboard` fails if it has drifted. [`docs/modules.md`](docs/modules.md) records what each run of code was identified as and on what evidence; [`docs/references.md`](docs/references.md) records the reference builds it was measured against.
<!-- SCOREBOARD_END -->

## Setup

### Dependencies (macOS)

```sh
brew install make python3
```

Use `gmake`, Homebrew's GNU make: macOS's built-in `make` is too old.

`splat` (the disassembler/splitter) comes from `requirements.txt` via pip; no
submodule is needed for the build itself.

`tools/setup_toolchain.sh` copies the MIPS binutils + IDO 5.3 toolchain from a
local Diddy Kong Racing decomp checkout (`DKR_PATH`, default
`~/Desktop/dev/Diddy-Kong-Racing`). Without one: `brew install
mips64-elf-binutils`, and take IDO 5.3 from the macOS build on
[decompals/ido-static-recomp](https://github.com/decompals/ido-static-recomp)'s
Releases page. The script documents both routes.

### Baserom

Place your own legally dumped ROM at `baseroms/mickey.us.z64`. Expected SHA1s
for all three regions are in `mickey.*.sha1` and `tools/verify_baseroms.sh`.

### Build

```sh
gmake setup   # venv, deps, toolchain, baserom SHA1 check, splat, git hooks
gmake -j2     # builds build/mickey.us.z64
gmake verify  # byte-compares it against the baserom SHA1
```

### Checks

| Command | Checks |
|---|---|
| `gmake verify` | the ROM rebuilds byte-identically |
| `gmake cleanroom` | no ROM-derived content (also run by the git hooks and CI) |
| `gmake check-docs` | derived numbers in the docs match the tree |
| `gmake check-scoreboard` | the Progress block matches the tree |
| `gmake audit-decoders` | the clean-room detectors aren't inventing words |
| `gmake check-fixtures` | the detectors still catch real ROM in every encoding |
| `gmake overlay-atlas` | the 107-module manifest and generated yaml segments have not drifted |
| `gmake overlay-donors` | DKR v77/v80 and JFG have a complete recorded result for every overlay |
| `gmake check-reference-builds` | the out-of-tree reference builds are the ones the names were mined from |
| `gmake progress` | matched functions/bytes/symbols |

[`docs/CONTRIBUTING.md`](docs/CONTRIBUTING.md#checks) has the full table,
including which are enforced and which need a build. `check-scoreboard` needs a
linked ELF, which needs a baserom, so CI runs
[`scoreboard.yml`](.github/workflows/scoreboard.yml)'s `--check-partial`
instead: the non-ELF-derived figures and the block's own arithmetic.

## Decompiling

`tools/asm-processor` is required to build; it is what makes
`#pragma GLOBAL_ASM(...)` work with IDO. `tools/asm-differ` (`diff.sh`) and
`tools/m2c` (`mips_to_c.sh`, `generate_ctx.sh`) are the matching workflow and
are not needed for `gmake verify`. `tools/ido-static-recomp` is left
uninitialised; `tools/setup_toolchain.sh` installs a prebuilt IDO 5.3.

```sh
git submodule update --init tools/asm-processor tools/asm-differ tools/m2c
```

`gmake setup` runs that for you.

To add a translation unit: add it to `mickey.*.yaml` as a `c` subsegment,
re-split (`gmake` does it automatically), and fill in `src/`. Every function
starts as `#pragma GLOBAL_ASM("asm/nonmatchings/<dir>/<func>.s")`, which
already builds byte-identically, and is replaced by C one function at a time.

```sh
./diff.sh <symbol>          # target vs. current disassembly; matched == no diffs
./diff.sh -mw <symbol>      # rebuild first, then re-diff on every file save
./generate_ctx.sh           # optional: ctx.c, so m2c knows project types
./mips_to_c.sh <symbol>     # m2c first draft into m2cfiles/<symbol>.c
gmake verify                # byte-identical or it does not count
```

For overlay work, begin with `config/overlays.us.json` rather than a synthetic
VMA: an overlay function's canonical identity is `(overlay, section, offset)`.
Before adopting a name or body, run `gmake overlay-donors-scan-check`; it checks
the complete DKR v77/v80 and JFG object surfaces and catches a stale donor
ledger. DKR is the first semantic cross-reference for game code, but a similar
system is not an exact match and Mickey's own bytes and call graph decide.

C compiles with `-O2 -mips1 -32` by default. Anything else is a per-file
override in the Makefile's "Per-file compiler flags" block, justified in the
source file's header comment. Measured per-file flags are in
[`docs/modules.md`](docs/modules.md) §6.1.

## Roadmap

The measurable epoch definitions and exit criteria live in
[`docs/campaigns.md`](docs/campaigns.md).

| Phase | Scope | State |
|---|---|---|
| 0 | Split the US ROM with splat; rebuild it byte-identically from disassembly | done |
| 1 | First matched C, libultra corridor, symbol/struct ontology | in progress |
| 2 | Clean-room gates; mine the published Rare decomps for matching objects | in progress |
| 3 | Overlay system: 107-module atlas, relocation graph, 106 buildable segments, DKR/JFG donor ledger | done |
| 4 | Overlay frontier tranche A: exact-match seeds, four structural pilots, one dependency neighborhood | done |
| 5 | Overlay semantic spine: 45/61/68 APIs, cohort closure, and hub API maps | done |
| 6 | Exact-leaf recovery: close narrow compiler blockers and add 1 KiB matched overlay C | done |
| 7 | Exact leaf and wrapper retirement across ten overlays (508 bytes) | done |
| 8 | Overlay 84 accessor closure plus resource wrappers (436 bytes) | done |
| 9 | Overlay 68 lifecycle/allocation semantic cluster (524 bytes) | done |
| 10 | Double-digit breakthrough: reach 10.00% whole-program resolved and close four Epoch 5 cohort modules | done |
| 11 | Fifteen-percent offensive: reach 15.00% whole-program resolved and close eight more overlays | active |
| 12 | Assets: `1172`/`1173` decompress/recompress with matching output | not started |

## Clean room

- No ROMs or extracted assets are committed.
- Names and adapted function bodies may come from the five named published
  retail-derived decomps, with a `PROVENANCE` note at the point of use.
  Leaked material is forbidden outright.
- [`docs/CLEANROOM.md`](docs/CLEANROOM.md) is the policy, including what the
  automated gates do and do not catch.
- [`docs/CONTRIBUTING.md`](docs/CONTRIBUTING.md) covers the commit/push gates
  that enforce it; `gmake setup` activates them.

## Contributing

Read [`docs/CONTRIBUTING.md`](docs/CONTRIBUTING.md) first, then
[`docs/CLEANROOM.md`](docs/CLEANROOM.md).
[`docs/modules.md`](docs/modules.md) is the module map and the evidence-tier
convention every name follows; [`docs/references.md`](docs/references.md)
records the reference builds.

## Credits

- [Ryan-Myers](https://github.com/Ryan-Myers/Mickeys-Speedway-USA), the
  original repository this project builds on, and the
  [Jet Force Gemini](https://github.com/Ryan-Myers/Jet-Force-Gemini) decomp
  that most of the tier-A names here come from.
- The [Diddy Kong Racing](https://github.com/DavidSM64/Diddy-Kong-Racing),
  [Perfect Dark](https://github.com/n64decomp/perfect_dark),
  [Banjo-Kazooie](https://github.com/n64decomp/banjo-kazooie) and
  [Conker's Bad Fur Day](https://github.com/mkst/conker) decomps.
- [splat](https://github.com/ethteck/splat),
  [asm-differ](https://github.com/simonlindholm/asm-differ),
  [asm-processor](https://github.com/simonlindholm/asm-processor),
  [m2c](https://github.com/matt-kempster/m2c) and
  [ido-static-recomp](https://github.com/decompals/ido-static-recomp).

## License

[CC0 1.0 Universal](LICENSE), covering this repository's own work only, not
Rare's original code or any asset in the ROM.
