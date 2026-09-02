#!/usr/bin/env python3
"""Refuse a merge that silently un-matches a function.

Compares the set of functions still carried by `#pragma GLOBAL_ASM` under
src/main/ and src/overlays/ between a base commit (default HEAD) and the
working tree.  Any name that had no GLOBAL_ASM at the base but has one now
was matched before the merge and is not matched after it: a lane's older
base won a whole-file conflict resolution, or a rebase dropped a body.

The exceptions are a pragma in a brand-new C file that replaces a raw
`asm`/`hasm` subsegment at the same configured ROM start, and an overlay TU
whose surviving exact functions are explicitly accounted for by reviewed
`mixed_tu_exact_c_ranges`.  The former was already unresolved assembly; the
latter uses the repository's corrected mixed-TU accounting recipe.

    tools/check_match_regression.py [base-commit]

Exit 1 with the offending names on stderr; exit 0 (quietly) otherwise.
"""
import glob
import re
import subprocess
import sys
import json

PAT = re.compile(r'#pragma GLOBAL_ASM\("asm/[^"]*/([^/"]+)\.s"\)')
SUBSEGMENT_PAT = re.compile(
    r"^\s*-\s*\[\s*(0x[0-9A-Fa-f]+)\s*,\s*([A-Za-z0-9_.]+)"
    r"(?:\s*,\s*([A-Za-z0-9_./-]+))?"
)
DIRS = ("src/main/", "src/overlays/", "src/libultra/")


def names_at(commit):
    files = subprocess.run(["git", "ls-tree", "-r", "--name-only", commit, *DIRS],
                           capture_output=True, text=True, check=True).stdout.split()
    out = set()
    for f in files:
        if not f.endswith(".c"):
            continue
        blob = subprocess.run(["git", "show", f"{commit}:{f}"], capture_output=True, text=True).stdout
        out.update(PAT.findall(blob))
    return out


def names_in_worktree():
    out = set()
    files = subprocess.run(["git", "ls-files", *DIRS], capture_output=True, text=True, check=True).stdout.split()
    for f in files:
        if not f.endswith(".c"):
            continue
        try:
            out.update(PAT.findall(open(f, encoding="utf-8", errors="ignore").read()))
        except FileNotFoundError:
            pass
    return out


def subsegments(text):
    out = {}
    for line in text.splitlines():
        match = SUBSEGMENT_PAT.match(line)
        if match:
            out[int(match.group(1), 16)] = (match.group(2), match.group(3))
    return out


def raw_asm_promotions(base):
    """Names newly exposed as pragmas by an asm/hasm-to-C split change."""
    base_yaml = subprocess.run(
        ["git", "show", f"{base}:mickey.us.yaml"],
        capture_output=True,
        text=True,
        check=True,
    ).stdout
    try:
        with open("mickey.us.yaml", encoding="utf-8") as fh:
            worktree_yaml = fh.read()
    except FileNotFoundError:
        return set()

    before = subsegments(base_yaml)
    after = subsegments(worktree_yaml)
    names = set()
    base_starts = sorted(before)
    for start, (kind, source_name) in after.items():
        # The base row that covered this start: an exact row, or the nearest
        # earlier row when a lane split one raw asm range into several C
        # rows (the functions were unresolved assembly either way).
        covering = None
        for base_start in base_starts:
            if base_start <= start:
                covering = base_start
            else:
                break
        old_kind, _ = before.get(covering, (None, None))
        if kind != "c" or not source_name or old_kind not in {"asm", "hasm"}:
            continue

        # Resident rows name the source relative to src/; overlay rows name
        # only the basename (the overlay segment supplies its directory), so
        # resolve either form against the tracked tree.
        candidates = [f"src/{source_name}.c"] + sorted(
            glob.glob(f"src/**/{source_name}.c", recursive=True))
        for source in candidates:
            existed = subprocess.run(
                ["git", "cat-file", "-e", f"{base}:{source}"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            ).returncode == 0
            if existed:
                continue
            try:
                with open(source, encoding="utf-8", errors="ignore") as fh:
                    names.update(PAT.findall(fh.read()))
            except FileNotFoundError:
                pass
    return names


NM_BLOCK = re.compile(r"^\s*#\s*ifdef\s+NON_MATCHING\b", re.MULTILINE)


def credited_overlay_sources(base):
    """Overlay source files whose ownership row was credited at `base`
    (matched and not nonmatching in config/overlays.us.json)."""
    try:
        blob = subprocess.run(["git", "show", f"{base}:config/overlays.us.json"],
                              capture_output=True, text=True, check=True).stdout
        atlas = json.loads(blob)
    except (subprocess.CalledProcessError, ValueError):
        return set()
    out = set()
    for module in atlas.get("modules", []):
        for row in module.get("text_ownership", []):
            if row.get("type") == "c" and row.get("matched") and not row.get("nonmatching"):
                out.add(f"src/{row['source']}.c")
    return out


def mixed_overlay_sources(atlas):
    """Return source paths protected by explicit mixed-TU exact ranges."""
    out = set()
    for module in atlas.get("modules", []):
        for row in module.get("mixed_tu_exact_c_ranges", []):
            source = row.get("source")
            if source:
                out.add(f"src/{source}.c")
    return out


def reviewed_mixed_overlay_sources():
    try:
        with open("config/overlays.us.json", encoding="utf-8") as fh:
            return mixed_overlay_sources(json.load(fh))
    except (FileNotFoundError, ValueError):
        return set()


def overlay_credit_regressions(base):
    """Credited overlay TUs that now carry a NON_MATCHING block they did not
    have at `base`. The atlas credits ownership per source file, so one
    candidate inside a matched TU silently un-credits every function in it
    (overlays 51 and 56 lost 5,024 bytes this way on 2026-08-28)."""
    bad = []
    reviewed_mixed = reviewed_mixed_overlay_sources()
    for f in sorted(credited_overlay_sources(base)):
        try:
            now = open(f, encoding="utf-8", errors="ignore").read()
        except FileNotFoundError:
            continue
        if not NM_BLOCK.search(now):
            continue
        then = subprocess.run(["git", "show", f"{base}:{f}"], capture_output=True, text=True).stdout
        if not NM_BLOCK.search(then) and f not in reviewed_mixed:
            bad.append(f)
    return bad


def main():
    base = sys.argv[1] if len(sys.argv) > 1 else "HEAD"
    before = names_at(base)
    after = names_in_worktree()
    regressed = sorted(after - before - raw_asm_promotions(base))
    status = 0
    if regressed:
        print(f"check_match_regression: {len(regressed)} function(s) matched at {base} carry GLOBAL_ASM again:",
              file=sys.stderr)
        for n in regressed:
            print(f"  {n}", file=sys.stderr)
        status = 1
    uncredited = overlay_credit_regressions(base)
    if uncredited:
        print(f"check_match_regression: {len(uncredited)} credited overlay TU(s) gained a NON_MATCHING block "
              "(the whole TU loses credit; put the candidate in its own file or leave the pragma):",
              file=sys.stderr)
        for f in uncredited:
            print(f"  {f}", file=sys.stderr)
        status = 1
    return status


if __name__ == "__main__":
    sys.exit(main())
