#!/usr/bin/env python3
"""Build a compact, fail-closed receipt for one IDO allocator trace.

The instrumented IDO 5.3 uopt pass identifies procedures by invocation ordinal,
not by linker symbol. This command joins that ordinal to the procedure name in
the exact Ucode stream consumed by uopt, and independently confirms the ELF
function's owned range. It also delegates the trace-off object comparison to
decomp-workbench's section/relocation/symbol fidelity gate before reporting
allocator evidence.

The receipt deliberately contains hashes and aggregate allocator decisions, not
instruction listings or raw trace rows.  Raw logs remain untracked workbench
evidence.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import site
import subprocess
import sys
from collections import Counter
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Sequence


SCHEMA = "mickey-allocator-trace-receipt-v1"
PROC_INDEX_RE = re.compile(
    r"^\[CDX\]\s+procindex\s+proc=(?P<proc>\d+)\s+decisions=(?P<decisions>\d+)\s*$"
)
CDX_RE = re.compile(r"^\[CDX\]\s+(?P<event>\S+)\s+(?P<fields>.*)$")
FIELD_RE = re.compile(r"([A-Za-z0-9_]+)=([^\s]+)")
UGEN_RESULT_RE = re.compile(
    r"^DKWB-FREELIST\s+(?P<event>ALLOC_GP_RESULT|ALLOC_FP_RESULT)\s+(?P<fields>.*)$"
)

INTEGER_REGISTERS = (
    "zero",
    "at",
    "v0",
    "v1",
    "a0",
    "a1",
    "a2",
    "a3",
    "t0",
    "t1",
    "t2",
    "t3",
    "t4",
    "t5",
    "t6",
    "t7",
    "s0",
    "s1",
    "s2",
    "s3",
    "s4",
    "s5",
    "s6",
    "s7",
    "t8",
    "t9",
    "k0",
    "k1",
    "gp",
    "sp",
    "s8",
    "ra",
)


class ReceiptError(RuntimeError):
    """An evidence precondition was absent or ambiguous."""


@dataclass(frozen=True)
class FunctionSymbol:
    name: str
    value: int
    size: int
    section: str

    @property
    def end(self) -> int:
        return self.value + self.size


@dataclass(frozen=True)
class FunctionGroup:
    value: int
    size: int
    section: str
    names: tuple[str, ...]

    @property
    def end(self) -> int:
        return self.value + self.size


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def sha256_text(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8", errors="strict")).hexdigest()


def group_function_symbols(
    symbols: Sequence[FunctionSymbol],
) -> list[FunctionGroup]:
    """Group true aliases while rejecting overlapping, ambiguous functions."""

    grouped: dict[tuple[str, int, int], list[str]] = {}
    by_start: dict[tuple[str, int], set[int]] = {}
    for symbol in symbols:
        grouped.setdefault((symbol.section, symbol.value, symbol.size), []).append(
            symbol.name
        )
        by_start.setdefault((symbol.section, symbol.value), set()).add(symbol.size)
    ambiguous = [key for key, sizes in by_start.items() if len(sizes) != 1]
    if ambiguous:
        section, value = sorted(ambiguous)[0]
        raise ReceiptError(
            "function symbols share a start but disagree on size: "
            f"section {section} offset 0x{value:X}"
        )
    groups = [
        FunctionGroup(
            section=section,
            value=value,
            size=size,
            names=tuple(sorted(set(names))),
        )
        for (section, value, size), names in grouped.items()
    ]
    groups.sort(
        key=lambda row: (
            int(row.section) if row.section.isdigit() else 10**9,
            row.section,
            row.value,
        )
    )
    return groups


def parse_proc_index(text: str) -> dict[int, int]:
    rows: dict[int, int] = {}
    for raw in text.splitlines():
        match = PROC_INDEX_RE.match(raw.strip())
        if match is None:
            continue
        proc = int(match.group("proc"))
        decisions = int(match.group("decisions"))
        if proc in rows:
            raise ReceiptError(
                f"procedure index contains duplicate proc={proc}; use one fresh capture"
            )
        rows[proc] = decisions
    if not rows:
        raise ReceiptError(
            "procedure index trace has no [CDX] procindex rows; run the "
            "instrumented uopt once with CDX_PROC set to a nonnumeric name"
        )
    expected = list(range(len(rows)))
    if sorted(rows) != expected:
        raise ReceiptError(
            "procedure index is not contiguous from zero; use one complete capture"
        )
    return rows


def map_symbol_to_procedure(
    symbol: str,
    symbols: Sequence[FunctionSymbol],
    proc_index: dict[int, int],
    procedure_names: Sequence[str],
) -> tuple[int, FunctionGroup, int]:
    groups = group_function_symbols(symbols)
    matching = [group for group in groups if symbol in group.names]
    if len(matching) != 1:
        raise ReceiptError(
            f"candidate object contains {len(matching)} nonempty FUNC ranges for {symbol!r}"
        )
    if len(procedure_names) != len(proc_index):
        raise ReceiptError(
            "cannot prove symbol-to-procedure mapping: retained Ucode has "
            f"{len(procedure_names)} named procedures but trace has "
            f"{len(proc_index)} procedure rows"
        )
    ordinals = [index for index, name in enumerate(procedure_names) if name == symbol]
    if len(ordinals) != 1:
        raise ReceiptError(
            f"retained Ucode contains {len(ordinals)} procedures named {symbol!r}"
        )
    ordinal = ordinals[0]
    if ordinal not in proc_index:
        raise ReceiptError(f"procedure index has no row for Ucode ordinal {ordinal}")
    return ordinal, matching[0], len(procedure_names)


def _load_workbench_parsers() -> tuple[Any, Any]:
    """Load the installed workbench, including an ordinary project venv."""

    try:
        from decomp_workbench.elf import parse_elf
        from decomp_workbench.ucode import parse_ucode

        return parse_elf, parse_ucode
    except ImportError:
        project = Path(__file__).resolve().parent.parent
        candidates = sorted((project / ".venv" / "lib").glob("python*/site-packages"))
        for candidate in candidates:
            # addsitedir also resolves editable-install .pth files; simply
            # prepending site-packages misses the workbench source checkout.
            site.addsitedir(str(candidate))
        try:
            from decomp_workbench.elf import parse_elf
            from decomp_workbench.ucode import parse_ucode

            return parse_elf, parse_ucode
        except ImportError as error:
            raise ReceiptError(
                "decomp-workbench Python package is unavailable; run gmake setup "
                "or invoke this command through the project .venv"
            ) from error


def procedure_names_from_ucode_records(records: Sequence[Any]) -> list[str]:
    """Extract each Uent's following procedure-name Ucomm payload."""

    names: list[str] = []
    for index, record in enumerate(records):
        if record.name != "ent":
            continue
        if index + 1 >= len(records) or records[index + 1].name != "comm":
            raise ReceiptError(
                f"Ucode Uent record {record.index} has no following name Ucomm"
            )
        name_record = records[index + 1]
        if len(name_record.words) < 6:
            raise ReceiptError(
                f"Ucode procedure-name record {name_record.index} is truncated"
            )
        length = name_record.words[4]
        payload = b"".join(
            word.to_bytes(4, "big") for word in name_record.words[6:]
        )
        if length < 1 or length > len(payload):
            raise ReceiptError(
                f"Ucode procedure-name record {name_record.index} has invalid length {length}"
            )
        try:
            name = payload[:length].rstrip(b"\0").decode("ascii")
        except UnicodeDecodeError as error:
            raise ReceiptError(
                f"Ucode procedure-name record {name_record.index} is not ASCII"
            ) from error
        if not re.fullmatch(r"[A-Za-z_$][A-Za-z0-9_.$]*", name):
            raise ReceiptError(
                f"Ucode procedure-name record {name_record.index} is malformed: {name!r}"
            )
        names.append(name)
    if not names:
        raise ReceiptError("retained uopt input contains no named Ucode procedures")
    return names


