#!/usr/bin/env python3
"""Build and validate the overlay shared-code donor ledger.

The ledger is deliberately exhaustive: every donor has a result for every one
of Mickey's 107 overlays, including explicit ``none`` and ``empty`` results.
That makes "check DKR first" a reviewable workflow rather than an anecdote.

Normal repository checks only validate the committed report and need no
out-of-tree reference builds::

    python3 tools/overlay_donor_scan.py --check

Maintainers with the pinned farm can reproduce it deterministically::

    python3 tools/overlay_donor_scan.py --write
    python3 tools/overlay_donor_scan.py --scan-check

The scanner delegates byte comparison to ``find_known_objects.py``.  No donor
source or object bytes enter this repository; the report contains only names,
relative object paths, hashes, match locations, and negative results.
"""

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys


REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ATLAS = os.path.join(REPO, "config", "overlays.us.json")
REPORT = os.path.join(REPO, "config", "overlay-donors.us.json")
MATCHER = os.path.join(REPO, "tools", "find_known_objects.py")
DIGESTER = os.path.join(REPO, "tools", "reference_build_digest.py")

DEFAULT_REFERENCES = (
    {
        "id": "dkr-us-v77",
        "project": "Diddy Kong Racing",
        "repo": "https://github.com/akratch/Diddy-Kong-Racing.git",
        "version": "us.v77",
        "baserom_sha1": "0cb115d8716dbbc2922fda38e533b9fe63bb9670",
        "object_root": os.path.expanduser(
            os.environ.get(
                "DKR_V77_OBJECTS",
                "~/Desktop/dev/decomp-refs/diddy-kong-racing/build",
            )
        ),
        "role": "primary same-lineage donor; pinned 100% matching revision",
        "expected_commit": "38d7f9ba39642e2b5311a76e0b83fb3fe2733262",
        "expected_digest": "917ba733782e07382dd753b50b496c9f8647caec8695f7bca0359a19f0cd763b",
        "expected_objects": 243,
    },
    {
        "id": "dkr-us-v80",
        "project": "Diddy Kong Racing",
        "repo": "https://github.com/akratch/Diddy-Kong-Racing.git",
        "version": "us.v80",
        "baserom_sha1": None,
        "object_root": os.path.expanduser(
            os.environ.get(
                "DKR_V80_OBJECTS",
                "~/Desktop/dev/Diddy-Kong-Racing/build",
            )
        ),
        "role": "secondary revision cross-check",
        "expected_commit": "38d7f9ba39642e2b5311a76e0b83fb3fe2733262",
        "expected_digest": "91e8524065ba66c094c1107fa7c9ef0d9a9dff4026799a665b59e78dcf3c9243",
        "expected_objects": 243,
    },
    {
        "id": "jfg-us",
        "project": "Jet Force Gemini",
        "repo": "https://github.com/Ryan-Myers/Jet-Force-Gemini",
        "version": "us",
        "baserom_sha1": "493ced9008dbe932d6e91179b68e8630cf23a023",
        "object_root": os.path.expanduser(
            os.environ.get("JFG_OBJECTS", "~/Desktop/dev/decomp-refs/jfg/build")
        ),
        "role": "same runtime-linker family and overlay-layout donor",
        "expected_commit": "c82affffe8f11cb5b440cfa918f4582ad8573279",
        "expected_digest": "8c4036b7e2404e989e010ba899331a1dfe972b0d570e8c9535d7fde53788585d",
        "expected_objects": 772,
    },
)

PLACEHOLDER_RE = re.compile(r"^(?:func_|_AutoExit|D_[0-9A-Fa-f])")


def load_json(path):
    with open(path, encoding="utf-8") as fh:
        return json.load(fh)


