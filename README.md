# Mickey's Speedway USA

A decompilation project for *Mickey's Speedway USA* (N64), started by
[Ryan-Myers](https://github.com/Ryan-Myers/Mickeys-Speedway-USA) and revived
here to explore the Diddy Kong Racing engine as it evolved for this game.

## Clean Room

- No ROMs or extracted assets are committed to this repository.
- Bring your own legally dumped, SHA1-verified ROM.
- See [`docs/CLEANROOM.md`](docs/CLEANROOM.md) for the full policy.

## Status

The US ROM rebuilds **byte-identically** from the disassembly (SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`). That's currently a 0%-matched,
all-assembly build — Phase 1 (decompiling functions to C, one at a time) is
starting next. PAL and JPN splat configs exist from the original stub but
have not yet been modernized/verified the way US has.

## Building (US ROM)

Prerequisites (macOS):

```sh
brew install make python3
```

(invoke `make` below as `gmake`, Homebrew's GNU make — macOS's built-in
`make` is too old)

Drop your own legally dumped ROM into `baseroms/mickey.us.z64`. Expected
SHA1s for all three regions are in `mickey.*.sha1` and
`tools/verify_baseroms.sh`.

Then, from a fresh clone:

```sh
gmake setup   # bootstraps the venv, installs deps, copies the toolchain
              # (tools/setup_toolchain.sh), verifies the baserom, splits it
gmake -j8     # builds build/mickey.us.z64
gmake verify  # confirms it matches the baserom SHA1 byte-for-byte
```

`splat` (the disassembler/splitter) comes from `requirements.txt` via pip —
no submodule needed for the build itself.

`tools/setup_toolchain.sh` copies the MIPS binutils + IDO 5.3 toolchain from
a local Diddy Kong Racing decomp checkout (`DKR_PATH` env var, defaults to
`~/Desktop/dev/Diddy-Kong-Racing`) if one is present. If you don't have one
handy, see the script for self-contained instructions: binutils via
`brew install mips64-elf-binutils`, and IDO 5.3 from the 5.3 macOS build on
the [decompals/ido-static-recomp](https://github.com/decompals/ido-static-recomp)
project's Releases page.

## Diffing/matching tools (Phase 1)

`tools/asm-processor` **is** needed to build, from the moment the first C
file landed: it is what makes `#pragma GLOBAL_ASM(...)` work with IDO.
`tools/asm-differ` (`diff.sh`) and `tools/m2c` (`mips_to_c.sh`,
`generate_ctx.sh`) are the matching workflow and are not needed for
`gmake verify`. `tools/ido-static-recomp` is deliberately left uninitialised
— `tools/setup_toolchain.sh` installs a prebuilt IDO 5.3 instead.

```sh
git submodule update --init tools/asm-processor tools/asm-differ tools/m2c
```

`gmake setup` runs that for you.

## Writing C

Add the translation unit to `mickey.*.yaml` as a `c` subsegment, re-split
(`gmake` does it automatically), and fill in `src/`. Every function starts
life as `#pragma GLOBAL_ASM("asm/nonmatchings/<dir>/<func>.s")` — that
already builds byte-identically — and gets replaced by real C one function
at a time.

```sh
./diff.sh <symbol>          # target vs. current disassembly; matched == no diffs
./diff.sh -mw <symbol>      # rebuild first, then re-diff on every file save
./generate_ctx.sh           # optional: ctx.c, so m2c knows project types
./mips_to_c.sh <symbol>     # m2c first draft into m2cfiles/<symbol>.c
gmake verify                # the only claim that counts
```

C compiles with `-O2 -mips1 -32` by default. Anything else is a per-file
override in the Makefile's "Per-file compiler flags" block and has to be
justified in the source file's header comment.
