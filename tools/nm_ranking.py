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
   -- to produce two function-comparable objects: target.o (the
   function's own asm/nonmatchings/**/<f>.s, assembled directly -- exactly
   the ROM's bytes, splat's own disassembly of them) and base.o (normally
   the `#ifdef NON_MATCHING` C body pruned to one function). If import
   pruning cannot compile it, the tool selects only that candidate within
   an untracked TU copy and reuses the Makefile-expanded raw compile command,
   preserving static/rodata context and exact per-TU flags while skipping
   POSTPROCESS. The comparison extracts only the named function's span and
   normalizes its relocations, so it needs no linking -- which matters because a
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

    # remove now-matched rows from the retained snapshot without compiling:
    .venv/bin/python tools/nm_ranking.py --prune-stale
"""
from __future__ import annotations

import argparse
import concurrent.futures
import dataclasses
import difflib
import json
import math
import pathlib
import re
import shlex
import stat
import struct
import subprocess
import sys
import tempfile
from collections import Counter
from typing import Optional

ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import permute_batch as pb  # noqa: E402

OBJDUMP = ROOT / "tools" / "binutils" / "mips64-elf-objdump"
OBJCOPY = ROOT / "tools" / "binutils" / "mips64-elf-objcopy"
WORK_DIR = ROOT / "build" / "nm_ranking"
DEFAULT_OUT = ROOT / "config" / "nonmatching-ranking.us.json"
DEFAULT_DOC = ROOT / "docs" / "nm-ranking.md"
DOC_BEGIN = "<!-- NM_RANKING_GENERATED_BEGIN -->"
DOC_END = "<!-- NM_RANKING_GENERATED_END -->"

CATEGORY_RANK = {
    "register-only": 0,
    "schedule-only": 1,
    "other": 2,
    "reloc-mismatch": 3,
    "size-mismatch": 4,
}

SYM_RE = re.compile(
    r"^(?P<value>[0-9a-f]{8})\s+\S+\s+\S+\s+(?P<sec>\S+)\s+"
    r"(?P<size>[0-9a-f]{8})\s+(?P<name>\S+)\s*$"
)
RELOC_RE = re.compile(
    r"^([0-9a-f]{8})\s+(\S+)\s+(\S+)\s*$"
)


def objdump_text(objfile: pathlib.Path) -> str:
    return subprocess.run(
        [str(OBJDUMP), "-t", str(objfile)], capture_output=True, text=True, check=True
    ).stdout


def func_symbol_span(objfile: pathlib.Path, func: str) -> Optional[tuple[int, int]]:
    """Section-relative (offset, size) for ``func`` in an object's .text."""
    for line in objdump_text(objfile).splitlines():
        m = SYM_RE.match(line)
        if m and m.group("name") == func and m.group("sec") == ".text":
            return int(m.group("value"), 16), int(m.group("size"), 16)
    return None


def text_bytes(objfile: pathlib.Path, start: int, size: int) -> bytes:
    """Raw bytes for one function's span within an object's .text."""
    tmp = objfile.with_suffix(".text.bin")
    subprocess.run(
        [str(OBJCOPY), "-O", "binary", "--only-section=.text", str(objfile), str(tmp)],
        capture_output=True, check=True,
    )
    raw = tmp.read_bytes()
    tmp.unlink(missing_ok=True)
    return raw[start:start + size]


def relocations(
    objfile: pathlib.Path, start: int, size: int
) -> dict[int, tuple[str, str]]:
    """Function-relative offset -> (type, value) for .text relocations."""
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
            absolute = int(offset, 16)
            if start <= absolute < start + size:
                result[absolute - start] = (rtype, value)
    return result


def assemble_target(target_asm: pathlib.Path, target_o: pathlib.Path) -> None:
    """Assemble a prepared target when import.py did not leave target.o."""
    standalone = target_o.with_suffix(".standalone.s")
    prelude = ROOT / "tools" / "permuter" / "prelude.inc"
    standalone.write_text(prelude.read_text() + target_asm.read_text())
    command = shlex.split(pb.ASSEMBLER_COMMAND) + [
        str(standalone), "-o", str(target_o)
    ]
    proc = subprocess.run(
        command, cwd=ROOT, capture_output=True, text=True
    )
    if proc.returncode != 0:
        raise RuntimeError(proc.stderr.strip() or "target assembly failed")


