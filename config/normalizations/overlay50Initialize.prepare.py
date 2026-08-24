#!/usr/bin/env python3
"""Normalize O50's shared-store base and complete branch-likely CFG web."""

import hashlib
import pathlib
import struct
import sys

EXPECTED = "b3893a8286f915dacbd3c70d289545b53dccb84c7da03783c4588105320351dd"
SYMBOL = "func_overlay_050_F0000000_1896970"
SIZE = 0x2E4
SECTION_SIZE = 0x2F0
DROPS = (0x188, 0x190)


def cstr(blob, offset):
    end = blob.find(b"\0", offset)
    return blob[offset:end].decode()


if len(sys.argv) != 2:
    raise SystemExit("usage: prepare_semantic_web.py OBJECT")
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
text_index = section_names.index(".text")
base, size = headers[text_index][4], headers[text_index][5]
body = bytes(data[base:base + size])
if size != SECTION_SIZE or hashlib.sha256(body).hexdigest() != EXPECTED:
    raise SystemExit("candidate_v03 natural text drift")
words = list(struct.unpack(">185I", body[:SIZE]))
expected = {
    0x188: 0x3C010000, 0x190: 0x3C010000,
    0x270: 0x3C010000, 0x274: 0x912A0000,
    0x278: 0x11400003, 0x27C: 0x00000000,
    0x280: 0x10000011, 0x284: 0xAC200000,
    0x2C8: 0x8FBF0014,
    0x150: 0x24C60000, 0x158: 0xACD90000,
    0x2B8: 0xAC220000,
}
for offset, word in expected.items():
    if words[offset // 4] != word:
        raise SystemExit(f"semantic web drift at {offset:#x}")

# Retail materializes the C4 address explicitly, uses branch-likely to place
# the nonzero store in its annulled delay slot, and duplicates the tiny RA-load
# join on the two paths.  Tokens remain tied to their natural semantic owners.
tokens = []
for index, word in enumerate(words):
    old = index * 4
    if old in DROPS or old in (0x27C, 0x280, 0x284, 0x2C8):
        continue
    # Two direct-global write webs use reciprocal materialization forms.
    # Preserve each relocation owner while transposing the store/addiu form;
    # the unrelocated D_BC store supplies the retained C4 value store.
    if old == 0x150:
        word = words[0x2B8 // 4]
    elif old == 0x2B8:
        word = words[0x150 // 4]
    if old == 0x274:
        tokens.append((0x284, 0x24210000, "c4_addr"))
        tokens.append((old, word, None))
        tokens.append((0x278, 0x55400000, "likely"))
        tokens.append((None, 0xAC200000, "early_store"))
        continue
    if old == 0x278:
        continue
    if old == 0x2CC:
        tokens.append((0x280, 0x10000000, "join"))
        tokens.append((0x2C8, 0x8FBF0014, None))
        tokens.append((None, 0xAC200000, "else_store"))
        tokens.append((None, 0x8FBF0014, "else_load"))
    tokens.append((old, word, None))
if len(tokens) != 185:
    raise SystemExit(f"prepared inventory drift: {len(tokens)}")

old_to_new = {}
markers = {}
for new_index, (old, _, marker) in enumerate(tokens):
    if old is not None:
        old_to_new.setdefault(old, new_index * 4)
    if marker:
        markers[marker] = new_index * 4
old_to_new[SIZE] = SIZE

out = []
for new_index, (old, word, marker) in enumerate(tokens):
    new_offset = new_index * 4
    if marker == "likely":
        target = markers["else_store"]
        word = (word & 0xFFFF0000) | (((target - new_offset - 4) // 4) & 0xFFFF)
    elif marker == "join":
        target = old_to_new[0x2CC]
        word = (word & 0xFFFF0000) | (((target - new_offset - 4) // 4) & 0xFFFF)
    elif old is not None:
        opcode = word >> 26
        if opcode in (1, 4, 5, 6, 7, 20, 21, 22, 23):
            immediate = word & 0xFFFF
            if immediate & 0x8000:
                immediate -= 0x10000
            old_target = old + 4 + immediate * 4
            target = old_to_new[old_target]
            word = ((word & 0xFFFF0000)
                    | (((target - new_offset - 4) // 4) & 0xFFFF))
    out.append(word)
data[base:base + SIZE] = struct.pack(">185I", *out)

for header in headers:
    if header[1] != 9 or header[7] != text_index:
        continue
    entsize = header[9] or 8
    for pos in range(header[4], header[4] + header[5], entsize):
        offset = struct.unpack_from(">I", data, pos)[0]
        if offset in DROPS:
            raise SystemExit(f"dropped shared-base HI still relocated at {offset:#x}")
        if offset not in old_to_new:
            raise SystemExit(f"unmapped relocation {offset:#x}")
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
                raise SystemExit("owner symbol drift")
            found += 1
if found != 1:
    raise SystemExit("owner symbol missing")
path.write_bytes(data)
