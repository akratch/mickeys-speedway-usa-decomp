#!/usr/bin/env python3
"""Synthesize an overlay object's relocation surface from the shipped tables.

Background
----------
An overlay module ships *unrelocated*: `runlinkDownloadCode` patches every
site named by the module's own `reloc1`/`reloc2` tables after the DMA.  What
the ROM image therefore stores at a relocation site is not the final address
but the record's **stored addend** -- the value the runtime adds its base to.
`docs/overlays.md` sections 5.1-5.4 establish that model; this tool is its
mechanical consequence.

A C translation unit compiled for an overlay cannot express those addends: IDO
emits an ordinary `R_MIPS_26` / `R_MIPS_HI16` + `R_MIPS_LO16` reference to a
symbol, and the symbol has no address in this build because it lives in a
different module (or in a section the runtime places).  The project's answer
is a placeholder extern (`overlay1Chain0Reloc`, `D_0210`, ...) whose *value*
is then supplied as a linker-script assignment in
`overlay_undefined_syms.us.txt`, so that the linked instruction word carries
exactly the stored addend the retail image carries.

That value is not a judgement call.  For a candidate whose instruction
schedule already agrees with the target, every placeholder's value is
**readable from the ROM at the site the relocation names**:

    R_MIPS_26   value = SYNTHETIC_VMA | (stored_imm26 << 2)
    HI16/LO16   value = (stored_hi16 << 16) + sign_extend16(stored_lo16)

This tool reads a compiled object, maps each of its undefined-symbol
relocations back to a module text offset, reads the shipped word from the
baserom, and prints the linker-script assignments (and, with --makefile, the
equivalent objcopy spec).  It also cross-checks each site against the decoded
module relocation table, which is what distinguishes a genuine relocation
site from a literal the compiler must not relocate.

Nothing ROM-derived is written: the baserom and the atlas are read at run
time and only *addresses and symbol values already required by the link* are
emitted.

Usage:
    tools/reloc_surface.py OBJECT [--overlay N] [--rom PATH] [--makefile]
    tools/reloc_surface.py --audit           # replay the whole tracked surface
"""

from __future__ import annotations

import argparse
import collections
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import overlay_tables as ot  # noqa: E402

REPO = Path(__file__).resolve().parent.parent
DEFAULT_ROM = REPO / "baseroms" / "mickey.us.z64"
SYNTHETIC_VMA = 0xF0000000
VERSION = "us"
LINK_SYMS = REPO / f"overlay_undefined_syms.{VERSION}.txt"

SHT_SYMTAB = 2
SHT_REL = 9
SHN_UNDEF = 0

R_MIPS_32, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16 = 2, 4, 5, 6
TYPE_NAMES = {R_MIPS_32: "R_MIPS_32", R_MIPS_26: "R_MIPS_26",
              R_MIPS_HI16: "R_MIPS_HI16", R_MIPS_LO16: "R_MIPS_LO16"}

# func_overlay_018_F0000000_18745B8 -- the generated identity carries both the
# synthetic VMA and the ROM address, so a module offset needs no extra table.
GEN_NAME_RE = re.compile(r"^func_overlay_(\d{3})_F([0-9A-F]{7})_[0-9A-F]+$")


# --------------------------------------------------------------------- ELF

