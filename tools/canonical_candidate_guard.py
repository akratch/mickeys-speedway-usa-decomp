#!/usr/bin/env python3
"""Keep candidate experiments out of the canonical ROM verification graph."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import tempfile


FORMAT_VERSION = 1


def digest(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            value.update(chunk)
    return value.hexdigest()


def load_receipt(path: Path) -> dict[str, str]:
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
        if payload.get("format_version") != FORMAT_VERSION:
            return {}
        objects = payload.get("objects")
        if not isinstance(objects, dict):
            return {}
        if not all(isinstance(key, str) and isinstance(value, str)
                   for key, value in objects.items()):
            return {}
        return objects
    except (OSError, ValueError, AttributeError):
        return {}


def dirty_objects(manifest: Path, objects: list[Path]) -> list[Path]:
    known = load_receipt(manifest)
    dirty: list[Path] = []
    for path in objects:
        try:
            current = digest(path)
        except OSError:
            dirty.append(path)
            continue
        if known.get(path.as_posix()) != current:
            dirty.append(path)
    return dirty


def write_receipt(manifest: Path, objects: list[Path]) -> None:
    missing = [path for path in objects if not path.is_file()]
    if missing:
        raise SystemExit(
            "cannot write canonical candidate receipt; missing: "
            + ", ".join(path.as_posix() for path in missing)
        )
    payload = {
        "format_version": FORMAT_VERSION,
        "objects": {
            path.as_posix(): digest(path)
            for path in sorted(objects, key=lambda item: item.as_posix())
        },
    }
    manifest.parent.mkdir(parents=True, exist_ok=True)
    handle, temporary = tempfile.mkstemp(
        prefix=manifest.name + ".", dir=manifest.parent
    )
    try:
        with os.fdopen(handle, "w", encoding="utf-8") as stream:
            json.dump(payload, stream, indent=2, sort_keys=True)
            stream.write("\n")
        os.replace(temporary, manifest)
    except BaseException:
        try:
            os.unlink(temporary)
        except OSError:
            pass
        raise


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("mode", choices=("dirty", "write"))
    parser.add_argument("objects", nargs="+", type=Path)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.mode == "dirty":
        for path in dirty_objects(args.manifest, args.objects):
            print(path.as_posix())
    else:
        write_receipt(args.manifest, args.objects)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
