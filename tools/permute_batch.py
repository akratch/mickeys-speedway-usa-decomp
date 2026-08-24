#!/usr/bin/env python3
"""Batch decomp-permuter runner over the NON_MATCHING queue.

docs/adr/0007-matching-tools.md: decomp-permuter runs only as a *bounded
batch job*, never inside an agent's own reasoning loop. This is that job
runner. It does not reason about candidates; it imports each queued
function, runs permuter.py under a wall-clock cap, and reports what came
back. See docs/permute-batch.md for the day-to-day usage writeup.

QUEUE DISCOVERY

Primary source: config/overlays.us.json's per-module `text_ownership` rows
(written by tools/overlay_atlas.py), each carrying a mechanically-derived
`nonmatching` flag (true iff the C source for that range still has an
`#ifdef NON_MATCHING` guard -- see overlay_atlas.is_nonmatching_source).
Cross-checked, and supplemented for anything the atlas doesn't cover yet,
by scanning src/**/*.c directly for `#ifdef NON_MATCHING` blocks.

THE OVERLAY NAMING QUIRK

splat auto-names every overlay function it disassembles from the ROM
`func_overlay_MMM_FOOOOOOO_ROMADDR` (the module shares one synthetic VMA
with every other overlay, so spimdisasm can't derive a unique name from the
address alone -- see overlay_atlas.py's SYNTHETIC_VMA comment). The C
symbol is the friendly name a human already gave it
(`overlay1GetEntry`). A queued function's `#ifdef NON_MATCHING` branch
already defines the friendly name; its `#else` branch's
`#pragma GLOBAL_ASM("asm/nonmatchings/.../func_overlay_...s")` still points
at the auto name. decomp-permuter's import.py derives the function name it
hunts for in the C from the target .s file's `glabel` line, so handing it
the auto-named .s directly fails ("not found in base.c"). This script
works around that without editing import.py (out of this lane's ownership)
or the project's own asm/ output (gitignored, never written to): it copies
the target .s into the function's scratch dir and renames the
glabel/endlabel pair to the friendly name -- a label rename, metadata only,
exactly the kind of thing the project's own POSTPROCESS
`objcopy --redefine-sym` steps already do to compiled objects (permitted
under docs/adr/0002-no-post-compile-instruction-editing.md). No instruction
word is touched and nothing under asm/ is modified in place.

THE NON_MATCHING DEFINE

A queued function's C file still carries both branches
(`#ifdef NON_MATCHING <candidate> #else <pragma> #endif`); the project's
own Makefile selects the candidate branch by building with
`-DNON_MATCHING` (see Makefile's `NON_MATCHING ?= 0` escape hatch). This
script's generated per-function permuter_settings.toml adds that same
define, so the unmodified C file, not a hand-edited copy, is what
import.py preprocesses -- consistent for a file with more than one queued
function in it too, since the C preprocessor (not this script) resolves
every `#ifdef NON_MATCHING` block in the translation unit the same way.

PROMOTION

When a run reaches score 0, this script does not stop at reporting it: an
exact permuter candidate is still just a candidate (ADR 0007's own
closing line) until it is compiled by the project's real toolchain, linked
at its real address, and byte-compared -- so:

  1. extract the winning candidate's function body (its output-0-*/source.c
     is the whole pruned translation unit, not just the function) and
     splice it into the real C file in place of the `#ifdef NON_MATCHING`
     wrapper, dropping the ifdef/else/pragma/endif;
  2. `gmake -jN` and `gmake verify` (byte-identical ROM rebuild);
  3. `tools/wb_compare.sh --rom <symbol>` as the linked-range oracle, since
     splat stops emitting a nonmatchings .s the moment the C matches;
  4. only on both gates passing, leave the promoted source in place and
     report it matched. Any failure reverts the C file to its prior text
     and the function stays queued, reported as a zero-score permuter hit
     that didn't survive promotion (recorded in the summary either way).

This intentionally does not touch a POSTPROCESS Makefile rule automatically
-- whether one is still needed after promotion is a per-function judgment
call (see docs/permute-batch.md and the pilot commit for the worked
example); --apply-makefile-cleanup is opt-in and only ever removes a rule
that becomes a dead reference to a symbol the object no longer emits.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import dataclasses
import json
import os
import re
import shutil
import subprocess
import sys
import threading
import time
from pathlib import Path
from typing import Optional

ROOT = Path(__file__).resolve().parent.parent
ATLAS_PATH = ROOT / "config" / "overlays.us.json"
PERMUTER_DIR = ROOT / "tools" / "permuter"
IMPORT_PY = PERMUTER_DIR / "import.py"
PERMUTER_PY = PERMUTER_DIR / "permuter.py"
PYTHON = ROOT / ".venv" / "bin" / "python"
BUILD_PERMUTER = ROOT / "build" / "permuter"
SUMMARY_JSON = BUILD_PERMUTER / "summary.json"
SUMMARY_TXT = BUILD_PERMUTER / "summary.txt"

NON_MATCHING_BLOCK_RE = re.compile(
    r"#ifdef NON_MATCHING\b(?P<body>.*?)#else\b(?P<else_>.*?)#endif\b",
    re.DOTALL,
)
FUNC_DEF_RE = re.compile(
    r"^[A-Za-z_][A-Za-z0-9_ \t\*]*?\b(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*"
    r"\([^;{]*\)\s*\{",
    re.MULTILINE,
)
GLOBAL_ASM_RE = re.compile(r'#pragma\s+GLOBAL_ASM\("(?P<path>[^"]+)"\)')
GLABEL_RE = re.compile(r"^\s*(glabel|endlabel|jlabel|dlabel)\s+(\S+)", re.MULTILINE)

# Flag groups this script knows about, keyed by a classifier over the
# source path relative to src/. Mirrors tools/permuter_settings.toml's own
# header comment and Makefile's OPT_FLAGS/MIPSISET defaults (line ~90) and
# per-directory overrides (main/%, overlays/%, ~line 661/666).
FLAG_GROUP_DEFAULT = ("-O2", "-mips1", "-32")  # libultra project default
FLAG_GROUP_OVERLAY = ("-O2", "-mips2", "-32")  # src/main/**, src/overlays/**
FLAG_GROUP_O2_G3 = ("-O2", "-g3", "-mips2", "-32")  # LIBULTRA_O2_G3_TUS

BASE_CC_ARGS = (
    "-c -non_shared -G 0 -Xcpluscomm -fullwarn -woff 649,838 -nostdinc "
    "-D_LANGUAGE_C -D_FINALROM -DTARGET_N64 -DVERSION_us -D_MIPS_SZLONG=32"
)
INCLUDES = "-I . -I include -I include/libc -I include/PR -I assets"
ASSEMBLER_COMMAND = (
    "tools/binutils/mips64-elf-as -march=vr4300 -32 -mabi=32 -G0 -I include"
)

PRESERVE_MACROS = """[preserve_macros]
"g[DS]P.*" = "void"
"gDma.*" = "void"
"gDkr.*" = "void"
"fast3d_cmd" = "void"
"OS_PHYSICAL_TO_K0" = "void *"
"_SHIFTL" = "unsigned int"
"""


# --------------------------------------------------------------------------
# Queue discovery
# --------------------------------------------------------------------------


@dataclasses.dataclass
class QueueItem:
    func: str
    c_file: Path  # absolute path
    overlay: Optional[int] = None
    source: Optional[str] = None  # atlas "source" string, e.g. overlays/o001/overlay1GetEntry

    @property
    def rel_c_file(self) -> str:
        return str(self.c_file.relative_to(ROOT))


def _libultra_o2_g3_tus() -> set[str]:
    """Parse Makefile's LIBULTRA_O2_G3_TUS := ... continuation list."""
    text = (ROOT / "Makefile").read_text(errors="replace")
    m = re.search(r"LIBULTRA_O2_G3_TUS\s*:=\s*(.*?)(?<!\\)\n", text, re.DOTALL)
    if not m:
        return set()
    raw = m.group(1).replace("\\\n", " ")
    return set(raw.split())


