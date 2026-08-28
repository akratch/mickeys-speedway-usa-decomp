#!/usr/bin/env python3
"""Linked-ROM promotion trial for every NON_MATCHING candidate.

The permuter scores a scratch object, and for overlay functions that scratch
can never reach zero: the splat target spells every call relocation with a
placeholder symbol and the real build resolves calls through the overlay
relocation table, so identical instruction words still score as mismatches.
The only oracle that counts is the linked ROM. This tool, for each candidate:

  1. splices the candidate over its `#ifdef NON_MATCHING ... #endif` wrapper
     (exactly what a promotion does), rebuilds the ROM, and byte-compares it
     with the baserom;
  2. maps every differing byte to the function's own ROM range or outside it;
  3. restores the source and records the result.

Classes reported per function:
  exact          ROM byte-identical: the candidate can be promoted as is
  text-exact     no difference inside the function's range, some outside it
                 (data/bss layout or relocation-table collateral: an
                 ownership fix, not code work)
  text-differs   N words differ inside the function's range
  build-error    the promoted tree did not build

Run it in a lane, never in the canonical worktree: it rewrites source files
one at a time and rebuilds ~20 s per candidate.

    tools/promotion_trial.py [--overlay N] [--function NAME] [--limit N]
                             [--resume] [--jobs 6]

Results accumulate in build/promotion-trial.json and .txt.
"""

from __future__ import annotations

import argparse
import dataclasses
import json
import os
import re
import subprocess
import sys
import time
from pathlib import Path
from typing import Optional

sys.path.insert(0, str(Path(__file__).resolve().parent))
import permute_batch as pb  # noqa: E402
import reloc_surface as rs  # noqa: E402

ROOT = pb.ROOT
BASEROM = ROOT / "baseroms" / "mickey.us.z64"
ROM = ROOT / "build" / "mickey.us.z64"
OUT_JSON = ROOT / "build" / "promotion-trial.json"
OUT_TXT = ROOT / "build" / "promotion-trial.txt"
LINK_SYMS = ROOT / "overlay_undefined_syms.us.txt"

# A relocation the link cannot resolve is reported by ld as one of these.
UNDEF_RE = re.compile(r"undefined reference to [`'\"]([A-Za-z_][A-Za-z0-9_]*)")
MARKER_RE = re.compile(r"PROMOTION-TRIAL: ([^:]+): (.*)")
TRUNC_RE = re.compile(r"relocation truncated to fit: (\S+)")
# A splat auto-name in the resident address space: func_8002997C, D_80003634.
RESIDENT_RE = re.compile(r"^(func|D)_8[0-9A-F]{7}$")


@dataclasses.dataclass
class Trial:
    func: str
    c_file: str
    overlay: Optional[int]
    rom_start: Optional[int]
    rom_end: Optional[int]
    klass: str = "unknown"
    in_range_words: int = 0
    out_of_range_bytes: int = 0
    first_in_range: Optional[int] = None
    first_out_of_range: Optional[int] = None
    seconds: float = 0.0
    error: Optional[str] = None
    cause: Optional[str] = None


def overlay_ranges() -> dict[str, tuple[int, int]]:
    """source stem -> (rom_start, rom_end) for every overlay ownership row."""
    atlas = json.loads((ROOT / "config" / "overlays.us.json").read_text())
    out: dict[str, tuple[int, int]] = {}
    for module in atlas["modules"]:
        base = int(module["rom"]["start"], 16)
        for row in module["text_ownership"]:
            if row.get("type") != "c":
                continue
            start = base + int(row["offset"], 16)
            out[row["source"]] = (start, start + int(row["size"], 16))
    return out


