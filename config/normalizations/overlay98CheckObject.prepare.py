#!/usr/bin/env python3
"""Fail-loud O98 +0848 frame/CFG/private-allocation normalization."""

from hashlib import sha256
from pathlib import Path
import struct
import sys

TEXT_HASH = "342e0030b65c65008e0132132b109afceda0fb99afd1911559c1340b1119b0c3"
RELOC_HASH = "de7cce761bb163250d215992ae66abb6d40c83188fbe4e9aa875b0cbc8285cc5"

WEBS = (
    (0x38, 0xAC, {"g": {12: 3, 13: 12}}),
    (0xAC, 0x148, {"g": {14: 13, 15: 14, 24: 15, 25: 24, 9: 25, 8: 9}}),
    (0x148, 0x1C8, {"g": {10: 8, 24: 15}, "f": {18: 2, 4: 18, 6: 4, 8: 6}}),
)

def u16(b, o): return struct.unpack_from(">H", b, o)[0]
def u32(b, o): return struct.unpack_from(">I", b, o)[0]
def p32(b, o, v): struct.pack_into(">I", b, o, v)

def fields(word):
    op = word >> 26
    if op == 0: return (("g", 21), ("g", 16), ("g", 11))
    if op == 1: return (("g", 21),)
    if op == 17:
        fmt = (word >> 21) & 31
        if fmt in (0, 4): return (("g", 16), ("f", 11))
        if fmt in (16, 20): return (("f", 16), ("f", 11), ("f", 6))
        return ()
    if op in (49, 57): return (("g", 21), ("f", 16))
    if op in (2, 3): return ()
    return (("g", 21), ("g", 16))

def apply_map(text, start, end, maps):
    for off in range(start, end, 4):
        word = u32(text, off)
        for kind, shift in fields(word):
            value = (word >> shift) & 31
            value = maps.get(kind, {}).get(value, value)
            word = (word & ~(31 << shift)) | (value << shift)
        p32(text, off, word)

def main():
    if len(sys.argv) != 3: raise SystemExit("usage: normalize.py input.o output.o")
    raw = bytearray(Path(sys.argv[1]).read_bytes())
    shoff = u32(raw, 0x20); shnum = u16(raw, 0x30); shstrndx = u16(raw, 0x32)
    sections = []
    for i in range(shnum):
        o = shoff + i * 40
        sections.append({"name_index": u32(raw, o), "offset": u32(raw, o + 16),
            "size": u32(raw, o + 20), "entsize": u32(raw, o + 36)})
    strings = sections[shstrndx]
    names = raw[strings["offset"]:strings["offset"] + strings["size"]]
    for s in sections:
        e = names.find(b"\0", s["name_index"])
        s["name"] = names[s["name_index"]:e].decode() if s["name_index"] else ""
    by = {s["name"]: s for s in sections}; tx = by[".text"]; rel = by[".rel.text"]
    candidate = bytes(raw[tx["offset"]:tx["offset"] + tx["size"]])
    candidate_rel = bytes(raw[rel["offset"]:rel["offset"] + rel["size"]])
    if tx["size"] != 0x1C0 or rel["size"] != 0x30:
        raise SystemExit("unexpected section layout")
    if sha256(candidate).hexdigest() != TEXT_HASH or sha256(candidate_rel).hexdigest() != RELOC_HASH:
        raise SystemExit("configured object hash guard failed")
    text = bytearray(candidate)
    guards = {0: 0x27BDFF80, 0x50: 0x1720000E, 0x84: 0x10000003,
              0x88: 0x46083502, 0x94: 0xC66C000C, 0x1B4: 0x27BD0080}
    for off, word in guards.items():
        if u32(text, off) != word: raise SystemExit(f"instruction guard changed at +0x{off:X}")
    p32(text, 0, 0x27BDFF58)
    pair = bytes(text[0x84:0x8C]); text[0x84:0x8C] = pair[4:] + pair[:4]
    text[0x8C:0x8C] = struct.pack(">I", 0xC66C000C)
    p32(text, 0x88, 0x10000004)
    text += bytes(4)
    p32(text, 0x50, 0x1720000F)
    p32(text, 0x1B8, 0x27BD00A8)

    apply_map(text, 0x38, 0x1B4, {"g": {23: 30, 30: 23}})
    p32(text, 0x18C, 0x8FB70044)
    p32(text, 0x1B0, 0x8FBE0048)
    for start, end, maps in WEBS: apply_map(text, start, end, maps)
    if len(text) != 0x1C8: raise SystemExit("normalized text-size invariant failed")

    rels = bytearray(candidate_rel)
    for off in range(0, len(rels), 8):
        target = u32(rels, off)
        if target >= 0xA0: p32(rels, off, target + 4)

    insert_at = tx["offset"] + tx["size"]
    out = raw[:insert_at] + bytearray(8) + raw[insert_at:]
    new_shoff = shoff + 8
    p32(out, 0x20, new_shoff)
    for i, s in enumerate(sections):
        base = new_shoff + i * 40
        if s["offset"] >= insert_at and s["offset"]: p32(out, base + 16, s["offset"] + 8)
        if s["name"] == ".text": p32(out, base + 20, 0x1C8)
    out[tx["offset"]:tx["offset"] + 0x1C8] = text
    new_rel_off = rel["offset"] + 8
    out[new_rel_off:new_rel_off + len(rels)] = rels

    sym = by[".symtab"]
    symoff = sym["offset"] + (8 if sym["offset"] >= insert_at else 0)
    found = 0
    for off in range(symoff, symoff + sym["size"], 16):
        if (out[off + 12] & 0xF) == 2 and u32(out, off + 4) == 0 and u32(out, off + 8) == 0x1B8:
            p32(out, off + 8, 0x1BC); found += 1
    if found != 1: raise SystemExit("function symbol guard failed")
    Path(sys.argv[2]).write_bytes(out)

if __name__ == "__main__": main()
