#!/usr/bin/env python3
"""Fail-loud preparation of the complete A6BC ABI/rematerialization web."""

import hashlib
import pathlib
import struct
import sys

EXPECTED_TEXT = "605e222fff8009002116634aa85191146ba79e3f563422821bc0877ab5c1831f"
SYMBOL = "overlay101TailA6BC"


def sections(data):
    shoff = struct.unpack_from(">I", data, 0x20)[0]
    shentsize = struct.unpack_from(">H", data, 0x2E)[0]
    shnum = struct.unpack_from(">H", data, 0x30)[0]
    shstrndx = struct.unpack_from(">H", data, 0x32)[0]
    headers = [struct.unpack_from(">10I", data, shoff + i * shentsize)
               for i in range(shnum)]
    shstr = headers[shstrndx]
    names = data[shstr[4]:shstr[4] + shstr[5]]
    result = []
    for index, header in enumerate(headers):
        end = names.find(b"\0", header[0])
        result.append((index, names[header[0]:end].decode(), header))
    return result


def instruction(op, rs, rt, imm):
    return (op << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF)


if len(sys.argv) != 2:
    raise SystemExit("usage: prepare_semantic_web.py OBJECT")
path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit("expected big-endian ELF32")
sects = sections(data)
text_index, _, text_header = next(item for item in sects if item[1] == ".text")
base, size = text_header[4], text_header[5]
text = bytes(data[base:base + size])
if size != 0x490 or hashlib.sha256(text).hexdigest() != EXPECTED_TEXT:
    raise SystemExit("candidate_v14 text drift")

# The only discarded natural instruction is IDO's unused a1 home. Convert it
# into the first rematerialized constant; the final two aligned zero words are
# converted into the other two members of the same complete web.
home = struct.unpack_from(">I", data, base + 0xB4)[0]
if home != instruction(43, 29, 5, 0x64):
    raise SystemExit("expected sw a1,0x64(sp) at +0xB4")
if data[base + 0x488:base + 0x490] != b"\0" * 8:
    raise SystemExit("expected two zero alignment words at +0x488")
struct.pack_into(">I", data, base + 0xB4, instruction(9, 0, 12, 2))
struct.pack_into(">I", data, base + 0x488, instruction(9, 0, 18, 24))
struct.pack_into(">I", data, base + 0x48C, instruction(9, 5, 5, 0))

# IDO naturally folds the initial D_1D0 LO16 into the following load. The
# retained schedule materializes the pointer separately, so transfer that one
# relocation to the rematerialized addiu and leave the load at offset zero.
relocation_matches = []
for _, name, header in sects:
    if header[1] != 9 or header[7] != text_index:
        continue
    entsize = header[9] or 8
    for pos in range(header[4], header[4] + header[5], entsize):
        offset = struct.unpack_from(">I", data, pos)[0]
        if offset == 0x2B4:
            relocation_matches.append(pos)
if len(relocation_matches) != 1:
    raise SystemExit("expected one folded LO16 relocation at +0x2B4")
struct.pack_into(">I", data, relocation_matches[0], 0x48C)
struct.pack_into(">H", data, base + 0x2B4 + 2, 0)

# Expand the sole function over the two compiler-alignment words.
matches = []
for _, _, header in sects:
    if header[1] != 2:
        continue
    strings_header = sects[header[6]][2]
    strings = data[strings_header[4]:strings_header[4] + strings_header[5]]
    entsize = header[9] or 16
    for pos in range(header[4], header[4] + header[5], entsize):
        name_off, value, sym_size = struct.unpack_from(">III", data, pos)
        end = strings.find(b"\0", name_off)
        if strings[name_off:end].decode() == SYMBOL:
            matches.append((pos, value, sym_size, struct.unpack_from(">H", data, pos + 14)[0]))
if matches != [(matches[0][0], 0, 0x488, text_index)] if matches else True:
    raise SystemExit("unexpected function symbol identity")
struct.pack_into(">I", data, matches[0][0] + 8, 0x490)
path.write_bytes(data)
