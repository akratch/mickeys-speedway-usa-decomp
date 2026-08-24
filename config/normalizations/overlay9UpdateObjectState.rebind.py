#!/usr/bin/env python3
"""Rebind O9 prefix call relocations to its defined owner carrier.

Retail uses the owner symbol as the static carrier for eleven runtime-patched
calls.  The C compiler necessarily emits the semantic callees.  This narrow
tool changes only those eleven R_MIPS_26 symbol indices, and refuses any
unexpected ELF, owner, offset, relocation type, or original callee.
"""

import pathlib
import struct
import sys


OWNER = "func_overlay_009_F0000000_1866678"
EXPECTED_CALLS = {
    0x040: "ext_o0_1ee14",
    0x0CC: "ext_o0_1d4c0",
    0x114: "ext_o0_29adc",
    0x144: "ext_o0_1312c",
    0x420: "ext_o0_2a470",
    0x448: "ext_o0_5aac4",
    0x468: "ext_o0_19668",
    0x498: "ext_o0_1d510",
    0x4D0: "ext_o0_2d98",
    0x4F4: "ext_o0_2b90",
    0x524: "ext_o0_3e99c",
}


def fail(message):
    raise SystemExit(message)


def c_string(data, offset, context):
    if offset >= len(data):
        fail(f"{context}: string offset out of range")
    end = data.find(b"\0", offset)
    if end < 0:
        fail(f"{context}: unterminated string")
    return data[offset:end].decode("ascii")


if len(sys.argv) != 2:
    fail("usage: rebind_calls_to_owner.py OBJECT")

path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    fail("expected a big-endian ELF32 object")

shoff = struct.unpack_from(">I", data, 0x20)[0]
shentsize = struct.unpack_from(">H", data, 0x2E)[0]
shnum = struct.unpack_from(">H", data, 0x30)[0]
shstrndx = struct.unpack_from(">H", data, 0x32)[0]
if shentsize < 40 or shstrndx >= shnum:
    fail("malformed ELF section table")
sections = []
for index in range(shnum):
    start = shoff + index * shentsize
    sections.append(struct.unpack_from(">10I", data, start))
shstr = sections[shstrndx]
shstrings = data[shstr[4] : shstr[4] + shstr[5]]
section_names = [c_string(shstrings, sh[0], f"section {i}") for i, sh in enumerate(sections)]

if section_names.count(".text") != 1 or section_names.count(".rel.text") != 1:
    fail("expected exactly one .text and one .rel.text")
text_index = section_names.index(".text")
rel = sections[section_names.index(".rel.text")]
if rel[1] != 9 or rel[7] != text_index or (rel[9] or 8) != 8:
    fail("unexpected .rel.text shape")
symtab_index = rel[6]
symtab = sections[symtab_index]
if symtab[1] != 2 or (symtab[9] or 16) != 16:
    fail("unexpected symbol table shape")
strtab = sections[symtab[6]]
strings = data[strtab[4] : strtab[4] + strtab[5]]

symbols = []
by_name = {}
for index, start in enumerate(range(symtab[4], symtab[4] + symtab[5], 16)):
    name_off, value, size, info, other, shndx = struct.unpack_from(">IIIBBH", data, start)
    name = c_string(strings, name_off, f"symbol {index}")
    symbols.append((name, value, size, info, other, shndx))
    if name:
        if name in by_name:
            fail(f"duplicate symbol name before normalization: {name}")
        by_name[name] = index

if OWNER not in by_name:
    fail(f"defined owner is absent: {OWNER}")
owner_index = by_name[OWNER]
owner = symbols[owner_index]
if owner[1] != 0 or owner[2] != 0x540 or (owner[3] & 0xF) != 2 or owner[5] != text_index:
    fail(f"owner symbol shape is unexpected: {owner}")

seen = set()
for start in range(rel[4], rel[4] + rel[5], 8):
    offset, info = struct.unpack_from(">II", data, start)
    if offset not in EXPECTED_CALLS:
        continue
    if offset in seen:
        fail(f"duplicate call relocation at {offset:#x}")
    seen.add(offset)
    symbol_index, relocation_type = info >> 8, info & 0xFF
    if relocation_type != 4:
        fail(f"{offset:#x}: expected R_MIPS_26, found type {relocation_type}")
    expected = EXPECTED_CALLS[offset]
    actual = symbols[symbol_index][0]
    if actual != expected:
        fail(f"{offset:#x}: expected {expected}, found {actual}")
    struct.pack_into(">I", data, start + 4, (owner_index << 8) | relocation_type)

missing = sorted(set(EXPECTED_CALLS) - seen)
if missing:
    fail("missing call relocation(s): " + ", ".join(hex(x) for x in missing))

path.write_bytes(data)
print(f"rebound {len(seen)} asserted R_MIPS_26 records to defined owner index {owner_index}")
