#!/usr/bin/env python3
"""Install Overlay 14 reset's assembler-authored REL record order."""

import pathlib
import struct
import sys


if len(sys.argv) != 2:
    raise SystemExit("usage: order_o14_reset_relocations.py OBJECT")

path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit("expected big-endian ELF32")

shoff = struct.unpack_from(">I", data, 0x20)[0]
entsize = struct.unpack_from(">H", data, 0x2E)[0]
count = struct.unpack_from(">H", data, 0x30)[0]
shstrndx = struct.unpack_from(">H", data, 0x32)[0]
headers = [
    struct.unpack_from(">10I", data, shoff + index * entsize)
    for index in range(count)
]
shstr = headers[shstrndx]
strings = data[shstr[4] : shstr[4] + shstr[5]]


def section_name(offset):
    end = strings.find(b"\0", offset)
    return strings[offset:end].decode("ascii")


text_index = next(
    index for index, header in enumerate(headers)
    if section_name(header[0]) == ".text"
)
rel_sections = [
    header for header in headers if header[1] == 9 and header[7] == text_index
]
if len(rel_sections) != 1:
    raise SystemExit("expected one .text REL table")

rel = rel_sections[0]
entries = [
    bytes(data[offset : offset + 8])
    for offset in range(rel[4], rel[4] + rel[5], 8)
]
order = [0x14, 0x4C, 0x64, 0x74, 0x78, 0x70, 0x7C, 0x6C, 0x80, 0x9C]
offsets = [struct.unpack_from(">I", entry)[0] for entry in entries]
if sorted(offsets) != sorted(order) or len(offsets) != len(order):
    raise SystemExit(f"unexpected configured relocation offsets: {offsets!r}")

rank = {offset: index for index, offset in enumerate(order)}
entries.sort(key=lambda entry: rank[struct.unpack_from(">I", entry)[0]])
data[rel[4] : rel[4] + rel[5]] = b"".join(entries)
path.write_bytes(data)
