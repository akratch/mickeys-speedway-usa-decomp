#!/usr/bin/env python3
"""
flag_sweep.py -- compile one translation unit under the whole lattice of
compiler flag groups this project has ever needed, and rank the results by
closeness to the target bytes. Run this BEFORE any hand permutation.

Usage:
    tools/flag_sweep.py <tu.c> --function NAME [options]

Where NAME is the symbol as it appears in the *compiled candidate* object
(the C name, e.g. "ProcessRelocationEntry"). If the ROM/asm side uses a
different name -- true for any not-yet-matched function, which splat names
func_<VRAM> until it is matched -- pass that name separately with
--target-symbol.

Target bytes, in order tried (first that resolves wins; --target-asm forces
the first branch):
  1. --target-asm PATH            : assemble this .s file directly.
  2. asm/nonmatchings/**/<target-symbol>.s : the function is still
     `#pragma GLOBAL_ASM`, or under `#ifdef NON_MATCHING`, and splat still
     emits its disassembly. Assembled exactly as tools/wb_compare.sh does.
  3. <target-symbol> in build/mickey.us.elf : the function is already
     matched. Bytes come from the baserom at the symbol's ROM offset (its
     size preferring symbol_addrs.us.txt, same fallback wb_compare.sh uses).

Every candidate is an *unlinked* object, so words carrying a relocation
(a `jal`, a `%hi`/`%lo` pair, ...) do not contain final addresses and cannot
be compared literally. Both sides are masked at those words before scoring;
see score_words() and docs/flag-sweep.md for exactly what that does and does
not prove.

Nothing here writes ROM-derived bytes into any tracked file: everything
happens under build/flag_sweep/ (gitignored via the tree's blanket `build/`
rule) and instruction words never reach this file, argv, or stdout as text
-- only counts and offsets do.
"""
from __future__ import annotations

import argparse
import concurrent.futures
import dataclasses
import os
import re
import struct
import subprocess
import sys
import time
from pathlib import Path
from typing import Dict, List, Optional, Sequence, Tuple

REPO_ROOT = Path(__file__).resolve().parent.parent
TOOLS_DIR = REPO_ROOT / "tools"
BINUTILS = TOOLS_DIR / "binutils"
AS = BINUTILS / "mips64-elf-as"
OBJDUMP = BINUTILS / "mips64-elf-objdump"
OBJCOPY = BINUTILS / "mips64-elf-objcopy"
IDO_CC = TOOLS_DIR / "ido" / "cc"
IDO_PHASES = TOOLS_DIR / "ido-phases.py"
ASM_PROCESSOR_BUILD = TOOLS_DIR / "asm-processor" / "build.py"
OBJDIFF_CLI = TOOLS_DIR / "objdiff" / "objdiff-cli"


def repo_cli_path(path: Path) -> Path:
    """Resolve a CLI path against the repository root, not the caller's CWD."""
    if path.is_absolute():
        return path.resolve()
    return (REPO_ROOT / path).resolve()


def display_path(path: Path) -> str:
    """Prefer a repository-relative path, while allowing external inputs."""
    try:
        return str(path.relative_to(REPO_ROOT))
    except ValueError:
        return str(path)

# The project's default C flags -- Makefile ~75-105. Kept in sync by hand;
# if the Makefile's CFLAGS/ASFLAGS/DEFINES/INCLUDE_CFLAGS lines change, this
# block needs the same edit. There is no included-Makefile trick available
# here because the compile step drives asm-processor/IDO directly, the same
# way the Makefile's own %.c.o rule does, not through `make`.
DEFINES = [
    "-D_LANGUAGE_C",
    "-D_FINALROM",
    "-DTARGET_N64",
    "-DVERSION_us",
    "-D_MIPS_SZLONG=32",
]
INCLUDE_CFLAGS = [
    "-I",
    ".",
    "-I",
    "include",
    "-I",
    "include/libc",
    "-I",
    "include/PR",
    "-I",
    "assets",
]
BASE_CFLAGS = (
    ["-non_shared", "-G", "0", "-Xcpluscomm", "-fullwarn", "-woff", "649,838", "-nostdinc"]
    + DEFINES
    + INCLUDE_CFLAGS
)
ASFLAGS = ["-march=vr4300", "-32", "-mabi=32", "-G0", "-I", "include"]
ASM_PROC_ASFLAGS = ASFLAGS + ["include/asm_processor_prelude.inc"]

