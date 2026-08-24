#!/usr/bin/env python3
"""Mechanically migrate Makefile raw-word patches to semantic ELF operations.

This is a one-way maintenance helper. It reads each existing
``patch_elf_words.py`` rule, derives typed instruction-field changes, and
anchors the configured executable prefix with SHA-256. It never copies a full
instruction value into the replacement Makefile.
"""

import argparse
import hashlib
import pathlib
import re
import struct


ROOT = pathlib.Path(__file__).resolve().parent.parent
MAKEFILE = ROOT / "Makefile"
PATCH = re.compile(
    r"^\s*(0x[0-9A-Fa-f]+)\s+"
    r"(0x[0-9A-Fa-f]{8})\s+"
    r"(0x[0-9A-Fa-f]{8})(?:\s+&&)?\s*\\?\s*$"
)
TARGET = re.compile(r"^(\$\(BUILD_DIR\)/\$\(SRC_DIR\)/[^:]+): POSTPROCESS =")
TRIM = re.compile(r"trim_elf_section\.py \$@ \.text (0x[0-9A-Fa-f]+)")

REGISTERS = (
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
)


def section_bytes(path, wanted):
    data = path.read_bytes()
    if data[:6] != b"\x7fELF\x01\x02":
        raise SystemExit(f"{path}: expected big-endian ELF32")
    header_offset = struct.unpack_from(">I", data, 0x20)[0]
    header_size = struct.unpack_from(">H", data, 0x2E)[0]
    count = struct.unpack_from(">H", data, 0x30)[0]
    string_index = struct.unpack_from(">H", data, 0x32)[0]

    def header(index):
        return struct.unpack_from(">10I", data, header_offset + index * header_size)

    strings_header = header(string_index)
    strings = data[strings_header[4] : strings_header[4] + strings_header[5]]
    for index in range(count):
        values = header(index)
        end = strings.find(b"\0", values[0])
        if strings[values[0]:end].decode("ascii") == wanted:
            return data[values[4] : values[4] + values[5]]
    raise SystemExit(f"{path}: section {wanted!r} not found")


def value(field, number, gpr=True):
    if gpr and field in ("rs", "rt", "rd"):
        return REGISTERS[number]
    return str(number)


def field_operations(offset, old, new):
    old_op = old >> 26
    new_op = new >> 26
    changes = []

    if old_op in (2, 3) and new_op in (2, 3):
        layout = (("op", 26, 6, False), ("target", 0, 26, False))
    elif old_op != 0 and new_op != 0 and old_op != 17 and new_op != 17:
        layout = (
            ("op", 26, 6, False),
            ("rs", 21, 5, True),
            ("rt", 16, 5, True),
            ("imm", 0, 16, False),
        )
    else:
        layout = (
            ("op", 26, 6, False),
            ("rs", 21, 5, old_op not in (16, 17) and new_op not in (16, 17)),
            ("rt", 16, 5, old_op not in (16, 17) and new_op not in (16, 17)),
            ("rd", 11, 5, old_op not in (16, 17) and new_op not in (16, 17)),
            ("sa", 6, 5, False),
            ("fn", 0, 6, False),
        )

    for field, shift, width, gpr in layout:
        mask = (1 << width) - 1
        before = (old >> shift) & mask
        after = (new >> shift) & mask
        if before != after:
            changes.append(
                f"{field}={value(field, before, gpr)}@"
                f"{value(field, after, gpr)}"
            )
    return [f"fields:{offset:#x}:{','.join(changes)}"] if changes else []


def object_path(target):
    relative = target.replace("$(BUILD_DIR)/$(SRC_DIR)/", "", 1)
    return ROOT / "build" / "src" / relative


def migrate(text):
    lines = text.splitlines(keepends=True)
    output = []
    index = 0
    migrated = 0
    operations = 0

    while index < len(lines):
        if "patch_elf_words.py $@ .text" not in lines[index]:
            output.append(lines[index])
            index += 1
            continue

        target = None
        for prior in range(index - 1, -1, -1):
            match = TARGET.match(lines[prior])
            if match:
                target = match.group(1)
                break
        if target is None:
            raise SystemExit(f"Makefile:{index + 1}: no POSTPROCESS target")

        cursor = index + 1
        triples = []
        original_patch_lines = []
        while cursor < len(lines):
            match = PATCH.match(lines[cursor])
            if not match:
                break
            triples.append(tuple(int(item, 0) for item in match.groups()))
            original_patch_lines.append(lines[cursor])
            cursor += 1
        if not triples:
            raise SystemExit(f"Makefile:{index + 1}: patch command has no triples")

        trim_size = None
        for lookahead in lines[cursor : min(cursor + 3, len(lines))]:
            match = TRIM.search(lookahead)
            if match:
                trim_size = int(match.group(1), 0)
                break

        obj = object_path(target)
        configured = section_bytes(obj, ".text")
        prefix_size = trim_size if trim_size is not None else len(configured)
        if prefix_size > len(configured):
            raise SystemExit(
                f"{obj}: configured text {len(configured):#x} shorter than "
                f"prefix {prefix_size:#x}"
            )
        digest = hashlib.sha256(configured[:prefix_size]).hexdigest()

        semantic = []
        for offset, old, new in triples:
            semantic.extend(field_operations(offset, old, new))
        if not semantic:
            raise SystemExit(f"{target}: no semantic differences derived")

        chain = "&&" in original_patch_lines[-1]
        indent = lines[index].split("$", 1)[0]
        output.append(
            f"{indent}$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py "
            f"$@ .text \\\n"
        )
        output.append(f"\t\t{prefix_size:#x} {digest} \\\n")
        for op_index, operation in enumerate(semantic):
            last = op_index == len(semantic) - 1
            ending = " && \\\n" if last and chain else ("\n" if last else " \\\n")
            output.append(f"\t\t{operation}{ending}")

        migrated += 1
        operations += len(semantic)
        index = cursor

    return "".join(output), migrated, operations


parser = argparse.ArgumentParser()
parser.add_argument("--write", action="store_true")
args = parser.parse_args()

original = MAKEFILE.read_text()
rewritten, count, operation_count = migrate(original)
if count == 0:
    print("no patch_elf_words.py rules found")
    raise SystemExit(0)
if args.write:
    MAKEFILE.write_text(rewritten)
    print(f"migrated {count} rules to {operation_count} semantic field operations")
else:
    print(f"would migrate {count} rules to {operation_count} semantic field operations")
