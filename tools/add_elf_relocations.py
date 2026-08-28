#!/usr/bin/env python3
"""Add asserted relocations to an exact configured ELF text section.

This is intentionally narrower than an assembler or linker. The executable
prefix must already have the supplied SHA-256, every requested site must be
aligned and currently relocation-free, and every replacement symbol must
already exist in the object's linked symbol table. Existing records are kept
byte-for-byte and the final table is sorted by instruction offset.

Usage:

    add_elf_relocations.py OBJECT SECTION SIZE SHA256 \
        OFFSET:TYPE:SYMBOL[:ADDEND] [...]

TYPE accepts ``R26``/``4``, ``HI16``/``5``, and ``LO16``/``6``. ADDEND defaults to zero and
must equal the instruction's existing immediate field. This keeps nonzero
retail relocation addends explicit and fail-loud at every call site.
"""

import hashlib
import pathlib
import struct
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import postprocess_guard as guard


SHT_NOBITS = 8
SHT_REL = 9
SHT_SYMTAB = 2
RELOCATION_TYPES = {"R26": 4, "4": 4, "HI16": 5, "LO16": 6, "5": 5, "6": 6}


def fail(message):
    # Under PROMOTION_TRIAL the guard reports and skips instead of
    # aborting the build; see tools/postprocess_guard.py.
    guard.fail(message)


def c_string(data, offset, context):
    if offset < 0 or offset >= len(data):
        fail(f"{context}: string offset is out of range")
    end = data.find(b"\0", offset)
    if end < 0:
        fail(f"{context}: unterminated string")
    return data[offset:end].decode("ascii")


if len(sys.argv) < 6:
    fail(
        "usage: add_elf_relocations.py OBJECT SECTION SIZE SHA256 "
        "OFFSET:TYPE:SYMBOL[:ADDEND] [...]"
    )

path = pathlib.Path(sys.argv[1])
section_name = sys.argv[2]
hash_size = int(sys.argv[3], 0)
expected_digest = sys.argv[4].lower()
if len(expected_digest) != 64 or any(c not in "0123456789abcdef" for c in expected_digest):
    fail("expected a lowercase SHA-256 digest")

requests = {}
for argument in sys.argv[5:]:
    try:
        parts = argument.split(":")
        if len(parts) not in (3, 4):
            raise ValueError
        offset_text, type_text, symbol = parts[:3]
        expected_addend = int(parts[3], 0) if len(parts) == 4 else 0
        offset = int(offset_text, 0)
        relocation_type = RELOCATION_TYPES[type_text.upper()]
    except (KeyError, ValueError):
        fail(f"invalid relocation specification {argument!r}")
    if offset < 0 or offset % 4:
        fail(f"unaligned relocation offset {offset:#x}")
    if not symbol or offset in requests:
        fail(f"invalid or duplicate relocation offset {offset:#x}")
    limit = 0x3FFFFFF if relocation_type == 4 else 0xFFFF
    if expected_addend < 0 or expected_addend > limit:
        fail(f"relocation addend outside encoded field range: {expected_addend}")
    requests[offset] = (relocation_type, symbol, expected_addend)

data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    fail("expected a big-endian ELF32 object")

section_header_offset = struct.unpack_from(">I", data, 0x20)[0]
section_header_size = struct.unpack_from(">H", data, 0x2E)[0]
section_count = struct.unpack_from(">H", data, 0x30)[0]
section_name_index = struct.unpack_from(">H", data, 0x32)[0]
if section_header_size < 40 or section_name_index >= section_count:
    fail("malformed ELF section table")

headers = []
for index in range(section_count):
    offset = section_header_offset + index * section_header_size
    if offset + 40 > len(data):
        fail("ELF section table extends past end of file")
    headers.append(list(struct.unpack_from(">10I", data, offset)))

name_header = headers[section_name_index]
section_names = data[name_header[4] : name_header[4] + name_header[5]]
names = [c_string(section_names, header[0], f"section {index}") for index, header in enumerate(headers)]
target_indices = [index for index, name in enumerate(names) if name == section_name]
if len(target_indices) != 1:
    fail(f"expected exactly one section named {section_name!r}")
target_index = target_indices[0]
target = headers[target_index]
if hash_size <= 0 or hash_size > target[5] or hash_size % 4:
    fail(f"hash size {hash_size:#x} outside aligned section size {target[5]:#x}")