# ---------------------------------------------------------------------------
# The lattice. Extend by adding rows -- everything downstream (pruning,
# compiling, ranking) reads this table, nothing is hardcoded per-combo.
# ---------------------------------------------------------------------------

# (name, OPT_FLAGS tokens)
OPT_GROUP: List[Tuple[str, List[str]]] = [
    ("bare", []),
    ("-O0", ["-O0"]),
    ("-O1", ["-O1"]),
    ("-O2", ["-O2"]),
    ("-O3", ["-O3"]),
    ("-g", ["-g"]),
    ("-g3", ["-g3"]),
    ("-O2-g3", ["-O2", "-g3"]),
]

# (name, MIPSISET tokens)
ISA_GROUP: List[Tuple[str, List[str]]] = [
    ("mips1", ["-mips1", "-32"]),
    ("mips2", ["-mips2", "-32"]),
    ("mips3", ["-mips3", "-32"]),
]

# (name, extra CFLAGS tokens, requires_optimization)
# requires_optimization=True means: prune this extra when OPT_GROUP has no
# -O flag at all (bare/-g/-g3) -- a loop-unroll hint has nothing to act on
# without an optimizing pass, and no in-tree use pairs the two.
EXTRAS_GROUP: List[Tuple[str, List[str], bool]] = [
    ("none", [], False),
    ("r4300_mul", ["-Wab,-r4300_mul"], False),
    ("loopunroll0", ["-Wo,-loopunroll,0"], True),
    ("loopunroll2", ["-Wo,-loopunroll,2"], True),
    ("loopunroll4", ["-Wo,-loopunroll,4"], True),
    ("woff835", ["-woff", "835"], False),
]

# Phase-driven variants: these go through tools/ido-phases.py (a drop-in
# `cc` replacement that can run individual IDO phases with per-phase
# overrides) instead of tools/ido/cc directly, and are not crossed against
# ISA_GROUP/EXTRAS_GROUP -- the project has exactly two documented uses of
# this family (Makefile ~515-597) and both pin their own ISA. Add rows here,
# not to the cross product, if a third phase-driven family shows up.
PHASE_VARIANTS = [
    {
        "id": "phase-uopt-O1",
        "opt": ["-O2"],
        "isa": ["-mips2", "-32"],
        "extra": ["-Xphase,uopt,+", "-Xphase,uopt,-O1"],
        "use_ido_phases": True,
        "note": "uopt at -O1, rest at -O2 (setglobalintmask.c's flag group)",
    },
    {
        "id": "phase-all-O3",
        "opt": ["-O2"],
        "isa": ["-mips2", "-32"],
        "extra": [
            "-Xphase,cfe,-O3",
            "-Xphase,uopt,-O3",
            "-Xphase,ugen,-O3",
            "-Xphase,as1,-O3",
        ],
        "use_ido_phases": True,
        "note": "all four IDO phases at -O3 (ll.c/ldiv.c's flag group)",
    },
]


@dataclasses.dataclass(frozen=True)
class Combo:
    id: str
    opt: Tuple[str, ...]
    isa: Tuple[str, ...]
    extra: Tuple[str, ...]
    use_ido_phases: bool = False
    note: str = ""


