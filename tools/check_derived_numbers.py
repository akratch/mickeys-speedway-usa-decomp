#!/usr/bin/env python3
"""Mechanically re-derive the numbers the docs claim, and fail on a mismatch.

Most of this project's prose is an argument that cannot be checked by a script.
A minority of it is pure arithmetic -- a VRAM address that is a ROM offset plus
a constant, a segment size that is one address minus another, a count of things
in `asm/` -- and those are exactly the claims that rot silently when a boundary
moves. A review found several: a corridor end off by 0x10 in three files, a
rodata bound quoting the wrong ROM address and the wrong word offset, a jump
table count three low, and a size given in MB that was neither MB nor MiB.

Every one of those was recomputable from the file that contained it. So they
are recomputed here, on every `gmake check-docs`, instead of on every review.

Scope is deliberately narrow: only claims this script can derive from first
principles. It says nothing about whether a name is justified.
"""

import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# VRAM = ROM + this, for the resident segment (docs/modules.md header).
DELTA = 0x7FFFF400
RESIDENT_ROM_END = 0x86640

HEX = r"0x([0-9A-Fa-f]+)"

problems = []
checked = 0


def report(path, lineno, msg):
    problems.append(f"{path}:{lineno}: {msg}")


def read(relpath):
    with open(os.path.join(ROOT, relpath), encoding="utf-8") as fh:
        return fh.read().split("\n")


# --- 1. "ROM 0xA[-0xB] ... (VRAM 0xC[-0xD])" and the reverse order -----------
# Only resident-segment addresses obey the delta, so pairs are checked when the
# VRAM side is in 0x8xxxxxxx and the ROM side is below the resident end.

PAIR = re.compile(
    r"ROM\s*`?" + HEX + r"`?\s*(?:[-–]\s*`?" + HEX + r"`?)?"
    r"[^\n]{0,40}?VRAM\s*`?" + HEX + r"`?\s*(?:[-–]\s*`?" + HEX + r"`?)?"
)
PAIR_REV = re.compile(
    r"VRAM\s*`?" + HEX + r"`?\s*(?:[-–]\s*`?" + HEX + r"`?)?"
    r"[^\n]{0,40}?ROM\s*`?" + HEX + r"`?\s*(?:[-–]\s*`?" + HEX + r"`?)?"
)

PROSE_FILES = [
    "docs/modules.md",
    "symbol_addrs.us.txt",
    "mickey.us.yaml",
] + sorted(
    os.path.join(d, f).replace(ROOT + os.sep, "")
    for d, _, fs in os.walk(os.path.join(ROOT, "src"))
    for f in fs
    if f.endswith((".c", ".h"))
)


def check_pair(path, lineno, rom, vram):
    global checked
    if not (rom and vram):
        return
    r, v = int(rom, 16), int(vram, 16)
    if not (0x80000000 <= v < 0x81000000) or r >= RESIDENT_ROM_END:
        return
    checked += 1
    if r + DELTA != v:
        report(path, lineno,
               f"ROM 0x{r:X} + 0x{DELTA:X} = 0x{r + DELTA:08X}, "
               f"but the text says VRAM 0x{v:08X}")


# A bare VRAM address with the ROM offset in parentheses after it, e.g.
# "`0x80080D24` (ROM `0x81924`)". No "VRAM" keyword, so PAIR_REV misses it --
# and this is the exact form the wrong rodata bound hid in.
BARE = re.compile(r"`?0x(8[0-9A-Fa-f]{7})`?\s*\(ROM\s*`?" + HEX + r"`?\)")

# "VRAM 0xA-0xB, 0xN bytes" -- the span and the stated size must agree. The
# corridor's end was wrong by 0x10 in this form for three files.
SPAN = re.compile(
    r"VRAM\s*`?" + HEX + r"`?\s*[-–]\s*`?" + HEX + r"`?[^\n]{0,12}?`?"
    + HEX + r"`?\s*bytes"
)

for path in PROSE_FILES:
    try:
        lines = read(path)
    except OSError:
        continue
    for i, line in enumerate(lines, 1):
        for m in PAIR.finditer(line):
            r1, r2, v1, v2 = m.groups()
            check_pair(path, i, r1, v1)
            check_pair(path, i, r2, v2)
        for m in PAIR_REV.finditer(line):
            v1, v2, r1, r2 = m.groups()
            check_pair(path, i, r1, v1)
            check_pair(path, i, r2, v2)
        for m in BARE.finditer(line):
            check_pair(path, i, m.group(2), m.group(1))
        for m in SPAN.finditer(line):
            lo, hi, size = (int(g, 16) for g in m.groups())
            checked += 1
            if hi - lo != size:
                report(path, i,
                       f"VRAM 0x{hi:08X} - 0x{lo:08X} = 0x{hi - lo:X}, "
                       f"but the text says 0x{size:X} bytes")


