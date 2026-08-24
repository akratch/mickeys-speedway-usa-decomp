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
number of masked (relocated) words in the comparison, how many places in the
search window the match occurred (`occ`), and -- with `--rom-occ` -- how many
places in the *whole image* it occurred (`romocc`). Trust `romocc=1` with a low
masked count; treat a short function with many masked words and several hits as
noise.

For overlays, use `--all-overlays` or `--overlay N`. The search is then
restricted to the atlas's text ranges and reports `overlay:N:text+offset`; it
never fabricates a VRAM by applying the resident segment's address delta.

`romocc` is the column the adoption threshold in `docs/modules.md` section 1.2
actually asks about: uniqueness across the whole 32MB image, not within the
window that happened to be scanned. `occ` alone once carried a wrong claim into
Task B. `romocc` is printed as `?` when the comparison has no run of two
consecutive unmasked words to anchor a full-image search on -- that is a
refusal to answer, not a `1`.
"""

import argparse
import glob
import json
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


def longest_fixed_run(masks):
    """(start, length) of the longest run of fully-unmasked words in `masks`."""
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
    return run_start, run_len


def masked_match(rom, blob, masks, lo, hi):
    """Every 4-byte-aligned offset in [lo, hi) where blob matches under masks."""
    if not masks:
        return []

    # Prefer anchoring on the longest run of *unmasked* words and letting
    # bytes.find do the work; fall back to a full aligned sweep when there is
    # no run long enough to be worth searching for.
    run_start, run_len = longest_fixed_run(masks)

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


def load_overlay_windows(path, selected):
    """Return ``[(start, end, overlay)]`` from the canonical atlas."""
    with open(path, encoding="utf-8") as fh:
        atlas = json.load(fh)
    modules = atlas.get("modules")
    if not isinstance(modules, list):
        sys.exit(f"invalid overlay atlas (no modules list): {path}")
    wanted = set(selected) if selected else None
    windows = []
    seen = set()
    for module in modules:
        overlay = module.get("overlay")
        if wanted is not None and overlay not in wanted:
            continue
        try:
            text = module["sections"]["text"]
            start, end = int(text["start"], 0), int(text["end"], 0)
        except (KeyError, TypeError, ValueError) as exc:
            sys.exit(f"invalid text range for overlay {overlay} in {path}: {exc}")
        seen.add(overlay)
        if end > start:
            windows.append((start, end, overlay))
    missing = (wanted or set()) - seen
    if missing:
        sys.exit(f"overlay(s) absent from atlas: {sorted(missing)}")
    if not windows:
        sys.exit("selected overlays have no text bytes")
    return sorted(windows)


def overlay_location(windows, hit, size):
    for start, end, overlay in windows:
        if start <= hit and hit + size <= end:
            return overlay, hit - start
    return None


def main():
    ap = argparse.ArgumentParser(
        description=__doc__.split("\n\n")[0],
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("reference", help="directory of built reference objects")
    ap.add_argument("--rom", default=os.path.join(REPO, "baseroms",
                                                  "mickey.us.z64"))
    overlay_group = ap.add_mutually_exclusive_group()
    overlay_group.add_argument(
        "--all-overlays", action="store_true",
        help="search every overlay text range from config/overlays.us.json")
    overlay_group.add_argument(
        "--overlay", type=int, action="append", metavar="N",
        help="search one overlay text range; repeat for several overlays")
    ap.add_argument(
        "--overlay-atlas",
        default=os.path.join(REPO, "config", "overlays.us.json"),
        help="canonical atlas used by overlay search")
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
    ap.add_argument("--rom-occ", action="store_true",
                    help="also count occurrences across the WHOLE image, not "
                         "just the window -- this is the uniqueness test the "
                         "adoption threshold asks for. Printed as `?` when the "
                         "comparison has no 2-word unmasked anchor to search "
                         "the image with")
    ap.add_argument("--json", action="store_true",
                    help="emit structured rows instead of the text table")
    args = ap.parse_args()

    with open(args.rom, "rb") as fh:
        rom = fh.read()

    overlay_windows = None
    if args.all_overlays or args.overlay:
        overlay_windows = load_overlay_windows(
            args.overlay_atlas, [] if args.all_overlays else args.overlay)
        search_start = min(row[0] for row in overlay_windows)
        search_end = max(row[1] for row in overlay_windows)
    else:
        search_start, search_end = args.start, args.end

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
            hits = masked_match(rom, blob, masks, search_start, search_end)
            locations = None
            if overlay_windows is not None:
                locations = {
                    hit: overlay_location(overlay_windows, hit, len(blob))
                    for hit in hits
                }
                hits = [hit for hit in hits if locations[hit] is not None]
            if not hits or len(hits) > args.max_occurrences:
                continue
            masked = sum(1 for m in masks if m != 0xFFFFFFFF)
            romocc = "-"
            if args.rom_occ:
                # Only anchored searches can afford the whole image; without an
                # anchor the fallback is an 8M-offset Python sweep per symbol.
                # Say `?` rather than guess -- a wrong `1` here is exactly the
                # failure this column exists to prevent.
                romocc = (str(len(masked_match(rom, blob, masks, 0, len(rom))))
                          if longest_fixed_run(masks)[1] >= 2 else "?")
            for hit in hits:
                row = {
                    "rom": hit,
                    "size": len(blob),
                    "symbol": name,
                    "reference_object": rel,
                    "masked_words": masked,
                    "search_occurrences": len(hits),
                    "rom_occurrences": romocc,
                }
                if overlay_windows is not None:
                    overlay, offset = locations[hit]
                    row.update({
                        "overlay": overlay,
                        "section": "text",
                        "section_offset": offset,
                        "location": f"overlay:{overlay}:text+0x{offset:X}",
                    })
                else:
                    row["vram"] = hit - args.vram_rom + args.vram
                rows.append(row)

    rows.sort(key=lambda row: (row["rom"], row["reference_object"], row["symbol"]))
    if args.json:
        print(json.dumps({
            "reference_root": os.path.abspath(args.reference),
            "search": (
                "overlay text ranges" if overlay_windows is not None
                else f"ROM {search_start:#x}..{search_end:#x}"
            ),
            "objects": len(objects),
            "matches": rows,
        }, indent=2))
        return

    if overlay_windows is not None:
        print(f"{'location':<30} {'ROM':>9} {'size':>7}  {'symbol':<28} "
              f"{'reference object':<34} {'masked':>6} {'occ':>3} romocc")
        for row in rows:
            print(f"{row['location']:<30} {row['rom']:#09x} {row['size']:#7x}  "
                  f"{row['symbol']:<28} {row['reference_object']:<34} "
                  f"{row['masked_words']:>6} {row['search_occurrences']:>3} "
                  f"{row['rom_occurrences']:>6}")
        scope = f"{len(overlay_windows)} overlay text range(s)"
    else:
        print(f"{'ROM':>9} {'VRAM':>10} {'size':>7}  {'symbol':<28} "
              f"{'reference object':<34} {'masked':>6} {'occ':>3} romocc")
        for row in rows:
            print(f"{row['rom']:#09x} {row['vram']:#010x} {row['size']:#7x}  "
                  f"{row['symbol']:<28} {row['reference_object']:<34} "
                  f"{row['masked_words']:>6} {row['search_occurrences']:>3} "
                  f"{row['rom_occurrences']:>6}")
        scope = f"{search_start:#x}..{search_end:#x}"
    print(f"\n{len(rows)} match(es) in {scope} from {len(objects)} objects")


if __name__ == "__main__":
    main()