def build_lattice() -> List[Combo]:
    combos: List[Combo] = []
    for opt_name, opt_tokens in OPT_GROUP:
        # bare/-g/-g3 carry no -On token at all.
        has_opt = any(t in ("-O0", "-O1", "-O2", "-O3") for t in opt_tokens)
        for isa_name, isa_tokens in ISA_GROUP:
            for extra_name, extra_tokens, needs_opt in EXTRAS_GROUP:
                if needs_opt and not has_opt:
                    continue  # pruned: loop-unroll hint with no optimizer pass
                combos.append(
                    Combo(
                        id=f"{opt_name}_{isa_name}_{extra_name}",
                        opt=tuple(opt_tokens),
                        isa=tuple(isa_tokens),
                        extra=tuple(extra_tokens),
                    )
                )
    for pv in PHASE_VARIANTS:
        combos.append(
            Combo(
                id=pv["id"],
                opt=tuple(pv["opt"]),
                isa=tuple(pv["isa"]),
                extra=tuple(pv["extra"]),
                use_ido_phases=True,
                note=pv["note"],
            )
        )
    return combos


# ---------------------------------------------------------------------------
# Compiling
# ---------------------------------------------------------------------------


@dataclasses.dataclass
class CompileResult:
    combo: Combo
    ok: bool
    obj_path: Optional[Path]
    error: str
    elapsed: float


def compile_combo(
    tu: Path, combo: Combo, outdir: Path, defines: Sequence[str]
) -> CompileResult:
    outdir.mkdir(parents=True, exist_ok=True)
    obj_path = outdir / "out.o"
    log_path = outdir / "compile.log"

    cc_tokens = (
        [sys.executable, str(IDO_PHASES)] if combo.use_ido_phases else [str(IDO_CC)]
    )
    cmd = (
        [sys.executable, str(ASM_PROCESSOR_BUILD)]
        + cc_tokens
        + ["--", str(AS)]
        + ASM_PROC_ASFLAGS
        + ["--", "-c"]
        + BASE_CFLAGS
        + [f"-D{d}" for d in defines]
        + list(combo.opt)
        + list(combo.isa)
        + list(combo.extra)
        + ["-o", str(obj_path), str(tu)]
    )

    start = time.monotonic()
    try:
        proc = subprocess.run(
            cmd,
            cwd=REPO_ROOT,
            capture_output=True,
            text=True,
            timeout=120,
        )
    except subprocess.TimeoutExpired:
        return CompileResult(combo, False, None, "timed out", time.monotonic() - start)
    elapsed = time.monotonic() - start

    log_path.write_text((proc.stdout or "") + (proc.stderr or ""))

    if proc.returncode != 0 or not obj_path.exists():
        # Keep only the last couple of lines -- IDO errors/warnings are project
        # source, not ROM content, so this is fine to surface, but there is no
        # reason to spray the whole log into the ranked table.
        tail = "\n".join((proc.stderr or proc.stdout or "").strip().splitlines()[-3:])
        return CompileResult(combo, False, None, tail or "compile failed", elapsed)

    return CompileResult(combo, True, obj_path, "", elapsed)


# ---------------------------------------------------------------------------
# Target/candidate byte extraction
# ---------------------------------------------------------------------------

RELOC_MASK = {
    "R_MIPS_26": 0x03FFFFFF,
    "R_MIPS_HI16": 0x0000FFFF,
    "R_MIPS_LO16": 0x0000FFFF,
    "R_MIPS_GPREL16": 0x0000FFFF,
    "R_MIPS_GOT16": 0x0000FFFF,
    "R_MIPS_CALL16": 0x0000FFFF,
    "R_MIPS_16": 0x0000FFFF,
    "R_MIPS_32": 0xFFFFFFFF,
}
DEFAULT_RELOC_MASK = 0x0000FFFF


def _run(cmd: Sequence[str]) -> str:
    return subprocess.run(
        list(map(str, cmd)), cwd=REPO_ROOT, capture_output=True, text=True, check=True
    ).stdout


def elf_symbol(obj_path: Path, name: str) -> Optional[Tuple[int, int, str]]:
    """(offset_or_vram, size, section) for an exact symbol name, or None."""
    out = _run([OBJDUMP, "-t", obj_path])
    for line in out.splitlines():
        parts = line.split()
        if len(parts) < 5 or parts[-1] != name:
            continue
        try:
            size = int(parts[-2], 16)
            addr = int(parts[0], 16)
        except ValueError:
            continue
        section = parts[-3]
        return addr, size, section
    return None


