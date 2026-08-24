#!/usr/bin/env python3
"""Audit every object's POSTPROCESS step against docs/acceleration-survey.md
sec.13.2's ruling: no gold-standard N64 decomp edits an instruction word
after compilation to reach a match.

This tool does not parse Makefile text by hand -- line continuations, the
`$(FOO_OBJ)` indirection a handful of overlays use as a target list, and
`ifeq`/`include` conditionals make that brittle and easy to silently
mis-scan.  Instead it asks GNU Make itself to expand every rule
(`gmake -p -q`, the "print database" dump) and reads the target-specific
`POSTPROCESS = ...` assignments straight out of that: what Make would
actually run, expanded.

For each such object it:

  * classifies the command as ``altered`` (touches instruction words:
    normalize_elf_instructions.py, normalize_o63_*.py, resize_elf_function.py,
    extend_elf_function_to_text.py, patch_elf_words.py), ``metadata``
    (trim_elf_section.py, filter/rebind/add_elf_relocations.py, objcopy
    --redefine-sym, externalize/set_elf_symbol_size/set_elf_flags.py,
    order_o*.py, or anything else that isn't in the altered set), or ``none``
    (no POSTPROCESS override -- not expected to appear here, since this
    audit only visits objects Make reports an override for);
  * records which tool(s) the command invokes;
  * joins the object to its ownership range in config/overlays.us.json's
    per-overlay ``text_ownership`` list (offset, size, matched-C-or-not),
    keyed by the object's path under src/ with the .c/.o stripped -- the
    same string overlay_atlas.py stores as ``source``.

Usage:
    tools/postprocess_audit.py               # table to stdout
    tools/postprocess_audit.py --write        # also refresh the committed JSON
    tools/postprocess_audit.py --check        # fail if the JSON is stale
"""

import argparse
import json
import os
import re
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
ROOT_DIR = os.path.normpath(os.path.join(SCRIPT_DIR, ".."))
DEFAULT_OUT = os.path.join(ROOT_DIR, "config", "postprocess-audit.us.json")
ATLAS_PATH = os.path.join(ROOT_DIR, "config", "overlays.us.json")

ALTERED_TOOLS = {
    "normalize_elf_instructions.py",
    "resize_elf_function.py",
    "extend_elf_function_to_text.py",
    "patch_elf_words.py",
}
# normalize_o63_* is a family (normalize_o63_foo.py, ...), matched by prefix.
ALTERED_PREFIXES = ("normalize_o63_",)

TOOL_RE = re.compile(r"([A-Za-z0-9_./-]+\.py)")
OBJCOPY_RE = re.compile(r"\bobjcopy\b", re.IGNORECASE)

TARGET_LINE_RE = re.compile(r"^(\S+): POSTPROCESS = (.*)$")


def run_make_database():
    """Return gmake's expanded rule database as text.

    `-p` prints the database, `-q` ("question mode") stops it from actually
    building anything; the exit code from `-q` is meaningless here (it means
    "up to date or not") and is ignored.
    """
    proc = subprocess.run(
        ["gmake", "-p", "-q"],
        cwd=ROOT_DIR,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        text=True,
        timeout=120,
    )
    return proc.stdout


def classify(command):
    tools = []
    for m in TOOL_RE.finditer(command):
        name = os.path.basename(m.group(1))
        if name not in tools:
            tools.append(name)
    if OBJCOPY_RE.search(command) and "objcopy" not in tools:
        tools.append("objcopy")

    is_altered = any(t in ALTERED_TOOLS for t in tools) or any(
        t.startswith(p) for t in tools for p in ALTERED_PREFIXES
    )
    return ("altered" if is_altered else "metadata"), tools


def load_atlas_index(atlas_path):
    """{('overlays/oNNN/name'): (overlay, offset, size, matched)} from the
    committed overlay atlas, plus the atlas's own totals for cross-checking.
    """
    with open(atlas_path, encoding="utf-8") as fh:
        atlas = json.load(fh)
    index = {}
    for module in atlas["modules"]:
        overlay = module["overlay"]
        for part in module["text_ownership"]:
            index[part["source"]] = {
                "overlay": overlay,
                "offset": int(part["offset"], 16),
                "size": int(part["size"], 16),
                "matched": part["matched"],
            }
    return index, atlas["totals"]


def object_to_src(target):
    """'build/src/overlays/o005/foo.c.o' -> 'src/overlays/o005/foo.c'."""
    if not target.startswith("build/") or not target.endswith(".o"):
        return None
    return target[len("build/") :][: -len(".o")]


