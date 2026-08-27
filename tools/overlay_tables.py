#!/usr/bin/env python3
"""Decode Mickey's overlay tables straight from ROM.

Finding: the tables are not compressed or synthesized at load time. They sit
flat in ROM at four fixed blocks (0x1848B70-0x184C3E0), and the boot-time
initializer `runlinkInit` (VRAM 0x800328CC, ROM 0x334CC) just mallocs and
DMA-copies each block into a BSS pointer. `mainRelocTable`'s block begins with
a u32 entry count; the initializer sets `mainRelocTable = copy + 4` past it.
Every prior scan of this region decoded 8-byte entries starting at the count
word -- one word out of phase -- which swapped the two fields of every entry
and made the table look unstructured. See runlink.h for the four struct
definitions this script decodes (RelocTableEntry, RomTableEntry, OverlayHeader,
RelocationEntry) and their field evidence.

Block layout:
    0x1848B70   u32 count (375), then RelocTableEntry[count]
    0x1849730   RomTableEntry[2004]
    0x184B680   OverlayHeader[107]      -- table row 0 == overlay number 1;
                                            overlay 0 ("main") is synthesized
                                            at load time and has no ROM row
    0x184C3E0   107 module images back to back, each [.text][.data][reloc1]
                [reloc2], ending exactly at 0x18F1FE0

Corrections this script encodes over a naive reading of runlink.h:
  - OverlayHeader[0x10] is bssSize, not rodataSize: dataSize (0x0C) covers
    data+rodata together, and the per-module byte-gap arithmetic (assertion 3
    below) only closes with 0x10 excluded from the ROM-bytes sum.
  - RelocTableEntry.unused (the low byte of word 1) is the flags byte shared
    with RelocationEntry.u.b.flags -- RELOC_TYPE_* high nibble, RELOC_OP_*
    low nibble.
  - OverlayHeader.romAddress, as stored in ROM, is an offset relative to
    0x184C3E0 (the module region base), not an absolute ROM address; the
    initializer adds 0x184C3E0 to it once at load time.

This script re-derives and *asserts* the claims above against the live ROM
image rather than trusting them, so a future misreading of the layout fails
loudly instead of silently reproducing the same phase error. See
`docs/overlays.md` section 5.3 for the validation writeup.

Usage:
    tools/overlay_tables.py                  # human-readable table + verify
    tools/overlay_tables.py --json
    tools/overlay_tables.py --tsv
    tools/overlay_tables.py --rom PATH/TO/rom.z64

Verification output goes to stderr; the module table (in whichever format)
goes to stdout, so `--json`/`--tsv` output stays machine-parseable even when
piped past the verification log. Exit status is nonzero if any assertion
fails.
"""

import argparse
import json
import os
import struct
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# --- Proven block bases (reloc-table-origin.md sec. 2) ----------------------
RELOC_TABLE_BASE = 0x1848B70   # u32 count, then RelocTableEntry[count]
ROM_TABLE_BASE = 0x1849730     # RomTableEntry[2004]
HEADER_TABLE_BASE = 0x184B680  # OverlayHeader[107]
MODULES_BASE = 0x184C3E0       # 107 module images
MODULES_END = 0x18F1FE0        # proven exact end == mickey.us.yaml rom_fill

ROM_TABLE_COUNT = 2004
HEADER_COUNT = 107
EXPECTED_RELOC_COUNT = 375

RELOC_RECORD_SIZE = 8

# jal 0x800333A0 (TrapDanglingJump), as a raw big-endian ROM word.
TRAP_DANGLING_JUMP_WORD = 0x0C00CCE8

# The resident segment's own overlayTable[0]: vramBase = 0x80000450. Reloc
# table call-site offsets are byte offsets from this.
RESIDENT_VRAM_BASE = 0x80000450
# VRAM = ROM + DELTA for the resident segment (tools/check_derived_numbers.py).
VRAM_ROM_DELTA = 0x7FFFF400