def resident_ranges() -> dict[str, tuple[int, int]]:
    """function -> (rom_start, rom_end) from the linked ELF (resident only)."""
    elf = ROOT / "build" / "mickey.us.elf"
    objdump = ROOT / "tools" / "binutils" / "mips64-elf-objdump"
    if not elf.is_file():
        return {}
    text = subprocess.run([str(objdump), "-t", str(elf)], capture_output=True, text=True).stdout
    out: dict[str, tuple[int, int]] = {}
    for line in text.splitlines():
        if " F " not in line:
            continue
        tok = line.split()
        if len(tok) < 4:
            continue
        name, size_hex, section = tok[-1], tok[-2], tok[-3]
        if not section.startswith(".text"):
            continue
        try:
            vram = int(tok[0], 16)
            size = int(size_hex, 16)
        except ValueError:
            continue
        # Resident text is loaded contiguously from ROM 0x1000 at 0x80000400.
        rom = vram - 0x80000400 + 0x1000
        out[name] = (rom, rom + size)
    return out


def splice(item: pb.QueueItem) -> Optional[str]:
    """Promote the candidate in place; return the original text (or None)."""
    original = item.c_file.read_text()

    def replace_block(m: re.Match) -> str:
        fn = pb.FUNC_DEF_RE.search(m.group("body"))
        if fn and fn.group("name") == item.func:
            return m.group("body")
        return m.group(0)

    new_text, n = pb.NON_MATCHING_BLOCK_RE.subn(replace_block, original)
    if n == 0 or new_text == original:
        return None
    item.c_file.write_text(new_text)
    return original


def build(jobs: int, full_log: bool = False) -> tuple[bool, str]:
    """One `gmake` pass with the POSTPROCESS guards in report-and-skip mode.

    PROMOTION_TRIAL turns every digest-guarded normalization into a marker line
    plus a skipped pass (tools/postprocess_guard.py), so a candidate whose
    codegen is the wrong size still reaches the link and still produces a ROM.
    The ROM is not a valid build and is never verified; it exists to be diffed.
    """
    env = dict(os.environ, PROMOTION_TRIAL="1")
    r = subprocess.run(["gmake", f"-j{jobs}"], cwd=ROOT, capture_output=True,
                       text=True, timeout=1800, env=env)
    log = r.stdout + r.stderr
    return r.returncode == 0, log if full_log else log[-6000:]


def synthesize_surface() -> tuple[bool, str]:
    """Regenerate overlay_undefined_syms.us.txt from the objects on disk.

    This is the whole point of the integration. A promoted candidate spells its
    cross-module and section references with placeholder externs that have no
    address in this build; the surface supplies each one's stored addend, read
    from the baserom at the site the module's own relocation table names. It
    has to run *after* the candidate compiles and *before* the link, which is
    exactly the window a plain `gmake` does not offer -- so the trial builds,
    regenerates, and builds again. The second pass relinks only.
    """
    r = subprocess.run([sys.executable, str(ROOT / "tools" / "reloc_surface.py"),
                        "generate", "--write", "--quiet"],
                       cwd=ROOT, capture_output=True, text=True, timeout=900)
    return r.returncode == 0, r.stdout + r.stderr


def undefined_sites(c_file: Path, symbol: str) -> set[str]:
    """Section names of every relocation in `c_file`'s object naming `symbol`."""
    rel = c_file.relative_to(ROOT)
    obj = ROOT / "build" / rel.parent / (rel.name + ".o")
    if not obj.is_file():
        return set()
    try:
        elf = rs.Elf(obj)
    except SystemExit:
        return set()
    syms = elf.symbols()
    idx = {i for i, s in enumerate(syms) if s[0] == symbol}
    out = set()
    for sec, _off, _t, si in elf.relocations(target=r".*"):
        if si in idx:
            out.add(sec)
    return out


