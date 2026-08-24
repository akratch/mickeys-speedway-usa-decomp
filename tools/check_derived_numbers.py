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

import json
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


# --- 6. the mining pass's own bookkeeping -----------------------------------
# Phase 2 Task 3 added 87 blocks to symbol_addrs.us.txt, each headed by a line
# naming the subsegment it adopted, the ROM offset, and the reference object it
# was cited from. Three things about that are pure arithmetic and one is a pure
# cross-reference, so all four are done here rather than in a reviewer's head.
#
# WHAT 6a DOES AND DOES NOT CATCH -- stated precisely, because an earlier
# version of this comment claimed more than the code delivers and a reviewer
# drove the difference.
#
# The review that produced this section found two defects: a header citing an
# object that does not contain the symbol beneath it, and a per-title table
# transcribed from a run log outside the tree. The second is made recomputable
# below. The FIRST IS NOT CHECKED HERE and cannot be:
#
#   * 6a compares the header's SUBSEGMENT NAME and ROM OFFSET against
#     mickey.us.yaml. It catches a header that adopts a subsegment the yaml does
#     not have at that offset, or has under a different name -- the two files
#     drifting apart, which is the failure mode a future edit to either one is
#     most likely to cause.
#   * It does NOT look at the cited object at all. Reinstating the exact bad
#     citation the review found (`epiread.c.o` for `osEPiWriteIo`, an object
#     that does not contain that symbol) passes this script cleanly. Verified,
#     not assumed.
#
# Checking a citation against its object would mean reading the reference
# builds, and those live outside this repository by design. The pins around
# them are checkable: tools/reference-builds.lock records each title's commit,
# baserom checksum and object digest, tools/verify_reference_builds.sh proves a
# local farm is the one those names were mined from, and check 7 below proves
# docs/references.md quotes the same pins. The citation itself is not. It is
# verifiable only by re-running the mining pass against a farm, and the honest
# place to record that is here, next to the check that does not do it.

TU_HDR = re.compile(
    r"^// (\S+)  ROM (0x[0-9a-f]+)  \((\w+)/\w+: (\S+)\)  whole \.text ")
TU_CORR = re.compile(r"^//   same bytes in: (.*)$")
TU_SYM = re.compile(r"^(\w+)\s*=\s*" + HEX + r";")

tu_blocks = []          # (subseg, rom, title, corroborating titles, n_names)
block = None
for i, line in enumerate(read("symbol_addrs.us.txt"), 1):
    m = TU_HDR.match(line)
    if m:
        block = [m.group(1), int(m.group(2), 16), m.group(3), [m.group(3)], 0, i]
        tu_blocks.append(block)
        continue
    if block is None:
        continue
    m = TU_CORR.match(line)
    if m:
        block[3] = [t.strip() for t in m.group(1).split(",")]
        continue
    if TU_SYM.match(line):
        block[4] += 1
        continue
    if line.startswith("//") and not line.startswith("//   "):
        block = None

