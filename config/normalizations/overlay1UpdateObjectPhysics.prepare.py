#!/usr/bin/env python3
"""Fail-loud expansion of the natural O1 +438C semantic candidate."""

import hashlib
import pathlib
import struct
import sys

EXPECTED_TEXT = "2891db906ec37aa6761a957a440831e3dabb17064d734397c0dbcf82fc4343ea"
SYMBOL = "func_overlay_001_F000438C_185076C"
OLD_SIZE = 0x17E0
FUNCTION_SIZE = 0x17E0
NEW_SIZE = 0x1818


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
headers = [
    list(struct.unpack_from(">10I", data, old_shoff + i * shentsize))
    for i in range(shnum)
]
shstr = headers[shstrndx]
names = data[shstr[4] : shstr[4] + shstr[5]]


def section_name(header):
    end = names.find(b"\0", header[0])
    return names[header[0] : end].decode("ascii")


text_index = next(i for i, h in enumerate(headers) if section_name(h) == ".text")
text_header = headers[text_index]
text_offset, text_size = text_header[4], text_header[5]
if text_size != OLD_SIZE:
    fail(f"natural .text size drift: {text_size:#x}")
natural = bytes(data[text_offset : text_offset + OLD_SIZE])
if hashlib.sha256(natural).hexdigest() != EXPECTED_TEXT:
    fail("candidate_v12 natural text drift")

# The configured object has one function followed immediately by the next ELF
# section.  Insert fourteen zero instructions as a representation pool: the
# target has thirteen additional architectural nops and one remaining natural
# opcode-class deficit.  The normalizer must consume every position and prove
# the final complete target digest.
insert_at = text_offset + OLD_SIZE
growth = NEW_SIZE - OLD_SIZE
data[insert_at:insert_at] = b"\0" * growth

# Move every section that originally followed .text, including the section
# table itself.  No section payload precedes .text except the string/symbol
# tables, so their offsets remain stable.
new_shoff = old_shoff + growth
struct.pack_into(">I", data, 0x20, new_shoff)
for i, header in enumerate(headers):
    if i == text_index:
        header[5] = NEW_SIZE
    elif header[1] != 8 and header[4] >= insert_at:
        header[4] += growth
    struct.pack_into(">10I", data, new_shoff + i * shentsize, *header)

# Expand the sole retained function through the representation pool.  The
# symbol table itself precedes .text and therefore did not move.
matches = []
for header in headers:
    if header[1] != 2:
        continue
    strings = headers[header[6]]
    string_data = data[strings[4] : strings[4] + strings[5]]
    entsize = header[9] or 16
    for entry in range(header[4], header[4] + header[5], entsize):
        name_off, value, size = struct.unpack_from(">III", data, entry)
        end = string_data.find(b"\0", name_off)
        if string_data[name_off:end].decode("ascii") == SYMBOL:
            matches.append((entry, value, size, struct.unpack_from(">H", data, entry + 14)[0]))
if len(matches) != 1 or matches[0][1:] != (0, FUNCTION_SIZE, text_index):
    fail(f"unexpected function symbol identity: {matches}")
struct.pack_into(">I", data, matches[0][0] + 8, NEW_SIZE)

path.write_bytes(data)
