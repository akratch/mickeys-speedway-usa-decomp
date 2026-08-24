#!/usr/bin/env python3
"""Drop one proved redundant count-address materialization from O12 +02E4."""

import hashlib
import pathlib
import struct
import sys


EXPECTED = "70efbb9b8f583e83ea5a8b21b8a5b5b7029406dc938529b5fa6a0820ba258cf0"
SYMBOL = "func_overlay_012_F00002E4_186D564"
OLD_SECTION = 0xD0
OLD_FUNCTION = 0xC8
NEW_FUNCTION = 0xC4
DROP = 0x8C
EXPECTED_RELOCS = (0x24, 0x2C, 0x30, 0x38, 0x3C)


def fail(message):
    raise SystemExit(message)


if len(sys.argv) != 2:
    fail("usage: overlay12SpawnParticle.prepare.py OBJECT")

path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    fail("expected big-endian ELF32")

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
if size != OLD_SECTION or hashlib.sha256(blob).hexdigest() != EXPECTED:
    fail("natural text drift")

words = list(struct.unpack(">52I", blob))
if words[DROP // 4] != 0x3C010000:
    fail("redundant LUI drift")


def translate(offset):
    return offset - 4 if offset > DROP else offset


new_words = []
for old, word in ((i * 4, value) for i, value in enumerate(words)):
    if old == DROP:
        continue
    opcode = word >> 26
    if opcode in (1, 4, 5, 6, 7, 20, 21, 22, 23):
        immediate = word & 0xFFFF
        if immediate & 0x8000:
            immediate -= 0x10000
        target = old + 4 + immediate * 4
        now = translate(old)
        new_target = translate(target)
        word = (word & 0xFFFF0000) | (((new_target - (now + 4)) // 4) & 0xFFFF)
    new_words.append(word)

if len(new_words) != 51 or new_words[-2:] != [0, 0]:
    fail("padding/topology drift")
prepared = struct.pack(">51I", *new_words)
data[base : base + 0xCC] = prepared
data[base + 0xCC : base + OLD_SECTION] = b"\0" * 4

_, rel = by_name[".rel.text"]
seen = []
for position in range(rel[4], rel[4] + rel[5], rel[9] or 8):
    offset = struct.unpack_from(">I", data, position)[0]
    seen.append(offset)
    if offset == DROP:
        fail("dropped relocation survived filter")
    struct.pack_into(">I", data, position, translate(offset))
if tuple(seen) != EXPECTED_RELOCS:
    fail(f"relocation drift: {seen}")

functions = []
section_symbols = []
for header in headers:
    if header[1] != 2:
        continue
    strings_header = headers[header[6]]
    strings = data[
        strings_header[4] : strings_header[4] + strings_header[5]
    ]
    entry_size = header[9] or 16
    for position in range(header[4], header[4] + header[5], entry_size):
        name_offset, value, symbol_size = struct.unpack_from(">III", data, position)
        info = data[position + 12]
        section_index = struct.unpack_from(">H", data, position + 14)[0]
        end = strings.find(b"\0", name_offset)
        name = strings[name_offset:end].decode()
        if name == SYMBOL:
            functions.append((position, value, symbol_size, section_index))
        if (info & 0xF) == 3 and section_index == text_index:
            section_symbols.append((position, value, symbol_size))

if len(functions) != 1 or functions[0][1:] != (
    0,
    OLD_FUNCTION,
    text_index,
):
    fail("function symbol drift")
if len(section_symbols) != 1 or section_symbols[0][1:] != (0, OLD_SECTION):
    fail("section symbol drift")
struct.pack_into(">I", data, functions[0][0] + 8, NEW_FUNCTION)
struct.pack_into(">I", data, section_symbols[0][0] + 8, NEW_FUNCTION)
path.write_bytes(data)