class Elf:
    def __init__(self, path: Path):
        self.path = path
        self.data = path.read_bytes()
        d = self.data
        if d[:6] != b"\x7fELF\x01\x02":
            raise SystemExit(f"{path}: expected a big-endian ELF32 object")
        shoff = struct.unpack_from(">I", d, 0x20)[0]
        shentsize = struct.unpack_from(">H", d, 0x2E)[0]
        shnum = struct.unpack_from(">H", d, 0x30)[0]
        shstrndx = struct.unpack_from(">H", d, 0x32)[0]
        self.sh = [struct.unpack_from(">10I", d, shoff + i * shentsize)
                   for i in range(shnum)]
        names_off, names_size = self.sh[shstrndx][4], self.sh[shstrndx][5]
        blob = d[names_off:names_off + names_size]
        self.names = [blob[s[0]:blob.find(b"\0", s[0])].decode() for s in self.sh]

    def section(self, name):
        for i, n in enumerate(self.names):
            if n == name:
                return i, self.sh[i]
        return None, None

    def symbols(self):
        """[(name, value, size, info, shndx)] from the first .symtab."""
        for i, s in enumerate(self.sh):
            if s[1] == SHT_SYMTAB:
                strtab = self.sh[s[6]]
                strs = self.data[strtab[4]:strtab[4] + strtab[5]]
                out = []
                for off in range(s[4], s[4] + s[5], s[9] or 16):
                    nm, val, size, info, _o, shndx = struct.unpack_from(
                        ">IIIBBH", self.data, off)
                    name = strs[nm:strs.find(b"\0", nm)].decode()
                    out.append((name, val, size, info, shndx))
                return out
        return []

    def relocations(self, target=r"\.text"):
        """[(section_name, offset, type, symbol_index)] for REL sections."""
        out = []
        for i, s in enumerate(self.sh):
            if s[1] != SHT_REL:
                continue
            tgt = self.names[s[7]] if s[7] < len(self.names) else ""
            if not re.fullmatch(target, tgt):
                continue
            for off in range(s[4], s[4] + s[5], 8):
                r_off, r_info = struct.unpack_from(">II", self.data, off)
                out.append((tgt, r_off, r_info & 0xFF, r_info >> 8))
        return out

    def section_bytes(self, name):
        i, s = self.section(name)
        return b"" if i is None else self.data[s[4]:s[4] + s[5]]


# ------------------------------------------------------------------ mapping

def tu_base(obj_path: Path, overlay: int, atlas_rows):
    """Module text offset of the translation unit this object came from.

    `config/overlays.us.json` records one contiguous `text_ownership` row per
    source file, so an object's base needs no name convention, no linked ELF,
    and no heuristic -- which matters because the link is exactly what is
    missing when a promotion fails to resolve.  The row's size is the object's
    trimmed `.text` extent, so a size disagreement is a real signal and is
    reported rather than silently mapped.
    """
    stem = obj_path.name
    for suffix in (".c.o", ".o"):
        if stem.endswith(suffix):
            stem = stem[: -len(suffix)]
            break
    for row in atlas_rows:
        if Path(row["source"]).name == stem and row.get("type") == "c":
            return int(row["offset"], 16), int(row["size"], 16)
    return None, None


# ------------------------------------------------------------------- solver

def sext16(v):
    return v - 0x10000 if v & 0x8000 else v


def stored_field(rom, site, rtype):
    word = struct.unpack_from(">I", rom, site)[0]
    if rtype == R_MIPS_26:
        return word & 0x03FFFFFF
    if rtype in (R_MIPS_HI16, R_MIPS_LO16):
        return word & 0xFFFF
    return word


def _pairs(sites):
    """Attach each R_MIPS_HI16 to its matching R_MIPS_LO16.

    IDO emits the pair in order against the same symbol, which is the MIPS REL
    convention `ld` itself relies on.  Pairing matters because a HI16's value
    is only meaningful together with the sign of its LO16.
    """
    pending = collections.defaultdict(list)
    out = []
    for s in sites:
        if s["type"] == R_MIPS_HI16:
            pending[s["symbol"]].append(s)
        elif s["type"] == R_MIPS_LO16:
            his = pending.pop(s["symbol"], [])
            if his:
                for h in his:
                    out.append((h, s))
            else:
                out.append((None, s))
        else:
            out.append((s, None))
    for group in pending.values():
        for h in group:
            out.append((h, None))
    return out