if tu_blocks:
    # 6a. every header must name a real, identically-named yaml subsegment at
    # the offset it claims. Scope is the header-vs-yaml agreement only; the
    # cited object is not read (see the note above).
    for subseg, rom, _title, _corr, _n, lineno_ in tu_blocks:
        checked += 1
        if subsegs and dict(subsegs).get(rom) != subseg:
            report("symbol_addrs.us.txt", lineno_,
                   f"header adopts `{subseg}` at ROM 0x{rom:X}, but "
                   f"mickey.us.yaml has "
                   f"{dict(subsegs).get(rom) or 'no named subsegment'} there")

    # 6b/6c/6d. the per-title figures docs/references.md publishes.
    adopted, names, corrob = {}, {}, {}
    for _s, _r, title, corr, n, _l in tu_blocks:
        adopted[title] = adopted.get(title, 0) + 1
        names[title] = names.get(title, 0) + n
        for t in corr:
            if t != title:
                corrob[t] = corrob.get(t, 0) + 1
    multi = sum(1 for b in tu_blocks if len(b[3]) > 1)

    TITLES = {"Jet Force Gemini": "jfg", "Perfect Dark": "perfect_dark",
              "Banjo-Kazooie": "banjo-kazooie",
              "Conker's Bad Fur Day": "conker"}
    # Summary rows look like:
    #   | Jet Force Gemini | `c82affff` | ... | 391 | **84** | **187** | 84 | — |
    refs = read("docs/references.md")
    seen_rows = 0
    for i, line in enumerate(refs, 1):
        if not line.startswith("|"):
            continue
        cells = [c.strip() for c in line.strip("|").split("|")]
        key = cells[0]
        if key not in TITLES:
            continue
        slug = TITLES[key]
        got = [c.replace("*", "") for c in cells[6:9]]
        if len(got) < 2:
            continue
        seen_rows += 1
        checked += 2
        if got[0].isdigit() and int(got[0]) != adopted.get(slug, 0):
            report("docs/references.md", i,
                   f"{key} row claims {got[0]} adopted TUs; symbol_addrs.us.txt "
                   f"has {adopted.get(slug, 0)}")
        if got[1].isdigit() and int(got[1]) != names.get(slug, 0):
            report("docs/references.md", i,
                   f"{key} row claims {got[1]} names; symbol_addrs.us.txt has "
                   f"{names.get(slug, 0)}")
    if seen_rows != len(TITLES):
        problems.append(
            f"docs/references.md: found {seen_rows} of {len(TITLES)} per-title "
            "summary rows -- the table was reshaped and this check stopped "
            "checking it")

    TOTALS = re.compile(
        r"\*\*(\d+) of the (\d+) adopted translation units were matched by "
        r"more than one title\*\*")
    hits = 0
    refs_text = "\n".join(refs)
    for m in TOTALS.finditer(refs_text):
        hits += 1
        checked += 2
        i = refs_text.count("\n", 0, m.start()) + 1
        if int(m.group(1)) != multi:
            report("docs/references.md", i,
                   f"claims {m.group(1)} multiply-matched TUs; "
                   f"symbol_addrs.us.txt has {multi}")
        if int(m.group(2)) != len(tu_blocks):
            report("docs/references.md", i,
                   f"claims {m.group(2)} adopted TUs; symbol_addrs.us.txt has "
                   f"{len(tu_blocks)}")
    if not hits:
        problems.append("docs/references.md: the 'N of the M adopted "
                        "translation units' totals claim is gone or reworded -- "
                        "this check is now checking nothing")

    # The prose wraps, so whitespace runs must be allowed to contain newlines.
    CORROB = re.compile(r"\*\*corroborates (\d+)\*\*\s+of\s+the\s+\d+\s+adopted")
    corrob_seen = 0
    for m in CORROB.finditer(refs_text):
        corrob_seen += 1
        i = refs_text.count("\n", 0, m.start()) + 1
        # attribute to the nearest preceding "## <title>" heading
        head = refs_text.rfind("\n## ", 0, m.start())
        title = refs_text[head + 4:refs_text.index("\n", head + 4)].strip()
        slug = TITLES.get(title)
        if slug is None:
            continue
        checked += 1
        if int(m.group(1)) != corrob.get(slug, 0):
            report("docs/references.md", i,
                   f"{title} claims {m.group(1)} corroborations; "
                   f"symbol_addrs.us.txt has {corrob.get(slug, 0)}")
    want_corrob = sum(1 for t in TITLES.values() if corrob.get(t))
    if corrob_seen != want_corrob:
        problems.append(
            f"docs/references.md: found {corrob_seen} per-title 'corroborates "
            f"N' claims, expected {want_corrob} (one per title that "
            "corroborates anything) -- a claim was dropped or reworded")

    # 6e. docs/modules.md restates 6d's figures in prose, and that restatement
    #     is the one place in the tree where two documents publish the same
    #     numbers.  It is also where they went wrong: the summary sentence once
    #     read "corroboration of 75 and 73", which is corroboration PLUS
    #     re-confirmation summed (2+73, 8+65) -- exactly the conflation that
    #     docs/references.md's "Three different things, counted separately"
    #     exists to prevent, and a 10x overstatement of the corroboration.
    #
    #     So both halves are checked, by different means, because they are
    #     different KINDS of number:
    #       * corroboration -- recomputable from symbol_addrs.us.txt, checked
    #         against `corrob` exactly as 6d checks references.md;
    #       * re-confirmation -- NOT recomputable from this tree (it comes from
    #         the mining run logs), so it is checked for agreement with what
    #         docs/references.md says, which is the authority for it.
    #     A missing sentence is a failure, not a pass: this check must not be
    #     silently disarmed by a reword.
    RECONF = re.compile(r"\*\*re-confirms (\d+)\*\*\s+subsegments")
    reconf = {}
    for m in RECONF.finditer(refs_text):
        head = refs_text.rfind("\n## ", 0, m.start())
        title = refs_text[head + 4:refs_text.index("\n", head + 4)].strip()
        slug = TITLES.get(title)
        if slug is not None:
            reconf[slug] = int(m.group(1))

    MODULES_SUMMARY = re.compile(
        r"BK and Conker contributed \*\*none\*\*.*?"
        r"corroboration of \*\*(\d+)\*\* and \*\*(\d+)\*\*\s+of the adopted "
        r"translation units,\s+and re-confirmation of \*\*(\d+)\*\* and "
        r"\*\*(\d+)\*\*\s+subsegments",
        re.DOTALL)
    mods_text = "\n".join(read("docs/modules.md"))
    hit = MODULES_SUMMARY.search(mods_text)
    if hit is None:
        problems.append(
            "docs/modules.md: the §1.3 sentence restating Banjo-Kazooie's and "
            "Conker's corroboration and re-confirmation figures is gone or "
            "reworded -- check 6e is now checking nothing. It is the only "
            "place two documents publish the same mining figures, and it is "
            "where they were last found summed together; keep it in the form "
            "'corroboration of **N** and **M** of the adopted translation "
            "units, and re-confirmation of **P** and **Q** subsegments' or "
            "update this check deliberately.")
    else:
        i = mods_text.count("\n", 0, hit.start()) + 1
        want = [("corroboration", "banjo-kazooie", corrob.get("banjo-kazooie", 0),
                 "symbol_addrs.us.txt"),
                ("corroboration", "conker", corrob.get("conker", 0),
                 "symbol_addrs.us.txt"),
                ("re-confirmation", "banjo-kazooie", reconf.get("banjo-kazooie"),
                 "docs/references.md"),
                ("re-confirmation", "conker", reconf.get("conker"),
                 "docs/references.md")]
        for got, (kind, slug, expect, source) in zip(hit.groups(), want):
            if expect is None:
                problems.append(
                    f"docs/references.md: no 're-confirms N subsegments' claim "
                    f"found for {slug}, so docs/modules.md's {kind} figure has "
                    "nothing to be checked against")
                continue
            checked += 1
            if int(got) != expect:
                report("docs/modules.md", i,
                       f"§1.3 claims {kind} of {got} for {slug}; {source} has "
                       f"{expect}"
                       + (f" (note {got} == {expect} + "
                          f"{int(got) - expect}: corroboration and "
                          "re-confirmation summed is the known failure here)"
                          if reconf.get(slug) is not None
                          and int(got) == expect + reconf[slug] else ""))