def make_tu_compile_command(
    item: "pb.QueueItem", output: pathlib.Path,
    source_override: Optional[pathlib.Path] = None,
) -> list[str]:
    """Return the Makefile-expanded raw compile command for one C TU.

    ``gmake NON_MATCHING=1`` normally follows compilation with metadata
    post-processing whose fixed-size trim can reject a larger candidate.
    A dry run supplies the exact asm-processor/IDO command, including every
    TU-specific C/optimizer/ISA flag; only its output path is redirected.
    """
    rel_source = item.c_file.relative_to(ROOT).as_posix()
    make_target = f"build_non_matching/{rel_source}.o"
    proc = subprocess.run(
        [
            "gmake", "--no-print-directory", "-n", "-W", rel_source,
            "NON_MATCHING=1", make_target,
        ],
        cwd=ROOT,
        capture_output=True,
        text=True,
    )
    if proc.returncode != 0:
        raise RuntimeError(
            proc.stderr.strip() or "could not expand the TU compile command"
        )
    logical_lines = proc.stdout.replace("\\\n", " ").splitlines()
    candidates = [
        line.strip()
        for line in logical_lines
        if "tools/asm-processor/build.py" in line and rel_source in line
    ]
    if len(candidates) != 1:
        raise RuntimeError(
            f"expected one Makefile compile command, found {len(candidates)}"
        )
    command = shlex.split(candidates[0])
    try:
        out_index = command.index("-o") + 1
    except ValueError as exc:
        raise RuntimeError("Makefile compile command has no -o argument") from exc
    command[out_index] = str(output)
    if source_override is not None:
        try:
            source_index = command.index(rel_source)
        except ValueError as exc:
            raise RuntimeError(
                "Makefile compile command has no source argument"
            ) from exc
        command[source_index] = str(source_override)
    return command


def run_tu_compile(
    item: "pb.QueueItem", output: pathlib.Path, log_path: pathlib.Path,
    source_override: Optional[pathlib.Path] = None,
) -> bool:
    """Run one raw TU compile, retaining diagnostics only under build/."""
    command = make_tu_compile_command(item, output, source_override)
    proc = subprocess.run(
        command, cwd=ROOT, capture_output=True, text=True
    )
    log_path.write_text((proc.stdout or "") + (proc.stderr or ""))
    return proc.returncode == 0 and output.is_file()


def normalized_body(body: str) -> str:
    """Whitespace-insensitive identity check for tracked candidate C."""
    return re.sub(r"\s+", "", body)


def selective_isolation_source(
    item: "pb.QueueItem", out_dir: pathlib.Path
) -> pathlib.Path:
    """Select only this candidate body; retain every other ASM fallback."""
    source_text = item.c_file.read_text(errors="replace")
    blocks = list(pb.iter_nonmatching_blocks(source_text))
    targets = [
        block
        for block in blocks
        if pb.block_function_name(source_text, block) == item.func
    ]
    if len(targets) != 1:
        raise RuntimeError(
            f"expected one NON_MATCHING body for {item.func}, found {len(targets)}"
        )
    target = targets[0]
    isolated = source_text
    for block in reversed(blocks):
        replacement = block.body if block is target else block.fallback
        isolated = isolated[:block.start] + replacement + isolated[block.end:]
    source = out_dir / "selective-context.c"
    source.write_text(isolated)
    return source


