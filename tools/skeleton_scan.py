#!/usr/bin/env python3
"""Masked-instruction-skeleton search: match reference-decomp functions
against Mickey's resident text and overlay text by opcode/funct/fmt shape,
with registers, immediates and jump targets masked out. Two functions compare
equal here when they are the same source built by the same compiler,
regardless of register allocation, addresses or constants -- the same
"skeleton" idea `docs/acceleration-survey.md` section 2-3 measures with.

This is a project tool grown out of that review's scratch prototype
(`scratchpad/fingerprint.py` / `kinship.py` / `resident.py` / `selfsim.py`).
It reads the baserom and reference *built object* files at runtime and prints
derived counts and offsets; it never writes ROM bytes, disassembly text, or
hexdumps to any file. `gmake cleanroom` covers this file the same as every
other tracked file -- it holds arithmetic on opcode fields, not instruction
text.

Subcommands
-----------
  scan      -- find reference-function skeletons inside a Mickey region.
  kinship   -- 8-gram kinship of a region against each reference project,
               plus reference-vs-reference calibration rows.
  similar   -- nearest reference functions to one target function, for use
               as in-context examples when hand-matching it.

See docs/skeleton-scan.md for usage and the calibration numbers measured
against this tree.
"""

import argparse
import collections
import json
import os
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BASEROM = ROOT / "baseroms" / "mickey.us.z64"
ATLAS = ROOT / "config" / "overlays.us.json"
SYMBOL_ADDRS = ROOT / "symbol_addrs.us.txt"

# Resident static segment, ROM offsets. Ground truth: mickey.us.yaml's `main`
# segment starts at 0x1050 (the `entry` segment supplies 0x1000-0x1050), and
# its .text runs to 0x76D10 -- the `- [0x76D10, bin]` / `- [0x76E60, data]`
# subsegments there are where .data begins. This is *not* the same as the
# review prototype's "resident tail 0x76D10-0x86990": that range is
# .data/.rodata, not text, so it is excluded here.
RESIDENT_TEXT_START = 0x1000
RESIDENT_TEXT_END = 0x76D10
RESIDENT_VRAM_BASE = 0x80000400  # VRAM of ROM 0x1000 (entrypoint; see mickey.us.yaml)

DEFAULT_REFS_ROOT = Path(os.environ.get("MICKEY_DECOMP_REFS", str(Path.home() / "Desktop" / "dev" / "decomp-refs")))
# short project tag -> directory name under DEFAULT_REFS_ROOT
DEFAULT_REF_PROJECTS = {
    "dkr": "diddy-kong-racing",
    "jfg": "jfg",
    "pd": "perfect_dark",
    "bk": "banjo-kazooie",
    "conker": "conker",
}

DEFAULT_MIN_WORDS = 10


def resident_rom_to_vram(rom_off):
    return rom_off - RESIDENT_TEXT_START + RESIDENT_VRAM_BASE


def resident_vram_to_rom(vram):
    return vram - RESIDENT_VRAM_BASE + RESIDENT_TEXT_START


# ---------------------------------------------------------------------------
# Masking. Keeps the fields that identify *shape* (primary opcode; SPECIAL's
# funct; REGIMM's rt sub-opcode; COP1's fmt and, for arithmetic, funct) and
# discards everything a recompile of the same source can change: register
# numbers, immediates, branch/jump targets. Each masked word packs down to 2
# bytes so a masked function is half the size of its raw form.
# ---------------------------------------------------------------------------

def mask_word(w):
    """Reduce one big-endian MIPS instruction word to its opcode-shape key.

    Pure bit arithmetic on the 32-bit word; not disassembly, and it never
    produces or consumes mnemonic text. Deterministic and stdlib-only so it
    can be unit-tested with synthetic words.
    """
    op = (w >> 26) & 0x3F
    if op == 0:  # SPECIAL: shape is the funct field
        return (op << 6) | (w & 0x3F)
    if op == 1:  # REGIMM: shape is the rt sub-opcode
        return (op << 6) | ((w >> 16) & 0x1F)
    if op == 0x11:  # COP1
        fmt = (w >> 21) & 0x1F
        if fmt >= 0x10:  # single/double arithmetic: fmt + funct both matter
            return (0x40 << 6) | ((fmt << 6) & 0xFC0) | (w & 0x3F)
        return (0x41 << 6) | fmt  # mfc1/mtc1/bc1 family: fmt alone
    return op << 6


