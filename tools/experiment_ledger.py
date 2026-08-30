#!/usr/bin/env python3
"""Append-only, untracked matching-experiment metrics journal.

The journal intentionally accepts no instruction text or target words.  Its
default and every CLI-selected destination must remain below ``build/``.
"""

from __future__ import annotations

import argparse
from collections import Counter
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import stat
import sys
from typing import Any, Iterable, Sequence


SCHEMA_VERSION = 1
DEFAULT_LEDGER = Path("build/experiment-ledger.jsonl")
MAX_LEDGER_BYTES = 64 * 1024 * 1024
MAX_RECORD_BYTES = 4096
MAX_RECORDS = 250_000

SYMBOL_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]{0,127}$", re.ASCII)
C_KEYWORDS = frozenset(
    """
    auto break case char const continue default do double else enum extern float
    for goto if inline int long register restrict return short signed sizeof
    static struct switch typedef union unsigned void volatile while
    _Alignas _Alignof _Atomic _Bool _Complex _Generic _Imaginary _Noreturn
    _Static_assert _Thread_local
    """.split()
)
TIMESTAMP_RE = re.compile(
    r"^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}Z$",
    re.ASCII,
)
SHA256_RE = re.compile(r"^[0-9a-f]{64}$", re.ASCII)
MACHINE_WORD_RE = re.compile(
    r"(?<![A-Za-z0-9])(?:0x)?[0-9A-Fa-f]{8,128}(?![A-Za-z0-9])", re.ASCII
)
BYTE_ESCAPE_RE = re.compile(r"(?:\\x[0-9A-Fa-f]{2}){2,}", re.ASCII)
LONG_DECIMAL_RE = re.compile(r"(?<![A-Za-z0-9])[0-9]{7,}(?![A-Za-z0-9])", re.ASCII)
INSTRUCTION_SHAPE_RE = re.compile(
    r"(?:^|\s)(?:and|break|move|nor|not|or|xor)\s+"
    r"(?:\$[^\s,]+|[^\s,]+\s*,)",
    re.ASCII | re.IGNORECASE,
)
ABSOLUTE_PATH_TEXT_RE = re.compile(
    r"(?:^|[\s(])(?:/|~/|[A-Za-z]:[\\/])", re.ASCII
)

# Deliberately excludes ambiguous English words such as "move" and "break".
# Hypotheses should name the mechanism (association, lifetime, signedness),
# not quote the target or candidate assembly.
MIPS_MNEMONICS = frozenset(
    """
    add addi addiu addu dadd daddi daddiu daddu sub subu dsub dsubu
    slt sltu slti sltiu andi ori xori lui li la
    sll sllv sra srav srl srlv mult multu div divu mfhi mflo mthi mtlo
    lb lbu lh lhu lw lwl lwr ld ldl ldr sb sh sw swl swr sd sdl sdr ll sc
    beq beql bne bnel bgez bgezl bgezal bgtz bgtzl blez blezl bltz bltzl bltzal
    j jal jalr jr nop syscall sync cache
    add.s add.d sub.s sub.d mul.s mul.d div.s div.d sqrt.s sqrt.d
    abs.s abs.d neg.s neg.d mov.s mov.d cvt.s.w cvt.d.w cvt.w.s cvt.w.d
    trunc.w.s trunc.w.d round.w.s round.w.d ceil.w.s ceil.w.d
    floor.w.s floor.w.d c.eq.s c.eq.d c.lt.s c.lt.d c.le.s c.le.d
    bc1f bc1fl bc1t bc1tl lwc1 swc1 mfc1 mtc1
    """.split()
)

VERDICTS = frozenset(
    {"attempt", "improved", "plateau", "exact", "non_matching", "non_equivalent"}
)
VERDICT_TIEBREAK = {
    "exact": 0,
    "improved": 1,
    "attempt": 2,
    "plateau": 3,
    "non_matching": 4,
    "non_equivalent": 5,
}

