#!/usr/bin/env python3
"""Ranks every `#ifdef NON_MATCHING` function by how close its candidate C
is to the ROM target, so a fleet of workers can attack the near-misses
first instead of triaging ~300 KB of queued functions by hand.

Two independent sources feed the ranking:

1. A per-function isolated compile-and-diff, the primary and always-on
   source. For each function in tools/permute_batch.py's queue
   (discover_queue(): every `#ifdef NON_MATCHING` block, atlas-backed or
   source-scanned), this reuses that module's own machinery -- the same
   settings.toml + asm-processor + IDO invocation the permuter itself runs
   -- to produce two small, function-scoped objects: target.o (the
   function's own asm/nonmatchings/**/<f>.s, assembled directly -- exactly
   the ROM's bytes, splat's own disassembly of them) and base.o (the
   `#ifdef NON_MATCHING` C body, compiled with this project's real flags
   for that translation unit). Both are single-function objects, so a
   straight word-for-word diff over their .text needs no linking, no
   symbol resolution, and no whole-TU rebuild -- which matters because a
   whole-tree `gmake NON_MATCHING=1` build fails outright on any
   POSTPROCESS-trimmed object whose queued function grew past the trimmed
   (matched-size) target (see docs/nm-ranking.md's "Two build paths").

2. objdiff-cli's per-function fuzzy_match_percent, read from a report
   generated over a real `gmake NON_MATCHING=1` build tree
   (build_non_matching/) diffed against the verified `expected/build/`
   snapshot -- supplementary context (it reflects the function in its real
   linked/whole-TU context, not isolated), and only available for the
   objects objdiff-cli's ELF reader can parse and that the NON_MATCHING
   build actually produced. Pass its report with --objdiff-report; omit it
   and every function's objdiff_match_pct is simply null.

No instruction words, mnemonics, or hex ever leave this tool: every
decoded instruction word lives only in memory for one comparison and is
reduced to a count (differing_words) or an offset (first_mismatch_offset)
before anything is written to config/nonmatching-ranking.us.json or
printed.

Usage:

    # after building both trees (see docs/nm-ranking.md):
    .venv/bin/python tools/nm_ranking.py \\
        --objdiff-report /path/to/objdiff-report.json

    # top 20 as a markdown table, for pasting into a fleet prompt:
    .venv/bin/python tools/nm_ranking.py --top 20 --markdown
"""
from __future__ import annotations

import argparse
import concurrent.futures
import dataclasses
import json
import pathlib
import re
import struct
import subprocess
import sys
from collections import Counter
from typing import Optional

ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import permute_batch as pb  # noqa: E402

OBJDUMP = ROOT / "tools" / "binutils" / "mips64-elf-objdump"
OBJCOPY = ROOT / "tools" / "binutils" / "mips64-elf-objcopy"
WORK_DIR = ROOT / "build" / "nm_ranking"
DEFAULT_OUT = ROOT / "config" / "nonmatching-ranking.us.json"

CATEGORY_RANK = {
    "register-only": 0,
    "schedule-only": 1,
    "other": 2,
    "reloc-mismatch": 3,
    "size-mismatch": 4,
}

SYM_RE = re.compile(
    r"^[0-9a-f]{8}\s+\S+\s+\S+\s+(?P<sec>\S+)\s+(?P<size>[0-9a-f]{8})\s+(?P<name>\S+)\s*$"
)
RELOC_RE = re.compile(
    r"^([0-9a-f]{8})\s+(\S+)\s+(\S+)\s*$"
)


def objdump_text(objfile: pathlib.Path) -> str:
    return subprocess.run(
        [str(OBJDUMP), "-t", str(objfile)], capture_output=True, text=True, check=True
    ).stdout


def func_symbol_size(objfile: pathlib.Path, func: str) -> Optional[int]:
    """Size (bytes) of `func`'s .text symbol in objfile, per objdump -t."""
    for line in objdump_text(objfile).splitlines():
        m = SYM_RE.match(line)
        if m and m.group("name") == func and m.group("sec") == ".text":
            return int(m.group("size"), 16)
    return None


