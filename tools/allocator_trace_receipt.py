#!/usr/bin/env python3
"""Build a compact, fail-closed receipt for one IDO allocator trace.

The instrumented IDO 5.3 uopt pass identifies procedures by invocation ordinal,
not by linker symbol. This command joins that ordinal to the procedure name in
the exact Ucode stream consumed by uopt, and independently confirms the ELF
function's owned range. It also delegates the trace-off object comparison to
decomp-workbench's section/relocation/symbol fidelity gate before reporting
allocator evidence.

The receipt deliberately contains hashes and compact allocator mechanisms, not
instruction listings or raw trace rows. Raw logs remain untracked workbench
evidence. Optional hash-bound workbench and compact target evidence add frame,
stack-home, temp-lifecycle, and first-divergence summaries without guessing
missing fields.
"""

from __future__ import annotations

import argparse
import difflib
import hashlib
import json
import re
import site
import subprocess
import sys
from collections import Counter
from dataclasses import dataclass
from pathlib import Path, PurePath
from typing import Any, Sequence


SCHEMA = "mickey-allocator-trace-receipt-v1"
TARGET_EVIDENCE_SCHEMA = "mickey-allocator-target-evidence-v1"
WORKBENCH_SUMMARY_SCHEMA = "mickey-wb-summary-v1"
PROC_INDEX_RE = re.compile(
    r"^\[CDX\]\s+procindex\s+proc=(?P<proc>\d+)\s+decisions=(?P<decisions>\d+)\s*$"
)
CDX_RE = re.compile(r"^\[CDX\]\s+(?P<event>\S+)\s+(?P<fields>.*)$")
FIELD_RE = re.compile(r"([A-Za-z0-9_]+)=([^\s]+)")
UGEN_ROW_RE = re.compile(
    r"^DKWB-FREELIST\s+(?P<event>[A-Z][A-Z0-9_]*)\s+(?P<fields>.*)$"
)
UGEN_RESULT_EVENTS = {"ALLOC_GP_RESULT", "ALLOC_FP_RESULT"}
UGEN_LIFECYCLE_EVENTS = UGEN_RESULT_EVENTS | {"FREE", "FORCE_FREE"}
UGEN_KNOWN_EVENTS = UGEN_LIFECYCLE_EVENTS | {
    "ADD",
    "ALLOC",
    "ALLOC_GP",
    "ALLOC_FP",
    "MOVE_END",
    "REMOVE",
}
STACK_WIDTHS = {1, 2, 4, 8, 16}
STACK_ACCESS_CLASSES = {
    "load",
    "store",
    "load-store",
    "read",
    "write",
    "read-write",
    "outgoing-argument",
    "spill",
}
TARGET_TOP_LEVEL_KEYS = {
    "schema",
    "symbol",
    "frame_size_bytes",
    "stack_homes",
    "temp_events",
}

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


def _load_workbench_allocator() -> tuple[Any, Any]:
    """Load the workbench's calibrated globalcolor and stack-home readers."""

    _load_workbench_parsers()
    try:
        from decomp_workbench.allocator_analysis import stack_home_report
        from decomp_workbench.globalcolor import parse_globalcolor_trace
    except ImportError as error:
        raise ReceiptError(
            "decomp-workbench allocator readers are unavailable; refresh the "
            "project environment before capturing allocator evidence"
        ) from error
    return parse_globalcolor_trace, stack_home_report


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
        raise ReceiptError("could not decode retained uopt Ucode") from error
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


def _trace_integer(
    fields: dict[str, str], key: str, *, required: bool = False
) -> int | None:
    value = fields.get(key)
    if value is None:
        if required:
            raise ReceiptError(f"ugen trace row lacks required {key}")
        return None
    try:
        result = int(value, 0)
    except ValueError as error:
        raise ReceiptError(f"ugen trace contains malformed {key}") from error
    return result


