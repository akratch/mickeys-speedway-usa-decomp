#!/usr/bin/env python3
"""Report commits that changed one guarded C function body.

The report is source-only: it emits commit hashes and subjects, never source
text, instruction rows, objects, or ROM bytes.  A commit is included only when
the requested function's token stream differs from its first parent, so edits
to another function in the same translation unit and comment/whitespace-only
changes do not become matching-work history.
"""

from __future__ import annotations

import argparse
import dataclasses
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path


REPO = Path(__file__).resolve().parent.parent

sys.path.insert(0, str(REPO / "tools"))
import proof_provenance as pp  # noqa: E402


class HistoryError(RuntimeError):
    """Git history or the requested source identity could not be inspected."""


@dataclasses.dataclass(frozen=True)
class HistoryEntry:
    commit: str
    subject: str


def _matching_brace(masked: str, opening: int) -> int | None:
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return index
    return None


def _c_tokens(text: str) -> tuple[str, ...]:
    """Return a compact lexical form while ignoring comments and whitespace."""

    tokens: list[str] = []
    index = 0
    while index < len(text):
        char = text[index]
        following = text[index + 1] if index + 1 < len(text) else ""
        if char.isspace():
            index += 1
            continue
        if char == "/" and following == "/":
            newline = text.find("\n", index + 2)
            index = len(text) if newline < 0 else newline + 1
            continue
        if char == "/" and following == "*":
            closing = text.find("*/", index + 2)
            index = len(text) if closing < 0 else closing + 2
            continue
        if char in {'"', "'"}:
            delimiter = char
            end = index + 1
            while end < len(text):
                if text[end] == "\\":
                    end += 2
                    continue
                end += 1
                if text[end - 1] == delimiter:
                    break
            tokens.append(text[index:end])
            index = end
            continue
        word = re.match(
            r"[A-Za-z_]\w*|(?:0[xX][0-9A-Fa-f]+|\d+)(?:[uUlLfF]+)?",
            text[index:],
        )
        if word:
            tokens.append(word.group(0))
            index += len(word.group(0))
            continue
        operator = next(
            (
                candidate
                for candidate in (
                    ">>=", "<<=", "...", "->", "++", "--", "&&", "||",
                    "<=", ">=", "==", "!=", "+=", "-=", "*=", "/=", "%=",
                    "&=", "|=", "^=", "<<", ">>",
                )
                if text.startswith(candidate, index)
            ),
            char,
        )
        tokens.append(operator)
        index += len(operator)
    return tuple(tokens)


def guarded_body_fingerprint(text: str, symbol: str) -> str | None:
    """Hash one guarded definition, or return ``None`` when it is absent."""

    facts = pp.source_facts(text, symbol)
    guarded = [row for row in facts.definitions if row.non_matching_state is True]
    if len(guarded) != 1:
        return None

    masked = pp._mask_c(text)
    starts = [0]
    starts.extend(match.end() for match in re.finditer("\n", masked))
    line_start = starts[guarded[0].line - 1]
    match = re.search(rf"\b{re.escape(symbol)}\s*\(", masked[line_start:])
    if not match:
        return None
    symbol_start = line_start + match.start()
    opening = masked.find("(", symbol_start, line_start + match.end())
    closing = pp._matching_paren(masked, opening)
    if closing is None:
        return None
    brace = closing + 1
    while brace < len(masked) and masked[brace].isspace():
        brace += 1
    if brace >= len(masked) or masked[brace] != "{":
        return None
    end = _matching_brace(masked, brace)
    if end is None:
        return None
    tokens = _c_tokens(text[symbol_start : end + 1])
    if not tokens:
        return None
    return hashlib.sha256("\0".join(tokens).encode("utf-8")).hexdigest()


def _git(root: Path, *arguments: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["git", *arguments],
        cwd=root,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )


def _blob(root: Path, revision: str, source: str) -> str | None:
    result = _git(root, "show", f"{revision}:{source}")
    return result.stdout if result.returncode == 0 else None


def guarded_body_history(
    source: Path,
    symbol: str,
    *,
    root: Path = REPO,
    limit: int = 6,
) -> list[HistoryEntry]:
    """Return newest-first commits that materially changed ``symbol``."""

    root = root.resolve()
    try:
        relative = source.resolve().relative_to(root).as_posix()
    except ValueError as error:
        raise HistoryError(f"source is outside the repository: {source}") from error
    if limit < 1:
        raise HistoryError("history limit must be positive")

    log = _git(root, "log", "--format=%H%x09%s", "--", relative)
    if log.returncode:
        detail = (log.stderr or "git log failed").strip().splitlines()
        raise HistoryError(detail[-1] if detail else "git log failed")

    entries: list[HistoryEntry] = []
    for line in log.stdout.splitlines():
        commit, separator, subject = line.partition("\t")
        if not separator or not re.fullmatch(r"[0-9a-fA-F]{40,64}", commit):
            continue
        current = _blob(root, commit, relative)
        if current is None:
            continue
        current_fingerprint = guarded_body_fingerprint(current, symbol)
        if current_fingerprint is None:
            continue
        parent = _blob(root, f"{commit}^", relative)
        parent_fingerprint = (
            guarded_body_fingerprint(parent, symbol) if parent is not None else None
        )
        if current_fingerprint == parent_fingerprint:
            continue
        entries.append(HistoryEntry(commit=commit, subject=subject.strip()))
        if len(entries) == limit:
            break
    return entries


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("symbol")
    parser.add_argument("source", type=Path)
    parser.add_argument("--limit", type=int, default=6)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)
    try:
        entries = guarded_body_history(
            args.source, args.symbol, root=REPO, limit=args.limit
        )
    except HistoryError as error:
        parser.error(str(error))
    rows = [dataclasses.asdict(entry) for entry in entries]
    if args.json:
        print(json.dumps(rows, indent=2))
    elif rows:
        for row in rows:
            print(f"{row['commit'][:10]}  {row['subject']}")
    else:
        print("no prior guarded-body changes found")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
