#!/usr/bin/env python3
"""Patch proved instruction words in one big-endian ELF32 section.

This is intentionally narrow: each requested patch includes the expected old
word, so a compiler-output change fails loudly instead of silently modifying a
different instruction.
"""

import pathlib
import struct
import sys


if len(sys.argv) < 6 or (len(sys.argv) - 3) % 3:
    raise SystemExit(
        "usage: patch_elf_words.py OBJECT SECTION OFFSET EXPECTED REPLACEMENT "
        "[OFFSET EXPECTED REPLACEMENT ...]"
    )

path = pathlib.Path(sys.argv[1])
section_name = sys.argv[2]
patches = [
    tuple(int(value, 0) for value in sys.argv[index : index + 3])
    for index in range(3, len(sys.argv), 3)
]
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit(f"{path}: expected a big-endian ELF32 object")

section_header_offset = struct.unpack_from(">I", data, 0x20)[0]
section_header_size = struct.unpack_from(">H", data, 0x2E)[0]
section_count = struct.unpack_from(">H", data, 0x30)[0]
string_table_index = struct.unpack_from(">H", data, 0x32)[0]


def header(index):
    offset = section_header_offset + index * section_header_size
    return struct.unpack_from(">10I", data, offset)


string_header = header(string_table_index)
string_offset, string_size = string_header[4], string_header[5]
strings = data[string_offset : string_offset + string_size]

for index in range(section_count):
    values = header(index)
    name_offset, file_offset, section_size = values[0], values[4], values[5]
    end = strings.find(b"\0", name_offset)
    name = strings[name_offset:end].decode("ascii")
    if name != section_name:
        continue
    for offset, expected, replacement in patches:
        if offset < 0 or offset + 4 > section_size or offset % 4:
            raise SystemExit(
                f"{path}: patch offset {offset:#x} is outside aligned "
                f"{name} size {section_size:#x}"
            )
        actual = struct.unpack_from(">I", data, file_offset + offset)[0]
        if actual != expected:
            raise SystemExit(
                f"{path}: {name}+{offset:#x} expected {expected:#010x}, "
                f"got {actual:#010x}"
            )
        struct.pack_into(">I", data, file_offset + offset, replacement)
    path.write_bytes(data)
    break
else:
    raise SystemExit(f"{path}: section {section_name!r} not found")