def _structured_fields(text: str, label: str) -> dict[str, str]:
    fields: dict[str, str] = {}
    for token in text.split():
        match = re.fullmatch(r"([A-Za-z0-9_]+)=([^\s]+)", token)
        if match is None:
            raise ReceiptError(f"{label} contains a malformed structured field")
        key, value = match.groups()
        if key in fields:
            raise ReceiptError(f"{label} contains a duplicate structured field")
        fields[key] = value
    if not fields:
        raise ReceiptError(f"{label} contains no structured fields")
    return fields


def parse_ugen_events(text: str, *, procedure: str) -> list[dict[str, Any]]:
    """Parse public-safe temp lifecycle events from one complete ugen trace."""

    rows: list[dict[str, Any]] = []
    result_count = 0
    for trace_line, raw in enumerate(text.splitlines(), 1):
        stripped = raw.strip()
        if not stripped.startswith("DKWB-FREELIST"):
            continue
        match = UGEN_ROW_RE.fullmatch(stripped)
        if match is None:
            raise ReceiptError("ugen trace contains a malformed DKWB-FREELIST row")
        event = match.group("event")
        if event not in UGEN_KNOWN_EVENTS:
            raise ReceiptError(f"ugen trace contains unsupported event {event!r}")
        fields = _structured_fields(match.group("fields"), "ugen trace row")
        register = _trace_integer(fields, "reg", required=True)
        emitted = _trace_integer(fields, "emitted")
        source_line = _trace_integer(fields, "line")
        assert register is not None
        if register < 0 or register > 255:
            raise ReceiptError("ugen trace register is outside the producer range")
        if emitted is not None and emitted < -1:
            raise ReceiptError("ugen trace emitted ordinal is outside the producer range")
        if source_line is not None and source_line < -1:
            raise ReceiptError("ugen trace source line is outside the producer range")
        if event not in UGEN_LIFECYCLE_EVENTS:
            continue
        if event in UGEN_RESULT_EVENTS:
            result_count += 1
            fp = event == "ALLOC_FP_RESULT"
            lifecycle = "birth"
            ring_action = "pop"
            temp_class = "floating-point" if fp else "integer"
        else:
            fp = register >= 32
            lifecycle = "death"
            ring_action = "push"
            temp_class = "floating-point" if fp else "integer"
        rows.append(
            {
                "ordinal": len(rows),
                "temp_class": temp_class,
                "lifecycle": lifecycle,
                "ring_action": ring_action,
                "register": register_name(register, fp=fp),
                "source": {
                    "procedure": procedure,
                    "line": None if source_line in {None, -1} else source_line,
                },
                # Kept only for validation and deterministic ordering. It is a
                # pass ordinal, not an object row or an address, and is removed
                # from the public receipt below.
                "_emitted": emitted,
                "_trace_line": trace_line,
            }
        )
    if result_count == 0:
        raise ReceiptError(
            "ugen trace has no ALLOC_GP_RESULT/ALLOC_FP_RESULT rows; the result "
            "hooks did not fire or this is not the instrumented textual trace"
        )
    return rows


def _public_temp_event(row: dict[str, Any]) -> dict[str, Any]:
    return {key: value for key, value in row.items() if not key.startswith("_")}


def summarize_ugen_results(text: str, *, procedure: str = "unavailable") -> dict[str, Any]:
    events = parse_ugen_events(text, procedure=procedure)
    result: dict[str, Any] = {}
    for key, temp_class in (
        ("integer_temps", "integer"),
        ("fp_temps", "floating-point"),
    ):
        selected = [
            row
            for row in events
            if row["temp_class"] == temp_class and row["lifecycle"] == "birth"
        ]
        registers = [row["register"] for row in selected]
        result[key] = {
            "allocations": len(registers),
            "registers": dict(sorted(Counter(registers).items())),
            "sequence_digest": hashlib.sha256("\n".join(registers).encode()).hexdigest(),
        }
    result["events"] = [_public_temp_event(row) for row in events]
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
        raise ReceiptError("could not decode candidate ELF") from error
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
        raise ReceiptError("fidelity gate did not emit valid JSON") from error
    if not isinstance(report, dict):
        raise ReceiptError("fidelity gate emitted an unsupported report")
    gates = report.get("gates")
    gates_pass = (
        isinstance(gates, dict)
        and bool(gates)
        and all(value is True for value in gates.values())
    )
    if process.returncode != 0 or report.get("pass") is not True or not gates_pass:
        failed_count = (
            sum(value is not True for value in gates.values())
            if isinstance(gates, dict)
            else 0
        )
        suffix = f" ({failed_count} failed gates)" if failed_count else ""
        raise ReceiptError(f"instrumentation fidelity gate failed{suffix}")
    return report


