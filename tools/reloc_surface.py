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

# func_80029FE4, D_80003634 -- splat's auto-name for a *resident* address.  An
# overlay that calls one of these spells the call with the resident function's
# own global name, which is the one name the relocation surface must not
# assign: the name is shared with the resident segment, so a value line for it
# would move the symbol for every resident caller too (§5.6).  The alias below
# is the per-module placeholder that carries the addend instead.
RESIDENT_NAME_RE = re.compile(r"^(?:func|D)_8[0-9A-F]{7}$")
RESIDENT_ALIAS_FMT = "%s_o%03dReloc"
RESIDENT_ALIAS_RE = re.compile(r"^(?:func|D)_8[0-9A-F]{7}_o\d{3}Reloc$")


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


def resident_defined_names():
    """Every symbol the *resident* side of the link defines.

    An overlay's call to a resident target does not have to be spelled with a
    splat auto-name.  `overlay5InitializeAudio` calls `alHeapDBAlloc`,
    `osCreateMesgQueue` and `n_alCSPSetMessageQ` -- ordinary libultra globals --
    and a value line for one of those breaks the resident link exactly as
    `func_80034448` does.  What the two shapes have in common is not the name,
    it is that the resident side owns it, so that is what this measures: the
    global symbols defined by every non-overlay object the linker script names,
    plus the names the auto-generated symbol scripts assign.

    Deliberately excludes the overlay objects.  A call to another *module's*
    function is a different case, already handled by the generated-identity
    alias block (S5.3), and must keep its own name.
    """
    ld = (REPO / f"mickey.{VERSION}.ld").read_text()
    out = set()
    for m in re.finditer(r"(build/(?!src/overlays/)\S+?\.o)\(", ld):
        path = REPO / m.group(1)
        if not path.is_file():
            continue
        try:
            elf = Elf(path)
        except SystemExit:
            continue
        for name, _v, _s, info, shndx in elf.symbols():
            if name and shndx != SHN_UNDEF and (info >> 4) != 0:
                out.add(name)
    for fname in (f"undefined_funcs_auto.{VERSION}.txt",
                  f"undefined_syms_auto.{VERSION}.txt",
                  f"libultra_undefined_syms.{VERSION}.txt"):
        path = REPO / fname
        if path.is_file():
            for line in path.read_text().splitlines():
                m = re.match(r"^\s*([A-Za-z_]\w*)\s*=", line)
                if m:
                    out.add(m.group(1))
    return out