# --- 2. docs/modules.md's ROM|VRAM anchor table -----------------------------
# Rows look like: | `0x2A250`-`0x2AE44` | `0x80029650` | anchor | tier | ... |
ROW = re.compile(
    r"^\|\s*`?" + HEX + r"`?(?:[-–]\s*`?" + HEX + r"`?)?\s*\|"
    r"\s*`?" + HEX + r"`?\s*\|"
)
for i, line in enumerate(read("docs/modules.md"), 1):
    m = ROW.match(line)
    if m:
        check_pair("docs/modules.md", i, m.group(1), m.group(3))


# --- 3. docs/modules.md's top-level ROM map: size column ---------------------
# | `0x087000`-`0x16B0000` | 22.16 MiB | ... |   -- hex sizes and MiB both.
# Scoped to section 2. Section 3's table looks identical but its second column
# is VRAM, not a size, and it is checked by rule 2 instead.
MAP_ROW = re.compile(
    r"^\|\s*`" + HEX + r"`[-–]`" + HEX + r"`\s*\|\s*([^|]+?)\s*\|"
)
map_total = 0
in_map = False
for i, line in enumerate(read("docs/modules.md"), 1):
    if line.startswith("## "):
        in_map = line.startswith("## 2.")
    if not in_map:
        continue
    m = MAP_ROW.match(line)
    if not m:
        continue
    lo, hi, size = int(m.group(1), 16), int(m.group(2), 16), m.group(3).strip()
    actual = hi - lo
    map_total += actual
    hexm = re.fullmatch(r"`?" + HEX + r"`?", size)
    mib = re.fullmatch(r"~?([\d.]+)\s*MiB", size)
    if hexm:
        checked += 1
        if int(hexm.group(1), 16) != actual:
            report("docs/modules.md", i,
                   f"size column says 0x{int(hexm.group(1), 16):X}, "
                   f"but 0x{hi:X} - 0x{lo:X} = 0x{actual:X}")
    elif mib:
        checked += 1
        want = actual / 1048576.0
        if abs(float(mib.group(1)) - want) > 0.02:
            report("docs/modules.md", i,
                   f"size column says {mib.group(1)} MiB, but "
                   f"0x{hi:X} - 0x{lo:X} = {actual:,} B = {want:.2f} MiB")
    else:
        report("docs/modules.md", i,
               f"size column {size!r} is neither a hex size nor a MiB figure; "
               "this table's units are checked, so keep them uniform")

if map_total and map_total != 0x2000000:
    problems.append(
        f"docs/modules.md: the top-level ROM map's rows sum to 0x{map_total:X}, "
        f"not the image's 0x2000000 -- a row is missing, overlapping, or wrong")
elif map_total:
    checked += 1


# --- 4. the jump-table count, against asm/ ----------------------------------
# asm/ is ROM-derived and gitignored, so it is only present after `gmake
# extract`. Without it the count is unverifiable and is reported as skipped
# rather than silently passed.
asm_dir = os.path.join(ROOT, "asm")
COUNT_CLAIM = re.compile(r"(\d+)\s+jump\s+tables|Mapping all (\d+) jump", re.I)
claims = []
for path in ("docs/modules.md", "mickey.us.yaml"):
    for i, line in enumerate(read(path), 1):
        m = COUNT_CLAIM.search(line)
        if m:
            claims.append((path, i, int(m.group(1) or m.group(2))))

if not os.path.isdir(asm_dir):
    print("check_derived_numbers: asm/ absent -- skipping the jump-table count "
          "(run `gmake extract` to enable it)")
elif claims:
    found = set()
    for d, _, fs in os.walk(asm_dir):
        for f in fs:
            if not f.endswith(".s"):
                continue
            with open(os.path.join(d, f), encoding="utf-8", errors="replace") as fh:
                found.update(re.findall(r"jtbl_[0-9A-Fa-f]{8}", fh.read()))
    for path, i, claimed in claims:
        checked += 1
        if claimed != len(found):
            report(path, i,
                   f"claims {claimed} jump tables; asm/ contains "
                   f"{len(found)} distinct jtbl_* symbols")


# --- 5. the libultra corridor's own arithmetic ------------------------------
# docs/modules.md 4.1 states four numbers that are all derivable from
# mickey.us.yaml's subsegment list plus symbol_addrs.us.txt: how many named
# subsegments the corridor has, how many named functions, how much of it is
# still unnamed, and what fraction that is. Phase 2 Task 3 moved all four at
# once, and Phase 1's standing corrective is that summary counts are DERIVED --
# so they are derived here instead of being retyped.

