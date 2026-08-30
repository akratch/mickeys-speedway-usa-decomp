#!/usr/bin/env python3

from __future__ import annotations

import contextlib
import io
from pathlib import Path
import sys
import tempfile
import unittest

import run_logged


class LoggedRunTests(unittest.TestCase):
    def test_success_is_compact_and_retains_full_log(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            repo = Path(directory)
            (repo / "build").mkdir()
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                status = run_logged.run_logged(
                    repo,
                    Path("build/success.log"),
                    "demo",
                    [sys.executable, "-c", "print('detail line')"],
                )
            self.assertEqual(0, status)
            self.assertIn("demo PASS", output.getvalue())
            self.assertNotIn("detail line", output.getvalue())
            self.assertIn("detail line", (repo / "build/success.log").read_text())

    def test_failure_prints_only_bounded_tail(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            repo = Path(directory)
            (repo / "build").mkdir()
            error = io.StringIO()
            code = "import sys; print('old'); print('recent'); sys.exit(7)"
            with contextlib.redirect_stderr(error):
                status = run_logged.run_logged(
                    repo,
                    Path("build/failure.log"),
                    "demo",
                    [sys.executable, "-c", code],
                    tail_count=1,
                )
            self.assertEqual(7, status)
            self.assertIn("demo FAIL", error.getvalue())
            self.assertIn("recent", error.getvalue())
            self.assertNotIn("\nold\n", error.getvalue())

    def test_log_must_remain_under_build(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            repo = Path(directory)
            (repo / "build").mkdir()
            with self.assertRaises(run_logged.LoggedRunError):
                run_logged.resolve_log(repo, Path("outside.log"))

    def test_empty_command_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            repo = Path(directory)
            (repo / "build").mkdir()
            with self.assertRaises(run_logged.LoggedRunError):
                run_logged.run_logged(repo, Path("build/demo.log"), "demo", [])


if __name__ == "__main__":
    unittest.main()