def resident_call_aliases(obj_path, overlay, atlas_rows, records,
                          resident_names=frozenset()):
    """(renames, refusals) for this object's calls to *resident* functions.

    An overlay module ships unrelocated, and §5.2's census says a `SYMBOL`
    `R_MIPS_26` record stores immediate zero whether its target is another
    module or the resident segment: the shipped word is `jal 0` and the runtime
    patches it.  So a resident call needs exactly the same addend every other
    cross-module call needs, `0xF0000000`, and nothing about it is special --
    except its *name*.

    Adopted C spells the call with splat's resident auto-name (`func_80029FE4`),
    and that name is global.  Assigning it in `overlay_undefined_syms.us.txt`
    does not give the overlay an addend, it moves the resident function for
    every resident caller as well: `func_80034448` is called from `models.c`,
    `level.c`, `menu.c` and four asm objects, and a value line for it turns all
    of them into `relocation truncated to fit: R_MIPS_26`.  That is the whole of
    the `resident-symbol-missing` and `relocation-truncated` classes.

    The fix is the one the hand-written POSTPROCESS rules already use (overlay
    49 rebinds `func_800254FC` to `overlay65UpdateReloc`): give the *site* a
    per-module placeholder name and value that.  This derives the same rebind
    mechanically -- one alias per (resident name, module) -- so the resident
    name keeps its real address and the overlay keeps its stored addend.

    Only `R_MIPS_26` sites are aliased.  A resident *data* reference stores the
    real resident address in its HI16/LO16 pair, so valuing it under its own
    name is already correct and already what the surface does (`D_80000040 =
    0x80000040`); renaming those would churn the file for no change in bytes.
    """
    elf = Elf(obj_path)
    syms = elf.symbols()
    base, _extent = tu_base(obj_path, overlay, atlas_rows)
    table = {}
    for r in records:
        table.setdefault(r["target_offset"], []).append(r)

    by_symbol = collections.defaultdict(list)
    for _sec, off, rtype, symidx in elf.relocations():
        if symidx >= len(syms):
            continue
        name, _v, _s, _i, shndx = syms[symidx]
        if shndx != SHN_UNDEF:
            continue
        if not (RESIDENT_NAME_RE.match(name) or name in resident_names):
            continue
        by_symbol[name].append((off, rtype))

    renames, refusals, notes = {}, [], []
    for name in sorted(by_symbol):
        sites = by_symbol[name]
        calls = [(off, t) for off, t in sites if t == R_MIPS_26]
        if not calls:
            continue
        other = [(off, t) for off, t in sites if t != R_MIPS_26]
        if other:
            refusals.append((name, "resident symbol reached by both a call and "
                                   "a data reference; one placeholder cannot "
                                   "carry both addends"))
            continue
        if base is None:
            refusals.append((name, "resident call in an object with no "
                                   "text_ownership row: no module offset"))
            continue
        # Corroboration is a *note* here, not a refusal.  `synthesize()`
        # already applies the module table's own filter when it picks the
        # addend, and under the alias a value read from an uncorroborated site
        # can only produce a differing word inside the promoted function --
        # which is the measurement the trial exists to take.  Refusing instead
        # costs the candidate its linked-ROM oracle for no gain:
        # `overlay34SortAndDraw` goes from 168 in-range words back to a bare
        # build failure.  Only the ambiguous cases above are refused.
        if not any([r for r in table.get(base + off, [])
                    if r["mode"] == R_MIPS_26] for off, _t in calls):
            notes.append((name,
                          "no resident call site (module offset %s) is named "
                          "by the module relocation table; the addend is read "
                          "from an uncorroborated site and the differing words "
                          "are reported in range"
                          % ", ".join("%#x" % (base + o) for o, _t in calls)))
        renames[name] = RESIDENT_ALIAS_FMT % (name, overlay)
    return renames, refusals, notes


def rebind_resident_calls(objects, rom, rows, mods, rom_table,
                          objcopy=None, records_cache=None,
                          refused_names=None):
    """Rename every resident-call placeholder in place; return the refusals.

    Idempotent: the alias no longer matches `RESIDENT_NAME_RE`, so a second
    pass finds nothing to rename and `generate()` values the alias from the
    same corroborated site.  `generate --check` therefore reproduces the same
    block whether or not the objects have already been rebound.
    """
    objcopy = objcopy or (REPO / "tools" / "binutils" / "mips64-elf-objcopy")
    refusals, notes = [], []
    refused_names = set() if refused_names is None else refused_names
    resident_names = resident_defined_names()
    for ov, obj in objects:
        recs = (records_cache or {}).get(ov)
        if recs is None:
            recs = ot.read_module_relocations(rom, mods[ov - 1], rom_table)
            if records_cache is not None:
                records_cache[ov] = recs
        try:
            renames, refused, noted = resident_call_aliases(
                obj, ov, rows.get(ov, []), recs, resident_names)
        except SystemExit:
            continue
        refusals += [(obj.name, n, why) for n, why in refused]
        notes += [(obj.name, n, why) for n, why in noted]
        refused_names.update(n for n, _why in refused)
        if not renames:
            continue
        import subprocess
        cmd = [str(objcopy)]
        for old in sorted(renames):
            cmd += ["--redefine-sym", "%s=%s" % (old, renames[old])]
        cmd.append(str(obj))
        subprocess.run(cmd, check=True)
    return refusals, notes


