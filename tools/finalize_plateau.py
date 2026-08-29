#!/usr/bin/env python3
"""Safely preserve one bounded NON_MATCHING plateau.

The command records concise measured evidence, runs source-only repository
gates, and commits only when --commit is supplied.  It never reads or records
target instruction rows.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import re
import subprocess
import sys


SYMBOL_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
SCORE_RE = re.compile(r"^(?:\d+/\d+ (?:words|instructions|bytes)|\d+ differing words)$")
FRAME_RE = re.compile(r"^(?:-?0x[0-9A-Fa-f]+|frameless|unknown)$")
MISMATCH_RE = re.compile(r"^(?:\+?0x[0-9A-Fa-f]+|none|unknown)$")
DIRECTIVE_RE = re.compile(r"^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)$")
LEGACY_HANDOFF_RE = re.compile(
    r"/\* PLATEAU-HANDOFF\n"
    r"(?: \* [^\n]*\n)*?"
    r" \*/\n",
)
KEYED_HANDOFF_RE = re.compile(
    r"/\* PLATEAU-HANDOFF:(?P<symbol>[A-Za-z_][A-Za-z0-9_]*):start\n"
    r"(?: \* [^\n]*\n)*?"
    r" \* PLATEAU-HANDOFF:(?P=symbol):end\n"
    r" \*/\n?",
)
GENERATED_OVERLAY_FALLBACK_RE = re.compile(
    r"^func_overlay_[0-9]{3}_F[0-9A-Fa-f]{7}_[0-9A-Fa-f]+\.s$"
)


class PlateauError(RuntimeError):
    """A concise refusal that leaves the candidate uncommitted."""


@dataclass(frozen=True)
class GuardedCandidate:
    ifdef_line: int
    else_line: int
    endif_line: int
    fallback: str


@dataclass(frozen=True)
class Metrics:
    score: str
    frame: str
    relocations: int
    first_mismatch: str
    summary: str = ""


def run_git(root: Path, *args: str, check: bool = True) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(
        ["git", *args], cwd=root, text=True, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if check and result.returncode != 0:
        detail = result.stderr.strip() or result.stdout.strip() or "git command failed"
        raise PlateauError(f"git {' '.join(args)}: {detail}")
    return result


def repository_root() -> Path:
    result = subprocess.run(
        ["git", "rev-parse", "--show-toplevel"], text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    )
    if result.returncode != 0:
        raise PlateauError("run this command inside a Git worktree")
    return Path(result.stdout.strip()).resolve()


def validate_one_line(value: str, label: str, limit: int = 160) -> str:
    if not value or len(value) > limit or any(character in value for character in "\r\n\t|"):
        raise PlateauError(f"{label} must be one concise line without table separators")
    return " ".join(value.split())


def validate_metrics(args: argparse.Namespace) -> Metrics:
    score = validate_one_line(args.score, "score", 64)
    frame = validate_one_line(args.frame, "frame", 24)
    first_mismatch = validate_one_line(args.first_mismatch, "first mismatch", 24)
    if not SCORE_RE.fullmatch(score):
        raise PlateauError("score must look like '98/101 words' or '43 differing words'")
    if not FRAME_RE.fullmatch(frame):
        raise PlateauError("frame must be hexadecimal, 'frameless', or 'unknown'")
    if not MISMATCH_RE.fullmatch(first_mismatch):
        raise PlateauError("first mismatch must be a hexadecimal offset, 'none', or 'unknown'")
    if args.relocations < 0:
        raise PlateauError("relocations must be non-negative")
    summary = validate_one_line(args.summary, "summary") if args.summary else ""
    return Metrics(score, frame, args.relocations, first_mismatch, summary)


def directive(line: str) -> tuple[str, str] | None:
    match = DIRECTIVE_RE.match(line)
    if not match:
        return None
    return match.group(1), match.group(2).strip()


def guarded_candidates(text: str, symbol: str) -> list[GuardedCandidate]:
    lines = text.splitlines(keepends=True)
    found: list[GuardedCandidate] = []
    definition = re.compile(rf"\b{re.escape(symbol)}\s*\([^;{{}}]*\)\s*\{{", re.DOTALL)
    fallback_re = re.compile(r'#\s*pragma\s+GLOBAL_ASM\s*\(\s*"([^"]+)"\s*\)')

    for start, line in enumerate(lines):
        parsed = directive(line)
        if parsed != ("ifdef", "NON_MATCHING"):
            continue
        depth = 1
        else_line: int | None = None
        end_line: int | None = None
        for index in range(start + 1, len(lines)):
            nested = directive(lines[index])
            if not nested:
                continue
            kind, _ = nested
            if kind in ("if", "ifdef", "ifndef"):
                depth += 1
            elif kind == "endif":
                depth -= 1
                if depth == 0:
                    end_line = index
                    break
            elif kind == "else" and depth == 1:
                if else_line is not None:
                    raise PlateauError(f"multiple top-level #else directives after line {start + 1}")
                else_line = index
        if else_line is None or end_line is None:
            raise PlateauError(f"unterminated NON_MATCHING guard after line {start + 1}")
        candidate_text = "".join(lines[start + 1:else_line])
        if not definition.search(candidate_text):
            continue
        fallback_text = "".join(lines[else_line + 1:end_line])
        fallbacks = fallback_re.findall(fallback_text)
        valid = [
            path for path in fallbacks
            if Path(path).name == f"{symbol}.s"
            or GENERATED_OVERLAY_FALLBACK_RE.fullmatch(Path(path).name)
        ]
        if len(fallbacks) != 1 or len(valid) != 1:
            raise PlateauError(
                f"{symbol} must have exactly one matching or generated-overlay "
                "#pragma GLOBAL_ASM fallback"
            )
        found.append(GuardedCandidate(start, else_line, end_line, valid[0]))
    return found


def require_guarded_candidate(text: str, symbol: str) -> GuardedCandidate:
    candidates = guarded_candidates(text, symbol)
    if len(candidates) != 1:
        if not candidates:
            raise PlateauError(
                f"{symbol} is not an unambiguous #ifdef NON_MATCHING candidate with a GLOBAL_ASM fallback"
            )
        raise PlateauError(f"{symbol} appears in more than one guarded candidate")
    return candidates[0]


def source_handoff(symbol: str, metrics: Metrics) -> str:
    fields = [
        f"symbol: {symbol}",
        f"score: {metrics.score}",
        f"frame: {metrics.frame}",
        f"relocations: {metrics.relocations}",
        f"first-mismatch: {metrics.first_mismatch}",
    ]
    if metrics.summary:
        fields.append(f"summary: {metrics.summary}")
    return (
        f"/* PLATEAU-HANDOFF:{symbol}:start\n"
        + "".join(f" * {field}\n" for field in fields)
        + f" * PLATEAU-HANDOFF:{symbol}:end\n"
        + " */\n"
    )


def update_source(text: str, symbol: str, handoff: str) -> str:
    if LEGACY_HANDOFF_RE.search(text):
        raise PlateauError(
            "legacy inline PLATEAU-HANDOFF would move measured source lines; "
            "migrate it manually and re-prove the candidate"
        )

    blocks = list(KEYED_HANDOFF_RE.finditer(text))
    marker_count = text.count("PLATEAU-HANDOFF:")
    if marker_count != 2 * len(blocks):
        raise PlateauError("malformed symbol-keyed PLATEAU-HANDOFF metadata")
    if blocks:
        metadata_start = blocks[0].start()
        remainder = KEYED_HANDOFF_RE.sub("", text[metadata_start:])
        if remainder.strip():
            raise PlateauError("PLATEAU-HANDOFF metadata must be a contiguous EOF suffix")

    owned = [block for block in blocks if block.group("symbol") == symbol]
    if len(owned) > 1:
        raise PlateauError(f"duplicate PLATEAU-HANDOFF metadata for {symbol}")
    if owned:
        block = owned[0]
        return text[:block.start()] + handoff + text[block.end():]

    if not text or text.endswith("\n\n"):
        separator = ""
    elif text.endswith("\n"):
        separator = "\n"
    else:
        separator = "\n\n"
    return text + separator + handoff


def markdown_handoff(symbol: str, source: str, metrics: Metrics) -> str:
    marker = f"plateau-handoff:{symbol}"
    rows = [
        f"<!-- {marker}:start -->",
        f"### `{symbol}` plateau handoff",
        "",
        f"- source: `{source}`",
        f"- score: {metrics.score}",
        f"- frame: {metrics.frame}",
        f"- relocations: {metrics.relocations}",
        f"- first mismatch: {metrics.first_mismatch}",
    ]
    if metrics.summary:
        rows.append(f"- summary: {metrics.summary}")
    rows.extend((f"<!-- {marker}:end -->", ""))
    return "\n".join(rows)


def update_markdown(text: str, symbol: str, block: str) -> str:
    marker = re.escape(f"plateau-handoff:{symbol}")
    pattern = re.compile(
        rf"<!-- {marker}:start -->.*?<!-- {marker}:end -->\n?",
        re.DOTALL,
    )
    if pattern.search(text):
        return pattern.sub(block, text, count=1)
    return text.rstrip() + "\n\n" + block


def changed_paths(root: Path) -> set[str]:
    paths: set[str] = set()
    for args in (
        ("diff", "--name-only"),
        ("diff", "--cached", "--name-only"),
        ("ls-files", "--others", "--exclude-standard"),
    ):
        output = run_git(root, *args).stdout
        paths.update(line for line in output.splitlines() if line)
    return paths


def require_only_allowed_dirt(root: Path, allowed: set[str]) -> None:
    unrelated = sorted(changed_paths(root) - allowed)
    if unrelated:
        raise PlateauError("unrelated worktree/index dirt: " + ", ".join(unrelated))


def tracked_relative_path(root: Path, value: str, kind: str, suffix: str) -> tuple[Path, str]:
    path = (root / value).resolve()
    try:
        relative = path.relative_to(root).as_posix()
    except ValueError as error:
        raise PlateauError(f"{kind} must stay inside this worktree") from error
    if path.suffix != suffix or not path.is_file():
        raise PlateauError(f"{kind} must be an existing {suffix} file: {relative}")
    if run_git(root, "ls-files", "--error-unmatch", relative, check=False).returncode != 0:
        raise PlateauError(f"{kind} must already be tracked: {relative}")
    return path, relative


def run_source_gates(root: Path) -> None:
    for target in ("cleanroom", "check-docs"):
        result = subprocess.run(["gmake", target], cwd=root)
        if result.returncode != 0:
            raise PlateauError(f"source-only gate failed: gmake {target}")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("symbol", help="Exact C/assembly symbol")
    parser.add_argument("source", help="Tracked source file containing the guarded candidate")
    parser.add_argument("--score", required=True, help="Measured score, e.g. '98/101 words'")
    parser.add_argument("--frame", required=True, help="Measured frame, e.g. 0x8 or frameless")
    parser.add_argument("--relocations", required=True, type=int, help="Measured relocation count")
    parser.add_argument("--first-mismatch", required=True, help="Measured offset, e.g. +0xC")
    parser.add_argument("--summary", default="", help="One-line blocker or next lever")
    parser.add_argument("--handoff-doc", help="Optional tracked Markdown handoff document")
    parser.add_argument("--commit", action="store_true", help="Commit only the source and handoff doc")
    parser.add_argument("--message", help="Commit subject; requires --commit")
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    try:
        if not SYMBOL_RE.fullmatch(args.symbol):
            raise PlateauError(f"invalid exact symbol {args.symbol!r}")
        if args.message and not args.commit:
            raise PlateauError("--message requires --commit")
        metrics = validate_metrics(args)
        root = repository_root()
        source_path, source_rel = tracked_relative_path(root, args.source, "source", ".c")
        if not source_rel.startswith("src/"):
            raise PlateauError("source must be under src/")
        doc_path: Path | None = None
        doc_rel: str | None = None
        if args.handoff_doc:
            doc_path, doc_rel = tracked_relative_path(
                root, args.handoff_doc, "handoff document", ".md"
            )
            if not doc_rel.startswith("docs/"):
                raise PlateauError("handoff document must be under docs/")
        allowed = {source_rel, *([doc_rel] if doc_rel else [])}
        require_only_allowed_dirt(root, allowed)

        source_text = source_path.read_text(encoding="utf-8")
        candidate = require_guarded_candidate(source_text, args.symbol)
        source_path.write_text(
            update_source(source_text, args.symbol, source_handoff(args.symbol, metrics)),
            encoding="utf-8", newline="\n",
        )
        if doc_path is not None and doc_rel is not None:
            doc_text = doc_path.read_text(encoding="utf-8")
            block = markdown_handoff(args.symbol, source_rel, metrics)
            doc_path.write_text(
                update_markdown(doc_text, args.symbol, block), encoding="utf-8", newline="\n"
            )

        require_guarded_candidate(source_path.read_text(encoding="utf-8"), args.symbol)
        require_only_allowed_dirt(root, allowed)
        run_source_gates(root)
        require_only_allowed_dirt(root, allowed)

        commit = "not requested"
        if args.commit:
            message = validate_one_line(args.message or f"Plateau {args.symbol}", "commit message", 100)
            run_git(root, "add", "--", *sorted(allowed))
            staged = set(run_git(root, "diff", "--cached", "--name-only").stdout.splitlines())
            if staged - allowed:
                raise PlateauError("refusing to commit unrelated staged paths")
            if not staged:
                commit = "unchanged"
            else:
                run_git(root, "commit", "-m", message)
                commit = run_git(root, "rev-parse", "HEAD").stdout.strip()

        print(f"symbol: {args.symbol}")
        print(f"source: {source_rel}")
        print(f"fallback: {candidate.fallback}")
        print(f"score: {metrics.score}")
        print(f"frame: {metrics.frame}")
        print(f"relocations: {metrics.relocations}")
        print(f"first-mismatch: {metrics.first_mismatch}")
        print("source-only-gates: cleanroom, check-docs")
        print(f"commit: {commit}")
    except (OSError, PlateauError) as error:
        print(f"finalize-plateau: {error}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