def text_bytes(objfile: pathlib.Path, size: int) -> bytes:
    """Raw .text section bytes, truncated to `size` (the function's own
    length -- both target.o and base.o here hold exactly one function, so
    the section may carry compiler alignment padding past it)."""
    tmp = objfile.with_suffix(".text.bin")
    subprocess.run(
        [str(OBJCOPY), "-O", "binary", "--only-section=.text", str(objfile), str(tmp)],
        capture_output=True, check=True,
    )
    raw = tmp.read_bytes()
    tmp.unlink(missing_ok=True)
    return raw[:size]


def relocations(objfile: pathlib.Path) -> dict[int, tuple[str, str]]:
    """offset -> (type, value) for every relocation in .text."""
    out = subprocess.run(
        [str(OBJDUMP), "-r", str(objfile)], capture_output=True, text=True, check=True
    ).stdout
    result: dict[int, tuple[str, str]] = {}
    in_text = False
    for line in out.splitlines():
        if line.startswith("RELOCATION RECORDS FOR [.text]"):
            in_text = True
            continue
        if line.startswith("RELOCATION RECORDS FOR"):
            in_text = False
            continue
        if not in_text:
            continue
        m = RELOC_RE.match(line.strip())
        if m:
            offset, rtype, value = m.groups()
            result[int(offset, 16)] = (rtype, value)
    return result


def words_of(data: bytes) -> list[int]:
    n = len(data) - (len(data) % 4)
    return [w for (w,) in struct.iter_unpack(">I", data[:n])]


def instr_reg_mask(word: int) -> int:
    """Zero out the register-select fields of one big-endian MIPS word,
    leaving the opcode/function/immediate bits that decide *what* the
    instruction does rather than *which registers* it names. R-type (and
    the COP register-format instructions, which share the same field
    layout) zero rs/rt/rd (bits 25-11); other I-type instructions zero
    rs/rt (bits 25-16) and keep the 16-bit immediate/branch-offset, since
    that field is not a register selector. J-type (j/jal) has no register
    fields at all, so it is left untouched -- a differing J-type word is
    always either a relocation or a genuine target difference, never a
    register swap."""
    op = (word >> 26) & 0x3F
    if op in (0x02, 0x03):  # j, jal
        return word
    if op == 0x00:  # SPECIAL (R-type)
        return word & 0xFC00003F
    return word & 0xFC00FFFF


@dataclasses.dataclass
class FuncResult:
    name: str
    file: str
    overlay: Optional[int]
    tu: str
    size_bytes: int
    differing_words: int
    first_mismatch_offset: Optional[int]
    size_delta: int
    category: str
    objdiff_match_pct: Optional[float] = None


def tu_category(rel_c_file: str) -> str:
    parts = pathlib.PurePosixPath(rel_c_file).parts
    # src/overlays/oNNN/... -> overlays/oNNN ; src/main/... -> main ; src/libultra/... -> libultra
    if len(parts) >= 3 and parts[0] == "src" and parts[1] == "overlays":
        return f"overlays/{parts[2]}"
    if len(parts) >= 2 and parts[0] == "src":
        return parts[1]
    return "?"