def dump_section(obj_path: Path, section: str, out_bin: Path) -> bytes:
    subprocess.run(
        [str(OBJCOPY), "-O", "binary", f"--only-section={section}", str(obj_path), str(out_bin)],
        cwd=REPO_ROOT,
        check=True,
        capture_output=True,
    )
    if not out_bin.exists():
        return b""
    return out_bin.read_bytes()


def words_from_bytes(data: bytes) -> List[int]:
    n = len(data) // 4
    if n == 0:
        return []
    return list(struct.unpack(">%dI" % n, data[: n * 4]))


def section_relocs(obj_path: Path, section: str) -> Dict[int, int]:
    """word-index (relative to `section`'s start) -> bits to mask before compare."""
    out = _run([OBJDUMP, "-r", obj_path])
    cur = None
    result: Dict[int, int] = {}
    for line in out.splitlines():
        m = re.match(r"RELOCATION RECORDS FOR \[(.+)\]:", line)
        if m:
            cur = m.group(1)
            continue
        if cur != section:
            continue
        parts = line.split()
        if len(parts) < 2:
            continue
        try:
            off = int(parts[0], 16)
        except ValueError:
            continue
        rtype = parts[1]
        if not rtype.startswith("R_MIPS"):
            continue
        result[off // 4] = RELOC_MASK.get(rtype, DEFAULT_RELOC_MASK)
    return result


def get_candidate(obj_path: Path, function: str, workdir: Path) -> Tuple[List[int], Dict[int, int], str]:
    """candidate words, function-local reloc mask, section name."""
    sym = elf_symbol(obj_path, function)
    if sym is None:
        raise LookupError(f"symbol {function!r} not found in {obj_path}")
    addr, size, section = sym
    if size == 0:
        raise LookupError(f"symbol {function!r} in {obj_path} has size 0 (no -O0 frame?)")
    raw = dump_section(obj_path, section, workdir / "cand_section.bin")
    func_bytes = raw[addr : addr + size]
    words = words_from_bytes(func_bytes)
    relocs = section_relocs(obj_path, section)
    start_word = addr // 4
    end_word = start_word + len(words)
    local = {k - start_word: v for k, v in relocs.items() if start_word <= k < end_word}
    return words, local, section


ASM_TARGET_HEADER = '.set noat\n.set noreorder\n.include "macro.inc"\n.section .text, "ax"\n'


def find_nonmatching_asm(target_symbol: str) -> Optional[Path]:
    for p in (REPO_ROOT / "asm" / "nonmatchings").rglob(f"{target_symbol}.s"):
        return p
    return None


def assemble_target_asm(asm_path: Path, workdir: Path) -> Tuple[List[int], Dict[int, int]]:
    src = workdir / "target.s"
    src.write_text(ASM_TARGET_HEADER + asm_path.read_text())
    obj = workdir / "target.o"
    subprocess.run(
        [str(AS)] + ASFLAGS + ["-o", str(obj), str(src)],
        cwd=REPO_ROOT,
        check=True,
        capture_output=True,
    )
    raw = dump_section(obj, ".text", workdir / "target_section.bin")
    words = words_from_bytes(raw)
    relocs = section_relocs(obj, ".text")
    return words, relocs


SYMBOL_ADDRS = REPO_ROOT / "symbol_addrs.us.txt"


def symbol_addrs_size(name: str) -> Optional[int]:
    pat = re.compile(
        r"^%s *= *0x[0-9A-Fa-f]+ *;.*size:0x([0-9A-Fa-f]+)" % re.escape(name), re.MULTILINE
    )
    m = pat.search(SYMBOL_ADDRS.read_text())
    return int(m.group(1), 16) if m else None


def section_vma_lma(elf_path: Path, section: str) -> Tuple[int, int]:
    """(VMA, LMA) of a section, from `objdump -h`.

    LMA is where the loader/ROM puts a section's bytes; VMA is where the
    program sees them at runtime. They agree for resident code (the whole
    ROM is memory-mapped 1:1 modulo a fixed offset) and *disagree* for
    overlays, which splat gives a synthetic shared VMA (0xF0000000 and up,
    since every overlay's code loads at the same runtime address) while LMA
    still names each overlay's own distinct ROM location. Deriving the ROM
    offset from LMA-VMA per section, rather than assuming the resident
    segment's fixed 0x7FFFF400, is what makes ELF-range mode work for
    overlay functions too.
    """
    out = _run([OBJDUMP, "-h", elf_path])
    for line in out.splitlines():
        parts = line.split()
        if len(parts) >= 6 and parts[1] == section:
            return int(parts[3], 16), int(parts[4], 16)
    raise LookupError(f"section {section!r} not found in {elf_path}")


def rom_target_bytes(target_symbol: str, elf_path: Path) -> Tuple[List[int], int]:
    """(target words, vram) read straight from the baserom, ELF-range mode."""
    sym = elf_symbol(elf_path, target_symbol)
    if sym is None:
        raise LookupError(f"{target_symbol!r} not found in {elf_path}")
    vram, elf_size, section = sym
    size = symbol_addrs_size(target_symbol) or elf_size
    if size == 0:
        raise LookupError(f"{target_symbol!r} has size 0 in both the ELF and symbol_addrs")
    vma, lma = section_vma_lma(elf_path, section)
    rom_off = lma + (vram - vma)
    baserom = REPO_ROOT / "baseroms" / "mickey.us.z64"
    with open(baserom, "rb") as f:
        f.seek(rom_off)
        data = f.read(size)
    return words_from_bytes(data), vram


def resolve_target(
    target_symbol: str, target_asm: Optional[Path], workdir: Path, elf_path: Path
) -> Tuple[List[int], Dict[int, int], str]:
    """(target words, function-local reloc mask, mode description)."""
    if target_asm is not None:
        words, relocs = assemble_target_asm(target_asm, workdir)
        return words, relocs, f"asm:{display_path(target_asm)}"

    found = find_nonmatching_asm(target_symbol)
    if found is not None:
        words, relocs = assemble_target_asm(found, workdir)
        return words, relocs, f"asm:{found.relative_to(REPO_ROOT)}"

    if elf_path.exists():
        try:
            words, vram = rom_target_bytes(target_symbol, elf_path)
            return words, {}, f"rom:0x{vram:08X} (build/mickey.us.elf + baserom)"
        except LookupError:
            pass

    raise LookupError(
        f"could not resolve target bytes for {target_symbol!r}: no "
        f"asm/nonmatchings/**/{target_symbol}.s, and it is not a symbol in "
        f"{display_path(elf_path)}. "
        "Pass --target-asm explicitly."
    )


# ---------------------------------------------------------------------------
# Scoring -- pure function of word arrays, exercised directly by
# tests/test_flag_sweep.py with synthetic data, no compiler involved.
# ---------------------------------------------------------------------------


@dataclasses.dataclass(frozen=True)
class Score:
    exact: bool
    size_delta: int  # bytes; candidate - target
    diff_words: int  # words differing after masking, plus any length mismatch
    first_mismatch: Optional[int]  # byte offset of the first differing word, or None


def score_words(
    target: Sequence[int],
    candidate: Sequence[int],
    masks: Optional[Dict[int, int]] = None,
) -> Score:
    """
    Compare two word arrays word-by-word. `masks` maps a word index to the
    bits that must be ignored at that word (a relocation site: the bits a
    linker or assembler would still have to fill in, so an unlinked object's
    placeholder there is not a real disagreement).

    Extra words on either side beyond the shorter length each count as one
    more differing word, which is what lets a size-only regression still
    show up in `diff_words` even when every shared word matches.
    """
    masks = masks or {}
    tlen, clen = len(target), len(candidate)
    n = min(tlen, clen)
    diff = 0
    first: Optional[int] = None
    for i in range(n):
        m = masks.get(i, 0)
        tw = target[i] & ~m & 0xFFFFFFFF
        cw = candidate[i] & ~m & 0xFFFFFFFF
        if tw != cw:
            diff += 1
            if first is None:
                first = i * 4
    extra = abs(tlen - clen)
    diff += extra
    if extra and first is None:
        first = n * 4
    size_delta = (clen - tlen) * 4
    exact = size_delta == 0 and diff == 0
    return Score(exact, size_delta, diff, first)


def rank_key(score: Score):
    return (
        0 if score.exact else 1,
        score.diff_words,
        abs(score.size_delta),
        -(score.first_mismatch if score.first_mismatch is not None else 1 << 30),
    )


# ---------------------------------------------------------------------------
# objdiff (optional, best-effort -- tools/objdiff/objdiff-cli is not part of
# this lane and may not exist; never treat its absence as an error)
# ---------------------------------------------------------------------------


def objdiff_match_percent(target_obj: Path, candidate_obj: Path, function: str) -> Optional[float]:
    if not OBJDIFF_CLI.exists():
        return None
    try:
        out = subprocess.run(
            [str(OBJDIFF_CLI), "diff", "-1", str(target_obj), "-2", str(candidate_obj), "-o", function],
            cwd=REPO_ROOT,
            capture_output=True,
            text=True,
            timeout=30,
        ).stdout
    except Exception:
        return None
    m = re.search(r"(\d+(?:\.\d+)?)\s*%", out)
    return float(m.group(1)) if m else None


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------


def parse_args(argv: Sequence[str]) -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("tu", type=Path, help="C translation unit to sweep")
    p.add_argument("--function", required=True, help="symbol name in the compiled candidate object")
    p.add_argument(
        "--target-symbol",
        default=None,
        help="symbol/file name on the ROM/asm side, if different from --function "
        "(e.g. a not-yet-matched function's splat name func_80031A30)",
    )
    p.add_argument("--target-asm", type=Path, default=None, help="assemble this .s file as the target directly")
    p.add_argument(
        "--define",
        action="append",
        default=[],
        help="extra -D define, repeatable (e.g. --define NON_MATCHING)",
    )
    p.add_argument("--jobs", type=int, default=None, help="parallel compile workers (default: ncpu-2)")
    p.add_argument("--limit", type=int, default=None, help="only show the top N rows")
    p.add_argument("--keep", action="store_true", help="keep build/flag_sweep/ after the run")
    p.add_argument("--objdiff", action="store_true", help="also print objdiff-cli's match %% for the top row, if installed")
    p.add_argument(
        "--elf",
        type=Path,
        default=REPO_ROOT / "build" / "mickey.us.elf",
        help="linked ELF to use for ROM-range target resolution",
    )
    return p.parse_args(argv)


def main(argv: Sequence[str]) -> int:
    args = parse_args(argv)
    tu = repo_cli_path(args.tu)
    if not tu.exists():
        print(f"flag_sweep: no such file: {tu}", file=sys.stderr)
        return 2

    target_asm = repo_cli_path(args.target_asm) if args.target_asm is not None else None
    elf_path = repo_cli_path(args.elf)

    target_symbol = args.target_symbol or args.function

    text = tu.read_text()
    defines = list(args.define)
    if "#ifdef NON_MATCHING" in text and not any(d.split("=")[0] == "NON_MATCHING" for d in defines):
        defines.append("NON_MATCHING")
        print("flag_sweep: TU has #ifdef NON_MATCHING; auto-adding -DNON_MATCHING", file=sys.stderr)

    lattice = build_lattice()
    ncpu = os.cpu_count() or 4
    workers = args.jobs or max(1, ncpu - 2)

    tu_stem = tu.stem
    sweep_root = REPO_ROOT / "build" / "flag_sweep" / tu_stem
    sweep_root.mkdir(parents=True, exist_ok=True)

    print(f"flag_sweep: {len(lattice)} combos, {workers} workers, target={target_symbol!r}")
    t0 = time.monotonic()
    results: List[CompileResult] = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
        futs = {
            pool.submit(compile_combo, tu, combo, sweep_root / combo.id, defines): combo
            for combo in lattice
        }
        for fut in concurrent.futures.as_completed(futs):
            results.append(fut.result())
    compile_elapsed = time.monotonic() - t0

    # Resolve the target once, in one scratch dir shared by all rows.
    target_dir = sweep_root / "_target"
    target_dir.mkdir(parents=True, exist_ok=True)
    try:
        target_words, target_relocs, target_mode = resolve_target(
            target_symbol, target_asm, target_dir, elf_path
        )
    except LookupError as e:
        print(f"flag_sweep: {e}", file=sys.stderr)
        return 2
    print(f"flag_sweep: target = {target_mode}, {len(target_words)} words")

    rows = []
    for r in results:
        if not r.ok:
            rows.append((r.combo, None, r.error, r.elapsed))
            continue
        try:
            cand_words, cand_relocs, _section = get_candidate(r.obj_path, args.function, r.obj_path.parent)
        except LookupError as e:
            rows.append((r.combo, None, str(e), r.elapsed))
            continue
        masks = dict(target_relocs)
        for k, v in cand_relocs.items():
            masks[k] = masks.get(k, 0) | v
        sc = score_words(target_words, cand_words, masks)
        rows.append((r.combo, sc, "", r.elapsed))

    scored = [row for row in rows if row[1] is not None]
    failed = [row for row in rows if row[1] is None]
    scored.sort(key=lambda row: rank_key(row[1]))

    if args.limit:
        scored_shown = scored[: args.limit]
    else:
        scored_shown = scored

    print()
    print(f"{'combo':<26}{'exact':<7}{'Δsize':>8}{'diffwords':>11}{'firstmiss':>11}   flags")
    for combo, sc, _err, _elapsed in scored_shown:
        flags = " ".join(combo.opt + combo.isa + combo.extra)
        cc = "ido-phases.py" if combo.use_ido_phases else "cc"
        fm = "-" if sc.first_mismatch is None else f"+0x{sc.first_mismatch:x}"
        print(
            f"{combo.id:<26}{'YES' if sc.exact else 'no':<7}{sc.size_delta:>+8}{sc.diff_words:>11}{fm:>11}   {cc} {flags}"
        )

    if scored:
        best = scored[0][0]
        print()
        print("Best flags (paste into the Makefile per-file override block):")
        cc_line = "CC := $(IDO_PHASES)" if best.use_ido_phases else ""
        if cc_line:
            print(f"  {cc_line}")
        print(f"  OPT_FLAGS := {' '.join(best.opt) or '(none)'}")
        print(f"  MIPSISET := {' '.join(best.isa)}")
        if best.extra:
            print(f"  CFLAGS += {' '.join(best.extra)}")
        if scored[0][1].exact:
            print("  -> BYTE-IDENTICAL under this tool's masked word comparison.")
            print("     Confirm with tools/wb_compare.sh before declaring it matched.")

    if failed:
        print(f"\n{len(failed)} combo(s) failed to compile or extract; rerun with --keep to inspect logs.")

    if args.objdiff and scored:
        best_combo = scored[0][0]
        best_obj = sweep_root / best_combo.id / "out.o"
        pct = objdiff_match_percent(target_dir / "target.o", best_obj, args.function)
        if pct is not None:
            print(f"\nobjdiff-cli match for the top row: {pct:.2f}%")
        else:
            print("\nobjdiff-cli not available or gave no parseable output; skipped.")

    print(
        f"\n{len(lattice)} combos compiled in {compile_elapsed:.1f}s "
        f"({compile_elapsed/len(lattice):.2f}s/combo average, {workers} workers)"
    )

    if not args.keep:
        import shutil

        shutil.rmtree(sweep_root, ignore_errors=True)
    else:
        print(f"kept: {sweep_root.relative_to(REPO_ROOT)}")

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
