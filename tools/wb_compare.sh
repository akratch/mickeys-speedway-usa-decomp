#!/usr/bin/env bash
# One decomp-workbench comparison for one function.
#
#   ./tools/wb_compare.sh [--diagnose] [--no-build] <symbol> [extra workbench args...]
#
# Friendly/generated aliases, the owning TU, and the canonical versus
# NON_MATCHING candidate tree are resolved automatically by
# function_preflight.py. The WB_CANDIDATE_BUILD_DIR and WB_CANDIDATE_SYMBOL
# environment overrides remain available for deliberately copied scratch
# objects; ordinary use should not need them.
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
# oracle, useless for diagnosing relocation questions. By default this forces
# only the cheap ELF -> BIN -> ROM derivation so a same-timestamp stale ROM
# cannot masquerade as current. --no-build instead requires the ROM to be
# strictly newer than its linked ELF.
set -euo pipefail
cd "$(dirname "$0")/.."

WB=.venv/bin/decomp-workbench
OBJDUMP=tools/binutils/mips64-elf-objdump
OBJCOPY=tools/binutils/mips64-elf-objcopy
AS=tools/binutils/mips64-elf-as
ASFLAGS="-march=vr4300 -32 -mabi=32 -G0 -I include"
OUT=build/wb
mkdir -p "$OUT"
PROVENANCE=.venv/bin/python
PROVENANCE_TOOL=tools/proof_provenance.py
PREFLIGHT_TOOL=tools/function_preflight.py

mode=asm
wb_command=compare
no_build=0
while [ $# -gt 0 ]; do
    case "$1" in
        --rom) mode=rom; shift ;;
        --diagnose) wb_command=diagnose; shift ;;
        --no-build) no_build=1; shift ;;
        --) shift; break ;;
        *) break ;;
    esac
done
if [ $# -lt 1 ]; then
    echo "usage: $0 [--rom] [--diagnose] [--no-build] <symbol> [workbench args...]" >&2
    exit 2
fi
sym=$1; shift
target_sym=$sym
candidate_sym=${WB_CANDIDATE_SYMBOL:-$sym}
source_path=
tu=
cand_build_dir=${WB_CANDIDATE_BUILD_DIR:-build}
asmfile=

if [ "$mode" = asm ]; then
    preflight_args=("$sym" --resolve-wb)
    if [ "$no_build" -eq 1 ]; then preflight_args+=(--no-build); fi
    resolution=$($PROVENANCE "$PREFLIGHT_TOOL" "${preflight_args[@]}") || exit $?
    IFS=$'\t' read -r resolved_target resolved_candidate resolved_source \
        resolved_tu resolved_build_dir resolved_asm <<< "$resolution"
    target_sym=$resolved_target
    candidate_sym=${WB_CANDIDATE_SYMBOL:-$resolved_candidate}
    source_path=$resolved_source
    tu=$resolved_tu
    cand_build_dir=${WB_CANDIDATE_BUILD_DIR:-$resolved_build_dir}
    asmfile=$resolved_asm
fi

# In asm mode, splat's extracted fallback can retain an auto-name while the
# linked TU exports the friendly C name.  The auto-name may survive only as a
# zero-size alias, so use the actual candidate symbol for linked ELF geometry.
# The target assembly lookup below must continue to use the requested auto-name.
linked_sym=$sym
if [ "$mode" = asm ]; then linked_sym=$candidate_sym; fi

proof_manifest="$OUT/$sym.provenance.json"

run_provenance() {
    local result=0
    "$PROVENANCE" "$PROVENANCE_TOOL" "$@" --manifest "$proof_manifest" || result=$?
    case "$result" in
        0) return 0 ;;
        3)
            # Keep fallback/unknown comparisons useful as diagnostics, but
            # make the comparator itself reject an exact result.  The census
            # is part of decomp-workbench's public CLI and composes with any
            # caller-provided assertions.
            set -- "${wb_extra_args[@]}" --census exact=false
            wb_extra_args=("$@")
            return 0
            ;;
        *) return "$result" ;;
    esac
}

wb_extra_args=("$@")

# Where the symbol ended up, straight from the linked ELF.
# (BSD awk has no strtonum, so the hex-to-decimal step is the shell's.)
read -r vram_hex size_hex section < <(
    "$OBJDUMP" -t build/mickey.us.elf \
    | awk -v s="$linked_sym" '$NF == s && NF >= 5 { print $1, $(NF-1), $(NF-2) }' \
    | head -1
)
vram=$(( 0x${vram_hex:-0} ))
size=$(( 0x${size_hex:-0} ))
if [ "$size" -eq 0 ]; then vram=""; fi
if [ -z "${vram:-}" ]; then echo "$0: '$linked_sym' not found in build/mickey.us.elf" >&2; exit 1; fi

