#!/usr/bin/env python3
"""Apply reviewed semantic instruction transformations to a big-endian ELF.

Unlike ``patch_elf_words.py``, this tool never stores complete instruction
words.  Each operation names the instruction field being changed, or an
instruction scheduling permutation.  A SHA-256 over the configured executable
prefix makes the transformation fail loudly if any unmentioned compiler output
drifts.

Usage:

    normalize_elf_instructions.py OBJECT SECTION SIZE SHA256 OP [OP ...]

For large reviewed transformations, an argument of ``@PATH`` expands one
operation per non-empty, non-comment line from PATH.  The expanded operations
remain subject to the same decoded-field assertions and final digest.

Operations are evaluated from left to right:

    set:OFFSET:FIELD:EXPECTED:REPLACEMENT
        Change one decoded field. FIELD is op, rs, rt, rd, sa, fn, imm, or
        target. GPR fields accept ABI register names (for example s0 or t7).

    fields:OFFSET:FIELD=EXPECTED@REPLACEMENT[,FIELD=EXPECTED@REPLACEMENT...]
        Change several non-overlapping decoded fields of one instruction.

    reorder:DEST=SOURCE[,DEST=SOURCE...]
        Simultaneously reorder complete instructions. Every destination and
        source must occur exactly once, making this a true permutation rather
        than an instruction injection. Any relocation attached to a moved
        instruction moves with it; multiple relocations on one moved
        instruction are rejected as ambiguous.

    drop-copy:OFFSET:SRC:DST:SYMBOL
        Delete one asserted ``or DST,SRC,zero`` from a single function whose
        natural size is exactly one word larger than SIZE. The site must own no
        relocation or incoming branch. Later relocations and crossing local
        branch displacements move with the shortened suffix, the function
        symbol shrinks by one word, and the vacated final word is zeroed for a
        following trim step. GPR operands accept ABI register names.

    drop-li:OFFSET:DST:IMM:SYMBOL
        Delete one asserted ``addiu DST,zero,IMM`` rematerialization under the
        same single-function, relocation, branch, suffix, and size guards as
        drop-copy. The reviewed field web must preserve the earlier producer
        and every intervening consumer of the complete value web.

    drop-branch:OFFSET:TARGET:SYMBOL
        Delete one asserted unconditional ``beq zero,zero,TARGET`` from a
        single function under the same ownership, relocation, incoming-branch,
        suffix, and size guards. This is reserved for a reviewed control-flow
        web in which the neighboring conditional branch is rewritten to own
        the same back edge.

    commute:OFFSET
        Swap the rs and rt fields of one instruction.

SIZE is the executable prefix to hash. A following trim step may separately
own compiler section alignment outside that prefix.
"""

import hashlib
import pathlib
import struct
import sys


REGISTERS = {
    "zero": 0,
    "at": 1,
    "v0": 2,
    "v1": 3,
    "a0": 4,
    "a1": 5,
    "a2": 6,
    "a3": 7,
    "t0": 8,
    "t1": 9,
    "t2": 10,
    "t3": 11,
    "t4": 12,
    "t5": 13,
    "t6": 14,
    "t7": 15,
    "s0": 16,
    "s1": 17,
    "s2": 18,
    "s3": 19,
    "s4": 20,
    "s5": 21,
    "s6": 22,
    "s7": 23,
    "t8": 24,
    "t9": 25,
    "k0": 26,
    "k1": 27,
    "gp": 28,
    "sp": 29,
    "fp": 30,
    "ra": 31,
}

FIELDS = {
    "op": (26, 6),
    "rs": (21, 5),
    "rt": (16, 5),
    "rd": (11, 5),
    "sa": (6, 5),
    "fn": (0, 6),
    "imm": (0, 16),
    "target": (0, 26),
}


def fail(message):
    raise SystemExit(message)


def parse_value(field, value):
    if field in ("rs", "rt", "rd") and value.lower() in REGISTERS:
        return REGISTERS[value.lower()]
    return int(value, 0)


