#!/usr/bin/env python3
"""Refuse a merge that silently un-matches a function.

Compares the set of functions still carried by `#pragma GLOBAL_ASM` under
src/main/ and src/overlays/ between a base commit (default HEAD) and the
working tree.  Any name that had no GLOBAL_ASM at the base but has one now
was matched before the merge and is not matched after it: a lane's older
base won a whole-file conflict resolution, or a rebase dropped a body.

    tools/check_match_regression.py [base-commit]

Exit 1 with the offending names on stderr; exit 0 (quietly) otherwise.
"""
import re
import subprocess
import sys

PAT = re.compile(r'#pragma GLOBAL_ASM\("asm/[^"]*/([^/"]+)\.s"\)')
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


def main():
    base = sys.argv[1] if len(sys.argv) > 1 else "HEAD"
    before = names_at(base)
    after = names_in_worktree()
    regressed = sorted(after - before)
    if regressed:
        print(f"check_match_regression: {len(regressed)} function(s) matched at {base} carry GLOBAL_ASM again:",
              file=sys.stderr)
        for n in regressed:
            print(f"  {n}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