def classify_failure(log: str, item, surface_log: str) -> tuple[str, str]:
    """(class, cause) for a build that did not produce a ROM.

    The four classes the relocation surface cannot close are named explicitly,
    because each needs different work and lumping them as `build-error` is what
    made the previous run's 169 failures unreadable.
    """
    markers = MARKER_RE.findall(log)
    for kind, message in markers:
        if item.c_file.stem in message or item.func in message:
            return "text-size-differs", kind
    if markers:
        return "text-size-differs", markers[0][0]

    undef = UNDEF_RE.findall(log)
    if undef:
        # Not a relocation-surface problem at all: a resident symbol the tree
        # does not define yet. Its own address is what the reference wants;
        # there is no addend to synthesize.
        resident = [m for m in undef if RESIDENT_RE.match(m)]
        if resident:
            return "build-error", "resident-symbol-missing (%s)" % \
                ", ".join(sorted(set(resident))[:3])
        # A symbol the synthesizer refused because the candidate's schedule
        # disagrees at the placeholder's own sites: either two sites demand
        # different addends, or the module's table corroborates the symbol but
        # not at any site this object still spells the same way. Either way no
        # consistent addend exists and the tool reports rather than invents.
        diverged = {m for m in undef
                    if re.search(r"\b%s\b: (\d+ distinct values|no corroborated"
                                 r" site)" % re.escape(m), surface_log)}
        if diverged:
            return "build-error", "schedule-divergence-at-site (%s)" % \
                ", ".join(sorted(diverged)[:3])
        unmapped = {m for m in undef
                    if re.search(r"\b%s\b: unmapped site" % re.escape(m),
                                 surface_log)}
        if unmapped:
            return "build-error", "unmapped-site (%s)" % \
                ", ".join(sorted(unmapped)[:3])
        aliased = [m for m in undef if rs.GEN_NAME_RE.match(m)]
        if aliased:
            return "build-error", "alias-coupling (%s)" % \
                ", ".join(sorted(aliased)[:3])
        nontext = [m for m in undef
                   if undefined_sites(item.c_file, m) - {".text"}]
        if nontext:
            return "build-error", "non-text-site (%s)" % \
                ", ".join(sorted(nontext)[:3])
        return "build-error", "unresolved-placeholder (%s)" % \
            ", ".join(sorted(set(undef))[:3])

    trunc = TRUNC_RE.findall(log)
    if trunc:
        return "build-error", "relocation-truncated (%s)" % \
            ", ".join(sorted(set(trunc))[:3])
    if re.search(r"\.c\.o\] Error|cfe: Error|\bError:", log):
        return "build-error", "compile-error"
    return "build-error", "other"


def rom_diff_offsets() -> list[int]:
    a = ROM.read_bytes()
    b = BASEROM.read_bytes()
    if len(a) != len(b):
        return [-1]
    return [i for i, (x, y) in enumerate(zip(a, b)) if x != y]


def run_trial(item: pb.QueueItem, rng: Optional[tuple[int, int]], jobs: int) -> Trial:
    t = Trial(item.func, item.rel_c_file, item.overlay, rng[0] if rng else None, rng[1] if rng else None)
    start = time.monotonic()
    original = splice(item)
    if original is None:
        t.klass, t.error = "unknown", "could not locate the NON_MATCHING block"
        return t
    surface = LINK_SYMS.read_text()
    surface_log = ""
    try:
        ok, log = build(jobs)
        if item.overlay is not None:
            # Compile is done either way; regenerate the surface against the
            # objects that now exist and relink. A candidate that already
            # linked is unaffected when the surface does not change.
            _sok, surface_log = synthesize_surface()
            ok, log2 = build(jobs)
            log = log + log2
        if not ok:
            t.klass, t.cause = classify_failure(log, item, surface_log)
            t.error = log[-1500:]
        else:
            diffs = rom_diff_offsets()
            if diffs == [-1]:
                # A skipped normalization can leave a section long enough to
                # move the image's own size; that is the size report, not a
                # separate failure.
                marker = MARKER_RE.findall(log)
                if marker:
                    t.klass, t.cause = "text-size-differs", marker[0][0]
                    t.error = "ROM size differs"
                else:
                    t.klass, t.error, t.cause = ("build-error",
                                                 "ROM size differs", "rom-size")
            elif not diffs:
                t.klass = "exact"
            else:
                inside = [d for d in diffs if rng and rng[0] <= d < rng[1]]
                outside = [d for d in diffs if not (rng and rng[0] <= d < rng[1])]
                t.in_range_words = len({d & ~3 for d in inside})
                t.out_of_range_bytes = len(outside)
                t.first_in_range = inside[0] if inside else None
                t.first_out_of_range = outside[0] if outside else None
                t.klass = "text-differs" if inside else "text-exact"
                marker = MARKER_RE.findall(log)
                if marker:
                    # A skipped normalization means the linked bytes are not a
                    # real build; the size report is the honest result.
                    t.klass, t.cause = "text-size-differs", marker[0][0]
    finally:
        item.c_file.write_text(original)
        if LINK_SYMS.read_text() != surface:
            LINK_SYMS.write_text(surface)
        t.seconds = time.monotonic() - start
    return t