def historical_isolation_source(
    item: "pb.QueueItem", out_dir: pathlib.Path
) -> Optional[pathlib.Path]:
    """Recover declaration context lost by a tracked TU consolidation.

    Consolidation retained some candidate bodies verbatim but replaced their
    private typed externs with a shared opaque declaration. Search only
    deleted tracked C files in this TU's directory, and accept one only when
    its parsed candidate body is identical to the current body. This is source
    provenance already in this repository, never an assembly/ROM fallback.
    """
    current_text = item.c_file.read_text(errors="replace")
    current_blocks = [
        block
        for block in pb.iter_nonmatching_blocks(current_text)
        if pb.block_function_name(current_text, block) == item.func
    ]
    if len(current_blocks) != 1:
        return None

    rel_dir = item.c_file.parent.relative_to(ROOT).as_posix()
    history = subprocess.run(
        [
            "git", "log", "--all", "--format=%H", "--diff-filter=D",
            "--", rel_dir,
        ],
        cwd=ROOT,
        capture_output=True,
        text=True,
        check=True,
    ).stdout.splitlines()
    wanted = normalized_body(current_blocks[0].body)
    for commit in history:
        deleted = subprocess.run(
            [
                "git", "diff-tree", "--no-commit-id", "--name-only", "-r",
                "--diff-filter=D", commit, "--", rel_dir,
            ],
            cwd=ROOT,
            capture_output=True,
            text=True,
            check=True,
        ).stdout.splitlines()
        for rel_path in deleted:
            shown = subprocess.run(
                ["git", "show", f"{commit}^:{rel_path}"],
                cwd=ROOT,
                capture_output=True,
                text=True,
            )
            if shown.returncode != 0:
                continue
            old_text = shown.stdout
            for block in pb.iter_nonmatching_blocks(old_text):
                if pb.block_function_name(old_text, block) != item.func:
                    continue
                if normalized_body(block.body) != wanted:
                    continue
                old_asm = pb.GLOBAL_ASM_RE.search(block.fallback)
                current_asm = pb.GLOBAL_ASM_RE.search(current_blocks[0].fallback)
                if old_asm is None or current_asm is None:
                    continue
                old_text = old_text.replace(
                    old_asm.group("path"), current_asm.group("path"), 1
                )
                source = out_dir / "historical-context.c"
                source.write_text(old_text)
                return source
    return None


def compile_tu_fallback(
    item: "pb.QueueItem", output: pathlib.Path, out_dir: pathlib.Path
) -> None:
    """Compile one selected body, then try verified historical context."""
    selective = selective_isolation_source(item, out_dir)
    if run_tu_compile(
        item, output, out_dir / "selective-compile.log", selective
    ):
        return
    output.unlink(missing_ok=True)
    historical = historical_isolation_source(item, out_dir)
    if historical is not None and run_tu_compile(
        item, output, out_dir / "historical-compile.log", historical
    ):
        return
    raise RuntimeError(
        f"raw TU isolation failed (see {out_dir.relative_to(ROOT)}/*compile.log)"
    )


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


class RankingDocumentError(ValueError):
    """The retained ranking cannot be consumed without guessing."""


def _plain_int(value: object, field: str, *, minimum: Optional[int] = None) -> int:
    if type(value) is not int:
        raise RankingDocumentError(f"{field} must be an integer")
    if minimum is not None and value < minimum:
        raise RankingDocumentError(f"{field} must be >= {minimum}")
    return value


