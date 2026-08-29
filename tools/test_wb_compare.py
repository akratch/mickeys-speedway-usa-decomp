#!/usr/bin/env python3
"""Focused integration tests for wb_compare.sh symbol routing."""

from __future__ import annotations

import os
import pathlib
import shutil
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]


class LinkedSymbolSelectionTests(unittest.TestCase):
    def test_asm_mode_uses_friendly_symbol_for_linked_geometry(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            tools = fixture / "tools"
            binutils = tools / "binutils"
            binutils.mkdir(parents=True)
            (fixture / "asm" / "nonmatchings").mkdir(parents=True)

            script = tools / "wb_compare.sh"
            shutil.copy2(ROOT / "tools" / "wb_compare.sh", script)

            objdump = binutils / "mips64-elf-objdump"
            objdump.write_text(
                "#!/bin/sh\n"
                "printf '%s\\n' '00000000 g F .text 000000ac friendly'\n"
            )
            objdump.chmod(0o755)

            env = os.environ.copy()
            env["WB_CANDIDATE_SYMBOL"] = "friendly"
            result = subprocess.run(
                [str(script), "func_overlay_auto"],
                cwd=fixture,
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 1)
            self.assertIn(
                "no asm/nonmatchings/**/func_overlay_auto.s.", result.stderr
            )
            self.assertNotIn("not found in build/mickey.us.elf", result.stderr)


if __name__ == "__main__":
    unittest.main()
