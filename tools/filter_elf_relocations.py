#!/usr/bin/env python3
"""Remove an asserted set of ELF32 REL records from one target section.

This packet-only helper is deliberately narrower than objcopy's
--remove-relocations: it preserves every unlisted relocation and fails unless
each requested offset, type, and symbol occurs exactly once.  Removed records
are compacted in place and the relocation section's size is reduced; no text,
symbol, or string bytes are changed.
"""

import pathlib
import struct
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import postprocess_guard as guard


SHT_SYMTAB = 2
SHT_REL = 9


def fail(message):
    # Under PROMOTION_TRIAL the guard reports and skips instead of
    # aborting the build; see tools/postprocess_guard.py.
    guard.fail(message)


def c_string(data, offset, context):
    if offset < 0 or offset >= len(data):
        fail(f"{context}: string offset {offset:#x} out of range")
    end = data.find(b"\0", offset)
    if end < 0:
        fail(f"{context}: unterminated string")
    return data[offset:end].decode("ascii")


def sections(data):
    if data[:6] != b"\x7fELF\x01\x02":
        fail("expected big-endian ELF32")
    table_offset = struct.unpack_from(">I", data, 0x20)[0]
    entry_size = struct.unpack_from(">H", data, 0x2E)[0]
    count = struct.unpack_from(">H", data, 0x30)[0]
    names_index = struct.unpack_from(">H", data, 0x32)[0]
    headers = []
    for index in range(count):
        header_offset = table_offset + index * entry_size
        headers.append((header_offset, struct.unpack_from(">10I", data, header_offset)))
    names_header = headers[names_index][1]
    names = data[names_header[4] : names_header[4] + names_header[5]]
    return [
        (index, c_string(names, values[0], f"section {index}"), header_offset, values)
        for index, (header_offset, values) in enumerate(headers)
    ]


def symbol_names(data, all_sections, symtab_index):
    _, name, _, symtab = all_sections[symtab_index]
    if symtab[1] != SHT_SYMTAB:
        fail(f"{name}: linked section is not a symbol table")
    strings = all_sections[symtab[6]][3]
    string_data = data[strings[4] : strings[4] + strings[5]]
    entry_size = symtab[9] or 16
    if entry_size < 16 or symtab[5] % entry_size:
        fail(f"{name}: malformed symbol table")
    out = []
    for entry_offset in range(symtab[4], symtab[4] + symtab[5], entry_size):
        string_offset = struct.unpack_from(">I", data, entry_offset)[0]
        out.append(c_string(string_data, string_offset, "symbol"))
    return out


if len(sys.argv) < 4:
    fail(
        "usage: filter_elf_relocations.py OBJECT SECTION "
        "OFFSET:TYPE:SYMBOL|@SPEC_FILE [...]"
    )

path = pathlib.Path(sys.argv[1])
target_name = sys.argv[2]
requests = {}
specs = []
for argument in sys.argv[3:]:
    if not argument.startswith("@"):
        specs.append(argument)
        continue
    spec_path = pathlib.Path(argument[1:])
    if not spec_path.is_file():
        fail(f"relocation specification file not found: {spec_path}")
    for line_number, line in enumerate(spec_path.read_text().splitlines(), 1):
        content = line.partition("#")[0].strip()
        if content:
            specs.extend(content.split())

for spec in specs:
    try:
        offset_text, type_text, symbol = spec.split(":", 2)
        key = (int(offset_text, 0), int(type_text, 0), symbol)
    except ValueError:
        fail(f"invalid removal specification {spec!r}")
    if key in requests:
        fail(f"duplicate removal specification {spec!r}")
    requests[key] = 0

if not requests:
    fail("no relocation removal specifications supplied")

data = bytearray(path.read_bytes())
all_sections = sections(data)
targets = [row for row in all_sections if row[1] == target_name]
if len(targets) != 1:
    fail(f"expected one section named {target_name!r}, found {len(targets)}")
target_index = targets[0][0]
rel_sections = [
    row for row in all_sections
    if row[3][1] == SHT_REL and row[3][7] == target_index
]
if len(rel_sections) != 1:
    fail(f"expected one REL section targeting {target_name!r}, found {len(rel_sections)}")

_, rel_name, rel_header_offset, rel = rel_sections[0]
entry_size = rel[9] or 8
if entry_size != 8 or rel[5] % entry_size:
    fail(f"{rel_name}: expected packed 8-byte REL entries")
names = symbol_names(data, all_sections, rel[6])
kept = []
for entry_offset in range(rel[4], rel[4] + rel[5], entry_size):
    instruction_offset, info = struct.unpack_from(">II", data, entry_offset)
    symbol_index = info >> 8
    relocation_type = info & 0xFF
    if symbol_index >= len(names):
        fail(f"{rel_name}: symbol index {symbol_index} out of range")
    key = (instruction_offset, relocation_type, names[symbol_index])
    if key in requests:
        requests[key] += 1
    else:
        kept.append(data[entry_offset : entry_offset + entry_size])

bad = [key for key, count in requests.items() if count != 1]
if bad:
    fail(f"requested relocation count was not exactly one: {bad!r}")

new_data = b"".join(kept)
old_size = rel[5]
data[rel[4] : rel[4] + len(new_data)] = new_data
data[rel[4] + len(new_data) : rel[4] + old_size] = b"\0" * (old_size - len(new_data))
struct.pack_into(">I", data, rel_header_offset + 0x14, len(new_data))
path.write_bytes(data)
