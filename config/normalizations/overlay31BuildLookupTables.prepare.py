#!/usr/bin/env python3
"""Select the complete O31 prefix induction/sign-extension topology."""

import hashlib
import pathlib
import struct
import sys

EXPECTED = "c9e8b8d8a227102b36a34b50dbf6bb911beed69515031d01b3b1a6266588b0d0"
SYMBOL = "func_overlay_031_F0000000_187F520"
OLD_SIZE = 0x2E0
OWNER_SIZE = 0x2E8
PADDED_SIZE = 0x2F0
EXPECTED_RELOCS = (0x2C, 0x34, 0x3C, 0x44, 0x4C,
                    0x200, 0x208, 0x214, 0x264, 0x270)

if len(sys.argv) != 2:
    raise SystemExit("usage: prepare_topology.py OBJECT")
path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit("expected big-endian ELF32")

old_shoff = struct.unpack_from(">I", data, 0x20)[0]
stride = struct.unpack_from(">H", data, 0x2E)[0]
count = struct.unpack_from(">H", data, 0x30)[0]
shstrndx = struct.unpack_from(">H", data, 0x32)[0]
headers = [list(struct.unpack_from(">10I", data, old_shoff + i * stride))
           for i in range(count)]
shstr = headers[shstrndx]
names = data[shstr[4]:shstr[4] + shstr[5]]

def section_name(header):
    end = names.find(b"\0", header[0])
    return names[header[0]:end].decode()

by_name = {section_name(h): (i, h) for i, h in enumerate(headers)}
text_index, text = by_name[".text"]
base, size = text[4], text[5]
blob = bytes(data[base:base + size])
if size != OLD_SIZE or hashlib.sha256(blob).hexdigest() != EXPECTED:
    raise SystemExit("candidate_v04 natural text drift")
old_words = list(struct.unpack(">" + "I" * (OLD_SIZE // 4), blob))
assert old_words[0x54 // 4] == 0x24160009       # hoisted end value
assert old_words[0x1F0 // 4] == 0x00C06025      # move t4,a2
assert old_words[0x1F4 // 4] == 0x14D6FF99      # bne a2,s6,loop
assert old_words[0x218 // 4] == 0xAEB2FFF8      # row pointer store
assert old_words[0x28C // 4] == 0x00198403      # sra s0,t9,16

# Each tuple is (word, original offset or None).  Relocate the hoisted 9 into
# its second-loop lifetime, expand the first induction reuse into retail's
# explicit increment/test, and retain the signed-16 result as a separate
# complete identity carrier.
entries = []
new_offset = {}
for old in range(0, OLD_SIZE, 4):
    word = old_words[old // 4]
    if old == 0x54:
        continue
    if old == 0x1F0:
        new_offset[old] = len(entries) * 4
        entries.append((0x258C0001, old))          # addiu t4,t4,1
        entries.append((0x29810009, None))         # slti at,t4,9
        continue
    if old == 0x1F4:
        new_offset[old] = len(entries) * 4
        entries.append((0x14200000, old))          # bnez at, original target
        continue
    if old == 0x218:
        entries.append((0x24160009, None))         # li s6,9
    new_offset[old] = len(entries) * 4
    if old == 0x28C:
        entries.append((0x00196C03, old))          # sra t5,t9,16
        entries.append((0x01A08025, None))         # move s0,t5
    else:
        entries.append((word, old))

if len(entries) * 4 != OWNER_SIZE:
    raise SystemExit(f"prepared owner size drift: {len(entries) * 4:#x}")

# Preserve every original branch destination through the insertions/removal.
for index, (word, origin) in enumerate(entries):
    if origin is None:
        continue
    original_word = old_words[origin // 4]
    op = original_word >> 26
    if op not in (1, 4, 5, 6, 7, 20, 21, 22, 23):
        continue
    immediate = original_word & 0xFFFF
    if immediate & 0x8000:
        immediate -= 0x10000
    old_target = origin + 4 + immediate * 4
    if old_target not in new_offset:
        raise SystemExit(f"branch target {old_target:#x} has no prepared mapping")
    here = index * 4
    new_imm = (new_offset[old_target] - (here + 4)) // 4
    entries[index] = ((word & 0xFFFF0000) | (new_imm & 0xFFFF), origin)

prepared = b"".join(struct.pack(">I", word) for word, _ in entries)

# Grow the physical section by one 16-byte aligned block.  The stock guarded
# trim step later proves and removes the final eight zero bytes.
insert_at = base + OLD_SIZE
data[insert_at:insert_at] = b"\0" * 16
new_shoff = old_shoff + (16 if old_shoff >= insert_at else 0)
struct.pack_into(">I", data, 0x20, new_shoff)
for header in headers:
    if header[4] >= insert_at and header[4] != 0:
        header[4] += 16
headers[text_index][5] = PADDED_SIZE
for index, header in enumerate(headers):
    struct.pack_into(">10I", data, new_shoff + index * stride, *header)
data[base:base + OWNER_SIZE] = prepared
data[base + OWNER_SIZE:base + PADDED_SIZE] = b"\0" * (PADDED_SIZE - OWNER_SIZE)

# Move compiler relocations with their originating instructions.
_, reltext = by_name[".rel.text"]
relbase = reltext[4]
relocs = []
for pos in range(relbase, relbase + reltext[5], reltext[9] or 8):
    offset, info = struct.unpack_from(">II", data, pos)
    relocs.append(offset)
    if offset not in new_offset:
        raise SystemExit(f"unexpected relocation offset {offset:#x}")
    struct.pack_into(">I", data, pos, new_offset[offset])
if tuple(relocs) != EXPECTED_RELOCS:
    raise SystemExit(f"relocation surface drift: {relocs}")

# Resize the sole function and the .text section symbol to the owned range.
function_matches = []
section_matches = []
for header in headers:
    if header[1] != 2:
        continue
    strings_header = headers[header[6]]
    strings = data[strings_header[4]:strings_header[4] + strings_header[5]]
    entsize = header[9] or 16
    for pos in range(header[4], header[4] + header[5], entsize):
        name_off, value, sym_size = struct.unpack_from(">III", data, pos)
        info = data[pos + 12]
        shndx = struct.unpack_from(">H", data, pos + 14)[0]
        end = strings.find(b"\0", name_off)
        name = strings[name_off:end].decode()
        if name == SYMBOL:
            function_matches.append((pos, value, sym_size, shndx))
        if (info & 0xF) == 3 and shndx == text_index:
            section_matches.append((pos, value, sym_size))
if len(function_matches) != 1 or function_matches[0][1:] != (0, OLD_SIZE, text_index):
    raise SystemExit(f"function symbol drift: {function_matches}")
if len(section_matches) != 1 or section_matches[0][1:] != (0, OLD_SIZE):
    raise SystemExit(f"section symbol drift: {section_matches}")
struct.pack_into(">I", data, function_matches[0][0] + 8, OWNER_SIZE)
struct.pack_into(">I", data, section_matches[0][0] + 8, OWNER_SIZE)
path.write_bytes(data)
