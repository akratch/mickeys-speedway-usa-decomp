#!/usr/bin/env python3
"""Delete the asm/ files splat orphaned but did not remove.

splat writes asm/, it never prunes it. Two edits leave a file behind that no
longer belongs to anything, and neither one fails a build:

  * an `asm` subsegment becoming `c`. asm/<name>.s survives with a glabel for
    every function in the translation unit. The link is unaffected (the
    generated linker script names objects explicitly and stops naming that
    one), so the ROM still verifies -- but tools/progress.py counts a function
    as unmatched while any glabel for it survives anywhere under asm/, so the
    matched count silently under-reports until the file is deleted by hand.
    That is how the libultra pass lost several functions off its own scoreboard.

  * a function being named in symbol_addrs. splat writes
    asm/nonmatchings/<tu>/<newname>.s and leaves <oldname>.s next to it.

Both are removed here, on the same rule: a file under asm/ that the current
configuration cannot produce is stale.

  1. For every `c` subsegment named N in the splat yaml, asm/N.s must not
     exist. Only splat's `asm` subsegments are allowed to write there.
  2. Every file under asm/nonmatchings/ must be named by a
     `#pragma GLOBAL_ASM("...")` in src/. That set is exact rather than
     heuristic: a nonmatchings .s is only ever consumed through a pragma, and
     a pragma naming a missing file fails the build immediately.

Anything else under asm/ is left alone. In particular this does not try to
decide whether an unnamed asm/<ADDR>.s still corresponds to a subsegment;
splat rewrites those every split and a wrong guess there would delete real
input.

Usage:
    tools/prune_stale_asm.py [mickey.us.yaml ...]     (default: mickey.us.yaml)
    tools/prune_stale_asm.py --dry-run <yaml>
"""

import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ASM_DIR = os.path.join(ROOT, "asm")
SRC_DIR = os.path.join(ROOT, "src")

# - [0xADDR, c, some/name]
C_SUBSEG = re.compile(r"^\s*- \[\s*0x[0-9A-Fa-f]+\s*,\s*c\s*,\s*(\S+?)\s*\]\s*$")
PRAGMA = re.compile(r'#pragma\s+GLOBAL_ASM\("([^"]+)"\)')


def c_subsegment_names(yaml_paths):
    names = set()
    for path in yaml_paths:
        try:
            with open(path, encoding="utf-8") as fh:
                for line in fh:
                    m = C_SUBSEG.match(line)
                    if m:
                        names.add(m.group(1))
        except OSError:
            continue
    return names


def referenced_asm_files():
    refs = set()
    for root, _dirs, files in os.walk(SRC_DIR):
        for f in files:
            if not f.endswith((".c", ".h")):
                continue
            with open(os.path.join(root, f), encoding="utf-8",
                      errors="replace") as fh:
                for m in PRAGMA.finditer(fh.read()):
                    refs.add(os.path.normpath(os.path.join(ROOT, m.group(1))))
    return refs


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("-")]
    dry_run = "--dry-run" in sys.argv[1:]
    yaml_paths = [os.path.join(ROOT, a) for a in args] or \
        [os.path.join(ROOT, "mickey.us.yaml")]

    if not os.path.isdir(ASM_DIR):
        return 0

    stale = []

    for name in sorted(c_subsegment_names(yaml_paths)):
        path = os.path.join(ASM_DIR, name + ".s")
        if os.path.isfile(path):
            stale.append((path, f"subsegment `{name}` is `c`"))

    nonmatchings = os.path.join(ASM_DIR, "nonmatchings")
    if os.path.isdir(nonmatchings):
        refs = referenced_asm_files()
        for root, _dirs, files in os.walk(nonmatchings):
            for f in files:
                if not f.endswith(".s"):
                    continue
                path = os.path.join(root, f)
                if os.path.normpath(path) not in refs:
                    stale.append((path, "no #pragma GLOBAL_ASM names it"))

    for path, why in stale:
        rel = os.path.relpath(path, ROOT)
        print(f"prune_stale_asm: {'would remove' if dry_run else 'removing'} "
              f"{rel} -- {why}")
        if not dry_run:
            os.remove(path)

    # Empty directories left behind by rule 2 are removed too, so a renamed
    # translation unit does not leave a hollow asm/nonmatchings/<tu>/.
    if not dry_run:
        for root, dirs, files in os.walk(nonmatchings, topdown=False):
            if root != nonmatchings and not dirs and not files:
                os.rmdir(root)

    if stale:
        print(f"prune_stale_asm: {len(stale)} stale file(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