def module_text_defs(objects, overlay, atlas_rows):
    """Names defined in this module's own .text, across all its objects.

    A reference to one of those is the only kind that needs no assignment: an
    intra-module `jal` is a JUMP record, whose stored immediate is the target's
    module offset shifted right two, which is exactly what the assembler emits
    for that symbol at the synthetic VMA.  Every other referenced symbol -- a
    resident function, another module's function, or this module's own data,
    which the runtime places separately -- must carry the stored addend
    instead of whatever address this build happens to give it.
    """
    out = set()
    for ov, obj in objects:
        if ov != overlay:
            continue
        try:
            elf = Elf(obj)
        except SystemExit:
            continue
        text_idx, _ = elf.section(".text")
        if text_idx is None:
            continue
        for name, _v, _s, _i, shndx in elf.symbols():
            if name and shndx == text_idx:
                out.add(name)
    return out


def synthesize(obj_path: Path, overlay: int, rom: bytes, atlas_rows, records,
               local_text=frozenset()):
    """Return (sites, {symbol: required link value}, conflicts).

    The required value is always `shipped_target - addend_already_in_the_object`.
    Subtracting the object's own addend is what lets one base symbol serve many
    struct-field references: IDO puts the field offset in the instruction, and
    only the base belongs in the linker script.
    """
    elf = Elf(obj_path)
    syms = elf.symbols()
    mods = ot.build_modules(ot.read_headers(rom))
    module = mods[overlay - 1]
    ranges = ot.module_section_ranges(module)
    text_start, text_size = ranges["text"][0], module["text_size"]
    base, extent = tu_base(obj_path, overlay, atlas_rows)
    obj_text = elf.section_bytes(".text")
    text_idx, _ = elf.section(".text")

    table = {}
    for r in records:
        table.setdefault(r["target_offset"], []).append(r)

    sites = []
    for _sec, off, rtype, symidx in elf.relocations():
        if symidx >= len(syms):
            continue
        name, _v, _s, _i, shndx = syms[symidx]
        if not name or name in local_text:
            continue
        if shndx != SHN_UNDEF and shndx == text_idx:
            # defined in this object's own .text: an intra-module call, already
            # correct at the synthetic VMA.
            continue
        mod = None if base is None else base + off
        entry = {"symbol": name, "obj_off": off, "type": rtype,
                 "module_off": mod, "stored": None, "obj": None,
                 "in_table": None, "op": None, "note": ""}
        if mod is None or mod >= text_size or off + 4 > len(obj_text):
            entry["note"] = "unmapped"
        else:
            entry["stored"] = stored_field(rom, text_start + mod, rtype)
            entry["obj"] = stored_field(obj_text, off, rtype)
            rec = [r for r in table.get(mod, []) if r["mode"] == rtype]
            entry["in_table"] = bool(rec)
            entry["op"] = rec[0]["op_name"] if rec else None
        sites.append(entry)

    # A site the module's own relocation table does not name is not a
    # relocation site in the shipped image: reading an addend there reads an
    # instruction word.  When any site for a symbol is corroborated by the
    # table, ignore the ones that are not -- that is the schedule-divergence
    # filter, and it is the ROM's own statement, not a heuristic.
    corroborated = {s["symbol"] for s in sites if s["in_table"]}

    wanted = collections.defaultdict(set)
    unmapped = set()
    for hi, lo in _pairs(sites):
        anchor = hi or lo
        name = anchor["symbol"]
        if name in corroborated and not all(
                s["in_table"] for s in (hi, lo) if s is not None):
            continue
        if any(s is not None and s["stored"] is None for s in (hi, lo)):
            unmapped.add(name)
            continue
        if hi is not None and lo is not None and hi["type"] == R_MIPS_HI16:
            target = (hi["stored"] << 16) + sext16(lo["stored"])
            have = (hi["obj"] << 16) + sext16(lo["obj"])
        elif anchor["type"] == R_MIPS_26:
            target = SYNTHETIC_VMA | (anchor["stored"] << 2)
            have = anchor["obj"] << 2
        elif anchor["type"] == R_MIPS_32:
            target, have = anchor["stored"], anchor["obj"]
        elif anchor["type"] == R_MIPS_HI16:
            target, have = anchor["stored"] << 16, anchor["obj"] << 16
        else:
            target, have = sext16(anchor["stored"]), sext16(anchor["obj"])
        wanted[name].add((target - have) & 0xFFFFFFFF)

    # A symbol every one of whose sites was dropped by the corroboration
    # filter is not "fine": it means the module's table corroborates the symbol
    # somewhere, but not at any site this object still spells the same way. No
    # addend is readable, and saying nothing would leave the caller with an
    # undefined reference and no reason for it.
    seen = {s["symbol"] for s in sites}
    filtered = sorted(seen - set(wanted) - unmapped)

    values, conflicts = {}, []
    for name in filtered:
        conflicts.append((name, "no corroborated site survived: the schedule "
                                "diverges at every site the table names"))
    for name in sorted(wanted):
        if name in unmapped:
            conflicts.append((name, "unmapped site"))
        elif len(wanted[name]) == 1:
            values[name] = next(iter(wanted[name]))
        else:
            conflicts.append((name, f"{len(wanted[name])} distinct values: "
                                    f"{sorted(hex(v) for v in wanted[name])}"))
    for name in sorted(unmapped - set(wanted)):
        conflicts.append((name, "unmapped site"))
    return sites, values, conflicts