def main(argv: list[str]) -> int:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--overlay", type=int)
    p.add_argument("--function")
    p.add_argument("--limit", type=int)
    p.add_argument("--resume", action="store_true")
    p.add_argument("--overlays-only", action="store_true")
    p.add_argument("--jobs", type=int, default=6)
    args = p.parse_args(argv)

    if not BASEROM.is_file():
        print("baserom missing", file=sys.stderr)
        return 2
    queue = pb.discover_queue()
    if args.overlay is not None:
        queue = [q for q in queue if q.overlay == args.overlay]
    if args.overlays_only:
        queue = [q for q in queue if q.overlay is not None]
    if args.function:
        queue = [q for q in queue if q.func == args.function]
    results: list[Trial] = []
    if args.resume and OUT_JSON.is_file():
        known = {f.name for f in dataclasses.fields(Trial)}
        for row in json.loads(OUT_JSON.read_text()).get("results", []):
            results.append(Trial(**{k: v for k, v in row.items() if k in known}))
        done = {r.func for r in results if r.klass != "build-error"}
        queue = [q for q in queue if q.func not in done]
    if args.limit:
        queue = queue[: args.limit]

    ov = overlay_ranges()
    res = resident_ranges()
    print(f"trialling {len(queue)} candidate(s)")
    for i, item in enumerate(queue, 1):
        rng = ov.get(item.source or "") if item.overlay is not None else res.get(item.func)
        if rng is None and item.overlay is not None:
            # atlas rows are keyed by the TU's source stem; fall back to any row
            # whose stem ends with the function's own file
            stem = str(item.c_file.relative_to(ROOT / "src")).removesuffix(".c")
            rng = ov.get(stem)
        t = run_trial(item, rng, args.jobs)
        results.append(t)
        print(f"[{i}/{len(queue)}] {t.func:34} {t.klass:16} in={t.in_range_words:<4} out={t.out_of_range_bytes:<5} {t.cause or '':40} ({t.seconds:.0f}s)", flush=True)
        write(results)
    write(results)
    subprocess.run(["gmake", f"-j{args.jobs}"], cwd=ROOT, capture_output=True)
    return 0


def write(results: list[Trial]) -> None:
    # One row per function, last write wins: a --resume run re-trials the
    # errored rows and must replace them, not append a second verdict.
    keyed = {}
    for r in results:
        keyed[r.func] = r
    results = list(keyed.values())
    OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps({"generated_by": "tools/promotion_trial.py",
                                    "results": [dataclasses.asdict(r) for r in results]}, indent=2) + "\n")
    lines = [f"{'function':34} {'class':16} {'in':>4} {'out':>6} {'ov':>4}  cause"]
    for r in results:
        lines.append(f"{r.func:34} {r.klass:16} {r.in_range_words:>4} {r.out_of_range_bytes:>6} {str(r.overlay):>4}  {r.cause or ''}")
    counts: dict[str, int] = {}
    for r in results:
        counts[r.klass] = counts.get(r.klass, 0) + 1
    causes: dict[str, int] = {}
    for r in results:
        if r.cause:
            key = re.sub(r" \(.*", "", r.cause)
            causes[key] = causes.get(key, 0) + 1
    lines.append("")
    lines.append(" ".join(f"{k}={v}" for k, v in sorted(counts.items())))
    lines.append(" ".join(f"{k}={v}" for k, v in sorted(causes.items())))
    OUT_TXT.write_text("\n".join(lines) + "\n")


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
