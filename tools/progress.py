#!/usr/bin/env python3
"""Progress metric for the decompilation.

Prints three derived numbers -- functions matched, static-segment .text
bytes matched, and symbols named -- with the derivation shown alongside each
one. This is deliberate: Phase 1 found more than one summary count that had
silently drifted from the tree it was supposed to describe (see
docs/workbench-improvement-log.md), and a progress metric that hides its own
method is exactly the kind of number that drifts. Nothing here is hardcoded;
every count is read from build/, asm/ and symbol_addrs.*.txt as they stand.

Method, in one paragraph: the built ELF's symbol table is the ground truth
for "what splat thinks is a function, and how big". A function counts as
*matched* when its name does not appear as a `glabel`/`alabel` anywhere under
asm/ -- i.e. no .s file anywhere in the tree still defines it, so whatever
produced its bytes in the link was C. That is a tree-wide name search rather
than "does a same-named .s file exist", because several asm/ subsegments
(asm/main/*.s, asm/libultra/*.s, the un-carved asm/<ADDR>.s files) are still
whole-file dumps holding many functions under one filename that does not
match any one of them -- matching by filename alone silently mis-scores
every function in those files. See the "why not X" note below the report for
the two failure modes this ruled out.
"""

import argparse
import bisect
import difflib
import os
import re
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
ROOT_DIR = os.path.normpath(os.path.join(SCRIPT_DIR, ".."))

LABEL_RE = re.compile(r"^\s*(?:glabel|alabel)\s+([A-Za-z_][A-Za-z0-9_]*)")
SYMBOL_ADDR_RE = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*0x[0-9A-Fa-f]+\s*;")


def find_objdump(tools_dir):
    cross = os.path.join(tools_dir, "binutils", "mips64-elf-objdump")
    if os.path.isfile(cross) and os.access(cross, os.X_OK):
        return cross
    return "objdump"  # fall back to the host's; works for MIPS ELF on macOS/Linux