actual_digest = hashlib.sha256(data[target[4] : target[4] + hash_size]).hexdigest()
if actual_digest != expected_digest:
    fail(
        f"{path}: configured {section_name} prefix SHA-256 mismatch: "
        f"expected {expected_digest}, got {actual_digest}"
    )
for offset in requests:
    if offset + 4 > hash_size:
        fail(f"relocation offset {offset:#x} outside hashed prefix")

relocation_indices = [
    index
    for index, header in enumerate(headers)
    if header[1] == SHT_REL and header[7] == target_index
]
if len(relocation_indices) != 1:
    fail(f"expected exactly one REL section for {section_name!r}")
relocation_index = relocation_indices[0]
relocation = headers[relocation_index]
entry_size = relocation[9] or 8
if entry_size != 8 or relocation[5] % entry_size:
    fail("expected an ELF32 REL table with eight-byte entries")

symbol_table_index = relocation[6]
if symbol_table_index >= section_count or headers[symbol_table_index][1] != SHT_SYMTAB:
    fail("relocation section does not link to a symbol table")
symbol_table = headers[symbol_table_index]
symbol_entry_size = symbol_table[9] or 16
if symbol_entry_size < 16 or symbol_table[5] % symbol_entry_size:
    fail("malformed symbol table")
string_table_index = symbol_table[6]
if string_table_index >= section_count:
    fail("symbol table has invalid string table")
string_table = headers[string_table_index]
strings = data[string_table[4] : string_table[4] + string_table[5]]
symbols = {}
for index, entry_offset in enumerate(
    range(symbol_table[4], symbol_table[4] + symbol_table[5], symbol_entry_size)
):
    name_offset = struct.unpack_from(">I", data, entry_offset)[0]
    name = c_string(strings, name_offset, f"symbol {index}")
    if name:
        if name in symbols:
            fail(f"duplicate symbol name {name!r}")
        symbols[name] = index

entries = []
occupied = set()
start = relocation[4]
end = start + relocation[5]
for entry_offset in range(start, end, entry_size):
    instruction_offset, info = struct.unpack_from(">II", data, entry_offset)
    if instruction_offset in occupied:
        fail(f"multiple existing relocations at {instruction_offset:#x}")
    occupied.add(instruction_offset)
    entries.append((instruction_offset, info))

for instruction_offset, (relocation_type, symbol, expected_addend) in requests.items():
    if instruction_offset in occupied:
        fail(f"relocation already exists at {instruction_offset:#x}")
    if symbol not in symbols:
        fail(f"replacement symbol {symbol!r} does not exist in object")
    instruction = struct.unpack_from(">I", data, target[4] + instruction_offset)[0]
    opcode = instruction >> 26
    immediate = instruction & (0x3FFFFFF if relocation_type == 4 else 0xFFFF)
    if relocation_type == 4 and opcode != 0x03:
        fail(f"R26 relocation site {instruction_offset:#x} is not JAL")
    if relocation_type == 5 and opcode != 0x0F:
        fail(f"HI16 relocation site {instruction_offset:#x} is not LUI")
    # Retail proves direct LO16 relocation of address operands on SWC1
    # (Overlay 2 +0x6E0) and SW (Overlay 57 +0x4C18). The command still names
    # the exact expected addend, so broad opcode support cannot silently admit
    # a different compiler representation.
    if relocation_type == 6 and opcode in (0x00, 0x01, 0x02, 0x03):
        fail(f"LO16 relocation site {instruction_offset:#x} is not an I-type address operation")
    if immediate != expected_addend:
        fail(
            f"relocation site {instruction_offset:#x} addend mismatch: "
            f"expected {expected_addend:#x}, got {immediate:#x}"
        )
    entries.append((instruction_offset, (symbols[symbol] << 8) | relocation_type))

entries.sort()
new_table = b"".join(struct.pack(">II", offset, info) for offset, info in entries)
delta = len(new_table) - relocation[5]
if delta <= 0:
    fail("no relocation records were added")

# Insert the enlarged table and shift every later file-backed section. Section
# virtual addresses and all section contents remain unchanged.
data[start:end] = new_table
new_section_header_offset = section_header_offset + (delta if section_header_offset >= end else 0)
struct.pack_into(">I", data, 0x20, new_section_header_offset)
relocation[5] = len(new_table)
for index, header in enumerate(headers):
    if index != relocation_index and header[1] != SHT_NOBITS and header[4] >= end:
        header[4] += delta
    destination = new_section_header_offset + index * section_header_size
    struct.pack_into(">10I", data, destination, *header)

path.write_bytes(data)