def _plain_string(value: object, field: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise RankingDocumentError(f"{field} must be a non-empty string")
    if any(ord(character) < 0x20 or ord(character) == 0x7F for character in value):
        raise RankingDocumentError(f"{field} must not contain control characters")
    return value


def _source_symbol_identity(
    file_name: object, symbol: object, field: str
) -> tuple[str, str]:
    source = _plain_string(file_name, f"{field}.file")
    name = _plain_string(symbol, f"{field}.symbol")
    path = pathlib.PurePosixPath(source)
    if (
        path.is_absolute()
        or ".." in path.parts
        or path.suffix != ".c"
        or source != path.as_posix()
        or not path.parts
        or path.parts[0] != "src"
    ):
        raise RankingDocumentError(
            f"{field}.file must be a relative C source path"
        )
    if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", name):
        raise RankingDocumentError(f"{field}.symbol is not a C identifier")
    return source, name


def _function_row_key(row: object) -> tuple[str, str]:
    if not isinstance(row, dict):
        raise RankingDocumentError("function row is not an object")
    return _source_symbol_identity(row.get("file"), row.get("name"), "function")


def _unresolved_row_key(row: object) -> tuple[str, str]:
    if not isinstance(row, list) or len(row) != 2:
        raise RankingDocumentError(
            "unresolved row needs [[file, name], diagnostic] identity"
        )
    key, diagnostic = row
    if (
        not isinstance(key, list)
        or len(key) != 2
        or not isinstance(diagnostic, str)
    ):
        raise RankingDocumentError(
            "unresolved row needs [[file, name], diagnostic] identity"
        )
    identity = _source_symbol_identity(key[0], key[1], "unresolved")
    _plain_string(diagnostic, "unresolved diagnostic")
    return identity


def validate_ranking_document(
    document: object, *, require_consistent_counts: bool = True
) -> dict[str, object]:
    """Validate the complete persisted schema and exact row identities.

    Pruning may deliberately repair stale derived counts, so its input opts out
    of count consistency while retaining every structural/type/identity check.
    Documentation generation and checking always require full consistency.
    """
    if not isinstance(document, dict):
        raise RankingDocumentError("ranking root is not an object")

    queue_size = _plain_int(document.get("queue_size"), "queue_size", minimum=0)
    resolved = _plain_int(document.get("resolved"), "resolved", minimum=0)
    unresolved_count = _plain_int(
        document.get("unresolved"), "unresolved", minimum=0
    )
    coverage = _plain_int(
        document.get("objdiff_match_pct_coverage"),
        "objdiff_match_pct_coverage",
        minimum=0,
    )
    if type(document.get("objdiff_report_used")) is not bool:
        raise RankingDocumentError("objdiff_report_used must be a boolean")

    functions = document.get("functions")
    unresolved = document.get("unresolved_functions")
    if not isinstance(functions, list) or not isinstance(unresolved, list):
        raise RankingDocumentError(
            "ranking needs functions and unresolved_functions lists"
        )

    function_keys: list[tuple[str, str]] = []
    measured_coverage = 0
    for index, row in enumerate(functions):
        key = _function_row_key(row)
        assert isinstance(row, dict)
        function_keys.append(key)
        prefix = f"functions[{index}]"
        overlay = row.get("overlay")
        tu = _plain_string(row.get("tu"), f"{prefix}.tu")
        source_parts = pathlib.PurePosixPath(key[0]).parts
        if overlay is not None:
            overlay = _plain_int(overlay, f"{prefix}.overlay", minimum=0)
            expected_tu = f"overlays/o{overlay:03d}"
            if len(source_parts) < 3 or "/".join(source_parts[1:3]) != expected_tu:
                raise RankingDocumentError(
                    f"{prefix}.overlay does not match its source path"
                )
        else:
            expected_tu = source_parts[1] if len(source_parts) >= 2 else ""
        if tu != expected_tu:
            raise RankingDocumentError(
                f"{prefix}.tu is {tu!r}, expected {expected_tu!r} from identity"
            )
        size = _plain_int(row.get("size_bytes"), f"{prefix}.size_bytes", minimum=0)
        if size % 4:
            raise RankingDocumentError(f"{prefix}.size_bytes must be word-aligned")
        _plain_int(
            row.get("differing_words"), f"{prefix}.differing_words", minimum=0
        )
        first = row.get("first_mismatch_offset")
        if first is not None:
            first = _plain_int(first, f"{prefix}.first_mismatch_offset", minimum=0)
            if first % 4:
                raise RankingDocumentError(
                    f"{prefix}.first_mismatch_offset must be word-aligned"
                )
        delta = _plain_int(row.get("size_delta"), f"{prefix}.size_delta")
        if delta % 4:
            raise RankingDocumentError(f"{prefix}.size_delta must be word-aligned")
        if size + delta < 0:
            raise RankingDocumentError(f"{prefix}.size_delta makes base size negative")
        differing = int(row["differing_words"])
        if (differing == 0) != (first is None):
            raise RankingDocumentError(
                f"{prefix}.first_mismatch_offset disagrees with differing_words"
            )
        _plain_string(row.get("category"), f"{prefix}.category")
        pct = row.get("objdiff_match_pct")
        if pct is not None:
            if isinstance(pct, bool) or not isinstance(pct, (int, float)):
                raise RankingDocumentError(
                    f"{prefix}.objdiff_match_pct must be a number or null"
                )
            if not math.isfinite(pct) or not 0 <= pct <= 100:
                raise RankingDocumentError(
                    f"{prefix}.objdiff_match_pct must be between 0 and 100"
                )
            measured_coverage += 1

    unresolved_keys = [_unresolved_row_key(row) for row in unresolved]
    all_keys = function_keys + unresolved_keys
    if len(set(all_keys)) != len(all_keys):
        raise RankingDocumentError("ranking contains duplicate function identities")

    if require_consistent_counts:
        expected = {
            "resolved": len(functions),
            "unresolved": len(unresolved),
            "queue_size": len(functions) + len(unresolved),
            "objdiff_match_pct_coverage": measured_coverage,
        }
        actual = {
            "resolved": resolved,
            "unresolved": unresolved_count,
            "queue_size": queue_size,
            "objdiff_match_pct_coverage": coverage,
        }
        for field, expected_value in expected.items():
            if actual[field] != expected_value:
                raise RankingDocumentError(
                    f"{field} is {actual[field]}, expected {expected_value} from rows"
                )
    return document


def prune_stale_document(
    document: object, live_keys: set[tuple[str, str]]
) -> tuple[dict[str, object], list[tuple[str, str]], list[tuple[str, str]]]:
    """Drop retained rows whose exact source/symbol identity is no longer queued.

    This deliberately does not invent measurements for newly queued functions.
    Their identities are returned as ``unranked`` so the caller can report that
    a full regeneration is still needed.
    """
    document = validate_ranking_document(
        document, require_consistent_counts=False
    )
    functions = document.get("functions")
    unresolved = document.get("unresolved_functions")
    assert isinstance(functions, list)
    assert isinstance(unresolved, list)

    function_keys = [_function_row_key(row) for row in functions]
    unresolved_keys = [_unresolved_row_key(row) for row in unresolved]
    all_keys = function_keys + unresolved_keys

    retained_functions = [
        row for row, key in zip(functions, function_keys) if key in live_keys
    ]
    retained_unresolved = [
        row for row, key in zip(unresolved, unresolved_keys) if key in live_keys
    ]
    retained_keys = {key for key in all_keys if key in live_keys}
    removed = sorted(set(all_keys) - live_keys)
    unranked = sorted(live_keys - retained_keys)

    pruned = dict(document)
    pruned["functions"] = retained_functions
    pruned["unresolved_functions"] = retained_unresolved
    pruned["resolved"] = len(retained_functions)
    pruned["unresolved"] = len(retained_unresolved)
    pruned["queue_size"] = len(retained_functions) + len(retained_unresolved)
    pruned["objdiff_match_pct_coverage"] = sum(
        1
        for row in retained_functions
        if isinstance(row, dict) and row.get("objdiff_match_pct") is not None
    )
    validate_ranking_document(pruned)
    return pruned, removed, unranked


def write_text_atomic(path: pathlib.Path, content: str) -> None:
    """Replace one text file only after its complete temp file is written."""
    path.parent.mkdir(parents=True, exist_ok=True)
    output_mode = stat.S_IMODE(path.stat().st_mode) if path.exists() else 0o644
    temporary: Optional[pathlib.Path] = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="w",
            encoding="utf-8",
            dir=path.parent,
            prefix=f".{path.name}.",
            suffix=".tmp",
            delete=False,
        ) as stream:
            temporary = pathlib.Path(stream.name)
            stream.write(content)
            stream.flush()
        temporary.chmod(output_mode)
        temporary.replace(path)
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def write_json_atomic(path: pathlib.Path, document: dict[str, object]) -> None:
    """Validate and atomically replace one persisted ranking document."""
    validate_ranking_document(document)
    write_text_atomic(path, json.dumps(document, indent=2) + "\n")