def read_ucode_procedure_names(path: Path) -> list[str]:
    _parse_elf, parse_ucode = _load_workbench_parsers()
    try:
        records = parse_ucode(path)
    except (OSError, ValueError) as error:
        raise ReceiptError(f"could not decode retained uopt Ucode: {error}") from error
    return procedure_names_from_ucode_records(records)


def parse_cdx_decisions(text: str, proc: int, expected_count: int) -> list[dict[str, str]]:
    decisions: list[dict[str, str]] = []
    for raw in text.splitlines():
        match = CDX_RE.match(raw.strip())
        if match is None or match.group("event") not in {"p1dec", "p2dec"}:
            continue
        fields = dict(FIELD_RE.findall(match.group("fields")))
        try:
            event_proc = int(fields.get("proc", ""), 0)
        except ValueError:
            continue
        if event_proc == proc:
            decisions.append({"phase": match.group("event")[:2], **fields})
    if len(decisions) != expected_count:
        raise ReceiptError(
            f"uopt detail trace has {len(decisions)} decisions for proc={proc}; "
            f"procedure index requires {expected_count}"
        )
    return decisions


def normalized_decision(row: dict[str, str]) -> str:
    keys = (
        "phase",
        "web",
        "class",
        "decision",
        "bestcolor",
        "bestreg",
        "forced",
    )
    return " ".join(f"{key}={row[key]}" for key in keys if key in row)


