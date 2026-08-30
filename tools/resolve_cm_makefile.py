#!/usr/bin/env python3
"""Resolve overlay-policy conflicts between consolidation lanes.

Each lane rewrote the per-object rules of its own overlays; two lanes'
hunks overlap because their overlays' rules sit next to each other. Within
each conflict hunk, rules are grouped into blocks (leading comments, the
target line, and its backslash-continued recipe lines). Blocks about overlays
already consolidated on this branch come from ours; every other block comes
from the lane (theirs).

Usage: resolve_cm_makefile.py <comma list of overlay numbers consolidated on HEAD> [mk/overlays.mk]
"""
import pathlib, re, sys
ours_ovs = {int(x) for x in sys.argv[1].split(',') if x}
path = sys.argv[2] if len(sys.argv) > 2 else (
    'mk/overlays.mk' if pathlib.Path('mk/overlays.mk').is_file()
    else 'Makefile'
)
s = open(path).read()
pat = re.compile(r'<<<<<<< [^\n]*\n(.*?)=======\n(.*?)>>>>>>> [^\n]*\n', re.S)
ovre = re.compile(r'overlays/o(\d{3})/')
def blocks(text):
    out, cur, cont = [], [], False
    for l in text.split('\n'):
        if l == '' and not cur: continue
        starts_new = not cont and not l.startswith(('\t', ' ')) 
        if starts_new and cur and not (cur[-1].lstrip().startswith('#') and l.lstrip().startswith('#')) and not cur[-1].lstrip().startswith('#'):
            out.append(cur); cur = []
        cur.append(l); cont = l.rstrip().endswith('\\')
    if cur: out.append(cur)
    return out
def about(b):
    for l in b:
        m = ovre.search(l)
        if m: return int(m.group(1))
    return None
objre = re.compile(r'^\s*\$\(BUILD_DIR\).*\.o \\$')
def is_objlist(text):
    ls = [l for l in text.split('\n') if l.strip()]
    return bool(ls) and all(objre.match(l) for l in ls)
def resolve(m):
    if is_objlist(m.group(1)) and is_objlist(m.group(2)):
        # one object per line: per-line classification, no blank lines
        keep = [l for l in m.group(2).split('\n') if l.strip() and (about([l]) not in ours_ovs)]
        add = [l for l in m.group(1).split('\n') if l.strip() and (about([l]) in ours_ovs)]
        return '\n'.join(keep + add) + '\n'
    ob, tb = blocks(m.group(1)), blocks(m.group(2))
    keep = [b for b in tb if about(b) not in ours_ovs]
    add = [b for b in ob if about(b) in ours_ovs]
    lines = [l for b in keep + add for l in b]
    return ('\n'.join(lines) + '\n') if lines else ''
n = len(pat.findall(s)); s = pat.sub(resolve, s)
assert '<<<<<<<' not in s; open(path, 'w').write(s); print(f'{path}: {n} hunk(s) resolved')
