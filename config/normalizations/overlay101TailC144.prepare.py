#!/usr/bin/env python3
"""Normalize two complete private representation webs in O101 +C144.

The compiler materializes the constant 255 twice before the creator call and
homes a volatile 1.0f automatic.  Retail retains one integer constant carrier
and rematerializes the floating constant at its five consumers.  This guarded
pass performs only those whole-web transformations while translating CFG,
symbols, and relocations.
"""

import hashlib
import pathlib
import struct
import sys

EXPECTED = "9da860f6f7f2f2b4cd23b0c3148a3f497a83189f9c49340e96375bf1efc180fa"
SYMBOL = "func_overlay_101_F000C144_18E7964"
NATURAL_SIZE = 0x5A0
OWNED_SIZE = 0x5A4
DROPS = (0x0F0, 0x1CC, 0x1D4, 0x1F0)
EXPANDS = (0x240, 0x2F4, 0x3A8, 0x45C, 0x518)
OPAQUE_WEB = {
    0x020: (0x240800FF, 0x240600FF),  # li t0,255 -> li a2,255
    0x054: (0xA2280032, 0xA2260032),  # sb t0,0x32(s1) -> a2
    0x058: (0xA2280033, 0xA2260033),
    0x0DC: (0xA0680012, 0xA0660012),
}


def cstr(blob, offset):
    end = blob.find(b"\0", offset)
    return blob[offset:end].decode()


if len(sys.argv) != 2:
    raise SystemExit("usage: prepare_c144_semantic_web.py OBJECT")
path = pathlib.Path(sys.argv[1])
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit("expected big-endian ELF32")

shoff = struct.unpack_from(">I", data, 0x20)[0]
shentsize = struct.unpack_from(">H", data, 0x2E)[0]
shnum = struct.unpack_from(">H", data, 0x30)[0]
shstrndx = struct.unpack_from(">H", data, 0x32)[0]
headers = [list(struct.unpack_from(">10I", data, shoff + i * shentsize))
           for i in range(shnum)]
shstr = headers[shstrndx]
names = data[shstr[4]:shstr[4] + shstr[5]]
section_names = [cstr(names, h[0]) for h in headers]
if section_names.count(".text") != 1:
    raise SystemExit("expected one .text")
text_index = section_names.index(".text")
text = headers[text_index]
base, size = text[4], text[5]
body = bytes(data[base:base + size])
if size != NATURAL_SIZE or hashlib.sha256(body).hexdigest() != EXPECTED:
    raise SystemExit("c144_v06 natural text drift")

words = list(struct.unpack(">360I", body))
for offset, (old, new) in OPAQUE_WEB.items():
    if words[offset // 4] != old:
        raise SystemExit(f"opaque web drift at {offset:#x}")
    words[offset // 4] = new
expected_drops = {
    0x0F0: 0x240600FF,
    0x1CC: 0x3C013F80,
    0x1D4: 0x44814000,
    0x1F0: 0xE7A8003C,
}
for offset, word in expected_drops.items():
    if words[offset // 4] != word:
        raise SystemExit(f"drop web drift at {offset:#x}: {words[offset // 4]:08x}")
for offset in EXPANDS:
    if words[offset // 4] != 0xC7A4003C:
        raise SystemExit(f"expected lwc1 f4,0x3c(sp) at {offset:#x}")


def translate(offset):
    return (offset
            - 4 * sum(drop < offset for drop in DROPS)
            + 4 * sum(expand < offset for expand in EXPANDS))


new_words = []
for old_index, word in enumerate(words):
    old_offset = old_index * 4
    if old_offset in DROPS:
        continue
    emitted = [word]
    if old_offset in EXPANDS:
        emitted = [0x3C013F80, 0x44812000]
    for emitted_index, emitted_word in enumerate(emitted):
        # Only original branch instructions require target translation.  The
        # inserted constant producers are non-control instructions.
        if emitted_index == 0:
            opcode = emitted_word >> 26
            if opcode in (1, 4, 5, 6, 7, 20, 21, 22, 23):
                immediate = emitted_word & 0xFFFF
                if immediate & 0x8000:
                    immediate -= 0x10000
                old_target = old_offset + 4 + immediate * 4
                new_offset = translate(old_offset)
                new_target = translate(old_target)
                immediate = (new_target - (new_offset + 4)) // 4
                emitted_word = ((emitted_word & 0xFFFF0000)
                                | (immediate & 0xFFFF))
        new_words.append(emitted_word)
if len(new_words) != 361:
    raise SystemExit("prepared instruction inventory drift")

prepared = struct.pack(">361I", *new_words)

# Grow .text by one word without discarding its relocation section.  Every
# later file-backed section and the section-header table move by four bytes.
cut = base + NATURAL_SIZE
data[cut:cut] = b"\0" * 4
old_shoff = shoff
shoff += 4
struct.pack_into(">I", data, 0x20, shoff)
for index, header in enumerate(headers):
    if header[4] >= cut:
        header[4] += 4
    if index == text_index:
        header[5] = OWNED_SIZE
    struct.pack_into(">10I", data, shoff + index * shentsize, *header)
data[base:base + OWNED_SIZE] = prepared

# Move every retained text relocation with its source instruction.  None of
# the dropped/expanded representation instructions owns a relocation.
for header in headers:
    if header[1] != 9 or header[7] != text_index:
        continue
    entsize = header[9] or 8
    for pos in range(header[4], header[4] + header[5], entsize):
        offset = struct.unpack_from(">I", data, pos)[0]
        if offset in DROPS or offset in EXPANDS:
            raise SystemExit(f"representation instruction owns relocation {offset:#x}")
        struct.pack_into(">I", data, pos, translate(offset))

# Translate text symbols and grow the sole owner and section view.
found = 0
for header in headers:
    if header[1] != 2:
        continue
    strings_header = headers[header[6]]
    strings = data[strings_header[4]:strings_header[4] + strings_header[5]]
    entsize = header[9] or 16
    for pos in range(header[4], header[4] + header[5], entsize):
        name_off, value, sym_size = struct.unpack_from(">III", data, pos)
        shndx = struct.unpack_from(">H", data, pos + 14)[0]
        info = data[pos + 12]
        name = cstr(strings, name_off)
        if shndx == text_index and value <= NATURAL_SIZE:
            struct.pack_into(">I", data, pos + 4, translate(value))
        if shndx == text_index and (info & 0xF) == 3:
            struct.pack_into(">I", data, pos + 8, OWNED_SIZE)
        if name == SYMBOL:
            if (value, sym_size, shndx) != (0, NATURAL_SIZE, text_index):
                raise SystemExit("owned function symbol drift")
            struct.pack_into(">I", data, pos + 8, OWNED_SIZE)
            found += 1
if found != 1:
    raise SystemExit("owned function symbol missing")

path.write_bytes(data)