RECORD_KEYS = frozenset(
    {
        "schema_version",
        "timestamp",
        "symbol",
        "source",
        "hypothesis",
        "candidate_words",
        "target_words",
        "raw_differences",
        "relocation_masked_differences",
        "frame",
        "candidate_relocations",
        "target_relocations",
        "relocation_identities",
        "first_raw_mismatch",
        "first_masked_mismatch",
        "verdict",
        "artifacts",
    }
)
REQUIRED_RECORD_KEYS = RECORD_KEYS - {"artifacts"}
ARTIFACT_KEYS = frozenset({"path", "sha256"})


class LedgerError(RuntimeError):
    """A fail-closed journal refusal."""


def project_root() -> Path:
    return Path(__file__).resolve().parents[1]


def utc_timestamp() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def _plain_int(value: Any, field: str, *, positive: bool = False) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise LedgerError(f"{field} must be an integer")
    floor = 1 if positive else 0
    if value < floor or value > 10_000_000:
        comparison = "positive" if positive else "non-negative"
        raise LedgerError(f"{field} must be a bounded {comparison} integer")
    return value


def validate_symbol(symbol: Any) -> str:
    if (
        not isinstance(symbol, str)
        or not SYMBOL_RE.fullmatch(symbol)
        or symbol in C_KEYWORDS
    ):
        raise LedgerError("symbol must be one ASCII C identifier of at most 128 characters")
    return symbol


def _relative_posix_path(value: Any, field: str, *, prefix: str, suffix: str | None = None) -> str:
    if not isinstance(value, str) or not value or len(value) > 240:
        raise LedgerError(f"{field} must be a concise relative path")
    if "\\" in value or any(ord(character) < 32 for character in value):
        raise LedgerError(f"{field} must use printable POSIX path syntax")
    path = PurePosixPath(value)
    if (
        path.is_absolute()
        or value.startswith("~")
        or path.as_posix() != value
        or any(part in ("", ".", "..") for part in path.parts)
    ):
        raise LedgerError(
            f"{field} must be normalized and relative; absolute/traversal paths are prohibited"
        )
    if not path.parts or path.parts[0] != prefix:
        raise LedgerError(f"{field} must be below {prefix}/")
    if suffix is not None and path.suffix != suffix:
        raise LedgerError(f"{field} must name a {suffix} file")
    return path.as_posix()


def validate_source(value: Any) -> str:
    return _relative_posix_path(value, "source", prefix="src", suffix=".c")


def validate_hypothesis(value: Any) -> str:
    if not isinstance(value, str):
        raise LedgerError("hypothesis must be text")
    if not value or len(value) > 160:
        raise LedgerError("hypothesis must contain 1-160 characters")
    if value != value.strip() or "  " in value:
        raise LedgerError("hypothesis must be normalized one-line text")
    if any(ord(character) < 32 or ord(character) == 127 for character in value):
        raise LedgerError("hypothesis must be printable one-line text")
    if "/" in value or "\\" in value or ABSOLUTE_PATH_TEXT_RE.search(value):
        raise LedgerError(
            "paths are prohibited in hypothesis text; use the source/artifacts fields"
        )
    if (
        MACHINE_WORD_RE.search(value)
        or LONG_DECIMAL_RE.search(value)
        or BYTE_ESCAPE_RE.search(value)
        or "0x" in value.lower()
    ):
        raise LedgerError(
            "instruction words, byte strings, and hexadecimal payloads are prohibited"
        )
    if "$" in value or INSTRUCTION_SHAPE_RE.search(value):
        raise LedgerError("instruction-shaped operand text is prohibited in hypotheses")
    words = {token.lower() for token in re.findall(r"[A-Za-z][A-Za-z0-9_.]*", value)}
    blocked = sorted(words & MIPS_MNEMONICS)
    if blocked:
        raise LedgerError(
            "instruction text is prohibited in hypotheses; describe the mechanism without "
            "mnemonic(s): "
            + ", ".join(blocked)
        )
    return value