def generate(rom, atlas, objects=None, quiet=False, rebind_resident=True):
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
    records = {}
    # A resident call has to stop spelling itself with the resident function's
    # global name *before* anything is valued; see `rebind_resident_calls`.
    # In the matching tree this renames nothing (no overlay object carries an
    # R_MIPS_26 against a resident auto-name), so the generated block is
    # unchanged; it fires only for a promoted candidate.
    refused_resident = set()
    resident_notes = []
    if rebind_resident:
        refused, resident_notes = rebind_resident_calls(
            objects, rom, rows, mods, rom_table, records_cache=records,
            refused_names=refused_resident)
        conflicts += refused
    for ov, obj in objects:
        if ov not in local:
            local[ov] = module_text_defs(objects, ov, rows.get(ov, []))
        if ov not in records:
            records[ov] = ot.read_module_relocations(rom, mods[ov - 1], rom_table)
        recs = records[ov]
        _sites, vals, unresolved = synthesize(obj, ov, rom, rows.get(ov, []),
                                              recs, local[ov])
        conflicts += [(obj.name, n, why) for n, why in unresolved]
        # A refused resident call must not fall back to a value line under its
        # *global* name.  `synthesize()` still reads an addend for it from the
        # corroborated sites, but assigning `func_8002A8C0` in the linker
        # script moves the resident function for `shadows.c`, `camera.c` and
        # every other resident caller -- 30-odd `relocation truncated to fit`
        # errors, from a line meant to help one overlay.  Refused means
        # refused: no value, and the reason is already reported.
        vals = {k: v for k, v in vals.items() if k not in refused_resident}
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
            "resident_notes": resident_notes, "shadowed": sorted(shadowed)}
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
    text, diag = generate(rom, atlas, rebind_resident=not args.compare)

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
    # A note is not a refusal: the symbol *is* valued and the link resolves it.
    # Printed under its own marker so a caller matching UNRESOLVED does not
    # class a measurable candidate as a failure.
    for obj, name, why in diag.get("resident_notes", []):
        print("/* NOTE %s %s: %s */" % (obj, name, why), file=sys.stderr)
    return 0


# ------------------------------------------------- permuter target annotation
#
# decomp-permuter scores a candidate object against a *target object* it
# assembles from splat's .s.  For an overlay function that comparison is
# invalid, and the reason is the same one section 1 gives: the shipped image
# stores addends, not addresses, so splat's .s assembles with no relocations
# at all.  Every cross-module call reads back as `jal <whatever symbol sits at
# module offset 0>` (the SYMBOL records all store immediate zero) and every
# address materialization as a bare `lui`/`addiu` pair carrying the stored
# addend.  The candidate object, compiled from C, carries an honest
# `R_MIPS_26` / `%hi`+`%lo` reference to a placeholder extern instead.
#
# The scorer's own symbol-difference rule cannot bridge that: it ignores a
# field mismatch only when the candidate's field looks like a symbol *and*
# the target line carries a relocation (scorer.py `field_matches_any_symbol(nf)
# and old_line.has_symbol`), and the target line carries none.  So a candidate
# that differs from the shipped ROM by two words scored 700 -- one insertion
# plus one deletion penalty for every relocation site in the function.
#
# The fix is not to relax the scorer but to give the target the relocations
# the shipped module says are there.  The module's own reloc1/reloc2 tables
# name every site and its type, and for a SYMBOL record they name the callee
# (overlay + offset) outright; that is a *stable identity*, independent of any
# name this tree happens to have chosen.  So:
#
#   * for each relocation the candidate's base object carries inside the
#     function, map its object offset to a module offset and look the site up
#     in the module's table.  A site the table does not name is not a
#     relocation site in the shipped image and is left alone (section 2);
#   * derive a canonical name for the *base symbol* from the record --
#     `__ovsym_o<overlay>_<offset>` for a SYMBOL record, `__ovjmp_<offset>`
#     for an intra-module JUMP, `__ovloc_<value>` for a LOCAL/DATA site whose
#     value the ROM spells at the site -- subtracting whatever addend the
#     object already carries, exactly as `synthesize()` does;
#   * rewrite the target .s line to reference that name symbolically, and
#     rename the candidate's placeholder to the same name with
#     `objcopy --redefine-sym`.
#
# Both sides then render identically at every corroborated site, so the score
# reflects only real codegen difference -- and a candidate that calls the
# *wrong* placeholder still scores a penalty, because the canonical name comes
# from the ROM's record, not from the candidate's own symbol table.
#
# Nothing here writes ROM-derived content anywhere: the rewritten .s lives in
# the permuter's gitignored scratch, exactly like the label rename
# permute_batch.py already performs.

