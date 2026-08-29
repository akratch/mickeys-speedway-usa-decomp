#!/usr/bin/env python3
"""Focused integration tests for wb_compare.sh routing and option forwarding."""

from __future__ import annotations

import os
import pathlib
import shutil
import subprocess
import sys
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]


class WrapperFixture:
    def __init__(self, root: pathlib.Path) -> None:
        self.root = root
        tools = root / "tools"
        binutils = tools / "binutils"
        binutils.mkdir(parents=True)
        (root / ".venv" / "bin").mkdir(parents=True)
        (root / "build_non_matching/src/overlays/o016").mkdir(parents=True)
        (root / "build").mkdir()
        (root / "src/overlays/o016").mkdir(parents=True)
        asm = root / "asm/nonmatchings/overlays/o016/example/func_overlay_016_F0000000_1.s"
        asm.parent.mkdir(parents=True)
        asm.write_text("glabel func_overlay_016_F0000000_1\n", encoding="utf-8")
        (root / "src/overlays/o016/example.c").write_text(
            "void friendly(void) {}\n", encoding="utf-8"
        )
        (root / "build_non_matching/src/overlays/o016/example.c.o").write_bytes(b"obj")
        (root / "build/mickey.us.elf").write_bytes(b"elf")

        shutil.copy2(ROOT / "tools/wb_compare.sh", tools / "wb_compare.sh")
        os.symlink(sys.executable, root / ".venv/bin/python")
        (tools / "function_preflight.py").write_text(
            "print('func_overlay_016_F0000000_1\\tfriendly\\t'"
            "+'src/overlays/o016/example.c\\toverlays/o016/example\\t'"
            "+'build_non_matching\\tasm/nonmatchings/overlays/o016/example/'"
            "+'func_overlay_016_F0000000_1.s')\n",
            encoding="utf-8",
        )
        (tools / "proof_provenance.py").write_text(
            "raise SystemExit(0)\n", encoding="utf-8"
        )
        self._executable(
            binutils / "mips64-elf-objdump",
            "#!/bin/sh\nprintf '%s\\n' '00000000 g F .text 000000ac friendly'\n",
        )
        self._executable(
            binutils / "mips64-elf-as",
            "#!/bin/sh\nwhile [ \"$1\" != -o ]; do shift; done\n"
            "shift\n: > \"$1\"\n",
        )
        self._executable(binutils / "mips64-elf-objcopy", "#!/bin/sh\nexit 0\n")
        self._executable(
            root / ".venv/bin/decomp-workbench",
            "#!/bin/sh\nprintf '%s\\n' \"$@\" > \"$WB_ARGS_OUT\"\n",
        )

    @staticmethod
    def _executable(path: pathlib.Path, text: str) -> None:
        path.write_text(text, encoding="utf-8")
        path.chmod(0o755)


class WrapperRoutingTests(unittest.TestCase):
    def test_resolves_friendly_symbol_and_nonmatching_tree_automatically(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            WrapperFixture(fixture)
            args_out = fixture / "args.txt"
            env = os.environ.copy()
            env["WB_ARGS_OUT"] = str(args_out)

            result = subprocess.run(
                [str(fixture / "tools/wb_compare.sh"), "friendly", "--json"],
                cwd=fixture,
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 0, result.stderr)
            arguments = args_out.read_text(encoding="utf-8").splitlines()
            self.assertEqual(arguments[0], "compare")
            self.assertIn(
                "build_non_matching/src/overlays/o016/example.c.o", arguments
            )
            self.assertIn("--function", arguments)
            self.assertIn("friendly", arguments)
            self.assertEqual(arguments[-1], "--json")

    def test_diagnose_mode_forwards_diagnostic_options_after_symbol(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            WrapperFixture(fixture)
            args_out = fixture / "args.txt"
            env = os.environ.copy()
            env["WB_ARGS_OUT"] = str(args_out)

            result = subprocess.run(
                [
                    str(fixture / "tools/wb_compare.sh"),
                    "--diagnose",
                    "func_overlay_016_F0000000_1",
                    "--trace",
                    "trace.log",
                    "--trace-proc",
                    "3",
                    "--terse",
                ],
                cwd=fixture,
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 0, result.stderr)
            arguments = args_out.read_text(encoding="utf-8").splitlines()
            self.assertEqual(arguments[0], "diagnose")
            tail = arguments[arguments.index("--objdump") + 2 :]
            self.assertEqual(
                tail, ["--trace", "trace.log", "--trace-proc", "3", "--terse"]
            )

    def test_preflight_freshness_failure_stops_before_workbench(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            WrapperFixture(fixture)
            (fixture / "tools/function_preflight.py").write_text(
                "import sys\n"
                "print('stale candidate object', file=sys.stderr)\n"
                "raise SystemExit(2)\n",
                encoding="utf-8",
            )
            args_out = fixture / "args.txt"
            env = os.environ.copy()
            env["WB_ARGS_OUT"] = str(args_out)

            result = subprocess.run(
                [str(fixture / "tools/wb_compare.sh"), "friendly", "--json"],
                cwd=fixture,
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 2)
            self.assertIn("stale candidate object", result.stderr)
            self.assertFalse(args_out.exists())


if __name__ == "__main__":
    unittest.main()
