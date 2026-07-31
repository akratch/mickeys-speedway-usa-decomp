#!/usr/bin/env python3
"""Locate known library code in the baserom by byte-comparing it against
another decomp's *built* object files.

Why this works at all: libultra is the same SDK source, built by the same
IDO 5.3, in every N64 title of this generation. Two different games' builds of
`osRecvMesg` therefore contain the *same instruction words*, and the only bytes
that can legitimately differ are the immediate fields the linker patches --
`%hi`/`%lo` halves of a data address, the 26-bit target of a `jal`, an absolute
`.word`. So: mask exactly those fields (they are named for us by the object's
own relocation records) and compare everything else verbatim.

Anything that survives that comparison is not a guess. It is the same compiler
emitting the same instructions from the same source, which is enough to adopt
the reference build's function name -- and, when a whole object's `.text`
matches in one piece, its file boundary too.

Usage:
    tools/find_known_objects.py <reference-build-dir> [options]

`<reference-build-dir>` is a directory tree of built ELF objects from a
permitted public decomp -- e.g. `.../Diddy-Kong-Racing/build/libultra`. Nothing
is read from that project's *source*; only the compiled bytes and the symbol
and relocation tables, which is what makes the result evidence rather than
transcription.

Example:
    tools/find_known_objects.py ~/src/Diddy-Kong-Racing/build/libultra \\
        --start 0x6F000 --end 0x77000 --min-size 0x10

Output columns: ROM offset, VRAM, size, symbol name, reference object, the
number of masked (relocated) words in the comparison, and how many places in
the search window the match occurred. Trust `occ=1` with a low masked count;
treat a short function with many masked words and several hits as noise.
"""

import argparse
import glob
import os
import re
import struct
import subprocess
import sys
import tempfile

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OBJDUMP = os.path.join(REPO, "tools", "binutils", "mips64-elf-objdump")
OBJCOPY = os.path.join(REPO, "tools", "binutils", "mips64-elf-objcopy")

# Which bits of an instruction word a relocation is allowed to change. A mask
# bit of 1 means "these bits must still match". PC-relative branches are
# deliberately absent: they are section-relative and so are stable across
# links, which is what lets a function with no data references match exactly.
RELOC_MASKS = {
    "R_MIPS_HI16": 0xFFFF0000,
    "R_MIPS_LO16": 0xFFFF0000,
    "R_MIPS_GPREL16": 0xFFFF0000,
    "R_MIPS_LITERAL": 0xFFFF0000,
    "R_MIPS_26": 0xFC000000,
    "R_MIPS_32": 0x00000000,
}
UNKNOWN_RELOC_MASK = 0x00000000  # unrecognised: assume the whole word moves

SYM_RE = re.compile(
    r"^([0-9a-f]{8})\s+\S+\s+\S*\s*\.text\s+([0-9a-f]{8})\s+(\S+)$")
REL_SECTION_RE = re.compile(r"RELOCATION RECORDS FOR \[(\S+)\]")
REL_RE = re.compile(r"^([0-9a-f]{8})\s+(\S+)\s+(\S+)")


