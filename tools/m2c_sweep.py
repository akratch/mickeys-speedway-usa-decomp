#!/usr/bin/env python3
"""Report-only, context-aware m2c sweep for Mickey's remaining GLOBAL_ASMs.

The default corpus is deliberately narrower than "every GLOBAL_ASM pragma":
fallback pragmas inside NON_MATCHING/NON_EQUIVALENT guards already have a C
candidate and are inventoried but excluded.  Each eligible bare pragma is:

1. decompiled by the vendored m2c with a preprocessed copy of its owning TU as
   context;
2. substituted into a scratch copy of that complete TU;
3. compiled by asm-processor/IDO with the effective Make variables for the
   real object; and
4. compared against a freshly assembled copy of the extracted function.

No canonical source is edited and no candidate is promoted.  All candidates,
objects, logs, and reports live below build/m2c_sweep/ by default.  Exact text
in this scratch harness is only ``scratch_text_exact``; matching relocation
records upgrades it to ``scratch_relocation_exact``.  This tool never claims a
canonical full-TU, linked, or ROM exact result.

Examples:
    nice -n 15 .venv/bin/python tools/m2c_sweep.py --inventory-only --fresh
    nice -n 15 .venv/bin/python tools/m2c_sweep.py --fresh
    nice -n 15 .venv/bin/python tools/m2c_sweep.py --symbol func_8002AA50 --fresh
"""

from __future__ import annotations

import argparse
import dataclasses
import functools
import hashlib
import json
import re
import shlex
import shutil
import subprocess
import sys
import time
from collections import Counter
from pathlib import Path
from typing import Iterable, Optional, Sequence


ROOT = Path(__file__).resolve().parent.parent
DEFAULT_OUT = ROOT / "build" / "m2c_sweep"
PYTHON = ROOT / ".venv" / "bin" / "python"
M2C = ROOT / "tools" / "m2c" / "m2c.py"
M2C_MACROS_INCLUDE = '#include "tools/m2c/m2c_macros.h"\n'
ASM_PROCESSOR = ROOT / "tools" / "asm-processor" / "build.py"
AS = ROOT / "tools" / "binutils" / "mips64-elf-as"
OBJDUMP = ROOT / "tools" / "binutils" / "mips64-elf-objdump"
OBJCOPY = ROOT / "tools" / "binutils" / "mips64-elf-objcopy"
ASM_FLAGS = ["-march=vr4300", "-32", "-mabi=32", "-G0", "-I", "include"]
ASM_PROC_FLAGS = ASM_FLAGS
ASM_HEADER = '.set noat\n.set noreorder\n.include "macro.inc"\n.section .text, "ax"\n'

PRAGMA_RE = re.compile(
    r'^\s*#pragma\s+GLOBAL_ASM\s*\(\s*"([^"]+\.s)"\s*\)\s*$', re.MULTILINE
)
IF_RE = re.compile(r"^\s*#\s*(if|ifdef|ifndef)\b(.*)$")
ELSE_RE = re.compile(r"^\s*#\s*(else|elif)\b(.*)$")
ENDIF_RE = re.compile(r"^\s*#\s*endif\b")
GUARDS = ("NON_MATCHING", "NON_EQUIVALENT")


@dataclasses.dataclass(frozen=True)
class Item:
    source: str
    line: int
    asm: str
    symbol: str
    category: str

    @property
    def key(self) -> str:
        return f"{self.source}:{self.line}:{self.symbol}"


def compact_error(text: str, lines: int = 8, chars: int = 2400) -> str:
    selected = [line.rstrip() for line in text.strip().splitlines()[-lines:]]
    return "\n".join(selected)[-chars:]


def run(
    cmd: Sequence[object],
    *,
    timeout: int,
    stdout_path: Optional[Path] = None,
) -> subprocess.CompletedProcess[str]:
    """Run one bounded child at low priority.

    The driver is intentionally sequential.  Prefixing every external child
    makes the workstation-safety property explicit even if the caller forgets
    to invoke the driver through ``nice``.
    """

    full_cmd = ["nice", "-n", "15"] + [str(part) for part in cmd]
    if stdout_path is None:
        return subprocess.run(
            full_cmd,
            cwd=ROOT,
            capture_output=True,
            text=True,
            timeout=timeout,
        )
    stdout_path.parent.mkdir(parents=True, exist_ok=True)
    with stdout_path.open("w", encoding="utf-8") as out:
        return subprocess.run(
            full_cmd,
            cwd=ROOT,
            stdout=out,
            stderr=subprocess.PIPE,
            text=True,
            timeout=timeout,
        )