def decision_family(row: dict[str, str]) -> str:
    """Class 2 is the pinned profile's floating-point allocator class."""

    value = row.get("class")
    if value == "2":
        return "fp_pool"
    if value is not None:
        try:
            int(value, 0)
        except ValueError:
            return "unknown_pool"
        return "integer_pool"
    return "unknown_pool"


def summarize_decisions(rows: Sequence[dict[str, str]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for family in ("integer_pool", "fp_pool", "unknown_pool"):
        selected = [row for row in rows if decision_family(row) == family]
        outcomes = Counter(row.get("decision", "unknown") for row in selected)
        phases = Counter(row.get("phase", "unknown") for row in selected)
        registers = Counter(
            row["bestreg"]
            for row in selected
            if row.get("bestreg") not in {None, "?"}
        )
        normalized = "\n".join(normalized_decision(row) for row in selected)
        result[family] = {
            "decisions": len(selected),
            "phases": dict(sorted(phases.items())),
            "outcomes": dict(sorted(outcomes.items())),
            "registers": dict(sorted(registers.items())),
            "decision_digest": hashlib.sha256(normalized.encode()).hexdigest(),
        }
    return result


def register_name(number: int, *, fp: bool) -> str:
    if fp:
        if 32 <= number < 64:
            return f"$f{number - 32}"
        return str(number)
    if 0 <= number < len(INTEGER_REGISTERS):
        return INTEGER_REGISTERS[number]
    return str(number)


def summarize_ugen_results(text: str) -> dict[str, Any]:
    rows: list[tuple[str, int]] = []
    for raw in text.splitlines():
        match = UGEN_RESULT_RE.match(raw.strip())
        if match is None:
            continue
        fields = dict(FIELD_RE.findall(match.group("fields")))
        try:
            register = int(fields["reg"], 0)
        except (KeyError, ValueError):
            raise ReceiptError("ugen result trace contains a malformed register")
        rows.append((match.group("event"), register))
    if not rows:
        raise ReceiptError(
            "ugen trace has no ALLOC_GP_RESULT/ALLOC_FP_RESULT rows; the result "
            "hooks did not fire or this is not the instrumented textual trace"
        )
    result: dict[str, Any] = {}
    for key, event, fp in (
        ("integer_temps", "ALLOC_GP_RESULT", False),
        ("fp_temps", "ALLOC_FP_RESULT", True),
    ):
        registers = [register_name(reg, fp=fp) for kind, reg in rows if kind == event]
        result[key] = {
            "allocations": len(registers),
            "registers": dict(sorted(Counter(registers).items())),
            "sequence_digest": hashlib.sha256("\n".join(registers).encode()).hexdigest(),
        }
    return result


def run_command(arguments: Sequence[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        list(arguments),
        check=False,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )


def read_object_symbols(path: Path) -> list[FunctionSymbol]:
    parse_elf, _parse_ucode = _load_workbench_parsers()
    try:
        parsed = parse_elf(path.read_bytes(), path=str(path))
    except (OSError, ValueError) as error:
        raise ReceiptError(f"could not decode candidate ELF: {error}") from error
    symbols = [
        FunctionSymbol(
            name=symbol.name,
            value=symbol.value,
            size=symbol.size,
            section=str(symbol.shndx),
        )
        for symbol in parsed.symbols
        if symbol.type == 2 and symbol.defined and symbol.size > 0
    ]
    if not symbols:
        raise ReceiptError("candidate object has no nonempty defined FUNC symbols")
    return symbols


def run_fidelity_gate(
    stock: Path,
    traced: Path,
    *,
    workbench: str,
    objdump: str,
) -> dict[str, Any]:
    process = run_command(
        [
            workbench,
            "fidelity",
            str(stock),
            str(traced),
            "--objdump",
            objdump,
            "--json",
        ]
    )
    try:
        report = json.loads(process.stdout)
    except json.JSONDecodeError as error:
        diagnostic = process.stderr.strip() or process.stdout.strip()
        raise ReceiptError(f"fidelity gate did not emit JSON: {diagnostic}") from error
    if process.returncode != 0 or report.get("pass") is not True:
        failed = sorted(name for name, passed in report.get("gates", {}).items() if not passed)
        suffix = f" ({', '.join(failed)})" if failed else ""
        raise ReceiptError(f"instrumentation fidelity gate failed{suffix}")
    return report


def compact_fidelity(report: dict[str, Any]) -> dict[str, Any]:
    return {
        "pass": True,
        "gates": dict(sorted(report.get("gates", {}).items())),
        "file_identical": bool(report.get("file_identical")),
        "stock_sha256": report.get("stock", {}).get("file_sha256"),
        "traced_sha256": report.get("instrumented", {}).get("file_sha256"),
    }


def build_mapping(args: argparse.Namespace) -> dict[str, Any]:
    candidate = Path(args.candidate_object)
    index_trace_path = Path(args.index_trace)
    ucode_path = Path(args.ucode_stream)
    for label, path in (
        ("candidate object", candidate),
        ("procedure index trace", index_trace_path),
        ("retained uopt Ucode", ucode_path),
    ):
        if not path.is_file():
            raise ReceiptError(f"{label} does not exist: {path}")

    index_text = index_trace_path.read_text(encoding="utf-8")
    symbols = read_object_symbols(candidate)
    proc_index = parse_proc_index(index_text)
    procedure_names = read_ucode_procedure_names(ucode_path)
    procedure, target, procedure_count = map_symbol_to_procedure(
        args.symbol, symbols, proc_index, procedure_names
    )
    return {
        "schema": SCHEMA,
        "status": "MAPPED",
        "baseline": {
            "symbol": args.symbol,
            "aliases": list(target.names),
            "object_range": {
                "section_index": target.section,
                "start": f"0x{target.value:X}",
                "end": f"0x{target.end:X}",
                "bytes": target.size,
            },
            "candidate_sha256": sha256_file(candidate),
            "index_trace_sha256": sha256_text(index_text),
            "ucode_sha256": sha256_file(ucode_path),
        },
        "procedure": {
            "ordinal": procedure,
            "procedure_count": procedure_count,
            "expected_decisions": proc_index[procedure],
            "mapping_proof": "ucode-ent-name-and-procindex",
        },
    }


def build_receipt(args: argparse.Namespace) -> dict[str, Any]:
    mapping = build_mapping(args)
    if not args.traced_object:
        raise ReceiptError("full receipt requires --traced-object")
    if args.attempts is None or args.budget is None:
        raise ReceiptError("full receipt requires --attempts and --budget")
    if args.attempts < 1 or args.budget < 1 or args.attempts > args.budget:
        raise ReceiptError("attempt accounting requires 1 <= attempts <= budget")

    candidate = Path(args.candidate_object)
    traced = Path(args.traced_object)
    uopt_trace_path = Path(args.uopt_trace or args.index_trace)
    for label, path in (
        ("traced object", traced),
        ("uopt detail trace", uopt_trace_path),
    ):
        if not path.is_file():
            raise ReceiptError(f"{label} does not exist: {path}")

    uopt_text = uopt_trace_path.read_text(encoding="utf-8")
    procedure = mapping["procedure"]["ordinal"]
    procedure_count = mapping["procedure"]["procedure_count"]
    expected_decisions = mapping["procedure"]["expected_decisions"]
    decisions = parse_cdx_decisions(
        uopt_text, procedure, expected_decisions
    )
    fidelity = run_fidelity_gate(
        candidate,
        traced,
        workbench=args.workbench,
        objdump=args.objdump,
    )

    if args.ugen_trace:
        if procedure_count != 1:
            raise ReceiptError(
                "current ugen result traces carry no compiled-procedure identity; "
                "a ugen receipt is accepted only when retained Ucode has one procedure"
            )
        ugen_path = Path(args.ugen_trace)
        if not ugen_path.is_file():
            raise ReceiptError(f"ugen trace does not exist: {ugen_path}")
        ugen_text = ugen_path.read_text(encoding="utf-8")
        ugen = {
            "status": "scoped-single-procedure",
            "trace_sha256": sha256_text(ugen_text),
            **summarize_ugen_results(ugen_text),
        }
        ugen_limit = "none: retained Ucode contains one compiled procedure"
    else:
        ugen = {"status": "not-provided"}
        ugen_limit = (
            "ugen temp/FP result rows are not attributed because the current "
            "producer has no compiled-procedure marker"
        )

    baseline = dict(mapping["baseline"])
    baseline["uopt_trace_sha256"] = sha256_text(uopt_text)
    return {
        **mapping,
        "status": "PASS",
        "baseline": baseline,
        "fidelity": compact_fidelity(fidelity),
        "attempts": {
            "used": args.attempts,
            "budget": args.budget,
            "remaining": args.budget - args.attempts,
        },
        "allocator": summarize_decisions(decisions),
        "ugen": ugen,
        "limits": [
            "procedure ordinals are run-local and must be remapped after any TU change",
            ugen_limit,
            "receipt aggregates are evidence routing, never match proof",
        ],
    }


def short_digest(value: str | None) -> str:
    return value[:12] if value else "unknown"


def render_text(receipt: dict[str, Any]) -> str:
    baseline = receipt["baseline"]
    function_range = baseline["object_range"]
    procedure = receipt["procedure"]
    attempts = receipt["attempts"]
    allocator = receipt["allocator"]
    lines = [
        "allocator trace receipt: PASS",
        (
            f"symbol={baseline['symbol']} proc={procedure['ordinal']} "
            f"range=section[{function_range['section_index']}]"
            f"+{function_range['start']}..{function_range['end']} "
            f"bytes={function_range['bytes']} mapping={procedure['mapping_proof']}"
        ),
        (
            f"baseline candidate={short_digest(baseline['candidate_sha256'])} "
            f"index-trace={short_digest(baseline['index_trace_sha256'])} "
            f"ucode={short_digest(baseline['ucode_sha256'])} "
            f"uopt-trace={short_digest(baseline['uopt_trace_sha256'])}"
        ),
        (
            "fidelity=PASS gates="
            + ",".join(
                name for name, passed in receipt["fidelity"]["gates"].items() if passed
            )
        ),
        (
            f"attempts={attempts['used']}/{attempts['budget']} "
            f"remaining={attempts['remaining']}"
        ),
    ]
    for family, label in (
        ("integer_pool", "uopt-integer"),
        ("fp_pool", "uopt-fp"),
        ("unknown_pool", "uopt-unknown"),
    ):
        row = allocator[family]
        lines.append(
            f"{label} decisions={row['decisions']} phases={row['phases']} "
            f"outcomes={row['outcomes']} digest={short_digest(row['decision_digest'])}"
        )
    ugen = receipt["ugen"]
    if ugen["status"] == "scoped-single-procedure":
        lines.append(
            f"ugen-temp gp={ugen['integer_temps']['allocations']} "
            f"fp={ugen['fp_temps']['allocations']} status={ugen['status']}"
        )
    else:
        lines.append(f"ugen-temp status={ugen['status']}")
    return "\n".join(lines)


def render_mapping(receipt: dict[str, Any]) -> str:
    baseline = receipt["baseline"]
    function_range = baseline["object_range"]
    procedure = receipt["procedure"]
    return (
        "allocator procedure map: "
        f"symbol={baseline['symbol']} proc={procedure['ordinal']} "
        f"range=section[{function_range['section_index']}]"
        f"+{function_range['start']}..{function_range['end']} "
        f"bytes={function_range['bytes']} procedures={procedure['procedure_count']} "
        f"candidate={short_digest(baseline['candidate_sha256'])} "
        f"index-trace={short_digest(baseline['index_trace_sha256'])} "
        f"proof={procedure['mapping_proof']}"
    )


def parser() -> argparse.ArgumentParser:
    command = argparse.ArgumentParser(
        description=(
            "Map an ELF symbol to its IDO optimizer procedure, enforce trace-off "
            "fidelity, and emit a compact allocator evidence receipt."
        )
    )
    command.add_argument("symbol")
    command.add_argument("--candidate-object", required=True)
    command.add_argument("--traced-object")
    command.add_argument("--index-trace", required=True)
    command.add_argument(
        "--ucode-stream",
        required=True,
        help="captured positional Ucode input consumed by instrumented uopt",
    )
    command.add_argument(
        "--uopt-trace",
        help="numeric-procedure detail trace (default: reuse --index-trace)",
    )
    command.add_argument(
        "--ugen-trace",
        help="optional ugen result trace; accepted only for one-procedure Ucode",
    )
    command.add_argument("--attempts", type=int)
    command.add_argument("--budget", type=int)
    command.add_argument(
        "--map-only",
        action="store_true",
        help="prove and print the procedure ordinal before capturing detail",
    )
    command.add_argument(
        "--objdump", default="tools/binutils/mips64-elf-objdump"
    )
    command.add_argument("--workbench", default=".venv/bin/decomp-workbench")
    command.add_argument("--json", action="store_true")
    return command


def main(argv: Sequence[str] | None = None) -> int:
    args = parser().parse_args(argv)
    try:
        result = build_mapping(args) if args.map_only else build_receipt(args)
    except (OSError, ReceiptError, UnicodeError) as error:
        print(f"allocator-trace-receipt: error: {error}", file=sys.stderr)
        return 2
    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
    else:
        print(render_mapping(result) if args.map_only else render_text(result))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
