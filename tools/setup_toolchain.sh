#!/usr/bin/env bash
# Bootstraps the MIPS binutils + IDO 5.3 toolchain from a local DKR checkout.
#
# Layout discovered in ~/Desktop/dev/Diddy-Kong-Racing (read-only reference,
# inspected 2026-07-31):
#   tools/binutils/            -- flat dir of prebuilt mips64-elf-{as,ld,ar,
#                                  objcopy,objdump,strip} binaries (no nested
#                                  bin/ subdir), target-triple prefix "mips64-elf"
#   tools/ido-recomp/macos/    -- decompals ido-static-recomp output (cc, as1,
#                                  ld, libc.so, etc.), IDO 5.3 recompiled to run
#                                  natively on macOS (universal x86_64/arm64)
# Both paths match what this script originally assumed, so no adjustment to
# the rsync sources was needed -- only confirmed against the real layout.
set -eu
DKR="${DKR_PATH:-$HOME/Desktop/dev/Diddy-Kong-Racing}"
cd "$(dirname "$0")"
if [ -d "$DKR/tools/binutils" ]; then
  rsync -a "$DKR/tools/binutils/" binutils/
  rsync -a "$DKR/tools/ido-recomp/macos/" ido/
  echo "Toolchain copied from $DKR"
else
  echo "DKR checkout not found. Get binutils via DKR's tools/get-binutils.sh"
  echo "and IDO 5.3 from https://github.com/decompals/ido-static-recomp releases."
  exit 1
fi