# ---------------------------------------------------------------- generate
#
# The whole of `overlay_undefined_syms.us.txt` is derivable.  It has exactly
# two kinds of line and both are mechanical:
#
#   value line   `gOverlay14Reset1C = 0x1C;`
#       the stored addend a placeholder extern must carry, synthesized by
#       `synthesize()` above from the baserom at the site the module's own
#       relocation table names.
#
#   alias line   `func_overlay_001_F0000CA8_184D088 = overlay1InterpolatePath;`
#       the generated splat identity for a module offset, pointed at whatever
#       friendly name the adopted C defines there.  `text_ownership` maps the
#       object to its module offset and the object's symbol table supplies the
#       name, so the pair is a pure function of the atlas plus the objects --
#       the same data the `--redefine-sym func_...=<friendly>` half of every
#       POSTPROCESS rule spells by hand.
#
# Generating both removes the duplicate assignments the hand-maintained file
# accumulated, and removes the alias-block coupling that made promoting one
# function in a shared TU strand another function's alias line.

STT_FUNC = 2
STB_GLOBAL = 1

GENERATED_HEADER = """\
/*
 * Generated by tools/reloc_surface.py generate -- do not edit by hand.
 * Regenerate with `gmake overlay-syms`; `gmake check-overlay-syms` fails on
 * drift.  See docs/reloc-surface.md.
 *
 * Raw overlay relocation addends used by adopted C, plus the generated-identity
 * aliases the splat assembly still references. The runtime linker supplies the
 * actual resident/module/section base; resolving these aliases to their stored
 * addends keeps the linked overlay text byte-identical.
 */"""


def generated_identity(overlay, module_off, rom_start):
    return "func_overlay_%03d_F%07X_%X" % (overlay, module_off,
                                           rom_start + module_off)


def linked_overlay_objects():
    """Every overlay object the link actually consumes, in ld-script order.

    The splat linker script names every input object explicitly, which is the
    only authoritative list: an earlier draft filtered build artifacts by
    whether the Makefile mentioned the object's name, and silently dropped the
    21 overlay objects that reach the link through a pattern rule.
    """
    ld = (REPO / f"mickey.{VERSION}.ld").read_text()
    seen = set()
    out = []
    for m in re.finditer(r"build/src/overlays/o(\d{3})/(\S+?\.o)\(", ld):
        rel = f"build/src/overlays/o{m.group(1)}/{m.group(2)}"
        if rel in seen:
            continue
        seen.add(rel)
        path = REPO / rel
        if path.is_file():
            out.append((int(m.group(1)), path))
    return out


