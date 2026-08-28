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
import signal
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
RANKING_PATH = ROOT / "config" / "nonmatching-ranking.us.json"

# Flags that shape codegen and therefore must match the real per-file build
# exactly (the same set tools/permute.sh recovers). Anything else in the cc
# line (-I, -D, -c, -o ...) is reproduced by BASE_CC_ARGS/INCLUDES already.
CODEGEN_FLAG_RE = re.compile(
    r"-mips[0-9]|-O[0-9]|-Wo,[^ ]*|-Wab,[^ ]*|-g[0-9]?(?= |$)|-(?:32|n32|64)(?= |$)"
)
# Post-compile ELF passes the scratch cannot replicate: they are digest-guarded
# against the *matched* bytes and would abort (or lie) on a permuted object.
# trim_elf_section only trims section padding and never touches a function's
# words, so it is harmless to skip.
UNREPLICABLE_POSTPROC_RE = re.compile(r"\.py\b")

NON_MATCHING_BLOCK_RE = re.compile(
    r"#ifdef NON_MATCHING\b(?P<body>.*?)#else\b(?P<else_>.*?)#endif\b",
    re.DOTALL,
)
PP_DIRECTIVE_RE = re.compile(
    r"^[ \t]*#[ \t]*(?P<kind>if|ifdef|ifndef|elif|else|endif)\b"
    r"(?P<argument>[^\n]*)",
    re.MULTILINE,
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


@dataclasses.dataclass(frozen=True)
class NonMatchingBlock:
    body: str
    fallback: str
    start: int
    end: int
    body_start: int
    body_end: int
    fallback_start: int
    fallback_end: int


def iter_nonmatching_blocks(source_text: str):
    """Yield top-level ``#ifdef NON_MATCHING`` body/fallback pairs.

    The old non-greedy regex stopped at the first nested ``#else`` or
    ``#endif``. Several candidates legitimately contain feature switches,
    so their GLOBAL_ASM fallback was invisible to queue isolation.
    """
    stack: list[dict[str, object]] = []
    completed: list[tuple[int, NonMatchingBlock]] = []
    for match in PP_DIRECTIVE_RE.finditer(source_text):
        kind = match.group("kind")
        argument = match.group("argument").strip()
        if kind in ("if", "ifdef", "ifndef"):
            stack.append(
                {
                    "target": kind == "ifdef" and argument == "NON_MATCHING",
                    "start": match.start(),
                    "body_start": match.end(),
                    "body_end": None,
                    "fallback_start": None,
                }
            )
            continue
        if not stack:
            continue
        current = stack[-1]
        if kind in ("else", "elif"):
            if current["target"] and current["body_end"] is None:
                current["body_end"] = match.start()
                current["fallback_start"] = match.end()
            continue
        if kind == "endif":
            current = stack.pop()
            if not current["target"]:
                continue
            body_end = current["body_end"]
            fallback_start = current["fallback_start"]
            if body_end is None or fallback_start is None:
                continue
            completed.append(
                (
                    int(current["start"]),
                    NonMatchingBlock(
                        body=source_text[int(current["body_start"]):int(body_end)],
                        fallback=source_text[int(fallback_start):match.start()],
                        start=int(current["start"]),
                        end=match.end(),
                        body_start=int(current["body_start"]),
                        body_end=int(body_end),
                        fallback_start=int(fallback_start),
                        fallback_end=match.start(),
                    ),
                )
            )
    for _, block in sorted(completed, key=lambda pair: pair[0]):
        yield block


def block_function_name(source_text: str, block: NonMatchingBlock) -> Optional[str]:
    """Return the C symbol defined by a NON_MATCHING candidate body.

    A few shared implementations spell the definition through a simple
    object-like macro. Resolve that one identifier without attempting to
    duplicate the C preprocessor.
    """
    fn = FUNC_DEF_RE.search(block.body)
    if fn is None:
        return None
    name = fn.group("name")
    for _ in range(8):
        macro = re.search(
            rf"^[ \t]*#[ \t]*define[ \t]+{re.escape(name)}[ \t]+"
            r"(?P<replacement>[A-Za-z_][A-Za-z0-9_]*)[ \t]*(?:/\*.*\*/)?$",
            source_text,
            re.MULTILINE,
        )
        if macro is None:
            break
        name = macro.group("replacement")
    return name


def _libultra_o2_g3_tus() -> set[str]:
    """Parse Makefile's LIBULTRA_O2_G3_TUS := ... continuation list."""
    text = (ROOT / "Makefile").read_text(errors="replace")
    m = re.search(r"LIBULTRA_O2_G3_TUS\s*:=\s*(.*?)(?<!\\)\n", text, re.DOTALL)
    if not m:
        return set()
    raw = m.group(1).replace("\\\n", " ")
    return set(raw.split())


_O2_G3_TUS = None


@dataclasses.dataclass(frozen=True)
class BuildRecipe:
    """What the project's real build does to one TU's object: the codegen
    flags on its cc line and any post-compile objcopy chain. Recovered from
    `gmake -n <obj>` (the source is touched first: gmake prints nothing for
    an up-to-date object, which is exactly the silent -mips1 false floor
    docs/matching-triage.md records)."""

    flags: tuple[str, ...]
    objcopy_steps: tuple[str, ...]  # shell fragments, real object path intact
    skipped_postproc: tuple[str, ...]  # digest-guarded passes not replicated
    from_dry_run: bool


_RECIPE_CACHE: dict[Path, BuildRecipe] = {}
_RECIPE_LOCK = threading.Lock()


def build_recipe_for(c_file: Path) -> BuildRecipe:
    with _RECIPE_LOCK:
        if c_file in _RECIPE_CACHE:
            return _RECIPE_CACHE[c_file]
    obj = f"build/{c_file.relative_to(ROOT).as_posix()}.o"
    try:
        os.utime(c_file, None)
    except OSError:
        pass
    dry = subprocess.run(
        ["gmake", "-n", obj], cwd=ROOT, stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL, text=True, timeout=120,
    ).stdout
    flags: tuple[str, ...] = ()
    objcopy_steps: list[str] = []
    skipped: list[str] = []
    # gmake echoes the recipe verbatim, so the cc command arrives as
    # "... tools/ido/cc -- <as> -- \" + a continuation line carrying the
    # flags and the object path. Join continuations before parsing.
    dry = dry.replace("\\\n", " ")
    for line in dry.splitlines():
        if obj not in line:
            continue
        if "objcopy" in line and "tools/ido/cc" not in line:
            for seg in (s.strip() for s in line.split("&&")):
                if not seg:
                    continue
                if UNREPLICABLE_POSTPROC_RE.search(seg):
                    skipped.append(seg)
                elif "objcopy" in seg:
                    objcopy_steps.append(seg)
                else:
                    skipped.append(seg)
        elif "tools/ido/cc" in line and not flags:
            found = CODEGEN_FLAG_RE.findall(line)
            if any(f.startswith("-mips") for f in found):
                flags = tuple(dict.fromkeys(found))
    if flags:
        recipe = BuildRecipe(flags, tuple(objcopy_steps), tuple(skipped), True)
    else:
        print(
            f"WARNING: could not recover real compile flags for {obj}; "
            f"falling back to the static flag group (may search the wrong ISA)",
            file=sys.stderr,
        )
        recipe = BuildRecipe(flag_group_for(c_file), tuple(objcopy_steps), tuple(skipped), False)
    with _RECIPE_LOCK:
        _RECIPE_CACHE[c_file] = recipe
    return recipe


def replicate_objcopy(scratch: Path, recipe: BuildRecipe, c_file: Path, out_dir: Path) -> None:
    """Append the TU's post-compile objcopy chain to the scratch's compile.sh,
    retargeted at the scratch object, so the scratch object == the real
    per-TU object (workbench improvement-backlog #9). Records what was and
    was not replicated in <out_dir>/recipe.txt."""
    obj = f"build/{c_file.relative_to(ROOT).as_posix()}.o"
    csh = scratch / "compile.sh"
    lines = [f"flags: {' '.join(recipe.flags)} ({'gmake -n' if recipe.from_dry_run else 'static group'})"]
    # Whole-token match only: a plain substring replace would also rewrite
    # `build/x.c.o.syms`-style arguments into nonexistent paths, and a step
    # that does not mention the object at all cannot be retargeted (it would
    # reach the scratch still pointing at the real build tree).
    mention = re.compile(r"(?<![\w./-])(?:\./)?" + re.escape(obj) + r"(?![\w.-])")
    if recipe.objcopy_steps and csh.is_file():
        with open(csh, "a") as f:
            f.write("\n")
            for step in recipe.objcopy_steps:
                if not mention.search(step):
                    lines.append(f"skipped (does not name the object): {step}")
                    continue
                remapped = mention.sub('"$OUTPUT"', step)
                f.write(remapped + "\n")
                lines.append(f"replicated: {remapped}")
    for s in recipe.skipped_postproc:
        lines.append(f"skipped (not replicable): {s}")
    (out_dir / "recipe.txt").write_text("\n".join(lines) + "\n")


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
            text = c_file.read_text(errors="replace")
            body_funcs = {
                name
                for block in iter_nonmatching_blocks(text)
                if (name := block_function_name(text, block)) is not None
            }
            # An atlas row identifies a C translation unit, not necessarily
            # a function. Consolidated overlay TUs therefore have source
            # basenames such as overlay_001 while their queued functions are
            # found by the source scan below. Do not invent a queue symbol.
            if func not in body_funcs:
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
        for block in iter_nonmatching_blocks(text):
            func = block_function_name(text, block)
            if func is None:
                continue
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
    for block in iter_nonmatching_blocks(text):
        if block_function_name(text, block) == item.func:
            am = GLOBAL_ASM_RE.search(block.fallback)
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
    flags: Optional[str] = None  # real codegen flags the scratch compiled with
    replicated_objcopy: int = 0  # post-compile objcopy steps appended to compile.sh
    extended: bool = False  # score-trend extension run happened
    stopped_flat: bool = False  # stopped early: no improvement by --flat-minutes
    commit_error: Optional[str] = None


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
    labels = [m.group(2) for m in GLABEL_RE.finditer(text)]
    label_set = set(labels)
    # The file stem is splat's symbol for the owned function. Auxiliary
    # labels (local branches and jump-table/data labels) must retain their
    # identities and relocations. Fall back to the C symbol for resident
    # assembly that already uses its friendly name.
    if asm_path.stem in label_set:
        auto_name = asm_path.stem
    elif item.func in label_set:
        auto_name = item.func
    else:
        raise RuntimeError(
            f"{asm_rel}: neither target stem {asm_path.stem!r} nor "
            f"C symbol {item.func!r} is defined"
        )
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
    # Keep the empty parent directory. Removing it races another concurrent
    # import.py between its os.makedirs("nonmatchings") and per-function
    # os.mkdir calls, producing a sporadic FileNotFoundError at --jobs > 1.
    return scratch


def _improved_over_base(scratch: Path, log_path: Path) -> bool:
    """True if some output-*/score.txt is strictly below the base score.
    The permuter also writes equal-score outputs, which are not progress."""
    text = log_path.read_text(errors="replace") if log_path.is_file() else ""
    m = re.search(r"base score = (\d+)", text)
    base = int(m.group(1)) if m else None
    for d in scratch.glob("output-*"):
        f = d / "score.txt"
        try:
            score = int(f.read_text().strip())
        except (OSError, ValueError):
            continue
        if base is None or score < base:
            return True
    return False


def wait_for_headroom(threshold: float, label: str = "") -> None:
    """Block until the 1-minute load average is under `threshold`. The
    machine froze under an unthrottled fleet (load ~20 on 14 cores); every
    compile-heavy launch here gates on headroom first."""
    if threshold <= 0:
        return
    waited = 0
    while True:
        try:
            load = os.getloadavg()[0]
        except OSError:
            return
        if load < threshold:
            return
        if waited == 0:
            print(f"[headroom] load {load:.1f} >= {threshold:.1f}; waiting {label}".rstrip())
        time.sleep(15)
        waited += 15


def run_permuter(scratch: Path, out_dir: Path, minutes: int, threads: int, extra_args: list[str],
                 log_name: str = "permuter.log", flat_minutes: int = 0) -> tuple[Optional[int], float, bool]:
    """Run one permuter search. Returns (base_score, elapsed_seconds, stopped_flat)."""
    log_path = out_dir / log_name
    # --stack-diffs is essential for a byte-identical rebuild: without it the
    # scorer normalizes sp-relative offsets and reports a false 0 for a spill
    # at the wrong slot (docs/matching-triage.md, func_80012574). nice keeps
    # the search from starving integration builds.
    args = [
        "nice", "-n", "15",
        str(PYTHON),
        str(PERMUTER_PY),
        "--stop-on-zero",
        "--quiet",
        "-j",
        str(threads),
    ]
    if "--stack-diffs" not in extra_args:
        args.append("--stack-diffs")
    args += [*extra_args, str(scratch)]
    start = time.monotonic()
    with open(log_path, "w") as log_f:
        # permuter.py -j N forks worker processes; killing only the parent on
        # timeout reparents the idle workers to PID 1 (observed: three batches
        # of orphans after one morning of sweeping). Run the search in its own
        # session and kill the whole process group at the cap.
        proc = subprocess.Popen(
            args,
            cwd=ROOT,
            stdout=log_f,
            stderr=subprocess.STDOUT,
            start_new_session=True,
        )
        # Early stop when flat: a search that has produced no improvement at
        # all after `flat_minutes` almost never does later (measured on the
        # first sweep day: every 20-minute run that was flat at 6 minutes was
        # still flat at 20). Improvements appear as output-* dirs, so poll
        # for one; the extension heuristic covers the descending case.
        deadline = time.monotonic() + minutes * 60
        flat_deadline = time.monotonic() + flat_minutes * 60 if flat_minutes > 0 else None
        returncode = None
        while True:
            try:
                returncode = proc.wait(timeout=20)
                break
            except subprocess.TimeoutExpired:
                pass
            now = time.monotonic()
            if now >= deadline:
                returncode = 124
                break
            if flat_deadline is not None and now >= flat_deadline:
                if not _improved_over_base(scratch, out_dir / log_name):
                    returncode = 125  # stopped flat
                    break
                flat_deadline = None
        if returncode in (124, 125):
            try:
                os.killpg(proc.pid, signal.SIGTERM)
                proc.wait(timeout=15)
            except (ProcessLookupError, subprocess.TimeoutExpired):
                try:
                    os.killpg(proc.pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass
                proc.wait()
    elapsed = time.monotonic() - start
    text = log_path.read_text(errors="replace")
    m = re.search(r"base score = (\d+)", text)
    base_score = int(m.group(1)) if m else None
    return base_score, elapsed, returncode == 125


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


def _best(scratch: Path) -> tuple[Optional[Path], Optional[int]]:
    best_dir = best_output_dir(scratch)
    if best_dir is None:
        return None, None
    score_file = best_dir / "score.txt"
    return best_dir, int(score_file.read_text().strip()) if score_file.is_file() else None


def commit_match(item: QueueItem) -> Optional[str]:
    """Commit a verified promotion on the current branch. Returns an error
    string, or None. Only the function's own C file is staged, so a batch
    never sweeps unrelated working-tree changes into a match commit."""
    with PROMOTE_LOCK:
        paths = [item.rel_c_file]
        if item.overlay is not None:
            # An overlay promotion changes the module's ownership rows; the
            # atlas (and its digest) must be regenerated or `gmake
            # overlay-atlas` fails and the overlay bytes are never credited.
            for cmd in (["gmake", "overlay-atlas-write"],
                        [str(PYTHON), "tools/refresh_atlas_digest.py"]):
                r = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, timeout=600)
                if r.returncode != 0:
                    return f"{' '.join(cmd)} failed: " + (r.stdout + r.stderr)[-2000:]
            paths += ["config/overlays.us.json", "config/overlay-donors.us.json", "mickey.us.yaml"]
            paths = [p for p in paths if (ROOT / p).exists()]
        add = subprocess.run(["git", "add", "--", *paths], cwd=ROOT, capture_output=True, text=True)
        if add.returncode != 0:
            return "git add failed: " + add.stderr[-2000:]
        msg = (
            f"Match {item.func} (permuter)\n\n"
            f"Found by tools/permute_batch.py (real per-file flags, replicated objcopy,\n"
            f"--stack-diffs); promoted only after gmake verify on the project build path.\n"
            f"Form may be permuter-shaped; see docs/cleanup-queue.md policy."
        )
        c = subprocess.run(["git", "commit", "-q", "-m", msg], cwd=ROOT, capture_output=True, text=True)
        if c.returncode != 0:
            return "git commit failed: " + (c.stdout + c.stderr)[-3000:]
    return None


def run_one(item: QueueItem, minutes: int, permuter_threads: int, build_jobs: int, apply: bool,
            extra_args: list[str], load_threshold: float = 0.0, extend_minutes: int = 0,
            commit: bool = False, flat_minutes: int = 0) -> RunResult:
    out_dir = BUILD_PERMUTER / item.func
    out_dir.mkdir(parents=True, exist_ok=True)
    result = RunResult(func=item.func, c_file=item.rel_c_file, overlay=item.overlay, ok=False)
    start = time.monotonic()
    try:
        recipe = build_recipe_for(item.c_file)
        result.flags = " ".join(recipe.flags)
        result.replicated_objcopy = len(recipe.objcopy_steps)
        settings_path = out_dir / "permuter_settings.toml"
        write_settings_toml(settings_path, recipe.flags)
        target_asm = prepare_target_asm(item, out_dir)
        scratch = run_import(item, out_dir, settings_path, target_asm)
        replicate_objcopy(scratch, recipe, item.c_file, out_dir)
        wait_for_headroom(load_threshold, f"before permuting {item.func}")
        base_score, elapsed, stopped_flat = run_permuter(
            scratch, out_dir, minutes, permuter_threads, extra_args, flat_minutes=flat_minutes)
        result.base_score = base_score
        result.stopped_flat = stopped_flat
        result.ok = True

        best_dir, best_score = _best(scratch)
        # Score-trend extension: if the run hit the cap and its best result
        # landed in the final third of the window, the search was still
        # descending -- re-seed from the best candidate and run once more.
        # A search that found its best early and then sat is not extended
        # (docs/permute-batch.md: search time is not the fix for a plateau).
        if (extend_minutes > 0 and best_dir is not None and best_score not in (None, 0)
                and elapsed >= minutes * 60 * 0.95):
            age = time.time() - best_dir.stat().st_mtime
            if age < minutes * 60 / 3:
                result.extended = True
                shutil.copy(best_dir / "source.c", scratch / "base.c")
                wait_for_headroom(load_threshold, f"before extending {item.func}")
                run_permuter(scratch, out_dir, extend_minutes, permuter_threads, extra_args, "permuter-extend.log")
                best_dir, best_score = _best(scratch)
        result.best_score = best_score
        if best_score == 0 and best_dir is not None:
            result.zero_found = True
            if apply:
                wait_for_headroom(load_threshold, f"before promoting {item.func}")
                promoted, err = promote(item, best_dir / "source.c", build_jobs)
                result.promoted = promoted
                result.promote_error = err
                if promoted and commit:
                    result.commit_error = commit_match(item)
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
        default=6,
        help="gmake -jN to use when promoting a zero-score match (default: 6, the "
        "integration-build cap that keeps the workstation responsive)",
    )
    p.add_argument(
        "--order",
        choices=["ranking", "queue"],
        default="ranking",
        help="ranking: closest first by config/nonmatching-ranking.us.json differing_words "
        "(unranked last, then by name); queue: discovery order (default: ranking)",
    )
    p.add_argument(
        "--resume",
        action="store_true",
        help="skip functions already present in build/permuter/summary.json (re-run only "
        "what a previous batch did not reach)",
    )
    p.add_argument(
        "--extend-minutes",
        type=int,
        default=0,
        help="if a run hits its cap while its score was still descending (best result in "
        "the last third of the window), re-seed from the best candidate and run this many "
        "more minutes once (default: 0 = off)",
    )
    p.add_argument(
        "--flat-minutes",
        type=int,
        default=6,
        help="stop a search early when it has produced no improvement at all by this "
        "many minutes (default: 6; 0 disables). Flat-at-six searches were flat-at-twenty "
        "on every measured run",
    )
    p.add_argument(
        "--load-threshold",
        type=float,
        default=9.0,
        help="wait until the 1-min load average is below this before each permuter launch "
        "and each promotion build (default: 9.0; 0 disables)",
    )
    p.add_argument(
        "--commit",
        action="store_true",
        help="with --apply: git-commit each verified promotion (only the function's C file "
        "is staged) as 'Match <fn> (permuter)'",
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
    if args.order == "ranking":
        rank: dict[str, tuple[int, int]] = {}
        if RANKING_PATH.is_file():
            for row in json.loads(RANKING_PATH.read_text()).get("functions", []):
                words = row.get("differing_words")
                if isinstance(words, int):
                    rank[row["name"]] = (words, row.get("size_bytes") or 0)
        queue.sort(key=lambda it: (rank.get(it.func, (10**9, 0)), it.func))
        unranked = sum(1 for it in queue if it.func not in rank)
        if unranked:
            print(f"note: {unranked} queued function(s) have no ranking row; they run last")
    if args.resume and SUMMARY_JSON.is_file():
        # A row that errored or never got a base score (import/compile fault)
        # is not "done": the fault may have been fixed since.
        done = {r["func"] for r in json.loads(SUMMARY_JSON.read_text()).get("results", [])
                if not r.get("error") and r.get("base_score") is not None}
        before = len(queue)
        queue = [it for it in queue if it.func not in done]
        print(f"--resume: skipping {before - len(queue)} already-run function(s)")
    if args.limit is not None:
        queue = queue[: args.limit]
    if args.commit and not args.apply:
        print("--commit requires --apply", file=sys.stderr)
        return 2

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
    if args.resume and SUMMARY_JSON.is_file():
        # Carry the earlier results forward so summary.json stays the whole
        # sweep's record, not just this invocation's slice.
        known = {f.name for f in dataclasses.fields(RunResult)}
        for row in json.loads(SUMMARY_JSON.read_text()).get("results", []):
            results.append(RunResult(**{k: v for k, v in row.items() if k in known}))

    print(
        f"Running {len(queue)} function(s), {jobs} concurrent, "
        f"{permuter_threads} permuter thread(s) each, {args.minutes} min cap each"
        + (", apply=on" if args.apply else ", apply=off (report only)")
    )

    if jobs == 1:
        for it in queue:
            r = run_one(it, args.minutes, permuter_threads, args.build_jobs, args.apply, extra_args,
                        args.load_threshold, args.extend_minutes, args.commit, args.flat_minutes)
            results.append(r)
            print_result(r)
            write_summary(results)
    else:
        with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as pool:
            futures = {
                pool.submit(
                    run_one, it, args.minutes, permuter_threads, args.build_jobs, args.apply, extra_args,
                    args.load_threshold, args.extend_minutes, args.commit, args.flat_minutes,
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
    if r.promoted:
        status = "MATCHED"
    elif r.zero_found:
        status = "zero-found"
    elif r.best_score is None or (r.base_score is not None and r.best_score >= r.base_score):
        status = "flat" if r.stopped_flat else "no improvement"
    else:
        status = f"improved{' (extended)' if r.extended else ''}"
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
