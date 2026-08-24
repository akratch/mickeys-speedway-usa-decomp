#!/usr/bin/env python3
"""Extend one ELF32 function symbol across proved trailing text alignment."""

import hashlib
import pathlib
import struct
import sys


def fail(message):
    raise SystemExit(message)


if len(sys.argv) != 6:
    fail(
        "usage: extend_elf_function_to_text.py OBJECT SYMBOL "
        "NATURAL_SIZE OWNED_SIZE NATURAL_TEXT_SHA256"
    )

path = pathlib.Path(sys.argv[1])
symbol = sys.argv[2]
natural_size = int(sys.argv[3], 0)
owned_size = int(sys.argv[4], 0)
expected_digest = sys.argv[5].lower()
if natural_size >= owned_size or natural_size % 4 or owned_size % 4:
    fail("sizes must be word-aligned and NATURAL_SIZE must be below OWNED_SIZE")

data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    fail("expected a big-endian ELF32 object")

section_table = struct.unpack_from(">I", data, 0x20)[0]
section_entry_size = struct.unpack_from(">H", data, 0x2E)[0]
section_count = struct.unpack_from(">H", data, 0x30)[0]
section_name_index = struct.unpack_from(">H", data, 0x32)[0]


def section(index):
    start = section_table + index * section_entry_size
    return struct.unpack_from(">10I", data, start)


sections = [section(index) for index in range(section_count)]
name_section = sections[section_name_index]
section_names = data[name_section[4] : name_section[4] + name_section[5]]


def section_name(values):
    start = values[0]
    end = section_names.find(b"\0", start)
    return section_names[start:end].decode("ascii")


text_matches = [
    (index, values)
    for index, values in enumerate(sections)
    if section_name(values) == ".text"
]
if len(text_matches) != 1:
    fail("expected exactly one .text section")
text_index, text = text_matches[0]
text_offset, text_size = text[4], text[5]
if text_size != owned_size:
    fail(f"expected .text size {owned_size:#x}, got {text_size:#x}")
text_bytes = bytes(data[text_offset : text_offset + text_size])
digest = hashlib.sha256(text_bytes).hexdigest()
if digest != expected_digest:
    fail(f"natural .text SHA-256 mismatch: expected {expected_digest}, got {digest}")
if text_bytes[natural_size:] != b"\0" * (owned_size - natural_size):
    fail("trailing owned text is not entirely zero alignment words")

matches = []
for values in sections:
    if values[1] != 2:  # SHT_SYMTAB
        continue
    sym_offset, sym_size = values[4], values[5]
    linked_strings, sym_entry_size = values[6], values[9]
    if sym_entry_size != 16 or sym_size % sym_entry_size:
        fail("unexpected ELF32 symbol-table layout")
    strings = sections[linked_strings]
    string_data = data[strings[4] : strings[4] + strings[5]]
    for entry in range(sym_offset, sym_offset + sym_size, sym_entry_size):
        name_offset, value, size = struct.unpack_from(">III", data, entry)
        info = data[entry + 12]
        shndx = struct.unpack_from(">H", data, entry + 14)[0]
        end = string_data.find(b"\0", name_offset)
        name = string_data[name_offset:end].decode("ascii")
        if name == symbol:
            matches.append((entry, value, size, info, shndx))

if len(matches) != 1:
    fail(f"expected one {symbol} symbol, found {len(matches)}")
entry, value, size, info, shndx = matches[0]
if value != 0 or size != natural_size or shndx != text_index or (info & 0xF) != 2:
    fail(
        f"unexpected {symbol} metadata: value={value:#x} size={size:#x} "
        f"section={shndx} type={info & 0xF}"
    )

struct.pack_into(">I", data, entry + 8, owned_size)
path.write_bytes(data)
