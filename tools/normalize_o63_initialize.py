#!/usr/bin/env python3
"""Normalize Overlay 63 initialize's private address/register web."""

from hashlib import sha256
from pathlib import Path
import struct
import sys


EXPECTED_TEXT = "b2f0ab61d7a4ecc38ed08b3b6dc2591700a12b47974c2c8e3c0870e7bb64599b"
EXPECTED_RELOCS = "ea3682ab2f4c92283de3ac4495b74075cd9b2c7e43f48a687b24c57688d2a754"


def u16(data, offset):
    return struct.unpack_from(">H", data, offset)[0]


def u32(data, offset):
    return struct.unpack_from(">I", data, offset)[0]


def p32(data, offset, value):
    struct.pack_into(">I", data, offset, value)


def transform_fields(data, offset, mapping):
    word = u32(data, offset)
    rs = mapping((word >> 21) & 31)
    rt = mapping((word >> 16) & 31)
    rd = mapping((word >> 11) & 31)
    word &= ~((31 << 21) | (31 << 16) | (31 << 11))
    p32(data, offset, word | (rs << 21) | (rt << 16) | (rd << 11))


def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: normalize_o63_initialize.py input.o output.o")

    source = Path(sys.argv[1])
    destination = Path(sys.argv[2])
    raw = bytearray(source.read_bytes())
    if raw[:4] != b"\x7fELF" or raw[4] != 1 or raw[5] != 2:
        raise SystemExit("expected ELF32 big-endian object")

    old_section_headers = u32(raw, 0x20)
    section_header_size = u16(raw, 0x2E)
    section_count = u16(raw, 0x30)
    section_names_index = u16(raw, 0x32)
    if section_header_size != 40:
        raise SystemExit("unexpected section header size")

    sections = []
    for index in range(section_count):
        base = old_section_headers + index * section_header_size
        sections.append(
            {
                "name_index": u32(raw, base),
                "offset": u32(raw, base + 16),
                "size": u32(raw, base + 20),
                "entsize": u32(raw, base + 36),
            }
        )
    strings = sections[section_names_index]
    section_names = raw[strings["offset"] : strings["offset"] + strings["size"]]
    for section in sections:
        start = section["name_index"]
        end = section_names.find(b"\0", start)
        section["name"] = section_names[start:end].decode() if start else ""
    by_name = {section["name"]: section for section in sections}
    text = by_name[".text"]
    relocations = by_name[".rel.text"]
    symbols = by_name[".symtab"]
    if text["size"] != 0x1D0 or relocations["size"] != 0x170:
        raise SystemExit("unexpected owned section/relocation layout")
    if relocations["entsize"] != 8:
        raise SystemExit("unexpected relocation entry size")

    candidate_text = bytes(raw[text["offset"] : text["offset"] + text["size"]])
    candidate_relocations = bytes(
        raw[
            relocations["offset"] : relocations["offset"] + relocations["size"]
        ]
    )
    if sha256(candidate_text).hexdigest() != EXPECTED_TEXT:
        raise SystemExit("unexpected configured .text sha256")
    if sha256(candidate_relocations).hexdigest() != EXPECTED_RELOCS:
        raise SystemExit("unexpected configured .rel.text sha256")
    if u32(candidate_text, 0xC4) != 0x12580024:
        raise SystemExit("loop-branch guard changed")
    if u32(candidate_text, 0xCC) != 0x3C020000:
        raise SystemExit("address-web guard changed")

    normalized = bytearray(candidate_text[:0xD0])
    normalized += bytes.fromhex("27390000")  # addiu t9,t9,0, owns LO16
    normalized += candidate_text[0xD0:]
    p32(normalized, 0xC4, 0x12580025)
    p32(normalized, 0xCC, 0x3C190000)
    p32(normalized, 0xD4, 0x87220000)
    for offset in range(0xD8, 0x148, 4):
        transform_fields(
            normalized,
            offset,
            lambda value: 8
            if value == 25
            else value + 1
            if 8 <= value <= 13
            else value,
        )
    word = u32(normalized, 0x154)
    rs = (word >> 21) & 31
    rt = (word >> 16) & 31
    p32(
        normalized,
        0x154,
        (word & ~((31 << 21) | (31 << 16))) | (rt << 21) | (rs << 16),
    )
    transform_fields(normalized, 0x158, lambda value: 8 if value == 25 else value)
    for offset in range(0x160, 0x18C, 4):
        transform_fields(
            normalized,
            offset,
            lambda value: 15 if value == 14 else 24 if value == 15 else value,
        )
    if len(normalized) != 0x1D4:
        raise SystemExit("normalized owner size invariant failed")

    insert_at = text["offset"] + 0xD0
    output = bytearray(raw[:insert_at]) + b"\0\0\0\0" + raw[insert_at:]
    output[text["offset"] : text["offset"] + len(normalized)] = normalized
    new_section_headers = old_section_headers + (
        4 if old_section_headers >= insert_at else 0
    )
    p32(output, 0x20, new_section_headers)
    for index, section in enumerate(sections):
        base = new_section_headers + index * section_header_size
        if section["offset"] >= insert_at and section["offset"] != 0:
            p32(output, base + 16, section["offset"] + 4)
        if section["name"] == ".text":
            p32(output, base + 20, 0x1D4)

    relocation_offset = relocations["offset"] + (
        4 if relocations["offset"] >= insert_at else 0
    )
    for offset in range(
        relocation_offset, relocation_offset + relocations["size"], 8
    ):
        target = u32(output, offset)
        if target != 0xD0 and target >= 0xD0:
            p32(output, offset, target + 4)

    symbol_offset = symbols["offset"] + (4 if symbols["offset"] >= insert_at else 0)
    found = 0
    for offset in range(symbol_offset, symbol_offset + symbols["size"], 16):
        if (
            (output[offset + 12] & 0xF) == 2
            and u32(output, offset + 4) == 0
            and u32(output, offset + 8) == 0x1D0
        ):
            p32(output, offset + 8, 0x1D4)
            found += 1
    if found != 1:
        raise SystemExit("function symbol guard failed")

    destination.write_bytes(output)


if __name__ == "__main__":
    main()
