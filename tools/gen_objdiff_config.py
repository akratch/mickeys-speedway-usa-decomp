#!/usr/bin/env python3
"""Regenerates objdiff.json from the current build/ tree.

One unit per built object (base_path=build/<f>, target_path=expected/build/<f>),
grouped into progress_categories by their top-level build/ subdirectory
(src/libultra, src/main, src/overlays/oNNN, assets/overlays/oNNN, ...). Objects
under build/permuter/ and build/wb/ (permuter and workbench scratch, not part
of the real build) are excluded.

Run after any change to what gets built (a new TU, a renamed overlay, ...):

    .venv/bin/python tools/gen_objdiff_config.py > objdiff.json

objdiff.json itself is committed (it is names and paths derived from the
build graph, not ROM content); tools/objdiff_report.sh regenerates it if the
build/ tree looks newer, so this rarely needs to be run by hand.

--base-dir DIR swaps the base ("current") side for a different build tree
(for example `build_non_matching`, the `gmake NON_MATCHING=1` tree used by
tools/nm_ranking.py) while the target side stays expected/build/ -- that
tree mirrors build/'s own directory layout one-for-one below the top-level
dir, so the relative paths line up unchanged. The emitted config is not
committed in that case; it is regenerated/consumed in memory by the caller.
"""
import argparse
import json
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
EXCLUDE_PREFIXES = ("permuter/", "wb/")

# Objects objdiff-cli's report generator can't parse (POSTPROCESS-trimmed
# sections; see tools/objdiff_report.sh's retry loop, which populates this
# file as it discovers them). One build/-relative path per line.
EXCLUDE_FILE = ROOT / "tools" / "objdiff_exclude.txt"


def load_excludes() -> set:
    if not EXCLUDE_FILE.is_file():
        return set()
    return {
        line.strip()
        for line in EXCLUDE_FILE.read_text().splitlines()
        if line.strip() and not line.startswith("#")
    }


def category_id(rel: pathlib.PurePosixPath) -> str:
    # build/src/overlays/o005/foo.c.o -> src-overlays-o005
    # build/src/libultra/foo.c.o      -> src-libultra
    # build/assets/overlays/o005/...  -> assets-overlays-o005
    # build/asm/1050.s.o               -> asm  (file directly under a top dir)
    dirs = rel.parts[:-1]  # drop the filename itself
    if len(dirs) >= 3 and dirs[0] in ("src", "assets") and dirs[1] == "overlays":
        return f"{dirs[0]}-overlays-{dirs[2]}"
    if len(dirs) >= 2:
        return "-".join(dirs[:2])
    if len(dirs) == 1:
        return dirs[0]
    return "root"


def category_name(cat_id: str) -> str:
    return cat_id


def parse_args(argv) -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument(
        "--base-dir",
        default="build",
        help="build tree to use for the base ('current') side of each unit "
        "(default: build). The target side is always expected/build/.",
    )
    return p.parse_args(argv)


def main() -> int:
    args = parse_args(sys.argv[1:])
    base_dir_name = args.base_dir.strip("/")
    base_dir = ROOT / base_dir_name
    if not base_dir.is_dir():
        print(f"no {base_dir_name}/ directory -- run `gmake` first.", file=sys.stderr)
        return 1

    excludes = load_excludes()
    units = []
    categories = {}
    for obj in sorted(base_dir.rglob("*.o")):
        rel = obj.relative_to(base_dir).as_posix()
        if rel.startswith(EXCLUDE_PREFIXES) or rel in excludes:
            continue
        rel_p = pathlib.PurePosixPath(rel)
        cat = category_id(rel_p)
        categories.setdefault(cat, category_name(cat))
        units.append(
            {
                "name": rel[: -len(".o")] if rel.endswith(".o") else rel,
                "target_path": f"expected/build/{rel}",
                "base_path": f"{base_dir_name}/{rel}",
                "metadata": {"progress_categories": [cat]},
            }
        )

    config = {
        "custom_make": "gmake",
        "build_target": False,
        "build_base": False,
        "progress_categories": [
            {"id": cat_id, "name": name} for cat_id, name in sorted(categories.items())
        ],
        "units": units,
    }
    json.dump(config, sys.stdout, indent=2)
    sys.stdout.write("\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