def validate_timestamp(value: Any) -> str:
    if not isinstance(value, str) or not TIMESTAMP_RE.fullmatch(value):
        raise LedgerError("timestamp must be canonical UTC YYYY-MM-DDTHH:MM:SSZ")
    try:
        parsed = datetime.strptime(value, "%Y-%m-%dT%H:%M:%SZ").replace(tzinfo=timezone.utc)
    except ValueError as error:
        raise LedgerError("timestamp is not a real UTC date/time") from error
    if parsed.strftime("%Y-%m-%dT%H:%M:%SZ") != value:
        raise LedgerError("timestamp must be canonical UTC YYYY-MM-DDTHH:MM:SSZ")
    return value


def validate_artifacts(value: Any) -> list[dict[str, str]]:
    if not isinstance(value, list) or not value or len(value) > 8:
        raise LedgerError("artifacts must contain 1-8 path/hash objects")
    checked: list[dict[str, str]] = []
    seen: set[str] = set()
    for index, artifact in enumerate(value):
        if not isinstance(artifact, dict) or set(artifact) != ARTIFACT_KEYS:
            raise LedgerError(f"artifacts[{index}] must contain only path and sha256")
        path = _relative_posix_path(artifact["path"], f"artifacts[{index}].path", prefix="build")
        digest = artifact["sha256"]
        if not isinstance(digest, str) or not SHA256_RE.fullmatch(digest):
            raise LedgerError(f"artifacts[{index}].sha256 must be 64 lowercase hexadecimal digits")
        if path in seen:
            raise LedgerError(f"duplicate artifact path: {path}")
        seen.add(path)
        checked.append({"path": path, "sha256": digest})
    return checked


def _validate_mismatch(value: Any, field: str, differences: int, maximum_words: int) -> int | None:
    if value is None:
        if differences:
            raise LedgerError(f"{field} is required when its difference count is nonzero")
        return None
    offset = _plain_int(value, field)
    if not differences:
        raise LedgerError(f"{field} must be null when its difference count is zero")
    if offset % 4 or offset >= maximum_words * 4:
        raise LedgerError(f"{field} must be an aligned byte offset within the compared words")
    return offset


def validate_record(record: Any) -> dict[str, Any]:
    if not isinstance(record, dict):
        raise LedgerError("each journal line must be one JSON object")
    keys = set(record)
    missing = REQUIRED_RECORD_KEYS - keys
    unknown = keys - RECORD_KEYS
    if missing or unknown:
        detail = []
        if missing:
            detail.append("missing " + ", ".join(sorted(missing)))
        if unknown:
            detail.append("unknown " + ", ".join(sorted(unknown)))
        raise LedgerError("record schema mismatch: " + "; ".join(detail))
    if type(record["schema_version"]) is not int or record["schema_version"] != SCHEMA_VERSION:
        raise LedgerError(f"schema_version must be {SCHEMA_VERSION}")

    timestamp = validate_timestamp(record["timestamp"])
    symbol = validate_symbol(record["symbol"])
    source = validate_source(record["source"])
    hypothesis = validate_hypothesis(record["hypothesis"])
    candidate_words = _plain_int(record["candidate_words"], "candidate_words", positive=True)
    target_words = _plain_int(record["target_words"], "target_words", positive=True)
    maximum_words = max(candidate_words, target_words)
    raw = _plain_int(record["raw_differences"], "raw_differences")
    masked = _plain_int(
        record["relocation_masked_differences"], "relocation_masked_differences"
    )
    if raw > maximum_words:
        raise LedgerError("raw_differences cannot exceed the longer instruction stream")
    if masked > raw:
        raise LedgerError("relocation_masked_differences cannot exceed raw_differences")
    frame = _plain_int(record["frame"], "frame")
    if frame % 8:
        raise LedgerError("frame must be a byte count aligned to 8 bytes; use 0 for frameless")

    candidate_relocations = _plain_int(record["candidate_relocations"], "candidate_relocations")
    target_relocations = _plain_int(record["target_relocations"], "target_relocations")
    identities = _plain_int(record["relocation_identities"], "relocation_identities")
    if identities > min(candidate_relocations, target_relocations):
        raise LedgerError("relocation_identities cannot exceed either relocation count")

    first_raw = _validate_mismatch(
        record["first_raw_mismatch"], "first_raw_mismatch", raw, maximum_words
    )
    first_masked = _validate_mismatch(
        record["first_masked_mismatch"],
        "first_masked_mismatch",
        masked,
        maximum_words,
    )
    verdict = record["verdict"]
    if not isinstance(verdict, str) or verdict not in VERDICTS:
        raise LedgerError("verdict must be one of: " + ", ".join(sorted(VERDICTS)))
    if verdict == "exact" and not (
        candidate_words == target_words
        and raw == 0
        and masked == 0
        and candidate_relocations == target_relocations == identities
    ):
        raise LedgerError(
            "exact verdict requires equal words, zero differences, and exact relocations"
        )

    checked: dict[str, Any] = {
        "schema_version": SCHEMA_VERSION,
        "timestamp": timestamp,
        "symbol": symbol,
        "source": source,
        "hypothesis": hypothesis,
        "candidate_words": candidate_words,
        "target_words": target_words,
        "raw_differences": raw,
        "relocation_masked_differences": masked,
        "frame": frame,
        "candidate_relocations": candidate_relocations,
        "target_relocations": target_relocations,
        "relocation_identities": identities,
        "first_raw_mismatch": first_raw,
        "first_masked_mismatch": first_masked,
        "verdict": verdict,
    }
    if "artifacts" in record:
        checked["artifacts"] = validate_artifacts(record["artifacts"])
    return checked