def compact_fidelity(report: dict[str, Any]) -> dict[str, Any]:
    gates = report.get("gates")
    if not isinstance(gates, dict) or any(
        not isinstance(name, str)
        or re.fullmatch(r"[A-Za-z0-9_.-]+", name) is None
        or not isinstance(passed, bool)
        for name, passed in gates.items()
    ):
        raise ReceiptError("fidelity gate contains malformed gate results")
    if not gates or any(passed is not True for passed in gates.values()):
        raise ReceiptError("fidelity gate did not prove every object field")
    if not isinstance(report.get("file_identical"), bool):
        raise ReceiptError("fidelity gate contains malformed identity status")

    def digest(section: str) -> str:
        row = report.get(section)
        value = row.get("file_sha256") if isinstance(row, dict) else None
        if not isinstance(value, str) or re.fullmatch(r"[0-9a-f]{64}", value) is None:
            raise ReceiptError("fidelity gate contains a malformed object digest")
        return value

    return {
        "pass": True,
        "gates": dict(sorted(gates.items())),
        "file_identical": report["file_identical"],
        "stock_sha256": digest("stock"),
        "traced_sha256": digest("instrumented"),
    }


def unavailable(reason: str) -> dict[str, Any]:
    return {"status": "unavailable", "reason": reason}


def _optional_nonnegative_integer(value: Any, label: str) -> int | None:
    if value is None:
        return None
    if not isinstance(value, int) or isinstance(value, bool) or value < 0:
        raise ReceiptError(f"{label} must be a nonnegative integer or null")
    return value


def _read_json_document(path_value: str, label: str) -> tuple[dict[str, Any], str]:
    path = Path(path_value)
    if not path.is_file():
        raise ReceiptError(f"{label} does not exist")
    try:
        raw = path.read_bytes()
        payload = json.loads(raw)
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ReceiptError(f"{label} is malformed or truncated") from error
    if not isinstance(payload, dict):
        raise ReceiptError(f"{label} must be one JSON object")
    return payload, hashlib.sha256(raw).hexdigest()


def load_workbench_summary(
    path_value: str, *, symbol: str, candidate_sha256: str
) -> dict[str, Any]:
    payload, digest = _read_json_document(path_value, "workbench summary")
    if payload.get("schema") != WORKBENCH_SUMMARY_SCHEMA:
        raise ReceiptError("workbench summary has an unsupported schema")
    symbols = payload.get("symbol")
    comparison = payload.get("comparison")
    provenance = payload.get("provenance")
    if not all(isinstance(row, dict) for row in (symbols, comparison, provenance)):
        raise ReceiptError("workbench summary lacks structured evidence sections")
    assert isinstance(symbols, dict)
    assert isinstance(comparison, dict)
    assert isinstance(provenance, dict)
    if symbols.get("candidate") != symbol or symbols.get("requested") != symbol:
        raise ReceiptError("workbench summary names a different candidate symbol")
    if provenance.get("candidate_object_sha256") != candidate_sha256:
        raise ReceiptError("workbench summary candidate digest is stale or conflicting")
    target = _optional_nonnegative_integer(
        comparison.get("target_frame_bytes"), "target frame size"
    )
    candidate = _optional_nonnegative_integer(
        comparison.get("candidate_frame_bytes"), "candidate frame size"
    )
    return {
        "status": "available" if candidate is not None else "unavailable",
        "candidate_bytes": candidate,
        "target_bytes": target,
        "evidence": "hash-bound-workbench-summary",
        "summary_sha256": digest,
    }


