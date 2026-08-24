#!/usr/bin/env python3
"""Drop three asserted IDO-only invariant-home words from O34 update."""

import hashlib
import pathlib
import struct
import sys

EXPECTED = "4594193547aaa1ea1566fc2d02ddb4b54fb4c68f4908a2bde33fe94411ae1320"
SYMBOL = "overlay34UpdateRecords"
NATURAL_SIZE = 0x140
OWNED_SIZE = 0x134
DROPS = (0x14, 0x60, 0x130)

if len(sys.argv) != 2:
    raise SystemExit("usage: prepare_update.py OBJECT")
path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit("expected big-endian ELF32")

shoff = struct.unpack_from(">I", data, 0x20)[0]
entsize = struct.unpack_from(">H", data, 0x2E)[0]
count = struct.unpack_from(">H", data, 0x30)[0]
names_index = struct.unpack_from(">H", data, 0x32)[0]
headers = [list(struct.unpack_from(">10I", data, shoff + i * entsize))
           for i in range(count)]
names_header = headers[names_index]
names = data[names_header[4]:names_header[4] + names_header[5]]

def cstr(blob, offset):
    end = blob.find(b"\0", offset)
    return blob[offset:end].decode()

section_names = [cstr(names, h[0]) for h in headers]
if section_names.count(".text") != 1:
    raise SystemExit("expected one .text")
text_index = section_names.index(".text")
text = headers[text_index]
base, size = text[4], text[5]
body = bytes(data[base:base + size])
if size != NATURAL_SIZE or hashlib.sha256(body).hexdigest() != EXPECTED:
    raise SystemExit("natural update text drift")

words = list(struct.unpack(">80I", body))

def fields(word):
    return (word >> 26, (word >> 21) & 31, (word >> 16) & 31,
            (word >> 11) & 31, word & 0xFFFF)

# sw s5,44(sp); coalesced move v1,s5; lw s5,44(sp). The retained natural
# addiu producer is subsequently scheduled into the vacated move site.
expected = {
    0x14: (0x2B, 29, 21, 0, 0x2C),
    0x60: (0x00, 21, 0, 3, 0x1825),
    0x130: (0x23, 29, 21, 0, 0x2C),
}
for offset, wanted in expected.items():
    if fields(words[offset // 4]) != wanted:
        raise SystemExit(f"unexpected dropped instruction at {offset:#x}")

def translate(offset):
    return offset - 4 * sum(drop < offset for drop in DROPS)

new_words = []
for old_offset, word in enumerate(words):
    old_offset *= 4
    if old_offset in DROPS:
        continue
    opcode = word >> 26
    if opcode in (1, 4, 5, 6, 7, 20, 21, 22, 23):
        immediate = word & 0xFFFF
        if immediate & 0x8000:
            immediate -= 0x10000
        old_target = old_offset + 4 + immediate * 4
        new_offset = translate(old_offset)
        new_target = translate(old_target)
        delta = (new_target - (new_offset + 4)) // 4
        if not -0x8000 <= delta < 0x8000:
            raise SystemExit("translated branch out of range")
        word = (word & 0xFFFF0000) | (delta & 0xFFFF)
    new_words.append(word)
if len(new_words) != 77:
    raise SystemExit("drop inventory drift")
new_body = struct.pack(">77I", *new_words) + b"\0" * 12
data[base:base + size] = new_body

# Move every text relocation with its retained instruction.
for header in headers:
    if header[1] != 9 or header[7] != text_index:
        continue
    entry_size = header[9] or 8
    for pos in range(header[4], header[4] + header[5], entry_size):
        offset = struct.unpack_from(">I", data, pos)[0]
        if offset in DROPS:
            raise SystemExit("dropped instruction owns relocation")
        struct.pack_into(">I", data, pos, translate(offset))

# Translate text-defined symbols and shrink the one owned function.
found = 0
for header in headers:
    if header[1] != 2:
        continue
    strings_header = headers[header[6]]
    strings = data[strings_header[4]:strings_header[4] + strings_header[5]]
    entry_size = header[9] or 16
    for pos in range(header[4], header[4] + header[5], entry_size):
        name_off, value, sym_size = struct.unpack_from(">III", data, pos)
        shndx = struct.unpack_from(">H", data, pos + 14)[0]
        name = cstr(strings, name_off)
        if shndx == text_index and value <= NATURAL_SIZE:
            struct.pack_into(">I", data, pos + 4, translate(value))
        if name == SYMBOL:
            if (value, sym_size, shndx) != (0, NATURAL_SIZE, text_index):
                raise SystemExit("owned function symbol drift")
            struct.pack_into(">I", data, pos + 8, OWNED_SIZE)
            found += 1
if found != 1:
    raise SystemExit("owned function symbol missing")

path.write_bytes(data)