def require_source_ownership(root: Path, source: str, symbol: str) -> None:
    source_path = root / source
    try:
        resolved = source_path.resolve(strict=True)
        resolved.relative_to(root.resolve(strict=True))
    except (FileNotFoundError, ValueError) as error:
        raise LedgerError(f"source is not an existing file in this worktree: {source}") from error
    if not resolved.is_file() or source_path.is_symlink():
        raise LedgerError(f"source must be a regular tracked-tree file: {source}")
    if resolved.stat().st_size > 8 * 1024 * 1024:
        raise LedgerError("source is unexpectedly large")
    try:
        text = resolved.read_text(encoding="utf-8")
    except (OSError, UnicodeError) as error:
        raise LedgerError(f"source is not readable UTF-8 text: {source}") from error
    if not re.search(rf"(?<![A-Za-z0-9_]){re.escape(symbol)}(?![A-Za-z0-9_])", text):
        raise LedgerError(f"symbol {symbol} is not named by source {source}")


def resolve_ledger_path(root: Path, supplied: str | Path | None) -> Path:
    root = root.resolve(strict=True)
    raw_relative = str(supplied) if supplied is not None else DEFAULT_LEDGER.as_posix()
    relative = Path(raw_relative)
    if (
        relative.is_absolute()
        or PurePosixPath(raw_relative).as_posix() != raw_relative
        or ".." in relative.parts
        or "." in relative.parts
    ):
        raise LedgerError("ledger path must be normalized, relative, and below build/")
    if not relative.parts or relative.parts[0] != "build":
        raise LedgerError("ledger path must be below build/ so it remains untracked")
    if relative.suffix != ".jsonl":
        raise LedgerError("ledger path must end in .jsonl")
    destination = root / relative
    cursor = root
    for part in relative.parts:
        cursor = cursor / part
        if cursor.is_symlink():
            raise LedgerError("symlinks are prohibited in the ledger path")
    return destination


