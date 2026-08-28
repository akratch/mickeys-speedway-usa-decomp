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

[![functions](https://img.shields.io/badge/functions_matched-1386_of_1846_(75.08%25)-blue)](#progress) [![bytes](https://img.shields.io/badge/C_bytes_matched-332648_of_947796_(35.10%25)-blue)](#progress) [![names](https://img.shields.io/badge/symbols_named-1272_adopted-blue)](#progress)

| Scope | Complete | Total | Progress |
|---|---:|---:|---:|
| Resident functions | 1386 | 1846 | 75.08% |
| Resident C bytes | 222796 | 478532 | 46.56% |
| Overlay C bytes | 109852 | 469264 | 23.41% |
| Whole-game C bytes | 332648 | 947796 | 35.10% |
| Whole-game resolved bytes | 349260 | 947796 | 36.85% |
| Named resident functions | 1510 | 1846 | 81.80% |

Resolved bytes include matched C and 16612 bytes in 82 verified hand-written assembly functions. The symbol file contains 1272 adopted names.

#### Resident progress by area

| Area | Matched functions | Total functions | Function progress | Matched C bytes | Text bytes | Byte progress |
|---|---:|---:|---:|---:|---:|---:|
| libultra | 269 | 306 | 87.91% | 79800 | 92204 | 86.55% |
| game code in named translation units | 851 | 1090 | 78.07% | 142996 | 315292 | 45.35% |
| game code not split into translation units | 266 | 450 | 59.11% | 0 | 71036 | 0.00% |
| **total** | 1386 | 1846 | 75.08% | 222796 | 478532 | 46.56% |

Resident function counts exclude overlays because complete overlay function boundaries are not yet available. Overlay progress is therefore reported by byte count in the summary table.

#### Whole-game code status

| Status | Bytes | Share of text |
|---|---:|---:|
| Matched C | 332648 | 35.10% |
| Verified hand-written assembly | 16612 | 1.75% |
| Extracted assembly | 284524 | 30.02% |
| C under `NON_MATCHING` | 314012 | 33.13% |
| C under `NON_EQUIVALENT` | 0 | 0.00% |

Source files: 808 translation units; 539 contain only C and 269 still include assembly. Directory totals: `src/libultra` 134, `src/main` 38, and `src/overlays` 636.

Run `gmake scoreboard` after a matching change. `gmake check-scoreboard` checks this section against the current build.
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