def object_aliases(obj_path, overlay, atlas_rows, rom_start):
    """[(generated_identity, friendly_name)] for one object's defined text.

    Only global function symbols matter: a static is not linkable and a name
    that already *is* its generated identity needs no alias (and a self
    assignment would be circular).
    """
    elf = Elf(obj_path)
    text_idx, _ = elf.section(".text")
    if text_idx is None:
        return []
    base, _extent = tu_base(obj_path, overlay, atlas_rows)
    if base is None:
        return []
    out = []
    for name, value, _size, info, shndx in elf.symbols():
        if shndx != text_idx or not name:
            continue
        if (info >> 4) != STB_GLOBAL or (info & 0xF) != STT_FUNC:
            continue
        ident = generated_identity(overlay, base + value, rom_start)
        if name != ident:
            out.append((ident, name))
    return out


def generate(rom, atlas, objects=None, quiet=False):
    """Return the whole linker-script block as text, plus a diagnostics dict."""
    rows = {m["overlay"]: m["text_ownership"] for m in atlas["modules"]}
    rom_starts = {m["overlay"]: int(m["rom"]["start"], 16) for m in atlas["modules"]}
    rom_table = ot.read_rom_table(rom)
    mods = ot.build_modules(ot.read_headers(rom))

    if objects is None:
        objects = linked_overlay_objects()

    values = {}          # name -> (value, first object that asked for it)
    conflicts = []       # (object, name, why)
    value_order = []     # (overlay, object name, [names])
    alias_pairs = {}     # identity -> (name, object)
    alias_order = []
    local = {}
    for ov, obj in objects:
        if ov not in local:
            local[ov] = module_text_defs(objects, ov, rows.get(ov, []))
        recs = ot.read_module_relocations(rom, mods[ov - 1], rom_table)
        _sites, vals, unresolved = synthesize(obj, ov, rom, rows.get(ov, []),
                                              recs, local[ov])
        conflicts += [(obj.name, n, why) for n, why in unresolved]
        mine = []
        for name in sorted(vals):
            if name in values:
                if values[name][0] != vals[name]:
                    conflicts.append((obj.name, name,
                                      "conflicts with %s: %#x vs %#x"
                                      % (values[name][1], values[name][0],
                                         vals[name])))
                continue
            values[name] = (vals[name], obj.name)
            mine.append(name)
        if mine:
            value_order.append((ov, obj, mine))
        mine_aliases = []
        for ident, name in object_aliases(obj, ov, rows.get(ov, []),
                                          rom_starts[ov]):
            if ident in alias_pairs:
                if alias_pairs[ident][0] != name:
                    conflicts.append((obj.name, ident,
                                      "alias conflict: %s vs %s"
                                      % (alias_pairs[ident][0], name)))
                continue
            alias_pairs[ident] = (name, obj.name)
            mine_aliases.append((ident, name))
        if mine_aliases:
            alias_order.append((ov, obj, mine_aliases))

    # A generated identity that is aliased to a friendly name is *defined* by
    # the alias, so a value line for the same name would be a shadowed
    # assignment -- exactly the duplicate the hand-maintained file carried, and
    # ld silently took the last one.  The alias is the definition that must
    # win, so drop the shadowed value lines outright.
    shadowed = set(values) & set(alias_pairs)
    if shadowed:
        values = {k: v for k, v in values.items() if k not in shadowed}
        value_order = [(ov, obj, [n for n in names if n not in shadowed])
                       for ov, obj, names in value_order]
        value_order = [row for row in value_order if row[2]]

    lines = [GENERATED_HEADER, ""]
    lines.append("/* Stored relocation addends, per translation unit. */")
    for ov, obj, names in value_order:
        lines.append("")
        lines.append("/* overlay %d: %s */" % (ov, obj.name))
        for name in names:
            lines.append("%s = %#010x;" % (name, values[name][0]))
    lines.append("")
    lines.append("/* Generated assembly still uses address-qualified labels "
                 "for C boundaries. */")
    for ov, obj, pairs in alias_order:
        lines.append("")
        lines.append("/* overlay %d: %s */" % (ov, obj.name))
        for ident, name in pairs:
            lines.append("%s = %s;" % (ident, name))
    lines.append("")
    text = "\n".join(lines)

    diag = {"values": len(values), "aliases": len(alias_pairs),
            "objects": len(objects), "conflicts": conflicts,
            "shadowed": sorted(shadowed)}
    return text, diag