def sha256_file(path):
    digest = hashlib.sha256()
    with open(path, "rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def run_output(argv, cwd=REPO):
    proc = subprocess.run(argv, cwd=cwd, text=True, capture_output=True)
    if proc.returncode:
        sys.stderr.write(proc.stderr)
        raise RuntimeError("command failed: %s" % " ".join(argv))
    return proc.stdout


def checkout_for_object_root(root):
    path = os.path.abspath(root)
    while path != os.path.dirname(path):
        if os.path.isdir(os.path.join(path, ".git")):
            return path
        path = os.path.dirname(path)
    raise RuntimeError("no git checkout above object root: %s" % root)


def mining_digest(checkout, object_root):
    rel = os.path.relpath(object_root, checkout)
    raw = run_output([sys.executable, DIGESTER, checkout, rel]).strip().split()
    if len(raw) != 2:
        raise RuntimeError("unexpected reference digest output")
    return raw[0], int(raw[1])


def normalized_match(row):
    symbol = row["symbol"].removesuffix(".NON_MATCHING")
    name_quality = "whole_text" if symbol == ".text" else (
        "placeholder" if PLACEHOLDER_RE.match(symbol) else "named"
    )
    strong = (
        row["rom_occurrences"] == "1"
        and row["search_occurrences"] == 1
    )
    return {
        "overlay": row["overlay"],
        "text_offset": "0x%X" % row["section_offset"],
        "rom": "0x%X" % row["rom"],
        "size": row["size"],
        "symbol": symbol,
        "name_quality": name_quality,
        "reference_object": row["reference_object"],
        "masked_words": row["masked_words"],
        "search_occurrences": row["search_occurrences"],
        "rom_occurrences": row["rom_occurrences"],
        "classification": "strong" if strong else "ambiguous",
    }


def scan_reference(spec, atlas):
    root = os.path.abspath(spec["object_root"])
    if not os.path.isdir(root):
        raise RuntimeError("missing %s object root: %s" % (spec["id"], root))
    checkout = checkout_for_object_root(root)
    commit = run_output(["git", "rev-parse", "HEAD"], cwd=checkout).strip()
    digest, objects = mining_digest(checkout, root)
    for key, got in (
        ("expected_commit", commit),
        ("expected_digest", digest),
        ("expected_objects", objects),
    ):
        if got != spec[key]:
            raise RuntimeError(
                "%s %s mismatch: got %s, expected %s"
                % (spec["id"], key, got, spec[key])
            )

    raw = json.loads(run_output([
        sys.executable,
        MATCHER,
        root,
        "--all-overlays",
        "--min-size", "0x18",
        "--max-occurrences", "4",
        "--rom-occ",
        "--json",
    ]))

    # JFG emits aliases suffixed .NON_MATCHING at the same address.  They are
    # one candidate, not two pieces of evidence.
    matches = {}
    for raw_row in raw["matches"]:
        row = normalized_match(raw_row)
        key = (
            row["overlay"], row["text_offset"], row["size"], row["symbol"],
            row["reference_object"],
        )
        matches[key] = row
    matches = sorted(matches.values(), key=lambda row: (
        row["overlay"], int(row["text_offset"], 0), -row["size"],
        row["reference_object"], row["symbol"],
    ))

    by_overlay = {n: [] for n in range(1, 108)}
    for row in matches:
        by_overlay[row["overlay"]].append(row)
    empty = {
        module["overlay"]
        for module in atlas["modules"]
        if int(module["sections"]["text"]["size"], 0) == 0
    }
    results = []
    for overlay in range(1, 108):
        candidates = by_overlay[overlay]
        if overlay in empty:
            status = "empty"
        elif any(row["classification"] == "strong" for row in candidates):
            status = "strong"
        elif candidates:
            status = "ambiguous"
        else:
            status = "none"
        results.append({
            "overlay": overlay,
            "status": status,
            "candidates": candidates,
        })

    return {
        "id": spec["id"],
        "project": spec["project"],
        "repo": spec["repo"],
        "version": spec["version"],
        "baserom_sha1": spec["baserom_sha1"],
        "role": spec["role"],
        "commit": commit,
        "object_count": objects,
        "mining_surface_digest": digest,
        "summary": {
            status: sum(row["status"] == status for row in results)
            for status in ("strong", "ambiguous", "none", "empty")
        },
        "overlay_results": results,
    }


def build_report():
    atlas = load_json(ATLAS)
    donors = [scan_reference(spec, atlas) for spec in DEFAULT_REFERENCES]
    return {
        "schema_version": 1,
        "generated_by": "tools/overlay_donor_scan.py",
        "atlas": {
            "path": "config/overlays.us.json",
            "sha256": sha256_file(ATLAS),
            "rom_sha1": atlas["source"]["sha1"],
            "modules": atlas["totals"]["modules"],
        },
        "scan_policy": {
            "scope": "all non-empty overlay text ranges",
            "minimum_symbol_size": 24,
            "maximum_search_occurrences": 4,
            "strong": "unique in selected ranges and unique in the full ROM",
            "adoption": (
                "A strong byte match proves shared code, but placeholder names "
                "are never promoted and named candidates still require provenance."
            ),
        },
        "donors": donors,
        "semantic_findings": [
            {
                "overlay": 61,
                "classification": "semantic",
                "finding": "ghost save/load and Controller Pak UI",
                "mickey_evidence": (
                    "overlay data contains MSU-GHOST, PAK PAGE, LOAD GHOST, "
                    "SAVE GHOST, PAK ERROR, and PAK FULL"
                ),
                "dkr_crosswalk": [
                    "src/save_data.c",
                    "src/racer.c",
                    "src/menu.c",
                ],
                "constraint": "workflow lead only; not an exact byte match",
            }
        ],
    }


def validate(report):
    errors = []
    atlas = load_json(ATLAS)
    expected_ids = list(range(1, 108))
    if report.get("schema_version") != 1:
        errors.append("unsupported report schema")
    metadata = report.get("atlas", {})
    if metadata.get("sha256") != sha256_file(ATLAS):
        errors.append("atlas digest is stale")
    if metadata.get("rom_sha1") != atlas["source"]["sha1"]:
        errors.append("atlas ROM SHA1 differs")
    donors = report.get("donors", [])
    expected_donors = [spec["id"] for spec in DEFAULT_REFERENCES]
    if [row.get("id") for row in donors] != expected_donors:
        errors.append("donor set/order differs from scanner defaults")
    specs = {row["id"]: row for row in DEFAULT_REFERENCES}
    for donor in donors:
        spec = specs.get(donor.get("id"))
        if spec:
            expected_metadata = {
                "project": spec["project"],
                "repo": spec["repo"],
                "version": spec["version"],
                "baserom_sha1": spec["baserom_sha1"],
                "commit": spec["expected_commit"],
                "object_count": spec["expected_objects"],
                "mining_surface_digest": spec["expected_digest"],
            }
            for key, expected in expected_metadata.items():
                if donor.get(key) != expected:
                    errors.append(
                        "%s %s differs from the pinned scanner metadata"
                        % (donor.get("id"), key)
                    )
        results = donor.get("overlay_results", [])
        if [row.get("overlay") for row in results] != expected_ids:
            errors.append("%s does not cover overlays 1..107" % donor.get("id"))
            continue
        counts = {
            status: sum(row.get("status") == status for row in results)
            for status in ("strong", "ambiguous", "none", "empty")
        }
        if donor.get("summary") != counts:
            errors.append("%s summary does not match results" % donor.get("id"))
        for row in results:
            candidates = row.get("candidates", [])
            if row.get("status") == "strong" and not any(
                item.get("classification") == "strong" for item in candidates
            ):
                errors.append("%s overlay %s has unsupported strong status" % (
                    donor.get("id"), row.get("overlay")
                ))
    if errors:
        for error in errors:
            print("[FAIL] %s" % error)
        return False
    print("[PASS] donor ledger covers 107 overlays for %d references" % len(donors))
    for donor in donors:
        summary = donor["summary"]
        print(
            "       %(id)s: %(strong)d strong, %(ambiguous)d ambiguous, "
            "%(none)d none, %(empty)d empty" % {**donor, **summary}
        )
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--write", action="store_true", help="scan and write report")
    mode.add_argument(
        "--scan-check", action="store_true",
        help="scan references and fail if the committed report differs",
    )
    mode.add_argument("--check", action="store_true", help="validate report")
    args = parser.parse_args()

    if args.write or args.scan_check:
        fresh = build_report()
        if args.scan_check:
            current = load_json(REPORT)
            if fresh != current:
                print("[FAIL] overlay donor ledger is stale; run with --write")
                return 1
        else:
            with open(REPORT, "w", encoding="utf-8") as fh:
                json.dump(fresh, fh, indent=2)
                fh.write("\n")
            print("wrote %s" % os.path.relpath(REPORT, REPO))
    if not os.path.isfile(REPORT):
        print("[FAIL] no donor ledger; run with --write")
        return 1
    return 0 if validate(load_json(REPORT)) else 1


if __name__ == "__main__":
    sys.exit(main())
