#!/usr/bin/env python3
"""Delete O26 +1158's proved dead post-load pointer increment."""

import hashlib
import pathlib
import struct
import sys

EXPECTED = "0de3ff0a1e573713bad932749f89094bbac8729a50cbbdccd79138aea294b17d"
SYMBOL = "func_overlay_026_F0001158_187B550"
OLD_TEXT_SIZE = 0x220
OLD_FUNCTION_SIZE = 0x21C
NEW_FUNCTION_SIZE = 0x218
DELETE = 0xE4
EXPECTED_DELETE = 0x24420001
EXPECTED_RELOCS = (0x108, 0x148, 0x1A4, 0x1C0)

if len(sys.argv) != 2:
    raise SystemExit("usage: overlay26DrawGroups.prepare.py OBJECT")
path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit("expected big-endian ELF32")

shoff = struct.unpack_from(">I", data, 0x20)[0]
stride = struct.unpack_from(">H", data, 0x2E)[0]
count = struct.unpack_from(">H", data, 0x30)[0]
shstrndx = struct.unpack_from(">H", data, 0x32)[0]
headers = [struct.unpack_from(">10I", data, shoff + i * stride)
           for i in range(count)]
shstr = headers[shstrndx]
names = data[shstr[4]:shstr[4] + shstr[5]]


def section_name(header):
    end = names.find(b"\0", header[0])
    return names[header[0]:end].decode()


by_name = {section_name(header): (index, header)
           for index, header in enumerate(headers)}
text_index, text = by_name[".text"]
base, size = text[4], text[5]
blob = bytes(data[base:base + size])
if size != OLD_TEXT_SIZE or hashlib.sha256(blob).hexdigest() != EXPECTED:
    raise SystemExit("natural text drift")

words = list(struct.unpack(">" + "I" * (OLD_TEXT_SIZE // 4), blob))
if words[DELETE // 4] != EXPECTED_DELETE:
    raise SystemExit("delete-site drift")
# The deleted addiu writes v0 after the sole load through v0. No subsequent
# instruction reads v0 before the first call, which independently defines it.
if words[0xDC // 4] != 0x014B1021 or words[0xE0 // 4] != 0x8C530011:
    raise SystemExit("effective-address producer/consumer drift")

kept = [offset for offset in range(0, OLD_TEXT_SIZE, 4)
        if offset != DELETE]
new_offset = {old: index * 4 for index, old in enumerate(kept)}
prepared = b"".join(struct.pack(">I", words[old // 4]) for old in kept)
if len(prepared) != OLD_TEXT_SIZE - 4:
    raise SystemExit("prepared size drift")
data[base:base + len(prepared)] = prepared
data[base + len(prepared):base + OLD_TEXT_SIZE] = b"\0" * 4

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
    strings = data[strings_header[4]:strings_header[4] + strings_header[5]]
    entry_size = header[9] or 16
    for pos in range(header[4], header[4] + header[5], entry_size):
        name_off, value, symbol_size = struct.unpack_from(">III", data, pos)
        info = data[pos + 12]
        shndx = struct.unpack_from(">H", data, pos + 14)[0]
        end = strings.find(b"\0", name_off)
        name = strings[name_off:end].decode()
        if name == SYMBOL:
            function_matches.append((pos, value, symbol_size, shndx))
        if (info & 0xF) == 3 and shndx == text_index:
            section_matches.append((pos, value, symbol_size))
if len(function_matches) != 1 or function_matches[0][1:] != (
        0, OLD_FUNCTION_SIZE, text_index):
    raise SystemExit("function symbol drift")
if len(section_matches) != 1 or section_matches[0][1:] != (
        0, OLD_TEXT_SIZE):
    raise SystemExit("section symbol drift")
struct.pack_into(">I", data, function_matches[0][0] + 8, NEW_FUNCTION_SIZE)
struct.pack_into(">I", data, section_matches[0][0] + 8, NEW_FUNCTION_SIZE)
path.write_bytes(data)