_O2_G3_TUS = None


def flag_group_for(c_file: Path) -> tuple[str, ...]:
    global _O2_G3_TUS
    rel = c_file.relative_to(ROOT / "src").as_posix()
    if rel.startswith("main/") or rel.startswith("overlays/"):
        return FLAG_GROUP_OVERLAY
    if rel.startswith("libultra/"):
        if _O2_G3_TUS is None:
            _O2_G3_TUS = _libultra_o2_g3_tus()
        stem = Path(rel).stem
        if stem in _O2_G3_TUS:
            return FLAG_GROUP_O2_G3
    return FLAG_GROUP_DEFAULT


def discover_queue_from_atlas() -> list[QueueItem]:
    if not ATLAS_PATH.is_file():
        return []
    data = json.loads(ATLAS_PATH.read_text())
    items = []
    for mod in data.get("modules", []):
        overlay = mod.get("overlay")
        for row in mod.get("text_ownership", []):
            if not row.get("nonmatching"):
                continue
            source = row.get("source")
            if not source:
                continue
            func = source.rsplit("/", 1)[-1]
            c_file = ROOT / "src" / f"{source}.c"
            if not c_file.is_file():
                continue
            items.append(
                QueueItem(func=func, c_file=c_file, overlay=overlay, source=source)
            )
    return items


