#!/usr/bin/env python3
"""Put O99 +02A0 configured REL records in retail assembly-owner order."""

import pathlib
import struct
import sys

if len(sys.argv) != 2:
    raise SystemExit("usage: order_o99_apply_segment_relocations.py OBJECT")
path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit("expected big-endian ELF32")
shoff = struct.unpack_from(">I", data, 0x20)[0]
entsize = struct.unpack_from(">H", data, 0x2E)[0]
count = struct.unpack_from(">H", data, 0x30)[0]
shstrndx = struct.unpack_from(">H", data, 0x32)[0]
headers = [
    struct.unpack_from(">10I", data, shoff + i * entsize)
    for i in range(count)
]
shstr = headers[shstrndx]
strings = data[shstr[4] : shstr[4] + shstr[5]]


def name(offset):
    end = strings.find(b"\0", offset)
    return strings[offset:end].decode("ascii")


text_index = next(i for i, h in enumerate(headers) if name(h[0]) == ".text")
rels = [h for h in headers if h[1] == 9 and h[7] == text_index]
if len(rels) != 1:
    raise SystemExit("expected one .text REL table")
rel = rels[0]
entries = [
    bytes(data[p : p + 8]) for p in range(rel[4], rel[4] + rel[5], 8)
]
order = [
    0x128, 0x12C, 0x124, 0x130, 0x120, 0x134, 0x11C, 0x13C,
    0x1B0, 0x220, 0x244, 0x2AC, 0x2D8,
]
offsets = [struct.unpack_from(">I", entry)[0] for entry in entries]
if sorted(offsets) != sorted(order) or len(offsets) != len(order):
    raise SystemExit(f"unexpected configured relocation offsets: {offsets!r}")
rank = {offset: index for index, offset in enumerate(order)}
entries.sort(key=lambda entry: rank[struct.unpack_from(">I", entry)[0]])
data[rel[4] : rel[4] + rel[5]] = b"".join(entries)
path.write_bytes(data)
