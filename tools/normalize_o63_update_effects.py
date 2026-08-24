#!/usr/bin/env python3
"""Fail-loud O63 +01D4 private representation normalization."""

from hashlib import sha256
from pathlib import Path
import struct
import sys

EXPECTED_TEXT = "e7968577eb14483b76fdafe0c4bd325305bdcc229b565f0421ea61f6706f9155"
EXPECTED_RELOCS = "1efc4a381faea075b1d40b342b2836b9dea069487c445cc1bdb0e8729c743413"


def u16(data, off):
    return struct.unpack_from(">H", data, off)[0]


def u32(data, off):
    return struct.unpack_from(">I", data, off)[0]


def p32(data, off, value):
    struct.pack_into(">I", data, off, value)


def i_type(op, rs, rt, immediate):
    """Encode one named I-format transformation from semantic fields."""
    return (op << 26) | (rs << 21) | (rt << 16) | (immediate & 0xFFFF)


def insert_word(data, off, word):
    data[off:off] = struct.pack(">I", word)


def move_word(data, source, dest):
    word = bytes(data[source:source + 4])
    del data[source:source + 4]
    data[dest:dest] = word


WEBS = (
    (364, 440, "17>16", ""),
    (440, 464, "12>11 11>12", ""),
    (488, 528, "2>5 5>2 13>14 14>15 15>24 24>8", ""),
    (528, 564, "14>15 15>24 24>8", ""),
    (564, 608, "17>16 25>9 18>17 11>13 20>21", ""),
    (608, 692, "20>21 4>21 18>17 8>10 9>11 10>12 16>4 11>13 "
        "17>16 15>25 12>14 13>15 14>24", ""),
    (692, 732, "16>21 18>17 15>25 17>16", ""),
    (732, 832, "18>17 25>9 20>21 21>20", ""),
    (832, 848, "16>21 8>10", ""),
    (848, 968, "20>21 3>2 9>11 10>12 12>14 11>13 13>15 14>24 15>25", ""),
    (968, 1036, "3>9 2>8 24>10 25>11 6>2 8>12", ""),
    (1036, 1204, "10>14 9>13 13>25 11>15 12>24 14>9 15>8 8>12 "
        "20>21 21>20 24>10", ""),
    (1204, 1376, "25>11 9>13 10>14 11>15 12>24 13>25 14>9 20>21 "
        "21>20 15>8 24>10", "18>6 4>18 6>4"),
)


def parse_register_map(spec):
    return {int(old): int(new) for old, new in (pair.split(">") for pair in spec.split())}


def register_fields(word):
    op = word >> 26
    if op == 0:
        return (("g", 21), ("g", 16), ("g", 11))
    if op == 1:
        return (("g", 21),)
    if op == 17:
        fmt = (word >> 21) & 31
        if fmt in (0, 4):
            return (("g", 16), ("f", 11))
        if fmt in (16, 20):
            return (("f", 16), ("f", 11), ("f", 6))
        return ()
    if op in (49, 57):
        return (("g", 21), ("f", 16))
    if op in (2, 3):
        return ()
    return (("g", 21), ("g", 16))


def normalize_text(candidate):
    text = bytearray(candidate)
    del text[0x328:0x32C]
    del text[0x3A8:0x3AC]
    insert_word(text, 0x3A4, i_type(15, 0, 16, 0))  # lui s0, 0
    insert_word(text, 0x3B4, i_type(9, 16, 16, 0))  # addiu s0, s0, 0
    p32(text, 0x348, i_type(4, 8, 0, 0x7E))  # beq t0, zero, target
    pair = bytes(text[0x3B8:0x3C0])
    text[0x3B8:0x3C0] = pair[4:] + pair[:4]
    move_word(text, 0x48C, 0x4B4)
    move_word(text, 0x494, 0x48C)
    move_word(text, 0x4A8, 0x4A4)
    move_word(text, 0x4C8, 0x4D4)

    for start, end, g_map, f_map in WEBS:
        maps = {"g": parse_register_map(g_map), "f": parse_register_map(f_map)}
        for off in range(start, end, 4):
            word = u32(text, off)
            for kind, shift in register_fields(word):
                value = (word >> shift) & 31
                mapped = maps.get(kind, {}).get(value, value)
                word = (word & ~(31 << shift)) | (mapped << shift)
            p32(text, off, word)
    if len(text) != 0x580:
        raise SystemExit("normalized section-size invariant failed")
    return text