def _safe_source_file(value: Any) -> str | None:
    if not isinstance(value, str) or not value:
        return None
    name = PurePath(value.replace("\\", "/")).name
    if not re.fullmatch(r"[A-Za-z0-9_.+-]+", name):
        return None
    return name


def _detail_rows(text: str, proc: int) -> dict[tuple[str, int], dict[str, str]]:
    rows: dict[tuple[str, int], dict[str, str]] = {}
    for raw in text.splitlines():
        stripped = raw.strip()
        if not stripped.startswith("[CDX]"):
            continue
        match = CDX_RE.fullmatch(stripped)
        if match is None:
            raise ReceiptError("uopt detail trace contains a malformed CDX row")
        fields = _structured_fields(match.group("fields"), "uopt CDX row")
        if match.group("event") != "webdetail":
            continue
        try:
            row_proc = int(fields.get("proc", ""), 0)
            web = int(fields.get("web", ""), 0)
        except ValueError as error:
            raise ReceiptError("uopt webdetail row has malformed procedure or web") from error
        if row_proc != proc:
            continue
        phase = fields.get("phase", "*")
        if phase not in {"p1", "p2", "*"}:
            raise ReceiptError("uopt webdetail row has an unsupported allocator phase")
        key = (phase, web)
        previous = rows.get(key)
        if previous is not None and previous != fields:
            raise ReceiptError("uopt detail trace has conflicting webdetail rows")
        rows[key] = fields
    return rows


def _detail_width(fields: dict[str, str]) -> int | None:
    values: set[int] = set()
    for key in ("width", "bytes", "size"):
        value = fields.get(key)
        if value is None:
            continue
        try:
            values.add(int(value, 0))
        except ValueError as error:
            raise ReceiptError("uopt stack-home width is malformed") from error
    if len(values) > 1:
        raise ReceiptError("uopt stack-home width fields conflict")
    if values and next(iter(values)) not in STACK_WIDTHS:
        raise ReceiptError("uopt stack-home width is unsupported")
    return next(iter(values)) if values else None


def _detail_access(fields: dict[str, str]) -> str | None:
    values = {
        fields[key].casefold().replace("_", "-")
        for key in ("access", "access_class")
        if fields.get(key)
    }
    if len(values) > 1:
        raise ReceiptError("uopt stack-home access fields conflict")
    if values and next(iter(values)) not in STACK_ACCESS_CLASSES:
        raise ReceiptError("uopt stack-home access class is unsupported")
    return next(iter(values)) if values else None


def summarize_stack_homes(
    text: str, *, procedure: int, symbol: str
) -> dict[str, Any]:
    parse_globalcolor_trace, stack_home_report = _load_workbench_allocator()
    try:
        report = stack_home_report(
            parse_globalcolor_trace(text), proc=procedure
        )
    except (KeyError, TypeError, ValueError) as error:
        raise ReceiptError("uopt allocator trace is unsupported or malformed") from error
    if report.get("capture_status") != "ready":
        return unavailable(str(report.get("capture_status", "no-stack-home-evidence")))
    details = _detail_rows(text, procedure)
    homes: list[dict[str, Any]] = []
    for home in report.get("homes", []):
        if not isinstance(home, dict):
            raise ReceiptError("workbench stack-home report is malformed")
        phase = home.get("phase")
        web = home.get("trace_local_web")
        if phase not in {"p1", "p2"} or not isinstance(web, int):
            raise ReceiptError("workbench stack-home identity is malformed")
        fields = details.get((phase, web), details.get(("*", web), {}))
        final_offset = home.get("final_offset")
        virtual_offset = home.get("virtual_offset")
        if final_offset is not None:
            offset = final_offset
            basis = "final-frame"
        elif isinstance(virtual_offset, int):
            offset = virtual_offset
            basis = "virtual-home"
        else:
            raise ReceiptError("stack-home report lacks an explicit offset")
        if not isinstance(offset, int):
            raise ReceiptError("stack-home report contains a malformed offset")
        source = home.get("source") if isinstance(home.get("source"), dict) else {}
        source_line = None
        if source.get("line") is not None:
            try:
                source_line = int(source["line"], 0)
            except (TypeError, ValueError) as error:
                raise ReceiptError("stack-home source line is malformed") from error
            if source_line < 0:
                source_line = None
        homes.append(
            {
                "offset": offset,
                "offset_basis": basis,
                "width_bytes": _detail_width(fields),
                "access_class": _detail_access(fields) or "unavailable",
                "kind": str(home.get("kind", "unavailable")),
                "source": {
                    "procedure": symbol,
                    "file": _safe_source_file(source.get("file")),
                    "line": source_line,
                },
            }
        )
    homes.sort(
        key=lambda row: (
            row["offset"],
            row["offset_basis"],
            row["kind"],
            row["source"]["line"] if row["source"]["line"] is not None else -1,
        )
    )
    return {"status": "available", "homes": homes}