def audit(root_dir=ROOT_DIR):
    dump = run_make_database()
    seen = {}
    for line in dump.splitlines():
        m = TARGET_LINE_RE.match(line)
        if not m:
            continue
        target, command = m.group(1), m.group(2)
        if not target.startswith("build/") or not target.endswith(".o"):
            continue
        if command.strip() == "@:":
            continue
        seen[target] = command  # dedupe: -p repeats a rule per prerequisite

    atlas_index, atlas_totals = load_atlas_index(ATLAS_PATH)

    rows = []
    for target in sorted(seen):
        command = seen[target]
        cls, tools = classify(command)
        src = object_to_src(target)
        overlay = None
        object_key = None
        m = re.search(r"overlays/o(\d+)/", src or "")
        if m:
            overlay = int(m.group(1))
        if src is not None and src.startswith("src/") and src.endswith(".c"):
            object_key = src[len("src/") : -len(".c")]
        atlas_hit = atlas_index.get(object_key) if object_key else None
        rows.append(
            {
                "object": target,
                "source": src,
                "overlay": overlay,
                "class": cls,
                "tools": tools,
                "offset": atlas_hit["offset"] if atlas_hit else None,
                "size": atlas_hit["size"] if atlas_hit else None,
                "matched_c": atlas_hit["matched"] if atlas_hit else None,
            }
        )
    return rows, atlas_totals


def summarize(rows):
    total = len(rows)
    by_class = {}
    altered_bytes = 0
    matched_c_bytes_with_postprocess = 0
    for r in rows:
        by_class[r["class"]] = by_class.get(r["class"], 0) + 1
        if r["matched_c"] and r["size"] is not None:
            matched_c_bytes_with_postprocess += r["size"]
            if r["class"] == "altered":
                altered_bytes += r["size"]
    return {
        "objects_with_postprocess": total,
        "by_class": by_class,
        "altered_bytes": altered_bytes,
        "matched_c_bytes_with_postprocess": matched_c_bytes_with_postprocess,
    }


def render_table(rows, summary, atlas_totals):
    lines = []
    lines.append(
        "%-70s %-9s %-6s %8s %8s  %s"
        % ("object", "class", "ovl", "offset", "size", "tools")
    )
    for r in sorted(rows, key=lambda r: (r["overlay"] or -1, r["object"])):
        lines.append(
            "%-70s %-9s %-6s %8s %8s  %s"
            % (
                r["object"],
                r["class"],
                r["overlay"] if r["overlay"] is not None else "-",
                hex(r["offset"]) if r["offset"] is not None else "-",
                hex(r["size"]) if r["size"] is not None else "-",
                ",".join(r["tools"]),
            )
        )
    lines.append("")
    lines.append(
        "objects with POSTPROCESS: %d (altered=%d metadata=%d)"
        % (
            summary["objects_with_postprocess"],
            summary["by_class"].get("altered", 0),
            summary["by_class"].get("metadata", 0),
        )
    )
    lines.append(
        "altered bytes: %d of %d matched-C bytes touching a POSTPROCESS'd "
        "object (atlas matched_overlay_c_bytes total: %d)"
        % (
            summary["altered_bytes"],
            summary["matched_c_bytes_with_postprocess"],
            atlas_totals["matched_overlay_c_bytes"],
        )
    )
    return "\n".join(lines)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--write", action="store_true", help="refresh " + DEFAULT_OUT)
    ap.add_argument(
        "--check", action="store_true", help="fail if " + DEFAULT_OUT + " is stale"
    )
    ap.add_argument("--out", default=DEFAULT_OUT)
    args = ap.parse_args()

    rows, atlas_totals = audit()
    summary = summarize(rows)
    doc = {
        "schema_version": 1,
        "generated_by": "tools/postprocess_audit.py",
        "summary": summary,
        "objects": rows,
    }
    payload = json.dumps(doc, indent=2, sort_keys=True) + "\n"

    if args.check:
        if not os.path.isfile(args.out):
            print("missing " + args.out, file=sys.stderr)
            return 1
        with open(args.out, encoding="utf-8") as fh:
            current = fh.read()
        if current != payload:
            print(args.out + " is stale; run with --write", file=sys.stderr)
            return 1
        print("OK: " + args.out + " matches the current tree")
        return 0

    if args.write:
        with open(args.out, "w", encoding="utf-8") as fh:
            fh.write(payload)
        print("wrote " + args.out)

    print(render_table(rows, summary, atlas_totals))
    return 0


if __name__ == "__main__":
    sys.exit(main())