def discover_queue_from_source_scan() -> list[QueueItem]:
    """Fallback / supplement: scan src/**/*.c directly for #ifdef
    NON_MATCHING blocks, independent of the atlas. Catches anything not
    (yet) reflected in config/overlays.us.json (non-overlay sources, or a
    tree where the atlas hasn't been regenerated since a conversion)."""
    items = []
    for c_file in sorted(ROOT.glob("src/**/*.c")):
        text = c_file.read_text(errors="replace")
        if "#ifdef NON_MATCHING" not in text:
            continue
        for m in NON_MATCHING_BLOCK_RE.finditer(text):
            fn = FUNC_DEF_RE.search(m.group("body"))
            if not fn:
                continue
            func = fn.group("name")
            overlay = None
            mo = re.search(r"src/overlays/o(\d+)/", str(c_file))
            if mo:
                overlay = int(mo.group(1))
            items.append(QueueItem(func=func, c_file=c_file, overlay=overlay))
    return items


def discover_queue() -> list[QueueItem]:
    """Union of the atlas rows and the direct source scan, de-duplicated by
    (c_file, func). The atlas is authoritative when both agree; the scan
    catches anything the atlas doesn't have a row for yet."""
    by_key: dict[tuple[str, str], QueueItem] = {}
    for it in discover_queue_from_atlas():
        by_key[(it.rel_c_file, it.func)] = it
    for it in discover_queue_from_source_scan():
        by_key.setdefault((it.rel_c_file, it.func), it)
    return sorted(by_key.values(), key=lambda it: (it.rel_c_file, it.func))


def find_asm_target(item: QueueItem) -> Optional[str]:
    """The GLOBAL_ASM path for this specific function's #else branch,
    read straight out of the C file text (works whether or not the tree
    has been `gmake extract`-ed with NON_MATCHING=0 active, since the
    pragma string is present in the source either way)."""
    text = item.c_file.read_text(errors="replace")
    for m in NON_MATCHING_BLOCK_RE.finditer(text):
        fn = FUNC_DEF_RE.search(m.group("body"))
        if fn and fn.group("name") == item.func:
            am = GLOBAL_ASM_RE.search(m.group("else_"))
            if am:
                return am.group("path")
    return None


# --------------------------------------------------------------------------
# Per-function permuter run
# --------------------------------------------------------------------------


