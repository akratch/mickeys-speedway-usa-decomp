#!/usr/bin/env python3
"""Remove five proved input-home artifacts from the O41 +1464 natural web."""

import hashlib
import pathlib
import struct
import sys

EXPECTED = "a4b51a24fc8f13af4968c7492a338a758e836535f91b5537b394e3ea9f4545f7"
SYMBOL = "func_overlay_041_F0001464_188879C"
OLD_SIZE = 0x200
NEW_SIZE = 0x1EC
DELETE = (0x30, 0x90, 0xA8, 0xB0, 0x140)
EXPECTED_DELETE = {
    0x30: 0x00000000,
    0x90: 0x8FAD0048,
    0xA8: 0x8FB80048,
    0xB0: 0x8FB80048,
    0x140: 0x8FA70048,
}
EXPECTED_RELOCS = (0xD8, 0x160, 0x180, 0x1BC, 0x1E0)

if len(sys.argv) != 2:
    raise SystemExit("usage: prepare.py OBJECT")
path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit("expected big-endian ELF32")

shoff = struct.unpack_from(">I", data, 0x20)[0]
stride = struct.unpack_from(">H", data, 0x2E)[0]
count = struct.unpack_from(">H", data, 0x30)[0]
shstrndx = struct.unpack_from(">H", data, 0x32)[0]
headers = [
    struct.unpack_from(">10I", data, shoff + i * stride) for i in range(count)
]
shstr = headers[shstrndx]
names = data[shstr[4] : shstr[4] + shstr[5]]


def section_name(header):
    end = names.find(b"\0", header[0])
    return names[header[0] : end].decode()


by_name = {section_name(header): (i, header) for i, header in enumerate(headers)}
text_index, text = by_name[".text"]
base, size = text[4], text[5]
blob = bytes(data[base : base + size])
if size != OLD_SIZE or hashlib.sha256(blob).hexdigest() != EXPECTED:
    raise SystemExit("natural text drift")

words = list(struct.unpack(">" + "I" * (OLD_SIZE // 4), blob))
for offset, expected in EXPECTED_DELETE.items():
    if words[offset // 4] != expected:
        raise SystemExit(f"delete-site drift at {offset:#x}")
if words[0x10 // 4] != 0xAFA40048:
    raise SystemExit("input-home producer drift")
words[0x10 // 4] = 0x00803825

kept_offsets = [
    offset for offset in range(0, OLD_SIZE, 4) if offset not in DELETE
]
new_offset = {old: index * 4 for index, old in enumerate(kept_offsets)}
prepared = b"".join(struct.pack(">I", words[old // 4]) for old in kept_offsets)
if len(prepared) != NEW_SIZE:
    raise SystemExit("prepared size drift")
data[base : base + NEW_SIZE] = prepared
data[base + NEW_SIZE : base + OLD_SIZE] = b"\0" * (OLD_SIZE - NEW_SIZE)

_, reltext = by_name[".rel.text"]
relocs = []
for pos in range(reltext[4], reltext[4] + reltext[5], reltext[9] or 8):
    offset, info = struct.unpack_from(">II", data, pos)
    relocs.append(offset)
    if offset not in new_offset:
        raise SystemExit(f"unexpected relocation offset {offset:#x}")
    struct.pack_into(">I", data, pos, new_offset[offset])
if tuple(relocs) != EXPECTED_RELOCS:
    raise SystemExit(f"relocation surface drift: {relocs}")

function_matches = []
section_matches = []
for header in headers:
    if header[1] != 2:
        continue
    strings_header = headers[header[6]]
    strings = data[strings_header[4] : strings_header[4] + strings_header[5]]
    entsize = header[9] or 16
    for pos in range(header[4], header[4] + header[5], entsize):
        name_off, value, sym_size = struct.unpack_from(">III", data, pos)
        info = data[pos + 12]
        shndx = struct.unpack_from(">H", data, pos + 14)[0]
        end = strings.find(b"\0", name_off)
        name = strings[name_off:end].decode()
        if name == SYMBOL:
            function_matches.append((pos, value, sym_size, shndx))
        if (info & 0xF) == 3 and shndx == text_index:
            section_matches.append((pos, value, sym_size))
if len(function_matches) != 1 or function_matches[0][1:] != (
    0,
    OLD_SIZE,
    text_index,
):
    raise SystemExit("function symbol drift")
if len(section_matches) != 1 or section_matches[0][1:] != (0, OLD_SIZE):
    raise SystemExit("section symbol drift")
struct.pack_into(">I", data, function_matches[0][0] + 8, NEW_SIZE)
struct.pack_into(">I", data, section_matches[0][0] + 8, NEW_SIZE)
path.write_bytes(data)
