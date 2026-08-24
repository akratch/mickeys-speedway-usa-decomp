#!/usr/bin/env python3
"""Remove one proven IDO scheduling NOP from O101 +0xC6E8."""

import hashlib
import pathlib
import struct
import sys

EXPECTED = "4c0f06fd80f56c0f2f2f95bcb609893ebdfdfb84bbba66dc7acb5f9bb14ecd8e"
SYMBOL = "func_overlay_101_F000C6E8_18E7F08"
NATURAL_SIZE = 0x500
FUNCTION_SIZE = 0x4F8
OWNED_SIZE = 0x4F4
DROP = 0x18


def cstr(blob, offset):
    end = blob.find(b"\0", offset)
    return blob[offset:end].decode()


if len(sys.argv) != 2:
    raise SystemExit("usage: prepare_semantic_web.py OBJECT")
path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit("expected big-endian ELF32")

shoff = struct.unpack_from(">I", data, 0x20)[0]
shentsize = struct.unpack_from(">H", data, 0x2E)[0]
shnum = struct.unpack_from(">H", data, 0x30)[0]
shstrndx = struct.unpack_from(">H", data, 0x32)[0]
headers = [list(struct.unpack_from(">10I", data, shoff + i * shentsize))
           for i in range(shnum)]
shstr = headers[shstrndx]
names = data[shstr[4]:shstr[4] + shstr[5]]
section_names = [cstr(names, h[0]) for h in headers]
if section_names.count(".text") != 1:
    raise SystemExit("expected one .text")
text_index = section_names.index(".text")
text = headers[text_index]
base, size = text[4], text[5]
body = bytes(data[base:base + size])
if size != NATURAL_SIZE or hashlib.sha256(body).hexdigest() != EXPECTED:
    raise SystemExit("candidate_v04 natural text drift")

words = list(struct.unpack(">320I", body))
if words[DROP // 4] != 0:
    raise SystemExit("expected sole scheduler NOP at +0x18")


def translate(offset):
    return offset - (4 if offset > DROP else 0)


new_words = []
for old_offset, word in enumerate(words):
    old_offset *= 4
    if old_offset == DROP:
        continue
    opcode = word >> 26
    if opcode in (1, 4, 5, 6, 7, 20, 21, 22, 23):
        immediate = word & 0xFFFF
        if immediate & 0x8000:
            immediate -= 0x10000
        old_target = old_offset + 4 + immediate * 4
        new_offset = translate(old_offset)
        new_target = translate(old_target)
        immediate = (new_target - (new_offset + 4)) // 4
        word = (word & 0xFFFF0000) | (immediate & 0xFFFF)
    new_words.append(word)
if len(new_words) != 319 or new_words[-2:] != [0, 0]:
    raise SystemExit("expected two final alignment words")
prepared = struct.pack(">317I", *new_words[:-2])
data[base:base + OWNED_SIZE] = prepared
data[base + OWNED_SIZE:base + NATURAL_SIZE] = b"\0" * (NATURAL_SIZE - OWNED_SIZE)

for header in headers:
    if header[1] != 9 or header[7] != text_index:
        continue
    entsize = header[9] or 8
    for pos in range(header[4], header[4] + header[5], entsize):
        offset = struct.unpack_from(">I", data, pos)[0]
        if offset == DROP:
            raise SystemExit("dropped NOP unexpectedly owns relocation")
        struct.pack_into(">I", data, pos, translate(offset))

found = 0
for header in headers:
    if header[1] != 2:
        continue
    strings_header = headers[header[6]]
    strings = data[strings_header[4]:strings_header[4] + strings_header[5]]
    entsize = header[9] or 16
    for pos in range(header[4], header[4] + header[5], entsize):
        name_off, value, sym_size = struct.unpack_from(">III", data, pos)
        shndx = struct.unpack_from(">H", data, pos + 14)[0]
        name = cstr(strings, name_off)
        if shndx == text_index and value <= FUNCTION_SIZE:
            struct.pack_into(">I", data, pos + 4, translate(value))
        if name == SYMBOL:
            if (value, sym_size, shndx) != (0, FUNCTION_SIZE, text_index):
                raise SystemExit("owned function symbol drift")
            struct.pack_into(">I", data, pos + 8, OWNED_SIZE)
            found += 1
if found != 1:
    raise SystemExit("owned function symbol missing")

struct.pack_into(">I", data, shoff + text_index * shentsize + 20, OWNED_SIZE)
path.write_bytes(data)
