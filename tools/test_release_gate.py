#!/usr/bin/env python3
"""Focused tests for release preflight and health-gated command execution."""

from __future__ import annotations

from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


TOOLS = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOLS))

import release_gate  # noqa: E402


class PublicPreflightTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory(prefix="release-gate-")
        self.repo = Path(self.temporary.name)
        self.git("init", "-q", "-b", "master")
        self.git("config", "user.email", "release@example.invalid")
        self.git("config", "user.name", "Release Test")
        self.git("remote", "add", "public", "https://example.invalid/public.git")
        (self.repo / "README.md").write_text("public project\n")
        self.git("add", "README.md")
        self.git("commit", "-q", "-m", "Initial public tree")
        self.git("update-ref", "refs/remotes/public/master", "HEAD")

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def git(self, *args: str) -> str:
        return subprocess.run(
            ["git", *args], cwd=self.repo, check=True, text=True,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        ).stdout

    def test_clean_public_tree_passes(self) -> None:
        detail = release_gate._public_preflight(self.repo, "master", "public")
        self.assertIn("remote=public", detail)

    def test_scans_all_tracked_text_even_without_outgoing_delta(self) -> None:
        token = "ghp_" + "A" * 30
        (self.repo / "README.md").write_text(f"credential {token}\n")
        self.git("add", "README.md")
        self.git("commit", "-q", "-m", "Historical unsafe content")
        self.git("update-ref", "refs/remotes/public/master", "HEAD")
        with self.assertRaisesRegex(release_gate.PreflightError, "tracked/outgoing"):
            release_gate._public_preflight(self.repo, "master", "public")

    def test_outgoing_commit_messages_are_scanned(self) -> None:
        (self.repo / "README.md").write_text("safe update\n")
        self.git("add", "README.md")
        self.git("commit", "-q", "-m", "Sync campaign/" + "unchain details")
        with self.assertRaisesRegex(release_gate.PreflightError, "tracked/outgoing"):
            release_gate._public_preflight(self.repo, "master", "public")

    def test_tracked_dirt_is_rejected(self) -> None:
        (self.repo / "README.md").write_text("dirty\n")
        with self.assertRaisesRegex(release_gate.PreflightError, "tracked worktree"):
            release_gate._public_preflight(self.repo, "master", "public")

    def test_secret_assignment_scan_distinguishes_code_from_credentials(self) -> None:
        self.assertFalse(release_gate._scan_text("code.c", "token = poll_next(0);"))
        self.assertTrue(
            release_gate._scan_text("config", 'password = "not-a-real-password"')
        )


class GateExecutionTests(unittest.TestCase):
    def test_fake_gate_failure_is_reported(self) -> None:
        with tempfile.TemporaryDirectory(prefix="release-gate-command-") as temporary:
            root = Path(temporary)
            command = root / "gate"
            command.write_text("#!/bin/sh\nexit 7\n")
            command.chmod(0o755)
            status, _elapsed = release_gate._run_gate(
                root,
                "verify",
                root / "gate.log",
                niceness=0,
                timeout=5,
                fake_command=command,
                dry_run=False,
            )
            self.assertEqual(status, "FAIL")


if __name__ == "__main__":
    unittest.main()
