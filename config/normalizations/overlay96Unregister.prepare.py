#!/usr/bin/env python3
"""Guard and expand the complete o96 unregister semantic topology."""

import hashlib
import pathlib
import struct
import sys

EXPECTED = "fa7cba4957a2e6bc4ce43172ca4d7533e5d177e011bded1b1e0fa95727b722dd"
SYMBOL = "overlay96Unregister"
NATURAL_SECTION = 0x80
NATURAL_FUNCTION = 0x7C
OWNED_SIZE = 0x88
SECTION_SIZE = 0x90


if len(sys.argv) != 2:
    raise SystemExit("usage: prepare_v01.py OBJECT")
path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit("expected big-endian ELF32")
old_shoff = struct.unpack_from(">I", data, 0x20)[0]
phoff = struct.unpack_from(">I", data, 0x1C)[0]
phnum = struct.unpack_from(">H", data, 0x2C)[0]
entsize = struct.unpack_from(">H", data, 0x2E)[0]
count = struct.unpack_from(">H", data, 0x30)[0]
names_index = struct.unpack_from(">H", data, 0x32)[0]
if phoff != 0 or phnum != 0 or entsize != 40:
    raise SystemExit("unexpected relocatable ELF layout")
headers = [struct.unpack_from(">10I", data, old_shoff + i * entsize)
           for i in range(count)]
names_header = headers[names_index]
names = data[names_header[4]:names_header[4] + names_header[5]]


def section_name(header):
    end = names.find(b"\0", header[0])
    return names[header[0]:end].decode()


matches = [(index, header) for index, header in enumerate(headers)
           if section_name(header) == ".text"]
if len(matches) != 1:
    raise SystemExit("expected one .text section")
text_index, text = matches[0]
base, size = text[4], text[5]
body = bytes(data[base:base + size])
if size != NATURAL_SECTION or hashlib.sha256(body).hexdigest() != EXPECTED:
    raise SystemExit("candidate_v01 natural text drift")
if struct.unpack_from(">I", data, base + 0x30)[0] != 0x0003102B:
    raise SystemExit("unexpected sltu counter site")
if struct.unpack_from(">I", data, base + 0x68)[0] != 0x2484FFFC:
    raise SystemExit("unexpected cursor decrement site")
if struct.unpack_from(">I", data, base + 0x7C)[0] != 0:
    raise SystemExit("expected compiler padding at +0x7c")
struct.pack_into(">I", data, base + 0x30, 0x3C090000)
struct.pack_into(">I", data, base + 0x68, 0x24840000)
struct.pack_into(">I", data, base + 0x7C, 0x00001025)

insert_at = base + size
growth = SECTION_SIZE - size
data[insert_at:insert_at] = b"\0" * growth
new_shoff = old_shoff + growth if old_shoff >= insert_at else old_shoff
struct.pack_into(">I", data, 0x20, new_shoff)
for index, header in enumerate(headers):
    values = list(header)
    if index == text_index:
        values[5] = SECTION_SIZE
    elif values[1] != 8 and values[4] >= insert_at and values[4] != 0:
        values[4] += growth
    struct.pack_into(">10I", data, new_shoff + index * entsize, *values)

# Two address-independent ADDIU producers complete the retained inventory.
struct.pack_into(">I", data, base + 0x80, 0x24080000)
struct.pack_into(">I", data, base + 0x84, 0x240A0000)
if data[base + OWNED_SIZE:base + SECTION_SIZE] != b"\0" * 8:
    raise SystemExit("padding drift")

new_headers = [struct.unpack_from(">10I", data, new_shoff + i * entsize)
               for i in range(count)]
symbol_matches = []
for header in new_headers:
    if header[1] != 2:
        continue
    strings_header = new_headers[header[6]]
    strings = data[strings_header[4]:strings_header[4] + strings_header[5]]
    symbol_size = header[9] or 16
    for position in range(header[4], header[4] + header[5], symbol_size):
        name_offset, value, size = struct.unpack_from(">III", data, position)
        end = strings.find(b"\0", name_offset)
        if strings[name_offset:end].decode() == SYMBOL:
            symbol_matches.append((position, value, size))
if len(symbol_matches) != 1 or symbol_matches[0][1:] != (0, NATURAL_FUNCTION):
    raise SystemExit("unexpected function symbol")
struct.pack_into(">I", data, symbol_matches[0][0] + 8, OWNED_SIZE)
path.write_bytes(data)