# clone/clone.c's __FILE__ strings (docs/modules.md sec. 3.1), used as an
# independent cross-check of the module-boundary arithmetic.
CLONE_STRINGS = (0x188B4D0, 0x188B4E0)
CLONE_OVERLAY_NUMBER = 43

RELOC_TYPE_NAMES = {2: "R_MIPS_32", 4: "R_MIPS_26", 5: "R_MIPS_HI16", 6: "R_MIPS_LO16"}
RELOC_OP_NAMES = {0: "SYMBOL", 1: "LOCAL", 2: "JUMP", 3: "DATA"}


def u32(d, a):
    return struct.unpack_from(">I", d, a)[0]


def s32(d, a):
    return struct.unpack_from(">i", d, a)[0]


def s16(d, a):
    return struct.unpack_from(">h", d, a)[0]


def u16(d, a):
    return struct.unpack_from(">H", d, a)[0]


def read_reloc_table(d):
    """(count, entries) for the main relocation table at RELOC_TABLE_BASE."""
    count = u32(d, RELOC_TABLE_BASE)
    base = RELOC_TABLE_BASE + 4
    entries = []
    for i in range(count):
        off = base + i * 8
        w1 = u32(d, off + 4)
        entries.append({
            "index": i,
            "entry_rom": off,
            "rom_table_index": u32(d, off),
            "call_site_offset": w1 >> 8,   # top 24 bits, big-endian bitfield
            "flags": w1 & 0xFF,            # bottom 8 bits
        })
    return count, entries


def read_rom_table(d):
    """Decode the 2004 packed RomTableEntry values.

    Each row names an exported address as ``overlay number + byte offset``.
    Overlay zero is the resident module; 0xFFD..0xFFF are the runtime linker's
    reserved section selectors.  Keep the table index in every row because it
    is the identifier stored in SYMBOL relocation records.
    """
    entries = []
    for i in range(ROM_TABLE_COUNT):
        word = u32(d, ROM_TABLE_BASE + i * 4)
        entries.append({
            "index": i,
            "entry_rom": ROM_TABLE_BASE + i * 4,
            "overlay": word >> 20,
            "offset": word & 0xFFFFF,
        })
    return entries


def read_headers(d):
    """OverlayHeader[HEADER_COUNT] at HEADER_TABLE_BASE, raw ROM fields."""
    out = []
    for i in range(HEADER_COUNT):
        b = HEADER_TABLE_BASE + i * 0x20
        out.append({
            "vram_base": s32(d, b + 0x00),
            "rom_address_raw": s32(d, b + 0x04),   # offset from MODULES_BASE
            "text_size": s32(d, b + 0x08),
            "data_size": s32(d, b + 0x0C),          # data + rodata
            "bss_size": s32(d, b + 0x10),           # was misdocumented rodataSize
            "reloc1_size": s16(d, b + 0x14),
            "reloc2_size": u16(d, b + 0x16),
            "init_function": s32(d, b + 0x18),      # unverified field, see runlink.h
            "resume_function": s32(d, b + 0x1C),
        })
    return out


def build_modules(headers):
    mods = []
    for i, h in enumerate(headers):
        rom_start = MODULES_BASE + h["rom_address_raw"]
        body = h["text_size"] + h["data_size"] + h["reloc1_size"] + h["reloc2_size"]
        mods.append({
            "overlay": i + 1,   # ROM row 0 == overlay number 1; 0 is synthesized main
            "rom_start": rom_start,
            "rom_end": rom_start + body,
            "text_size": h["text_size"],
            "data_size": h["data_size"],
            "bss_size": h["bss_size"],
            "reloc1_size": h["reloc1_size"],
            "reloc2_size": h["reloc2_size"],
            "vram_base": h["vram_base"],
            "init_function": h["init_function"],
            "resume_function": h["resume_function"],
        })
    return mods