if [ "$mode" = rom ]; then
    rom_build_dir=${WB_ROM_BUILD_DIR:-build}
    candidate_elf=$rom_build_dir/mickey.us.elf
    candidate_rom=$rom_build_dir/mickey.us.z64
    if [ ! -f "$candidate_elf" ]; then
        echo "$0: no linked ELF under '$rom_build_dir' -- run gmake first." >&2
        exit 1
    fi
    if [ "$no_build" -eq 0 ]; then
        # GNU Make can miss this edge when the link and prior ROM happen in the
        # same filesystem timestamp tick.  --what-if marks only the linked ELF
        # as newly changed, forcing the cheap ELF -> BIN -> ROM derivation
        # without recompiling or relinking the project.
        nice -n 10 gmake -j2 --no-print-directory \
            -W "$candidate_elf" "$candidate_rom"
    elif [ ! -f "$candidate_rom" ] || [ ! "$candidate_rom" -nt "$candidate_elf" ]; then
        echo "$0: '$candidate_rom' is not strictly newer than '$candidate_elf';" >&2
        echo "  omit --no-build to refresh the ROM proof artifact." >&2
        exit 2
    fi

    read -r candidate_vram_hex candidate_size_hex candidate_section < <(
        "$OBJDUMP" -t "$candidate_elf" \
        | awk -v s="$sym" '$NF == s && NF >= 5 { print $1, $(NF-1), $(NF-2) }' \
        | head -1
    )
    candidate_vram=$(( 0x${candidate_vram_hex:-0} ))
    candidate_size=$(( 0x${candidate_size_hex:-0} ))
    if [ "$candidate_size" -eq 0 ]; then
        echo "$0: '$sym' not found in $candidate_elf" >&2
        exit 1
    fi

    # A section's LMA is its ROM position and its VMA is its runtime address.
    # Their delta handles both resident code and overlays, whose shared load
    # VMA cannot use the resident segment's fixed ROM bias.
    symbol_rom() {
        local elf=$1 symbol_vram=$2 symbol_section=$3
        local section_vram_hex section_rom_hex section_vram section_rom
        read -r section_vram_hex section_rom_hex < <(
            "$OBJDUMP" -h "$elf" \
            | awk -v s="$symbol_section" '$2 == s { print $4, $5; exit }'
        )
        if [ -z "${section_vram_hex:-}" ] || [ -z "${section_rom_hex:-}" ]; then
            echo "$0: section '$symbol_section' not found in $elf" >&2
            return 1
        fi
        section_vram=$(( 0x$section_vram_hex ))
        section_rom=$(( 0x$section_rom_hex ))
        echo $(( section_rom + symbol_vram - section_vram ))
    }
    target_rom=$(symbol_rom build/mickey.us.elf "$vram" "$section")
    candidate_rom_offset=$(symbol_rom "$candidate_elf" "$candidate_vram" "$candidate_section")

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
        local input=$1 output=$2 dump_vram=$3 dump_rom=$4 dump_size=$5
        "$OBJDUMP" -D -b binary -m mips:4300 -EB \
            --adjust-vma=$(( dump_vram - dump_rom )) \
            --start-address=$(printf 0x%x "$dump_vram") \
            --stop-address=$(printf 0x%x $(( dump_vram + dump_size ))) \
            "$input" > "$output"
    }
    # --start/--stop-address are read in the *adjusted* space, so they are VRAM
    # addresses here, not ROM offsets. Passing ROM offsets silently produces an
    # empty dump, which the workbench then rejects for having no instructions.
    dump baseroms/mickey.us.z64 "$OUT/$sym.target.objdump" \
        "$vram" "$target_rom" "$tsize"
    dump "$candidate_rom" "$OUT/$sym.candidate.objdump" \
        "$candidate_vram" "$candidate_rom_offset" "$candidate_size"
    run_provenance \
        --mode rom \
        --symbol "$sym" \
        --candidate-symbol "$sym" \
        --candidate-build-dir "$rom_build_dir" \
        --candidate-artifact "$OUT/$sym.candidate.objdump" \
        --target-artifact "$OUT/$sym.target.objdump" \
        --objdump "$OBJDUMP"
    rom_command=compare-dumps
    if [ "$wb_command" = diagnose ]; then rom_command=diagnose-dumps; fi
    exec "$WB" "$rom_command" "$OUT/$sym.target.objdump" \
        "$OUT/$sym.candidate.objdump" "${wb_extra_args[@]}"
fi

if [ -z "$asmfile" ] || [ ! -f "$asmfile" ]; then
    echo "$0: no unique asm/nonmatchings fallback for '$target_sym'." >&2
    echo "  splat drops it once the C implements the function; try --rom." >&2
    exit 1
fi
cand=$cand_build_dir/src/$tu.c.o
if [ ! -f "$cand" ]; then echo "$0: no candidate object $cand -- run gmake first." >&2; exit 1; fi

printf '.set noat\n.set noreorder\n.include "macro.inc"\n.section .text, "ax"\n' > "$OUT/$sym.target.s"
cat "$asmfile" >> "$OUT/$sym.target.s"
$AS $ASFLAGS -o "$OUT/$sym.target.o" "$OUT/$sym.target.s"
if [ "$candidate_sym" != "$target_sym" ]; then
    $OBJCOPY --redefine-sym "$target_sym=$candidate_sym" "$OUT/$sym.target.o"
fi

run_provenance \
    --mode asm \
    --symbol "$target_sym" \
    --candidate-symbol "$candidate_sym" \
    --source "$source_path" \
    --candidate-build-dir "$cand_build_dir" \
    --candidate-object "$cand" \
    --target-object "$OUT/$sym.target.o" \
    --candidate-artifact "$cand" \
    --target-artifact "$OUT/$sym.target.o" \
    --objdump "$OBJDUMP"
exec "$WB" "$wb_command" "$OUT/$sym.target.o" "$cand" \
    --function "$candidate_sym" --objdump "$OBJDUMP" "${wb_extra_args[@]}"
