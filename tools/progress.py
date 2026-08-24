#!/usr/bin/env python3
"""Progress metric for the decompilation.

Prints the resident function/C-match counts, whole-program resolved text bytes,
and symbols named, with the derivation shown alongside each
one. This is deliberate: Phase 1 found more than one summary count that had
silently drifted from the tree it was supposed to describe (see
docs/workbench-improvement-log.md), and a progress metric that hides its own
method is exactly the kind of number that drifts. Nothing here is hardcoded;
every count is read from build/, asm/, symbol_addrs.*.txt, and the canonical
generated overlay atlas as they stand.

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
import hashlib
import json
import os
import re
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
ROOT_DIR = os.path.normpath(os.path.join(SCRIPT_DIR, ".."))

LABEL_RE = re.compile(r"^\s*(?:glabel|alabel)\s+([A-Za-z_][A-Za-z0-9_]*)")
SYMBOL_ADDR_RE = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*0x[0-9A-Fa-f]+\s*;")
PRAGMA_RE = re.compile(r"^\s*#pragma\s+GLOBAL_ASM\s*\(", re.MULTILINE)


def find_objdump(tools_dir):
    cross = os.path.join(tools_dir, "binutils", "mips64-elf-objdump")
    if os.path.isfile(cross) and os.access(cross, os.X_OK):
        return cross
    return "objdump"  # fall back to the host's; works for MIPS ELF on macOS/Linux


def get_elf_functions(elf_path, objdump):
    """Returns (all_funcs, func_addrs, abs_placeholder_names).

    all_funcs: {name: size} for every resident STT_FUNC symbol that has real
    code, i.e. lives in an actual non-overlay section with nonzero size.

    Overlay functions are deliberately excluded here. All overlays share a
    synthetic link VMA and their text bytes already enter the whole-program
    denominator through config/overlays.us.json. Counting their ELF symbols
    here would both double-count those bytes and mistake generated overlay
    labels for adopted resident names.

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
        if section.startswith(".overlay_"):
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


def get_verified_asm_subsegments(path):
    """Named subsegments established as original hand-written assembly.

    This is intentionally an explicit evidence ledger rather than an inference
    from a file extension. Generated `asm/` contains both compiler output and
    original assembly, and treating every `.s` file as resolved would make the
    metric meaningless.
    """
    names = set()
    if not os.path.isfile(path):
        return names
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            line = line.split("#", 1)[0].strip()
            if line:
                names.add(line)
    return names


