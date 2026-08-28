# Mickey's Speedway USA

This project decompiles the US release of *Mickey's Speedway USA* for Nintendo
64. The current source rebuilds the target ROM byte for byte.

You must provide your own legally obtained ROM. This repository does not
contain ROM data, extracted assets, or compiler binaries.

Target ROM SHA-1: `507341c0a40ca3e9a7cee969b396ee53facfb548`

The PAL and Japanese configuration files come from the original repository.
They have not been updated or verified.

<!-- SCOREBOARD_BEGIN -->
### Progress

[![functions](https://img.shields.io/badge/functions_matched-1022_of_1460_(70.00%25)-blue)](#progress) [![bytes](https://img.shields.io/badge/code_bytes_resolved-357016_of_944368_(37.80%25)-blue)](#progress) [![names](https://img.shields.io/badge/symbols_named-1277_adopted-blue)](#progress)

```
functions     1022 / 1460    70.00%   matched to C, byte-identical
.text bytes 228320 / 475104  48.06%   matched C in the resident segment
verified asm  17184 / 475104   3.62%   original hand-written assembly (84 functions)
overlay C   111512 / 469264  23.76%   matched C keyed by overlay and offset
whole resolved 357016 / 944368  37.80%   resident C + verified asm + overlay C
named         1134 / 1460    77.67%   functions carrying an adopted name
symbols       1277                    adopted in symbol_addrs.us.txt
```

DKR-style report (docs/acceleration-survey.md sec.13.1: NON_MATCHING and NON_EQUIVALENT count as unmatched, exactly like extracted assembly):

```
decompiled              339832 / 944368  (35.99%)
handwritten asm          17184 / 944368  ( 1.82%)
GLOBAL_ASM remaining    275000 / 944368  (29.12%)
NON_MATCHING            312352 / 944368  (33.08%)
NON_EQUIVALENT               0 / 944368  ( 0.00%)
```

| Area | Functions | Matched to C | Named, still asm | Unnamed | Identified |
| :--- | ---: | ---: | ---: | ---: | :--- |
| libultra corridor | 288 | 256 | 32 | 0 | `██████████████████▓▓` 100.0% |
| game code, TU identified | 1003 | 766 | 77 | 160 | `███████████████▓▓░░░` 84.0% |
| game code, not yet split | 169 | 0 | 3 | 166 | `▓░░░░░░░░░░░░░░░░░░░` 1.8% |
| **total** | 1460 | 1022 | 112 | 326 | `██████████████▓▓░░░░` 77.7% |

`█` matched to C · `▓` named but still assembly · `░` neither. Naming runs ahead of matching: a function is decompiled against an already-identified translation unit.

**Source organization**: 558 fully-C translation units and 266 C scaffolds that still include assembly. 139 under `src/libultra/`; 49 under `src/main/`, including `matrix.c` (matrix/vector maths) and `runlink.c` (the runtime overlay linker core); 8 under `src/overlays/o001/`; 17 under `src/overlays/o002/`; 9 under `src/overlays/o003/`; 1 under `src/overlays/o004/`; 7 under `src/overlays/o005/`; 1 under `src/overlays/o006/`; 3 under `src/overlays/o007/`; 1 under `src/overlays/o008/`; 1 under `src/overlays/o009/`; 1 under `src/overlays/o010/`; 26 under `src/overlays/o011/`; 6 under `src/overlays/o012/`; 8 under `src/overlays/o013/`; 28 under `src/overlays/o014/`; 1 under `src/overlays/o015/`; 1 under `src/overlays/o016/`; 5 under `src/overlays/o017/`; 4 under `src/overlays/o018/`; 7 under `src/overlays/o019/`; 16 under `src/overlays/o020/`; 2 under `src/overlays/o021/`; 4 under `src/overlays/o022/`; 4 under `src/overlays/o023/`; 1 under `src/overlays/o024/`; 1 under `src/overlays/o025/`; 5 under `src/overlays/o026/`; 1 under `src/overlays/o027/`; 1 under `src/overlays/o028/`; 11 under `src/overlays/o029/`; 2 under `src/overlays/o030/`; 7 under `src/overlays/o031/`; 7 under `src/overlays/o033/`; 8 under `src/overlays/o034/`; 5 under `src/overlays/o035/`; 21 under `src/overlays/o036/`; 5 under `src/overlays/o037/`; 3 under `src/overlays/o038/`; 1 under `src/overlays/o039/`; 9 under `src/overlays/o040/`; 14 under `src/overlays/o041/`; 1 under `src/overlays/o042/`; 9 under `src/overlays/o043/`; 5 under `src/overlays/o044/`; 4 under `src/overlays/o045/`; 11 under `src/overlays/o046/`; 4 under `src/overlays/o047/`; 4 under `src/overlays/o048/`; 1 under `src/overlays/o049/`; 5 under `src/overlays/o050/`; 1 under `src/overlays/o051/`; 4 under `src/overlays/o052/`; 5 under `src/overlays/o053/`; 5 under `src/overlays/o054/`; 6 under `src/overlays/o055/`; 1 under `src/overlays/o056/`; 22 under `src/overlays/o057/`; 11 under `src/overlays/o058/`; 10 under `src/overlays/o059/`; 6 under `src/overlays/o060/`; 12 under `src/overlays/o061/`; 3 under `src/overlays/o062/`; 4 under `src/overlays/o063/`; 1 under `src/overlays/o064/`; 6 under `src/overlays/o065/`; 4 under `src/overlays/o066/`; 1 under `src/overlays/o067/`; 18 under `src/overlays/o068/`; 3 under `src/overlays/o069/`; 3 under `src/overlays/o070/`; 4 under `src/overlays/o071/`; 1 under `src/overlays/o072/`; 2 under `src/overlays/o073/`; 2 under `src/overlays/o074/`; 3 under `src/overlays/o075/`; 1 under `src/overlays/o076/`; 2 under `src/overlays/o077/`; 1 under `src/overlays/o078/`; 8 under `src/overlays/o079/`; 2 under `src/overlays/o080/`; 1 under `src/overlays/o081/`; 2 under `src/overlays/o082/`; 9 under `src/overlays/o083/`; 22 under `src/overlays/o084/`; 1 under `src/overlays/o085/`; 6 under `src/overlays/o086/`; 4 under `src/overlays/o087/`; 3 under `src/overlays/o088/`; 5 under `src/overlays/o089/`; 1 under `src/overlays/o090/`; 2 under `src/overlays/o091/`; 3 under `src/overlays/o092/`; 1 under `src/overlays/o093/`; 3 under `src/overlays/o094/`; 1 under `src/overlays/o095/`; 6 under `src/overlays/o096/`; 11 under `src/overlays/o097/`; 4 under `src/overlays/o098/`; 8 under `src/overlays/o099/`; 7 under `src/overlays/o100/`; 71 under `src/overlays/o101/`; 1 under `src/overlays/o102/`; 1 under `src/overlays/o103/`; 1 under `src/overlays/o104/`; 1 under `src/overlays/o105/`; 1 under `src/overlays/o106/`; 1 under `src/overlays/o107/`.

Generated by `gmake scoreboard` from the built ELF, the splat config, the `asm/` tree and `symbol_addrs.us.txt`; `gmake check-scoreboard` fails if it has drifted. [`docs/modules.md`](docs/modules.md) records what each run of code was identified as and on what evidence; [`docs/references.md`](docs/references.md) records the reference builds it was measured against.
<!-- SCOREBOARD_END -->

## Requirements

On macOS:

```sh
brew install make python3 mips64-elf-binutils
```

Use GNU Make as `gmake`. The build also needs IDO 5.3. If a local
[Diddy Kong Racing](https://github.com/DavidSM64/Diddy-Kong-Racing) checkout is
available, `tools/setup_toolchain.sh` copies its compatible binutils and IDO
files. Otherwise, install the macOS build from
[decompals/ido-static-recomp](https://github.com/decompals/ido-static-recomp/releases)
under `tools/ido/` and place the prefixed binutils under `tools/binutils/`.

## Build

1. Place the US ROM at `baseroms/mickey.us.z64`.
2. Run the setup target once.
3. Build and verify the ROM.

```sh
gmake setup
gmake -j$(sysctl -n hw.ncpu)
gmake verify
```

`gmake setup` checks the ROM hash, creates the Python environment, initializes
the required submodules, splits the ROM, and enables the repository hooks.

## Project checks

| Command | Purpose |
|---|---|
| `gmake verify` | Rebuild the US ROM and compare it with the target hash |
| `gmake cleanroom` | Reject tracked ROM data and prohibited file types |
| `gmake check-docs` | Recompute numeric claims in the documentation |
| `gmake check-scoreboard` | Compare this progress section with the current build |
| `gmake overlay-atlas` | Check the overlay map and generated YAML segments |
| `gmake overlay-donors` | Check the recorded overlay reference results |
| `gmake check-reference-builds` | Check local reference objects against the lock file |

Some checks need a local ROM or reference checkout and therefore do not run in
public CI. See [Contributing](docs/CONTRIBUTING.md#checks) for the full list.

## Decompilation workflow

Functions that have not matched remain as `#pragma GLOBAL_ASM` includes. Replace
one function at a time, build its translation unit, compare the object, and run
`gmake verify` before recording it as matched.

Useful commands:

```sh
./diff.sh <symbol>          # compare target and current assembly
./diff.sh -mw <symbol>      # rebuild and watch the comparison
./generate_ctx.sh           # generate ctx.c for m2c
./mips_to_c.sh <symbol>     # create an initial C translation
gmake verify                # verify the complete ROM
```

Overlay addresses are reused at runtime. Identify overlay code by overlay,
section, and offset, using `config/overlays.us.json`; a virtual address alone
is not sufficient.

The default compiler flags are `-O2 -mips1 -32`. Per-file differences are
listed in the Makefile and summarized in [The module map](docs/modules.md).

## Documentation

- [Contributing](docs/CONTRIBUTING.md) describes the development and review
  process.
- [Clean-room policy](docs/CLEANROOM.md) lists permitted sources and prohibited
  tracked content.
- [Module map](docs/modules.md) defines the address map and evidence levels.
- [Resident code](docs/resident.md) and [overlays](docs/overlays.md) describe
  the two code regions.
- [Reference builds](docs/references.md) records the external builds used for
  symbol comparison.
- [JFG code mining guide](docs/jfg-code-mining.md) points Jet Force Gemini
  contributors to matching shared-engine C in this repository.
- [Architecture decisions](docs/adr/README.md) records project policy.

## Contributing

Read [Contributing](docs/CONTRIBUTING.md) and the
[clean-room policy](docs/CLEANROOM.md) before changing source. Do not bypass
the git hooks. A function counts as matched only when the unmodified compiler
output links to the correct location and matches every owned byte.

## Credits

- [Ryan Myers](https://github.com/Ryan-Myers/Mickeys-Speedway-USA), who created
  the original repository, and the
  [Jet Force Gemini](https://github.com/Ryan-Myers/Jet-Force-Gemini)
  decompilation used as a documented reference.
- The
  [Diddy Kong Racing](https://github.com/DavidSM64/Diddy-Kong-Racing),
  [Perfect Dark](https://github.com/n64decomp/perfect_dark),
  [Banjo-Kazooie](https://github.com/n64decomp/banjo-kazooie), and
  [Conker's Bad Fur Day](https://github.com/mkst/conker) projects.
- [splat](https://github.com/ethteck/splat),
  [asm-differ](https://github.com/simonlindholm/asm-differ),
  [asm-processor](https://github.com/simonlindholm/asm-processor),
  [m2c](https://github.com/matt-kempster/m2c), and
  [ido-static-recomp](https://github.com/decompals/ido-static-recomp).

## License

The original work in this repository is released under
[CC0 1.0 Universal](LICENSE). This license does not cover Rare's original code
or any data in the ROM.
