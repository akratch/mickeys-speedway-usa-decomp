#!/usr/bin/env python3
"""Run release gates serially with compact output and complete build logs.

Public-push mode is a preflight only. This program never pushes.
"""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
import os
from pathlib import Path
import re
import signal
import subprocess
import sys
import time

sys.dont_write_bytecode = True

from system_health import collect_health, format_report


GATES = (
    "verify",
    "check-tooling",
    "cleanroom",
    "check-docs",
    "check-scoreboard",
    "check-overlay-syms",
)
PRIVATE_MARKERS = (
    re.compile("/" + r"Users/(?!<)"),
    re.compile("campaign/" + "unchain"),
    re.compile(r"(?:^|/)\.codex" + "/"),
    re.compile(r"\bAGENTS" + r"\.md\b"),
    re.compile(r"\bCLAUDE" + r"\.md\b"),
    re.compile("mickey-" + "public"),
    re.compile(r"\bprivate (?:lane|branch|repository|repo|workflow)\b", re.I),
)
SECRET_MARKERS = (
    re.compile(r"\bAKIA[0-9A-Z]{16}\b"),
    re.compile(r"\bgh[oprsu]_[A-Za-z0-9]{30,}\b"),
    re.compile("-----BEGIN " + r"[A-Z ]+" + "PRIVATE KEY-----"),
    re.compile(
        r"(?i)\b(?:password|passwd|secret|token)\s*=\s*[^\s$<{][^\s]{7,}"
    ),
)


class PreflightError(RuntimeError):
    pass


