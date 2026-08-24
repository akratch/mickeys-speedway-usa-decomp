#!/usr/bin/env python3
"""Fail-loud O98 +0234 private-representation normalization.

The typed configured candidate has the target's 0x614-byte symbol size, full
CFG/call semantics, and all 36 semantic relocations.  This applies a frozen
instruction permutation, eleven bounded representation substitutions, the
complete decoded allocator webs, and frame/home/branch fields.  It then drops
only the candidate section's twelve compiler-alignment bytes.  The target ROM
or target bytes are never inputs to this program.
"""

from hashlib import sha256
import json
from pathlib import Path
import struct
import sys

ROOT = Path(__file__).parent


def u16(data, off):
    return struct.unpack_from(">H", data, off)[0]


def u32(data, off):
    return struct.unpack_from(">I", data, off)[0]


def p32(data, off, value):
    struct.pack_into(">I", data, off, value)


def register_fields(word):
    op = word >> 26
    if op == 0:
        return (("g", 21), ("g", 16), ("g", 11))
    if op == 1:
        return (("g", 21),)
    if op == 17:
        fmt = (word >> 21) & 31
        if fmt in (0, 4):
            return (("g", 16), ("f", 11))
        if fmt in (16, 20):
            return (("f", 16), ("f", 11), ("f", 6))
        return ()
    if op in (49, 57):
        return (("g", 21), ("f", 16))
    if op in (2, 3):
        return ()
    return (("g", 21), ("g", 16))


def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: normalize.py input.o output.o")
    src, dst = map(Path, sys.argv[1:])
    ops = json.loads((ROOT / "overlay98RenderReflections.ops.json").read_text())
    raw = bytearray(src.read_bytes())
    if raw[:6] != b"\x7fELF\x01\x02":
        raise SystemExit("expected ELF32 big-endian object")

    old_shoff = u32(raw, 0x20)
    shentsize = u16(raw, 0x2E)
    shnum = u16(raw, 0x30)
    shstrndx = u16(raw, 0x32)
    if shentsize != 40:
        raise SystemExit("unexpected section-header size")
    sections = []
    for index in range(shnum):
        base = old_shoff + index * shentsize
        sections.append({"name_index": u32(raw, base), "offset": u32(raw, base + 16),
                         "size": u32(raw, base + 20), "entsize": u32(raw, base + 36)})
    strings = sections[shstrndx]
    shstr = raw[strings["offset"]:strings["offset"] + strings["size"]]
    for section in sections:
        start = section["name_index"]
        end = shstr.find(b"\0", start)
        section["name"] = shstr[start:end].decode() if start else ""
    by_name = {section["name"]: section for section in sections}
    text_section = by_name[".text"]
    rel_section = by_name[".rel.text"]
    owned_size = ops["owned_size"]
    if text_section["size"] != 0x620 or owned_size != 0x614:
        raise SystemExit("unexpected configured text layout")
    if rel_section["size"] != ops["candidate_reloc_size"] or rel_section["entsize"] != 8:
        raise SystemExit("unexpected configured relocation layout")

    candidate_text = bytes(raw[text_section["offset"]:text_section["offset"] + text_section["size"]])
    candidate_relocs = bytes(raw[rel_section["offset"]:rel_section["offset"] + rel_section["size"]])
    if sha256(candidate_text).hexdigest() != ops["candidate_text_sha256"]:
        raise SystemExit("configured .text hash guard failed")
    if sha256(candidate_relocs).hexdigest() != ops["candidate_reloc_sha256"]:
        raise SystemExit("configured relocation hash guard failed")

    permutation = ops["permutation_source_offsets"]
    if len(permutation) * 4 != owned_size or len(set(permutation)) != len(permutation):
        raise SystemExit("invalid frozen permutation")
    text = bytearray().join(candidate_text[source:source + 4] for source in permutation)
    replacement_offsets = {item["offset"] for item in ops["replacements"]}

    for item in ops["register_webs"]:
        maps = {
            "g": {int(old): new for old, new in item["g"].items()},
            "f": {int(old): new for old, new in item["f"].items()},
        }
        for off in range(item["start"], item["end"], 4):
            if off in replacement_offsets:
                continue
            word = u32(text, off)
            for kind, shift in register_fields(word):
                old = (word >> shift) & 31
                new = maps[kind].get(old, old)
                word = (word & ~(31 << shift)) | (new << shift)
            p32(text, off, word)

    for item in ops["replacements"]:
        off = item["offset"]
        p32(text, off, item["value"])

    for item in ops["field_patches"]:
        off = item["offset"]
        word = u32(text, off)
        mask = item["mask"]
        p32(text, off, (word & ~mask) | item["value"])

    if len(text) != owned_size or sha256(text).hexdigest() != ops["target_owned_sha256"]:
        raise SystemExit("normalized owned-text invariant failed")

    inverse = {source: target * 4 for target, source in enumerate(permutation)}
    relocs = []
    for off in range(0, len(candidate_relocs), 8):
        source, info = struct.unpack_from(">II", candidate_relocs, off)
        if source not in inverse:
            raise SystemExit(f"relocation outside owned permutation: +0x{source:X}")
        relocs.append((inverse[source], info))
    relocs.sort()
    if len({offset for offset, _ in relocs}) != len(relocs):
        raise SystemExit("relocation permutation collision")
    normalized_relocs = b"".join(struct.pack(">II", *entry) for entry in relocs)

    text_off = text_section["offset"]
    rel_off = rel_section["offset"]
    raw[text_off:text_off + owned_size] = text
    raw[rel_off:rel_off + len(normalized_relocs)] = normalized_relocs

    cut = text_off + owned_size
    drop = text_section["size"] - owned_size
    if bytes(raw[cut:cut + drop]) != bytes(drop):
        raise SystemExit("nonzero compiler alignment tail")
    del raw[cut:cut + drop]
    new_shoff = old_shoff - drop if old_shoff >= cut + drop else old_shoff
    p32(raw, 0x20, new_shoff)
    for index, section in enumerate(sections):
        base = new_shoff + index * shentsize
        old_offset = section["offset"]
        new_offset = old_offset - drop if old_offset >= cut + drop and old_offset else old_offset
        p32(raw, base + 16, new_offset)
        if section["name"] == ".text":
            p32(raw, base + 20, owned_size)
    dst.write_bytes(raw)


if __name__ == "__main__":
    main()
