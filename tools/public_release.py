#!/usr/bin/env python3
"""Reconcile and preflight one public release without ever pushing it.

The default mode is read-only: generators rerender in check mode, release
metrics are compared with the named remote-tracking branch, and the complete
gate suite runs.  ``--write-derived`` is the only mode that may alter tracked
files, and it invokes only the explicit public-safe generators listed below.
This tool has no merge, copy, fetch, or push operation.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import json
import os
from pathlib import Path
import re
import signal
import subprocess
import sys
import time
from typing import Iterable

sys.dont_write_bytecode = True

import release_gate
import overlay_atlas
from system_health import collect_health, format_report


READ_ONLY_GENERATORS = (
    ("overlay-atlas", "gmake", "-j1", "overlay-atlas"),
    (
        "postprocess-audit",
        sys.executable,
        "tools/postprocess_audit.py",
        "--check",
    ),
    ("overlay-donors", "gmake", "-j1", "overlay-donors"),
)
WRITE_GENERATORS = (
    ("overlay-atlas", "gmake", "-j1", "overlay-atlas-write"),
    ("atlas-digest", sys.executable, "tools/refresh_atlas_digest.py"),
    (
        "postprocess-audit",
        sys.executable,
        "tools/postprocess_audit.py",
        "--write",
    ),
    ("overlay-symbols", "gmake", "-j1", "overlay-syms"),
    ("scoreboard", "gmake", "-j1", "scoreboard"),
)
DERIVED_PATHS = {
    "README.md",
    "config/overlay-donors.us.json",
    "config/overlays.us.json",
    "config/postprocess-audit.us.json",
    "mickey.us.yaml",
    "overlay_undefined_syms.us.txt",
}

# Construct operator-only names in pieces so this public-safe scanner does not
# diagnose its own source.  These are path rules, not content allowlists.
FORBIDDEN_PATH_PARTS = (
    ("." + "codex",),
    ("." + "decomp-workbench",),
    ("base" + "roms",),
    ("a" + "sm",),
    ("ass" + "ets",),
    ("expec" + "ted",),
    ("tools", "i" + "do"),
    ("tools", "bin" + "utils"),
)
FORBIDDEN_BASENAMES = {"AG" + "ENTS.md", "CLA" + "UDE.md"}
MESSAGE_MARKERS = (
    re.compile(r"(?im)^co-authored-by:.*(?:codex|claude|agent|bot)"),
    re.compile(r"(?im)^generated-by:"),
)

SCOREBOARD_BEGIN = "<!-- SCOREBOARD_BEGIN -->"
SCOREBOARD_END = "<!-- SCOREBOARD_END -->"
METRIC_PATTERNS = (
    ("functions", re.compile(r"^functions\s+(\d+)\s*/\s*(\d+)", re.M)),
    ("resident C bytes", re.compile(r"^\.text bytes\s+(\d+)\s*/\s*(\d+)", re.M)),
    ("verified asm bytes", re.compile(r"^verified asm\s+(\d+)\s*/\s*(\d+)", re.M)),
    ("overlay C bytes", re.compile(r"^overlay C\s+(\d+)\s*/\s*(\d+)", re.M)),
    ("whole resolved bytes", re.compile(r"^whole resolved\s+(\d+)\s*/\s*(\d+)", re.M)),
    ("named functions", re.compile(r"^named\s+(\d+)\s*/\s*(\d+)", re.M)),
    ("adopted symbols", re.compile(r"^symbols\s+(\d+)\b", re.M)),
    ("decompiled bytes", re.compile(r"^decompiled\s+(\d+)\s*/\s*(\d+)", re.M)),
    ("GLOBAL_ASM bytes", re.compile(r"^GLOBAL_ASM remaining\s+(\d+)\s*/\s*(\d+)", re.M)),
    ("NON_MATCHING bytes", re.compile(r"^NON_MATCHING\s+(\d+)\s*/\s*(\d+)", re.M)),
)


class PublicReleaseError(RuntimeError):
    pass


@dataclass(frozen=True)
class ReleaseContext:
    repo: Path
    branch: str
    remote: str
    base_ref: str
    base_oid: str
    head_oid: str
    fetch_url: str
    push_url: str
    outgoing_commits: tuple[str, ...]


@dataclass(frozen=True)
class Metric:
    value: int
    total: int | None


def _git(
    repo: Path, *args: str, check: bool = True, binary: bool = False
) -> str | bytes:
    result = subprocess.run(
        ["git", *args],
        cwd=repo,
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=not binary,
    )
    if check and result.returncode != 0:
        stderr = result.stderr if not binary else result.stderr.decode(errors="replace")
        stdout = result.stdout if not binary else result.stdout.decode(errors="replace")
        detail = stderr.strip() or stdout.strip() or f"exit {result.returncode}"
        raise PublicReleaseError(f"git {' '.join(args)}: {detail}")
    return result.stdout


def _repo_root(start: Path) -> Path:
    return Path(str(_git(start, "rev-parse", "--show-toplevel")).strip()).resolve()


def _release_context(
    repo: Path, branch: str, remote: str, *, require_clean: bool
) -> ReleaseContext:
    current = str(_git(repo, "branch", "--show-current")).strip()
    if current != branch:
        raise PublicReleaseError(
            f"current branch is {current or '(detached)'!r}, expected {branch!r}"
        )
    if require_clean:
        dirt = str(
            _git(repo, "status", "--porcelain=v1", "--untracked-files=no")
        ).strip()
        if dirt:
            raise PublicReleaseError("tracked worktree/index dirt is present")

    fetch_url = str(_git(repo, "remote", "get-url", remote)).strip()
    push_url = str(_git(repo, "remote", "get-url", "--push", remote)).strip()
    if not fetch_url or not push_url:
        raise PublicReleaseError(f"remote {remote!r} lacks a fetch or push URL")
    url_findings = release_gate._scan_text(
        f"remote {remote} URL", fetch_url + "\n" + push_url
    )
    if url_findings:
        raise PublicReleaseError("remote URL failed release text scan")

    base_ref = f"refs/remotes/{remote}/{branch}"
    probe = subprocess.run(
        ["git", "show-ref", "--verify", "--quiet", base_ref], cwd=repo, check=False
    )
    if probe.returncode != 0:
        raise PublicReleaseError(
            f"missing local comparison ref {base_ref!r}; fetch it outside this tool"
        )
    ancestor = subprocess.run(
        ["git", "merge-base", "--is-ancestor", base_ref, "HEAD"],
        cwd=repo,
        check=False,
    )
    if ancestor.returncode != 0:
        raise PublicReleaseError(
            f"HEAD is not a fast-forward descendant of {remote}/{branch}"
        )

    base_oid = str(_git(repo, "rev-parse", base_ref)).strip()
    head_oid = str(_git(repo, "rev-parse", "HEAD")).strip()
    commits = tuple(
        line
        for line in str(
            _git(repo, "rev-list", "--reverse", f"{base_ref}..HEAD")
        ).splitlines()
        if line
    )
    return ReleaseContext(
        repo=repo,
        branch=branch,
        remote=remote,
        base_ref=base_ref,
        base_oid=base_oid,
        head_oid=head_oid,
        fetch_url=fetch_url,
        push_url=push_url,
        outgoing_commits=commits,
    )


def _forbidden_path(path: str) -> bool:
    parts = tuple(part for part in Path(path).parts if part not in {"", "."})
    if parts and parts[-1] in FORBIDDEN_BASENAMES:
        return True
    return any(parts[: len(prefix)] == prefix for prefix in FORBIDDEN_PATH_PARTS)


def _tree_entries(repo: Path, ref: str) -> Iterable[tuple[str, str]]:
    raw = bytes(_git(repo, "ls-tree", "-r", "-z", "--full-tree", ref, binary=True))
    for record in raw.split(b"\0"):
        if not record:
            continue
        header, raw_path = record.split(b"\t", 1)
        _mode, kind, oid = header.decode("ascii").split()
        if kind == "blob":
            yield oid, raw_path.decode("utf-8", errors="surrogateescape")


def _blob(repo: Path, oid: str) -> bytes:
    return bytes(_git(repo, "cat-file", "blob", oid, binary=True))


def _index_entries(repo: Path) -> Iterable[tuple[str, str]]:
    raw = bytes(_git(repo, "ls-files", "--stage", "-z", binary=True))
    for record in raw.split(b"\0"):
        if not record:
            continue
        header, raw_path = record.split(b"\t", 1)
        mode, _oid, stage = header.decode("ascii").split()
        if stage != "0":
            raise PublicReleaseError("index contains unmerged entries")
        yield mode, raw_path.decode("utf-8", errors="surrogateescape")


def _scan_payload(label: str, payload: bytes) -> list[str]:
    if b"\0" in payload:
        return []
    text = payload.decode("utf-8", errors="replace")
    return release_gate._scan_text(label, text)


def _scan_release(ctx: ReleaseContext, *, include_worktree: bool) -> list[str]:
    findings: list[str] = []
    seen_blobs: set[str] = set()
    for ref in ctx.outgoing_commits:
        short = ref[:12]
        message = str(_git(ctx.repo, "show", "-s", "--format=%B", ref))
        findings.extend(release_gate._scan_text(f"commit {short} message", message))
        for pattern in MESSAGE_MARKERS:
            if pattern.search(message):
                findings.append(f"commit {short} message: pattern {pattern.pattern!r}")

    refs = (*ctx.outgoing_commits, ctx.head_oid)
    for ref in dict.fromkeys(refs):
        short = ref[:12]
        for oid, path in _tree_entries(ctx.repo, ref):
            if _forbidden_path(path):
                findings.append(f"commit {short}: forbidden tracked path {path}")
            if oid in seen_blobs:
                continue
            seen_blobs.add(oid)
            findings.extend(_scan_payload(f"commit {short}:{path}", _blob(ctx.repo, oid)))

    if include_worktree:
        for mode, path in _index_entries(ctx.repo):
            if _forbidden_path(path):
                findings.append(f"worktree: forbidden tracked path {path}")
            if mode == "160000":
                # A gitlink's tracked payload is its commit ID, already covered
                # by the outgoing tree scan. An uninitialized submodule is a
                # valid public checkout and has no worktree file to inspect.
                continue
            full = ctx.repo / path
            if full.is_symlink():
                payload = os.readlink(full).encode("utf-8", errors="surrogateescape")
            elif full.is_file():
                payload = full.read_bytes()
            else:
                findings.append(f"worktree: tracked path is missing: {path}")
                continue
            findings.extend(_scan_payload(f"worktree:{path}", payload))
    return sorted(set(findings))


def _check_diff_hygiene(ctx: ReleaseContext, *, include_worktree: bool) -> None:
    outgoing = str(_git(ctx.repo, "diff", "--check", f"{ctx.base_ref}..HEAD"))
    if outgoing.strip():
        raise PublicReleaseError("outgoing committed diff has whitespace errors")
    if include_worktree:
        working = str(_git(ctx.repo, "diff", "--check"))
        cached = str(_git(ctx.repo, "diff", "--cached", "--check"))
        if working.strip() or cached.strip():
            raise PublicReleaseError("tracked worktree/index diff has whitespace errors")


def _show_file(repo: Path, ref: str, path: str) -> str | None:
    result = subprocess.run(
        ["git", "show", f"{ref}:{path}"],
        cwd=repo,
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    return result.stdout if result.returncode == 0 else None


def _scoreboard_metrics(text: str | None) -> dict[str, Metric]:
    if text is None:
        return {}
    start = text.find(SCOREBOARD_BEGIN)
    end = text.find(SCOREBOARD_END)
    if start < 0 or end < start:
        return {}
    block = text[start:end]
    metrics: dict[str, Metric] = {}
    for label, pattern in METRIC_PATTERNS:
        match = pattern.search(block)
        if match:
            metrics[label] = Metric(
                int(match.group(1)), int(match.group(2)) if match.lastindex == 2 else None
            )
    return metrics


def _metric_delta_lines(old_text: str | None, new_text: str | None) -> list[str]:
    old = _scoreboard_metrics(old_text)
    new = _scoreboard_metrics(new_text)
    lines: list[str] = []
    for label, _pattern in METRIC_PATTERNS:
        if label not in old or label not in new:
            continue
        before, after = old[label], new[label]
        delta = after.value - before.value
        total = ""
        if before.total != after.total:
            total = f"; total {before.total} -> {after.total}"
        lines.append(
            f"metric {label}: {before.value} -> {after.value} ({delta:+d}){total}"
        )
    return lines


def _read_json(text: str | None) -> dict | None:
    if text is None:
        return None
    try:
        value = json.loads(text)
    except json.JSONDecodeError as exc:
        raise PublicReleaseError(f"invalid overlay atlas JSON: {exc}") from exc
    return value if isinstance(value, dict) else None


def _exact_range_delta(old: dict, new: dict) -> dict[str, object]:
    """Use the atlas tool's canonical, fail-closed exact-C identity model."""
    try:
        return overlay_atlas.compare_exact_c_atlases(old, new)
    except overlay_atlas.AtlasDeltaError as exc:
        raise PublicReleaseError(f"overlay exact-range delta is invalid: {exc}") from exc


