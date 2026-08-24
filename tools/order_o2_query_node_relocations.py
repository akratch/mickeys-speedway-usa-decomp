#!/usr/bin/env python3
"""Install the exact retail static REL record order for O2 +0x16A0."""

from pathlib import Path
import struct
import sys

if len(sys.argv) != 2:
    raise SystemExit("usage: order_o2_query_node_relocations.py OBJECT")

path = Path(sys.argv[1])
data = bytearray(path.read_bytes())
assert data[:6] == b"\x7fELF\x01\x02"
shoff = struct.unpack_from(">I", data, 0x20)[0]
entsize = struct.unpack_from(">H", data, 0x2E)[0]
count = struct.unpack_from(">H", data, 0x30)[0]
shstrndx = struct.unpack_from(">H", data, 0x32)[0]
headers = [
    struct.unpack_from(">10I", data, shoff + i * entsize) for i in range(count)
]
shstr = headers[shstrndx]
strings = data[shstr[4] : shstr[4] + shstr[5]]


def section_name(offset):
    end = strings.find(b"\0", offset)
    return strings[offset:end].decode("ascii")


text_index = next(
    i for i, header in enumerate(headers) if section_name(header[0]) == ".text"
)
rels = [header for header in headers if header[1] == 9 and header[7] == text_index]
assert len(rels) == 1
rel = rels[0]
entries = [
    bytes(data[offset : offset + 8])
    for offset in range(rel[4], rel[4] + rel[5], 8)
]

order = [
    0x68,
    0x6C,
    0x64,
    0x70,
    0x60,
    0x74,
    0xC4,
    0x1B4,
    0x1B8,
    0x1D4,
    0x1D8,
    0x1C0,
    0x1E4,
    0x218,
    0x21C,
    0x25C,
    0x294,
    0x2CC,
    0x2EC,
    0x30C,
    0x334,
    0x360,
    0x380,
    0x3A0,
    0x3C8,
]
offsets = [struct.unpack_from(">I", entry)[0] for entry in entries]
if sorted(offsets) != sorted(order) or len(offsets) != len(order):
    raise SystemExit(f"unexpected configured relocation offsets: {offsets!r}")
rank = {offset: index for index, offset in enumerate(order)}
entries.sort(key=lambda entry: rank[struct.unpack_from(">I", entry)[0]])
data[rel[4] : rel[4] + rel[5]] = b"".join(entries)
path.write_bytes(data)