def elf_sections(data):
    if data[:6] != b"\x7fELF\x01\x02":
        fail("expected a big-endian ELF32 object")

    header_offset = struct.unpack_from(">I", data, 0x20)[0]
    header_size = struct.unpack_from(">H", data, 0x2E)[0]
    count = struct.unpack_from(">H", data, 0x30)[0]
    string_index = struct.unpack_from(">H", data, 0x32)[0]

    def header(index):
        offset = header_offset + index * header_size
        return struct.unpack_from(">10I", data, offset)

    headers = [header(index) for index in range(count)]
    string_header = headers[string_index]
    strings = data[string_header[4] : string_header[4] + string_header[5]]
    sections = []
    for index, values in enumerate(headers):
        name_offset = values[0]
        end = strings.find(b"\0", name_offset)
        name = strings[name_offset:end].decode("ascii")
        sections.append((index, name, values))
    return sections


def elf_section(data, wanted):
    sections = elf_sections(data)
    for index, name, values in sections:
        if name == wanted:
            return index, values[4], values[5], sections
    fail(f"section {wanted!r} not found")


def relocation_entries(data, sections, target_index, moved_offsets):
    """Return file positions of relocations attached to moved instructions."""
    attached = {offset: [] for offset in moved_offsets}
    for _, name, values in sections:
        section_type = values[1]
        section_offset = values[4]
        section_size = values[5]
        target_section = values[7]
        entry_size = values[9]
        if section_type not in (4, 9) or target_section != target_index:
            continue
        expected_size = 12 if section_type == 4 else 8
        if entry_size == 0:
            entry_size = expected_size
        if entry_size < expected_size or section_size % entry_size:
            fail(f"malformed relocation section {name!r}")
        for entry_offset in range(section_offset, section_offset + section_size, entry_size):
            instruction_offset = struct.unpack_from(">I", data, entry_offset)[0]
            if instruction_offset in attached:
                attached[instruction_offset].append((name, entry_offset))
    for instruction_offset, entries in attached.items():
        if len(entries) > 1:
            names = ", ".join(name for name, _ in entries)
            fail(
                f"multiple relocations at {instruction_offset:#x} in "
                f"{names}; reorder is ambiguous"
            )
    return attached


def function_symbol(data, sections, symbol_name):
    matches = []
    for _, _, values in sections:
        if values[1] != 2:
            continue
        strings = sections[values[6]][2]
        string_data = data[strings[4] : strings[4] + strings[5]]
        entry_size = values[9] or 16
        for entry in range(values[4], values[4] + values[5], entry_size):
            name_offset, value, size = struct.unpack_from(">III", data, entry)
            end = string_data.find(b"\0", name_offset)
            name = string_data[name_offset:end].decode("ascii")
            if name != symbol_name:
                continue
            matches.append(
                (
                    entry,
                    value,
                    size,
                    data[entry + 12],
                    struct.unpack_from(">H", data, entry + 14)[0],
                )
            )
    if len(matches) != 1:
        fail(f"expected exactly one function symbol {symbol_name!r}")
    return matches[0]


def read_word(data, base, size, offset):
    if offset < 0 or offset + 4 > size or offset % 4:
        fail(f"instruction offset {offset:#x} outside aligned section size {size:#x}")
    return struct.unpack_from(">I", data, base + offset)[0]


def write_word(data, base, offset, word):
    struct.pack_into(">I", data, base + offset, word)


if len(sys.argv) < 6:
    fail(
        "usage: normalize_elf_instructions.py OBJECT SECTION SIZE SHA256 "
        "OP [OP ...]"
    )

path = pathlib.Path(sys.argv[1])
section_name = sys.argv[2]
hash_size = int(sys.argv[3], 0)
expected_digest = sys.argv[4].lower()
if len(expected_digest) != 64 or any(c not in "0123456789abcdef" for c in expected_digest):
    fail("expected a lowercase SHA-256 digest")

operations = []
for argument in sys.argv[5:]:
    if not argument.startswith("@"):
        operations.append(argument)
        continue
    operation_path = pathlib.Path(argument[1:])
    if not operation_path.is_file():
        fail(f"operation file {operation_path} not found")
    for line_number, line in enumerate(operation_path.read_text().splitlines(), 1):
        operation = line.strip()
        if not operation or operation.startswith("#"):
            continue
        if any(character.isspace() for character in operation):
            fail(
                f"{operation_path}:{line_number}: operation may not contain whitespace"
            )
        operations.append(operation)

