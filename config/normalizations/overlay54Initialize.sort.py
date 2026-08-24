#!/usr/bin/env python3
"""Fail-loud ordering for the exact reviewed O54 +0x0000 REL contract."""

import struct
import sys
from pathlib import Path

if len(sys.argv) != 2:
    raise SystemExit("usage: overlay54Initialize.sort.py OBJECT")
PATH = Path(sys.argv[1])

EXPECTED_ROWS = [
    (0x28, 4, "func_overlay_054_F0000000_189ECA0"),
    (0x38, 4, "func_overlay_054_F0000000_189ECA0"),
    (0x44, 4, "func_overlay_054_F0000000_189ECA0"),
    (0x50, 4, "func_overlay_054_F0000000_189ECA0"),
    (0x4c, 5, "D_24"),
    (0x54, 6, "D_24"),
    (0x58, 4, "func_overlay_054_F0000000_189ECA0"),
    (0x6c, 4, "func_overlay_054_F0000000_189ECA0"),
    (0x78, 4, "func_overlay_054_F00003CC_189F06C"),
    (0x74, 5, "D_2C"),
    (0x7c, 6, "D_2C"),
    (0x84, 4, "func_overlay_054_F00003CC_189F06C"),
    (0x80, 5, "D_4C"),
    (0x88, 6, "D_4C"),
    (0x90, 4, "func_overlay_054_F00003CC_189F06C"),
    (0x8c, 5, "D_6C"),
    (0x94, 6, "D_6C"),
    (0x9c, 4, "func_overlay_054_F00003CC_189F06C"),
    (0x98, 5, "D_9C"),
    (0xa0, 6, "D_9C"),
    (0xa8, 4, "func_overlay_054_F00003CC_189F06C"),
    (0xa4, 5, "D_CC"),
    (0xac, 6, "D_CC"),
    (0xb4, 4, "func_overlay_054_F00003CC_189F06C"),
    (0xb0, 5, "D_16C"),
    (0xb8, 6, "D_16C"),
    (0xc0, 4, "func_overlay_054_F00003CC_189F06C"),
    (0xbc, 5, "D_278"),
    (0xc4, 6, "D_278"),
    (0xc8, 5, "D_648"),
    (0xcc, 6, "D_648"),
    (0xec, 5, "D_640"),
    (0xf0, 6, "D_640"),
    (0xe8, 5, "D_654"),
    (0xf4, 6, "D_654"),
    (0xe4, 5, "D_5C0"),
    (0xf8, 6, "D_5C0"),
    (0xe0, 5, "D_340"),
    (0xfc, 6, "D_340"),
    (0xdc, 5, "D_280"),
    (0x100, 6, "D_280"),
    (0xd8, 5, "D_1C0"),
    (0x104, 6, "D_1C0"),
    (0xd4, 5, "D_140"),
    (0x108, 6, "D_140"),
    (0xd0, 5, "D_C0"),
    (0x10c, 6, "D_C0"),
    (0x118, 5, "D_2C"),
    (0x11c, 6, "D_2C"),
    (0x128, 4, "func_overlay_054_F000041C_189F0BC"),
    (0x130, 5, "D_4C"),
    (0x134, 6, "D_4C"),
    (0x140, 4, "func_overlay_054_F000041C_189F0BC"),
    (0x148, 5, "D_6C"),
    (0x14c, 6, "D_6C"),
    (0x158, 4, "func_overlay_054_F000041C_189F0BC"),
    (0x160, 5, "D_9C"),
    (0x164, 6, "D_9C"),
    (0x170, 4, "func_overlay_054_F000041C_189F0BC"),
    (0x178, 5, "D_CC"),
    (0x17c, 6, "D_CC"),
    (0x188, 4, "func_overlay_054_F000041C_189F0BC"),
    (0x190, 5, "D_16C"),
    (0x194, 6, "D_16C"),
    (0x1a0, 4, "func_overlay_054_F000041C_189F0BC"),
    (0x21c, 5, "D_268"),
    (0x220, 6, "D_268"),
    (0x218, 5, "D_1E8"),
    (0x224, 6, "D_1E8"),
    (0x214, 5, "D_10"),
    (0x228, 6, "D_10"),
    (0x2b0, 4, "func_overlay_054_F0000000_189ECA0"),
    (0x2b8, 4, "func_overlay_054_F0000000_189ECA0"),
    (0x2d8, 5, "D_A0"),
    (0x2dc, 6, "D_A0"),
    (0x2f8, 5, "D_660"),
    (0x2fc, 6, "D_660"),
    (0x330, 5, "D_668"),
    (0x338, 6, "D_668"),
    (0x33c, 4, "func_overlay_054_F0000000_189ECA0"),
    (0x344, 4, "func_overlay_054_F0000000_189ECA0"),
    (0x34c, 4, "func_overlay_054_F0000000_189ECA0"),
    (0x360, 4, "func_overlay_054_F0000000_189ECA0"),
    (0x368, 5, "D_668"),
    (0x36c, 6, "D_668"),
    (0x378, 4, "func_overlay_054_F0000000_189ECA0"),
]

data = bytearray(PATH.read_bytes())
shoff = struct.unpack_from(">I", data, 0x20)[0]
shentsize = struct.unpack_from(">H", data, 0x2E)[0]
shnum = struct.unpack_from(">H", data, 0x30)[0]
shstrndx = struct.unpack_from(">H", data, 0x32)[0]
headers = [struct.unpack_from(">10I", data, shoff + i * shentsize)
           for i in range(shnum)]
shstr = headers[shstrndx]
names_blob = data[shstr[4]:shstr[4] + shstr[5]]


def cstr(blob, offset):
    return bytes(blob[offset:blob.index(0, offset)]).decode("ascii")


names = [cstr(names_blob, h[0]) for h in headers]
text_index = names.index(".text")
rel_index = next(i for i, h in enumerate(headers)
                 if h[1] == 9 and h[7] == text_index)
rel = headers[rel_index]
symtab = headers[rel[6]]
strtab = headers[symtab[6]]
strings = data[strtab[4]:strtab[4] + strtab[5]]
symbol_names = []
for offset in range(symtab[4], symtab[4] + symtab[5], symtab[9] or 16):
    symbol_names.append(cstr(strings, struct.unpack_from(">I", data, offset)[0]))

entries = {}
for offset in range(rel[4], rel[4] + rel[5], 8):
    where, info = struct.unpack_from(">II", data, offset)
    key = (where, info & 0xFF, symbol_names[info >> 8])
    assert key not in entries, key
    entries[key] = bytes(data[offset:offset + 8])

desired = EXPECTED_ROWS
assert set(entries) == set(desired), (set(entries) - set(desired),
                                      set(desired) - set(entries))
ordered = b"".join(entries[key] for key in desired)
assert len(ordered) == rel[5]
data[rel[4]:rel[4] + rel[5]] = ordered
PATH.write_bytes(data)
print(f"PASS ordered {len(desired)} target-static .text relocations")