def object_functions(obj):
    """Yield (name, offset, text_bytes, per_word_masks) for each .text symbol.

    `offset` is the symbol's own offset within `.text`, taken from the symbol
    table -- not by searching for the bytes. Two symbols in one object can be
    byte-identical (`__ll_rem` and `__ull_rem` in libultra's `ll.c` are), so a
    byte search would silently collapse them onto one address.

    A synthetic ".text" entry for the whole section is yielded first when the
    section is non-empty: a whole-section match is the strongest result the
    tool can produce, because it pins a translation-unit boundary and not just
    a function.
    """
    with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as tmp:
        raw = tmp.name
    try:
        subprocess.run([OBJCOPY, "-O", "binary", "-j", ".text", obj, raw],
                       capture_output=True)
        if not os.path.exists(raw):
            return
        with open(raw, "rb") as fh:
            text = fh.read()
    finally:
        if os.path.exists(raw):
            os.unlink(raw)
    if not text:
        return

    relocs = {}
    section = None
    dump = subprocess.run([OBJDUMP, "-r", obj], capture_output=True,
                          text=True).stdout
    for line in dump.split("\n"):
        m = REL_SECTION_RE.match(line)
        if m:
            section = m.group(1)
            continue
        m = REL_RE.match(line)
        if m and section == ".text":
            relocs[int(m.group(1), 16)] = RELOC_MASKS.get(
                m.group(2), UNKNOWN_RELOC_MASK)

    def masks_for(base, size):
        out = [0xFFFFFFFF] * (size // 4)
        for off, mask in relocs.items():
            if base <= off < base + size:
                out[(off - base) // 4] &= mask
        return out

    yield ".text", 0, text, masks_for(0, len(text) & ~3)

    dump = subprocess.run([OBJDUMP, "-t", obj], capture_output=True,
                          text=True).stdout
    for line in dump.split("\n"):
        m = SYM_RE.match(line)
        if not m:
            continue
        addr, size, name = int(m.group(1), 16), int(m.group(2), 16), m.group(3)
        if name == ".text":
            continue  # the section symbol; already yielded synthetically above
        if size and addr + size <= len(text):
            yield name, addr, text[addr:addr + size], masks_for(addr, size)


def masked_match(rom, blob, masks, lo, hi):
    """Every 4-byte-aligned offset in [lo, hi) where blob matches under masks."""
    if not masks:
        return []

    # Prefer anchoring on the longest run of *unmasked* words and letting
    # bytes.find do the work; fall back to a full aligned sweep when there is
    # no run long enough to be worth searching for.
    fixed = [i for i, m in enumerate(masks) if m == 0xFFFFFFFF]
    run_start = run_len = 0
    if fixed:
        start = fixed[0]
        for i in range(1, len(fixed) + 1):
            if i == len(fixed) or fixed[i] != fixed[i - 1] + 1:
                if fixed[i - 1] - start + 1 > run_len:
                    run_len, run_start = fixed[i - 1] - start + 1, start
                if i < len(fixed):
                    start = fixed[i]

    if run_len >= 2:
        anchor = blob[run_start * 4:(run_start + run_len) * 4]
        candidates = []
        pos = lo
        while True:
            idx = rom.find(anchor, pos, hi)
            if idx < 0:
                break
            pos = idx + 4
            candidates.append(idx - run_start * 4)
    else:
        candidates = range(lo, hi - len(blob) + 1, 4)

    hits = []
    for start in candidates:
        if start < lo or start % 4 or start + len(blob) > hi:
            continue
        window = rom[start:start + len(blob)]
        for i, mask in enumerate(masks):
            if not mask:
                continue
            if mask == 0xFFFFFFFF:
                if window[i * 4:i * 4 + 4] != blob[i * 4:i * 4 + 4]:
                    break
            else:
                a, = struct.unpack_from(">I", window, i * 4)
                b, = struct.unpack_from(">I", blob, i * 4)
                if (a & mask) != (b & mask):
                    break
        else:
            hits.append(start)
    return hits


def main():
    ap = argparse.ArgumentParser(
        description=__doc__.split("\n\n")[0],
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("reference", help="directory of built reference objects")
    ap.add_argument("--rom", default=os.path.join(REPO, "baseroms",
                                                  "mickey.us.z64"))
    ap.add_argument("--start", type=lambda s: int(s, 0), default=0x1000,
                    help="ROM offset to start searching (default 0x1000)")
    ap.add_argument("--end", type=lambda s: int(s, 0), default=0x87000,
                    help="ROM offset to stop searching (default 0x87000)")
    ap.add_argument("--vram", type=lambda s: int(s, 0), default=0x80000400,
                    help="VRAM of --start's segment base (default 0x80000400)")
    ap.add_argument("--vram-rom", type=lambda s: int(s, 0), default=0x1000,
                    help="ROM offset that --vram maps to (default 0x1000)")
    ap.add_argument("--min-size", type=lambda s: int(s, 0), default=0x10,
                    help="ignore symbols smaller than this (default 0x10)")
    ap.add_argument("--max-occurrences", type=int, default=4,
                    help="drop matches occurring more often than this; they "
                         "are generic code, not identification (default 4)")
    ap.add_argument("--sections", action="store_true",
                    help="report whole-object .text matches only, i.e. "
                         "translation-unit boundaries")
    args = ap.parse_args()

    with open(args.rom, "rb") as fh:
        rom = fh.read()

    objects = sorted(glob.glob(os.path.join(args.reference, "**", "*.o"),
                               recursive=True))
    if not objects:
        sys.exit(f"no .o files under {args.reference}")

    rows = []
    for obj in objects:
        rel = os.path.relpath(obj, args.reference)
        for name, _off, blob, masks in object_functions(obj):
            if len(blob) < args.min_size:
                continue
            if args.sections and name != ".text":
                continue
            hits = masked_match(rom, blob, masks, args.start, args.end)
            if not hits or len(hits) > args.max_occurrences:
                continue
            masked = sum(1 for m in masks if m != 0xFFFFFFFF)
            for hit in hits:
                rows.append((hit, len(blob), name, rel, masked, len(hits)))

    rows.sort()
    print(f"{'ROM':>9} {'VRAM':>10} {'size':>7}  {'symbol':<28} "
          f"{'reference object':<34} {'masked':>6} occ")
    for offset, size, name, rel, masked, occ in rows:
        vram = offset - args.vram_rom + args.vram
        print(f"{offset:#09x} {vram:#010x} {size:#7x}  {name:<28} "
              f"{rel:<34} {masked:>6} {occ:>3}")
    print(f"\n{len(rows)} match(es) in "
          f"{args.start:#x}..{args.end:#x} from {len(objects)} objects")


if __name__ == "__main__":
    main()