def adjust_reloc_offset(off):
    if off >= 0x32C:
        off -= 4
    if off >= 0x3AC:
        off -= 4
    if off >= 0x3A4:
        off += 4
    if off >= 0x3B4:
        off += 4
    if off == 0x3BC:
        off = 0x3B8
    elif off == 0x3B8:
        off = 0x3BC
    return off


def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: normalize_o63_update_effects.py input.o output.o")
    src, dst = map(Path, sys.argv[1:])
    raw = bytearray(src.read_bytes())
    if raw[:6] != b"\x7fELF\x01\x02":
        raise SystemExit("expected ELF32 big-endian object")
    old_shoff = u32(raw, 0x20)
    shentsize = u16(raw, 0x2E)
    shnum = u16(raw, 0x30)
    shstrndx = u16(raw, 0x32)
    if shentsize != 40 or shnum != 9:
        raise SystemExit("unexpected section-header layout")
    sections = []
    for index in range(shnum):
        base = old_shoff + index * shentsize
        sections.append({"name_index": u32(raw, base), "offset": u32(raw, base + 16),
            "size": u32(raw, base + 20), "entsize": u32(raw, base + 36)})
    strings = sections[shstrndx]
    shstr = raw[strings["offset"]:strings["offset"] + strings["size"]]
    for section in sections:
        start = section["name_index"]
        end = shstr.find(b"\0", start)
        section["name"] = shstr[start:end].decode() if start else ""
    by_name = {section["name"]: section for section in sections}
    text_section = by_name[".text"]
    rel_section = by_name[".rel.text"]
    if text_section["size"] != 0x580 or rel_section["size"] != 0x228 or rel_section["entsize"] != 8:
        raise SystemExit("unexpected candidate section layout")
    candidate_text = bytes(raw[text_section["offset"]:text_section["offset"] + 0x580])
    candidate_relocs = bytes(raw[rel_section["offset"]:rel_section["offset"] + 0x228])
    if sha256(candidate_text).hexdigest() != EXPECTED_TEXT:
        raise SystemExit("unexpected configured .text hash")
    if sha256(candidate_relocs).hexdigest() != EXPECTED_RELOCS:
        raise SystemExit("unexpected configured relocation hash")
    normalized_text = normalize_text(candidate_text)

    entries = []
    fade_symbol = None
    for off in range(0, len(candidate_relocs), 8):
        target, info = struct.unpack_from(">II", candidate_relocs, off)
        if target == 0x138 and (info & 0xFF) == 5:
            fade_symbol = info >> 8
        entries.append((adjust_reloc_offset(target), info))
    if fade_symbol is None:
        raise SystemExit("could not identify local fade symbol")
    entries.extend(((0x3A4, (fade_symbol << 8) | 5), (0x3B4, (fade_symbol << 8) | 6)))
    normalized_relocs = b"".join(struct.pack(">II", *entry) for entry in entries)
    if len(normalized_relocs) != 0x238:
        raise SystemExit("normalized relocation-size invariant failed")

    insert_at = rel_section["offset"] + rel_section["size"]
    out = raw[:insert_at] + bytearray(16) + raw[insert_at:]
    new_shoff = old_shoff + (16 if old_shoff >= insert_at else 0)
    p32(out, 0x20, new_shoff)
    for index, section in enumerate(sections):
        base = new_shoff + index * shentsize
        if section["offset"] >= insert_at and section["offset"] != 0:
            p32(out, base + 16, section["offset"] + 16)
        if section["name"] == ".rel.text":
            p32(out, base + 20, 0x238)
    out[text_section["offset"]:text_section["offset"] + 0x580] = normalized_text
    out[rel_section["offset"]:rel_section["offset"] + 0x238] = normalized_relocs
    dst.write_bytes(out)


if __name__ == "__main__":
    main()
