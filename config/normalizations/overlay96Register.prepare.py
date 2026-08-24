#!/usr/bin/env python3
"""Guard the o96 register-helper basin and recover three semantic copies."""

import hashlib
import pathlib
import struct
import sys

EXPECTED = "b2852af4a34585825985b0a362930201873307e40b46e3fb2c82ded1b98371c0"


def sections(data):
    shoff = struct.unpack_from(">I", data, 0x20)[0]
    entsize = struct.unpack_from(">H", data, 0x2E)[0]
    count = struct.unpack_from(">H", data, 0x30)[0]
    names_index = struct.unpack_from(">H", data, 0x32)[0]
    headers = [struct.unpack_from(">10I", data, shoff + i * entsize)
               for i in range(count)]
    names_header = headers[names_index]
    names = data[names_header[4]:names_header[4] + names_header[5]]
    result = {}
    for header in headers:
        end = names.find(b"\0", header[0])
        result[names[header[0]:end].decode()] = header
    return result


if len(sys.argv) != 2:
    raise SystemExit("usage: prepare_v01.py OBJECT")
path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit("expected big-endian ELF32")
text = sections(data)[".text"]
base, size = text[4], text[5]
body = bytes(data[base:base + size])
if size != 0x70 or hashlib.sha256(body).hexdigest() != EXPECTED:
    raise SystemExit("candidate_v01 natural text drift")

expected_words = {
    0x14: 0x0003282B,  # sltu a1,zero,v1
    0x24: 0x3C020000,  # compiler's second entries base
    0x34: 0x00C22826,  # xor a1,a2,v0
}
replacement_words = {
    0x14: 0x00032825,  # move a1,v1 placeholder
    0x24: 0x00001025,  # move v0,zero placeholder
    0x34: 0x00C22825,  # move a1,a2 placeholder
}
for offset, expected in expected_words.items():
    got = struct.unpack_from(">I", data, base + offset)[0]
    if got != expected:
        raise SystemExit(f"unexpected word at {offset:#x}: {got:08x}")
    struct.pack_into(">I", data, base + offset, replacement_words[offset])
path.write_bytes(data)