def _validate_target_stack_homes(value: Any) -> dict[str, Any]:
    if value is None:
        return unavailable("target stack-home evidence was not provided")
    if not isinstance(value, dict) or value.get("status") not in {"available", "unavailable"}:
        raise ReceiptError("target stack-home evidence is malformed")
    if set(value) - {"status", "homes"}:
        raise ReceiptError("target stack-home evidence contains unsupported fields")
    if value["status"] == "unavailable":
        return unavailable("target stack-home evidence is unavailable")
    entries = value.get("homes")
    if not isinstance(entries, list):
        raise ReceiptError("target stack-home evidence lacks a homes list")
    homes: list[dict[str, Any]] = []
    for entry in entries:
        if not isinstance(entry, dict) or set(entry) - {
            "offset", "width_bytes", "access_class"
        }:
            raise ReceiptError("target stack-home entry is malformed")
        offset = entry.get("offset")
        if not isinstance(offset, int) or isinstance(offset, bool):
            raise ReceiptError("target stack-home offset must be an integer")
        width = entry.get("width_bytes")
        if width is not None and width not in STACK_WIDTHS:
            raise ReceiptError("target stack-home width is unsupported")
        access = entry.get("access_class", "unavailable")
        if access != "unavailable" and access not in STACK_ACCESS_CLASSES:
            raise ReceiptError("target stack-home access class is unsupported")
        homes.append(
            {"offset": offset, "width_bytes": width, "access_class": access}
        )
    homes.sort(key=lambda row: (row["offset"], row["width_bytes"] or -1, row["access_class"]))
    return {"status": "available", "homes": homes}


def _validate_target_temp_events(value: Any, *, symbol: str) -> dict[str, Any]:
    if value is None:
        return unavailable("target temp-event evidence was not provided")
    if not isinstance(value, dict) or value.get("status") not in {"available", "unavailable"}:
        raise ReceiptError("target temp-event evidence is malformed")
    if set(value) - {"status", "events"}:
        raise ReceiptError("target temp-event evidence contains unsupported fields")
    if value["status"] == "unavailable":
        return unavailable("target temp-event evidence is unavailable")
    entries = value.get("events")
    if not isinstance(entries, list):
        raise ReceiptError("target temp-event evidence lacks an events list")
    events: list[dict[str, Any]] = []
    for ordinal, entry in enumerate(entries):
        if not isinstance(entry, dict) or set(entry) - {
            "temp_class", "lifecycle", "source_line"
        }:
            raise ReceiptError("target temp-event entry is malformed")
        temp_class = entry.get("temp_class")
        lifecycle = entry.get("lifecycle")
        source_line = entry.get("source_line")
        if temp_class not in {"integer", "floating-point"}:
            raise ReceiptError("target temp-event class is unsupported")
        if lifecycle not in {"birth", "death"}:
            raise ReceiptError("target temp-event lifecycle is unsupported")
        if source_line is not None and (
            not isinstance(source_line, int)
            or isinstance(source_line, bool)
            or source_line < 0
        ):
            raise ReceiptError("target temp-event source line is malformed")
        events.append(
            {
                "ordinal": ordinal,
                "temp_class": temp_class,
                "lifecycle": lifecycle,
                "ring_action": "pop" if lifecycle == "birth" else "push",
                "source": {"procedure": symbol, "line": source_line},
            }
        )
    return {"status": "available", "events": events}


