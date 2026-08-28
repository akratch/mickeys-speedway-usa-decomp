#!/usr/bin/env python3
"""Refuse a merge that silently un-matches a function.

Compares the set of functions still carried by `#pragma GLOBAL_ASM` under
src/main/ and src/overlays/ between a base commit (default HEAD) and the
working tree.  Any name that had no GLOBAL_ASM at the base but has one now
was matched before the merge and is not matched after it: a lane's older
base won a whole-file conflict resolution, or a rebase dropped a body.

The one exception is a pragma in a brand-new C file that replaces a raw
`asm`/`hasm` subsegment at the same configured ROM start.  Those functions
were already unresolved assembly; moving the range into a mixed C translation
unit merely makes their fallback representation explicit.

    tools/check_match_regression.py [base-commit]

Exit 1 with the offending names on stderr; exit 0 (quietly) otherwise.
"""
import re
import subprocess
import sys

PAT = re.compile(r'#pragma GLOBAL_ASM\("asm/[^"]*/([^/"]+)\.s"\)')
SUBSEGMENT_PAT = re.compile(
    r"^\s*-\s*\[\s*(0x[0-9A-Fa-f]+)\s*,\s*([A-Za-z0-9_.]+)"
    r"(?:\s*,\s*([A-Za-z0-9_./-]+))?"
)
DIRS = ("src/main/", "src/overlays/", "src/libultra/")


def names_at(commit):
    files = subprocess.run(["git", "ls-tree", "-r", "--name-only", commit, *DIRS],
                           capture_output=True, text=True, check=True).stdout.split()
    out = set()
    for f in files:
        if not f.endswith(".c"):
            continue
        blob = subprocess.run(["git", "show", f"{commit}:{f}"], capture_output=True, text=True).stdout
        out.update(PAT.findall(blob))
    return out


def names_in_worktree():
    out = set()
    files = subprocess.run(["git", "ls-files", *DIRS], capture_output=True, text=True, check=True).stdout.split()
    for f in files:
        if not f.endswith(".c"):
            continue
        try:
            out.update(PAT.findall(open(f, encoding="utf-8", errors="ignore").read()))
        except FileNotFoundError:
            pass
    return out


def subsegments(text):
    out = {}
    for line in text.splitlines():
        match = SUBSEGMENT_PAT.match(line)
        if match:
            out[int(match.group(1), 16)] = (match.group(2), match.group(3))
    return out


def raw_asm_promotions(base):
    """Names newly exposed as pragmas by an asm/hasm-to-C split change."""
    base_yaml = subprocess.run(
        ["git", "show", f"{base}:mickey.us.yaml"],
        capture_output=True,
        text=True,
        check=True,
    ).stdout
    try:
        with open("mickey.us.yaml", encoding="utf-8") as fh:
            worktree_yaml = fh.read()
    except FileNotFoundError:
        return set()

    before = subsegments(base_yaml)
    after = subsegments(worktree_yaml)
    names = set()
    for start, (kind, source_name) in after.items():
        old_kind, _ = before.get(start, (None, None))
        if kind != "c" or not source_name or old_kind not in {"asm", "hasm"}:
            continue

        source = f"src/{source_name}.c"
        existed = subprocess.run(
            ["git", "cat-file", "-e", f"{base}:{source}"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        ).returncode == 0
        if existed:
            continue

        try:
            with open(source, encoding="utf-8", errors="ignore") as fh:
                names.update(PAT.findall(fh.read()))
        except FileNotFoundError:
            pass
    return names


def main():
    base = sys.argv[1] if len(sys.argv) > 1 else "HEAD"
    before = names_at(base)
    after = names_in_worktree()
    regressed = sorted(after - before - raw_asm_promotions(base))
    if regressed:
        print(f"check_match_regression: {len(regressed)} function(s) matched at {base} carry GLOBAL_ASM again:",
              file=sys.stderr)
        for n in regressed:
            print(f"  {n}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