def _git(repo: Path, *args: str, check: bool = True) -> str:
    try:
        result = subprocess.run(
            ["git", *args],
            cwd=repo,
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
    except OSError as exc:
        raise PreflightError(f"git {' '.join(args)}: {exc}") from exc
    if check and result.returncode != 0:
        detail = result.stderr.strip() or result.stdout.strip() or f"exit {result.returncode}"
        raise PreflightError(f"git {' '.join(args)}: {detail}")
    return result.stdout


def _repo_root(start: Path) -> Path:
    return Path(_git(start, "rev-parse", "--show-toplevel").strip()).resolve()


def _scan_text(label: str, text: str) -> list[str]:
    findings: list[str] = []
    for pattern in (*PRIVATE_MARKERS, *SECRET_MARKERS):
        if pattern.search(text):
            findings.append(f"{label}: pattern {pattern.pattern!r}")
    return findings


def _tracked_text_findings(repo: Path) -> list[str]:
    """Scan every tracked text blob at HEAD, not only the outgoing delta.

    A release is about the resulting public tree. Restricting this check to
    changed paths would let an older private marker or secret survive forever
    merely because the current release did not touch its file.
    """
    findings: list[str] = []
    names = _git(repo, "ls-files", "-z").split("\0")
    for name in (item for item in names if item):
        blob = subprocess.run(
            ["git", "show", f"HEAD:{name}"],
            cwd=repo,
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        if blob.returncode != 0 or b"\0" in blob.stdout:
            continue
        findings.extend(_scan_text(name, blob.stdout.decode("utf-8", errors="replace")))
    return findings


def _public_preflight(repo: Path, branch: str | None, remote: str | None) -> str:
    if not branch or not remote:
        raise PreflightError(
            "public-push preflight requires explicit --branch and --remote"
        )
    current = _git(repo, "branch", "--show-current").strip()
    if not current or current != branch:
        raise PreflightError(
            f"current branch is {current or '(detached)'!r}, expected {branch!r}"
        )
    remote_url = _git(repo, "remote", "get-url", remote).strip()
    if not remote_url:
        raise PreflightError(f"remote {remote!r} has no URL")
    tracked_dirt = _git(
        repo, "status", "--porcelain=v1", "--untracked-files=no"
    ).strip()
    if tracked_dirt:
        raise PreflightError("tracked worktree/index dirt is present")

    base = f"refs/remotes/{remote}/{branch}"
    exists = subprocess.run(
        ["git", "show-ref", "--verify", "--quiet", base], cwd=repo, check=False
    )
    if exists.returncode != 0:
        raise PreflightError(f"missing fetched comparison ref {base!r}")
    short_base = f"{remote}/{branch}"
    findings = _scan_text(
        "outgoing commit messages",
        _git(repo, "log", "--format=%B", f"{short_base}..HEAD"),
    )
    findings.extend(_tracked_text_findings(repo))
    if findings:
        preview = "; ".join(findings[:5])
        if len(findings) > 5:
            preview += f"; and {len(findings) - 5} more"
        raise PreflightError(f"public tracked/outgoing scan failed: {preview}")
    return f"branch={branch} remote={remote} base={short_base}"


def _nice_child(increment: int) -> None:
    if increment:
        os.nice(increment)


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


def _run_gate(
    repo: Path,
    gate: str,
    log_path: Path,
    *,
    niceness: int,
    timeout: float,
    fake_command: Path | None,
    dry_run: bool,
) -> tuple[str, float]:
    command = [str(fake_command), gate] if fake_command else ["gmake", "-j1", gate]
    started = time.monotonic()
    with log_path.open("wb") as log:
        log.write(("command: " + " ".join(command) + "\n").encode())
        log.flush()
        if dry_run:
            log.write(b"dry-run: command not executed\n")
            return "DRY-RUN", time.monotonic() - started
        try:
            proc = subprocess.Popen(
                command,
                cwd=repo,
                stdout=log,
                stderr=subprocess.STDOUT,
                start_new_session=True,
                preexec_fn=lambda: _nice_child(niceness),
            )
        except (OSError, subprocess.SubprocessError) as exc:
            log.write(f"launch failed: {exc}\n".encode())
            return "FAIL", time.monotonic() - started
        try:
            return_code = proc.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            log.write(f"timeout after {timeout:.1f}s\n".encode())
            log.flush()
            _terminate(proc)
            return "TIMEOUT", time.monotonic() - started
    return ("PASS" if return_code == 0 else "FAIL"), time.monotonic() - started


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=Path.cwd())
    parser.add_argument("--log-dir", type=Path, default=Path("build/release-gate"))
    parser.add_argument("--nice", type=int, default=15)
    parser.add_argument("--timeout", type=float, default=3600.0, metavar="SECONDS")
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument(
        "--fake-command",
        type=Path,
        help="test command invoked once per gate with the gate name as argv[1]",
    )
    parser.add_argument("--public-push-preflight", action="store_true")
    parser.add_argument("--branch")
    parser.add_argument("--remote")
    parser.add_argument("--max-load-per-core", type=float, default=1.25)
    parser.add_argument("--min-memory-available-percent", type=float, default=10.0)
    parser.add_argument("--max-campaign-cpu-percent", type=float, default=85.0)
    return parser


def main(argv: list[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    if not 0 <= args.nice <= 19:
        raise SystemExit("--nice must be between 0 and 19")
    if args.timeout <= 0:
        raise SystemExit("--timeout must be positive")
    if args.max_load_per_core <= 0:
        raise SystemExit("--max-load-per-core must be positive")
    if not 0 <= args.min_memory_available_percent <= 100:
        raise SystemExit("--min-memory-available-percent must be between 0 and 100")
    if not 0 <= args.max_campaign_cpu_percent <= 100:
        raise SystemExit("--max-campaign-cpu-percent must be between 0 and 100")
    if (args.branch or args.remote) and not args.public_push_preflight:
        raise SystemExit("--branch/--remote are only valid with --public-push-preflight")
    if args.fake_command:
        args.fake_command = args.fake_command.expanduser().resolve()
        if not args.fake_command.is_file() or not os.access(args.fake_command, os.X_OK):
            raise SystemExit("--fake-command must name an executable file")

    try:
        repo = _repo_root(args.repo.resolve())
    except PreflightError as exc:
        print(f"PREFLIGHT FAIL: {exc}", file=sys.stderr)
        return 2
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    build_root = (repo / "build").resolve()
    log_root = args.log_dir if args.log_dir.is_absolute() else repo / args.log_dir
    log_root = log_root.resolve()
    try:
        log_root.relative_to(build_root)
    except ValueError:
        print(
            "PREFLIGHT FAIL: --log-dir must remain under the repository build/",
            file=sys.stderr,
        )
        return 2
    run_dir = log_root / f"{stamp}-{os.getpid()}"
    try:
        run_dir.mkdir(parents=True, exist_ok=False)
    except OSError as exc:
        print(f"PREFLIGHT FAIL: cannot create log directory: {exc}", file=sys.stderr)
        return 2

    if args.public_push_preflight:
        try:
            detail = _public_preflight(repo, args.branch, args.remote)
        except PreflightError as exc:
            path = run_dir / "public-preflight.log"
            path.write_text(f"FAIL: {exc}\n")
            print(f"public-preflight FAIL 0.0s log={path}")
            return 2
        path = run_dir / "public-preflight.log"
        path.write_text(f"PASS: {detail}\n")
        print(f"public-preflight PASS 0.0s log={path}")

    total_started = time.monotonic()
    for index, gate in enumerate(GATES, 1):
        report = collect_health(
            repo,
            max_load_per_core=args.max_load_per_core,
            min_memory_available_percent=args.min_memory_available_percent,
            max_campaign_cpu_percent=args.max_campaign_cpu_percent,
        )
        health_log = run_dir / f"{index:02d}-{gate}-health.log"
        health_log.write_text(format_report(report) + "\n")
        if report.verdict != "HEALTHY":
            print(f"health-before-{gate} {report.verdict} 0.0s log={health_log}")
            print(f"release-gate FAIL {time.monotonic() - total_started:.1f}s logs={run_dir}")
            return 2
        log_path = run_dir / f"{index:02d}-{gate}.log"
        status, elapsed = _run_gate(
            repo,
            gate,
            log_path,
            niceness=args.nice,
            timeout=args.timeout,
            fake_command=args.fake_command,
            dry_run=args.dry_run,
        )
        print(f"{gate} {status} {elapsed:.1f}s log={log_path}")
        if status not in {"PASS", "DRY-RUN"}:
            print(f"release-gate FAIL {time.monotonic() - total_started:.1f}s logs={run_dir}")
            return 1

    if args.public_push_preflight and not args.dry_run:
        try:
            _public_preflight(repo, args.branch, args.remote)
        except PreflightError as exc:
            path = run_dir / "public-postflight.log"
            path.write_text(f"FAIL: {exc}\n")
            print(f"public-postflight FAIL 0.0s log={path}")
            print(f"release-gate FAIL {time.monotonic() - total_started:.1f}s logs={run_dir}")
            return 2
    final_status = "DRY-RUN" if args.dry_run else "PASS"
    print(
        f"release-gate {final_status} {time.monotonic() - total_started:.1f}s "
        f"logs={run_dir}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
