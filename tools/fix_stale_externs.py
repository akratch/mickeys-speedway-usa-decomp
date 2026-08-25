#!/usr/bin/env python3
"""Rewrite func_<VRAM> references in src/ and include/ whose address has since
been given an adopted name in symbol_addrs.us.txt.

Lanes name functions independently; when one lane adopts `foo = 0x8002E148`
while another still declares `extern ... func_8002E148(...)`, the link fails
after integration. This makes the rename the symbol file already records.
"""
import pathlib, re, sys
ROOT = pathlib.Path(__file__).resolve().parent.parent
names = {}
for line in (ROOT / "symbol_addrs.us.txt").read_text().splitlines():
    m = re.match(r"\s*([A-Za-z_]\w*)\s*=\s*0x([0-9A-Fa-f]{8});", line)
    if m and not m.group(1).startswith("func_"):
        names[m.group(2).upper()] = m.group(1)
pat = re.compile(r"\bfunc_([0-9A-Fa-f]{8})\b")
changed = 0
# Overlays are excluded: their extern names are lane-owned and their objects
# link through the overlay relocation model, not the resident symbol table.
for path in (list((ROOT / "src/main").rglob("*.c")) + list((ROOT / "src/libultra").rglob("*.c"))
             + list((ROOT / "include/game").rglob("*.h"))):
    text = path.read_text()
    # Never touch GLOBAL_ASM pragma paths: splat names the .s after the
    # symbol file, and a C definition may already carry the adopted name.
    gpat = re.compile(r'(#pragma GLOBAL_ASM\("[^"]*/)func_([0-9A-Fa-f]{8})(\.s"\))')
    def fix_line(l):
        if l.lstrip().startswith("#pragma GLOBAL_ASM"):
            # splat writes the .s under the adopted name, so the path follows it
            return gpat.sub(lambda m: m.group(1) + names.get(m.group(2).upper(), "func_" + m.group(2)) + m.group(3), l)
        if l.lstrip().startswith("#pragma"):
            return l
        return pat.sub(lambda m: names.get(m.group(1).upper(), m.group(0)), l)
    new = "\n".join(fix_line(l) for l in text.split("\n"))
    if new != text:
        path.write_text(new); changed += 1
        for m in set(pat.findall(text)):
            if m.upper() in names: print(f"{path.relative_to(ROOT)}: func_{m} -> {names[m.upper()]}")
print(f"{changed} file(s) rewritten")