def get_overlay_text_bytes(rom_path, atlas_path=None):
    """Read the overlay denominator and matched-C ownership from the atlas.

    The atlas is itself regenerated from the shipped headers by
    tools/overlay_atlas.py.  Its source SHA1 is checked here so progress cannot
    silently combine a manifest from one ROM with an ELF from another.
    """
    if atlas_path is None:
        atlas_path = os.path.join(ROOT_DIR, "config", "overlays.us.json")
    with open(atlas_path, encoding="utf-8") as fh:
        atlas = json.load(fh)
    digest = hashlib.sha1()
    with open(rom_path, "rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            digest.update(chunk)
    got = digest.hexdigest()
    expected = atlas.get("source", {}).get("sha1")
    if got != expected:
        raise RuntimeError(
            "overlay atlas source SHA1 is stale: ROM %s, atlas %s"
            % (got, expected)
        )
    return (
        int(atlas["totals"]["text_bytes"]),
        int(atlas["totals"].get("matched_overlay_c_bytes", 0)),
        int(atlas["totals"].get("nonmatching_overlay_c_bytes", 0)),
    )


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


def subsegment_of(addr, index):
    """Named splat subsegment containing `addr`, or the empty string."""
    i = bisect.bisect_right(index, (addr, "￿")) - 1
    return index[i][1] if i >= 0 else ""


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


def source_tus(src_dir):
    """[(path-under-src, note, fully_c)] for every C TU in the tree."""
    out = []
    for root, _dirs, files in os.walk(src_dir):
        for f in sorted(files):
            if f.endswith(".c"):
                path = os.path.join(root, f)
                rel = os.path.relpath(path, src_dir)
                with open(path, encoding="utf-8", errors="replace") as fh:
                    fully_c = PRAGMA_RE.search(fh.read()) is None
                out.append((rel, TU_NOTES.get(rel, ""), fully_c))
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
        f"[![bytes]({badge('code bytes resolved', st['byte_msg'], 'blue')})]"
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
        f"{st['byte_pct']:5.2f}%   matched C in the resident segment"
    )
    L.append(
        f"verified asm {st['verified_asm_bytes']:>6} / {st['total_bytes']:<6}  "
        f"{st['verified_asm_pct']:5.2f}%   original hand-written assembly "
        f"({st['n_verified_asm']} functions)"
    )
    L.append(
        f"overlay C   {st['overlay_matched_bytes']:>6} / {st['overlay_text_bytes']:<6}  "
        f"{st['overlay_byte_pct']:5.2f}%   matched C keyed by overlay and offset"
    )
    L.append(
        f"whole resolved {st['resolved_bytes']:>6} / {st['whole_text_bytes']:<6}  "
        f"{st['resolved_pct']:5.2f}%   resident C + verified asm + overlay C"
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
        "DKR-style report (docs/acceleration-survey.md sec.13.1: NON_MATCHING "
        "and NON_EQUIVALENT count as unmatched, exactly like extracted "
        "assembly):"
    )
    L.append("")
    L.append("```")
    w = st["dkr_whole_bytes"]
    for label, key in (
        ("decompiled", "dkr_decompiled_bytes"),
        ("handwritten asm", "dkr_handwritten_asm_bytes"),
        ("GLOBAL_ASM remaining", "dkr_global_asm_bytes"),
        ("NON_MATCHING", "dkr_non_matching_bytes"),
        ("NON_EQUIVALENT", "dkr_non_equivalent_bytes"),
    ):
        value = st[key]
        pct = value / w * 100 if w else 0.0
        L.append(f"{label:<22} {value:>7} / {w:<7} ({pct:5.2f}%)")
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
        "`░` neither. Naming runs ahead of matching: a function is "
        "decompiled against an already-identified translation unit."
    )
    L.append("")
    tus = st["tus"]
    by_dir = {}
    for rel, note, fully_c in tus:
        by_dir.setdefault(os.path.dirname(rel) or ".", []).append((rel, note, fully_c))
    parts = []
    for d in sorted(by_dir):
        items = by_dir[d]
        noted = [f"`{os.path.basename(r)}` ({n})" for r, n, _ in items if n]
        if noted:
            parts.append(
                f"{len(items)} under `src/{d}/`, including "
                + " and ".join(noted)
            )
        else:
            parts.append(f"{len(items)} under `src/{d}/`")
    n_fully_c = sum(fully_c for _, _, fully_c in tus)
    L.append(
        f"**Source organization**: {n_fully_c} fully-C translation units and "
        f"{len(tus) - n_fully_c} C scaffolds that still include assembly. "
        + "; ".join(parts) + "."
    )
    L.append("")
    L.append(
        "Generated by `gmake scoreboard` from the built ELF, the splat "
        f"config, the `asm/` tree and `symbol_addrs.{st['version']}.txt`; "
        "`gmake check-scoreboard` fails if it has drifted. "
        "[`docs/modules.md`](docs/modules.md) records what each run of code "
        "was identified as and on what evidence; "
        "[`docs/references.md`](docs/references.md) records the reference "
        "builds it was measured against."
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


def check_partial(args):
    """CI-safe subset of --check-readme.

    `--check-readme` needs a linked ELF (get_elf_functions() below), and
    building one needs a split from the baserom, which clean-room policy
    forbids committing -- so CI has no baserom and cannot produce an ELF.
    See docs/CONTRIBUTING.md's "scoreboard in CI" section for the full
    reasoning. This is the strongest check that remains possible without one:

    1. The two figures in the block that are NOT derived from the ELF --
       the adopted-symbol count (symbol_addrs.*.txt) and the matched-TU list
       (src/*.c) -- are recomputed from the tree and compared against the
       committed README, using the real render_markdown() so the comparison
       cannot drift from what `gmake scoreboard` actually generates. Every
       ELF-derived field is passed in as a zero placeholder and this check
       never reads it back.
    2. The block's own internal arithmetic is re-derived from the numbers it
       prints, independent of whether the tree agrees with them: each
       ratio's percentage against its own numerator/denominator, the three
       per-area rows summing to the total row, and the "functions" and
       "named" lines agreeing on the same total function count.

    What this explicitly does NOT verify: whether the functions-matched,
    bytes-matched, or per-area breakdown are still correct against the built
    ELF -- i.e. whether someone forgot to run `gmake scoreboard` after
    matching a function. Only `gmake check-scoreboard`, run locally with a
    baserom, catches that; there is no way to catch it in CI.
    """
    readme_path = os.path.join(ROOT_DIR, "README.md")
    with open(readme_path, encoding="utf-8") as fh:
        current = fh.read()

    if BEGIN not in current or END not in current:
        print(f"Error: README.md is missing the {BEGIN} / {END} markers", file=sys.stderr)
        return 1

    problems = []

    # --- 1. Non-ELF-derived figures, checked by reusing the real generator -
    symbol_addrs_path = os.path.join(ROOT_DIR, f"symbol_addrs.{args.version}.txt")
    n_named = count_named_symbols(symbol_addrs_path)
    tus = source_tus(os.path.join(ROOT_DIR, "src"))

    zero_areas = {
        label: dict(funcs=0, bytes=0, matched=0, matched_bytes=0, named=0, unnamed=0)
        for label, _ in AREAS
    }
    zero_areas["**total**"] = dict(
        funcs=0, bytes=0, matched=0, matched_bytes=0, named=0, unnamed=0
    )
    placeholder_st = dict(
        version=args.version,
        n_matched=0,
        n_total=0,
        func_pct=0.0,
        matched_bytes=0,
        total_bytes=0,
        byte_pct=0.0,
        verified_asm_bytes=0,
        verified_asm_pct=0.0,
        n_verified_asm=0,
        overlay_matched_bytes=0,
        overlay_text_bytes=0,
        overlay_byte_pct=0.0,
        resolved_bytes=0,
        whole_text_bytes=0,
        resolved_pct=0.0,
        n_named=n_named,
        n_named_funcs=0,
        named_pct=0.0,
        areas=zero_areas,
        tus=tus,
        func_msg="0 of 0 (0.00%)",
        byte_msg="0 of 0 (0.00%)",
        name_msg=f"{n_named} adopted",
        dkr_decompiled_bytes=0,
        dkr_handwritten_asm_bytes=0,
        dkr_global_asm_bytes=0,
        dkr_non_matching_bytes=0,
        dkr_non_equivalent_bytes=0,
        dkr_whole_bytes=0,
    )
    rendered = render_markdown(placeholder_st)

    def find_line(text, prefix):
        for line in text.splitlines():
            if line.startswith(prefix):
                return line
        return None

    exp_symbols = find_line(rendered, "symbols")
    cur_symbols = find_line(current, "symbols")
    if exp_symbols is None:
        problems.append("could not find a 'symbols' line in the generated block")
    elif exp_symbols != cur_symbols:
        problems.append(
            "'symbols' line is stale against symbol_addrs."
            f"{args.version}.txt:\n"
            f"    committed: {cur_symbols!r}\n"
            f"    tree says: {exp_symbols!r}"
        )

    exp_matched = find_line(rendered, "**Source organization**")
    cur_matched = find_line(current, "**Source organization**")
    if exp_matched is None:
        problems.append("could not find a 'Source organization' line in the generated block")
    elif exp_matched != cur_matched:
        problems.append(
            "'Source organization' line is stale against the src/ tree "
            f"({len(tus)} .c files found):\n"
            f"    committed: {cur_matched!r}\n"
            f"    tree says: {exp_matched!r}"
        )

    symbols_badge_url = badge("symbols named", f"{n_named} adopted", "blue")
    if symbols_badge_url not in current:
        problems.append(
            f"symbols badge is stale: expected the URL for '{n_named} adopted' "
            "somewhere in README.md"
        )

    # --- 2. Internal arithmetic consistency of the committed block itself, -
    # regardless of whether its inputs are current.
    start = current.find(BEGIN) + len(BEGIN)
    end = current.find(END)
    block = current[start:end]

    ratios = {}  # label -> (num, den, pct)

    def check_ratio(label, pattern):
        m = re.search(pattern, block, re.MULTILINE)
        if not m:
            problems.append(f"could not find the '{label}' line to check its arithmetic")
            return
        num, den, pct = int(m.group(1)), int(m.group(2)), float(m.group(3))
        ratios[label] = (num, den, pct)
        if den == 0:
            return
        expected_pct = num / den * 100
        if abs(expected_pct - pct) > 0.01:
            problems.append(
                f"'{label}' line's percentage ({pct}%) does not match its own "
                f"numbers ({num}/{den} = {expected_pct:.2f}%)"
            )

    check_ratio("functions", r"^functions\s+(\d+)\s*/\s*(\d+)\s+(\d+\.\d+)%")
    check_ratio(".text bytes", r"^\.text bytes\s+(\d+)\s*/\s*(\d+)\s+(\d+\.\d+)%")
    check_ratio("verified asm", r"^verified asm\s+(\d+)\s*/\s*(\d+)\s+(\d+\.\d+)%")
    check_ratio("overlay C", r"^overlay C\s+(\d+)\s*/\s*(\d+)\s+(\d+\.\d+)%")
    check_ratio("whole resolved", r"^whole resolved\s+(\d+)\s*/\s*(\d+)\s+(\d+\.\d+)%")
    check_ratio("named", r"^named\s+(\d+)\s*/\s*(\d+)\s+(\d+\.\d+)%")

    if "functions" in ratios and "named" in ratios:
        func_total = ratios["functions"][1]
        named_total = ratios["named"][1]
        if func_total != named_total:
            problems.append(
                "'functions' and 'named' lines disagree on the total function "
                f"count ({func_total} vs {named_total})"
            )

    row_re = re.compile(
        r"^\| (.+?) \| (\d+) \| (\d+) \| (\d+) \| (\d+) \| `[^`]*` (\d+\.\d+)% \|",
        re.MULTILINE,
    )
    rows = row_re.findall(block)
    if len(rows) != len(AREAS) + 1:
        problems.append(
            f"expected {len(AREAS) + 1} area-table rows ({len(AREAS)} areas + "
            f"total), found {len(rows)}"
        )
    else:
        for label, funcs, matched, named, unnamed, pct in rows:
            funcs, matched, named, unnamed, pct = (
                int(funcs), int(matched), int(named), int(unnamed), float(pct)
            )
            if funcs != matched + named + unnamed:
                problems.append(
                    f"area '{label}': funcs ({funcs}) != matched + named + "
                    f"unnamed ({matched}+{named}+{unnamed}={matched + named + unnamed})"
                )
            if funcs:
                expected_pct = (matched + named) / funcs * 100
                if abs(expected_pct - pct) > 0.05:
                    problems.append(
                        f"area '{label}': identified% ({pct}%) does not "
                        f"match its own numbers ({expected_pct:.1f}%)"
                    )
        total_row = rows[-1]
        summed = [sum(int(r[i]) for r in rows[:-1]) for i in range(1, 5)]
        total_vals = [int(v) for v in total_row[1:5]]
        if summed != total_vals:
            problems.append(
                f"area rows do not sum to the '**total**' row: areas sum to "
                f"{summed}, total row says {total_vals} "
                "(funcs, matched, named, unnamed)"
            )
        if "functions" in ratios and total_vals[0] != ratios["functions"][1]:
            problems.append(
                "area table's total function count "
                f"({total_vals[0]}) disagrees with the 'functions' line's "
                f"total ({ratios['functions'][1]})"
            )

    print("scoreboard --check-partial: CI-safe subset (no linked ELF used)")
    print(
        "  checked: adopted-symbol count and badge, matched-TU list, "
        "internal arithmetic\n"
        "           (ratios' own percentages, area-table row sums, "
        "cross-line total agreement)"
    )
    print(
        "  NOT checked (needs a linked ELF, which CI cannot build without a "
        "baserom):\n"
        "           functions/bytes matched, per-area breakdown, whether "
        "those figures\n"
        "           are still current -- run `gmake check-scoreboard` "
        "locally for that"
    )
    if problems:
        print("\nscoreboard --check-partial: FAIL", file=sys.stderr)
        for p in problems:
            print(f"  - {p}", file=sys.stderr)
        return 1
    print("scoreboard --check-partial: OK")
    return 0


def main(args):
    if args.check_partial:
        return check_partial(args)

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

    yaml_path = os.path.join(ROOT_DIR, f"mickey.{args.version}.yaml")
    index = subsegment_index(yaml_path)
    verified_asm_path = os.path.join(ROOT_DIR, f"verified_asm.{args.version}.txt")
    verified_asm_subsegments = get_verified_asm_subsegments(verified_asm_path)
    verified_asm_funcs = {
        name for name, addr in func_addrs.items()
        if name not in matched_funcs
        and subsegment_of(addr, index) in verified_asm_subsegments
    }
    verified_asm_bytes = sum(all_funcs[name] for name in verified_asm_funcs)
    verified_asm_pct = verified_asm_bytes / total_bytes * 100 if total_bytes else 0.0
    overlay_text_bytes, overlay_matched_bytes, overlay_nonmatching_bytes = (
        get_overlay_text_bytes(
            os.path.join(ROOT_DIR, "baseroms", f"mickey.{args.version}.z64")
        )
    )
    overlay_byte_pct = (
        overlay_matched_bytes / overlay_text_bytes * 100 if overlay_text_bytes else 0.0
    )
    whole_text_bytes = total_bytes + overlay_text_bytes
    resolved_bytes = matched_bytes + verified_asm_bytes + overlay_matched_bytes
    resolved_pct = resolved_bytes / whole_text_bytes * 100 if whole_text_bytes else 0.0

    # DKR's five-line report (docs/acceleration-survey.md sec.13.1):
    # tools/python/score.py there rewrites any `#ifdef NON_MATCHING ... #else
    # GLOBAL_ASM ... #endif` block back to a bare GLOBAL_ASM before counting,
    # so a NON_MATCHING function counts as unmatched, same as extracted
    # assembly it has not been given a C body for at all. `decompiled` below
    # is therefore exactly resident matched_bytes (unaffected -- no resident
    # object has been converted yet) plus the overlay atlas's
    # matched_overlay_c_bytes, which tools/overlay_atlas.py already excludes
    # any range whose owning .c carries "#ifdef NON_MATCHING" from (see its
    # mechanically-derived `nonmatching` field). `global_asm_remaining` is
    # whatever text neither matched nor is explicitly NON_MATCHING: resident
    # asm still glabel'd under asm/, plus every overlay range that is either
    # raw unreviewed "asm" ownership or GLOBAL_ASM'd without a NON_MATCHING
    # C body. NON_EQUIVALENT has no functions yet (no NON_EQUIVALENT-guarded
    # branch exists in the tree); the line is still reported so the report
    # shape matches DKR's even before one is needed.
    dkr_decompiled_bytes = matched_bytes + overlay_matched_bytes
    dkr_handwritten_asm_bytes = verified_asm_bytes
    dkr_non_matching_bytes = overlay_nonmatching_bytes  # resident: none yet
    dkr_global_asm_bytes = (
        (total_bytes - matched_bytes - verified_asm_bytes)
        + (overlay_text_bytes - overlay_matched_bytes - overlay_nonmatching_bytes)
    )
    dkr_non_equivalent_bytes = 0
    dkr_whole_bytes = whole_text_bytes
    if dkr_whole_bytes and (
        dkr_decompiled_bytes
        + dkr_handwritten_asm_bytes
        + dkr_non_matching_bytes
        + dkr_global_asm_bytes
        + dkr_non_equivalent_bytes
        != dkr_whole_bytes
    ):
        raise RuntimeError("DKR-line bytes do not sum to the whole-program total")

    n_total = len(total_funcs)
    n_matched = len(matched_funcs)
    func_pct = (n_matched / n_total * 100) if n_total else 0.0
    byte_pct = (matched_bytes / total_bytes * 100) if total_bytes else 0.0

    n_named = count_named_symbols(symbol_addrs_path)

    # Per-area breakdown. Each function is attributed to the splat subsegment
    # its address falls inside, and then to one of three tiers: matched to C,
    # carrying an adopted name but still assembled from .s, or neither.
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
            verified_asm_bytes=verified_asm_bytes,
            verified_asm_pct=verified_asm_pct,
            n_verified_asm=len(verified_asm_funcs),
            overlay_matched_bytes=overlay_matched_bytes,
            overlay_text_bytes=overlay_text_bytes,
            overlay_byte_pct=overlay_byte_pct,
            resolved_bytes=resolved_bytes,
            whole_text_bytes=whole_text_bytes,
            resolved_pct=resolved_pct,
            n_named=n_named,
            n_named_funcs=n_named_funcs,
            named_pct=named_pct,
            areas=areas,
            tus=source_tus(os.path.join(ROOT_DIR, "src")),
            func_msg=f"{n_matched} of {n_total} ({func_pct:.2f}%)",
            byte_msg=f"{resolved_bytes} of {whole_text_bytes} ({resolved_pct:.2f}%)",
            name_msg=f"{n_named} adopted",
            dkr_decompiled_bytes=dkr_decompiled_bytes,
            dkr_handwritten_asm_bytes=dkr_handwritten_asm_bytes,
            dkr_global_asm_bytes=dkr_global_asm_bytes,
            dkr_non_matching_bytes=dkr_non_matching_bytes,
            dkr_non_equivalent_bytes=dkr_non_equivalent_bytes,
            dkr_whole_bytes=dkr_whole_bytes,
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
        print(
            f"#   verified original assembly = functions in named subsegments "
            f"listed by verified_asm.{args.version}.txt"
        )
        print(
            f"#   overlay matched C = explicit `(overlay, offset)` ownership ranges "
            f"in config/overlays.{args.version}.json"
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
    print(
        f"asm:       {verified_asm_bytes} verified / {total_bytes} resident "
        f"({verified_asm_pct:.2f}%; {len(verified_asm_funcs)} functions)"
    )
    print(
        f"overlays:  {overlay_matched_bytes} matched / {overlay_text_bytes} text "
        f"({overlay_byte_pct:.2f}%)"
    )
    print(
        f"resolved:  {resolved_bytes} / {whole_text_bytes} whole-program text "
        f"({resolved_pct:.2f}%; resident C + verified asm + overlay C)"
    )
    print(f"symbols:   {n_named} named")

    print()
    print("DKR-style report (docs/acceleration-survey.md sec.13.1):")
    for label, value in (
        ("decompiled", dkr_decompiled_bytes),
        ("handwritten asm", dkr_handwritten_asm_bytes),
        ("GLOBAL_ASM remaining", dkr_global_asm_bytes),
        ("NON_MATCHING", dkr_non_matching_bytes),
        ("NON_EQUIVALENT", dkr_non_equivalent_bytes),
    ):
        pct = value / dkr_whole_bytes * 100 if dkr_whole_bytes else 0.0
        print(f"  {label:<22} {value:>7} / {dkr_whole_bytes:<7} ({pct:5.2f}%)")

    if args.csv:
        print()
        print("metric,matched,total,pct")
        print(f"functions,{n_matched},{n_total},{func_pct:.4f}")
        print(f"bytes,{matched_bytes},{total_bytes},{byte_pct:.4f}")
        print(f"verified_asm_bytes,{verified_asm_bytes},{total_bytes},{verified_asm_pct:.4f}")
        print(f"overlay_c_bytes,{overlay_matched_bytes},{overlay_text_bytes},{overlay_byte_pct:.4f}")
        print(f"whole_resolved_bytes,{resolved_bytes},{whole_text_bytes},{resolved_pct:.4f}")
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
    parser.add_argument(
        "--check-partial",
        action="store_true",
        help="CI-safe subset of --check-readme: verifies the block's "
        "non-ELF-derived figures and its own internal arithmetic, without "
        "requiring a linked ELF. See check_partial()'s docstring for "
        "exactly what this does and does not catch.",
    )
    args = parser.parse_args()

    sys.exit(main(args))
