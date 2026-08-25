#!/usr/bin/env python3
"""Resolve docs/modules.md merge conflicts across its section splits.

docs/modules.md was split to stay under the 256 KB tracked-file limit:
section 5 lives in docs/overlays.md (2026-08-24) and section 3 in
docs/resident.md (2026-08-25). A lane branched before a split edits the old
whole file; merging it conflicts with the split hunk. This script performs
the merge the split intended: it takes the three versions of docs/modules.md
(merge base, ours, theirs), divides each into the moved sections and the
rest, three-way merges each part separately with `git merge-file` (union on
adjacent ledger insertions), and writes docs/modules.md (rest plus stubs) and
each split file. Run inside an in-progress merge; also usable standalone to
perform the split on a working tree (`--split-only`).
"""
import pathlib, re, subprocess, sys, tempfile

ROOT = pathlib.Path(subprocess.check_output(["git", "rev-parse", "--show-toplevel"], text=True).strip())
MOD = "docs/modules.md"
SPLITS = {  # section number -> (file, header, stub)
    5: ("docs/overlays.md",
        "# The overlay system (docs/modules.md section 5)\n\nSplit out of `docs/modules.md` on 2026-08-24; evidence tiers and naming rules are defined in that file, section 1. Section numbers below keep their original 5.x identity so existing references resolve.\n\n",
        "## 5. The overlay system\n\nThe overlay ledger (module layout, per-overlay ownership and evidence, the\ndonor scan, and the retired normalization notes) moved to\n[`docs/overlays.md`](overlays.md) on 2026-08-24 when this file crossed the\n256 KB tracked-file limit. Section numbering there continues as 5.x.\n"),
    3: ("docs/resident.md",
        "# The resident segment (docs/modules.md section 3)\n\nSplit out of `docs/modules.md` on 2026-08-25; evidence tiers and naming rules are defined in that file, section 1. Section numbers below keep their original 3.x identity so existing references resolve.\n\n",
        "## 3. The resident segment (`main`)\n\nThe resident ledger (per-translation-unit census, evidence, plateaus and\nPROVENANCE) moved to [`docs/resident.md`](resident.md) on 2026-08-25 when\nthis file crossed the 256 KB tracked-file limit again. Section numbering\nthere continues as 3.x.\n"),
}

def show(rev, path):
    r = subprocess.run(["git", "show", f"{rev}:{path}"], capture_output=True, text=True)
    return r.stdout if r.returncode == 0 else ""

def split(text):
    """Return (rest, {section: body}); stubs count as absent."""
    lines = text.split("\n"); out = {}; keep = []; i = 0
    heads = [(n, i) for i, l in enumerate(lines) if (n := re.match(r"## (\d+)\. ", l)) ]
    starts = {int(m.group(1)): i for m, i in heads}
    ends = {int(m.group(1)): (heads[k + 1][1] if k + 1 < len(heads) else len(lines)) for k, (m, i) in enumerate(heads)}
    moved = set()
    for sec in SPLITS:
        if sec in starts:
            body = "\n".join(lines[starts[sec]:ends[sec]])
            if "moved to" in body and SPLITS[sec][0].split("/")[-1] in body:
                continue  # already a stub
            out[sec] = body; moved.add(sec)
    for sec in sorted(moved, reverse=True):
        lines[starts[sec]:ends[sec]] = SPLITS[sec][2].split("\n")
    return "\n".join(lines), out

def body(text, sec):
    hdr = SPLITS[sec][1]
    return text[len(hdr):] if text.startswith(hdr) else text

def merge3(base, ours, theirs, label):
    with tempfile.TemporaryDirectory() as d:
        p = [pathlib.Path(d, n) for n in ("ours", "base", "theirs")]
        for f, t in zip(p, (ours, base, theirs)): f.write_text(t)
        r = subprocess.run(["git", "merge-file", "-p", "-L", "ours", "-L", "base", "-L", "theirs", *map(str, p)], capture_output=True, text=True)
        if r.returncode != 0 or "<<<<<<<" in r.stdout:
            sys.stderr.write(f"{label}: adjacent-hunk conflict, taking the union of both sides\n")
            r = subprocess.run(["git", "merge-file", "-p", "--union", *map(str, p)], capture_output=True, text=True)
        return r.stdout, (0 if "<<<<<<<" not in r.stdout else 1)

def write_split(rest, parts):
    (ROOT / MOD).write_text(rest)
    for sec, txt in parts.items():
        f, hdr, _ = SPLITS[sec]
        (ROOT / f).write_text(hdr + txt.rstrip("\n") + "\n")

if "--split-only" in sys.argv:
    rest, parts = split((ROOT / MOD).read_text())
    for sec in parts:
        f = ROOT / SPLITS[sec][0]
        if f.exists():  # append after existing content of that split file
            parts[sec] = body(f.read_text(), sec).rstrip("\n") + "\n\n" + parts[sec]
    write_split(rest, parts); print("split:", ", ".join(SPLITS[s][0] for s in parts) or "nothing to do"); sys.exit(0)

base_rev = subprocess.check_output(["git", "merge-base", "HEAD", "MERGE_HEAD"], text=True).strip()
b_rest, b_parts = split(show(base_rev, MOD)); o_rest, o_parts = split(show("HEAD", MOD)); t_rest, t_parts = split(show("MERGE_HEAD", MOD))
rest, rc = merge3(b_rest, o_rest, t_rest, MOD); ok = rc == 0
out_parts = {}
for sec, (f, hdr, _) in SPLITS.items():
    b = body(show(base_rev, f), sec) if show(base_rev, f) else b_parts.get(sec, "")
    o = body(show("HEAD", f), sec) if show("HEAD", f) else o_parts.get(sec, "")
    t = body(show("MERGE_HEAD", f), sec) if show("MERGE_HEAD", f) else t_parts.get(sec, "")
    if not (b or o or t): continue
    m, rc = merge3(b, o, t, f); ok = ok and rc == 0; out_parts[sec] = m
write_split(rest, out_parts)
if ok: subprocess.run(["git", "add", MOD, *[SPLITS[s][0] for s in out_parts]])
print("resolved" if ok else "CONFLICTS REMAIN", MOD, *[SPLITS[s][0] for s in out_parts]); sys.exit(0 if ok else 1)