def load_target_evidence(path_value: str, *, symbol: str) -> dict[str, Any]:
    payload, digest = _read_json_document(path_value, "target allocator evidence")
    if payload.get("schema") != TARGET_EVIDENCE_SCHEMA:
        raise ReceiptError("target allocator evidence has an unsupported schema")
    if set(payload) - TARGET_TOP_LEVEL_KEYS:
        raise ReceiptError("target allocator evidence contains unsupported fields")
    if payload.get("symbol") != symbol:
        raise ReceiptError("target allocator evidence names a different symbol")
    frame = _optional_nonnegative_integer(
        payload.get("frame_size_bytes"), "target frame size"
    )
    return {
        "status": "available",
        "sha256": digest,
        "frame_size_bytes": frame,
        "stack_homes": _validate_target_stack_homes(payload.get("stack_homes")),
        "temp_events": _validate_target_temp_events(
            payload.get("temp_events"), symbol=symbol
        ),
    }


def _compare_frame(candidate: dict[str, Any], target: dict[str, Any]) -> dict[str, Any]:
    candidate_value = candidate.get("candidate_bytes")
    target_value = target.get("frame_size_bytes")
    if target_value is None:
        target_value = candidate.get("target_bytes")
    if candidate_value is None or target_value is None:
        return unavailable("candidate and target frame sizes are not both proved")
    return {
        "status": "equal" if candidate_value == target_value else "divergent",
        "candidate_bytes": candidate_value,
        "target_bytes": target_value,
    }


def _compare_stack_homes(candidate: dict[str, Any], target: dict[str, Any]) -> dict[str, Any]:
    if candidate.get("status") != "available" or target.get("status") != "available":
        return unavailable("candidate and target stack homes are not both available")
    candidate_homes = candidate["homes"]
    target_homes = target["homes"]
    candidate_offsets = [row["offset"] for row in candidate_homes]
    target_offsets = [row["offset"] for row in target_homes]
    if candidate_offsets != target_offsets:
        index = next(
            (
                i
                for i, pair in enumerate(zip(target_offsets, candidate_offsets))
                if pair[0] != pair[1]
            ),
            min(len(target_offsets), len(candidate_offsets)),
        )
        return {
            "status": "divergent",
            "mechanism": (
                "stack-home-count" if len(target_offsets) != len(candidate_offsets)
                else "stack-home-displacement"
            ),
            "index": index,
            "target_offset": target_offsets[index] if index < len(target_offsets) else None,
            "candidate_offset": (
                candidate_offsets[index] if index < len(candidate_offsets) else None
            ),
        }
    unavailable_fields: set[str] = set()
    for index, (expected, actual) in enumerate(zip(target_homes, candidate_homes)):
        for field in ("width_bytes", "access_class"):
            left = expected.get(field)
            right = actual.get(field)
            if left in {None, "unavailable"} or right in {None, "unavailable"}:
                unavailable_fields.add(field)
            elif left != right:
                return {
                    "status": "divergent",
                    "mechanism": f"stack-home-{field.replace('_', '-')}",
                    "index": index,
                    "target": left,
                    "candidate": right,
                }
    return {
        "status": "equal" if not unavailable_fields else "partial",
        "home_count": len(candidate_homes),
        "unavailable_fields": sorted(unavailable_fields),
    }


def _temp_key(row: dict[str, Any]) -> tuple[Any, ...]:
    return row.get("temp_class"), row.get("lifecycle")


