#!/usr/bin/env bash
# Smoke-tests that every tool this project depends on actually starts, and
# prints its version. Doesn't build anything or touch the ROM; safe to run
# any time. See docs/tools.md for what each one is for.
#
#   ./tools/check_tools.sh
set -u
cd "$(dirname "$0")/.."

PYTHON=.venv/bin/python
fail=0

check() {
    local name=$1; shift
    printf '%-22s ' "$name"
    if out=$("$@" 2>&1); then
        echo "OK  $(echo "$out" | head -1)"
    else
        echo "FAIL"
        echo "$out" | sed 's/^/    /'
        fail=1
    fi
}

check "splat"            "$PYTHON" -m splat --version
check "spimdisasm"       "$PYTHON" -c "import spimdisasm; print(spimdisasm.__version__)"
check "asm-differ"       "$PYTHON" tools/asm-differ/diff.py --help
check "m2c"              "$PYTHON" tools/m2c/m2c.py --help
check "mapfile_parser"   "$PYTHON" -c "import mapfile_parser; print(mapfile_parser.__version__)"
check "toml (permuter)"  "$PYTHON" -c "import toml; print(toml.__version__ if hasattr(toml, '__version__') else 'ok')"

if [ -e tools/permuter ]; then
    check "decomp-permuter" "$PYTHON" tools/permuter/permuter.py --help
else
    echo "decomp-permuter       SKIP (no tools/permuter symlink -- see docs/tools.md)"
fi

if [ -x tools/objdiff/objdiff-cli ]; then
    check "objdiff-cli" tools/objdiff/objdiff-cli --version
else
    echo "objdiff-cli            SKIP (run tools/setup_objdiff.sh)"
fi

if [ -x tools/ido/cc ]; then
    check "IDO (tools/ido/cc)" tools/ido/cc --version
else
    echo "IDO                    SKIP (gitignored, fetched by tools/setup_toolchain.sh)"
fi

if [ -x tools/binutils/mips64-elf-objdump ]; then
    check "mips64-elf-objdump" tools/binutils/mips64-elf-objdump --version
else
    echo "mips64-elf-objdump     SKIP (gitignored, fetched by tools/setup_toolchain.sh)"
fi

exit $fail