def _markdown_code(value: object) -> str:
    text = str(value).replace("\\", "\\\\").replace("|", "\\|")
    text = text.replace("`", "\\`")
    return f"`{text}`"


def _markdown_text(value: object) -> str:
    return str(value).replace("\\", "\\\\").replace("|", "\\|")


def render_ranking_markdown(document: object) -> str:
    """Render the authoritative human-readable snapshot from validated JSON."""
    document = validate_ranking_document(document)
    functions = document["functions"]
    unresolved = document["unresolved_functions"]
    assert isinstance(functions, list)
    assert isinstance(unresolved, list)

    queue_size = int(document["queue_size"])
    resolved = int(document["resolved"])
    unresolved_count = int(document["unresolved"])
    coverage = int(document["objdiff_match_pct_coverage"])
    total_size = sum(int(row["size_bytes"]) for row in functions)
    overlays = sorted(
        {int(row["overlay"]) for row in functions if row["overlay"] is not None}
    )
    resident_tus = sorted(
        {str(row["tu"]) for row in functions if row["overlay"] is None}
    )

    lines = [
        "## Current generated ranking snapshot",
        "",
        "> Generated by `tools/nm_ranking.py --write-doc` from",
        "> `config/nonmatching-ranking.us.json`. Do not edit this region by hand;",
        "> `--check-doc` and `gmake check-docs` fail on any drift.",
        "",
        f"The snapshot contains **{queue_size:,} queued identities**: "
        f"**{resolved:,} resolved measurements** and **{unresolved_count:,} "
        f"unresolved identities**. Resolved target size totals **{total_size:,} "
        f"bytes ({total_size / 1024:.1f} KiB)**.",
        "",
        f"Resolved rows span **{len(overlays):,} overlays** and "
        f"**{len(resident_tus):,} resident TU "
        f"{'group' if len(resident_tus) == 1 else 'groups'}**"
        + (f" ({', '.join(_markdown_code(tu) for tu in resident_tus)})" if resident_tus else "")
        + ".",
        "",
        "A supplementary objdiff report "
        + ("was supplied" if document["objdiff_report_used"] else "was not supplied")
        + f"; `objdiff_match_pct` covers **{coverage:,} / {resolved:,}** resolved rows.",
        "",
        "### Category distribution",
        "",
        "| Category | Count | Share of resolved |",
        "|---|---:|---:|",
    ]

    category_counts = Counter(str(row["category"]) for row in functions)
    category_order: list[str] = []
    for row in functions:
        category = str(row["category"])
        if category not in category_order:
            category_order.append(category)
    for category in category_order:
        count = category_counts[category]
        share = count * 100 / resolved if resolved else 0.0
        lines.append(f"| {_markdown_code(category)} | {count:,} | {share:.1f}% |")

    lines.extend([
        "",
        "### Differing-word thresholds",
        "",
        "| Threshold | Count |",
        "|---|---:|",
    ])
    for threshold in (5, 10, 20):
        count = sum(int(row["differing_words"]) <= threshold for row in functions)
        lines.append(f"| `differing_words <= {threshold}` | {count:,} |")

    lines.extend([
        "",
        "### Complete ranked queue",
        "",
        "Rank is the persisted `functions` array order. The exact identity is",
        "(`file`, `symbol`), so repeated symbol spellings in different translation",
        "units remain distinct.",
        "",
        "| Rank | File | Symbol | Overlay/TU | Category | Target bytes | Diff words | First mismatch | Size delta | Objdiff% |",
        "|---:|---|---|---|---|---:|---:|---:|---:|---:|",
    ])
    for rank, row in enumerate(functions, 1):
        location = (
            f"o{int(row['overlay']):03d}"
            if row["overlay"] is not None
            else str(row["tu"])
        )
        first = row["first_mismatch_offset"]
        pct = row["objdiff_match_pct"]
        lines.append(
            "| "
            + " | ".join([
                str(rank),
                _markdown_code(row["file"]),
                _markdown_code(row["name"]),
                _markdown_code(location),
                _markdown_code(row["category"]),
                f"{int(row['size_bytes']):,}",
                f"{int(row['differing_words']):,}",
                "—" if first is None else f"{int(first):,}",
                f"{int(row['size_delta']):,}",
                "—" if pct is None else json.dumps(pct, allow_nan=False),
            ])
            + " |"
        )

    lines.extend([
        "",
        "### Unresolved identities",
        "",
    ])
    if unresolved:
        lines.extend([
            "| File | Symbol | Diagnostic |",
            "|---|---|---|",
        ])
        for key, diagnostic in sorted(
            unresolved, key=lambda row: (row[0][0], row[0][1])
        ):
            lines.append(
                f"| {_markdown_code(key[0])} | {_markdown_code(key[1])} | "
                f"{_markdown_text(diagnostic)} |"
            )
    else:
        lines.append("None.")
    return "\n".join(lines) + "\n"


