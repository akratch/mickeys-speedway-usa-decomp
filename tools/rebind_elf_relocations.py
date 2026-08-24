#!/usr/bin/env python3
"""Rebind selected ELF32 relocations to existing symbols, with assertions.

This is a narrow post-compiler normalization tool for cases where the compiler
reuses one undefined symbol for multiple address materializations but retail's
link result requires different bindings at specific sites.  It changes only
the symbol index in an existing relocation; the relocation type, instruction,
symbol table, and string table are left untouched.

Usage:

    rebind_elf_relocations.py OBJECT SECTION \
        OFFSET:EXPECTED_SYMBOL:REPLACEMENT_SYMBOL [...]

An argument of ``@PATH`` expands non-empty, non-comment specification lines
from PATH. This keeps large reviewed relocation contracts target-local and
diffable without weakening the per-offset assertions.
"""

import pathlib
import struct
import sys


SHT_SYMTAB = 2
SHT_RELA = 4
SHT_REL = 9


def fail(message):
    raise SystemExit(message)


def c_string(data, offset, context):
    if offset < 0 or offset >= len(data):
        fail(f"{context}: string offset {offset:#x} is out of range")
    end = data.find(b"\0", offset)
    if end < 0:
        fail(f"{context}: unterminated string")
    try:
        return data[offset:end].decode("ascii")
    except UnicodeDecodeError as error:
        fail(f"{context}: non-ASCII string: {error}")


def elf_sections(data):
    if data[:6] != b"\x7fELF\x01\x02":
        fail("expected a big-endian ELF32 object")

    header_offset = struct.unpack_from(">I", data, 0x20)[0]
    header_size = struct.unpack_from(">H", data, 0x2E)[0]
    count = struct.unpack_from(">H", data, 0x30)[0]
    string_index = struct.unpack_from(">H", data, 0x32)[0]
    if header_size < 40 or string_index >= count:
        fail("malformed ELF section table")

    headers = []
    for index in range(count):
        offset = header_offset + index * header_size
        if offset + 40 > len(data):
            fail("ELF section table extends past end of file")
        headers.append(struct.unpack_from(">10I", data, offset))

    string_header = headers[string_index]
    start, size = string_header[4], string_header[5]
    section_strings = data[start : start + size]
    if len(section_strings) != size:
        fail("section-name string table extends past end of file")

    sections = []
    for index, values in enumerate(headers):
        name = c_string(section_strings, values[0], f"section {index}")
        sections.append((index, name, values))
    return sections


def symbol_names(data, sections, symbol_table_index):
    _, table_name, table = sections[symbol_table_index]
    if table[1] != SHT_SYMTAB:
        fail(f"relocation section does not link to a symbol table: {table_name!r}")
    string_index = table[6]
    if string_index >= len(sections):
        fail(f"symbol table {table_name!r} has invalid string-table index")
    strings = sections[string_index][2]
    string_data = data[strings[4] : strings[4] + strings[5]]
    if len(string_data) != strings[5]:
        fail(f"string table for {table_name!r} extends past end of file")

    entry_size = table[9] or 16
    if entry_size < 16 or table[5] % entry_size:
        fail(f"malformed symbol table {table_name!r}")
    names = []
    by_name = {}
    for index, entry_offset in enumerate(
        range(table[4], table[4] + table[5], entry_size)
    ):
        if entry_offset + 16 > len(data):
            fail(f"symbol table {table_name!r} extends past end of file")
        name_offset = struct.unpack_from(">I", data, entry_offset)[0]
        name = c_string(string_data, name_offset, f"symbol {index}")
        names.append(name)
        if name:
            if name in by_name:
                fail(f"duplicate symbol name {name!r} in {table_name!r}")
            by_name[name] = index
    return names, by_name


if len(sys.argv) < 4:
    fail(
        "usage: rebind_elf_relocations.py OBJECT SECTION "
        "OFFSET:EXPECTED_SYMBOL:REPLACEMENT_SYMBOL [...]"
    )

path = pathlib.Path(sys.argv[1])
section_name = sys.argv[2]
requests = {}
specifications = []
for argument in sys.argv[3:]:
    if not argument.startswith("@"):
        specifications.append(argument)
        continue
    spec_path = pathlib.Path(argument[1:])
    if not spec_path.is_file():
        fail(f"relocation rebind specification file not found: {spec_path}")
    for line in spec_path.read_text().splitlines():
        content = line.partition("#")[0].strip()
        if content:
            specifications.extend(content.split())

for argument in specifications:
    try:
        offset_text, expected, replacement = argument.split(":", 2)
        offset = int(offset_text, 0)
    except ValueError:
        fail(f"invalid relocation rebind specification {argument!r}")
    if offset < 0 or offset % 4:
        fail(f"unaligned relocation offset {offset:#x}")
    if offset in requests:
        fail(f"duplicate relocation offset {offset:#x}")
    if not expected or not replacement or expected == replacement:
        fail(f"invalid relocation symbols in {argument!r}")
    requests[offset] = (expected, replacement)

data = bytearray(path.read_bytes())
sections = elf_sections(data)
targets = [index for index, name, _ in sections if name == section_name]
if len(targets) != 1:
    fail(f"expected exactly one section named {section_name!r}, found {len(targets)}")
target_index = targets[0]

matches = {offset: [] for offset in requests}
symbol_cache = {}
for _, relocation_name, relocation in sections:
    section_type = relocation[1]
    if section_type not in (SHT_REL, SHT_RELA) or relocation[7] != target_index:
        continue
    expected_entry_size = 8 if section_type == SHT_REL else 12
    entry_size = relocation[9] or expected_entry_size
    if entry_size < expected_entry_size or relocation[5] % entry_size:
        fail(f"malformed relocation section {relocation_name!r}")
    symbol_table_index = relocation[6]
    if symbol_table_index not in symbol_cache:
        symbol_cache[symbol_table_index] = symbol_names(
            data, sections, symbol_table_index
        )
    names, by_name = symbol_cache[symbol_table_index]

    for entry_offset in range(
        relocation[4], relocation[4] + relocation[5], entry_size
    ):
        if entry_offset + expected_entry_size > len(data):
            fail(f"relocation section {relocation_name!r} extends past end of file")
        instruction_offset, info = struct.unpack_from(">II", data, entry_offset)
        if instruction_offset not in requests:
            continue
        symbol_index = info >> 8
        if symbol_index >= len(names):
            fail(f"invalid symbol index at {section_name}+{instruction_offset:#x}")
        matches[instruction_offset].append(
            (entry_offset, info, names[symbol_index], by_name)
        )

for offset, (expected, replacement) in requests.items():
    entries = matches[offset]
    if len(entries) != 1:
        fail(
            f"expected exactly one relocation at {section_name}+{offset:#x}, "
            f"found {len(entries)}"
        )
    entry_offset, info, actual, by_name = entries[0]
    if actual != expected:
        fail(
            f"{section_name}+{offset:#x}: expected relocation symbol "
            f"{expected!r}, found {actual!r}"
        )
    if replacement not in by_name:
        fail(f"replacement symbol {replacement!r} is absent from the symbol table")
    replacement_info = (by_name[replacement] << 8) | (info & 0xFF)
    struct.pack_into(">I", data, entry_offset + 4, replacement_info)

path.write_bytes(data)