def cmd_generate(argv):
    p = argparse.ArgumentParser(
        prog="reloc_surface.py generate",
        description="Synthesize the whole overlay_undefined_syms block.")
    p.add_argument("--rom", type=Path, default=DEFAULT_ROM)
    p.add_argument("--out", type=Path, default=LINK_SYMS)
    p.add_argument("--write", action="store_true",
                   help="rewrite --out in place (the default is to print)")
    p.add_argument("--check", action="store_true",
                   help="exit 1 if --out is not what the tree generates")
    p.add_argument("--quiet", action="store_true")
    p.add_argument("--compare", action="store_true",
                   help="report which tracked symbols the generator does not "
                        "reproduce, and which it adds")
    args = p.parse_args(argv)

    import json
    rom = args.rom.read_bytes()
    atlas = json.loads((REPO / "config" / "overlays.us.json").read_text())
    text, diag = generate(rom, atlas)

    if args.compare:
        old = args.out.read_text().splitlines()
        def names(lines):
            out = {}
            for line in lines:
                m = re.match(r"^\s*([A-Za-z_]\w*)\s*=\s*(.+?)\s*;", line)
                if m:
                    out[m.group(1)] = m.group(2)
            return out
        a, b = names(old), names(text.splitlines())
        missing = sorted(set(a) - set(b))
        added = sorted(set(b) - set(a))
        changed = sorted(k for k in set(a) & set(b) if a[k] != b[k])
        print("tracked=%d generated=%d missing=%d added=%d changed=%d"
              % (len(a), len(b), len(missing), len(added), len(changed)))
        for n in changed:
            print("  CHANGED", n, a[n], "->", b[n])
        for n in missing:
            print("  MISSING", n, "=", a[n])
        for n in added[:60]:
            print("  ADDED  ", n, "=", b[n])
        return 0

    if args.check:
        current = args.out.read_text() if args.out.is_file() else ""
        if current == text:
            if not args.quiet:
                print("%s: up to date (%d values, %d aliases, %d objects)"
                      % (args.out.name, diag["values"], diag["aliases"],
                         diag["objects"]))
            return 0
        print("%s: out of date; run `gmake overlay-syms`" % args.out.name,
              file=sys.stderr)
        import difflib
        for line in list(difflib.unified_diff(
                current.splitlines(), text.splitlines(),
                "tracked", "generated", lineterm=""))[:60]:
            print(line, file=sys.stderr)
        return 1

    if args.write:
        args.out.write_text(text)
        if not args.quiet:
            print("%s: %d values, %d aliases from %d objects"
                  % (args.out.name, diag["values"], diag["aliases"],
                     diag["objects"]))
    else:
        sys.stdout.write(text)
    for obj, name, why in diag["conflicts"]:
        print("/* UNRESOLVED %s %s: %s */" % (obj, name, why), file=sys.stderr)
    return 0


# -------------------------------------------------------------------- audit

def tracked_values(path=None):
    out = {}
    path = path or LINK_SYMS
    if not path.is_file():
        return out
    for line in path.read_text().splitlines():
        m = re.match(r"^\s*([A-Za-z_]\w*)\s*=\s*(0x[0-9A-Fa-f]+|\d+)\s*;", line)
        if m:
            out[m.group(1)] = int(m.group(2), 0)
    return out


def overlay_of(path: Path):
    m = re.search(r"/o(\d{3})/", str(path))
    return int(m.group(1)) if m else None


