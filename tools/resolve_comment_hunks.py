#!/usr/bin/env python3
"""Resolve merge-conflict hunks that differ only in comment lines (take ours).

Lanes rewrite the explanatory comments in mickey.us.yaml (the jump-table
inventory paragraph, most often); those hunks carry no data. Any hunk with a
non-comment line on either side is left for a human. Usage: FILE [FILE...]
"""
import re, sys
pat = re.compile(r"<<<<<<< [^\n]*\n(.*?)=======\n(.*?)>>>>>>> [^\n]*\n", re.S)
def only_comments(block):
    return all(l.strip() == "" or l.lstrip().startswith("#") for l in block.split("\n"))
for path in sys.argv[1:]:
    s = open(path).read(); left = 0
    def sub(m):
        global left
        if only_comments(m.group(1)) and only_comments(m.group(2)):
            return m.group(1)
        left += 1; return m.group(0)
    s = pat.sub(sub, s); open(path, "w").write(s)
    print(f"{path}: {'resolved' if left == 0 else f'{left} hunk(s) left'}")
    if left: sys.exit(1)