@dataclasses.dataclass
class RunResult:
    func: str
    c_file: str
    overlay: Optional[int]
    ok: bool  # ran without infrastructure error
    base_score: Optional[int] = None
    best_score: Optional[int] = None
    zero_found: bool = False
    promoted: bool = False
    promote_error: Optional[str] = None
    error: Optional[str] = None
    seconds: float = 0.0


def write_settings_toml(out_path: Path, flags: tuple[str, ...]) -> None:
    opt_mips = " ".join(flags)
    compiler_command = (
        f'tools/ido/cc {BASE_CC_ARGS} -DNON_MATCHING \\\n'
        f'{INCLUDES} {opt_mips}'
    )
    text = (
        'compiler_type = "ido"\n'
        'compiler_command = """\n'
        f"{compiler_command}\n"
        '"""\n'
        f'assembler_command = "{ASSEMBLER_COMMAND}"\n\n'
        f"{PRESERVE_MACROS}\n"
        "[decompme.compilers]\n"
        '"tools/ido/cc" = "ido5.3"\n'
    )
    out_path.write_text(text)


def prepare_target_asm(item: QueueItem, out_dir: Path) -> Path:
    """Copy the function's target .s into the scratch area, renaming its
    glabel/endlabel pair from splat's ROM-derived auto name to the
    friendly C name import.py expects to find. See module docstring:
    "THE OVERLAY NAMING QUIRK". A pure label rename -- metadata, never an
    instruction word -- and never written back into asm/ itself."""
    asm_rel = find_asm_target(item)
    if asm_rel is None:
        raise RuntimeError(
            f"no #pragma GLOBAL_ASM(...) found for {item.func} in {item.rel_c_file}"
        )
    asm_path = ROOT / asm_rel
    if not asm_path.is_file():
        raise RuntimeError(
            f"target asm {asm_rel} does not exist -- run `gmake extract` first"
        )
    text = asm_path.read_text(errors="replace")
    labels = set(m.group(2) for m in GLABEL_RE.finditer(text))
    if len(labels) != 1:
        raise RuntimeError(
            f"{asm_rel}: expected exactly one label, found {sorted(labels)}"
        )
    (auto_name,) = labels
    if auto_name != item.func:
        text = re.sub(
            r"\b" + re.escape(auto_name) + r"\b", item.func, text
        )
    target = out_dir / "target.s"
    target.write_text(text)
    return target


