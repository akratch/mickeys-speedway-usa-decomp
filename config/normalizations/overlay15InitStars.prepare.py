#!/usr/bin/env python3
"""Fail-loud capture of O15 +06E8's two-word representation deficit."""

import hashlib
import pathlib
import struct
import sys

EXPECTED_TEXT = "3b066e44f6d7a422dcaa746ec9adb44a619bd9cf8ba554e36f2bc4648c36d6b2"
SYMBOL = "overlay15InitStars"
OLD_SIZE = 0x2F0
FUNCTION_SIZE = 0x2EC
NEW_SIZE = 0x2F8


def fail(message):
    raise SystemExit(message)


if len(sys.argv) != 2:
    fail("usage: prepare_candidate.py OBJECT")
path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    fail("expected big-endian ELF32")

old_shoff = struct.unpack_from(">I", data, 0x20)[0]
shentsize = struct.unpack_from(">H", data, 0x2E)[0]
shnum = struct.unpack_from(">H", data, 0x30)[0]
shstrndx = struct.unpack_from(">H", data, 0x32)[0]
headers = [list(struct.unpack_from(">10I", data, old_shoff + i * shentsize))
           for i in range(shnum)]
shstr = headers[shstrndx]
names = data[shstr[4]:shstr[4] + shstr[5]]


def section_name(header):
    end = names.find(b"\0", header[0])
    return names[header[0]:end].decode("ascii")


text_index = next(i for i, h in enumerate(headers) if section_name(h) == ".text")
text_header = headers[text_index]
text_offset, text_size = text_header[4], text_header[5]
if text_size != OLD_SIZE:
    fail(f"natural .text size drift: {text_size:#x}")
natural = bytes(data[text_offset:text_offset + OLD_SIZE])
if hashlib.sha256(natural).hexdigest() != EXPECTED_TEXT:
    fail("candidate_v11 natural text drift")

insert_at = text_offset + OLD_SIZE
growth = NEW_SIZE - OLD_SIZE
data[insert_at:insert_at] = b"\0" * growth
new_shoff = old_shoff + growth
struct.pack_into(">I", data, 0x20, new_shoff)
for i, header in enumerate(headers):
    if i == text_index:
        header[5] = NEW_SIZE
    elif header[1] != 8 and header[4] >= insert_at:
        header[4] += growth
    struct.pack_into(">10I", data, new_shoff + i * shentsize, *header)

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
if len(matches) != 1 or matches[0][1:] != (0, FUNCTION_SIZE, text_index):
    fail(f"unexpected function symbol identity: {matches}")
struct.pack_into(">I", data, matches[0][0] + 8, NEW_SIZE)
path.write_bytes(data)