def _delta_report(ctx: ReleaseContext) -> list[str]:
    old_readme = _show_file(ctx.repo, ctx.base_ref, "README.md")
    current_readme = (ctx.repo / "README.md").read_text(errors="replace")
    lines = _metric_delta_lines(old_readme, current_readme)
    if not lines:
        lines.append("metric deltas: unavailable (scoreboard absent or unparseable)")

    old_atlas = _read_json(
        _show_file(ctx.repo, ctx.base_ref, "config/overlays.us.json")
    )
    current_path = ctx.repo / "config/overlays.us.json"
    new_atlas = _read_json(
        current_path.read_text(errors="replace") if current_path.is_file() else None
    )
    if old_atlas is None or new_atlas is None:
        lines.append("overlay exact-range deltas: unavailable")
        return lines
    delta = _exact_range_delta(old_atlas, new_atlas)
    totals = delta["totals"]
    assert isinstance(totals, dict)
    promoted = int(totals["promotion_bytes"])
    retracted = int(totals["retraction_bytes"])
    lines.append(
        f"overlay exact ranges: promoted={promoted} retracted={retracted} "
        f"net={promoted - retracted:+d}"
    )
    for kind, key in (("promotion", "promotions"), ("retraction", "retractions")):
        rows = delta[key]
        assert isinstance(rows, list)
        for row in rows:
            lines.append(
                f"  {kind} overlay {int(row['overlay'])} "
                f"+0x{int(row['offset']):X}..+0x{int(row['end_offset']):X} "
                f"{int(row['size'])} bytes source={row['source']}"
            )
    return lines