def _json_object_without_duplicates(text: str) -> Any:
    def pairs_hook(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
        result: dict[str, Any] = {}
        for key, value in pairs:
            if key in result:
                raise LedgerError(f"duplicate JSON key: {key}")
            result[key] = value
        return result

    try:
        return json.loads(text, object_pairs_hook=pairs_hook)
    except LedgerError:
        raise
    except json.JSONDecodeError as error:
        raise LedgerError(f"invalid JSON: {error.msg}") from error


def _decode_records(payload: bytes) -> list[dict[str, Any]]:
    if not payload:
        return []
    if len(payload) > MAX_LEDGER_BYTES:
        raise LedgerError("journal exceeds the bounded 64 MiB reader limit")
    if not payload.endswith(b"\n"):
        raise LedgerError("journal has a truncated final record")
    try:
        text = payload.decode("utf-8")
    except UnicodeDecodeError as error:
        raise LedgerError("journal is not UTF-8") from error
    lines = text.splitlines()
    if len(lines) > MAX_RECORDS:
        raise LedgerError("journal exceeds the bounded record-count limit")
    records: list[dict[str, Any]] = []
    for line_number, line in enumerate(lines, 1):
        if not line or len(line.encode("utf-8")) > MAX_RECORD_BYTES:
            raise LedgerError(f"line {line_number}: record is empty or oversized")
        try:
            records.append(validate_record(_json_object_without_duplicates(line)))
        except LedgerError as error:
            raise LedgerError(f"line {line_number}: {error}") from error
    return records


def _read_fd(fd: int) -> bytes:
    size = os.fstat(fd).st_size
    if size > MAX_LEDGER_BYTES:
        raise LedgerError("journal exceeds the bounded 64 MiB reader limit")
    os.lseek(fd, 0, os.SEEK_SET)
    chunks: list[bytes] = []
    remaining = size
    while remaining:
        chunk = os.read(fd, min(remaining, 1024 * 1024))
        if not chunk:
            raise LedgerError("journal changed while it was being read")
        chunks.append(chunk)
        remaining -= len(chunk)
    return b"".join(chunks)


def load_records(path: Path) -> list[dict[str, Any]]:
    if not path.exists():
        return []
    flags = os.O_RDONLY
    if hasattr(os, "O_NOFOLLOW"):
        flags |= os.O_NOFOLLOW
    try:
        fd = os.open(path, flags)
    except OSError as error:
        raise LedgerError(f"cannot open journal safely: {error}") from error
    try:
        fcntl.flock(fd, fcntl.LOCK_SH)
        metadata = os.fstat(fd)
        if not stat.S_ISREG(metadata.st_mode) or metadata.st_nlink != 1:
            raise LedgerError("journal must be one regular file with no hard links")
        return _decode_records(_read_fd(fd))
    finally:
        os.close(fd)


def append_record(path: Path, record: dict[str, Any]) -> dict[str, Any]:
    checked = validate_record(record)
    encoded = (json.dumps(checked, sort_keys=True, separators=(",", ":")) + "\n").encode("utf-8")
    if len(encoded) > MAX_RECORD_BYTES:
        raise LedgerError("record exceeds the 4096-byte compact-record limit")

    path.parent.mkdir(parents=True, exist_ok=True)
    if path.is_symlink():
        raise LedgerError("journal must not be a symlink")
    existed = path.exists()
    flags = os.O_RDWR | os.O_APPEND | os.O_CREAT
    if hasattr(os, "O_NOFOLLOW"):
        flags |= os.O_NOFOLLOW
    try:
        fd = os.open(path, flags, 0o600)
    except OSError as error:
        raise LedgerError(f"cannot open journal safely: {error}") from error
    try:
        fcntl.flock(fd, fcntl.LOCK_EX)
        metadata = os.fstat(fd)
        if not stat.S_ISREG(metadata.st_mode) or metadata.st_nlink != 1:
            raise LedgerError("journal must be one regular file with no hard links")
        payload = _read_fd(fd)
        _decode_records(payload)  # Refuse to extend a malformed or truncated journal.
        if len(payload) + len(encoded) > MAX_LEDGER_BYTES:
            raise LedgerError("append would exceed the bounded 64 MiB journal limit")
        written = os.write(fd, encoded)  # O_APPEND + one bounded write preserves prior bytes.
        if written != len(encoded):
            os.ftruncate(fd, len(payload))
            os.fsync(fd)
            raise LedgerError("short append was rolled back")
        os.fsync(fd)
    finally:
        os.close(fd)

    if not existed:
        directory_flags = os.O_RDONLY
        if hasattr(os, "O_DIRECTORY"):
            directory_flags |= os.O_DIRECTORY
        directory_fd = os.open(path.parent, directory_flags)
        try:
            os.fsync(directory_fd)
        finally:
            os.close(directory_fd)
    return checked


def best_records(
    records: Iterable[dict[str, Any]], symbol: str | None = None
) -> list[dict[str, Any]]:
    if symbol is not None:
        validate_symbol(symbol)
    grouped: dict[str, list[tuple[int, dict[str, Any]]]] = {}
    for index, record in enumerate(records):
        checked = validate_record(record)
        if symbol is None or checked["symbol"] == symbol:
            grouped.setdefault(checked["symbol"], []).append((index, checked))

    selected: list[dict[str, Any]] = []
    for name in sorted(grouped):
        _, record = min(
            grouped[name],
            key=lambda item: (
                item[1]["verdict"] != "exact",
                item[1]["relocation_masked_differences"],
                item[1]["raw_differences"],
                abs(item[1]["candidate_words"] - item[1]["target_words"]),
                max(0, item[1]["target_relocations"] - item[1]["relocation_identities"]),
                VERDICT_TIEBREAK[item[1]["verdict"]],
                -item[0],  # Equal evidence resolves to the latest immutable attempt.
            ),
        )
        selected.append(record)
    return selected


def summarize_records(
    records: Sequence[dict[str, Any]], symbol: str | None = None
) -> dict[str, Any]:
    if symbol is not None:
        validate_symbol(symbol)
    checked = [validate_record(record) for record in records]
    filtered = [record for record in checked if symbol is None or record["symbol"] == symbol]
    best = best_records(filtered)
    verdicts = Counter(record["verdict"] for record in filtered)
    return {
        "schema_version": SCHEMA_VERSION,
        "records": len(filtered),
        "symbols": len({record["symbol"] for record in filtered}),
        "exact_symbols": sum(record["verdict"] == "exact" for record in best),
        "verdicts": {name: verdicts[name] for name in sorted(verdicts)},
        "best_relocation_masked_differences": {
            record["symbol"]: record["relocation_masked_differences"] for record in best
        },
    }


def _parse_nonnegative(value: str) -> int:
    try:
        parsed = int(value, 0)
    except ValueError as error:
        raise argparse.ArgumentTypeError("expected a non-negative integer") from error
    if parsed < 0:
        raise argparse.ArgumentTypeError("expected a non-negative integer")
    return parsed


def _parse_frame(value: str) -> int:
    if value == "frameless":
        return 0
    return _parse_nonnegative(value)


def _parse_optional_offset(value: str) -> int | None:
    if value == "none":
        return None
    if value.startswith("+"):
        value = value[1:]
    return _parse_nonnegative(value)


def _parse_artifact(value: str) -> dict[str, str]:
    if "=" not in value:
        raise argparse.ArgumentTypeError("artifact must be RELATIVE_PATH=SHA256")
    path, digest = value.rsplit("=", 1)
    return {"path": path, "sha256": digest.lower()}


def _record_from_args(args: argparse.Namespace, root: Path) -> dict[str, Any]:
    source = validate_source(args.source)
    symbol = validate_symbol(args.symbol)
    require_source_ownership(root, source, symbol)
    record: dict[str, Any] = {
        "schema_version": SCHEMA_VERSION,
        "timestamp": utc_timestamp(),
        "symbol": symbol,
        "source": source,
        "hypothesis": args.hypothesis,
        "candidate_words": args.candidate_words,
        "target_words": args.target_words,
        "raw_differences": args.raw_differences,
        "relocation_masked_differences": args.masked_differences,
        "frame": args.frame,
        "candidate_relocations": args.candidate_relocations,
        "target_relocations": args.target_relocations,
        "relocation_identities": args.relocation_identities,
        "first_raw_mismatch": args.first_raw_mismatch,
        "first_masked_mismatch": args.first_masked_mismatch,
        "verdict": args.verdict,
    }
    if args.artifact:
        record["artifacts"] = args.artifact
    return validate_record(record)


def _print_records(records: Sequence[dict[str, Any]], as_json: bool) -> None:
    if as_json:
        print(json.dumps(records, sort_keys=True, indent=2))
        return
    for record in records:
        print(
            f"{record['timestamp']} {record['symbol']} {record['verdict']} "
            f"words={record['candidate_words']}/{record['target_words']} "
            f"diff={record['raw_differences']}/{record['relocation_masked_differences']} "
            f"frame={record['frame']} "
            f"reloc={record['candidate_relocations']}/{record['target_relocations']} "
            f"identities={record['relocation_identities']} "
            f"first={record['first_raw_mismatch']}/{record['first_masked_mismatch']} "
            f"source={record['source']} hypothesis={record['hypothesis']!r}"
        )


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--ledger",
        default=DEFAULT_LEDGER.as_posix(),
        help="relative JSONL path below build/ (default: %(default)s)",
    )
    commands = parser.add_subparsers(dest="command", required=True)

    append = commands.add_parser("append", help="validate and atomically append one record")
    append.add_argument("symbol")
    append.add_argument("--source", required=True)
    append.add_argument("--hypothesis", required=True)
    append.add_argument("--candidate-words", required=True, type=_parse_nonnegative)
    append.add_argument("--target-words", required=True, type=_parse_nonnegative)
    append.add_argument("--raw-differences", required=True, type=_parse_nonnegative)
    append.add_argument("--masked-differences", required=True, type=_parse_nonnegative)
    append.add_argument("--frame", required=True, type=_parse_frame)
    append.add_argument("--candidate-relocations", required=True, type=_parse_nonnegative)
    append.add_argument("--target-relocations", required=True, type=_parse_nonnegative)
    append.add_argument("--relocation-identities", required=True, type=_parse_nonnegative)
    append.add_argument("--first-raw-mismatch", required=True, type=_parse_optional_offset)
    append.add_argument("--first-masked-mismatch", required=True, type=_parse_optional_offset)
    append.add_argument("--verdict", required=True, choices=sorted(VERDICTS))
    append.add_argument(
        "--artifact", action="append", type=_parse_artifact, default=[], metavar="PATH=SHA256"
    )
    append.add_argument("--json", action="store_true", help="print the appended record as JSON")

    listing = commands.add_parser("list", help="list immutable records in append order")
    listing.add_argument("--symbol")
    listing.add_argument("--limit", type=_parse_nonnegative)
    listing.add_argument("--json", action="store_true")

    best = commands.add_parser("best", help="show the strongest record per symbol")
    best.add_argument("symbol", nargs="?")
    best.add_argument("--json", action="store_true")

    summary = commands.add_parser("summarize", help="summarize journal counts and best evidence")
    summary.add_argument("--symbol")
    summary.add_argument("--json", action="store_true")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    root = project_root()
    try:
        ledger = resolve_ledger_path(root, args.ledger)
        if args.command == "append":
            record = append_record(ledger, _record_from_args(args, root))
            _print_records([record], args.json)
            return 0

        records = load_records(ledger)
        if args.command == "list":
            if args.symbol is not None:
                symbol = validate_symbol(args.symbol)
                records = [record for record in records if record["symbol"] == symbol]
            if args.limit is not None:
                records = records[-args.limit :] if args.limit else []
            _print_records(records, args.json)
            return 0
        if args.command == "best":
            selected = best_records(records, args.symbol)
            if args.symbol is not None and not selected:
                raise LedgerError(f"no records for symbol {args.symbol}")
            _print_records(selected, args.json)
            return 0
        if args.command == "summarize":
            summary = summarize_records(records, args.symbol)
            if args.json:
                print(json.dumps(summary, sort_keys=True, indent=2))
            else:
                print(
                    f"schema={summary['schema_version']} records={summary['records']} "
                    f"symbols={summary['symbols']} exact_symbols={summary['exact_symbols']}"
                )
                verdicts = " ".join(
                    f"{name}={count}" for name, count in summary["verdicts"].items()
                )
                if verdicts:
                    print("verdicts " + verdicts)
                for name, differences in summary["best_relocation_masked_differences"].items():
                    print(f"best {name} masked_differences={differences}")
            return 0
        raise LedgerError(f"unsupported command: {args.command}")
    except LedgerError as error:
        print(f"experiment-ledger: refusal: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