data = bytearray(path.read_bytes())
section_index, section_offset, section_size, sections = elf_section(data, section_name)
if hash_size <= 0 or hash_size > section_size or hash_size % 4:
    fail(f"hash size {hash_size:#x} outside aligned section size {section_size:#x}")

for operation in operations:
    parts = operation.split(":")
    kind = parts[0]

    if kind in ("set", "fields"):
        if kind == "set" and len(parts) == 5:
            changes = [(parts[2], parts[3], parts[4])]
        elif kind == "fields" and len(parts) == 3:
            changes = []
            for change in parts[2].split(","):
                field, values = change.split("=", 1)
                expected, replacement = values.split("@", 1)
                changes.append((field, expected, replacement))
        else:
            fail(f"invalid semantic operation {operation!r}")
        offset = int(parts[1], 0)
        word = read_word(data, section_offset, section_size, offset)
        occupied = 0
        for field, expected_text, replacement_text in changes:
            if field not in FIELDS:
                fail(f"unknown instruction field {field!r}")
            shift, width = FIELDS[field]
            mask = (1 << width) - 1
            field_mask = mask << shift
            if occupied & field_mask:
                fail(f"overlapping fields in {operation!r}")
            occupied |= field_mask
            expected = parse_value(field, expected_text)
            replacement = parse_value(field, replacement_text)
            if expected & ~mask or replacement & ~mask:
                fail(f"{field} value outside {width}-bit field in {operation!r}")
            actual = (word >> shift) & mask
            if actual != expected:
                fail(
                    f"{path}: {section_name}+{offset:#x} {field} expected "
                    f"{expected:#x}, got {actual:#x}"
                )
            word = (word & ~field_mask) | (replacement << shift)
        write_word(data, section_offset, offset, word)
        continue

    if kind == "commute" and len(parts) == 2:
        offset = int(parts[1], 0)
        word = read_word(data, section_offset, section_size, offset)
        rs = (word >> 21) & 0x1F
        rt = (word >> 16) & 0x1F
        word = (word & ~((0x1F << 21) | (0x1F << 16))) | (rt << 21) | (rs << 16)
        write_word(data, section_offset, offset, word)
        continue

    if kind == "reorder" and len(parts) == 2:
        mappings = []
        for mapping in parts[1].split(","):
            destination, source = mapping.split("=", 1)
            mappings.append((int(destination, 0), int(source, 0)))
        destinations = [item[0] for item in mappings]
        sources = [item[1] for item in mappings]
        if len(set(destinations)) != len(destinations) or sorted(destinations) != sorted(sources):
            fail(f"reorder must be a one-to-one permutation: {operation!r}")
        snapshot = {
            offset: read_word(data, section_offset, section_size, offset)
            for offset in set(destinations + sources)
        }
        attached = relocation_entries(data, sections, section_index, set(sources))
        source_to_destination = {source: destination for destination, source in mappings}
        for destination, source in mappings:
            write_word(data, section_offset, destination, snapshot[source])
        for source, entries in attached.items():
            if entries:
                _, entry_offset = entries[0]
                struct.pack_into(">I", data, entry_offset, source_to_destination[source])
        continue

    is_value_drop = kind in ("drop-copy", "drop-li") and len(parts) == 5
    is_branch_drop = kind == "drop-branch" and len(parts) == 4
    if is_value_drop or is_branch_drop:
        offset = int(parts[1], 0)
        if kind == "drop-copy":
            src = parse_value("rs", parts[2])
            dst = parse_value("rd", parts[3])
        elif kind == "drop-li":
            dst = parse_value("rt", parts[2])
            expected_immediate = int(parts[3], 0)
            if not -0x8000 <= expected_immediate <= 0xFFFF:
                fail(f"drop-li immediate outside 16-bit field in {operation!r}")
        else:
            expected_target = int(parts[2], 0)
        symbol_name = parts[-1]
        symbol_entry, symbol_start, symbol_size, symbol_info, symbol_section = (
            function_symbol(data, sections, symbol_name)
        )
        if symbol_start != 0 or symbol_section != section_index or (symbol_info & 0xF) != 2:
            fail(f"unexpected ownership for function symbol {symbol_name!r}")
        if symbol_size != hash_size + 4:
            fail(
                f"{symbol_name} must own exactly one word beyond SIZE: "
                f"symbol={symbol_size:#x}, SIZE={hash_size:#x}"
            )
        word = read_word(data, section_offset, symbol_size, offset)
        if kind == "drop-copy":
            fields = (
                (word >> 26) & 0x3F,
                (word >> 21) & 0x1F,
                (word >> 16) & 0x1F,
                (word >> 11) & 0x1F,
                (word >> 6) & 0x1F,
                word & 0x3F,
            )
            if fields != (0, src, 0, dst, 0, 0x25):
                fail(
                    f"{path}: {section_name}+{offset:#x} is not the asserted "
                    f"or-copy {parts[3]},{parts[2]},zero"
                )
        elif kind == "drop-li":
            fields = (
                (word >> 26) & 0x3F,
                (word >> 21) & 0x1F,
                (word >> 16) & 0x1F,
                word & 0xFFFF,
            )
            if fields != (9, 0, dst, expected_immediate & 0xFFFF):
                fail(
                    f"{path}: {section_name}+{offset:#x} is not the asserted "
                    f"addiu-li {parts[2]},{parts[3]}"
                )
        else:
            immediate = word & 0xFFFF
            if immediate & 0x8000:
                immediate -= 0x10000
            actual_target = offset + 4 + immediate * 4
            fields = (
                (word >> 26) & 0x3F,
                (word >> 21) & 0x1F,
                (word >> 16) & 0x1F,
            )
            if fields != (4, 0, 0) or actual_target != expected_target:
                fail(
                    f"{path}: {section_name}+{offset:#x} is not the asserted "
                    f"unconditional branch to {expected_target:#x}"
                )

        for _, name, values in sections:
            section_type = values[1]
            if section_type not in (4, 9) or values[7] != section_index:
                continue
            entry_size = values[9] or (12 if section_type == 4 else 8)
            if values[5] % entry_size:
                fail(f"malformed relocation section {name!r}")
            for entry in range(values[4], values[4] + values[5], entry_size):
                relocation_offset = struct.unpack_from(">I", data, entry)[0]
                if relocation_offset == offset:
                    fail(f"{kind} site {offset:#x} unexpectedly owns a relocation")
                if offset < relocation_offset < symbol_size:
                    struct.pack_into(">I", data, entry, relocation_offset - 4)

        branch_opcodes = {1, 4, 5, 6, 7, 20, 21, 22, 23}
        for instruction_offset in range(0, symbol_size, 4):
            if instruction_offset == offset:
                continue
            position = section_offset + instruction_offset
            instruction = struct.unpack_from(">I", data, position)[0]
            opcode = (instruction >> 26) & 0x3F
            if opcode not in branch_opcodes:
                continue
            immediate = instruction & 0xFFFF
            if immediate & 0x8000:
                immediate -= 0x10000
            target = instruction_offset + 4 + immediate * 4
            if target == offset:
                fail(f"a local branch targets {kind} site {offset:#x}")
            adjusted = immediate
            if instruction_offset < offset < target:
                adjusted -= 1
            elif target < offset < instruction_offset:
                adjusted += 1
            if adjusted == immediate:
                continue
            if not -0x8000 <= adjusted <= 0x7FFF:
                fail("adjusted branch displacement overflowed")
            instruction = (instruction & 0xFFFF0000) | (adjusted & 0xFFFF)
            struct.pack_into(">I", data, position, instruction)

        start = section_offset + offset
        end = section_offset + symbol_size
        data[start : end - 4] = data[start + 4 : end]
        data[end - 4 : end] = b"\0\0\0\0"
        struct.pack_into(">I", data, symbol_entry + 8, symbol_size - 4)
        continue

    fail(f"invalid semantic operation {operation!r}")

actual_digest = hashlib.sha256(
    data[section_offset : section_offset + hash_size]
).hexdigest()
if actual_digest != expected_digest:
    fail(
        f"{path}: configured {section_name} prefix SHA-256 mismatch: "
        f"expected {expected_digest}, got {actual_digest}"
    )

path.write_bytes(data)