def _terminate(proc: subprocess.Popen[bytes]) -> None:
    try:
        os.killpg(proc.pid, signal.SIGTERM)
        proc.wait(timeout=5)
    except ProcessLookupError:
        return
    except subprocess.TimeoutExpired:
        try:
            os.killpg(proc.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass


def _run_commands(
    repo: Path,
    commands: tuple[tuple[str, ...], ...],
    *,
    niceness: int,
    timeout: float,
) -> bool:
    log_dir = repo / "build" / "public-release" / "reconcile"
    log_dir.mkdir(parents=True, exist_ok=True)
    for index, command_row in enumerate(commands, 1):
        label, *command = command_row
        report = collect_health(repo)
        health_path = log_dir / f"{index:02d}-{label}-health.log"
        health_path.write_text(format_report(report) + "\n")
        if report.verdict != "HEALTHY":
            print(f"reconcile {label}: {report.verdict} log={health_path}")
            return False
        log_path = log_dir / f"{index:02d}-{label}.log"
        started = time.monotonic()
        with log_path.open("wb") as log:
            log.write(("command: " + " ".join(command) + "\n").encode())
            log.flush()
            try:
                proc = subprocess.Popen(
                    command,
                    cwd=repo,
                    stdout=log,
                    stderr=subprocess.STDOUT,
                    start_new_session=True,
                    preexec_fn=(lambda: os.nice(niceness)) if niceness else None,
                )
                return_code = proc.wait(timeout=timeout)
            except subprocess.TimeoutExpired:
                log.write(f"timeout after {timeout:.1f}s\n".encode())
                _terminate(proc)
                return_code = 124
            except OSError as exc:
                log.write(f"launch failed: {exc}\n".encode())
                return_code = 127
        status = "PASS" if return_code == 0 else "FAIL"
        print(
            f"reconcile {label}: {status} {time.monotonic() - started:.1f}s "
            f"log={log_path}"
        )
        if return_code != 0:
            return False
    return True


def _release_gate_command(
    ctx: ReleaseContext,
    *,
    niceness: int,
    timeout: float,
    clean: bool,
) -> list[str]:
    command = [
        sys.executable,
        "tools/release_gate.py",
        "--repo",
        str(ctx.repo),
        "--log-dir",
        "build/public-release/gates",
        "--nice",
        str(niceness),
        "--timeout",
        str(timeout),
    ]
    if clean:
        command.extend(
            [
                "--public-push-preflight",
                "--branch",
                ctx.branch,
                "--remote",
                ctx.remote,
            ]
        )
    return command


def _reconciliation_commands(
    ctx: ReleaseContext, *, write_derived: bool
) -> tuple[tuple[str, ...], ...]:
    commands = WRITE_GENERATORS if write_derived else READ_ONLY_GENERATORS
    if not ctx.outgoing_commits:
        return commands
    range_check = (
        "outgoing-cleanroom",
        "bash",
        "tools/cleanroom_check.sh",
        "--range",
        f"{ctx.base_ref}..HEAD",
    )
    return (range_check, *commands)


def _tracked_dirt(repo: Path) -> str:
    return str(_git(repo, "status", "--porcelain=v1", "--untracked-files=no")).strip()


def _unexpected_derived_changes(repo: Path) -> list[str]:
    names = {
        line
        for line in str(_git(repo, "diff", "--name-only", "HEAD")).splitlines()
        if line
    }
    names.update(
        line
        for line in str(_git(repo, "diff", "--cached", "--name-only", "HEAD")).splitlines()
        if line
    )
    return sorted(names - DERIVED_PATHS)


def _print_context(ctx: ReleaseContext) -> None:
    print(
        f"release context: branch={ctx.branch} remote={ctx.remote} "
        f"base={ctx.base_oid[:12]} head={ctx.head_oid[:12]} "
        f"outgoing={len(ctx.outgoing_commits)}"
    )
    print(f"remote fetch URL: {ctx.fetch_url}")
    print(f"remote push URL:  {ctx.push_url}")


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=Path.cwd())
    parser.add_argument("--remote", required=True)
    parser.add_argument("--branch", required=True)
    parser.add_argument(
        "--write-derived",
        action="store_true",
        help="run the explicit derived-artifact writers; still never push",
    )
    parser.add_argument("--nice", type=int, default=15)
    parser.add_argument("--timeout", type=float, default=3600.0, metavar="SECONDS")
    return parser