# splat's disassembly line: `/* <rom> <vma> <word> */  mnemonic operands`.
ASM_LINE_RE = re.compile(
    r"^(?P<pre>\s*/\* [0-9A-Fa-f]+ (?P<vma>[0-9A-Fa-f]{8}) [0-9A-Fa-f]{8} \*/\s*)"
    r"(?P<mnem>\S+)(?P<sp>\s*)(?P<ops>.*?)\s*$")
_MEM_OPERAND_RE = re.compile(r"^(?P<disp>.*)\((?P<reg>\$\w+)\)$")


class AnnotationError(Exception):
    """The target .s could not be annotated; the caller falls back."""


def _function_text_symbol(elf, names):
    """(offset, size) of the first of `names` defined in the object's .text."""
    text_idx, _ = elf.section(".text")
    if text_idx is None:
        raise AnnotationError("object has no .text")
    by_name = {}
    for name, value, size, _info, shndx in elf.symbols():
        if shndx == text_idx and name:
            by_name.setdefault(name, (value, size))
    for name in names:
        if name in by_name:
            return by_name[name]
    raise AnnotationError("none of %s is defined in the object's .text"
                          % ", ".join(names))


def permuter_annotation(target_s_text, base_o: Path, func_names, overlay: int,
                        rom: bytes, records=None):
    """Annotate a permuter target .s with the module's own relocation sites.

    Returns ``(annotated_text, renames, notes)``:

      annotated_text  the .s with every corroborated site rewritten to a
                      symbolic operand (unchanged if nothing corroborated);
      renames         {candidate symbol: canonical name} to hand
                      `objcopy --redefine-sym`;
      notes           human-readable diagnostics (site/rename counts, and any
                      symbol whose sites disagree about its identity).

    Raises AnnotationError when the inputs cannot be mapped at all.
    """
    lines = target_s_text.split("\n")
    line_of = {}
    for i, raw in enumerate(lines):
        m = ASM_LINE_RE.match(raw)
        if m:
            line_of[int(m.group("vma"), 16) - SYNTHETIC_VMA] = i
    if not line_of:
        raise AnnotationError("no addressed instruction lines in the target .s")
    fn_start = min(line_of)

    mods = ot.build_modules(ot.read_headers(rom))
    if not 1 <= overlay <= len(mods):
        raise AnnotationError(f"overlay {overlay} is out of range")
    module = mods[overlay - 1]
    if records is None:
        records = ot.read_module_relocations(rom, module, ot.read_rom_table(rom))
    table = {}
    for r in records:
        table.setdefault((r["target_offset"], r["mode"]), r)
    text_start = module["rom_start"]

    elf = Elf(base_o)
    syms = elf.symbols()
    obj_text = elf.section_bytes(".text")
    fn_off, fn_size = _function_text_symbol(elf, func_names)

    sites = []
    for _sec, off, rtype, symidx in elf.relocations():
        if not (fn_off <= off < fn_off + fn_size) or symidx >= len(syms):
            continue
        name = syms[symidx][0]
        if not name:
            continue
        sites.append({"symbol": name, "obj_off": off, "type": rtype,
                      "module_off": fn_start + (off - fn_off)})

    # Pass 1: name each corroborated site.  A HI16/LO16 pair is named by the
    # link *value* the ROM spells at it -- `synthesize()`'s own quantity, and
    # the only identity that is well defined: two placeholders the surface
    # values identically produce the same linked words, whatever the record's
    # operation says the runtime will add to them afterwards.  An `R_MIPS_26`
    # SYMBOL site stores immediate zero (docs/reloc-surface.md section 1), so
    # its value carries no identity at all and the record's own ROM-table
    # entry -- the callee's overlay and offset -- names it instead.
    proposals, annotations = [], {}
    for hi, lo in _pairs(sites):
        anchor = hi or lo
        members = [s for s in (hi, lo) if s is not None]
        recs = [table.get((s["module_off"], s["type"])) for s in members]
        if any(r is None for r in recs):
            continue  # not a relocation site in the shipped image
        rec = recs[0]
        if hi is not None and lo is not None:
            have = ((stored_field(obj_text, hi["obj_off"], R_MIPS_HI16) << 16)
                    + sext16(stored_field(obj_text, lo["obj_off"], R_MIPS_LO16)))
            full = ((stored_field(rom, text_start + hi["module_off"],
                                  R_MIPS_HI16) << 16)
                    + sext16(stored_field(rom, text_start + lo["module_off"],
                                          R_MIPS_LO16)))
            base = "__ovval_%08X" % ((full - have) & 0xFFFFFFFF)
            where = [(hi["module_off"], "hi"), (lo["module_off"], "lo")]
        elif anchor["type"] == R_MIPS_26:
            # `jal sym` carries no addend the assembler can spell back, so a
            # site whose object word is not a bare zero is left alone.
            if stored_field(obj_text, anchor["obj_off"], R_MIPS_26):
                continue
            have = 0
            if rec["op_name"] == "SYMBOL":
                base = "__ovcall_o%d_%X" % (rec["target_overlay"],
                                            rec["target_symbol_offset"])
            elif rec["op_name"] == "JUMP":
                base = "__ovjump_%06X" % (
                    stored_field(rom, text_start + anchor["module_off"],
                                 R_MIPS_26) << 2)
            else:
                continue
            where = [(anchor["module_off"], "26")]
        else:
            continue  # a lone HI16/LO16 or an R_MIPS_32: nothing to pair with
        proposals.append((anchor["symbol"], base, have, where))

    # Pass 2: a symbol whose sites do not agree on one name has no canonical
    # identity, so none of its sites is annotated.  Renaming it either way
    # would make the target disagree with the candidate at the sites that
    # wanted the other name -- worse than leaving it alone, and silently so.
    proposed = collections.defaultdict(set)
    for symbol, base, _have, _where in proposals:
        proposed[symbol].add(base)
    conflicts = ["%s: sites disagree (%s); left unannotated"
                 % (symbol, ", ".join(sorted(names)))
                 for symbol, names in sorted(proposed.items()) if len(names) > 1]
    renames = {symbol: next(iter(names))
               for symbol, names in proposed.items() if len(names) == 1}
    for symbol, base, have, where in proposals:
        if symbol not in renames:
            continue
        for module_off, kind in where:
            annotations[module_off] = (kind, base, have)

    for module_off, (kind, base, have) in annotations.items():
        index = line_of.get(module_off)
        if index is None:
            continue
        m = ASM_LINE_RE.match(lines[index])
        spelled = base if not have else "%s+0x%X" % (base, have)
        if kind == "26":
            operands = spelled
        else:
            prefix = "%hi" if kind == "hi" else "%lo"
            fields = [f.strip() for f in m.group("ops").split(",")]
            mem = _MEM_OPERAND_RE.match(fields[-1])
            if mem:
                fields[-1] = "%s(%s)(%s)" % (prefix, spelled, mem.group("reg"))
            else:
                fields[-1] = "%s(%s)" % (prefix, spelled)
            operands = ", ".join(fields)
        lines[index] = m.group("pre") + m.group("mnem") + m.group("sp") + operands

    notes = ["%d relocation sites annotated, %d placeholder symbols renamed"
             % (len(annotations), len(renames))]
    notes += conflicts
    return "\n".join(lines), renames, notes


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
