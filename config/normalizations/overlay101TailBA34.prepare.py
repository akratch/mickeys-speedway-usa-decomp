#!/usr/bin/env python3
"""Normalize complete duplicated-1.0 and volatile-scale webs in O101 +BA34."""

import hashlib
import pathlib
import struct
import sys

EXPECTED = "299b50ec5128ae69f71d75eb4ca2f356b40c4d1f9b86cfabfe02cc2b9f1422bf"
SYMBOL = "func_overlay_101_F000BA34_18E7254"
SIZE = 0x710
BRANCH_PAIR = (0x1C8, 0x1CC)
INSERT_BEFORE = 0x1B8
DROPS = (0x250, 0x254, 0x348, 0x350, 0x36C)
EXPANDS = (0x3C4, 0x474, 0x524, 0x5D4, 0x68C)


def cstr(blob, offset):
    end = blob.find(b"\0", offset)
    return blob[offset:end].decode()


if len(sys.argv) != 2:
    raise SystemExit("usage: prepare_ba34_semantic_web.py OBJECT")
path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit("expected big-endian ELF32")
shoff = struct.unpack_from(">I", data, 0x20)[0]
shentsize = struct.unpack_from(">H", data, 0x2E)[0]
shnum = struct.unpack_from(">H", data, 0x30)[0]
shstrndx = struct.unpack_from(">H", data, 0x32)[0]
headers = [list(struct.unpack_from(">10I", data, shoff + i * shentsize))
           for i in range(shnum)]
shstr = headers[shstrndx]
names = data[shstr[4]:shstr[4] + shstr[5]]
section_names = [cstr(names, h[0]) for h in headers]
if section_names.count(".text") != 1:
    raise SystemExit("expected one .text")
text_index = section_names.index(".text")
text = headers[text_index]
base, size = text[4], text[5]
body = bytes(data[base:base + size])
if size != SIZE or hashlib.sha256(body).hexdigest() != EXPECTED:
    raise SystemExit("ba34_v06 natural text drift")
words = list(struct.unpack(">452I", body))

expected = {
    0x1C8: 0x3C013F80,
    0x1CC: 0x44814000,
    0x250: 0x3C013F80,
    0x254: 0x44815000,
    0x290: 0xE60A000C,
    0x348: 0x3C013F80,
    0x350: 0x44818000,
    0x36C: 0xE7B00044,
}
for offset, word in expected.items():
    if words[offset // 4] != word:
        raise SystemExit(f"semantic web drift at {offset:#x}: {words[offset // 4]:08x}")
for offset in EXPANDS:
    if words[offset // 4] != 0xC7A80044:
        raise SystemExit(f"expected lwc1 f8,0x44(sp) at {offset:#x}")

# Build a provenance-preserving token stream.  The zero/nonzero branch's first
# 1.0f producer pair moves to the dominator, the duplicate pair disappears,
# and the second consumer is rebound to the retained FPR.  The volatile scale
# home disappears and each of its five loads expands to a literal pair.
tokens = []
for old_index, word in enumerate(words):
    old_offset = old_index * 4
    if old_offset == INSERT_BEFORE:
        tokens.append((BRANCH_PAIR[0], words[BRANCH_PAIR[0] // 4]))
        tokens.append((BRANCH_PAIR[1], words[BRANCH_PAIR[1] // 4]))
    if old_offset in BRANCH_PAIR or old_offset in DROPS:
        continue
    if old_offset == 0x290:
        word = 0xE608000C  # swc1 f8,0xc(s0): retained dominator value
    if old_offset in EXPANDS:
        tokens.append((old_offset, 0x3C013F80))
        tokens.append((old_offset, 0x44814000))
    else:
        tokens.append((old_offset, word))
if len(tokens) != 452:
    raise SystemExit("prepared instruction inventory drift")

old_to_new = {}
for new_index, (old_offset, _) in enumerate(tokens):
    old_to_new.setdefault(old_offset, new_index * 4)
old_to_new[SIZE] = SIZE

new_words = []
for new_index, (old_offset, word) in enumerate(tokens):
    opcode = word >> 26
    if opcode in (1, 4, 5, 6, 7, 20, 21, 22, 23):
        immediate = word & 0xFFFF
        if immediate & 0x8000:
            immediate -= 0x10000
        old_target = old_offset + 4 + immediate * 4
        if old_target not in old_to_new:
            raise SystemExit(f"unmapped branch target {old_target:#x}")
        new_offset = new_index * 4
        new_target = old_to_new[old_target]
        immediate = (new_target - (new_offset + 4)) // 4
        word = (word & 0xFFFF0000) | (immediate & 0xFFFF)
    new_words.append(word)
data[base:base + SIZE] = struct.pack(">452I", *new_words)

for header in headers:
    if header[1] != 9 or header[7] != text_index:
        continue
    entsize = header[9] or 8
    for pos in range(header[4], header[4] + header[5], entsize):
        offset = struct.unpack_from(">I", data, pos)[0]
        if offset in BRANCH_PAIR or offset in DROPS or offset in EXPANDS:
            raise SystemExit(f"representation instruction owns relocation {offset:#x}")
        struct.pack_into(">I", data, pos, old_to_new[offset])

found = 0
for header in headers:
    if header[1] != 2:
        continue
    strings_header = headers[header[6]]
    strings = data[strings_header[4]:strings_header[4] + strings_header[5]]
    entsize = header[9] or 16
    for pos in range(header[4], header[4] + header[5], entsize):
        name_off, value, sym_size = struct.unpack_from(">III", data, pos)
        shndx = struct.unpack_from(">H", data, pos + 14)[0]
        name = cstr(strings, name_off)
        if shndx == text_index and value in old_to_new:
            struct.pack_into(">I", data, pos + 4, old_to_new[value])
        if name == SYMBOL:
            if (value, sym_size, shndx) != (0, SIZE, text_index):
                raise SystemExit("owned function symbol drift")
            found += 1
if found != 1:
    raise SystemExit("owned function symbol missing")
path.write_bytes(data)