def masked_bytes(data):
    """Mask every big-endian word of `data`, packed 2 bytes per instruction.

    Trailing bytes that don't complete a word are dropped.
    """
    out = bytearray()
    for i in range(0, len(data) - 3, 4):
        w = struct.unpack_from(">I", data, i)[0]
        out += struct.pack(">H", mask_word(w))
    return bytes(out)


# ---------------------------------------------------------------------------
# Reference object parsing: pull FUNC/NOTYPE symbols out of .text in a
# big-endian MIPS ELF relocatable object, and slice .text between successive
# symbol starts to get function bodies. Same approach as
# tools/find_known_objects.py uses for whole-object matching; here we chop
# each object into its member functions.
# ---------------------------------------------------------------------------

def parse_elf_o(path):
    try:
        d = path.read_bytes()
    except OSError:
        return None
    if d[:6] != b"\x7fELF\x01\x02":
        return None
    if len(d) < 0x34:
        return None
    shoff = struct.unpack_from(">I", d, 0x20)[0]
    shentsize, shnum, shstrndx = struct.unpack_from(">HHH", d, 0x2E)
    if not shoff or not shnum:
        return None
    secs = []
    for i in range(shnum):
        off = shoff + i * shentsize
        name, typ, flags, addr, offset, size, link, info, align, entsize = struct.unpack_from(">10I", d, off)
        secs.append(dict(name=name, type=typ, offset=offset, size=size, link=link, info=info))
    shstr = secs[shstrndx]

    def sname(s):
        o = shstr["offset"] + s["name"]
        end = d.index(b"\0", o)
        return d[o:end].decode(errors="replace")

    for s in secs:
        s["n"] = sname(s)
    text = next((s for s in secs if s["n"] == ".text"), None)
    symtab = next((s for s in secs if s["type"] == 2), None)  # SHT_SYMTAB
    if not text or not symtab or text["size"] == 0:
        return None
    strtab = secs[symtab["link"]]
    textidx = secs.index(text)
    syms = []
    for i in range(symtab["size"] // 16):
        o = symtab["offset"] + i * 16
        st_name, st_value, st_size, st_info, st_other, st_shndx = struct.unpack_from(">IIIBBH", d, o)
        if st_shndx != textidx:
            continue
        typ = st_info & 0xF
        if typ not in (0, 2):  # STT_NOTYPE, STT_FUNC
            continue
        so = strtab["offset"] + st_name
        end = d.index(b"\0", so)
        nm = d[so:end].decode(errors="replace")
        if not nm or nm.startswith(".") or nm.startswith("$"):
            continue
        syms.append((st_value, nm))
    syms.sort()
    tdata = d[text["offset"]: text["offset"] + text["size"]]
    funcs = []
    for i, (v, nm) in enumerate(syms):
        end = syms[i + 1][0] if i + 1 < len(syms) else text["size"]
        if end <= v:
            continue
        funcs.append((nm, tdata[v:end]))
    return funcs


def resolve_ref_dirs(refs_arg):
    """--refs values -> list of (project_tag, Path). Defaults to the five
    named reference builds under DEFAULT_REFS_ROOT."""
    if not refs_arg:
        out = []
        for tag, dirname in DEFAULT_REF_PROJECTS.items():
            p = DEFAULT_REFS_ROOT / dirname
            if p.is_dir():
                out.append((tag, p))
        return out
    out = []
    alias_by_dirname = {v: k for k, v in DEFAULT_REF_PROJECTS.items()}
    for raw in refs_arg:
        p = Path(raw).expanduser().resolve()
        tag = alias_by_dirname.get(p.name, re.sub(r"[^a-z0-9]+", "", p.name.lower()) or p.name)
        out.append((tag, p))
    return out


def load_refs(ref_dirs, min_words=DEFAULT_MIN_WORDS, verbose=False):
    """-> list of (project_tag, name, masked_bytes, size_bytes)"""
    refs = []
    for tag, base in ref_dirs:
        n = 0
        for p in base.rglob("*.o"):
            funcs = parse_elf_o(p)
            if not funcs:
                continue
            for nm, body in funcs:
                if len(body) // 4 < min_words:
                    continue
                refs.append((tag, nm, masked_bytes(body), len(body)))
                n += 1
        if verbose:
            print(f"{tag}: {n} functions >= {min_words} words ({base})", file=sys.stderr)
    return refs


def skel_index(refs):
    """masked skeleton bytes -> set of (project, name); also -> a representative size."""
    by_skel = collections.defaultdict(set)
    size_of = {}
    for proj, nm, mb, sz in refs:
        by_skel[mb].add((proj, nm))
        size_of.setdefault(mb, sz)
    return by_skel, size_of


# ---------------------------------------------------------------------------
# Mickey-side region loading.
# ---------------------------------------------------------------------------

def load_rom():
    if not BASEROM.is_file():
        sys.exit(
            f"error: baserom not found at {BASEROM} "
            "(see docs/CONTRIBUTING.md setup)"
        )
    return BASEROM.read_bytes()


def load_atlas():
    return json.loads(ATLAS.read_text())


def named_resident_starts():
    """symbol_addrs.us.txt type:func lines -> {rom_offset}."""
    if not SYMBOL_ADDRS.is_file():
        return set()
    starts = set()
    line_re = re.compile(r"^(\w+)\s*=\s*0x([0-9A-Fa-f]+)\s*;\s*//\s*(.*)$")
    for line in SYMBOL_ADDRS.read_text().splitlines():
        m = line_re.match(line.strip())
        if not m or "type:func" not in m.group(3):
            continue
        vram = int(m.group(2), 16)
        if vram < RESIDENT_VRAM_BASE:
            continue
        starts.add(resident_vram_to_rom(vram))
    return starts


def overlay_regions(atlas, overlay_filter=None):
    """-> list of dicts: overlay, rom_start, body(bytes), matched_ranges (offset pairs)"""
    rom = load_rom()
    out = []
    for m in atlas["modules"]:
        if overlay_filter is not None and m["overlay"] not in overlay_filter:
            continue
        t = m["sections"]["text"]
        s, e = int(t["start"], 16), int(t["end"], 16)
        if e <= s:
            continue
        matched = [
            (int(o["offset"], 16), int(o["end_offset"], 16))
            for o in m.get("text_ownership", [])
            if o.get("matched")
        ]
        out.append(dict(overlay=m["overlay"], rom_start=s, body=rom[s:e], matched=matched))
    return out


# ---------------------------------------------------------------------------
# Greedy non-overlapping skeleton search over one body of masked bytes.
# ---------------------------------------------------------------------------

def find_hits(body_masked, by_skel):
    """-> list of (offset_bytes, size_bytes, names) sorted by offset, greedy
    longest-match-first, non-overlapping."""
    found = []
    for skel, names in by_skel.items():
        if not skel:
            continue
        start = 0
        while True:
            i = body_masked.find(skel, start)
            if i < 0:
                break
            if i % 2 == 0:  # must land on a word boundary (2 bytes/word)
                found.append((i // 2 * 4, len(skel) // 2 * 4, names))
            start = i + 2
    found.sort(key=lambda x: -x[1])
    taken = []
    hits = []
    for off, sz, names in found:
        if any(not (off + sz <= a or off >= b) for a, b in taken):
            continue
        taken.append((off, off + sz))
        hits.append((off, sz, names))
    hits.sort(key=lambda h: h[0])
    return hits


# ---------------------------------------------------------------------------
# scan
# ---------------------------------------------------------------------------

def cmd_scan(args):
    ref_dirs = resolve_ref_dirs(args.refs)
    if not ref_dirs:
        sys.exit("error: no reference directories found/resolved; pass --refs")
    refs = load_refs(ref_dirs, min_words=args.min_words, verbose=not args.json)
    by_skel, _ = skel_index(refs)

    region = args.region
    results = []  # dicts, uniform across region kinds

    if region == "resident":
        rom = load_rom()
        body = rom[RESIDENT_TEXT_START:RESIDENT_TEXT_END]
        mb = masked_bytes(body)
        hits = find_hits(mb, by_skel)
        named = named_resident_starts()
        for off, sz, names in hits:
            rom_off = RESIDENT_TEXT_START + off
            already = rom_off in named
            if args.unnamed_only and already:
                continue
            results.append(dict(
                region="resident", rom_offset=rom_off, vram=resident_rom_to_vram(rom_off),
                size=sz, donors=sorted(f"{p}:{n}" for p, n in names),
                ambiguity=len(names),
                already_named=already,
            ))

    elif region == "overlays":
        atlas = load_atlas()
        for ov in overlay_regions(atlas):
            mb = masked_bytes(ov["body"])
            hits = find_hits(mb, by_skel)
            for off, sz, names in hits:
                already = any(a <= off and off + sz <= b for a, b in ov["matched"])
                if args.unnamed_only and already:
                    continue
                results.append(dict(
                    region="overlays", overlay=ov["overlay"], rom_offset=ov["rom_start"] + off,
                    offset=off, size=sz, donors=sorted(f"{p}:{n}" for p, n in names),
                    ambiguity=len(names), already_named=already,
                ))

    elif region.startswith("rom:"):
        spec = region[len("rom:"):]
        try:
            lo_s, hi_s = spec.split("-", 1)
            lo, hi = int(lo_s, 16), int(hi_s, 16)
        except ValueError:
            sys.exit("error: --region rom:START-END expects hex offsets, e.g. rom:0x1000-0x76D10")
        rom = load_rom()
        body = rom[lo:hi]
        mb = masked_bytes(body)
        hits = find_hits(mb, by_skel)
        for off, sz, names in hits:
            rom_off = lo + off
            results.append(dict(
                region="rom", rom_offset=rom_off, size=sz,
                donors=sorted(f"{p}:{n}" for p, n in names), ambiguity=len(names),
                already_named=False,
            ))
    else:
        sys.exit(f"error: unknown --region {region!r} (expected resident, overlays, or rom:START-END)")

    if args.json:
        print(json.dumps(results, indent=2))
        return

    total_bytes = sum(r["size"] for r in results)
    print(f"\n{len(results)} hits, {total_bytes} bytes, region={region}, min_words={args.min_words}")
    for r in results:
        loc = (f"ROM 0x{r['rom_offset']:X} vram 0x{r['vram']:X}" if region == "resident"
               else f"o{r['overlay']:03d} +0x{r['offset']:X} (ROM 0x{r['rom_offset']:X})" if region == "overlays"
               else f"ROM 0x{r['rom_offset']:X}")
        tag = " [named]" if r["already_named"] else ""
        donors = ", ".join(r["donors"][:4])
        print(f"  {loc:38s} {r['size']:5d}B  amb={r['ambiguity']:<3d} {donors}{tag}")

    if args.emit_symbols:
        print("\n# --emit-symbols: only meaningful for --region resident (symbol_addrs.us.txt")
        print("# names resident VRAM symbols only). Paste candidates into symbol_addrs.us.txt")
        print("# yourself, with a PROVENANCE note -- this tool never writes to that file.")
        if region != "resident":
            print("# (region is not 'resident': skipped)")
        else:
            for r in results:
                if r["already_named"] or r["ambiguity"] != 1:
                    continue
                proj, name = r["donors"][0].split(":", 1)
                ident = re.sub(r"[^A-Za-z0-9_]", "_", name)
                print(f"{ident} = 0x{r['vram']:X}; // type:func size:0x{r['size']:X} tier:B skeleton:{proj}:{name}")


# ---------------------------------------------------------------------------
# kinship
# ---------------------------------------------------------------------------

def grams(mb, n):
    words = [mb[i:i + 2] for i in range(0, len(mb), 2)]
    return {b"".join(words[i:i + n]) for i in range(0, len(words) - n + 1)}


def cmd_kinship(args):
    ref_dirs = resolve_ref_dirs(args.refs)
    if not ref_dirs:
        sys.exit("error: no reference directories found/resolved; pass --refs")
    refs = load_refs(ref_dirs, min_words=args.min_words, verbose=True)
    proj_grams = collections.defaultdict(set)
    for proj, nm, mb, sz in refs:
        proj_grams[proj] |= grams(mb, args.ngram)
    tags = [t for t, _ in ref_dirs]

    def report(label, data):
        g = grams(masked_bytes(data), args.ngram)
        if not g:
            print(f"{label}: empty")
            return
        row = [f"{label:38s} {len(data):7d}B grams={len(g):6d}"]
        for t in tags:
            row.append(f"{t}:{100 * len(g & proj_grams[t]) / len(g):5.1f}%")
        allref = set().union(*proj_grams.values()) if proj_grams else set()
        row.append(f"any:{100 * len(g & allref) / len(g):5.1f}%")
        print("  ".join(row))

    print(f"{args.ngram}-gram kinship (fraction of a region's masked {args.ngram}-word n-grams "
          "that occur in a reference build):")
    print("\ncalibration -- reference project vs reference project:")
    for a in tags:
        ga = proj_grams[a]
        if not ga:
            continue
        others = "  ".join(f"{b}:{100 * len(ga & proj_grams[b]) / len(ga):5.1f}%" for b in tags if b != a)
        print(f"  ref {a:8s} vs others: {others}")

    print()
    rom = load_rom()
    report("mickey resident text 0x1000-0x76D10", rom[RESIDENT_TEXT_START:RESIDENT_TEXT_END])
    atlas = load_atlas()
    allov = b"".join(ov["body"] for ov in overlay_regions(atlas))
    report("mickey ALL overlay text", allov)
    for ov in overlay_regions(atlas):
        if len(ov["body"]) >= 8000:
            report(f"overlay {ov['overlay']:03d}", ov["body"])


# ---------------------------------------------------------------------------
# similar
# ---------------------------------------------------------------------------

def _atlas_range_int(row, field, *, overlay, kind):
    value = row.get(field)
    try:
        if isinstance(value, bool):
            raise ValueError
        if isinstance(value, int):
            parsed = value
        elif isinstance(value, str):
            parsed = int(value, 0)
        else:
            raise ValueError
    except ValueError:
        sys.exit(
            f"error: overlay {overlay} has invalid {kind} {field} {value!r}"
        )
    return parsed


def _function_sized_mixed_range(row, *, overlay, text_size):
    """Validate one mixed-TU exact-C row and return its half-open extent."""
    if not isinstance(row, dict):
        sys.exit(f"error: overlay {overlay} has a malformed mixed-TU exact row")
    start = _atlas_range_int(
        row, "offset", overlay=overlay, kind="mixed-TU exact row"
    )
    end = _atlas_range_int(
        row, "end_offset", overlay=overlay, kind="mixed-TU exact row"
    )
    size = _atlas_range_int(
        row, "size", overlay=overlay, kind="mixed-TU exact row"
    )
    label = row.get("label")
    source = row.get("source")
    if (
        start < 0
        or start >= end
        or size != end - start
        or start % 4
        or end % 4
        or end > text_size
        or not isinstance(label, str)
        or not label
        or not isinstance(source, str)
        or not source
    ):
        sys.exit(
            f"error: overlay {overlay} mixed-TU exact row at +0x{start:X} "
            "is not one unambiguous function-sized range"
        )
    return start, end


def resolve_target_bytes(target, atlas_cache):
    """target: 'vram:0x8000abcd' style resident vram, or 'N:+0xOFF' overlay
    spec. -> (label, bytes) or None if boundaries can't be determined."""
    m = re.match(r"^(\d+):\+?0x([0-9A-Fa-f]+)$", target)
    if m:
        ov_num, off_s = int(m.group(1)), int(m.group(2), 16)
        off = off_s
        atlas = atlas_cache.setdefault("atlas", load_atlas())
        modules = [row for row in atlas.get("modules", []) if row.get("overlay") == ov_num]
        if not modules:
            sys.exit(f"error: overlay {ov_num} not found in atlas")
        if len(modules) != 1:
            sys.exit(f"error: overlay {ov_num} has ambiguous module identity in atlas")
        regions = overlay_regions(atlas, overlay_filter={ov_num})
        if len(regions) != 1:
            sys.exit(f"error: overlay {ov_num} has no unique non-empty text region")
        module = modules[0]
        body = regions[0]["body"]

        mixed = []
        for row in module.get("mixed_tu_exact_c_ranges", []):
            start, end = _function_sized_mixed_range(
                row, overlay=ov_num, text_size=len(body)
            )
            if start == off:
                mixed.append((start, end))
        if len(mixed) > 1:
            sys.exit(
                f"error: o{ov_num:03d}+0x{off:X} has ambiguous mixed-TU exact identity"
            )
        if mixed:
            start, end = mixed[0]
            return f"o{ov_num:03d}+0x{off:X}", body[start:end]

        ownership = [(a, b) for a, b in regions[0]["matched"] if a == off]
        if len(ownership) > 1:
            sys.exit(
                f"error: o{ov_num:03d}+0x{off:X} has ambiguous text_ownership identity"
            )
        if ownership:
            start, end = ownership[0]
            return f"o{ov_num:03d}+0x{off:X}", body[start:end]
        sys.exit(
            f"error: o{ov_num:03d}+0x{off:X} is not a known function start in "
            "mixed_tu_exact_c_ranges or text_ownership; pass a resident vram instead, "
            "or extend the overlay atlas"
        )
    m = re.match(r"^(?:vram:)?0x([0-9A-Fa-f]+)$", target)
    if m:
        vram = int(m.group(1), 16)
        rom_off = resident_vram_to_rom(vram)
        named = {}
        line_re = re.compile(r"^(\w+)\s*=\s*0x([0-9A-Fa-f]+)\s*;\s*//\s*.*\bsize:0x([0-9A-Fa-f]+)")
        for line in SYMBOL_ADDRS.read_text().splitlines():
            mm = line_re.match(line.strip())
            if mm:
                named[int(mm.group(2), 16)] = int(mm.group(3), 16)
        if vram not in named:
            sys.exit(f"error: vram 0x{vram:X} has no size:0x.. entry in symbol_addrs.us.txt "
                      "to bound the function; --similar needs a known start+size")
        sz = named[vram]
        rom = load_rom()
        return f"vram:0x{vram:X}", rom[rom_off:rom_off + sz]
    sys.exit("error: --target expects 'N:+0xOFF' (overlay) or '0xVRAM' (resident)")


def cmd_similar(args):
    ref_dirs = resolve_ref_dirs(args.refs)
    if not ref_dirs:
        sys.exit("error: no reference directories found/resolved; pass --refs")
    refs = load_refs(ref_dirs, min_words=args.min_words, verbose=False)

    label, body = resolve_target_bytes(args.target, {})
    if len(body) // 4 < args.min_words:
        print(f"warning: target is only {len(body) // 4} words, below --min-words {args.min_words}",
              file=sys.stderr)
    target_mb = masked_bytes(body)
    target_grams = grams(target_mb, args.ngram)
    target_size = len(body)
    lo, hi = target_size * 0.7, target_size * 1.3

    scored = []
    for proj, nm, mb, sz in refs:
        if not (lo <= sz <= hi):
            continue
        g = grams(mb, args.ngram)
        if not g and not target_grams:
            continue
        union = target_grams | g
        if not union:
            continue
        jac = len(target_grams & g) / len(union)
        scored.append((jac, proj, nm, sz))
    scored.sort(key=lambda x: -x[0])
    top = scored[: args.top]

    if args.json:
        print(json.dumps(
            [dict(project=p, name=n, size=sz, jaccard=round(j, 4)) for j, p, n, sz in top],
            indent=2))
        return

    print(f"target {label}: {target_size} bytes ({target_size // 4} words), "
          f"size window [{int(lo)}, {int(hi)}]")
    print(f"top {len(top)} by masked {args.ngram}-gram Jaccard, size within +/-30%:")
    for j, p, n, sz in top:
        print(f"  {j:5.3f}  {p}:{n:40s} {sz:5d}B")


# ---------------------------------------------------------------------------
# argparse
# ---------------------------------------------------------------------------

def build_parser():
    ap = argparse.ArgumentParser(prog="skeleton_scan.py", description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)

    def add_common(p):
        p.add_argument("--refs", nargs="+", metavar="DIR",
                        help="reference project directories (default: the five named builds "
                             f"under {DEFAULT_REFS_ROOT}, or $MICKEY_DECOMP_REFS)")
        p.add_argument("--min-words", type=int, default=DEFAULT_MIN_WORDS,
                        help=f"minimum function size in words to index/match (default {DEFAULT_MIN_WORDS})")
        p.add_argument("--json", action="store_true", help="machine-readable output")

    p_scan = sub.add_parser("scan", help="find reference-function skeletons in a Mickey region")
    add_common(p_scan)
    p_scan.add_argument("--region", default="resident",
                         help="resident | overlays | rom:START-END (hex), default resident")
    p_scan.add_argument("--unnamed-only", action="store_true",
                         help="drop hits that start at an already-named/matched location")
    p_scan.add_argument("--emit-symbols", action="store_true",
                         help="also print ready-to-paste symbol_addrs.us.txt candidate lines "
                              "for unambiguous, unnamed resident hits")
    p_scan.set_defaults(func=cmd_scan)

    p_kin = sub.add_parser("kinship", help="8-gram kinship of Mickey regions vs reference projects")
    add_common(p_kin)
    p_kin.add_argument("--ngram", type=int, default=8, help="n-gram length in words (default 8)")
    p_kin.set_defaults(func=cmd_kinship)

    p_sim = sub.add_parser("similar", help="nearest reference functions to one target function")
    add_common(p_sim)
    p_sim.add_argument("--target", required=True,
                        help="'N:+0xOFF' for overlay N at text offset OFF (must be a known "
                             "exact mixed-TU or text_ownership function start), or '0xVRAM' "
                             "for a resident "
                             "function (must have a size:0x.. entry in symbol_addrs.us.txt)")
    p_sim.add_argument("--top", type=int, default=10)
    p_sim.add_argument("--ngram", type=int, default=4, help="n-gram length in words (default 4)")
    p_sim.set_defaults(func=cmd_similar)

    return ap


def main():
    ap = build_parser()
    args = ap.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