else:
    problems.append("symbol_addrs.us.txt: no mining-pass TU block headers found "
                    "-- section 6 of this script is checking nothing")


# --- 7. docs/references.md against tools/reference-builds.lock ---------------
#
# The lock is the machine-readable copy of the same pins docs/references.md
# publishes in prose, and tools/verify_reference_builds.sh checks a farm against
# the lock. That leaves exactly one gap: prose and lock disagreeing, which would
# make a green farm check vouch for numbers the page does not actually quote.
# Cheap to close, so it is closed here.

LOCK_SECTION = re.compile(r"^\[(.+)\]$")
LOCK_FIELD = re.compile(r"^(\w+)\s*=\s*(.*)$")

lock = {}
section = None
for line in read("tools/reference-builds.lock"):
    line = line.rstrip("\n")
    if line.startswith("#"):
        continue
    m = LOCK_SECTION.match(line)
    if m:
        section = {}
        lock[m.group(1)] = section
        continue
    m = LOCK_FIELD.match(line)
    if m and section is not None:
        section[m.group(1)] = m.group(2).strip()

if len(lock) != 5:
    problems.append(f"tools/reference-builds.lock: parsed {len(lock)} titles, "
                    "expected 5 -- the lock was reshaped and check 7 stopped "
                    "checking it")
