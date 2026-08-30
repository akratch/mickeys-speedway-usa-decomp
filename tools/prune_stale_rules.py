#!/usr/bin/env python3
"""Drop make rules and object-list entries whose source file is gone.

After overlay consolidation the per-function .c files are deleted; a merge
can resurrect their rules. Any `$(BUILD_DIR)/$(SRC_DIR)/<p>.c.o` whose
`<p>.c` does not exist loses its rule block (target line + continuation
lines) and its object-list line. Reports consolidated overlays whose
overlay_NNN.c.o is missing from the object list.
"""
import pathlib, re, sys
ROOT = pathlib.Path(__file__).resolve().parent.parent
paths = [ROOT / 'Makefile', *sorted((ROOT / 'mk').glob('*.mk'))]
objre = re.compile(r'\$\(BUILD_DIR\)/\$\(SRC_DIR\)/([^\s:$%]+)\.c\.o')
texts = []
dropped = 0
for p in paths:
    if not p.is_file():
        continue
    lines = p.read_text().split('\n')
    out = []
    skip = False
    for l in lines:
        m = objre.search(l)
        if m and not (ROOT / 'src' / (m.group(1) + '.c')).exists():
            dropped += 1
            skip = l.rstrip().endswith('\\') and l.lstrip().startswith('$(BUILD_DIR)')
            continue
        if skip:
            if l.startswith(('\t', ' ')):
                if not l.rstrip().endswith('\\'):
                    skip = False
                continue
            skip = False
        out.append(l)
    text = '\n'.join(out)
    p.write_text(text)
    texts.append(text)
print(f'dropped {dropped} stale line(s)')
text = '\n'.join(texts)
for c in sorted(ROOT.glob('src/overlays/o*/overlay_*.c')):
    rel = c.relative_to(ROOT).with_suffix('')
    if f'{rel.relative_to("src")}.c.o' not in text: print('WARNING: no rule/list entry for', rel)
