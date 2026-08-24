#!/usr/bin/env python3
"""Expand O31's asset-state index carrier into the shipped equivalent web."""

import hashlib
import pathlib
import struct
import sys

EXPECTED = "2e0e502bdd2260d6b96852742da823aa9563100049917e794fefe731619b6334"
SYMBOL = "func_overlay_031_F00002E8_187F808"
NATURAL_FUNCTION_SIZE = 0x204
OWNED_SIZE = 0x210
EXPECTED_RELOCS = (
    0x04, 0x0C, 0x08, 0x10, 0x40, 0x48, 0x4C, 0x54, 0x58, 0x98, 0xAC,
    0xB0, 0xA0, 0xA4, 0xF8, 0x100, 0x114, 0x118, 0x108, 0x10C, 0x154,
    0x160, 0x164, 0x1A8,
)


def cstr(blob, offset):
    end = blob.find(b"\0", offset)
    return blob[offset:end].decode()


if len(sys.argv) != 2:
    raise SystemExit("usage: overlay31InitializeParticleAssets.prepare.py OBJECT")
path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit("expected big-endian ELF32")

shoff = struct.unpack_from(">I", data, 0x20)[0]
stride = struct.unpack_from(">H", data, 0x2E)[0]
count = struct.unpack_from(">H", data, 0x30)[0]
shstrndx = struct.unpack_from(">H", data, 0x32)[0]
headers = [struct.unpack_from(">10I", data, shoff + i * stride) for i in range(count)]
shstr = headers[shstrndx]
names = data[shstr[4] : shstr[4] + shstr[5]]
by_name = {cstr(names, header[0]): (i, header) for i, header in enumerate(headers)}
text_index, text = by_name[".text"]
base, size = text[4], text[5]
blob = bytes(data[base : base + size])
if size != OWNED_SIZE or hashlib.sha256(blob).hexdigest() != EXPECTED:
    raise SystemExit("natural text drift")

words = list(struct.unpack(">132I", blob))
if words[129:] != [0, 0, 0]:
    raise SystemExit("expected exactly three natural alignment words")
expected_head = {
    0x00: 0x27BDFFD0,
    0x04: 0x3C020000,
    0x08: 0x3C010000,
    0x0C: 0x24420000,
    0x10: 0xAC200000,
    0x28: 0xAFB10018,
}
for offset, expected in expected_head.items():
    if words[offset // 4] != expected:
        raise SystemExit(f"head drift at {offset:#x}")

# Preserve every natural semantic operation. Replace the compiler's folded
# base+4 spelling with the complete equivalent stateIndex=1 carrier, consuming
# only the three existing alignment words as executable ownership.
head = [
    words[0],
    words[0x28 // 4],
    0x24110001,
    0x3C0F0000,
    words[0x08 // 4],
    0x25EF0000,
    0x00117080,
    words[0x10 // 4],
    0x01CF1021,
    words[0x14 // 4],
    words[0x18 // 4],
    words[0x1C // 4],
    words[0x20 // 4],
    words[0x24 // 4],
    words[0x2C // 4],
    (words[0x30 // 4] & 0xFFFF0000) | 0x0004,
    (words[0x34 // 4] & 0xFFFF0000) | 0x0008,
    (words[0x38 // 4] & 0xFFFF0000) | 0x000C,
    (words[0x3C // 4] & 0xFFFF0000) | 0x0000,
]
prepared_words = head + words[0x40 // 4 : 129]
if len(prepared_words) != 132:
    raise SystemExit("prepared instruction inventory drift")
data[base : base + OWNED_SIZE] = struct.pack(">132I", *prepared_words)

_, reltext = by_name[".rel.text"]
relocs = []
first_map = {0x04: 0x0C, 0x08: 0x10, 0x0C: 0x14, 0x10: 0x1C}
for pos in range(reltext[4], reltext[4] + reltext[5], reltext[9] or 8):
    offset = struct.unpack_from(">I", data, pos)[0]
    relocs.append(offset)
    if offset in first_map:
        new_offset = first_map[offset]
    elif offset >= 0x40:
        new_offset = offset + 0x0C
    else:
        raise SystemExit(f"unexpected relocation offset {offset:#x}")
    struct.pack_into(">I", data, pos, new_offset)
if tuple(relocs) != EXPECTED_RELOCS:
    raise SystemExit(f"relocation surface drift: {relocs}")

found = 0
for header in headers:
    if header[1] != 2:
        continue
    strings_header = headers[header[6]]
    strings = data[strings_header[4] : strings_header[4] + strings_header[5]]
    entsize = header[9] or 16
    for pos in range(header[4], header[4] + header[5], entsize):
        name_off, value, symbol_size = struct.unpack_from(">III", data, pos)
        shndx = struct.unpack_from(">H", data, pos + 14)[0]
        name = cstr(strings, name_off)
        if name == SYMBOL:
            if (value, symbol_size, shndx) != (0, NATURAL_FUNCTION_SIZE, text_index):
                raise SystemExit("owned function symbol drift")
            struct.pack_into(">I", data, pos + 8, OWNED_SIZE)
            found += 1
if found != 1:
    raise SystemExit("owned function symbol missing")

path.write_bytes(data)
