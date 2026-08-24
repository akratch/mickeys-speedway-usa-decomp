#!/usr/bin/env python3
"""Fail loudly while converting IDO's 0x3E0 alignment to the 0x3DC owner."""

import hashlib
import pathlib
import struct
import sys

EXPECTED_OWNED = "c0d678e621f5a53e0a9b0fa385ea4c718acd6440dcc0a0760aadf65c08bc5f58"
SYMBOL = "overlay15InitStarsAndPalette"
OLD_SECTION_SIZE = 0x3E0
OLD_FUNCTION_SIZE = 0x3D8
NEW_SIZE = 0x3DC


def fail(message):
    raise SystemExit(message)


if len(sys.argv) != 2:
    fail("usage: finalize_owned_section.py OBJECT")
path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    fail("expected big-endian ELF32")

shoff = struct.unpack_from(">I", data, 0x20)[0]
shentsize = struct.unpack_from(">H", data, 0x2E)[0]
shnum = struct.unpack_from(">H", data, 0x30)[0]
shstrndx = struct.unpack_from(">H", data, 0x32)[0]
headers = [list(struct.unpack_from(">10I", data, shoff + i * shentsize))
           for i in range(shnum)]
shstr = headers[shstrndx]
names = data[shstr[4]:shstr[4] + shstr[5]]


def section_name(header):
    end = names.find(b"\0", header[0])
    return names[header[0]:end].decode("ascii")


text_index = next(i for i, h in enumerate(headers) if section_name(h) == ".text")
text_header = headers[text_index]
text_offset, text_size = text_header[4], text_header[5]
if text_size != OLD_SECTION_SIZE:
    fail(f"natural/normalized .text size drift: {text_size:#x}")
owned = bytes(data[text_offset:text_offset + NEW_SIZE])
if hashlib.sha256(owned).hexdigest() != EXPECTED_OWNED:
    fail("owned text drift")
if data[text_offset + NEW_SIZE:text_offset + OLD_SECTION_SIZE] != b"\0" * 4:
    fail("expected one trailing IDO alignment nop")

text_header[5] = NEW_SIZE
struct.pack_into(">10I", data, shoff + text_index * shentsize, *text_header)

matches = []
for header in headers:
    if header[1] != 2:
        continue
    strings = headers[header[6]]
    string_data = data[strings[4]:strings[4] + strings[5]]
    entsize = header[9] or 16
    for entry in range(header[4], header[4] + header[5], entsize):
        name_off, value, size = struct.unpack_from(">III", data, entry)
        end = string_data.find(b"\0", name_off)
        if string_data[name_off:end].decode("ascii") == SYMBOL:
            matches.append((entry, value, size,
                            struct.unpack_from(">H", data, entry + 14)[0]))
if len(matches) != 1 or matches[0][1:] != (0, OLD_FUNCTION_SIZE, text_index):
    fail(f"unexpected function symbol identity: {matches}")
struct.pack_into(">I", data, matches[0][0] + 8, NEW_SIZE)
path.write_bytes(data)