def module_section_ranges(module):
    """Canonical half-open ROM ranges for one shipped module image."""
    text_start = module["rom_start"]
    data_start = text_start + module["text_size"]
    reloc1_start = data_start + module["data_size"]
    reloc2_start = reloc1_start + module["reloc1_size"]
    return {
        "text": (text_start, data_start),
        # The header has one size for initialized data.  It does not expose a
        # data/rodata boundary, so naming this range `data_rodata` is an
        # evidence constraint rather than a cosmetic preference.
        "data_rodata": (data_start, reloc1_start),
        "reloc1": (reloc1_start, reloc2_start),
        "reloc2": (reloc2_start, module["rom_end"]),
    }


def read_module_relocations(d, module, rom_table=None):
    """Decode both RelocationEntry arrays for one module.

    ``reloc1_size`` and ``reloc2_size`` are byte counts.  A SYMBOL operation's
    ``symbol_index`` addresses ``overlayRomTable``; LOCAL/JUMP/DATA operations
    interpret it differently, so target fields are only attached to SYMBOL
    rows.  This distinction prevents an offset from being reported as a table
    index merely because both occupy the same word in the serialized union.
    """
    if rom_table is None:
        rom_table = read_rom_table(d)
    ranges = module_section_ranges(module)
    records = []
    for table_number, section_name in ((1, "reloc1"), (2, "reloc2")):
        start, end = ranges[section_name]
        if (end - start) % RELOC_RECORD_SIZE:
            raise ValueError(
                f"overlay {module['overlay']} {section_name} has non-record "
                f"size {end - start:#x}"
            )
        for index, entry_rom in enumerate(range(start, end, RELOC_RECORD_SIZE)):
            symbol_index = u32(d, entry_rom)
            info = u32(d, entry_rom + 4)
            flags = info & 0xFF
            op = flags & 0xF
            mode = flags >> 4
            record = {
                "table": table_number,
                "index": index,
                "entry_rom": entry_rom,
                "symbol_index": symbol_index,
                "target_offset": info >> 8,
                "flags": flags,
                "mode": mode,
                "mode_name": RELOC_TYPE_NAMES.get(mode, f"type{mode}"),
                "op": op,
                "op_name": RELOC_OP_NAMES.get(op, f"op{op}"),
            }
            if op == 0:
                if symbol_index >= len(rom_table):
                    raise ValueError(
                        f"overlay {module['overlay']} {section_name}[{index}] "
                        f"uses ROM-table index {symbol_index}, past "
                        f"{len(rom_table)} entries"
                    )
                target = rom_table[symbol_index]
                record["target_overlay"] = target["overlay"]
                record["target_symbol_offset"] = target["offset"]
            records.append(record)
    return records


