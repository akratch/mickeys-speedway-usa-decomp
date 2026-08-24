#!/usr/bin/env python3
"""Recompute the jump-table claim that check-docs reports as stale.

Resident TU splits change the number of jtbl_* symbols splat still emits;
docs/modules.md and mickey.us.yaml carry that number as a claim. This runs
check_derived_numbers and rewrites the claimed number on exactly the lines it
reports, so the claim is recomputed rather than remembered.
"""
import pathlib, re, subprocess, sys
ROOT = pathlib.Path(__file__).resolve().parent.parent
out = subprocess.run([sys.executable, "tools/check_derived_numbers.py"], cwd=ROOT, capture_output=True, text=True).stdout
fixed = 0
for m in re.finditer(r"^\s*(\S+):(\d+): claims (\d+) jump tables; asm/ contains (\d+) distinct", out, re.M):
    path, line, old, new = m.group(1), int(m.group(2)), m.group(3), m.group(4)
    p = ROOT / path; lines = p.read_text().split("\n")
    if old in lines[line - 1]:
        lines[line - 1] = lines[line - 1].replace(old, new, 1); p.write_text("\n".join(lines)); fixed += 1
        print(f"{path}:{line}: {old} -> {new}")
print(f"{fixed} claim(s) recomputed")