def classify(
    base_size: int,
    target_size: int,
    base_words: list[int],
    target_words: list[int],
    base_reloc: dict[int, tuple[str, str]],
    target_reloc: dict[int, tuple[str, str]],
) -> tuple[str, int, Optional[int]]:
    """Returns (category, differing_words, first_mismatch_offset)."""
    size_delta = base_size - target_size
    n = min(len(base_words), len(target_words))
    diff_positions = [i for i in range(n) if base_words[i] != target_words[i]]
    # Words beyond the shorter side's length are unmatched by construction.
    extra = abs(len(base_words) - len(target_words))
    differing_words = len(diff_positions) + extra
    first_mismatch_offset: Optional[int]
    if diff_positions:
        first_mismatch_offset = diff_positions[0] * 4
    elif extra:
        first_mismatch_offset = n * 4
    else:
        first_mismatch_offset = None

    if size_delta != 0:
        return "size-mismatch", differing_words, first_mismatch_offset

    if not diff_positions:
        return "other", 0, None

    if Counter(base_words) == Counter(target_words):
        return "schedule-only", differing_words, first_mismatch_offset

    if all(
        instr_reg_mask(base_words[i]) == instr_reg_mask(target_words[i])
        for i in diff_positions
    ):
        return "register-only", differing_words, first_mismatch_offset

    def explained_by_reloc(i: int) -> bool:
        off = i * 4
        b, t = base_reloc.get(off), target_reloc.get(off)
        return b is not None or t is not None

    if all(explained_by_reloc(i) for i in diff_positions):
        return "reloc-mismatch", differing_words, first_mismatch_offset

    return "other", differing_words, first_mismatch_offset


def process_item(item: "pb.QueueItem") -> tuple[Optional[FuncResult], Optional[str]]:
    safe = re.sub(r"[^A-Za-z0-9_.-]", "_", item.func)
    out_dir = WORK_DIR / safe
    out_dir.mkdir(parents=True, exist_ok=True)
    settings_path = out_dir / "settings.toml"
    flags = pb.flag_group_for(item.c_file)
    pb.write_settings_toml(settings_path, flags)
    try:
        target_asm = pb.prepare_target_asm(item, out_dir)
        scratch = pb.run_import(item, out_dir, settings_path, target_asm)
    except Exception as e:  # noqa: BLE001 - reported per-function, not fatal
        return None, f"{item.func} ({item.rel_c_file}): {e}"

    base_o = scratch / "base.o"
    target_o = scratch / "target.o"
    if not base_o.is_file() or not target_o.is_file():
        return None, f"{item.func} ({item.rel_c_file}): import.py did not produce base.o/target.o"

    target_size = func_symbol_size(target_o, item.func)
    base_size = func_symbol_size(base_o, item.func)
    if target_size is None or base_size is None:
        return None, f"{item.func} ({item.rel_c_file}): couldn't find .text symbol in base.o/target.o"

    base_words = words_of(text_bytes(base_o, base_size))
    target_words = words_of(text_bytes(target_o, target_size))
    base_reloc = relocations(base_o)
    target_reloc = relocations(target_o)

    category, differing_words, first_mismatch_offset = classify(
        base_size, target_size, base_words, target_words, base_reloc, target_reloc
    )

    result = FuncResult(
        name=item.func,
        file=item.rel_c_file,
        overlay=item.overlay,
        tu=tu_category(item.rel_c_file),
        size_bytes=target_size,
        differing_words=differing_words,
        first_mismatch_offset=first_mismatch_offset,
        size_delta=base_size - target_size,
        category=category,
    )
    return result, None


def load_objdiff_pct(report_path: Optional[pathlib.Path]) -> dict[str, float]:
    if report_path is None:
        return {}
    data = json.loads(report_path.read_text())
    out: dict[str, float] = {}
    for unit in data.get("units", []):
        for fn in unit.get("functions", []):
            name = fn.get("name")
            pct = fn.get("fuzzy_match_percent")
            if name is not None and pct is not None:
                # A deduplicated/weak symbol can appear in more than one
                # unit; keep the lowest score seen, the more conservative
                # (worse-case) one for triage purposes.
                if name not in out or pct < out[name]:
                    out[name] = pct
    return out


def sort_key(r: FuncResult):
    return (CATEGORY_RANK.get(r.category, 99), r.differing_words)