def run_import(item: QueueItem, out_dir: Path, settings_path: Path, target_asm: Path) -> Path:
    root_nonmatchings = ROOT / "nonmatchings" / item.func
    if root_nonmatchings.exists():
        shutil.rmtree(root_nonmatchings)
    log_path = out_dir / "import.log"
    proc = subprocess.run(
        [
            str(PYTHON),
            str(IMPORT_PY),
            str(item.c_file),
            str(target_asm),
            "--settings",
            str(settings_path),
        ],
        cwd=ROOT,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    log_path.write_text(proc.stdout)
    if proc.returncode != 0 or not root_nonmatchings.is_dir():
        raise RuntimeError(
            f"import.py failed for {item.func} (see {log_path.relative_to(ROOT)})"
        )
    scratch = out_dir / "scratch"
    if scratch.exists():
        shutil.rmtree(scratch)
    shutil.move(str(root_nonmatchings), str(scratch))
    # nonmatchings/ at repo root may now be empty; tidy it up.
    try:
        (ROOT / "nonmatchings").rmdir()
    except OSError:
        pass
    return scratch


def run_permuter(scratch: Path, out_dir: Path, minutes: int, threads: int, extra_args: list[str]) -> tuple[Optional[int], float]:
    log_path = out_dir / "permuter.log"
    args = [
        str(PYTHON),
        str(PERMUTER_PY),
        "--stop-on-zero",
        "--quiet",
        "-j",
        str(threads),
        *extra_args,
        str(scratch),
    ]
    start = time.monotonic()
    with open(log_path, "w") as log_f:
        try:
            proc = subprocess.run(
                args,
                cwd=ROOT,
                stdout=log_f,
                stderr=subprocess.STDOUT,
                timeout=minutes * 60,
            )
            returncode = proc.returncode
        except subprocess.TimeoutExpired:
            returncode = 124
    elapsed = time.monotonic() - start
    text = log_path.read_text(errors="replace")
    m = re.search(r"base score = (\d+)", text)
    base_score = int(m.group(1)) if m else None
    return base_score, elapsed


def best_output_dir(scratch: Path) -> Optional[Path]:
    candidates = []
    for d in scratch.glob("output-*"):
        if not d.is_dir():
            continue
        parts = d.name.split("-")
        if len(parts) >= 2 and parts[1].lstrip("-").isdigit():
            candidates.append((int(parts[1]), d))
    if not candidates:
        return None
    candidates.sort(key=lambda t: t[0])
    return candidates[0][1]


# --------------------------------------------------------------------------
# Promotion: splice a zero-score candidate into the real C file
# --------------------------------------------------------------------------


def extract_function_text(source_text: str, func: str) -> str:
    m = None
    for cand in FUNC_DEF_RE.finditer(source_text):
        if cand.group("name") == func:
            m = cand
            break
    if m is None:
        raise RuntimeError(f"could not find {func}() in candidate source.c")
    start = m.start()
    depth = 0
    i = m.end() - 1  # at the opening brace
    while True:
        if source_text[i] == "{":
            depth += 1
        elif source_text[i] == "}":
            depth -= 1
            if depth == 0:
                break
        i += 1
    end = i + 1
    return source_text[start:end]


# A --jobs > 1 batch runs several functions' permuter searches concurrently,
# but promotion (splice + `gmake` + `gmake verify`) mutates the one shared
# working tree and build/ directory this lane owns -- two threads promoting
# at once would race on the same objects and ELF. Only the search itself is
# parallel; promotion is serialized across the whole batch.
PROMOTE_LOCK = threading.Lock()


def promote(item: QueueItem, winning_source: Path, jobs: int) -> tuple[bool, Optional[str]]:
    """Splice the winning candidate into the real C file, rebuild, and
    verify byte-identity. Reverts the C file on any failure. Returns
    (promoted, error)."""
    with PROMOTE_LOCK:
        return _promote_locked(item, winning_source, jobs)


def _promote_locked(item: QueueItem, winning_source: Path, jobs: int) -> tuple[bool, Optional[str]]:
    original = item.c_file.read_text()
    candidate_text = winning_source.read_text()
    try:
        new_fn_text = extract_function_text(candidate_text, item.func)
    except RuntimeError as e:
        return False, str(e)

    def replace_block(m: re.Match) -> str:
        fn = FUNC_DEF_RE.search(m.group("body"))
        if fn and fn.group("name") == item.func:
            return new_fn_text
        return m.group(0)

    new_text, n = NON_MATCHING_BLOCK_RE.subn(replace_block, original)
    if n == 0 or new_text == original:
        return False, f"could not locate {item.func}'s NON_MATCHING block to replace"

    item.c_file.write_text(new_text)

    def revert(reason: str) -> tuple[bool, str]:
        item.c_file.write_text(original)
        # Best-effort: rebuild the tree back to the pre-splice state so a
        # failed promotion doesn't leave build/ out of sync with the source
        # for whatever runs next (another function's promotion, or the
        # caller's own later `gmake verify`). Failure here is reported
        # alongside the original reason, not swallowed.
        try:
            subprocess.run(
                ["gmake", f"-j{jobs}"],
                cwd=ROOT,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                timeout=1800,
                check=False,
            )
        except subprocess.TimeoutExpired:
            reason += "\n(also: rebuild-after-revert timed out; build/ may be stale)"
        return False, reason

    try:
        build = subprocess.run(
            ["gmake", f"-j{jobs}"],
            cwd=ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            timeout=1800,
        )
        if build.returncode != 0:
            return revert("gmake build failed:\n" + build.stdout[-4000:])

        verify = subprocess.run(
            ["gmake", "verify"],
            cwd=ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            timeout=600,
        )
        if verify.returncode != 0:
            return revert("gmake verify failed:\n" + verify.stdout[-4000:])

        wb = subprocess.run(
            ["tools/wb_compare.sh", "--rom", item.func],
            cwd=ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            timeout=300,
        )
        if wb.returncode != 0:
            return revert("wb_compare.sh --rom failed:\n" + wb.stdout[-4000:])

        return True, None
    except subprocess.TimeoutExpired as e:
        return revert(f"timed out: {e}")


def run_one(item: QueueItem, minutes: int, permuter_threads: int, build_jobs: int, apply: bool, extra_args: list[str]) -> RunResult:
    out_dir = BUILD_PERMUTER / item.func
    out_dir.mkdir(parents=True, exist_ok=True)
    result = RunResult(func=item.func, c_file=item.rel_c_file, overlay=item.overlay, ok=False)
    start = time.monotonic()
    try:
        flags = flag_group_for(item.c_file)
        settings_path = out_dir / "permuter_settings.toml"
        write_settings_toml(settings_path, flags)
        target_asm = prepare_target_asm(item, out_dir)
        scratch = run_import(item, out_dir, settings_path, target_asm)
        base_score, _elapsed = run_permuter(scratch, out_dir, minutes, permuter_threads, extra_args)
        result.base_score = base_score
        result.ok = True

        best_dir = best_output_dir(scratch)
        if best_dir is not None:
            score_file = best_dir / "score.txt"
            best_score = int(score_file.read_text().strip()) if score_file.is_file() else None
            result.best_score = best_score
            if best_score == 0:
                result.zero_found = True
                if apply:
                    promoted, err = promote(item, best_dir / "source.c", build_jobs)
                    result.promoted = promoted
                    result.promote_error = err
    except Exception as e:  # noqa: BLE001 -- report, don't crash the batch
        result.error = str(e)
    result.seconds = time.monotonic() - start
    return result


# --------------------------------------------------------------------------
# CLI
# --------------------------------------------------------------------------


def parse_args(argv: list[str]) -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--overlay", type=int, help="restrict to one overlay number")
    p.add_argument("--function", help="restrict to one function name")
    p.add_argument("--limit", type=int, help="cap the number of functions processed")
    p.add_argument("--minutes", type=int, default=20, help="per-function wall-clock cap (default: 20)")
    p.add_argument("--jobs", type=int, default=1, help="concurrent functions (default: 1)")
    p.add_argument(
        "--permuter-threads",
        type=int,
        help="threads per permuter.py instance (-j); default: split (ncpu - 2) across --jobs",
    )
    p.add_argument(
        "--build-jobs",
        type=int,
        default=max(os.cpu_count() or 4, 1),
        help="gmake -jN to use when promoting a zero-score match (default: ncpu)",
    )
    p.add_argument(
        "--apply",
        action="store_true",
        help="on a zero-score result, splice the winning candidate into the C file, "
        "rebuild, and verify byte-identity before reporting it matched",
    )
    p.add_argument("--list", action="store_true", help="print the discovered queue and exit")
    p.add_argument(
        "permuter_args",
        nargs=argparse.REMAINDER,
        help="extra args forwarded to permuter.py, after --",
    )
    return p.parse_args(argv)


def ncpu() -> int:
    return os.cpu_count() or 4


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    extra_args = args.permuter_args
    if extra_args and extra_args[0] == "--":
        extra_args = extra_args[1:]

    queue = discover_queue()
    if args.overlay is not None:
        queue = [it for it in queue if it.overlay == args.overlay]
    if args.function is not None:
        queue = [it for it in queue if it.func == args.function]
    if args.limit is not None:
        queue = queue[: args.limit]

    if args.list or not queue:
        print(f"{len(queue)} queued function(s):")
        for it in queue:
            ov = f"o{it.overlay:03d}" if it.overlay is not None else "?"
            print(f"  [{ov}] {it.func}  ({it.rel_c_file})")
        if not queue:
            print("nothing to do.")
            return 0
        if args.list:
            return 0

    jobs = max(1, args.jobs)
    permuter_threads = args.permuter_threads
    if permuter_threads is None:
        budget = max(ncpu() - 2, 1)
        permuter_threads = max(1, budget // jobs)

    BUILD_PERMUTER.mkdir(parents=True, exist_ok=True)
    results: list[RunResult] = []

    print(
        f"Running {len(queue)} function(s), {jobs} concurrent, "
        f"{permuter_threads} permuter thread(s) each, {args.minutes} min cap each"
        + (", apply=on" if args.apply else ", apply=off (report only)")
    )

    if jobs == 1:
        for it in queue:
            r = run_one(it, args.minutes, permuter_threads, args.build_jobs, args.apply, extra_args)
            results.append(r)
            print_result(r)
            write_summary(results)
    else:
        with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as pool:
            futures = {
                pool.submit(
                    run_one, it, args.minutes, permuter_threads, args.build_jobs, args.apply, extra_args
                ): it
                for it in queue
            }
            for fut in concurrent.futures.as_completed(futures):
                r = fut.result()
                results.append(r)
                print_result(r)
                write_summary(results)

    write_summary(results, final=True)
    print(f"\nSummary written to {SUMMARY_JSON.relative_to(ROOT)} and {SUMMARY_TXT.relative_to(ROOT)}")
    print_table(results)
    return 0


def print_result(r: RunResult) -> None:
    if r.error:
        print(f"[{r.func}] ERROR: {r.error}")
        return
    status = "MATCHED" if r.promoted else ("zero-found" if r.zero_found else "no improvement" if r.best_score is None else "improved")
    print(
        f"[{r.func}] base={r.base_score} best={r.best_score} "
        f"{status} ({r.seconds:.0f}s)"
    )


def print_table(results: list[RunResult]) -> None:
    if not results:
        return
    print(f"\n{'function':<32} {'base':>6} {'best':>6}  {'zero':<5} {'matched':<8} {'time':>7}")
    for r in results:
        print(
            f"{r.func:<32} {str(r.base_score):>6} {str(r.best_score):>6}  "
            f"{'yes' if r.zero_found else 'no':<5} {'yes' if r.promoted else 'no':<8} "
            f"{r.seconds:>6.0f}s"
        )
    n = len(results)
    n_zero = sum(1 for r in results if r.zero_found)
    n_matched = sum(1 for r in results if r.promoted)
    n_err = sum(1 for r in results if r.error)
    print(
        f"\n{n} run, {n_zero} zero-score ({n_zero / n:.0%}), "
        f"{n_matched} promoted to matched ({n_matched / n:.0%}), {n_err} errored"
    )


def write_summary(results: list[RunResult], final: bool = False) -> None:
    payload = {
        "generated_by": "tools/permute_batch.py",
        "final": final,
        "results": [dataclasses.asdict(r) for r in results],
        "totals": {
            "run": len(results),
            "zero_found": sum(1 for r in results if r.zero_found),
            "promoted": sum(1 for r in results if r.promoted),
            "errored": sum(1 for r in results if r.error),
        },
    }
    SUMMARY_JSON.write_text(json.dumps(payload, indent=2) + "\n")
    lines = [
        f"{'function':<32} {'base':>6} {'best':>6}  {'zero':<5} {'matched':<8} {'time':>7}"
    ]
    for r in results:
        lines.append(
            f"{r.func:<32} {str(r.base_score):>6} {str(r.best_score):>6}  "
            f"{'yes' if r.zero_found else 'no':<5} {'yes' if r.promoted else 'no':<8} "
            f"{r.seconds:>6.0f}s"
        )
    SUMMARY_TXT.write_text("\n".join(lines) + "\n")


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
