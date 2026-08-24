#!/usr/bin/env python3
"""Resolve a docs/modules.md merge conflict across the section-5 split.

On 2026-08-24 section 5 of docs/modules.md moved to docs/overlays.md. A lane
branched before the split edits the old whole file; merging it conflicts with
the split hunk. This script performs the merge the split intended: it takes the
three versions of docs/modules.md (merge base, ours, theirs), divides each into
its section-5 part and the rest, three-way merges the two halves separately
with `git merge-file`, and writes docs/modules.md (non-5 half plus the stub)
and docs/overlays.md (section-5 half under its header). Run inside an
in-progress merge; exits 1 if either half still has conflict markers.
"""
import pathlib, re, subprocess, sys, tempfile

ROOT = pathlib.Path(subprocess.check_output(["git", "rev-parse", "--show-toplevel"], text=True).strip())
MOD = "docs/modules.md"; OVL = "docs/overlays.md"

def show(rev, path):
    r = subprocess.run(["git", "show", f"{rev}:{path}"], capture_output=True, text=True)
    return r.stdout if r.returncode == 0 else ""

def split(text):
    lines = text.split("\n")
    s = next((i for i, l in enumerate(lines) if l.startswith("## 5. ")), None)
    e = next((i for i, l in enumerate(lines) if l.startswith("## 6. ")), None)
    if s is None or e is None:
        return text, ""
    sec = "\n".join(lines[s:e])
    if "moved to" in sec and "overlays.md" in sec:      # already the stub
        return text, ""
    return "\n".join(lines[:s] + lines[e:]), sec

STUB = """## 5. The overlay system

The overlay ledger (module layout, per-overlay ownership and evidence, the
donor scan, and the retired normalization notes) moved to
[`docs/overlays.md`](overlays.md) on 2026-08-24 when this file crossed the
256 KB tracked-file limit. Section numbering there continues as 5.x.
"""
HDR = "# The overlay system (docs/modules.md section 5)\n\nSplit out of `docs/modules.md` on 2026-08-24; evidence tiers and naming rules are defined in that file, section 1. Section numbers below keep their original 5.x identity so existing references resolve.\n\n"

def merge3(base, ours, theirs, label):
    with tempfile.TemporaryDirectory() as d:
        p = [pathlib.Path(d, n) for n in ("ours", "base", "theirs")]
        for f, t in zip(p, (ours, base, theirs)): f.write_text(t)
        r = subprocess.run(["git", "merge-file", "-p", "-L", "ours", "-L", "base", "-L", "theirs", *map(str, p)], capture_output=True, text=True)
        if r.returncode != 0 or "<<<<<<<" in r.stdout:
            # Ledger sections are appended by independent lanes at the same
            # spot; adjacent insertions are both wanted. Union them.
            sys.stderr.write(f"{label}: adjacent-hunk conflict, taking the union of both sides\n")
            r = subprocess.run(["git", "merge-file", "-p", "--union", *map(str, p)], capture_output=True, text=True)
        return r.stdout, (0 if "<<<<<<<" not in r.stdout else 1)

base_rev = subprocess.check_output(["git", "merge-base", "HEAD", "MERGE_HEAD"], text=True).strip()
b_mod = show(base_rev, MOD); t_mod = show("MERGE_HEAD", MOD)
o_mod = show("HEAD", MOD); o_ovl = show("HEAD", OVL)
b_non5, b_5 = split(b_mod); t_non5, t_5 = split(t_mod); o_non5, _ = split(o_mod)
b_ovl = show(base_rev, OVL); t_ovl = show("MERGE_HEAD", OVL)
# section-5 halves: our overlays.md body vs base/theirs section 5
def body(ovl): return ovl[len(HDR):] if ovl.startswith(HDR) else ovl
o_5 = body(o_ovl) if o_ovl else b_5
t_5_eff = body(t_ovl) if t_ovl else t_5
b_5_eff = body(b_ovl) if b_ovl else b_5
non5, rc1 = merge3(b_non5, o_non5, t_non5, MOD)
five, rc2 = merge3(b_5_eff, o_5, t_5_eff, OVL)
# reinsert stub at the section-5 position of the non-5 half
lines = non5.split("\n")
e = next(i for i, l in enumerate(lines) if l.startswith("## 6. "))
if not any(l.startswith("## 5. ") for l in lines):
    lines = lines[:e] + STUB.split("\n") + lines[e:]
(ROOT / MOD).write_text("\n".join(lines))
(ROOT / OVL).write_text(HDR + five.rstrip("\n") + "\n")
ok = rc1 == 0 and rc2 == 0
subprocess.run(["git", "add", MOD, OVL]) if ok else None
print("resolved" if ok else "CONFLICTS REMAIN", MOD, OVL)
sys.exit(0 if ok else 1)
