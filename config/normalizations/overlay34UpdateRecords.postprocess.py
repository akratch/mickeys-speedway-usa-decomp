#!/usr/bin/env python3
"""Run the guarded Overlay 34 update-record postprocessing chain."""

import pathlib
import subprocess
import sys


REPO = pathlib.Path(__file__).resolve().parents[2]
PREPARE = REPO / "config/normalizations/overlay34UpdateRecords.prepare.py"
OPS = REPO / "config/normalizations/overlay34UpdateRecords.ops"
NORMALIZE = REPO / "tools/normalize_elf_instructions.py"
TRIM = REPO / "tools/trim_elf_section.py"
OBJCOPY = REPO / "tools/binutils/mips64-elf-objcopy"
EXPECTED = "1ef3cff290da4b1b623f4aa86de717e4cafaf4dcd3df71b8eb3be07d43c4005e"


if len(sys.argv) != 2:
    raise SystemExit("usage: overlay34UpdateRecords.postprocess.py OBJECT")

obj = pathlib.Path(sys.argv[1])
subprocess.run([sys.executable, PREPARE, obj], cwd=REPO, check=True)
subprocess.run(
    [sys.executable, NORMALIZE, obj, ".text", "0x134", EXPECTED, f"@{OPS}"],
    cwd=REPO,
    check=True,
)
subprocess.run(
    [OBJCOPY, "--redefine-sym", "overlay34RemoveRecord=overlay34RemoveRecordReloc", obj],
    cwd=REPO,
    check=True,
)
subprocess.run([sys.executable, TRIM, obj, ".text", "0x134"], cwd=REPO, check=True)