def replace_generated_markdown(document_text: str, generated: str) -> str:
    """Replace exactly one marked region while preserving authored text."""
    if document_text.count(DOC_BEGIN) != 1 or document_text.count(DOC_END) != 1:
        raise RankingDocumentError(
            "documentation needs exactly one generated begin/end marker"
        )
    begin = document_text.index(DOC_BEGIN)
    end = document_text.index(DOC_END)
    if begin >= end:
        raise RankingDocumentError("documentation generated markers are reversed")
    begin_content = begin + len(DOC_BEGIN)
    return (
        document_text[:begin_content]
        + "\n"
        + generated
        + DOC_END
        + document_text[end + len(DOC_END):]
    )


def expected_document_text(
    ranking_document: object, documentation_path: pathlib.Path
) -> tuple[str, str]:
    current = documentation_path.read_text(encoding="utf-8")
    expected = replace_generated_markdown(
        current, render_ranking_markdown(ranking_document)
    )
    return current, expected


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
    # Successful isolated imports must use the same Makefile-expanded recipe
    # as the real TU. The static path-only flag group misses per-file
    # overrides such as track.c's -Wab,-r4300_mul and can rank the wrong ISA
    # result even though the fallback path below preserves the real command.
    recipe = pb.build_recipe_for(item.c_file)
    pb.write_settings_toml(settings_path, recipe.flags)
    import_error: Optional[str] = None
    scratch: Optional[pathlib.Path] = None
    try:
        target_asm = pb.prepare_target_asm(item, out_dir)
        scratch = pb.run_import(item, out_dir, settings_path, target_asm)
    except Exception as e:  # noqa: BLE001 - reported per-function, not fatal
        import_error = str(e)
        try:
            target_asm = pb.prepare_target_asm(item, out_dir)
        except Exception as target_error:  # noqa: BLE001
            return None, f"{item.func} ({item.rel_c_file}): {target_error}"

    base_o = scratch / "base.o" if scratch is not None else out_dir / "base.o"
    target_o = scratch / "target.o" if scratch is not None else out_dir / "target.o"
    try:
        if not target_o.is_file():
            assemble_target(target_asm, target_o)
        if not base_o.is_file():
            base_o = out_dir / "base-tu.o"
            base_o.unlink(missing_ok=True)
            compile_tu_fallback(item, base_o, out_dir)
    except Exception as e:  # noqa: BLE001 - reported per-function, not fatal
        prefix = f"import failed ({import_error}); " if import_error else ""
        return None, f"{item.func} ({item.rel_c_file}): {prefix}{e}"

    target_span = func_symbol_span(target_o, item.func)
    base_span = func_symbol_span(base_o, item.func)
    if target_span is None or base_span is None:
        return None, f"{item.func} ({item.rel_c_file}): couldn't find .text symbol in base.o/target.o"
    target_start, target_size = target_span
    base_start, base_size = base_span

    base_words = words_of(text_bytes(base_o, base_start, base_size))
    target_words = words_of(text_bytes(target_o, target_start, target_size))
    base_reloc = relocations(base_o, base_start, base_size)
    target_reloc = relocations(target_o, target_start, target_size)

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
    ap.add_argument(
        "--doc",
        type=pathlib.Path,
        default=DEFAULT_DOC,
        help=f"generated Markdown document (default {DEFAULT_DOC})",
    )
    doc_mode = ap.add_mutually_exclusive_group()
    doc_mode.add_argument(
        "--write-doc",
        action="store_true",
        help="validate --out and update only the marked region in --doc",
    )
    doc_mode.add_argument(
        "--check-doc",
        action="store_true",
        help="validate --out and fail if the marked region in --doc has drifted",
    )
    ap.add_argument("--top", type=int, default=None, help="only print the top N rows")
    ap.add_argument("--markdown", action="store_true", help="print the table as markdown")
    ap.add_argument("--no-table", action="store_true", help="skip printing the table")
    ap.add_argument("--limit", type=int, default=None,
                     help="only process the first N queue items (debugging)")
    ap.add_argument(
        "--prune-stale",
        action="store_true",
        help="without compiling, remove rows from --out whose exact file/symbol "
             "identity is no longer in the current NON_MATCHING queue; validates "
             "the retained document and reports newly queued unranked functions",
    )
    args = ap.parse_args()

    if args.write_doc or args.check_doc:
        incompatible = []
        if args.objdiff_report is not None:
            incompatible.append("--objdiff-report")
        if args.limit is not None:
            incompatible.append("--limit")
        if args.top is not None:
            incompatible.append("--top")
        if args.markdown:
            incompatible.append("--markdown")
        if args.no_table:
            incompatible.append("--no-table")
        if args.prune_stale:
            incompatible.append("--prune-stale")
        if incompatible:
            print(
                "error: documentation mode cannot be combined with "
                + ", ".join(incompatible),
                file=sys.stderr,
            )
            return 2
        try:
            document = json.loads(args.out.read_text(encoding="utf-8"))
            current_doc, expected_doc = expected_document_text(document, args.doc)
        except (OSError, json.JSONDecodeError, RankingDocumentError) as exc:
            print(
                f"error: refusing to process ranking documentation: {exc}",
                file=sys.stderr,
            )
            return 2
        if args.write_doc:
            write_text_atomic(args.doc, expected_doc)
            print(f"updated generated ranking in {args.doc}", file=sys.stderr)
            return 0
        if current_doc != expected_doc:
            print(
                f"error: generated ranking documentation is stale: {args.doc}",
                file=sys.stderr,
            )
            sys.stderr.writelines(
                difflib.unified_diff(
                    current_doc.splitlines(keepends=True),
                    expected_doc.splitlines(keepends=True),
                    fromfile=str(args.doc),
                    tofile=f"{args.doc} (generated)",
                )
            )
            return 1
        print(f"generated ranking documentation is current: {args.doc}")
        return 0

    if args.prune_stale:
        incompatible = []
        if args.objdiff_report is not None:
            incompatible.append("--objdiff-report")
        if args.limit is not None:
            incompatible.append("--limit")
        if args.top is not None:
            incompatible.append("--top")
        if args.markdown:
            incompatible.append("--markdown")
        if incompatible:
            print(
                "error: --prune-stale cannot be combined with "
                + ", ".join(incompatible),
                file=sys.stderr,
            )
            return 2
        try:
            document = json.loads(args.out.read_text(encoding="utf-8"))
            live_keys = {
                (item.rel_c_file, item.func) for item in pb.discover_queue()
            }
            pruned, removed, unranked = prune_stale_document(document, live_keys)
            expected_doc: Optional[str] = None
            if args.out.resolve() == DEFAULT_OUT.resolve():
                _, expected_doc = expected_document_text(pruned, args.doc)
            write_json_atomic(args.out, pruned)
            if expected_doc is not None:
                write_text_atomic(args.doc, expected_doc)
        except (OSError, json.JSONDecodeError, RankingDocumentError) as exc:
            print(
                f"error: refusing to prune {args.out}: {exc}", file=sys.stderr
            )
            return 2
        print(
            f"pruned {len(removed)} stale row(s); "
            f"retained {pruned['queue_size']} ranked/unresolved row(s)",
            file=sys.stderr,
        )
        if unranked:
            print(
                f"note: {len(unranked)} live queue item(s) are absent from the "
                "snapshot and remain unranked; run a full ranking pass to add them",
                file=sys.stderr,
            )
        return 0

    # The permuter is intentionally installed separately from this repository.
    # Fail before touching the committed queue when that dependency is absent;
    # otherwise every item reports the same import error and a successful exit
    # silently replaces a useful ranking with an empty one.
    permuter_required = [
        ROOT / "tools" / "permuter" / "import.py",
        ROOT / "tools" / "permuter" / "prelude.inc",
    ]
    missing = [path for path in permuter_required if not path.is_file()]
    if missing:
        print(
            "error: decomp-permuter is not installed; refusing to overwrite "
            f"{args.out}",
            file=sys.stderr,
        )
        for path in missing:
            print(f"  missing: {path.relative_to(ROOT)}", file=sys.stderr)
        print(
            "install decomp-permuter under tools/permuter as described in "
            "README.md, then rerun this command",
            file=sys.stderr,
        )
        return 2

    WORK_DIR.mkdir(parents=True, exist_ok=True)

    queue = pb.discover_queue()
    if args.limit:
        queue = queue[: args.limit]

    results: list[FuncResult] = []
    errors: list[tuple[tuple[str, str], str]] = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for item, (result, error) in zip(queue, pool.map(process_item, queue)):
            if result is not None:
                results.append(result)
            if error is not None:
                errors.append(((item.rel_c_file, item.func), error))

    # A full ranking pass can overlap matching work for hours. Re-scan the
    # canonical source immediately before publishing so functions promoted
    # while this process was compiling do not survive as stale ranking rows.
    # Filter the original queue too, preserving --limit and the reported
    # queue/resolution counts for the exact set this invocation processed.
    live_keys = {
        (item.rel_c_file, item.func)
        for item in pb.discover_queue_from_source_scan()
    }
    queue = [
        item for item in queue
        if (item.rel_c_file, item.func) in live_keys
    ]
    results = [
        result for result in results
        if (result.file, result.name) in live_keys
    ]
    errors = [(key, error) for key, error in errors if key in live_keys]

    pct_by_name = load_objdiff_pct(args.objdiff_report)
    for r in results:
        r.objdiff_match_pct = pct_by_name.get(r.name)

    results.sort(key=sort_key)

    if queue and not results:
        print(
            f"error: 0/{len(queue)} candidates resolved; refusing to overwrite "
            f"{args.out}",
            file=sys.stderr,
        )
        if errors:
            print(f"first failure: {errors[0][1]}", file=sys.stderr)
        return 2

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
        "unresolved_functions": [[list(key), error] for key, error in errors],
    }
    expected_doc = None
    if args.out.resolve() == DEFAULT_OUT.resolve():
        _, expected_doc = expected_document_text(out_doc, args.doc)
    write_json_atomic(args.out, out_doc)
    if expected_doc is not None:
        write_text_atomic(args.doc, expected_doc)

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
