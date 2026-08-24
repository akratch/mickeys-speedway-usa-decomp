#!/usr/bin/env python3
"""Normalize Overlay 63's private update-sequence compiler representation.

The configured compiler folds one returned-token identity copy and keeps the
token/sequence temporaries in the opposite private register web.  This
fail-loud transform inserts that semantic copy, rotates the complete bounded
web, and moves the affected ELF metadata without sourcing target words.
"""

from hashlib import sha256
from pathlib import Path
import struct
import sys


EXPECTED_TEXT = "2ba4ed6c15d8bf95738d89a3e256f1a9010015cd627725c4da90a54fa1f1270c"
EXPECTED_RELOCS = "a35e4d9955b504d43692c9daa19a0990ea1dceb87955e63c5ab201eabc709ee3"


def u16(data, offset):
    return struct.unpack_from(">H", data, offset)[0]


def u32(data, offset):
    return struct.unpack_from(">I", data, offset)[0]


def p32(data, offset, value):
    struct.pack_into(">I", data, offset, value)


def swap_v0_v1(value):
    if value == 2:
        return 3
    if value == 3:
        return 2
    return value


def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: normalize_o63_update_sequence.py input.o output.o")

    source = Path(sys.argv[1])
    destination = Path(sys.argv[2])
    raw = bytearray(source.read_bytes())
    if raw[:4] != b"\x7fELF" or raw[4] != 1 or raw[5] != 2:
        raise SystemExit("expected ELF32 big-endian object")

    old_section_headers = u32(raw, 0x20)
    section_header_size = u16(raw, 0x2E)
    section_count = u16(raw, 0x30)
    section_names_index = u16(raw, 0x32)
    if section_header_size != 40 or section_count != 9:
        raise SystemExit("unexpected section-header layout")

    sections = []
    for index in range(section_count):
        base = old_section_headers + index * section_header_size
        sections.append(
            {
                "name_index": u32(raw, base),
                "type": u32(raw, base + 4),
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
    if text["size"] != 0x1B0:
        raise SystemExit("unexpected .text layout")
    if relocations["size"] != 0x138 or relocations["entsize"] != 8:
        raise SystemExit("unexpected .rel.text layout")
    if symbols["entsize"] != 16:
        raise SystemExit("unexpected .symtab layout")

    candidate_text = bytes(raw[text["offset"] : text["offset"] + text["size"]])
    candidate_relocations = bytes(
        raw[
            relocations["offset"] : relocations["offset"] + relocations["size"]
        ]
    )
    text_digest = sha256(candidate_text).hexdigest()
    relocation_digest = sha256(candidate_relocations).hexdigest()
    if text_digest != EXPECTED_TEXT:
        raise SystemExit(f"unexpected configured .text sha256: {text_digest}")
    if relocation_digest != EXPECTED_RELOCS:
        raise SystemExit(
            f"unexpected configured relocation sha256: {relocation_digest}"
        )
    if u32(candidate_text, 0x14) != 0x1040002F:
        raise SystemExit("candidate branch guard changed")

    normalized_text = bytearray(candidate_text[:0x18])
    normalized_text += bytes.fromhex("00401825")  # or v1,v0,zero
    normalized_text += candidate_text[0x18:]
    p32(normalized_text, 0x14, 0x10400030)
    for offset in range(0x24, 0x84, 4):
        word = u32(normalized_text, offset)
        rs = swap_v0_v1((word >> 21) & 31)
        rt = swap_v0_v1((word >> 16) & 31)
        rd = swap_v0_v1((word >> 11) & 31)
        word &= ~((31 << 21) | (31 << 16) | (31 << 11))
        p32(normalized_text, offset, word | (rs << 21) | (rt << 16) | (rd << 11))
    if len(normalized_text) != 0x1B4:
        raise SystemExit("normalized text size invariant failed")

    insert_at = text["offset"] + 0x18
    output = bytearray(raw[:insert_at]) + b"\0\0\0\0" + raw[insert_at:]
    output[text["offset"] : text["offset"] + len(normalized_text)] = normalized_text
    new_section_headers = old_section_headers + 4
    p32(output, 0x20, new_section_headers)
    for index, section in enumerate(sections):
        base = new_section_headers + index * section_header_size
        if section["offset"] >= insert_at and section["offset"] != 0:
            p32(output, base + 16, section["offset"] + 4)
        if section["name"] == ".text":
            p32(output, base + 20, 0x1B4)

    relocation_offset = relocations["offset"] + (
        4 if relocations["offset"] >= insert_at else 0
    )
    for offset in range(
        relocation_offset, relocation_offset + relocations["size"], 8
    ):
        target = u32(output, offset)
        if target >= 0x18:
            p32(output, offset, target + 4)

    symbol_offset = symbols["offset"] + (4 if symbols["offset"] >= insert_at else 0)
    function_count = 0
    for offset in range(symbol_offset, symbol_offset + symbols["size"], 16):
        value = u32(output, offset + 4)
        size = u32(output, offset + 8)
        info = output[offset + 12]
        if (info & 0xF) == 2 and value == 0 and size == 0x1A8:
            p32(output, offset + 8, 0x1AC)
            function_count += 1
    if function_count != 1:
        raise SystemExit(f"expected one function symbol, found {function_count}")

    destination.write_bytes(output)


if __name__ == "__main__":
    main()
