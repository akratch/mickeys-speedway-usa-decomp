#!/usr/bin/env python3
"""Focused tests for deterministic public-release reconciliation."""

from __future__ import annotations

import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


TOOLS = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOLS))

import public_release as pr  # noqa: E402


class GitRepoCase(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory(prefix="public-release-")
        self.repo = Path(self.temporary.name)
        self.git("init", "-q", "-b", "master")
        self.git("config", "user.email", "release@example.invalid")
        self.git("config", "user.name", "Release Test")
        self.git("remote", "add", "public", "https://example.invalid/project.git")
        (self.repo / "README.md").write_text("public project\n")
        (self.repo / "code.c").write_text("int example;\n")
        self.commit("Initial public tree")
        self.git("update-ref", "refs/remotes/public/master", "HEAD")

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def git(self, *args: str) -> str:
        return subprocess.run(
            ["git", *args],
            cwd=self.repo,
            check=True,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        ).stdout

    def commit(self, message: str) -> None:
        self.git("add", "-A")
        self.git("commit", "-q", "-m", message)


class ReleaseContextTests(GitRepoCase):
    def test_named_fast_forward_context_passes(self) -> None:
        (self.repo / "README.md").write_text("safe outgoing update\n")
        self.commit("Safe update")
        ctx = pr._release_context(
            self.repo, "master", "public", require_clean=True
        )
        self.assertEqual(ctx.branch, "master")
        self.assertEqual(ctx.remote, "public")
        self.assertEqual(len(ctx.outgoing_commits), 1)
        self.assertFalse(pr._scan_release(ctx, include_worktree=False))
        commands = pr._reconciliation_commands(ctx, write_derived=False)
        self.assertEqual(commands[0][0], "outgoing-cleanroom")
        self.assertIn("refs/remotes/public/master..HEAD", commands[0])

    def test_wrong_branch_fails_closed(self) -> None:
        self.git("switch", "-q", "-c", "topic")
        with self.assertRaisesRegex(pr.PublicReleaseError, "current branch"):
            pr._release_context(self.repo, "master", "public", require_clean=True)

    def test_remote_credential_is_rejected_before_url_is_reported(self) -> None:
        credential = "ghp_" + "A" * 30
        self.git(
            "remote",
            "set-url",
            "public",
            f"https://{credential}@example.invalid/project.git",
        )
        with self.assertRaisesRegex(pr.PublicReleaseError, "remote URL"):
            pr._release_context(self.repo, "master", "public", require_clean=True)

    def test_transient_outgoing_text_is_scanned_after_later_removal(self) -> None:
        marker = "campaign/" + "unchain"
        (self.repo / "README.md").write_text(f"temporary {marker}\n")
        self.commit("Temporary text")
        (self.repo / "README.md").write_text("clean final tree\n")
        self.commit("Remove temporary text")
        ctx = pr._release_context(
            self.repo, "master", "public", require_clean=True
        )
        findings = pr._scan_release(ctx, include_worktree=False)
        self.assertTrue(any("README.md" in row for row in findings))

    def test_operator_only_path_is_rejected(self) -> None:
        hidden = self.repo / ("." + "codex")
        hidden.mkdir()
        (hidden / "note.txt").write_text("ordinary prose\n")
        self.commit("Add misplaced operator note")
        ctx = pr._release_context(
            self.repo, "master", "public", require_clean=True
        )
        findings = pr._scan_release(ctx, include_worktree=False)
        self.assertTrue(any("forbidden tracked path" in row for row in findings))


class DeltaTests(unittest.TestCase):
    def test_scoreboard_metric_deltas_are_exact(self) -> None:
        def scoreboard(functions: int, resident: int, overlay: int) -> str:
            return f"""before
{pr.SCOREBOARD_BEGIN}
## Progress
```
functions      {functions} / 20
.text bytes    {resident} / 1000
verified asm   40 / 1000
overlay C      {overlay} / 800
whole resolved {resident + overlay + 40} / 1800
named          15 / 20
symbols        200
```
```
decompiled              {resident + overlay} / 1800
GLOBAL_ASM remaining    300 / 1800
NON_MATCHING            200 / 1800
```
{pr.SCOREBOARD_END}
after
"""

        lines = pr._metric_delta_lines(scoreboard(10, 400, 200), scoreboard(11, 428, 232))
        self.assertIn("metric functions: 10 -> 11 (+1)", lines)
        self.assertIn("metric resident C bytes: 400 -> 428 (+28)", lines)
        self.assertIn("metric overlay C bytes: 200 -> 232 (+32)", lines)
        self.assertIn("metric whole resolved bytes: 640 -> 700 (+60)", lines)

    def test_overlay_promotions_and_retractions_reconcile_to_total(self) -> None:
        def row(start: int, end: int, *, exact: bool, source: str) -> dict:
            return {
                "offset": hex(start),
                "end_offset": hex(end),
                "size": hex(end - start),
                "type": "c",
                "matched": True,
                "nonmatching": not exact,
                "source": source,
            }

        def atlas(rows: list[dict], matched: int) -> dict:
            return {
                "schema_version": 1,
                "totals": {"matched_overlay_c_bytes": matched},
                "modules": [{"overlay": 7, "text_ownership": rows}],
            }

        old = atlas(
            [
                row(0x0, 0x20, exact=True, source="oldExact"),
                row(0x20, 0x40, exact=False, source="oldCandidate"),
            ],
            32,
        )
        new = atlas(
            [
                row(0x0, 0x20, exact=False, source="oldExact"),
                row(0x20, 0x40, exact=True, source="oldCandidate"),
            ],
            32,
        )
        delta = pr._exact_range_delta(old, new)
        self.assertEqual(
            [(row["offset"], row["end_offset"], row["size"]) for row in delta["retractions"]],
            [(0x0, 0x20, 32)],
        )
        self.assertEqual(delta["promotions"][0]["offset"], 0x20)
        self.assertEqual(delta["totals"]["net_exact_c_bytes"], 0)

    def test_overlay_delta_fails_if_totals_do_not_reconcile(self) -> None:
        empty = {
            "schema_version": 1,
            "totals": {"matched_overlay_c_bytes": 0},
            "modules": [],
        }
        stale = json.loads(json.dumps(empty))
        stale["totals"]["matched_overlay_c_bytes"] = 4
        with self.assertRaisesRegex(pr.PublicReleaseError, "atlas declares 4"):
            pr._exact_range_delta(empty, stale)


class CommandPlanTests(unittest.TestCase):
    def test_default_plan_is_read_only_and_no_plan_can_publish(self) -> None:
        dry_targets = [part for row in pr.READ_ONLY_GENERATORS for part in row]
        write_targets = [part for row in pr.WRITE_GENERATORS for part in row]
        self.assertIn("overlay-atlas", dry_targets)
        self.assertIn("overlay-atlas-write", write_targets)
        for forbidden in ("push", "merge", "fetch", "cp", "rsync"):
            self.assertNotIn(forbidden, dry_targets)
            self.assertNotIn(forbidden, write_targets)

    def test_new_public_files_do_not_trigger_their_own_text_scan(self) -> None:
        root = TOOLS.parent
        for relative in (
            "Makefile",
            "docs/tools.md",
            "tools/public_release.py",
            "tools/test_public_release.py",
        ):
            findings = pr._scan_payload(relative, (root / relative).read_bytes())
            self.assertFalse(findings, f"{relative}: {findings}")

    def test_tracked_local_toolchain_links_are_forbidden_paths(self) -> None:
        self.assertTrue(pr._forbidden_path("tools/" + "ido"))
        self.assertTrue(pr._forbidden_path("tools/" + "binutils"))


class DerivedPathTests(GitRepoCase):
    def test_write_mode_allowlist_rejects_unexpected_tracked_change(self) -> None:
        (self.repo / "README.md").write_text("generated scoreboard\n")
        self.assertFalse(pr._unexpected_derived_changes(self.repo))
        (self.repo / "code.c").write_text("int changed;\n")
        self.assertEqual(pr._unexpected_derived_changes(self.repo), ["code.c"])


if __name__ == "__main__":
    unittest.main()