def _compare_temp_events(candidate: dict[str, Any], target: dict[str, Any]) -> dict[str, Any]:
    if candidate.get("status") != "available" or target.get("status") != "available":
        return unavailable("candidate and target temp events are not both available")
    candidate_events = candidate["events"]
    target_events = target["events"]
    matcher = difflib.SequenceMatcher(
        a=[_temp_key(row) for row in target_events],
        b=[_temp_key(row) for row in candidate_events],
        autojunk=False,
    )
    for tag, target_start, target_end, candidate_start, candidate_end in matcher.get_opcodes():
        if tag == "equal":
            continue
        target_row = target_events[target_start] if target_start < target_end else None
        candidate_row = (
            candidate_events[candidate_start] if candidate_start < candidate_end else None
        )
        if tag == "insert" and candidate_row is not None:
            mechanism = f"extra-temp-{candidate_row['lifecycle']}"
        elif tag == "delete" and target_row is not None:
            mechanism = f"missing-temp-{target_row['lifecycle']}"
        else:
            mechanism = "temp-event-order-or-attribution"
        return {
            "status": "divergent",
            "mechanism": mechanism,
            "target_ordinal": target_start if target_row is not None else None,
            "candidate_ordinal": candidate_start if candidate_row is not None else None,
            "target_event": target_row,
            "candidate_event": candidate_row,
        }
    source_line_unavailable = False
    for ordinal, (target_row, candidate_row) in enumerate(
        zip(target_events, candidate_events)
    ):
        target_line = target_row.get("source", {}).get("line")
        candidate_line = candidate_row.get("source", {}).get("line")
        if target_line is None or candidate_line is None:
            source_line_unavailable = True
        elif target_line != candidate_line:
            return {
                "status": "divergent",
                "mechanism": "temp-source-line",
                "target_ordinal": ordinal,
                "candidate_ordinal": ordinal,
                "target_line": target_line,
                "candidate_line": candidate_line,
            }
    return {
        "status": "partial" if source_line_unavailable else "equal",
        "event_count": len(candidate_events),
        "unavailable_fields": ["source_line"] if source_line_unavailable else [],
    }


def _next_lever(mechanism: str) -> str:
    if mechanism == "frame-size":
        return "restore the target frame through a natural declaration, scope, or lifetime shape"
    if mechanism.startswith("stack-home"):
        return "adjust declaration order or the proved call-crossing lifetime for the first displaced home"
    if mechanism == "extra-temp-birth":
        return "inspect the attributed expression for a redundant conversion, comparison carrier, or grouping that advances the temp ring"
    if mechanism == "missing-temp-birth":
        return "restore the attributed expression boundary that creates the missing temporary"
    if mechanism.startswith("temp-") or mechanism.startswith("extra-temp") or mechanism.startswith("missing-temp"):
        return "change only the first attributed expression boundary, then recapture the allocator trace"
    return "capture narrower producer evidence for the first divergent allocator field"


