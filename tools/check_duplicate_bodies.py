#!/usr/bin/env python3
"""Refuse a tree where a function has both an unguarded C definition and an
unguarded `#pragma GLOBAL_ASM` in the same translation unit.

A hunk-level merge of two lanes that each replaced neighbouring pragmas can
keep one lane's C body and the other lane's pragma line for the same
function; asm-processor then fails with `symbol ".text" defined twice` only
at the next clean build. Guarded (`#ifdef NON_MATCHING`) candidates are
allowed to coexist with their fallback pragma by design.

    tools/check_duplicate_bodies.py [src ...]
Exit 1 with the offending TU/function pairs on stderr; 0 otherwise.
"""
import os
import re
import sys

PRAGMA = re.compile(r'GLOBAL_ASM\("[^"]*/(\w+)\.s"\)')
DEF = re.compile(r'^[A-Za-z_][\w \*]*?\b(\w+)\(\s*[^;]*\)\s*\{?\s*$')


def scan(path):
    depth = []
    pragmas = {}
    defs = {}
    with open(path, encoding="utf-8", errors="ignore") as fh:
        for n, line in enumerate(fh, 1):
            s = line.strip()
            if s.startswith("#if"):
                depth.append("NON_MATCHING" in s)
            elif s.startswith("#endif") and depth:
                depth.pop()
            guarded = any(depth)
            m = PRAGMA.search(s)
            if m and not guarded:
                pragmas[m.group(1)] = n
                continue
            if guarded or line[:1].isspace() or s.startswith(("extern", "static inline", "//", "/*", "*")) or s.endswith(";"):
                continue
            m = DEF.match(line)
            if m:
                defs.setdefault(m.group(1), n)
    return [(name, defs[name], pragmas[name]) for name in pragmas if name in defs]


def main():
    roots = sys.argv[1:] or ["src"]
    bad = []
    for root in roots:
        for d, _dirs, files in os.walk(root):
            for f in files:
                if f.endswith(".c"):
                    p = os.path.join(d, f)
                    for name, dline, pline in scan(p):
                        bad.append(f"{p}: {name} defined at line {dline} and still GLOBAL_ASM at line {pline}")
    if bad:
        print("check_duplicate_bodies: " + "\n  ".join([""] + bad), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
