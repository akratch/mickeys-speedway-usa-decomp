#!/usr/bin/env python3
"""Print a read-only campaign workstation health summary."""

from __future__ import annotations

import argparse
import getpass
import os
from pathlib import Path
import re
import subprocess
import sys
from dataclasses import dataclass


@dataclass(frozen=True)
class ProcessInfo:
    pid: int
    ppid: int
    nice: int
    cpu_percent: float
    age: str
    cwd: str | None


@dataclass(frozen=True)
class HealthReport:
    verdict: str
    reasons: tuple[str, ...]
    load: tuple[float, float, float] | None
    cores: int
    memory_total: int | None
    memory_available: int | None
    processes: tuple[ProcessInfo, ...]


def _capture(argv: list[str], cwd: Path | None = None) -> str | None:
    try:
        result = subprocess.run(
            argv,
            cwd=cwd,
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
            timeout=10,
        )
    except (OSError, subprocess.TimeoutExpired):
        return None
    return result.stdout if result.returncode == 0 else None


def _primary_root(start: Path) -> Path:
    common = _capture(
        ["git", "rev-parse", "--path-format=absolute", "--git-common-dir"], start
    )
    if common:
        common_path = Path(common.strip()).resolve()
        if common_path.name == ".git":
            return common_path.parent
    return start.resolve()


def _memory_linux() -> tuple[int, int] | None:
    try:
        fields: dict[str, int] = {}
        for line in Path("/proc/meminfo").read_text().splitlines():
            name, value = line.split(":", 1)
            fields[name] = int(value.strip().split()[0]) * 1024
        return fields["MemTotal"], fields["MemAvailable"]
    except (OSError, KeyError, ValueError):
        return None


def _memory_macos() -> tuple[int, int] | None:
    total_text = _capture(["sysctl", "-n", "hw.memsize"])
    stats = _capture(["vm_stat"])
    if not total_text or not stats:
        return None
    try:
        total = int(total_text.strip())
        page_match = re.search(r"page size of (\d+) bytes", stats)
        if not page_match:
            return None
        page_size = int(page_match.group(1))
        pages: dict[str, int] = {}
        for line in stats.splitlines()[1:]:
            match = re.match(r"([^:]+):\s+(\d+)\.?$", line.strip())
            if match:
                pages[match.group(1)] = int(match.group(2))
        available_pages = sum(
            pages.get(name, 0)
            for name in (
                "Pages free",
                "Pages inactive",
                "Pages speculative",
                "Pages purgeable",
            )
        )
        return total, min(total, available_pages * page_size)
    except ValueError:
        return None


def _memory_portable() -> tuple[int, int] | None:
    if sys.platform.startswith("linux"):
        result = _memory_linux()
        if result:
            return result
    if sys.platform == "darwin":
        result = _memory_macos()
        if result:
            return result
    try:
        page_size = os.sysconf("SC_PAGE_SIZE")
        total_pages = os.sysconf("SC_PHYS_PAGES")
        available_pages = os.sysconf("SC_AVPHYS_PAGES")
        return page_size * total_pages, page_size * available_pages
    except (AttributeError, OSError, ValueError):
        return None


def _macos_cwds() -> dict[int, str]:
    output = _capture(
        ["lsof", "-n", "-a", "-d", "cwd", "-u", getpass.getuser(), "-Fpn"]
    )
    if not output:
        return {}
    result: dict[int, str] = {}
    current_pid: int | None = None
    for line in output.splitlines():
        if line.startswith("p") and line[1:].isdigit():
            current_pid = int(line[1:])
        elif line.startswith("n") and current_pid is not None:
            result[current_pid] = line[1:]
    return result


def _process_cwd(pid: int, macos_cwds: dict[int, str]) -> str | None:
    if sys.platform.startswith("linux"):
        try:
            return os.readlink(f"/proc/{pid}/cwd")
        except OSError:
            return None
    return macos_cwds.get(pid)


def _is_campaign_path(cwd: str | None, primary: Path) -> bool:
    if not cwd:
        return False
    try:
        relative = Path(cwd).resolve().relative_to(primary.parent)
    except (OSError, ValueError):
        return False
    if not relative.parts:
        return False
    checkout = relative.parts[0]
    return checkout == primary.name or checkout.startswith(f"{primary.name}-lane-")


def _campaign_processes(primary: Path) -> tuple[ProcessInfo, ...]:
    output = _capture(["ps", "-axo", "pid=,ppid=,ni=,pcpu=,etime=,command="])
    if not output:
        return ()
    macos_cwds = _macos_cwds() if sys.platform == "darwin" else {}
    processes: list[ProcessInfo] = []
    lane_marker = str(primary.parent / f"{primary.name}-lane-")
    for line in output.splitlines():
        fields = line.strip().split(None, 5)
        if len(fields) != 6:
            continue
        try:
            pid = int(fields[0])
            ppid = int(fields[1])
            nice = int(fields[2])
            cpu = float(fields[3])
        except ValueError:
            continue
        age, command = fields[4], fields[5]
        cwd = _process_cwd(pid, macos_cwds)
        if not (
            _is_campaign_path(cwd, primary)
            or str(primary) in command
            or lane_marker in command
        ):
            continue
        processes.append(ProcessInfo(pid, ppid, nice, cpu, age, cwd))
    return tuple(sorted(processes, key=lambda item: (-item.cpu_percent, item.pid)))


