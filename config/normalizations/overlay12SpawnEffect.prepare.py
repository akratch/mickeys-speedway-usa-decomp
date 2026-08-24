#!/usr/bin/env python3
"""Apply the proved independent initialization schedule for O12 +01B4."""

import hashlib
import pathlib
import struct
import sys

EXPECTED = "6d57e3f749f00a1ae4bd9c2ec43d22dbf95a9de7b557df554150f3678d6065bd"
SIZE = 0x130

# Destination offset -> natural source offset. This is a bijection over the
# straight-line record initializer; no branch, call, relocation, or delay slot
# moves.
MAP = {
    0x70: 0x70,
    0x74: 0x74,
    0x78: 0x78,
    0x7C: 0x7C,
    0x80: 0x80,
    0x84: 0x84,
    0x88: 0x88,
    0x8C: 0x8C,
    0x90: 0xAC,
    0x94: 0xA4,
    0x98: 0xA8,
    0x9C: 0x94,
    0xA0: 0x90,
    0xA4: 0x9C,
    0xA8: 0x98,
    0xAC: 0xA0,
    0xB0: 0xB4,
    0xB4: 0xB8,
    0xB8: 0xC0,
    0xBC: 0xBC,
    0xC0: 0xC4,
    0xC4: 0xC8,
    0xC8: 0xB0,
    0xCC: 0xCC,
}

if len(sys.argv) != 2:
    raise SystemExit("usage: overlay12SpawnEffect.prepare.py OBJECT")

path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit("expected big-endian ELF32")

section_offset = struct.unpack_from(">I", data, 0x20)[0]
section_stride = struct.unpack_from(">H", data, 0x2E)[0]
section_count = struct.unpack_from(">H", data, 0x30)[0]
string_index = struct.unpack_from(">H", data, 0x32)[0]
headers = [
    struct.unpack_from(">10I", data, section_offset + i * section_stride)
    for i in range(section_count)
]
string_header = headers[string_index]
names = data[
    string_header[4] : string_header[4] + string_header[5]
]


def section_name(header):
    end = names.find(b"\0", header[0])
    return names[header[0] : end].decode()


text = next(header for header in headers if section_name(header) == ".text")
base, size = text[4], text[5]
blob = bytes(data[base : base + size])
if size != SIZE or hashlib.sha256(blob).hexdigest() != EXPECTED:
    raise SystemExit("natural text drift")
if set(MAP) != set(range(0x70, 0xD0, 4)) or set(MAP.values()) != set(MAP):
    raise SystemExit("schedule is not bijective")

words = list(struct.unpack(">76I", blob))
before = words[:]
for destination, source in MAP.items():
    words[destination // 4] = before[source // 4]

# Prove every word outside the private initializer is unchanged.
for offset in list(range(0, 0x70, 4)) + list(range(0xD0, SIZE, 4)):
    if words[offset // 4] != before[offset // 4]:
        raise SystemExit("non-private schedule drift")

data[base : base + SIZE] = struct.pack(">76I", *words)
path.write_bytes(data)