def guard_category(stack: Sequence[set[str]]) -> str:
    active = set().union(*stack) if stack else set()
    if "NON_EQUIVALENT" in active:
        return "non_equivalent"
    if "NON_MATCHING" in active:
        return "non_matching"
    return "global_asm"


def inventory() -> list[Item]:
    items: list[Item] = []
    for source in sorted((ROOT / "src").rglob("*.c")):
        rel_source = source.relative_to(ROOT).as_posix()
        stack: list[set[str]] = []
        for line_no, line in enumerate(source.read_text(encoding="utf-8").splitlines(), 1):
            if ENDIF_RE.match(line):
                if stack:
                    stack.pop()
                continue
            if IF_RE.match(line):
                stack.append({guard for guard in GUARDS if guard in line})
                continue
            if ELSE_RE.match(line):
                # The classification follows the enclosing guard regardless
                # of which branch contains the fallback pragma.
                continue
            match = PRAGMA_RE.match(line)
            if not match:
                continue
            asm = match.group(1)
            items.append(
                Item(
                    source=rel_source,
                    line=line_no,
                    asm=asm,
                    symbol=Path(asm).stem,
                    category=guard_category(stack),
                )
            )
    return items


def write_json(path: Path, value: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def safe_output_root(path: Path) -> Path:
    resolved = path.resolve()
    build = (ROOT / "build").resolve()
    if resolved == build or build not in resolved.parents:
        raise ValueError(f"output must be a child of {build}, got {resolved}")
    return resolved


def prepare_output(path: Path, *, fresh: bool, resume: bool) -> None:
    if path.exists() and fresh:
        shutil.rmtree(path)
    elif path.exists() and not resume:
        raise FileExistsError(f"{path} already exists; pass --fresh or --resume")
    path.mkdir(parents=True, exist_ok=True)


def preprocess_context(source: Path, context: Path, timeout: int) -> tuple[bool, str]:
    cmd = [
        "gcc",
        "-E",
        "-P",
        "-I.",
        "-Iinclude",
        "-Iinclude/libc",
        "-Iinclude/PR",
        "-Iinclude/sys",
        "-Iassets",
        "-Isrc",
        "-undef",
        "-D__sgi",
        "-D_LANGUAGE_C",
        "-D_Static_assert(x,y)=",
        "-D__attribute__(x)=",
        source.relative_to(ROOT),
    ]
    try:
        proc = run(cmd, timeout=timeout, stdout_path=context)
    except subprocess.TimeoutExpired as exc:
        return False, f"context_timeout:{exc.timeout}s"
    if proc.returncode != 0:
        return False, compact_error(proc.stderr or "context preprocessing failed")
    return True, ""


def effective_make_flags(source: Path, timeout: int) -> tuple[Optional[dict[str, str]], str]:
    obj = "build/" + source.relative_to(ROOT).as_posix() + ".o"
    recipe = (
        f"{obj}: ; @printf '%s\\n' "
        "'CC=$(CC)' 'CFLAGS=$(CFLAGS)' 'OPT_FLAGS=$(OPT_FLAGS)' "
        "'MIPSISET=$(MIPSISET)'"
    )
    try:
        proc = run(["gmake", "-s", "-B", f"--eval={recipe}", obj], timeout=timeout)
    except subprocess.TimeoutExpired as exc:
        return None, f"make_flags_timeout:{exc.timeout}s"
    if proc.returncode != 0:
        return None, compact_error(proc.stderr or proc.stdout or "make flag query failed")
    values: dict[str, str] = {}
    for line in proc.stdout.splitlines():
        key, sep, value = line.partition("=")
        if sep and key in {"CC", "CFLAGS", "OPT_FLAGS", "MIPSISET"}:
            values[key] = value.strip()
    missing = {"CC", "CFLAGS", "OPT_FLAGS", "MIPSISET"} - values.keys()
    if missing:
        return None, "make flag query omitted: " + ", ".join(sorted(missing))
    return values, ""


def make_work_name(index: int, item: Item) -> str:
    digest = hashlib.sha1(item.key.encode("utf-8")).hexdigest()[:8]
    symbol = re.sub(r"[^A-Za-z0-9_.-]", "_", item.symbol)
    return f"{index:04d}_{symbol}_{digest}"


@functools.lru_cache(maxsize=1)
def data_label_index() -> dict[str, Path]:
    """Map extracted data labels to their owning assembly file.

    Splat keeps jump tables in separate rodata files, while m2c needs the
    table and function in the same invocation.  This is an input-discovery
    step only; no data from the table is copied into a tracked file.
    """

    result: dict[str, Path] = {}
    label_re = re.compile(r"^\s*(?:dlabel|glabel|alabel)\s+(jtbl_[A-Za-z0-9_]+)\s*$")
    for path in sorted((ROOT / "asm").rglob("*.s")):
        for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
            match = label_re.match(line)
            if match:
                result.setdefault(match.group(1), path)
    return result


def supplemental_asm(item: Item) -> list[Path]:
    text = (ROOT / item.asm).read_text(encoding="utf-8", errors="replace")
    labels = sorted(set(re.findall(r"\bjtbl_[A-Za-z0-9_]+\b", text)))
    index = data_label_index()
    return sorted({index[label] for label in labels if label in index})


def generate_m2c(
    item: Item,
    context: Optional[Path],
    extra_asm: Sequence[Path],
    output: Path,
    timeout: int,
) -> tuple[bool, str]:
    cmd = [
        PYTHON,
        M2C,
        "--target",
        "mips-ido-c",
        "--pointer-style",
        "right",
        "--valid-syntax",
        "--no-cache",
    ]
    if context is not None:
        cmd += ["--context", context]
    cmd += ["-f", item.symbol, ROOT / item.asm] + list(extra_asm)
    try:
        proc = run(cmd, timeout=timeout, stdout_path=output)
    except subprocess.TimeoutExpired as exc:
        return False, f"m2c_timeout:{exc.timeout}s"
    if proc.returncode != 0:
        emitted = output.read_text(encoding="utf-8", errors="replace") if output.exists() else ""
        return False, compact_error(proc.stderr or emitted or "m2c failed")
    if not output.exists() or not output.stat().st_size:
        return False, "m2c produced empty output"
    return True, compact_error(proc.stderr) if proc.stderr else ""


def scratch_source(item: Item, generated: Path, output: Path) -> tuple[bool, str]:
    source = ROOT / item.source
    lines = source.read_text(encoding="utf-8").splitlines(keepends=True)
    index = item.line - 1
    if index < 0 or index >= len(lines):
        return False, "recorded pragma line is outside the source file"
    match = PRAGMA_RE.match(lines[index].rstrip("\n"))
    if not match or match.group(1) != item.asm:
        return False, "source pragma changed since inventory"
    body = generated.read_text(encoding="utf-8")
    lines[index] = M2C_MACROS_INCLUDE + body.rstrip() + "\n"
    output.write_text("".join(lines), encoding="utf-8")
    return True, ""


def classify_compile_error(text: str) -> str:
    lower = text.lower()
    if "timed out" in lower:
        return "compile_timeout"
    if "signal 11" in lower or "internal compiler error" in lower:
        return "compile_internal_error"
    if "too many errors" in lower or "limit of 30 errors" in lower:
        return "compile_too_many_errors"
    if "dereferenced a non-pointer" in lower or "shall have pointer type" in lower:
        return "compile_nonpointer"
    if "bad operand type" in lower or "unacceptable operand" in lower:
        return "compile_bad_operand"
    if "number of arguments" in lower or "full prototype" in lower:
        return "compile_prototype_error"
    if "redecl" in lower or "conflicting" in lower or "incompatible" in lower:
        return "compile_type_conflict"
    if "syntax error" in lower or "parse error" in lower or "expected" in lower:
        return "compile_syntax_error"
    if "undeclared" in lower or "undefined" in lower or "unknown identifier" in lower:
        return "compile_undeclared"
    if "failed to assemble" in lower or "assembler" in lower:
        return "asm_processor_error"
    return "compile_error"


def compile_candidate(
    candidate: Path,
    output: Path,
    flags: dict[str, str],
    timeout: int,
) -> tuple[bool, str, str]:
    cc = shlex.split(flags["CC"])
    cmd = (
        [PYTHON, ASM_PROCESSOR, "--asm-prelude", "include/asm_processor_prelude.inc"]
        + cc
        + ["--", AS]
        + ASM_PROC_FLAGS
        + ["--", "-c"]
        + shlex.split(flags["CFLAGS"])
        + shlex.split(flags["OPT_FLAGS"])
        + shlex.split(flags["MIPSISET"])
        + ["-o", output, candidate]
    )
    try:
        proc = run(cmd, timeout=timeout)
    except subprocess.TimeoutExpired as exc:
        return False, "compile_timeout", f"compile_timeout:{exc.timeout}s"
    if proc.returncode != 0:
        err = compact_error((proc.stdout or "") + "\n" + (proc.stderr or ""))
        return False, classify_compile_error(err), err
    return True, "", compact_error(proc.stderr) if proc.stderr else ""


def assemble_target(item: Item, source: Path, output: Path, timeout: int) -> tuple[bool, str]:
    source.write_text(ASM_HEADER + (ROOT / item.asm).read_text(encoding="utf-8"), encoding="utf-8")
    try:
        proc = run([AS] + ASM_FLAGS + ["-o", output, source], timeout=timeout)
    except subprocess.TimeoutExpired as exc:
        return False, f"target_assemble_timeout:{exc.timeout}s"
    if proc.returncode != 0:
        return False, compact_error(proc.stderr or proc.stdout or "target assembly failed")
    return True, ""


def checked_output(cmd: Sequence[object], timeout: int) -> str:
    proc = run(cmd, timeout=timeout)
    if proc.returncode != 0:
        raise RuntimeError(compact_error(proc.stderr or proc.stdout or "command failed"))
    return proc.stdout


def elf_symbol(obj: Path, symbol: str, timeout: int) -> Optional[tuple[int, int, str]]:
    output = checked_output([OBJDUMP, "-t", obj], timeout)
    for line in output.splitlines():
        parts = line.split()
        if len(parts) < 5 or parts[-1] != symbol:
            continue
        try:
            address = int(parts[0], 16)
            size = int(parts[-2], 16)
        except ValueError:
            continue
        return address, size, parts[-3]
    return None


def section_bytes(obj: Path, section: str, output: Path, timeout: int) -> bytes:
    if output.exists():
        output.unlink()
    proc = run([OBJCOPY, "-O", "binary", f"--only-section={section}", obj, output], timeout=timeout)
    if proc.returncode != 0:
        raise RuntimeError(compact_error(proc.stderr or "objcopy failed"))
    return output.read_bytes() if output.exists() else b""


def relocations(
    obj: Path,
    section: str,
    start: int,
    size: int,
    timeout: int,
) -> list[tuple[int, str, str]]:
    output = checked_output([OBJDUMP, "-r", obj], timeout)
    current = ""
    result: list[tuple[int, str, str]] = []
    for line in output.splitlines():
        match = re.match(r"RELOCATION RECORDS FOR \[(.+)\]:", line)
        if match:
            current = match.group(1)
            continue
        if current != section:
            continue
        parts = line.split()
        if len(parts) < 3 or not parts[1].startswith("R_MIPS"):
            continue
        try:
            offset = int(parts[0], 16)
        except ValueError:
            continue
        if start <= offset < start + size:
            result.append((offset - start, parts[1], " ".join(parts[2:])))
    return result


def mismatch_metrics(target: bytes, candidate: bytes) -> tuple[Optional[int], int]:
    shared_words = min(len(target), len(candidate)) // 4
    first: Optional[int] = None
    different = 0
    for index in range(shared_words):
        offset = index * 4
        if target[offset : offset + 4] != candidate[offset : offset + 4]:
            different += 1
            if first is None:
                first = offset
    extra_words = (abs(len(target) - len(candidate)) + 3) // 4
    different += extra_words
    if extra_words and first is None:
        first = shared_words * 4
    return first, different


def compare_objects(item: Item, target: Path, candidate: Path, work: Path, timeout: int) -> dict[str, object]:
    target_symbol = elf_symbol(target, item.symbol, timeout)
    candidate_symbol = elf_symbol(candidate, item.symbol, timeout)
    if target_symbol is None:
        raise LookupError("target_symbol_missing")
    if candidate_symbol is None:
        raise LookupError("candidate_symbol_missing")

    target_start, target_size, target_section = target_symbol
    candidate_start, candidate_size, candidate_section = candidate_symbol
    target_section_bytes = section_bytes(target, target_section, work / "target_section.bin", timeout)
    candidate_section_bytes = section_bytes(candidate, candidate_section, work / "candidate_section.bin", timeout)
    target_text = target_section_bytes[target_start : target_start + target_size]
    candidate_text = candidate_section_bytes[candidate_start : candidate_start + candidate_size]
    first, different = mismatch_metrics(target_text, candidate_text)
    text_exact = target_size == candidate_size and target_text == candidate_text

    target_relocs = relocations(target, target_section, target_start, target_size, timeout)
    candidate_relocs = relocations(candidate, candidate_section, candidate_start, candidate_size, timeout)
    reloc_exact = target_relocs == candidate_relocs
    return {
        "target_size": target_size,
        "candidate_size": candidate_size,
        "size_delta": candidate_size - target_size,
        "different_words": different,
        "first_mismatch": first,
        "text_exact": text_exact,
        "target_relocation_count": len(target_relocs),
        "candidate_relocation_count": len(candidate_relocs),
        "relocation_exact": reloc_exact,
    }


def load_resume(path: Path) -> dict[str, dict[str, object]]:
    rows: dict[str, dict[str, object]] = {}
    if not path.exists():
        return rows
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line.strip():
            continue
        row = json.loads(line)
        rows[str(row["key"])] = row
    return rows


def append_result(path: Path, result: dict[str, object]) -> None:
    with path.open("a", encoding="utf-8") as fh:
        fh.write(json.dumps(result, sort_keys=True) + "\n")


def summarize(items: Sequence[Item], results: Iterable[dict[str, object]], started: float) -> dict[str, object]:
    rows = list(results)
    categories = Counter(item.category for item in items)
    reasons = Counter(str(row.get("reason", "unknown")) for row in rows)
    generated = sum(bool(row.get("generated")) for row in rows)
    compileable = sum(bool(row.get("compileable")) for row in rows)
    scratch_exact = sum(bool((row.get("compare") or {}).get("text_exact")) for row in rows)
    relocation_exact = sum(
        bool((row.get("compare") or {}).get("text_exact"))
        and bool((row.get("compare") or {}).get("relocation_exact"))
        for row in rows
    )
    exact_candidates = [
        {
            "source": row["source"],
            "line": row["line"],
            "symbol": row["symbol"],
            "stage": row["stage"],
        }
        for row in rows
        if (row.get("compare") or {}).get("text_exact")
    ]
    return {
        "schema_version": 1,
        "generated_by": "tools/m2c_sweep.py",
        "elapsed_seconds": round(time.monotonic() - started, 3),
        "inventory": {
            "total_pragmas": len(items),
            "by_category": dict(sorted(categories.items())),
            "eligible_global_asm": categories.get("global_asm", 0),
        },
        "attempted": len(rows),
        "generated": generated,
        "compileable": compileable,
        "exact_by_evidence_stage": {
            "scratch_text_exact": scratch_exact,
            "scratch_relocation_exact": relocation_exact,
            "canonical_full_tu_exact": 0,
            "linked_exact": 0,
            "rom_exact": 0,
        },
        "reason_counts": dict(sorted(reasons.items())),
        "exact_candidates": exact_candidates,
    }


def render_report(summary: dict[str, object]) -> str:
    inv = summary["inventory"]
    stages = summary["exact_by_evidence_stage"]
    lines = [
        "# Report-only m2c sweep",
        "",
        f"- Pragmas inventoried: {inv['total_pragmas']}",
        f"- Eligible bare GLOBAL_ASM: {inv['eligible_global_asm']}",
        f"- Attempted: {summary['attempted']}",
        f"- m2c-generated: {summary['generated']}",
        f"- Scratch full-TU compileable: {summary['compileable']}",
        f"- Scratch text exact: {stages['scratch_text_exact']}",
        f"- Scratch text + relocation exact: {stages['scratch_relocation_exact']}",
        "- Canonical full-TU / linked / ROM exact: not attempted",
        "",
        "## Inventory categories",
        "",
    ]
    for key, count in inv["by_category"].items():
        lines.append(f"- {key}: {count}")
    lines.extend(["", "## Result reasons", ""])
    for key, count in summary["reason_counts"].items():
        lines.append(f"- {key}: {count}")
    lines.extend(["", "## Exact candidates", ""])
    if summary["exact_candidates"]:
        for row in summary["exact_candidates"]:
            lines.append(f"- {row['symbol']} ({row['source']}:{row['line']}): {row['stage']}")
    else:
        lines.append("- None.")
    lines.extend(
        [
            "",
            "The scratch harness compiles a complete copied TU with the real effective",
            "Make flags, but it does not compile the canonical path, run target-specific",
            "post-processing, link an overlay/ROM, or compare a linked owned range. Its",
            "strongest possible result is therefore scratch relocation exact.",
            "",
        ]
    )
    return "\n".join(lines)


def parse_args(argv: Sequence[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    parser.add_argument("--inventory-only", action="store_true")
    parser.add_argument("--symbol", action="append", default=[], help="attempt only this symbol; repeatable")
    parser.add_argument("--limit", type=int, default=None, help="attempt only the first N selected items")
    parser.add_argument("--timeout", type=int, default=120, help="per external process timeout in seconds")
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--fresh", action="store_true", help="replace an existing output directory")
    mode.add_argument("--resume", action="store_true", help="skip keys already recorded in results.jsonl")
    return parser.parse_args(argv)


def main(argv: Sequence[str]) -> int:
    args = parse_args(argv)
    try:
        out = safe_output_root(args.out if args.out.is_absolute() else ROOT / args.out)
        prepare_output(out, fresh=args.fresh, resume=args.resume)
    except (ValueError, FileExistsError) as exc:
        print(f"m2c_sweep: {exc}", file=sys.stderr)
        return 2

    started = time.monotonic()
    all_items = inventory()
    write_json(out / "inventory.json", [dataclasses.asdict(item) for item in all_items])
    selected = [item for item in all_items if item.category == "global_asm"]
    if args.symbol:
        wanted = set(args.symbol)
        selected = [item for item in selected if item.symbol in wanted]
        missing = wanted - {item.symbol for item in selected}
        if missing:
            print("m2c_sweep: no eligible bare GLOBAL_ASM for: " + ", ".join(sorted(missing)), file=sys.stderr)
            return 2
    if args.limit is not None:
        selected = selected[: args.limit]

    results_path = out / "results.jsonl"
    previous = load_resume(results_path) if args.resume else {}
    results = list(previous.values())
    write_json(
        out / "run.json",
        {
            "selected": len(selected),
            "timeout_seconds": args.timeout,
            "mode": "inventory" if args.inventory_only else "sweep",
            "command": sys.argv,
        },
    )

    if not args.inventory_only:
        contexts: dict[str, tuple[bool, Path, str]] = {}
        flags_cache: dict[str, tuple[Optional[dict[str, str]], str]] = {}
        pending = [item for item in selected if item.key not in previous]
        for number, item in enumerate(pending, 1):
            row: dict[str, object] = {
                "key": item.key,
                "source": item.source,
                "line": item.line,
                "asm": item.asm,
                "symbol": item.symbol,
                "category": item.category,
                "generated": False,
                "compileable": False,
                "stage": "inventory_only",
            }
            asm_path = ROOT / item.asm
            if not asm_path.is_file():
                row.update(reason="missing_asm", error=f"missing {item.asm}")
                append_result(results_path, row)
                results.append(row)
                print(f"[{number}/{len(pending)}] {item.symbol}: missing_asm", flush=True)
                continue

            source = ROOT / item.source
            if item.source not in contexts:
                context = out / "contexts" / (item.source + ".ctx.c")
                context.parent.mkdir(parents=True, exist_ok=True)
                ok, error = preprocess_context(source, context, args.timeout)
                contexts[item.source] = (ok, context, error)
            context_ok, context, context_error = contexts[item.source]
            if not context_ok:
                row.update(reason="context_error", error=context_error)
                append_result(results_path, row)
                results.append(row)
                print(f"[{number}/{len(pending)}] {item.symbol}: context_error", flush=True)
                continue

            if item.source not in flags_cache:
                flags_cache[item.source] = effective_make_flags(source, args.timeout)
            flags, flags_error = flags_cache[item.source]
            if flags is None:
                row.update(reason="make_flags_error", error=flags_error)
                append_result(results_path, row)
                results.append(row)
                print(f"[{number}/{len(pending)}] {item.symbol}: make_flags_error", flush=True)
                continue
            row["make_flags"] = flags

            work = out / "items" / make_work_name(number, item)
            work.mkdir(parents=True, exist_ok=True)
            generated = work / "m2c.c"
            extra_asm = supplemental_asm(item)
            if extra_asm:
                row["supplemental_asm"] = [path.relative_to(ROOT).as_posix() for path in extra_asm]
            ok, note = generate_m2c(item, context, extra_asm, generated, args.timeout)
            if not ok:
                context_attempt_error = note
                row["context_attempt_error"] = context_attempt_error
                ok, note = generate_m2c(item, None, extra_asm, generated, args.timeout)
                if ok:
                    row["m2c_context_mode"] = "context_parse_failed_then_no_context"
            else:
                row["m2c_context_mode"] = "owning_tu"
            if not ok:
                row.update(reason="m2c_error", error=note, stage="context_generated")
                append_result(results_path, row)
                results.append(row)
                print(f"[{number}/{len(pending)}] {item.symbol}: m2c_error", flush=True)
                continue
            row["generated"] = True
            row["stage"] = "m2c_generated"
            if note:
                row["m2c_note"] = note

            candidate_source = work / "candidate.c"
            ok, error = scratch_source(item, generated, candidate_source)
            if not ok:
                row.update(reason="scratch_source_error", error=error)
                append_result(results_path, row)
                results.append(row)
                print(f"[{number}/{len(pending)}] {item.symbol}: scratch_source_error", flush=True)
                continue

            target_source = work / "target.s"
            target_obj = work / "target.o"
            ok, error = assemble_target(item, target_source, target_obj, args.timeout)
            if not ok:
                row.update(reason="target_assemble_error", error=error)
                append_result(results_path, row)
                results.append(row)
                print(f"[{number}/{len(pending)}] {item.symbol}: target_assemble_error", flush=True)
                continue

            candidate_obj = work / "candidate.o"
            ok, compile_reason, compile_note = compile_candidate(
                candidate_source, candidate_obj, flags, args.timeout
            )
            if not ok:
                row.update(reason=compile_reason, error=compile_note)
                append_result(results_path, row)
                results.append(row)
                print(f"[{number}/{len(pending)}] {item.symbol}: {compile_reason}", flush=True)
                continue
            row["compileable"] = True
            row["stage"] = "scratch_full_tu_compileable"
            if compile_note:
                row["compile_note"] = compile_note

            try:
                comparison = compare_objects(item, target_obj, candidate_obj, work, args.timeout)
            except (LookupError, RuntimeError, subprocess.TimeoutExpired) as exc:
                reason = str(exc) if isinstance(exc, LookupError) else "comparison_error"
                row.update(reason=reason, error=compact_error(str(exc)))
                append_result(results_path, row)
                results.append(row)
                print(f"[{number}/{len(pending)}] {item.symbol}: {reason}", flush=True)
                continue

            row["compare"] = comparison
            if comparison["text_exact"] and comparison["relocation_exact"]:
                row.update(reason="scratch_relocation_exact", stage="scratch_relocation_exact")
            elif comparison["text_exact"]:
                row.update(reason="scratch_text_exact_relocation_mismatch", stage="scratch_text_exact")
            elif comparison["candidate_size"] != comparison["target_size"]:
                row.update(reason="compiled_size_mismatch", stage="compiled_nonexact")
            elif not comparison["relocation_exact"]:
                row.update(reason="compiled_text_and_relocation_mismatch", stage="compiled_nonexact")
            else:
                row.update(reason="compiled_text_mismatch", stage="compiled_nonexact")
            append_result(results_path, row)
            results.append(row)
            print(f"[{number}/{len(pending)}] {item.symbol}: {row['reason']}", flush=True)

    summary = summarize(all_items, results, started)
    write_json(out / "summary.json", summary)
    (out / "report.md").write_text(render_report(summary), encoding="utf-8")
    print(
        "m2c_sweep: "
        f"eligible={summary['inventory']['eligible_global_asm']} "
        f"attempted={summary['attempted']} generated={summary['generated']} "
        f"compileable={summary['compileable']} "
        f"scratch_exact={summary['exact_by_evidence_stage']['scratch_text_exact']} "
        f"relocation_exact={summary['exact_by_evidence_stage']['scratch_relocation_exact']}"
    )
    print(f"m2c_sweep: report={out.relative_to(ROOT) / 'report.md'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
