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
import struct
import sys
from typing import Any, Iterable, Mapping, Sequence


LEGACY_SCHEMA_VERSION = 1
SCHEMA_VERSION = 2
SUPPORTED_SCHEMA_VERSIONS = frozenset({LEGACY_SCHEMA_VERSION, SCHEMA_VERSION})
DEFAULT_LEDGER = Path("build/experiment-ledger.jsonl")
MAX_LEDGER_BYTES = 64 * 1024 * 1024
MAX_RECORD_BYTES = 4096
MAX_RECORDS = 250_000
MAX_PREFLIGHT_BYTES = 4 * 1024 * 1024
MAX_ARTIFACT_BYTES = 256 * 1024 * 1024
PREFLIGHT_SCHEMA = "mickey-function-evidence-preflight-v1"
FINGERPRINT_ALGORITHM = "mips-elf-function-v1"
GENERATED_BUILD_ROOTS = frozenset({"build", "build_non_matching"})
UNSET = object()

ELF_MAGIC = b"\x7fELF"
ELFCLASS32 = 1
ELFDATA2MSB = 2
ET_REL = 1
EM_MIPS = 8
SHT_PROGBITS = 1
SHT_STRTAB = 3
SHT_RELA = 4
SHT_REL = 9
SHT_SYMTAB = 2
SHN_UNDEF = 0
STT_FUNC = 2
STT_SECTION = 3
R_MIPS_32 = 2
R_MIPS_26 = 4
R_MIPS_HI16 = 5
R_MIPS_LO16 = 6
SUPPORTED_FINGERPRINT_RELOCATIONS = frozenset(
    {R_MIPS_32, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16}
)

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

