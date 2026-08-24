#!/usr/bin/env python3
"""Fail-loud O98 +0144 frame/home/schedule normalization."""

from hashlib import sha256
from pathlib import Path
import struct
import sys

TEXT_HASH = "987b16b9e7c617a696dadfab9617de52f6314a0a028befa339a4470a0ea76f40"
RELOC_HASH = "aed92fcd8c6e1d1ef788e1d480ca1d21ee7c900ea99317f797b2eb62f1060aa2"


def u16(b, o): return struct.unpack_from(">H", b, o)[0]
def u32(b, o): return struct.unpack_from(">I", b, o)[0]
def p32(b, o, v): struct.pack_into(">I", b, o, v)


def reg(word, shift, old, new):
    if ((word >> shift) & 31) != old:
        raise SystemExit("register-field guard changed")
    return (word & ~(31 << shift)) | (new << shift)


def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: normalize.py input.o output.o")
    raw = bytearray(Path(sys.argv[1]).read_bytes())
    shoff = u32(raw, 0x20); shnum = u16(raw, 0x30); shstrndx = u16(raw, 0x32)
    sections = []
    for i in range(shnum):
        o = shoff + i * 40
        sections.append((u32(raw, o), u32(raw, o + 16), u32(raw, o + 20)))
    no, so, ss = sections[shstrndx]; strings = raw[so:so + ss]
    named = {}
    for no, off, size in sections:
        end = strings.find(b"\0", no)
        name = strings[no:end].decode() if no else ""
        named[name] = (off, size)
    to, ts = named[".text"]; ro, rs = named[".rel.text"]
    text = bytearray(raw[to:to + ts]); rel = bytes(raw[ro:ro + rs])
    if ts != 0xF0 or rs != 0x30:
        raise SystemExit("unexpected section layout")
    if sha256(text).hexdigest() != TEXT_HASH or sha256(rel).hexdigest() != RELOC_HASH:
        raise SystemExit("configured object hash guard failed")

    if u32(text, 0) != 0x27BDFFA8 or u32(text, 0xEC) != 0x27BD0058:
        raise SystemExit("frame guard changed")
    p32(text, 0, 0x27BDFFB0)
    p32(text, 0x58, 0x27B40044)
    p32(text, 0xEC, 0x27BD0050)

    words = [u32(text, o) for o in range(0x88, 0xB0, 4)]
    expected = (0x8E430000, 0x0003C0C0, 0x02D81021, 0xAC500000,
                0x24640001, 0xC7A40054, 0xAE440000, 0x28810050,
                0x14200004, 0xE4440004)
    if tuple(words) != expected:
        raise SystemExit("entry-update schedule guard changed")
    w0 = reg(words[0], 16, 3, 2)
    w1 = reg(words[1], 16, 3, 2)
    w2 = reg(words[2], 11, 2, 3)
    w3 = reg(words[3], 21, 2, 3)
    w4 = reg(reg(words[4], 21, 3, 2), 16, 4, 25)
    w5 = (words[5] & 0xFFFF0000) | 0x44
    w6 = reg(words[6], 16, 4, 25)
    w7 = reg(words[7], 21, 4, 25)
    w9 = reg(words[9], 21, 2, 3)
    normalized = (w0, w5, w1, w4, w2, w7, w3, w6, words[8], w9)
    for off, word in zip(range(0x88, 0xB0, 4), normalized):
        p32(text, off, word)
    raw[to:to + ts] = text
    Path(sys.argv[2]).write_bytes(raw)


if __name__ == "__main__": main()
