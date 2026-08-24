#!/usr/bin/env python3
"""Sort only the asserted O52 +0540 six-record ELF32 REL table."""

import pathlib
import struct
import sys

if len(sys.argv) != 2:
    raise SystemExit("usage: sort_relocations.py OBJECT")
path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit("expected big-endian ELF32")
shoff = struct.unpack_from(">I", data, 0x20)[0]
shentsize = struct.unpack_from(">H", data, 0x2E)[0]
shnum = struct.unpack_from(">H", data, 0x30)[0]
shstrndx = struct.unpack_from(">H", data, 0x32)[0]
headers = [struct.unpack_from(">10I", data, shoff + i * shentsize)
           for i in range(shnum)]
shstr = headers[shstrndx]
strings = data[shstr[4]:shstr[4] + shstr[5]]

def section_name(offset):
    end = strings.find(b"\0", offset)
    if end < 0:
        raise SystemExit("unterminated section name")
    return strings[offset:end].decode("ascii")

text = [i for i, sh in enumerate(headers) if section_name(sh[0]) == ".text"]
if len(text) != 1:
    raise SystemExit("expected one .text")
rels = [sh for sh in headers if sh[1] == 9 and sh[7] == text[0]]
if len(rels) != 1 or (rels[0][9] or 8) != 8 or rels[0][5] != 48:
    raise SystemExit("expected exactly six packed .rel.text records")
rel = rels[0]
start = rel[4]
entries = [bytes(data[p:p + 8]) for p in range(start, start + 48, 8)]
offsets = [struct.unpack_from(">I", row)[0] for row in entries]
expected_natural = [0x18, 0x40, 0x74, 0x78, 0x64, 0x68]
if offsets != expected_natural:
    raise SystemExit(f"natural relocation order drift: {offsets!r}")
entries.sort(key=lambda row: struct.unpack_from(">I", row)[0])
if [struct.unpack_from(">I", row)[0] for row in entries] != [
        0x18, 0x40, 0x64, 0x68, 0x74, 0x78]:
    raise SystemExit("sorted relocation order drift")
data[start:start + 48] = b"".join(entries)
path.write_bytes(data)
