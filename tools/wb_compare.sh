#!/usr/bin/env bash
# One decomp-workbench comparison for one function.
#
#   ./tools/wb_compare.sh <symbol> [extra decomp-workbench args...]
#
# The target object is the project's own disassembly of the function
# (asm/nonmatchings/**/<symbol>.s) assembled with the project's assembler; the
# candidate is the function as it came out of the normal, full-translation-unit
# C build. That pairing is the one the workbench documents: no isolation
# harness, so the candidate's codegen is the codegen that actually ships.
#
# Note the ordering constraint this creates. splat stops emitting a function's
# nonmatchings .s the moment the C file implements it, so the target object can
# only be built while the function is still unmatched. Run this *during*
# iteration, not after; `--rom` below is the fallback for a function that has
# already matched.
#
#   ./tools/wb_compare.sh --rom <symbol> [args...]
#
# compares the baserom's bytes against the built ROM's bytes over the symbol's
# address range instead. Fully relocated on both sides, so it can only ever
# report instruction-words-identical or a real difference -- useful as a final
# oracle, useless for diagnosing relocation questions.
set -euo pipefail
cd "$(dirname "$0")/.."

WB=.venv/bin/decomp-workbench
OBJDUMP=tools/binutils/mips64-elf-objdump
AS=tools/binutils/mips64-elf-as
ASFLAGS="-march=vr4300 -32 -mabi=32 -G0 -I include"
OUT=build/wb
mkdir -p "$OUT"

mode=asm
if [ "${1:-}" = "--rom" ]; then mode=rom; shift; fi
if [ $# -lt 1 ]; then echo "usage: $0 [--rom] <symbol> [args...]" >&2; exit 2; fi
sym=$1; shift

# Where the symbol ended up, straight from the linker map.
# (BSD awk has no strtonum, so the hex-to-decimal step is the shell's.)
read -r vram_hex size_hex < <(
    "$OBJDUMP" -t build/mickey.us.elf \
    | awk -v s="$sym" '$NF == s && NF >= 5 { print $1, $(NF-1) }' \
    | head -1
)
vram=$(( 0x${vram_hex:-0} ))
size=$(( 0x${size_hex:-0} ))
if [ "$size" -eq 0 ]; then vram=""; fi
if [ -z "${vram:-}" ]; then echo "$0: '$sym' not found in build/mickey.us.elf" >&2; exit 1; fi

if [ "$mode" = rom ]; then
    rom=$(( vram - 0x7FFFF400 ))
    # The candidate's size comes from the ELF, but the *target's* must not:
    # if the candidate is the wrong length, using its size for both sides
    # truncates or overruns the target and the comparison silently answers a
    # different question. symbol_addrs.us.txt carries the ROM's real size, so
    # prefer it and fall back to the ELF only for symbols that lack one.
    tsize=$(sed -n "s/^$sym *= *0x[0-9A-Fa-f]* *;.*size:0x\([0-9A-Fa-f]*\).*/\1/p" \
            symbol_addrs.us.txt | head -1)
    if [ -n "$tsize" ]; then tsize=$(( 0x$tsize )); else tsize=$size; fi
    # `set --` here would eat the caller's extra workbench arguments, so the
    # two dumps are written by name rather than by looping over pairs.
    dump() {
        "$OBJDUMP" -D -b binary -m mips:4300 -EB \
            --adjust-vma=$(( vram - rom )) \
            --start-address=$(printf 0x%x $vram) \
            --stop-address=$(printf 0x%x $(( vram + $3 ))) \
            "$1" > "$2"
    }
    # --start/--stop-address are read in the *adjusted* space, so they are VRAM
    # addresses here, not ROM offsets. Passing ROM offsets silently produces an
    # empty dump, which the workbench then rejects for having no instructions.
    dump baseroms/mickey.us.z64 "$OUT/$sym.target.objdump"    $tsize
    dump build/mickey.us.z64    "$OUT/$sym.candidate.objdump" $size
    exec "$WB" compare-dumps "$OUT/$sym.target.objdump" "$OUT/$sym.candidate.objdump" "$@"
fi

asmfile=$(find asm/nonmatchings -type f -name "$sym.s" | head -1)
if [ -z "$asmfile" ]; then
    echo "$0: no asm/nonmatchings/**/$sym.s." >&2
    echo "  splat drops it once the C implements the function; try --rom." >&2
    exit 1
fi
# The TU object is the one whose subsegment owns the asm file.
tu=$(dirname "${asmfile#asm/nonmatchings/}")
cand=build/src/$tu.c.o
if [ ! -f "$cand" ]; then echo "$0: no candidate object $cand -- run gmake first." >&2; exit 1; fi

printf '.set noat\n.set noreorder\n.include "macro.inc"\n.section .text, "ax"\n' > "$OUT/$sym.target.s"
cat "$asmfile" >> "$OUT/$sym.target.s"
$AS $ASFLAGS -o "$OUT/$sym.target.o" "$OUT/$sym.target.s"

exec "$WB" compare "$OUT/$sym.target.o" "$cand" --function "$sym" --objdump "$OBJDUMP" "$@"