def get_elf_functions(elf_path, objdump):
    """Returns (all_funcs, func_addrs, abs_placeholder_names).

    all_funcs: {name: size} for every STT_FUNC symbol that has real code, i.e.
    lives in an actual section with nonzero size.

    func_addrs: {name: vram} for those same symbols. Used only to attribute a
    function to the splat subsegment it falls inside, for the per-area
    breakdown -- see subsegment_index().

    abs_placeholder_names: STT_FUNC symbols the linker resolved to a bare
    *ABS* address with size 0. These come from undefined_funcs_auto.*.txt /
    undefined_syms_auto.*.txt -- splat's auto-generated stand-ins for names
    referenced from one not-yet-organized asm file but not themselves a
    distinct function boundary (verified case: func_80059278 and its
    neighbours are `alabel`s for shared branch targets *inside*
    func_800591B0 in asm/59DB0.s, a hand-written, heavily-unrolled function --
    not separate functions). They are excluded from the denominator for that
    reason, and reported separately so the exclusion is visible rather than
    silent.
    """
    try:
        result = subprocess.run(
            [objdump, "-x", elf_path], stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
    except FileNotFoundError:
        print(f"Error: objdump not found ({objdump})", file=sys.stderr)
        sys.exit(1)
    if result.returncode != 0 or not result.stdout:
        print(f"Error: could not run objdump on {elf_path}", file=sys.stderr)
        print("       make sure the project is built (gmake).", file=sys.stderr)
        sys.exit(1)

    all_funcs = {}
    func_addrs = {}
    abs_placeholders = set()
    for line in result.stdout.decode().splitlines():
        # objdump -x symbol-table lines look like:
        #   "<addr> <flags> <section>\t<size> <name>"
        # where <flags> is a fixed-width field (type "F" = function) that can
        # itself contain embedded literal spaces for unset flag slots, so a
        # naive left-to-right split() cannot assume a fixed token count.
        # Anchoring on the three unambiguous trailing fields (name has no
        # spaces, size is hex, section starts with '.' or '*') and checking
        # for the "F" type flag as a substring of what's left is robust to
        # that either way.
        if " F " not in line:
            continue
        tokens = line.split()
        if len(tokens) < 4:
            continue
        name, size_hex, section = tokens[-1], tokens[-2], tokens[-3]
        try:
            size = int(size_hex, 16)
        except ValueError:
            continue
        if section == "*ABS*" and size == 0:
            abs_placeholders.add(name)
            continue
        all_funcs[name] = size
        try:
            func_addrs[name] = int(tokens[0], 16)
        except ValueError:
            pass

    return all_funcs, func_addrs, abs_placeholders


def get_asm_labelled_names(asm_dir):
    """Every glabel/alabel identifier appearing anywhere under asm/.

    Deliberately a name search across the whole tree, not a per-file
    filename match: asm/main/*.s and asm/libultra/*.s are still whole-file
    dumps (one .s per original TU, e.g. asm/main/gzip_asm.s holds five
    `gzip_inflate_*` functions under a filename that matches none of them),
    and the 107 not-yet-organized asm/<ADDR>.s files are similarly
    multi-function. A name that still has a glabel/alabel anywhere in this
    tree has not been replaced by C, however its .s file happens to be
    named.
    """
    names = set()
    for root, _dirs, files in os.walk(asm_dir):
        for f in files:
            if not f.endswith(".s"):
                continue
            path = os.path.join(root, f)
            with open(path, "r", errors="replace") as fh:
                for line in fh:
                    m = LABEL_RE.match(line)
                    if m:
                        names.add(m.group(1))
    return names


def count_named_symbols(symbol_addrs_path):
    """Lines of the form `Name = 0xADDR;` in symbol_addrs.*.txt -- i.e. names
    the project has actually adopted, per docs/modules.md's tier system.
    Comments and blank lines don't match and aren't counted."""
    if not os.path.isfile(symbol_addrs_path):
        return 0
    count = 0
    with open(symbol_addrs_path, "r", errors="replace") as fh:
        for line in fh:
            if SYMBOL_ADDR_RE.match(line):
                count += 1
    return count


# Auto-generated splat names: a symbol matching one of these carries no
# information beyond its address, so it is "unnamed" for scoreboard purposes.
# Anything else in the ELF's symbol table got there because symbol_addrs.*.txt
# adopted a name for it, per docs/modules.md's tier system.
AUTO_NAME_RE = re.compile(r"^(?:func|D|L|jtbl|RO)_[0-9A-Fa-f]{8}$")

# Areas, in the order they appear in the scoreboard table. Each is a predicate
# over the *subsegment name* the function's address falls inside -- which is
# where the project records what a run of code has been identified as.
AREAS = [
    ("libultra corridor", lambda n: n.startswith("libultra/")),
    ("game code, TU identified", lambda n: bool(n)),
    ("game code, not yet split", lambda n: True),
]

# Short descriptions for matched translation units, keyed by their path under
# src/. Kept here rather than typed into the README so the "what's matched"
# line is generated like every other part of the block; a TU with no entry is
# still listed, just without a gloss.
TU_NOTES = {
    "main/runlink.c": "the runtime overlay linker core",
    "main/matrix.c": "matrix/vector maths",
}


def subsegment_index(yaml_path):
    """Sorted [(vram_start, subsegment_name)] for every subsegment in the splat
    config, so a function address can be attributed to the run of code the
    project has (or hasn't) identified.

    The ROM->VRAM delta is derived per segment from its own `start`/`vram`
    pair rather than hardcoded, so a segment that is remapped cannot silently
    mis-attribute every function inside it.
    """
    try:
        import yaml
    except ImportError:
        return []
    if not os.path.isfile(yaml_path):
        return []
    with open(yaml_path, encoding="utf-8") as fh:
        cfg = yaml.safe_load(fh)
    index = []
    for seg in cfg.get("segments", []):
        if not isinstance(seg, dict):
            continue
        start, vram = seg.get("start"), seg.get("vram")
        if not isinstance(start, int) or not isinstance(vram, int):
            continue
        delta = vram - start
        for sub in seg.get("subsegments") or []:
            if not isinstance(sub, list) or not sub or not isinstance(sub[0], int):
                continue
            name = sub[2] if len(sub) > 2 and isinstance(sub[2], str) else ""
            index.append((sub[0] + delta, name))
    index.sort()
    return index


def area_of(addr, index):
    """The scoreboard area a function address belongs to."""
    i = bisect.bisect_right(index, (addr, "￿")) - 1
    name = index[i][1] if i >= 0 else ""
    for label, pred in AREAS:
        if pred(name):
            return label
    return AREAS[-1][0]


def bar(matched, named, total, width=20):
    """A three-state text bar: matched to C, named but still asm, unnamed.

    This project's shape is a large naming lead over a small matching lead, and
    a single-value bar would hide exactly that. Any nonzero tier is rounded up
    to at least one cell so it is visible rather than rounded out of existence.
    """
    if total <= 0:
        return "░" * width
    m = min(round(matched / total * width), width)
    if matched > 0:
        m = max(m, 1)
    mn = min(round((matched + named) / total * width), width)
    if matched + named > 0:
        mn = max(mn, m, 1)
    return "█" * m + "▓" * (mn - m) + "░" * (width - mn)


def badge(label, message, color):
    """A shields.io static badge URL carrying generated numbers.

    Every badge in the block is rebuilt from the same counts as the table, so a
    badge cannot claim a different number than the text beside it.
    """
    def esc(s):
        return (
            s.replace("_", "__").replace("-", "--").replace(" ", "_").replace("%", "%25")
        )

    return f"https://img.shields.io/badge/{esc(label)}-{esc(message)}-{color}"


def matched_tus(src_dir):
    """[(path-under-src, note)] for every C translation unit in the tree."""
    out = []
    for root, _dirs, files in os.walk(src_dir):
        for f in sorted(files):
            if f.endswith(".c"):
                rel = os.path.relpath(os.path.join(root, f), src_dir)
                out.append((rel, TU_NOTES.get(rel, "")))
    return sorted(out)


def render_markdown(st):
    """The scoreboard block: everything between the README's markers.

    Deliberately decimal-only -- counts and percentages, no hex words and no
    address columns. The clean-room content detectors treat dense machine-word
    and hex-token tables as a ROM leak signature, and a scoreboard has no need
    of either, so the block is written to stay far from that line by
    construction rather than by exemption.
    """
    L = []
    L.append("### Progress")
    L.append("")
    L.append(
        f"[![functions]({badge('functions matched', st['func_msg'], 'blue')})]"
        f"(#progress) "
        f"[![bytes]({badge('.text bytes matched', st['byte_msg'], 'blue')})]"
        f"(#progress) "
        f"[![names]({badge('symbols named', st['name_msg'], 'blue')})](#progress)"
    )
    L.append("")
    L.append("```")
    L.append(
        f"functions   {st['n_matched']:>6} / {st['n_total']:<6}  "
        f"{st['func_pct']:5.2f}%   matched to C, byte-identical"
    )
    L.append(
        f".text bytes {st['matched_bytes']:>6} / {st['total_bytes']:<6}  "
        f"{st['byte_pct']:5.2f}%   of the static resident segment"
    )
    L.append(
        f"named       {st['n_named_funcs']:>6} / {st['n_total']:<6}  "
        f"{st['named_pct']:5.2f}%   functions carrying an adopted name"
    )
    L.append(
        f"symbols     {st['n_named']:>6}            "
        f"        adopted in symbol_addrs.{st['version']}.txt"
    )
    L.append("```")
    L.append("")
    L.append(
        "| Area | Functions | Matched to C | Named, still asm | Unnamed | "
        "Identified |"
    )
    L.append("| :--- | ---: | ---: | ---: | ---: | :--- |")
    for label in [a[0] for a in AREAS] + ["**total**"]:
        r = st["areas"][label]
        pct = (r["matched"] + r["named"]) / r["funcs"] * 100 if r["funcs"] else 0.0
        L.append(
            f"| {label} | {r['funcs']} | {r['matched']} | {r['named']} | "
            f"{r['unnamed']} | `{bar(r['matched'], r['named'], r['funcs'])}` "
            f"{pct:.1f}% |"
        )
    L.append("")
    L.append(
        "`█` matched to C · `▓` named but still assembly · "
        "`░` neither. The gap between those first two columns is the "
        "point: this project has deliberately run identification ahead of "
        "matching, so that every function decompiled is decompiled against a "
        "known translation unit rather than into one."
    )
    L.append("")
    tus = st["tus"]
    by_dir = {}
    for rel, note in tus:
        by_dir.setdefault(os.path.dirname(rel) or ".", []).append((rel, note))
    parts = []
    for d in sorted(by_dir):
        items = by_dir[d]
        noted = [f"`{os.path.basename(r)}` ({n})" for r, n in items if n]
        if noted:
            parts.append(
                f"{len(items)} under `src/{d}/` — including "
                + " and ".join(noted)
            )
        else:
            parts.append(f"{len(items)} under `src/{d}/`")
    L.append(
        f"**Matched so far** — {len(tus)} C translation units that build "
        f"byte-identically: " + "; ".join(parts) + "."
    )
    L.append("")
    L.append(
        "Every number above is regenerated by `gmake scoreboard` from the "
        "built ELF, the splat config, the `asm/` tree and "
        f"`symbol_addrs.{st['version']}.txt`; `gmake check-scoreboard` fails if "
        "this block has drifted from them. Depth: "
        "[`docs/modules.md`](docs/modules.md) for what each run of code was "
        "identified as and on what evidence, "
        "[`docs/references.md`](docs/references.md) for the reference builds "
        "that identification was measured against."
    )
    return "\n".join(L)


BEGIN = "<!-- SCOREBOARD_BEGIN -->"
END = "<!-- SCOREBOARD_END -->"


def splice_readme(readme_text, block):
    """README text with the scoreboard block swapped in between the markers."""
    start = readme_text.find(BEGIN)
    end = readme_text.find(END)
    if start < 0 or end < 0 or end < start:
        print(
            f"Error: README.md is missing the {BEGIN} / {END} markers",
            file=sys.stderr,
        )
        sys.exit(1)
    return readme_text[: start + len(BEGIN)] + "\n" + block + "\n" + readme_text[end:]


def main(args):
    build_dir = os.path.join(ROOT_DIR, "build")
    elf_path = os.path.join(build_dir, f"mickey.{args.version}.elf")
    asm_dir = os.path.join(ROOT_DIR, "asm")
    symbol_addrs_path = os.path.join(ROOT_DIR, f"symbol_addrs.{args.version}.txt")
    tools_dir = os.path.join(ROOT_DIR, "tools")
    objdump = find_objdump(tools_dir)

    all_funcs, func_addrs, abs_placeholders = get_elf_functions(elf_path, objdump)
    if not all_funcs:
        print(f"Error: no function symbols found in {elf_path}", file=sys.stderr)
        sys.exit(1)

    nonmatching_names = get_asm_labelled_names(asm_dir)

    total_funcs = set(all_funcs.keys())
    matched_funcs = total_funcs - nonmatching_names

    total_bytes = sum(all_funcs.values())
    matched_bytes = sum(all_funcs[n] for n in matched_funcs)

    n_total = len(total_funcs)
    n_matched = len(matched_funcs)
    func_pct = (n_matched / n_total * 100) if n_total else 0.0
    byte_pct = (matched_bytes / total_bytes * 100) if total_bytes else 0.0

    n_named = count_named_symbols(symbol_addrs_path)

    # Per-area breakdown. Each function is attributed to the splat subsegment
    # its address falls inside, and then to one of three tiers: matched to C,
    # carrying an adopted name but still assembled from .s, or neither.
    index = subsegment_index(os.path.join(ROOT_DIR, f"mickey.{args.version}.yaml"))
    areas = {
        label: dict(funcs=0, bytes=0, matched=0, matched_bytes=0, named=0, unnamed=0)
        for label, _ in AREAS
    }
    areas["**total**"] = dict(
        funcs=0, bytes=0, matched=0, matched_bytes=0, named=0, unnamed=0
    )
    for name, size in all_funcs.items():
        label = area_of(func_addrs.get(name, 0), index) if index else AREAS[-1][0]
        for key in (label, "**total**"):
            r = areas[key]
            r["funcs"] += 1
            r["bytes"] += size
            if name in matched_funcs:
                r["matched"] += 1
                r["matched_bytes"] += size
            elif AUTO_NAME_RE.match(name):
                r["unnamed"] += 1
            else:
                r["named"] += 1
    n_named_funcs = areas["**total**"]["matched"] + areas["**total**"]["named"]
    named_pct = (n_named_funcs / n_total * 100) if n_total else 0.0

    if args.markdown or args.update_readme or args.check_readme:
        st = dict(
            version=args.version,
            n_matched=n_matched,
            n_total=n_total,
            func_pct=func_pct,
            matched_bytes=matched_bytes,
            total_bytes=total_bytes,
            byte_pct=byte_pct,
            n_named=n_named,
            n_named_funcs=n_named_funcs,
            named_pct=named_pct,
            areas=areas,
            tus=matched_tus(os.path.join(ROOT_DIR, "src")),
            func_msg=f"{n_matched} of {n_total} ({func_pct:.2f}%)",
            byte_msg=f"{matched_bytes} of {total_bytes} ({byte_pct:.2f}%)",
            name_msg=f"{n_named} adopted",
        )
        block = render_markdown(st)
        readme_path = os.path.join(ROOT_DIR, "README.md")

        if args.markdown:
            print(block)
            return 0

        with open(readme_path, encoding="utf-8") as fh:
            current = fh.read()
        updated = splice_readme(current, block)

        if args.update_readme:
            if updated == current:
                print("scoreboard: README.md already up to date")
            else:
                with open(readme_path, "w", encoding="utf-8") as fh:
                    fh.write(updated)
                print("scoreboard: README.md updated")
            return 0

        # --check-readme
        if updated == current:
            print("scoreboard: OK  README.md matches freshly-generated output")
            return 0
        print(
            "scoreboard: FAIL  README.md's scoreboard block is stale.\n"
            "  The committed block does not match what tools/progress.py "
            "generates from the tree as it stands.\n"
            "  Run `gmake scoreboard` and commit the result.\n",
            file=sys.stderr,
        )
        diff = difflib.unified_diff(
            current.splitlines(keepends=True),
            updated.splitlines(keepends=True),
            fromfile="README.md (committed)",
            tofile="README.md (regenerated)",
            n=1,
        )
        sys.stderr.writelines(diff)
        return 1

    if args.verbose:
        print("# Derivation")
        print(f"#   ELF:            {os.path.relpath(elf_path, ROOT_DIR)}")
        print(f"#   objdump:        {os.path.relpath(objdump, ROOT_DIR) if os.path.isabs(objdump) else objdump}")
        print(f"#   asm/ tree:      {os.path.relpath(asm_dir, ROOT_DIR)}")
        print(f"#   symbol_addrs:   {os.path.relpath(symbol_addrs_path, ROOT_DIR)}")
        print(
            f"#   total functions = STT_FUNC symbols in the linked ELF with "
            f"real (nonzero) size in a real section"
        )
        print(
            f"#     ({len(abs_placeholders)} zero-size *ABS* placeholder symbols "
            f"excluded -- see get_elf_functions() docstring; these are "
            f"undefined_funcs_auto/undefined_syms_auto stand-ins for "
            f"shared-tail branch-target labels inside larger hand-written "
            f"functions, not distinct functions of their own)"
        )
        print(
            f"#   matched = total functions minus every name that still "
            f"appears as a glabel/alabel anywhere under asm/ "
            f"({len(nonmatching_names)} such names found)"
        )
        print(
            f"#   symbols named = `Name = 0xADDR;` lines in "
            f"symbol_addrs.{args.version}.txt (comments/blank lines excluded)"
        )
        print()

    if matched_bytes > total_bytes:
        print("Warning: matched bytes exceed total bytes -- derivation bug!", file=sys.stderr)

    print(
        f"functions: {n_matched} matched / {n_total} total ({func_pct:.2f}%)"
    )
    print(
        f"bytes:     {matched_bytes} matched / {total_bytes} total "
        f"({byte_pct:.2f}% of static-segment .text)"
    )
    print(f"symbols:   {n_named} named")

    if args.csv:
        print()
        print("metric,matched,total,pct")
        print(f"functions,{n_matched},{n_total},{func_pct:.4f}")
        print(f"bytes,{matched_bytes},{total_bytes},{byte_pct:.4f}")
        print(f"symbols_named,{n_named},,")

    return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Reports the project's decompilation progress, derived "
        "from the built ELF and the current asm/ and symbol_addrs.*.txt "
        "state (never hardcoded)."
    )
    parser.add_argument(
        "--version", default="us", help="ROM version to measure (default: us)"
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="also print the derivation method before the numbers",
    )
    parser.add_argument("--csv", action="store_true", help="also print a CSV block")
    parser.add_argument(
        "--markdown",
        action="store_true",
        help="print the README scoreboard block instead of the plain report",
    )
    parser.add_argument(
        "--update-readme",
        action="store_true",
        help="splice the scoreboard block into README.md between its markers",
    )
    parser.add_argument(
        "--check-readme",
        action="store_true",
        help="exit nonzero if README.md's scoreboard block has gone stale",
    )
    args = parser.parse_args()

    sys.exit(main(args))