CORRIDOR_LO, CORRIDOR_HI = 0x6F420, 0x76D10
CORRIDOR_VRAM_LO = CORRIDOR_LO + DELTA
CORRIDOR_VRAM_HI = CORRIDOR_HI + DELTA

SUBSEG = re.compile(r"^\s*- \[" + HEX + r", (\w+)(?:, (\S+))?\]\s*$")
subsegs = []
for line in read("mickey.us.yaml"):
    m = SUBSEG.match(line)
    if m:
        subsegs.append((int(m.group(1), 16), m.group(3)))
subsegs.sort()

named = sum(1 for a, n in subsegs if CORRIDOR_LO <= a < CORRIDOR_HI and n)
unnamed_bytes = 0
for i, (a, n) in enumerate(subsegs):
    if not (CORRIDOR_LO <= a < CORRIDOR_HI) or n:
        continue
    nxt = subsegs[i + 1][0] if i + 1 < len(subsegs) else CORRIDOR_HI
    unnamed_bytes += min(nxt, CORRIDOR_HI) - a

SYMROW = re.compile(r"^(\w+)\s*=\s*" + HEX + r";")
corridor_funcs = 0
for line in read("symbol_addrs.us.txt"):
    m = SYMROW.match(line)
    if m and CORRIDOR_VRAM_LO <= int(m.group(2), 16) < CORRIDOR_VRAM_HI:
        corridor_funcs += 1

# These claims wrap across lines in the prose, so they are matched against the
# whole file rather than line by line; the line number is recovered from the
# match offset. A claim that a regex silently fails to find is a check that
# silently passes, so each one asserts it matched at least once.
CORRIDOR_CLAIM = re.compile(
    r"\*\*(\d+) named subsegments,[^*]*?and (\d+) named functions\*\*",
    re.S)
DRIFT_CLAIM = re.compile(
    r"is `0x([0-9A-Fa-f]+)`[^\n]*?\*\*([\d.]+)% of the corridor\*\*")

modules_text = "\n".join(read("docs/modules.md"))


def lineno(off):
    return modules_text.count("\n", 0, off) + 1


hits = 0
for m in CORRIDOR_CLAIM.finditer(modules_text):
    hits += 1
    checked += 2
    i = lineno(m.start())
    if int(m.group(1)) != named:
        report("docs/modules.md", i,
               f"claims {m.group(1)} named corridor subsegments; "
               f"mickey.us.yaml has {named}")
    if int(m.group(2)) != corridor_funcs:
        report("docs/modules.md", i,
               f"claims {m.group(2)} named corridor functions; "
               f"symbol_addrs.us.txt has {corridor_funcs} in "
               f"0x{CORRIDOR_VRAM_LO:08X}-0x{CORRIDOR_VRAM_HI:08X}")
if not hits:
    problems.append("docs/modules.md: the corridor's "
                    "'N named subsegments ... and N named functions' claim is "
                    "gone or reworded -- this check is now checking nothing")

hits = 0
for m in DRIFT_CLAIM.finditer(modules_text):
    hits += 1
    checked += 2
    i = lineno(m.start())
    if int(m.group(1), 16) != unnamed_bytes:
        report("docs/modules.md", i,
               f"claims 0x{int(m.group(1), 16):X} of unnamed corridor; "
               f"mickey.us.yaml has 0x{unnamed_bytes:X}")
    want = unnamed_bytes * 100.0 / (CORRIDOR_HI - CORRIDOR_LO)
    if abs(float(m.group(2)) - want) > 0.05:
        report("docs/modules.md", i,
               f"claims {m.group(2)}% of the corridor unnamed; "
               f"0x{unnamed_bytes:X}/0x{CORRIDOR_HI - CORRIDOR_LO:X} "
               f"= {want:.1f}%")
if not hits:
    problems.append("docs/modules.md: the corridor's remaining-drift claim is "
                    "gone or reworded -- this check is now checking nothing")


# --- verdict ----------------------------------------------------------------
if problems:
    print("check_derived_numbers: FAILED", file=sys.stderr)
    for p in problems:
        print("  " + p, file=sys.stderr)
    print(f"\n{len(problems)} mismatch(es). These are arithmetic, not judgement "
          "calls -- recompute and fix the text.", file=sys.stderr)
    sys.exit(1)

print(f"check_derived_numbers: OK -- {checked} derived numbers re-computed and "
      "matched")