def main(argv: list[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    if not 0 <= args.nice <= 19:
        raise SystemExit("--nice must be between 0 and 19")
    if args.timeout <= 0:
        raise SystemExit("--timeout must be positive")
    try:
        repo = _repo_root(args.repo.resolve())
        ctx = _release_context(repo, args.branch, args.remote, require_clean=True)
        _check_diff_hygiene(ctx, include_worktree=False)
        findings = _scan_release(ctx, include_worktree=False)
        if findings:
            raise PublicReleaseError("release text scan failed: " + "; ".join(findings[:8]))
    except PublicReleaseError as exc:
        print(f"PUBLIC RELEASE PREFLIGHT FAIL: {exc}", file=sys.stderr)
        return 2

    _print_context(ctx)
    mode = "WRITE-DERIVED" if args.write_derived else "DRY-RUN"
    print(f"mode: {mode}; network mutation=disabled; push=disabled")
    generators = _reconciliation_commands(
        ctx, write_derived=args.write_derived
    )
    if not _run_commands(repo, generators, niceness=args.nice, timeout=args.timeout):
        print("PUBLIC RELEASE PREFLIGHT FAIL: reconciliation command failed", file=sys.stderr)
        return 1

    dirty = _tracked_dirt(repo)
    try:
        if args.write_derived:
            unexpected = _unexpected_derived_changes(repo)
            if unexpected:
                raise PublicReleaseError(
                    "derived generators changed paths outside the allowlist: "
                    + ", ".join(unexpected)
                )
        ctx = _release_context(
            repo, args.branch, args.remote, require_clean=not args.write_derived
        )
        _check_diff_hygiene(ctx, include_worktree=bool(dirty))
        findings = _scan_release(ctx, include_worktree=bool(dirty))
        if findings:
            raise PublicReleaseError("release text scan failed: " + "; ".join(findings[:8]))
    except PublicReleaseError as exc:
        print(f"PUBLIC RELEASE PREFLIGHT FAIL: {exc}", file=sys.stderr)
        return 2

    gate_command = _release_gate_command(
        ctx, niceness=args.nice, timeout=args.timeout, clean=not bool(dirty)
    )
    gate_result = subprocess.run(gate_command, cwd=repo, check=False)
    if gate_result.returncode != 0:
        print("PUBLIC RELEASE PREFLIGHT FAIL: release gates failed", file=sys.stderr)
        return gate_result.returncode

    try:
        final_ctx = _release_context(
            repo, args.branch, args.remote, require_clean=not args.write_derived
        )
        final_dirty = _tracked_dirt(repo)
        _check_diff_hygiene(final_ctx, include_worktree=bool(final_dirty))
        findings = _scan_release(final_ctx, include_worktree=bool(final_dirty))
        if findings:
            raise PublicReleaseError(
                "post-gate release text scan failed: " + "; ".join(findings[:8])
            )
        for line in _delta_report(final_ctx):
            print(line)
    except PublicReleaseError as exc:
        print(f"PUBLIC RELEASE PREFLIGHT FAIL: {exc}", file=sys.stderr)
        return 2

    if final_dirty:
        print("derived artifacts changed:")
        print(final_dirty)
        print(
            "PUBLIC RELEASE RECONCILIATION PASS: review and commit only these "
            "generated changes, then rerun dry-run preflight; push=disabled"
        )
    else:
        print("PUBLIC RELEASE PREFLIGHT PASS: release-ready=yes; push=disabled")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
