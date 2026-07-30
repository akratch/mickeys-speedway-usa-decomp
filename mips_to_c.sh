#!/usr/bin/env bash
# m2c first draft for one function:
#
#   ./mips_to_c.sh <symbol> [extra m2c args...]
#
# Writes m2cfiles/<symbol>.c. The output is a *draft* -- m2c reconstructs
# control flow faithfully but names and types are guesses, and the result
# almost never matches as-is. Rewrite it as ordinary C, then check with
# `./diff.sh <symbol>` and `gmake verify`.
#
# Run ./generate_ctx.sh first if you want m2c to know the project's types;
# without ctx.c it falls back to u32/s32 for everything, which is usually
# fine for small leaf functions.
set -euo pipefail
cd "$(dirname "$0")"

if [ $# -lt 1 ]; then
    echo "usage: $0 <symbol> [extra m2c args...]" >&2
    exit 2
fi
sym=$1
shift

# Find the disassembly. Prefer asm/nonmatchings/**/<symbol>.s -- the
# per-function files splat writes for `c` subsegments -- and fall back to
# searching the whole-file asm/*.s output for the function's glabel.
#
# The old version of this script ran `find . -name "*$sym.s"`, a *suffix*
# match: once asm/nonmatchings/ is populated, "strchr" also matches a
# hypothetical "mystrchr.s", and multiple matches silently expanded into
# several filenames on m2c's command line. Both lookups below are exact, and
# an ambiguous result is an error rather than a guess.
hits=()
while IFS= read -r line; do
    hits+=("$line")
done < <(find asm/nonmatchings -type f -name "$sym.s" 2>/dev/null || true)

if [ ${#hits[@]} -eq 0 ]; then
    while IFS= read -r line; do
        hits+=("$line")
    done < <(grep -rlE "^[[:space:]]*(glabel|alabel)[[:space:]]+$sym\$" \
        asm --include='*.s' 2>/dev/null || true)
fi

if [ ${#hits[@]} -eq 0 ]; then
    echo "$0: no disassembly found for '$sym'." >&2
    echo "  Looked for asm/nonmatchings/**/$sym.s and 'glabel $sym' in asm/**.s." >&2
    echo "  Has the ROM been split? Try: gmake extract" >&2
    exit 1
fi

if [ ${#hits[@]} -gt 1 ]; then
    echo "$0: '$sym' is ambiguous, found in:" >&2
    printf '  %s\n' "${hits[@]}" >&2
    exit 1
fi

asmfile=${hits[0]}

mkdir -p m2cfiles
out=m2cfiles/$sym.c

ctx_args=()
if [ -f ctx.c ]; then
    ctx_args=(--context ctx.c)
    printf '#include "../ctx.c"\n\n' > "$out"
else
    : > "$out"
fi

echo "$0: $asmfile -> $out" >&2

# --target mips-ido-c: IDO codegen conventions, which is what this ROM was
# built with. --pointer-style right puts the * on the variable.
.venv/bin/python tools/m2c/m2c.py \
    --target mips-ido-c \
    --pointer-style right \
    "${ctx_args[@]}" \
    -f "$sym" \
    "$@" \
    "$asmfile" >> "$out"
