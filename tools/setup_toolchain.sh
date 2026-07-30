#!/usr/bin/env bash
# Bootstraps the MIPS binutils + IDO 5.3 toolchain from a local DKR checkout,
# falling back to self-contained (non-DKR) instructions when no such
# checkout is available.
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

# This script only knows two concrete recipes: copy from a local DKR
# checkout (any OS the checkout itself targets), or the self-contained macOS
# fallback below. Anything else, say so honestly instead of guessing.
IDO_SUBDIR="macos"
case "$(uname -s)" in
  Darwin)
    ;;
  Linux)
    if [ -d "$DKR/tools/ido-recomp/linux" ]; then
      IDO_SUBDIR="linux"
    else
      echo "Linux detected, but no DKR checkout with a tools/ido-recomp/linux" >&2
      echo "output was found (DKR_PATH=$DKR)." >&2
      echo "This script has no self-contained Linux fallback yet: obtain/build" >&2
      echo "IDO 5.3 for Linux yourself (see" >&2
      echo "https://github.com/decompals/ido-static-recomp) and a mips64-elf" >&2
      echo "binutils for your distro, then populate tools/binutils/ and" >&2
      echo "tools/ido/ by hand to match the layout this Makefile expects." >&2
      exit 1
    fi
    ;;
  *)
    echo "Unsupported OS: $(uname -s)." >&2
    echo "This script only supports macOS, and Linux when a DKR checkout with" >&2
    echo "Linux IDO output is available via DKR_PATH." >&2
    exit 1
    ;;
esac

if [ -d "$DKR/tools/binutils" ]; then
  rsync -a "$DKR/tools/binutils/" binutils/
  rsync -a "$DKR/tools/ido-recomp/$IDO_SUBDIR/" ido/
  echo "Toolchain copied from $DKR"
else
  cat >&2 <<'EOF'
No DKR checkout found (set DKR_PATH to point at one, or set up the
toolchain yourself):

binutils (mips64-elf-{as,ld,ar,objcopy,objdump,strip}):
  brew install mips64-elf-binutils
  The binaries land under the Homebrew prefix (e.g. "$(brew --prefix)/bin")
  with a mips64-elf- prefix. Symlink or copy them into tools/binutils/ so
  the layout matches what this Makefile expects
  (CROSS = tools/binutils/mips64-elf-).

IDO 5.3 (the cc/as1/ld/libc.so that make the recompiled compiler runnable
natively):
  Download the 5.3 macOS build from the project's Releases page --
  https://github.com/decompals/ido-static-recomp/releases -- and extract it
  into tools/ido/.
EOF
  exit 1
fi