def print_table(results: list[FuncResult], top: Optional[int], markdown: bool) -> None:
    rows = sorted(results, key=sort_key)
    if top is not None:
        rows = rows[:top]
    headers = [
        "name", "overlay/TU", "size", "objdiff%", "diff_words",
        "first_mismatch", "size_delta", "category",
    ]

    def fmt_row(r: FuncResult) -> list[str]:
        loc = f"o{r.overlay:03d}" if r.overlay is not None else r.tu
        pct = f"{r.objdiff_match_pct:.1f}" if r.objdiff_match_pct is not None else "-"
        fmo = str(r.first_mismatch_offset) if r.first_mismatch_offset is not None else "-"
        return [
            r.name, loc, str(r.size_bytes), pct, str(r.differing_words),
            fmo, str(r.size_delta), r.category,
        ]

    data_rows = [fmt_row(r) for r in rows]

    if markdown:
        print("| " + " | ".join(headers) + " |")
        print("|" + "|".join("---" for _ in headers) + "|")
        for row in data_rows:
            print("| " + " | ".join(row) + " |")
        return

    widths = [len(h) for h in headers]
    for row in data_rows:
        for i, cell in enumerate(row):
            widths[i] = max(widths[i], len(cell))
    def line(cells: list[str]) -> str:
        return "  ".join(c.ljust(widths[i]) for i, c in enumerate(cells))
    print(line(headers))
    print(line(["-" * w for w in widths]))
    for row in data_rows:
        print(line(row))


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--objdiff-report", type=pathlib.Path, default=None,
                     help="objdiff-cli 'report generate' JSON, run against build_non_matching/ "
                          "vs expected/build/ (see docs/nm-ranking.md). Optional: supplies "
                          "objdiff_match_pct where available.")
    ap.add_argument("--jobs", type=int, default=8,
                     help="parallel isolated compiles (default 8)")
    ap.add_argument("--out", type=pathlib.Path, default=DEFAULT_OUT,
                     help=f"where to write the ranking JSON (default {DEFAULT_OUT})")
    ap.add_argument("--top", type=int, default=None, help="only print the top N rows")
    ap.add_argument("--markdown", action="store_true", help="print the table as markdown")
    ap.add_argument("--no-table", action="store_true", help="skip printing the table")
    ap.add_argument("--limit", type=int, default=None,
                     help="only process the first N queue items (debugging)")
    args = ap.parse_args()

    WORK_DIR.mkdir(parents=True, exist_ok=True)

    queue = pb.discover_queue()
    if args.limit:
        queue = queue[: args.limit]

    results: list[FuncResult] = []
    errors: list[str] = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for result, error in pool.map(process_item, queue):
            if result is not None:
                results.append(result)
            if error is not None:
                errors.append(error)

    pct_by_name = load_objdiff_pct(args.objdiff_report)
    for r in results:
        r.objdiff_match_pct = pct_by_name.get(r.name)

    results.sort(key=sort_key)

    with_pct = sum(1 for r in results if r.objdiff_match_pct is not None)
    out_doc = {
        "queue_size": len(queue),
        "resolved": len(results),
        "unresolved": len(errors),
        # Not the report's path (machine-local, not portable/reproducible
        # content): just whether one was supplied and how many resolved
        # functions it actually covered.
        "objdiff_report_used": args.objdiff_report is not None,
        "objdiff_match_pct_coverage": with_pct,
        "functions": [
            {
                "name": r.name,
                "file": r.file,
                "overlay": r.overlay,
                "tu": r.tu,
                "size_bytes": r.size_bytes,
                "objdiff_match_pct": r.objdiff_match_pct,
                "differing_words": r.differing_words,
                "first_mismatch_offset": r.first_mismatch_offset,
                "size_delta": r.size_delta,
                "category": r.category,
            }
            for r in results
        ],
        "unresolved_functions": errors,
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(out_doc, indent=2) + "\n")

    print(f"{len(results)}/{len(queue)} queued functions resolved "
          f"({len(errors)} could not be isolated-compiled)", file=sys.stderr)
    counts = Counter(r.category for r in results)
    print("category counts: " + ", ".join(f"{k}={v}" for k, v in sorted(counts.items())),
          file=sys.stderr)

    if not args.no_table:
        print_table(results, args.top, args.markdown)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
