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
    """Returns (all_funcs, abs_placeholder_names).

    all_funcs: {name: size} for every STT_FUNC symbol that has real code, i.e.
    lives in an actual section with nonzero size.

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

    return all_funcs, abs_placeholders


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


def main(args):
    build_dir = os.path.join(ROOT_DIR, "build")
    elf_path = os.path.join(build_dir, f"mickey.{args.version}.elf")
    asm_dir = os.path.join(ROOT_DIR, "asm")
    symbol_addrs_path = os.path.join(ROOT_DIR, f"symbol_addrs.{args.version}.txt")
    tools_dir = os.path.join(ROOT_DIR, "tools")
    objdump = find_objdump(tools_dir)

    all_funcs, abs_placeholders = get_elf_functions(elf_path, objdump)
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
    args = parser.parse_args()

    main(args)
