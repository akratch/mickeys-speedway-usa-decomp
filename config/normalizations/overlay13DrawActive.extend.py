#!/usr/bin/env python3
"""Fail-loudly assign O13 +0874's natural alignment nop to the owner."""
import pathlib
import struct
import sys

path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit("expected big-endian ELF32")
shoff = struct.unpack_from(">I", data, 0x20)[0]
entsize = struct.unpack_from(">H", data, 0x2E)[0]
count = struct.unpack_from(">H", data, 0x30)[0]
shstrndx = struct.unpack_from(">H", data, 0x32)[0]
headers = [struct.unpack_from(">10I", data, shoff + i * entsize)
           for i in range(count)]
shstr = headers[shstrndx]
names = data[shstr[4]:shstr[4] + shstr[5]]

def name(header):
    end = names.find(b"\0", header[0])
    return names[header[0]:end].decode("ascii")

text_index = next(i for i, header in enumerate(headers)
                  if name(header) == ".text")
if headers[text_index][5] != 0x298:
    raise SystemExit("unexpected .text size")
symbol = "overlay13DrawActive"
matches = []
for header in headers:
    if header[1] != 2:
        continue
    strings = headers[header[6]]
    blob = data[strings[4]:strings[4] + strings[5]]
    step = header[9] or 16
    for offset in range(header[4], header[4] + header[5], step):
        name_offset, value, size = struct.unpack_from(">III", data, offset)
        end = blob.find(b"\0", name_offset)
        if blob[name_offset:end].decode("ascii") == symbol:
            matches.append((offset, value, size,
                            struct.unpack_from(">H", data, offset + 14)[0]))
if len(matches) != 1 or matches[0][1:] != (0, 0x294, text_index):
    raise SystemExit(f"unexpected function identity: {matches}")
struct.pack_into(">I", data, matches[0][0] + 8, 0x298)
path.write_bytes(data)