def verify(d, count, entries, mods):
    """Re-derive reloc-table-origin.md's five claims; fail loudly on mismatch."""
    ok = True

    def check(cond, label, detail=""):
        nonlocal ok
        print(f"[{'PASS' if cond else 'FAIL'}] {label}", file=sys.stderr)
        if detail:
            print(f"       {detail}", file=sys.stderr)
        if not cond:
            ok = False
        return cond

    # 1. Count word and phase.
    check(count == EXPECTED_RELOC_COUNT, f"reloc table count == {EXPECTED_RELOC_COUNT}",
          f"got {count}")
    check(bool(entries) and entries[0]["entry_rom"] == RELOC_TABLE_BASE + 4,
          "table decodes starting at base+4 (past the count word)")

    # 2. 370/375 entries -> jal TrapDanglingJump; characterize the rest.
    hits, misses = [], []
    for e in entries:
        call_vram = e["call_site_offset"] + RESIDENT_VRAM_BASE
        call_rom = call_vram - VRAM_ROM_DELTA
        word = u32(d, call_rom)
        e["call_site_rom"] = call_rom
        e["call_site_word"] = word
        (hits if word == TRAP_DANGLING_JUMP_WORD else misses).append(e)
    check(len(hits) == 370 and len(misses) == 5,
          "370/375 reloc entries resolve to jal TrapDanglingJump (0x0C00CCE8)",
          f"got {len(hits)} matching, {len(misses)} not")
    for e in misses:
        t, o = (e["flags"] >> 4) & 0xF, e["flags"] & 0xF
        tname = RELOC_TYPE_NAMES.get(t, f"type{t}")
        oname = RELOC_OP_NAMES.get(o, f"op{o}")
        print(f"       non-jal #{e['index']}: flags={e['flags']:#04x} "
              f"({tname}/{oname}) call_site_rom={e['call_site_rom']:#x} "
              f"word={e['call_site_word']:#010x}", file=sys.stderr)

    # 3. Contiguous module byte sizes, all 107.
    gap_mismatches = [i for i in range(len(mods) - 1)
                       if mods[i]["rom_end"] != mods[i + 1]["rom_start"]]
    check(not gap_mismatches,
          "next.romAddress - this.romAddress == text+data+reloc1+reloc2 (106 gaps)",
          f"mismatches at index {gap_mismatches}" if gap_mismatches else "")

    # 4. Final module ends byte-exact at MODULES_END.
    check(mods[-1]["rom_end"] == MODULES_END, f"final module ends at {MODULES_END:#x}",
          f"got {mods[-1]['rom_end']:#x}")

    # 5. Independent cross-check: clone.c's __FILE__ strings land in overlay
    #    43's data section under this arithmetic (docs/modules.md sec. 3.1).
    ov = mods[CLONE_OVERLAY_NUMBER - 1]
    data_start = ov["rom_start"] + ov["text_size"]
    data_end = data_start + ov["data_size"]
    s1, s2 = CLONE_STRINGS
    in_range = data_start <= s1 < data_end and data_start <= s2 < data_end
    check(in_range,
          f"clone.c strings {s1:#x}/{s2:#x} land in overlay {CLONE_OVERLAY_NUMBER}'s .data",
          f"overlay {CLONE_OVERLAY_NUMBER} data=[{data_start:#x},{data_end:#x}) "
          f"offsets={s1 - ov['rom_start']:#x}/{s2 - ov['rom_start']:#x}")

    return ok


def print_human(mods):
    print(f"{'ov':>3} {'rom_start':>9} {'rom_end':>9} {'text':>6} {'data':>6} "
          f"{'bss':>6} {'reloc1':>6} {'reloc2':>6} {'vram':>10} "
          f"{'init':>10} {'resume':>10}")
    for m in mods:
        print(f"{m['overlay']:>3} {m['rom_start']:#09x} {m['rom_end']:#09x} "
              f"{m['text_size']:#06x} {m['data_size']:#06x} {m['bss_size']:#06x} "
              f"{m['reloc1_size']:#06x} {m['reloc2_size']:#06x} "
              f"{m['vram_base']:#010x} {m['init_function']:#010x} "
              f"{m['resume_function']:#010x}")


def print_tsv(mods):
    cols = ["overlay", "rom_start", "rom_end", "text_size", "data_size",
            "bss_size", "reloc1_size", "reloc2_size", "vram_base",
            "init_function", "resume_function"]
    print("\t".join(cols))
    for m in mods:
        print("\t".join(str(m[c]) for c in cols))


def main():
    ap = argparse.ArgumentParser(
        description=__doc__.split("\n\n")[0],
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--rom", default=os.path.join(REPO, "baseroms", "mickey.us.z64"),
                     help="path to mickey.us.z64 (default: baseroms/mickey.us.z64)")
    fmt = ap.add_mutually_exclusive_group()
    fmt.add_argument("--json", action="store_true", help="emit JSON instead of a table")
    fmt.add_argument("--tsv", action="store_true", help="emit TSV instead of a table")
    args = ap.parse_args()

    with open(args.rom, "rb") as fh:
        d = fh.read()

    count, entries = read_reloc_table(d)
    headers = read_headers(d)
    mods = build_modules(headers)

    ok = verify(d, count, entries, mods)

    if args.json:
        print(json.dumps(mods, indent=2))
    elif args.tsv:
        print_tsv(mods)
    else:
        print_human(mods)

    if not ok:
        print("\noverlay_tables.py: one or more assertions FAILED -- see stderr",
              file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