def collect_health(
    start: Path,
    *,
    max_load_per_core: float = 1.25,
    min_memory_available_percent: float = 10.0,
    max_campaign_cpu_percent: float = 85.0,
) -> HealthReport:
    primary = _primary_root(start)
    cores = os.cpu_count() or 1
    try:
        load = os.getloadavg()
    except (AttributeError, OSError):
        load = None
    memory = _memory_portable()
    total, available = memory if memory else (None, None)
    processes = _campaign_processes(primary)

    reasons: list[str] = []
    unknown: list[str] = []
    if load is None:
        unknown.append("load unavailable")
    elif load[0] / cores > max_load_per_core:
        reasons.append(
            f"1m load/core {load[0] / cores:.2f} exceeds {max_load_per_core:.2f}"
        )
    if total is None or available is None or total <= 0:
        unknown.append("memory unavailable")
    else:
        available_percent = 100.0 * available / total
        if available_percent < min_memory_available_percent:
            reasons.append(
                f"available memory {available_percent:.1f}% is below "
                f"{min_memory_available_percent:.1f}%"
            )
    campaign_capacity = cores * 100.0
    campaign_percent = 100.0 * sum(p.cpu_percent for p in processes) / campaign_capacity
    if campaign_percent > max_campaign_cpu_percent:
        reasons.append(
            f"campaign CPU {campaign_percent:.1f}% of host capacity exceeds "
            f"{max_campaign_cpu_percent:.1f}%"
        )

    if reasons:
        verdict = "WARN"
    elif unknown:
        verdict = "UNKNOWN"
        reasons.extend(unknown)
    else:
        verdict = "HEALTHY"
    return HealthReport(
        verdict, tuple(reasons), load, cores, total, available, processes
    )


def _format_bytes(value: int | None) -> str:
    if value is None:
        return "?"
    units = ("B", "KiB", "MiB", "GiB", "TiB")
    amount = float(value)
    for unit in units:
        if amount < 1024.0 or unit == units[-1]:
            return f"{amount:.1f}{unit}"
        amount /= 1024.0
    return "?"


def format_report(report: HealthReport) -> str:
    if report.load:
        load_text = "/".join(f"{value:.2f}" for value in report.load)
        load_text += f" ({report.load[0] / report.cores:.2f}/core 1m)"
    else:
        load_text = "unavailable"
    if report.memory_total and report.memory_available is not None:
        memory_text = (
            f"{_format_bytes(report.memory_available)} available / "
            f"{_format_bytes(report.memory_total)} total "
            f"({100.0 * report.memory_available / report.memory_total:.1f}% available)"
        )
    else:
        memory_text = "unavailable"
    campaign_cpu = sum(process.cpu_percent for process in report.processes)
    lines = [
        f"HEALTH {report.verdict}",
        f"load: {load_text}; cores: {report.cores}",
        f"memory: {memory_text}",
        f"campaign: {len(report.processes)} processes; {campaign_cpu:.1f}% CPU",
    ]
    if report.processes:
        lines.append("PID    PPID  NI   CPU%  AGE          CWD")
        for process in report.processes:
            lines.append(
                f"{process.pid:<6} {process.ppid:<5} {process.nice:>3} "
                f"{process.cpu_percent:>6.1f} {process.age:<12} "
                f"{process.cwd or '?'}"
            )
    lines.extend(f"reason: {reason}" for reason in report.reasons)
    return "\n".join(lines)


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=Path.cwd())
    parser.add_argument("--max-load-per-core", type=float, default=1.25)
    parser.add_argument("--min-memory-available-percent", type=float, default=10.0)
    parser.add_argument("--max-campaign-cpu-percent", type=float, default=85.0)
    parser.add_argument(
        "--strict", action="store_true", help="exit nonzero for WARN or UNKNOWN"
    )
    return parser


def main(argv: list[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    if args.max_load_per_core <= 0:
        raise SystemExit("--max-load-per-core must be positive")
    if not 0 <= args.min_memory_available_percent <= 100:
        raise SystemExit("--min-memory-available-percent must be between 0 and 100")
    if not 0 <= args.max_campaign_cpu_percent <= 100:
        raise SystemExit("--max-campaign-cpu-percent must be between 0 and 100")
    report = collect_health(
        args.repo,
        max_load_per_core=args.max_load_per_core,
        min_memory_available_percent=args.min_memory_available_percent,
        max_campaign_cpu_percent=args.max_campaign_cpu_percent,
    )
    print(format_report(report))
    return 0 if not args.strict or report.verdict == "HEALTHY" else 2


if __name__ == "__main__":
    raise SystemExit(main())