def main(argv):
    if argv and argv[0] == "generate":
        return cmd_generate(argv[1:])
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("object", nargs="*", type=Path)
    p.add_argument("--overlay", type=int)
    p.add_argument("--rom", type=Path, default=DEFAULT_ROM)
    p.add_argument("--against", type=Path, default=LINK_SYMS,
                   help="file the audit replays against (default: the "
                        "generated overlay_undefined_syms block; point it at a "
                        "hand-maintained copy to replay that instead)")
    p.add_argument("--audit", action="store_true",
                   help="replay every built overlay object and score the "
                        "synthesized values against overlay_undefined_syms.us.txt")
    p.add_argument("--sites", action="store_true", help="print every site")
    p.add_argument("--apply", action="store_true",
                   help="rename every synthesized undefined symbol in the "
                        "object to a unique alias and emit its value")
    p.add_argument("--ld-out", type=Path,
                   help="append the synthesized assignments to this linker "
                        "script fragment")
    p.add_argument("--objcopy", type=Path,
                   default=REPO / "tools" / "binutils" / "mips64-elf-objcopy")
    args = p.parse_args(argv)

    rom = args.rom.read_bytes()
    import json
    atlas = json.loads((REPO / "config" / "overlays.us.json").read_text())
    rows = {m["overlay"]: m["text_ownership"] for m in atlas["modules"]}
    rom_table = ot.read_rom_table(rom)
    mods = ot.build_modules(ot.read_headers(rom))

    linked = linked_overlay_objects()
    local_defs = {}
    objects = list(args.object)
    if args.audit:
        objects = [obj for _ov, obj in linked]
    if not objects:
        p.error("no objects")

    stale = []
    known = tracked_values(args.against)
    agree = disagree = unknown = 0
    bad = []
    all_conflicts = []
    for obj in objects:
        ov = args.overlay or overlay_of(obj)
        if ov is None:
            print(f"skip {obj}: no overlay", file=sys.stderr)
            continue
        if ov not in local_defs:
            local_defs[ov] = module_text_defs(linked, ov, rows.get(ov, []))
        recs = ot.read_module_relocations(rom, mods[ov - 1], rom_table)
        sites, values, conflicts = synthesize(obj, ov, rom, rows.get(ov, []),
                                              recs, local_defs[ov])
        all_conflicts += [(obj.name, n, why) for n, why in conflicts]
        if args.audit:
            for name, v in values.items():
                if name in known:
                    if known[name] == v:
                        agree += 1
                    else:
                        disagree += 1
                        bad.append((obj.name, name, hex(known[name]), hex(v)))
                else:
                    unknown += 1
            continue
        if args.sites:
            for s in sites:
                mo = "?" if s["module_off"] is None else f'{s["module_off"]:#x}'
                st = "?" if s["stored"] is None else f'{s["stored"]:#x}'
                print(f'  {s["obj_off"]:#06x} -> {mo:>8} '
                      f'{TYPE_NAMES.get(s["type"], s["type"]):12} '
                      f'stored={st:>10} table={s["op"] or "-":6} {s["symbol"]}')
        alias = {n: f"rs_{ov}_{obj.name.replace('.', '_')}_{n}" for n in values}
        if args.apply and values:
            import subprocess
            cmd = [str(args.objcopy)]
            for n in sorted(values):
                cmd += ["--redefine-sym", f"{n}={alias[n]}"]
            cmd.append(str(obj))
            subprocess.run(cmd, check=True)
        lines = [f"{alias[n] if args.apply else n} = {values[n]:#010x};"
                 for n in sorted(values)]
        if args.ld_out:
            with open(args.ld_out, "a") as fh:
                fh.write(f"/* {obj} */\n" + "\n".join(lines) + "\n")
        print(f"/* {obj.name}: {len(values)} synthesized symbol values */")
        for line in lines:
            print(line)
        for name, why in conflicts:
            print(f"/* UNRESOLVED {name}: {why} */")

    if args.audit:
        total = agree + disagree
        print(f"tracked-value replay: {agree}/{total} agree "
              f"({100.0 * agree / total if total else 0:.1f}%), "
              f"{disagree} disagree, {unknown} not tracked, "
              f"{len(all_conflicts)} unresolved, "
              f"{len(stale)} stale object(s) skipped")
        for row in bad[:40]:
            print("  MISMATCH", *row)
        for row in all_conflicts[:40]:
            print("  UNRESOLVED", *row)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
