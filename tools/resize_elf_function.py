#!/usr/bin/env python3
"""Fail-loudly resize one function symbol after a reviewed text permutation.

This changes ELF metadata only.  The caller supplies the exact old/new sizes
and the SHA-256 of the complete new executable prefix; the object must contain
exactly one STT_FUNC in the selected section, beginning at offset zero.
"""

import hashlib
import pathlib
import struct
import sys


def fail(message):
    raise SystemExit(message)


if len(sys.argv) != 7:
    fail(
        "usage: resize_elf_function.py OBJECT SECTION SYMBOL OLD_SIZE "
        "NEW_SIZE EXPECTED_SHA256"
    )

path = pathlib.Path(sys.argv[1])
section_name = sys.argv[2]
symbol_name = sys.argv[3]
old_size = int(sys.argv[4], 0)
new_size = int(sys.argv[5], 0)
expected_digest = sys.argv[6].lower()
if old_size <= 0 or new_size <= 0 or old_size % 4 or new_size % 4:
    fail("function sizes must be positive and word aligned")
if len(expected_digest) != 64 or any(c not in "0123456789abcdef" for c in expected_digest):
    fail("expected a lowercase SHA-256 digest")

data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    fail("expected a big-endian ELF32 object")

section_table = struct.unpack_from(">I", data, 0x20)[0]
section_entry_size = struct.unpack_from(">H", data, 0x2E)[0]
section_count = struct.unpack_from(">H", data, 0x30)[0]
section_name_index = struct.unpack_from(">H", data, 0x32)[0]
headers = [
    struct.unpack_from(">10I", data, section_table + index * section_entry_size)
    for index in range(section_count)
]
name_header = headers[section_name_index]
section_names = data[name_header[4] : name_header[4] + name_header[5]]


def header_name(header):
    end = section_names.find(b"\0", header[0])
    return section_names[header[0] : end].decode("ascii")


section_matches = [
    (index, header)
    for index, header in enumerate(headers)
    if header_name(header) == section_name
]
if len(section_matches) != 1:
    fail(f"expected exactly one section {section_name!r}")
target_section_index, target_section = section_matches[0]
section_offset = target_section[4]
section_size = target_section[5]
if new_size > section_size:
    fail(f"new size {new_size:#x} exceeds section size {section_size:#x}")

actual_digest = hashlib.sha256(
    data[section_offset : section_offset + new_size]
).hexdigest()
if actual_digest != expected_digest:
    fail(
        f"{path}: configured {section_name} prefix SHA-256 mismatch: "
        f"expected {expected_digest}, got {actual_digest}"
    )

function_symbols = []
for header in headers:
    if header[1] != 2:
        continue
    strings_header = headers[header[6]]
    strings = data[
        strings_header[4] : strings_header[4] + strings_header[5]
    ]
    entry_size = header[9] or 16
    if header[5] % entry_size:
        fail("malformed symbol table")
    for entry in range(header[4], header[4] + header[5], entry_size):
        name_offset, value, size = struct.unpack_from(">III", data, entry)
        info = data[entry + 12]
        symbol_section = struct.unpack_from(">H", data, entry + 14)[0]
        if (info & 0xF) != 2 or symbol_section != target_section_index:
            continue
        end = strings.find(b"\0", name_offset)
        name = strings[name_offset:end].decode("ascii")
        function_symbols.append((entry, name, value, size))

if len(function_symbols) != 1 or function_symbols[0][1:] != (
    symbol_name,
    0,
    old_size,
):
    observed = ", ".join(
        f"{name}@{value:#x}+{size:#x}" for _, name, value, size in function_symbols
    )
    fail(
        f"expected sole function {symbol_name!r}@0+{old_size:#x}; got "
        f"{observed or 'none'}"
    )

struct.pack_into(">I", data, function_symbols[0][0] + 8, new_size)
path.write_bytes(data)