else:
    refs_text = "".join(read("docs/references.md"))
    for slug, entry in lock.items():
        for key in ("commit", "baserom_sha1"):
            checked += 1
            value = entry.get(key, "")
            if value and value not in refs_text:
                problems.append(
                    f"docs/references.md: does not quote {slug}'s {key} "
                    f"{value} that tools/reference-builds.lock pins")
    # The Objects column of the summary table, against the count the lock
    # records for the same title. Same row shape check 6b reads. The name-to-
    # section map is repeated rather than shared: check 6 only builds it when
    # the symbol file has mining blocks, and this check must not go quiet with
    # it.
    ROWS = {"Diddy Kong Racing": "diddy-kong-racing",
            "Jet Force Gemini": "jfg", "Perfect Dark": "perfect_dark",
            "Banjo-Kazooie": "banjo-kazooie",
            "Conker's Bad Fur Day": "conker"}
    for i, line in enumerate(read("docs/references.md"), 1):
        if not line.startswith("|"):
            continue
        cells = [c.strip() for c in line.strip("|").split("|")]
        if cells[0] not in ROWS or len(cells) < 5:
            continue
        want = lock.get(ROWS[cells[0]], {}).get("objects")
        if not (cells[4].isdigit() and want):
            continue
        checked += 1
        if cells[4] != want:
            report("docs/references.md", i,
                   f"{cells[0]} row claims {cells[4]} objects; "
                   f"tools/reference-builds.lock records {want}")


# --- 8. canonical overlay atlas and generated yaml projection ----------------
# tools/overlay_atlas.py now owns the projection from the shipped tables into
# 107 module records and 106 non-empty code segments. Check its structural
# invariants here as well as its prose claims; `overlay-atlas --check` performs
# the stronger byte-for-byte regeneration check.

with open(os.path.join(ROOT, "config", "overlays.us.json"), encoding="utf-8") as fh:
    atlas = json.load(fh)
modules = atlas.get("modules", [])
totals = atlas.get("totals", {})

expected_totals = {
    "modules": 107,
    "nonempty_modules": 106,
    "text_bytes": 469264,
    "data_rodata_bytes": 61312,
    "bss_bytes": 77680,
    "module_relocations": 18542,
    "cross_overlay_relocations": 608,
    "cross_overlay_edges": 97,
}
for key, expected in expected_totals.items():
    checked += 1
    if totals.get(key) != expected:
        problems.append(
            f"config/overlays.us.json: totals.{key} is {totals.get(key)!r}, "
            f"expected {expected}")

checked += 1
if [m.get("overlay") for m in modules] != list(range(1, 108)):
    problems.append("config/overlays.us.json: modules do not cover overlays 1..107")

for module in modules:
    sections = module.get("sections", {})
    ranges = [sections.get(name, {}) for name in
              ("text", "data_rodata", "reloc1", "reloc2")]
    try:
        starts = [int(row["start"], 0) for row in ranges]
        ends = [int(row["end"], 0) for row in ranges]
    except (KeyError, TypeError, ValueError):
        problems.append(
            f"config/overlays.us.json: overlay {module.get('overlay')} has an "
            "invalid section range")
        continue
    checked += 1
    if any(ends[i] != starts[i + 1] for i in range(3)):
        problems.append(
            f"config/overlays.us.json: overlay {module['overlay']} sections "
            "are not contiguous")
    if starts[0] != int(module["rom"]["start"], 0) or \
            ends[-1] != int(module["rom"]["end"], 0):
        problems.append(
            f"config/overlays.us.json: overlay {module['overlay']} section "
            "ranges do not span its ROM range")

yaml_text = "\n".join(read("mickey.us.yaml"))
generated_segments = re.findall(r"^\s*- name: overlay_(\d{3})$", yaml_text,
                                re.MULTILINE)
checked += 2
if len(generated_segments) != totals.get("nonempty_modules"):
    problems.append(
        "mickey.us.yaml: generated overlay segment count differs from atlas")
if "overlay_032 is a real zero-length header row" not in yaml_text:
    problems.append("mickey.us.yaml: empty overlay 32 is not represented")

claims = {
    r"OverlayHeader\[(\d+)\]": totals.get("modules"),
    r"RomTableEntry\[(\d+)\]": totals.get("rom_table_entries"),
    r"RelocTableEntry\[(\d+)\]": totals.get("resident_relocations"),
}
for pattern, expected in claims.items():
    hits = 0
    rx = re.compile(pattern)
    for path in ("docs/modules.md", "docs/overlays.md", "include/game/runlink.h"):
        for i, line in enumerate(read(path), 1):
            for match in rx.finditer(line):
                hits += 1
                checked += 1
                if int(match.group(1)) != expected:
                    report(path, i, f"claims {match.group(1)}; atlas has {expected}")
    if not hits:
        problems.append(f"no /{pattern}/ claim found; check 8 is checking nothing")


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