LEGACY_RECORD_KEYS = frozenset(
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
RECORD_KEYS = LEGACY_RECORD_KEYS | {"candidate_fingerprint"}
LEGACY_REQUIRED_RECORD_KEYS = LEGACY_RECORD_KEYS - {"artifacts"}
REQUIRED_RECORD_KEYS = RECORD_KEYS - {"artifacts"}
ARTIFACT_KEYS = frozenset({"path", "sha256"})
FINGERPRINT_KEYS = frozenset({"algorithm", "sha256", "size", "relocations"})


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
        path = _relative_generated_path(artifact["path"], f"artifacts[{index}].path")
        digest = artifact["sha256"]
        if not isinstance(digest, str) or not SHA256_RE.fullmatch(digest):
            raise LedgerError(f"artifacts[{index}].sha256 must be 64 lowercase hexadecimal digits")
        if path in seen:
            raise LedgerError(f"duplicate artifact path: {path}")
        seen.add(path)
        checked.append({"path": path, "sha256": digest})
    return checked


def validate_candidate_fingerprint(value: Any) -> dict[str, Any]:
    if not isinstance(value, dict) or set(value) != FINGERPRINT_KEYS:
        raise LedgerError(
            "candidate_fingerprint must contain only algorithm, sha256, size, and relocations"
        )
    if value.get("algorithm") != FINGERPRINT_ALGORITHM:
        raise LedgerError(
            f"candidate_fingerprint.algorithm must be {FINGERPRINT_ALGORITHM!r}"
        )
    digest = value.get("sha256")
    if not isinstance(digest, str) or not SHA256_RE.fullmatch(digest):
        raise LedgerError(
            "candidate_fingerprint.sha256 must be 64 lowercase hexadecimal digits"
        )
    size = _plain_int(value.get("size"), "candidate_fingerprint.size", positive=True)
    if size % 4:
        raise LedgerError("candidate_fingerprint.size must be aligned to four bytes")
    relocations = _plain_int(
        value.get("relocations"), "candidate_fingerprint.relocations"
    )
    return {
        "algorithm": FINGERPRINT_ALGORITHM,
        "sha256": digest,
        "size": size,
        "relocations": relocations,
    }


def _relative_generated_path(value: Any, field: str, *, suffix: str | None = None) -> str:
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
    if not path.parts or path.parts[0] not in GENERATED_BUILD_ROOTS:
        allowed = " or ".join(f"{name}/" for name in sorted(GENERATED_BUILD_ROOTS))
        raise LedgerError(f"{field} must be below {allowed}")
    if suffix is not None and path.suffix != suffix:
        raise LedgerError(f"{field} must name a {suffix} file")
    return path.as_posix()


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
    version = record.get("schema_version")
    if type(version) is not int or version not in SUPPORTED_SCHEMA_VERSIONS:
        supported = ", ".join(str(item) for item in sorted(SUPPORTED_SCHEMA_VERSIONS))
        raise LedgerError(f"schema_version must be one of: {supported}")
    expected_keys = RECORD_KEYS if version == SCHEMA_VERSION else LEGACY_RECORD_KEYS
    required_keys = (
        REQUIRED_RECORD_KEYS
        if version == SCHEMA_VERSION
        else LEGACY_REQUIRED_RECORD_KEYS
    )
    keys = set(record)
    missing = required_keys - keys
    unknown = keys - expected_keys
    if missing or unknown:
        detail = []
        if missing:
            detail.append("missing " + ", ".join(sorted(missing)))
        if unknown:
            detail.append("unknown " + ", ".join(sorted(unknown)))
        raise LedgerError("record schema mismatch: " + "; ".join(detail))
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
        "schema_version": version,
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
    if version == SCHEMA_VERSION:
        fingerprint = validate_candidate_fingerprint(record["candidate_fingerprint"])
        if fingerprint["size"] != candidate_words * 4:
            raise LedgerError(
                "candidate_fingerprint.size must equal candidate_words multiplied by four"
            )
        if fingerprint["relocations"] != candidate_relocations:
            raise LedgerError(
                "candidate_fingerprint.relocations must equal candidate_relocations"
            )
        checked["candidate_fingerprint"] = fingerprint
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


def _safe_regular_fd(root: Path, relative: str, label: str) -> int:
    root = root.resolve(strict=True)
    path = root / relative
    cursor = root
    for part in PurePosixPath(relative).parts:
        cursor = cursor / part
        if cursor.is_symlink():
            raise LedgerError(f"symlinks are prohibited in the {label} path")
    flags = os.O_RDONLY
    if hasattr(os, "O_NOFOLLOW"):
        flags |= os.O_NOFOLLOW
    try:
        fd = os.open(path, flags)
    except OSError as error:
        raise LedgerError(f"cannot open {label} safely: {error}") from error
    metadata = os.fstat(fd)
    if not stat.S_ISREG(metadata.st_mode):
        os.close(fd)
        raise LedgerError(f"{label} must be a regular file")
    return fd


def _load_preflight_report_with_mtime(
    root: Path, supplied: str | Path
) -> tuple[dict[str, Any], int]:
    relative = _relative_posix_path(
        str(supplied), "preflight JSON", prefix="build", suffix=".json"
    )
    fd = _safe_regular_fd(root, relative, "preflight JSON")
    try:
        before = os.fstat(fd)
        if before.st_size <= 0 or before.st_size > MAX_PREFLIGHT_BYTES:
            raise LedgerError("preflight JSON must be a non-empty file no larger than 4 MiB")
        payload = _read_fd(fd)
        after = os.fstat(fd)
        if (before.st_size, before.st_mtime_ns, before.st_ino) != (
            after.st_size,
            after.st_mtime_ns,
            after.st_ino,
        ):
            raise LedgerError("preflight JSON changed while it was being read")
    finally:
        os.close(fd)
    try:
        text = payload.decode("utf-8")
    except UnicodeDecodeError as error:
        raise LedgerError("preflight JSON is not UTF-8") from error
    report = _json_object_without_duplicates(text)
    if not isinstance(report, dict) or report.get("schema") != PREFLIGHT_SCHEMA:
        schema = report.get("schema") if isinstance(report, dict) else None
        raise LedgerError(f"unexpected preflight report schema: {schema!r}")
    return report, before.st_mtime_ns


def load_preflight_report(root: Path, supplied: str | Path) -> dict[str, Any]:
    report, _mtime_ns = _load_preflight_report_with_mtime(root, supplied)
    return report


def _bounded_slice(data: bytes, offset: int, size: int, label: str) -> bytes:
    if offset < 0 or size < 0 or offset > len(data) or size > len(data) - offset:
        raise LedgerError(f"candidate object has an out-of-bounds {label}")
    return data[offset : offset + size]


def _elf_string(blob: bytes, offset: int, label: str) -> str:
    if offset < 0 or offset >= len(blob):
        raise LedgerError(f"candidate object has an invalid {label} string offset")
    end = blob.find(b"\0", offset)
    if end < 0:
        raise LedgerError(f"candidate object has an unterminated {label} string")
    try:
        value = blob[offset:end].decode("utf-8")
    except UnicodeDecodeError as error:
        raise LedgerError(f"candidate object has a non-UTF-8 {label} string") from error
    if any(ord(character) < 32 or ord(character) == 127 for character in value):
        raise LedgerError(f"candidate object has a non-printable {label} string")
    return value


def _candidate_relocation_evidence(report: Mapping[str, Any]) -> int:
    comparison = _mapping(report.get("relocation_comparison"))
    candidate_count = _report_int(
        comparison.get("candidate_record_count", UNSET),
        "relocation_comparison.candidate_record_count",
    )
    resolved_count = _report_int(
        comparison.get("candidate_identity_resolved_count", UNSET),
        "relocation_comparison.candidate_identity_resolved_count",
    )
    unresolved = comparison.get("candidate_identity_unresolved_records", UNSET)
    if candidate_count is UNSET or resolved_count is UNSET or unresolved is UNSET:
        raise LedgerError(
            "preflight relocation comparison lacks candidate count/identity evidence"
        )
    if not isinstance(unresolved, list):
        raise LedgerError(
            "preflight candidate unresolved-relocation evidence must be a list"
        )
    if resolved_count > candidate_count:
        raise LedgerError("preflight candidate relocation identity counts are inconsistent")
    if resolved_count != candidate_count or unresolved:
        raise LedgerError(
            "candidate function fingerprint requires every static relocation identity "
            "to be resolved"
        )

    preflight = _mapping(report.get("preflight"))
    if preflight.get("status") != "complete":
        raise LedgerError(
            "candidate function fingerprint requires complete function_preflight evidence"
        )
    counts = _mapping(preflight.get("counts"))
    counted = _report_int(
        counts.get("candidate_static_relocations", UNSET),
        "preflight.counts.candidate_static_relocations",
    )
    resolved = _report_int(
        counts.get("candidate_identities_resolved", UNSET),
        "preflight.counts.candidate_identities_resolved",
    )
    unresolved_count = _report_int(
        counts.get("candidate_identities_unresolved", UNSET),
        "preflight.counts.candidate_identities_unresolved",
    )
    if any(value is UNSET for value in (counted, resolved, unresolved_count)):
        raise LedgerError("preflight summary lacks candidate relocation identity counts")
    if not (counted == resolved == candidate_count and unresolved_count == 0):
        raise LedgerError(
            "preflight summary disagrees with candidate relocation identity evidence"
        )
    return candidate_count


def _symbol_identity(
    symbol: tuple[str, int, int, int, int],
    symbols: Sequence[tuple[str, int, int, int, int]],
    section_names: Sequence[str],
    function_section: int,
    function_start: int,
    function_end: int,
) -> str:
    name, value, _size, info, section_index = symbol
    symbol_type = info & 0xF
    if symbol_type == STT_SECTION or not name:
        raise LedgerError(
            "candidate function relocation has an unnamed or section-only target identity"
        )
    same_name = {
        (candidate[1], candidate[2], candidate[3], candidate[4])
        for candidate in symbols
        if candidate[0] == name
    }
    if len(same_name) != 1:
        raise LedgerError(
            f"candidate function relocation target {name!r} has ambiguous symbol definitions"
        )
    if section_index == SHN_UNDEF:
        return f"undefined:{name}"
    if section_index >= len(section_names):
        raise LedgerError(
            f"candidate function relocation target {name!r} has unsupported section identity"
        )
    if section_index == function_section and function_start <= value < function_end:
        return f"self:+{value - function_start}:{name}"
    section = section_names[section_index]
    if not section:
        raise LedgerError(
            f"candidate function relocation target {name!r} has an unnamed section"
        )
    return f"symbol:{name}:{section}"


def _relocation_addends(
    rows: list[dict[str, Any]], function_bytes: bytearray
) -> dict[int, int]:
    addends: dict[int, int] = {}
    pending_high: dict[int, list[dict[str, Any]]] = {}
    for row in rows:
        relative = row["relative_offset"]
        word = struct.unpack_from(">I", function_bytes, relative)[0]
        relocation_type = row["type"]
        symbol_index = row["symbol_index"]
        if relocation_type == R_MIPS_HI16:
            pending_high.setdefault(symbol_index, []).append(row)
            continue
        if relocation_type == R_MIPS_LO16:
            low = word & 0xFFFF
            low_signed = low - 0x10000 if low & 0x8000 else low
            highs = pending_high.pop(symbol_index, [])
            if highs:
                paired = {
                    (
                        (
                            struct.unpack_from(
                                ">I", function_bytes, high["relative_offset"]
                            )[0]
                            & 0xFFFF
                        )
                        << 16
                    )
                    + low_signed
                    for high in highs
                }
                if len(paired) != 1:
                    raise LedgerError(
                        "candidate function has an ambiguous MIPS HI16/LO16 addend group"
                    )
                full_addend = next(iter(paired))
                for high in highs:
                    addends[high["relative_offset"]] = full_addend
                addends[relative] = full_addend
            else:
                addends[relative] = low_signed
            continue
        if relocation_type == R_MIPS_26:
            addends[relative] = (word & 0x03FFFFFF) << 2
        elif relocation_type == R_MIPS_32:
            addends[relative] = word
    if pending_high:
        raise LedgerError("candidate function has an unpaired MIPS HI16 relocation")
    return addends


def _fingerprint_candidate_function(
    payload: bytes,
    symbol_name: str,
    candidate_words: int,
    expected_relocations: int,
) -> dict[str, Any]:
    if len(payload) < 52 or payload[:4] != ELF_MAGIC:
        raise LedgerError("candidate object is not an ELF file")
    if payload[4] != ELFCLASS32 or payload[5] != ELFDATA2MSB or payload[6] != 1:
        raise LedgerError("candidate object must be a version-1 big-endian ELF32 file")
    try:
        header = struct.unpack_from(">16sHHIIIIIHHHHHH", payload, 0)
    except struct.error as error:
        raise LedgerError("candidate object has a truncated ELF header") from error
    (
        _ident,
        elf_type,
        machine,
        version,
        _entry,
        _program_offset,
        section_offset,
        _flags,
        header_size,
        _program_entry_size,
        program_count,
        section_entry_size,
        section_count,
        names_index,
    ) = header
    if elf_type != ET_REL:
        raise LedgerError("candidate object must be a relocatable ELF object")
    if machine != EM_MIPS or version != 1 or program_count != 0:
        raise LedgerError("candidate object must use unambiguous MIPS ELF geometry")
    if header_size != 52 or section_entry_size != 40:
        raise LedgerError("candidate object has unsupported ELF header geometry")
    if not 1 <= section_count <= 4096 or not 0 < names_index < section_count:
        raise LedgerError("candidate object has ambiguous ELF section geometry")
    _bounded_slice(
        payload, section_offset, section_count * section_entry_size, "section table"
    )
    sections = [
        struct.unpack_from(">10I", payload, section_offset + index * section_entry_size)
        for index in range(section_count)
    ]
    names_header = sections[names_index]
    names_blob = _bounded_slice(
        payload, names_header[4], names_header[5], "section-name table"
    )
    section_names = [
        _elf_string(names_blob, section[0], "section name") for section in sections
    ]
    text_indexes = [index for index, name in enumerate(section_names) if name == ".text"]
    if len(text_indexes) != 1:
        raise LedgerError(
            f"candidate object must contain exactly one .text section, found {len(text_indexes)}"
        )
    text_index = text_indexes[0]
    text_header = sections[text_index]
    if text_header[1] != SHT_PROGBITS:
        raise LedgerError("candidate object .text section must contain program bytes")
    text = _bounded_slice(payload, text_header[4], text_header[5], ".text section")

    symtab_indexes = [
        index for index, section in enumerate(sections) if section[1] == SHT_SYMTAB
    ]
    if len(symtab_indexes) != 1:
        raise LedgerError(
            f"candidate object must contain exactly one symbol table, found {len(symtab_indexes)}"
        )
    symtab_index = symtab_indexes[0]
    symtab = sections[symtab_index]
    if symtab[9] != 16 or symtab[5] % 16 or symtab[6] >= section_count:
        raise LedgerError("candidate object has unsupported symbol-table geometry")
    string_header = sections[symtab[6]]
    if string_header[1] != SHT_STRTAB:
        raise LedgerError("candidate object symbol table does not link a string table")
    strings = _bounded_slice(
        payload, string_header[4], string_header[5], "symbol string table"
    )
    symbol_blob = _bounded_slice(payload, symtab[4], symtab[5], "symbol table")
    symbols: list[tuple[str, int, int, int, int]] = []
    for offset in range(0, len(symbol_blob), 16):
        name_offset, value, size, info, _other, section_index = struct.unpack_from(
            ">IIIBBH", symbol_blob, offset
        )
        name = _elf_string(strings, name_offset, "symbol")
        symbols.append((name, value, size, info, section_index))

    definitions = [
        row
        for row in symbols
        if row[0] == symbol_name
        and row[4] != SHN_UNDEF
        and row[2] > 0
        and (row[3] & 0xF) == STT_FUNC
        and row[4] == text_index
    ]
    if len(definitions) != 1:
        raise LedgerError(
            f"candidate object must contain one nonzero .text function {symbol_name}, "
            f"found {len(definitions)}"
        )
    _name, function_start, function_size, _info, _section = definitions[0]
    function_end = function_start + function_size
    if function_size % 4 or function_end > len(text):
        raise LedgerError("candidate function has invalid or out-of-bounds ELF geometry")
    if function_size != candidate_words * 4:
        raise LedgerError(
            "candidate function ELF size disagrees with function_preflight candidate_words"
        )
    overlapping_functions = [
        row[0]
        for row in symbols
        if row[0] != symbol_name
        and row[4] == text_index
        and row[2] > 0
        and (row[3] & 0xF) == STT_FUNC
        and max(function_start, row[1]) < min(function_end, row[1] + row[2])
    ]
    if overlapping_functions:
        raise LedgerError(
            "candidate function boundary overlaps another ELF function definition: "
            + ", ".join(sorted(set(overlapping_functions)))
        )

    if any(
        section[1] == SHT_RELA and section[7] == text_index for section in sections
    ):
        raise LedgerError("candidate object uses unsupported RELA relocations for .text")
    relocation_sections = [
        section
        for section in sections
        if section[1] == SHT_REL and section[7] == text_index
    ]
    if len(relocation_sections) > 1:
        raise LedgerError("candidate object has multiple relocation sections for .text")
    rows: list[dict[str, Any]] = []
    if relocation_sections:
        relocation_section = relocation_sections[0]
        if (
            relocation_section[9] != 8
            or relocation_section[5] % 8
            or relocation_section[6] != symtab_index
        ):
            raise LedgerError("candidate object has unsupported .text relocation geometry")
        relocation_blob = _bounded_slice(
            payload,
            relocation_section[4],
            relocation_section[5],
            ".text relocation table",
        )
        seen_offsets: set[int] = set()
        for offset in range(0, len(relocation_blob), 8):
            site, info = struct.unpack_from(">II", relocation_blob, offset)
            if not function_start <= site < function_end:
                continue
            relative = site - function_start
            relocation_type = info & 0xFF
            symbol_index = info >> 8
            if relative % 4 or relative + 4 > function_size:
                raise LedgerError("candidate function has an unaligned relocation site")
            if relative in seen_offsets:
                raise LedgerError("candidate function has duplicate relocation sites")
            if relocation_type not in SUPPORTED_FINGERPRINT_RELOCATIONS:
                raise LedgerError(
                    f"candidate function uses unsupported relocation type {relocation_type}"
                )
            if symbol_index >= len(symbols):
                raise LedgerError("candidate function relocation has an invalid symbol index")
            seen_offsets.add(relative)
            rows.append(
                {
                    "relative_offset": relative,
                    "type": relocation_type,
                    "symbol_index": symbol_index,
                    "identity": _symbol_identity(
                        symbols[symbol_index],
                        symbols,
                        section_names,
                        text_index,
                        function_start,
                        function_end,
                    ),
                }
            )
    rows.sort(key=lambda row: row["relative_offset"])
    if len(rows) != expected_relocations:
        raise LedgerError(
            "candidate function ELF relocation count disagrees with function_preflight "
            f"evidence: object={len(rows)}, preflight={expected_relocations}"
        )

    function_bytes = bytearray(text[function_start:function_end])
    addends = _relocation_addends(rows, function_bytes)
    relocation_payload: list[list[Any]] = []
    for row in rows:
        relative = row["relative_offset"]
        word = struct.unpack_from(">I", function_bytes, relative)[0]
        if row["type"] == R_MIPS_32:
            masked = 0
        elif row["type"] == R_MIPS_26:
            masked = word & 0xFC000000
        else:
            masked = word & 0xFFFF0000
        struct.pack_into(">I", function_bytes, relative, masked)
        relocation_payload.append(
            [relative, row["type"], row["identity"], addends[relative]]
        )
    fingerprint_input = {
        "algorithm": FINGERPRINT_ALGORITHM,
        "size": function_size,
        "masked_text_sha256": hashlib.sha256(function_bytes).hexdigest(),
        "relocations": relocation_payload,
    }
    encoded = json.dumps(
        fingerprint_input, sort_keys=True, separators=(",", ":"), ensure_ascii=True
    ).encode("ascii")
    return {
        "algorithm": FINGERPRINT_ALGORITHM,
        "sha256": hashlib.sha256(encoded).hexdigest(),
        "size": function_size,
        "relocations": len(rows),
    }


def _candidate_artifact_and_fingerprint(
    root: Path,
    value: Any,
    symbol: str,
    candidate_words: int,
    report: Mapping[str, Any],
    preflight_mtime_ns: int,
) -> tuple[dict[str, str], dict[str, Any]]:
    expected_relocations = _candidate_relocation_evidence(report)
    relative = _relative_generated_path(value, "candidate_object", suffix=".o")
    fd = _safe_regular_fd(root, relative, "candidate object")
    try:
        before = os.fstat(fd)
        if before.st_size <= 0 or before.st_size > MAX_ARTIFACT_BYTES:
            raise LedgerError(
                "candidate object must be a non-empty regular file no larger than 256 MiB"
            )
        if before.st_mtime_ns > preflight_mtime_ns:
            raise LedgerError(
                "candidate object is newer than the function_preflight report; rerun preflight"
            )
        chunks: list[bytes] = []
        remaining = before.st_size
        while remaining:
            chunk = os.read(fd, min(remaining, 1024 * 1024))
            if not chunk:
                raise LedgerError("candidate object changed while it was being read")
            chunks.append(chunk)
            remaining -= len(chunk)
        after = os.fstat(fd)
        if (before.st_size, before.st_mtime_ns, before.st_ino) != (
            after.st_size,
            after.st_mtime_ns,
            after.st_ino,
        ):
            raise LedgerError("candidate object changed while it was being read")
    finally:
        os.close(fd)
    payload = b"".join(chunks)
    artifact = {"path": relative, "sha256": hashlib.sha256(payload).hexdigest()}
    fingerprint = _fingerprint_candidate_function(
        payload, symbol, candidate_words, expected_relocations
    )
    return artifact, fingerprint


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


def _candidate_digest_from_record(record: Mapping[str, Any]) -> str | None:
    object_digests = {
        artifact["sha256"]
        for artifact in record.get("artifacts", [])
        if PurePosixPath(artifact["path"]).suffix == ".o"
    }
    return next(iter(object_digests)) if len(object_digests) == 1 else None


def _record_has_object_digest(record: Mapping[str, Any], digest: str) -> bool:
    return any(
        artifact["sha256"] == digest
        and PurePosixPath(artifact["path"]).suffix == ".o"
        for artifact in record.get("artifacts", [])
    )


def _candidate_fingerprint_from_record(record: Mapping[str, Any]) -> str | None:
    fingerprint = record.get("candidate_fingerprint")
    if not isinstance(fingerprint, Mapping):
        return None
    digest = fingerprint.get("sha256")
    return digest if isinstance(digest, str) and SHA256_RE.fullmatch(digest) else None


def append_record(
    path: Path,
    record: dict[str, Any],
    *,
    candidate_sha256: str | None = None,
    candidate_fingerprint: str | None = None,
) -> dict[str, Any]:
    checked = validate_record(record)
    digest = candidate_sha256 or _candidate_digest_from_record(checked)
    if digest is not None and not SHA256_RE.fullmatch(digest):
        raise LedgerError("candidate-object SHA-256 must be 64 lowercase hexadecimal digits")
    fingerprint = candidate_fingerprint or _candidate_fingerprint_from_record(checked)
    if fingerprint is not None and not SHA256_RE.fullmatch(fingerprint):
        raise LedgerError(
            "candidate-function fingerprint must be 64 lowercase hexadecimal digits"
        )
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
        records = _decode_records(payload)  # Refuse to extend a malformed journal.
        for line_number, existing in enumerate(records, 1):
            if existing["symbol"] != checked["symbol"]:
                continue
            existing_fingerprint = _candidate_fingerprint_from_record(existing)
            if fingerprint is not None and existing_fingerprint == fingerprint:
                raise LedgerError(
                    "duplicate candidate function fingerprint for "
                    f"{checked['symbol']}: matches line {line_number} "
                    f"({existing['timestamp']}, hypothesis={existing['hypothesis']!r})"
                )
            # Schema-v1 rows have no isolated fingerprint. Preserve their
            # exact-object duplicate protection without letting whole-TU
            # identity override a schema-v2 function fingerprint.
            if (
                digest is not None
                and existing_fingerprint is None
                and _record_has_object_digest(existing, digest)
            ):
                raise LedgerError(
                    "duplicate legacy candidate artifact for "
                    f"{checked['symbol']}: matches line {line_number} "
                    f"({existing['timestamp']}, hypothesis={existing['hypothesis']!r})"
                )
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


def _mapping(value: Any) -> Mapping[str, Any]:
    return value if isinstance(value, Mapping) else {}


def _first_report_value(*values: Any) -> Any:
    return next((value for value in values if value is not UNSET and value is not None), UNSET)


def _first_report_offset_value(*values: Any) -> Any:
    return next((value for value in values if value is not UNSET), UNSET)


def _report_int(value: Any, field: str) -> int | object:
    if value is UNSET or value is None:
        return UNSET
    return _plain_int(value, f"preflight {field}")


def _report_offset(value: Any, field: str) -> int | None | object:
    if value is UNSET:
        return UNSET
    if value is None:
        return None
    if isinstance(value, bool):
        raise LedgerError(f"preflight {field} must be an aligned byte offset or null")
    if isinstance(value, int):
        return _plain_int(value, f"preflight {field}")
    if isinstance(value, str):
        try:
            return _parse_optional_offset(value)
        except argparse.ArgumentTypeError as error:
            raise LedgerError(
                f"preflight {field} must be an aligned byte offset or null"
            ) from error
    raise LedgerError(f"preflight {field} must be an aligned byte offset or null")


def _evidence_or_override(field: str, reported: Any, explicit: Any) -> Any:
    if reported is not UNSET:
        if explicit is not UNSET and explicit != reported:
            raise LedgerError(
                f"--{field.replace('_', '-')} disagrees with preflight evidence; "
                "explicit values may fill only absent report evidence"
            )
        return reported
    if explicit is not UNSET:
        return explicit
    raise LedgerError(
        f"preflight report lacks {field}; supply --{field.replace('_', '-')} explicitly"
    )


def _preflight_metrics(report: Mapping[str, Any], args: argparse.Namespace) -> dict[str, Any]:
    workbench = _mapping(report.get("workbench"))
    relocation = _mapping(report.get("relocation_comparison"))
    future_relocation = _mapping(report.get("relocation_report"))

    candidate_words = _evidence_or_override(
        "candidate_words",
        _report_int(
            _first_report_value(workbench.get("candidate_words", UNSET),
                                report.get("candidate_words", UNSET)),
            "candidate_words",
        ),
        args.candidate_words,
    )
    target_words = _evidence_or_override(
        "target_words",
        _report_int(
            _first_report_value(workbench.get("target_words", UNSET),
                                report.get("target_words", UNSET)),
            "target_words",
        ),
        args.target_words,
    )
    raw = _evidence_or_override(
        "raw_differences",
        _report_int(
            _first_report_value(
                workbench.get("raw_differences", UNSET),
                workbench.get("differing_words", UNSET),
                report.get("raw_differences", UNSET),
            ),
            "raw_differences",
        ),
        args.raw_differences,
    )
    reported_masked = _report_int(
        _first_report_value(
            workbench.get("relocation_masked_differences", UNSET),
            workbench.get("relocation_masked_differing_words", UNSET),
            report.get("relocation_masked_differences", UNSET),
        ),
        "relocation_masked_differences",
    )
    masked = (
        _evidence_or_override(
            "masked_differences", reported_masked, args.masked_differences
        )
        if reported_masked is not UNSET or args.masked_differences is not UNSET
        else raw
    )
    frame = _evidence_or_override(
        "frame",
        _report_int(
            _first_report_value(
                workbench.get("candidate_frame", UNSET),
                workbench.get("candidate_frame_size", UNSET),
                report.get("candidate_frame", UNSET),
            ),
            "frame",
        ),
        args.frame,
    )

    candidate_relocations = _evidence_or_override(
        "candidate_relocations",
        _report_int(
            _first_report_value(
                relocation.get("candidate_record_count", UNSET),
                future_relocation.get("candidate_record_count", UNSET),
                report.get("candidate_relocations", UNSET),
            ),
            "candidate_relocations",
        ),
        args.candidate_relocations,
    )
    target_relocations = _evidence_or_override(
        "target_relocations",
        _report_int(
            _first_report_value(
                relocation.get("target_record_count", UNSET),
                future_relocation.get("target_record_count", UNSET),
                report.get("target_relocations", UNSET),
            ),
            "target_relocations",
        ),
        args.target_relocations,
    )
    reported_identities = _report_int(
        _first_report_value(
            relocation.get("effective_identity_alignment_count", UNSET),
            future_relocation.get("effective_identity_alignment_count", UNSET),
            relocation.get("stable_identity_alignment_count", UNSET),
            future_relocation.get("stable_identity_alignment_count", UNSET),
            report.get("relocation_identities", UNSET),
        ),
        "relocation_identities",
    )
    if reported_identities is UNSET and candidate_relocations == target_relocations == 0:
        reported_identities = 0
    relocation_identities = _evidence_or_override(
        "relocation_identities", reported_identities, args.relocation_identities
    )

    first_raw = _evidence_or_override(
        "first_raw_mismatch",
        _report_offset(
            _first_report_offset_value(
                workbench.get("first_raw_mismatch", UNSET),
                workbench.get("first_mismatch", UNSET),
                report.get("first_raw_mismatch", UNSET),
            ),
            "first_raw_mismatch",
        ),
        args.first_raw_mismatch,
    )
    reported_first_masked = _report_offset(
        _first_report_offset_value(
            workbench.get("first_masked_mismatch", UNSET),
            workbench.get("relocation_masked_first_mismatch", UNSET),
            report.get("first_masked_mismatch", UNSET),
        ),
        "first_masked_mismatch",
    )
    if reported_first_masked is not UNSET or args.first_masked_mismatch is not UNSET:
        first_masked = _evidence_or_override(
            "first_masked_mismatch", reported_first_masked, args.first_masked_mismatch
        )
    elif masked == 0:
        first_masked = None
    elif masked == raw:
        first_masked = first_raw
    else:
        raise LedgerError(
            "preflight report lacks first_masked_mismatch for the distinct masked score; "
            "supply --first-masked-mismatch explicitly"
        )

    return {
        "candidate_words": candidate_words,
        "target_words": target_words,
        "raw_differences": raw,
        "relocation_masked_differences": masked,
        "frame": frame,
        "candidate_relocations": candidate_relocations,
        "target_relocations": target_relocations,
        "relocation_identities": relocation_identities,
        "first_raw_mismatch": first_raw,
        "first_masked_mismatch": first_masked,
    }


def _merge_artifacts(
    explicit: Sequence[dict[str, str]], candidate: dict[str, str] | None
) -> list[dict[str, str]]:
    artifacts = list(explicit)
    if candidate is None:
        return artifacts
    matching = [row for row in artifacts if row["path"] == candidate["path"]]
    if matching:
        if any(row["sha256"] != candidate["sha256"] for row in matching):
            raise LedgerError("explicit artifact hash disagrees with the candidate object")
        return artifacts
    return [candidate, *artifacts]


def _record_from_args(
    args: argparse.Namespace, root: Path
) -> tuple[dict[str, Any], str | None, str | None]:
    if args.preflight_json:
        report, preflight_mtime_ns = _load_preflight_report_with_mtime(
            root, args.preflight_json
        )
    else:
        report = None
        preflight_mtime_ns = None
    if report is None:
        symbol = validate_symbol(args.symbol)
        if args.source is None:
            raise LedgerError("--source is required without --preflight-json")
        source = validate_source(args.source)
        metrics = {
            "candidate_words": _evidence_or_override(
                "candidate_words", UNSET, args.candidate_words
            ),
            "target_words": _evidence_or_override("target_words", UNSET, args.target_words),
            "raw_differences": _evidence_or_override(
                "raw_differences", UNSET, args.raw_differences
            ),
            "relocation_masked_differences": _evidence_or_override(
                "masked_differences", UNSET, args.masked_differences
            ),
            "frame": _evidence_or_override("frame", UNSET, args.frame),
            "candidate_relocations": _evidence_or_override(
                "candidate_relocations", UNSET, args.candidate_relocations
            ),
            "target_relocations": _evidence_or_override(
                "target_relocations", UNSET, args.target_relocations
            ),
            "relocation_identities": _evidence_or_override(
                "relocation_identities", UNSET, args.relocation_identities
            ),
            "first_raw_mismatch": _evidence_or_override(
                "first_raw_mismatch", UNSET, args.first_raw_mismatch
            ),
            "first_masked_mismatch": _evidence_or_override(
                "first_masked_mismatch", UNSET, args.first_masked_mismatch
            ),
        }
        candidate_artifact = None
        candidate_fingerprint = None
        schema_version = LEGACY_SCHEMA_VERSION
    else:
        aliases = {
            value
            for key in ("requested_symbol", "target_symbol", "candidate_symbol")
            if isinstance((value := report.get(key)), str)
        }
        if args.symbol not in aliases:
            raise LedgerError(
                f"requested symbol {args.symbol!r} is not named by the preflight report"
            )
        symbol = validate_symbol(report.get("candidate_symbol"))
        reported_source = report.get("source", UNSET)
        if reported_source is UNSET or reported_source is None:
            if args.source is None:
                raise LedgerError("preflight report lacks source; supply --source explicitly")
            source = validate_source(args.source)
        else:
            source = validate_source(reported_source)
            if args.source is not None and validate_source(args.source) != source:
                raise LedgerError(
                    "--source disagrees with preflight evidence; explicit values may fill "
                    "only absent report evidence"
                )
        metrics = _preflight_metrics(report, args)
        candidate_value = report.get("candidate_object", UNSET)
        if candidate_value is UNSET or candidate_value is None:
            raise LedgerError(
                "preflight report lacks candidate_object required for isolated fingerprinting"
            )
        assert preflight_mtime_ns is not None
        candidate_artifact, candidate_fingerprint = _candidate_artifact_and_fingerprint(
            root,
            candidate_value,
            symbol,
            metrics["candidate_words"],
            report,
            preflight_mtime_ns,
        )
        schema_version = SCHEMA_VERSION

    require_source_ownership(root, source, symbol)
    record: dict[str, Any] = {
        "schema_version": schema_version,
        "timestamp": utc_timestamp(),
        "symbol": symbol,
        "source": source,
        "hypothesis": args.hypothesis,
        **metrics,
        "verdict": args.verdict,
    }
    if candidate_fingerprint is not None:
        record["candidate_fingerprint"] = candidate_fingerprint
    artifacts = _merge_artifacts(args.artifact, candidate_artifact)
    if artifacts:
        record["artifacts"] = artifacts
    checked = validate_record(record)
    digest = candidate_artifact["sha256"] if candidate_artifact is not None else None
    fingerprint_digest = (
        candidate_fingerprint["sha256"] if candidate_fingerprint is not None else None
    )
    return checked, digest, fingerprint_digest


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
    append.add_argument("--source")
    append.add_argument(
        "--preflight-json",
        metavar="build/REPORT.json",
        help="ingest scalar evidence and the candidate object from function_preflight JSON",
    )
    append.add_argument("--hypothesis", required=True)
    append.add_argument("--candidate-words", type=_parse_nonnegative, default=UNSET)
    append.add_argument("--target-words", type=_parse_nonnegative, default=UNSET)
    append.add_argument("--raw-differences", type=_parse_nonnegative, default=UNSET)
    append.add_argument("--masked-differences", type=_parse_nonnegative, default=UNSET)
    append.add_argument("--frame", type=_parse_frame, default=UNSET)
    append.add_argument("--candidate-relocations", type=_parse_nonnegative, default=UNSET)
    append.add_argument("--target-relocations", type=_parse_nonnegative, default=UNSET)
    append.add_argument("--relocation-identities", type=_parse_nonnegative, default=UNSET)
    append.add_argument("--first-raw-mismatch", type=_parse_optional_offset, default=UNSET)
    append.add_argument("--first-masked-mismatch", type=_parse_optional_offset, default=UNSET)
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
            pending, candidate_digest, candidate_fingerprint = _record_from_args(
                args, root
            )
            record = append_record(
                ledger,
                pending,
                candidate_sha256=candidate_digest,
                candidate_fingerprint=candidate_fingerprint,
            )
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
