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

  3. An unnamed `asm/<ADDR>.s` whose address is the start of a `c`
     subsegment. splat names an unnamed `asm` subsegment after its ROM
     offset; once that row is carved into a `c` subsegment the file can no
     longer be produced, but its glabels still hide every function in the
     new translation unit from the scoreboard (the Epoch 15 carves hit this).
Anything else under asm/ is left alone: an unnamed asm/<ADDR>.s whose address
is not a `c` subsegment start may still be a live `asm` row that splat
rewrites every split, and a wrong guess there would delete real input.

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
C_SUBSEG = re.compile(r"^\s*- \[\s*0x([0-9A-Fa-f]+)\s*,\s*c\s*,\s*(\S+?)\s*\]\s*$")
PRAGMA = re.compile(r'#pragma\s+GLOBAL_ASM\("([^"]+)"\)')


def c_subsegments(yaml_paths):
    """Return ({name}, {start address}) of every `c` subsegment row."""
    names = set()
    starts = set()
    for path in yaml_paths:
        try:
            with open(path, encoding="utf-8") as fh:
                for line in fh:
                    m = C_SUBSEG.match(line)
                    if m:
                        starts.add(int(m.group(1), 16))
                        names.add(m.group(2))
        except OSError:
            continue
    return names, starts


def c_subsegment_names(yaml_paths):
    return c_subsegments(yaml_paths)[0]


UNNAMED_ASM = re.compile(r"^([0-9A-Fa-f]+)\.s$")


def stale_unnamed_asm(asm_dir, c_starts):
    """Rule 3: asm/<ADDR>.s where 0xADDR now starts a `c` subsegment."""
    out = []
    try:
        entries = sorted(os.listdir(asm_dir))
    except OSError:
        return out
    for f in entries:
        m = UNNAMED_ASM.match(f)
        if m and int(m.group(1), 16) in c_starts:
            out.append((os.path.join(asm_dir, f),
                        f"0x{m.group(1).upper()} is now a `c` subsegment start"))
    return out


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

    c_names, c_starts = c_subsegments(yaml_paths)
    for name in sorted(c_names):
        path = os.path.join(ASM_DIR, name + ".s")
        if os.path.isfile(path):
            stale.append((path, f"subsegment `{name}` is `c`"))
    stale.extend(stale_unnamed_asm(ASM_DIR, c_starts))

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