def compare_trace_summary(
    *,
    frame: dict[str, Any],
    stack_homes: dict[str, Any],
    temp_events: dict[str, Any],
    target: dict[str, Any],
) -> dict[str, Any]:
    fields = {
        "frame": _compare_frame(frame, target),
        "stack_homes": _compare_stack_homes(stack_homes, target["stack_homes"]),
        "temp_events": _compare_temp_events(temp_events, target["temp_events"]),
    }
    frame_row = fields["frame"]
    if frame_row["status"] == "divergent":
        mechanism = "frame-size"
        evidence = frame_row
    else:
        evidence = next(
            (row for key in ("stack_homes", "temp_events") if (row := fields[key]).get("status") == "divergent"),
            None,
        )
        mechanism = evidence.get("mechanism") if evidence is not None else None
    if mechanism is not None and evidence is not None:
        first = {
            "status": "divergent",
            "mechanism": mechanism,
            "evidence": evidence,
            "next_lever": _next_lever(mechanism),
        }
    elif all(row.get("status") == "equal" for row in fields.values()):
        first = {
            "status": "no-divergence",
            "mechanism": None,
            "next_lever": None,
        }
    else:
        first = unavailable(
            "available fields show no divergence, but at least one comparison field is unavailable or partial"
        )
    return {"fields": fields, "first_divergence": first}


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

    workbench_summary_path = getattr(args, "workbench_summary", None)
    if workbench_summary_path:
        frame = load_workbench_summary(
            workbench_summary_path,
            symbol=args.symbol,
            candidate_sha256=mapping["baseline"]["candidate_sha256"],
        )
    else:
        frame = {
            **unavailable("hash-bound workbench frame evidence was not provided"),
            "candidate_bytes": None,
            "target_bytes": None,
        }

    stack_homes = summarize_stack_homes(
        uopt_text, procedure=procedure, symbol=args.symbol
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
            **summarize_ugen_results(ugen_text, procedure=args.symbol),
        }
        temp_events = {"status": "available", "events": ugen["events"]}
        ugen_limit = "none: retained Ucode contains one compiled procedure"
    else:
        ugen = {"status": "not-provided"}
        temp_events = unavailable("a procedure-scoped ugen trace was not provided")
        ugen_limit = (
            "ugen temp/FP result rows are not attributed because the current "
            "producer has no compiled-procedure marker"
        )

    target_path = getattr(args, "target_evidence", None)
    if target_path:
        target = load_target_evidence(target_path, symbol=args.symbol)
    else:
        target = {
            "status": "unavailable",
            "sha256": None,
            "frame_size_bytes": None,
            "stack_homes": unavailable("target stack-home evidence was not provided"),
            "temp_events": unavailable("target temp-event evidence was not provided"),
        }
    if (
        target["frame_size_bytes"] is not None
        and frame.get("target_bytes") is not None
        and target["frame_size_bytes"] != frame["target_bytes"]
    ):
        raise ReceiptError(
            "target allocator evidence conflicts with the workbench target frame"
        )
    comparison = compare_trace_summary(
        frame=frame,
        stack_homes=stack_homes,
        temp_events=temp_events,
        target=target,
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
        "trace_summary": {
            "frame": frame,
            "stack_homes": stack_homes,
            "temp_events": temp_events,
        },
        "target_evidence": {
            "status": target["status"],
            "sha256": target["sha256"],
        },
        "comparison": comparison,
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
    if (
        receipt["target_evidence"]["status"] == "available"
        or receipt["trace_summary"]["frame"].get("evidence")
    ):
        first = receipt["comparison"]["first_divergence"]
        if first["status"] == "divergent":
            lines.append(
                f"first-mechanism={first['mechanism']} next={first['next_lever']}"
            )
        else:
            lines.append(f"first-mechanism={first['status']}")
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
    command.add_argument(
        "--workbench-summary",
        help="optional hash-bound mickey-wb-summary-v1 frame evidence",
    )
    command.add_argument(
        "--target-evidence",
        help="optional compact mickey-allocator-target-evidence-v1 comparison input",
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


def redact_diagnostic(message: str) -> str:
    """Remove host paths from diagnostics while retaining the failed condition."""

    message = re.sub(
        r"(?<![A-Za-z0-9_.-])/(?:[^\s'\"():]+/?)+",
        "<redacted-path>",
        message,
    )
    return re.sub(
        r"(?<![A-Za-z0-9_.-])[A-Za-z]:\\(?:[^\s'\"():]+\\?)+",
        "<redacted-path>",
        message,
    )


def main(argv: Sequence[str] | None = None) -> int:
    args = parser().parse_args(argv)
    try:
        result = build_mapping(args) if args.map_only else build_receipt(args)
    except (OSError, ReceiptError, UnicodeError) as error:
        print(
            f"allocator-trace-receipt: error: {redact_diagnostic(str(error))}",
            file=sys.stderr,
        )
        return 2
    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
    else:
        print(render_mapping(result) if args.map_only else render_text(result))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
