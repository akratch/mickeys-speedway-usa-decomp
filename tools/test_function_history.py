#!/usr/bin/env python3
"""Focused tests for exact-function Git source history."""

from __future__ import annotations

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import function_history as history  # noqa: E402


class GuardedBodyHistoryTests(unittest.TestCase):
    def git(self, root: Path, *arguments: str) -> str:
        result = subprocess.run(
            ["git", *arguments],
            cwd=root,
            check=True,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        return result.stdout.strip()

    def commit(self, root: Path, subject: str) -> str:
        self.git(root, "add", "src/example.c")
        self.git(root, "commit", "-m", subject)
        return self.git(root, "rev-parse", "HEAD")

    def test_reports_only_commits_that_changed_requested_guarded_body(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.git(root, "init", "-q")
            self.git(root, "config", "user.name", "History Test")
            self.git(root, "config", "user.email", "history@example.invalid")
            source = root / "src/example.c"
            source.parent.mkdir(parents=True)
            source.write_text(
                "#ifdef NON_MATCHING\n"
                "s32 target(s32 value) { return value + 1; }\n"
                "#else\n#pragma GLOBAL_ASM(\"asm/target.s\")\n#endif\n"
                "s32 other(void) { return 0; }\n",
                encoding="utf-8",
            )
            initial = self.commit(root, "Add first target candidate")

            source.write_text(
                source.read_text(encoding="utf-8").replace("return 0", "return 2"),
                encoding="utf-8",
            )
            self.commit(root, "Change unrelated function")

            source.write_text(
                source.read_text(encoding="utf-8").replace(
                    "return value + 1;", "/* layout note */ return  value + 1 ;"
                ),
                encoding="utf-8",
            )
            self.commit(root, "Reformat target only")

            source.write_text(
                source.read_text(encoding="utf-8").replace(
                    "return  value + 1 ;", "return value + 3;"
                ),
                encoding="utf-8",
            )
            changed = self.commit(root, "Try authentic target lifetime")

            rows = history.guarded_body_history(source, "target", root=root)

        self.assertEqual([changed, initial], [row.commit for row in rows])
        self.assertEqual(
            ["Try authentic target lifetime", "Add first target candidate"],
            [row.subject for row in rows],
        )

    def test_unguarded_definition_is_not_reported_as_candidate_history(self) -> None:
        text = "s32 target(void) { return 1; }\n"
        self.assertIsNone(history.guarded_body_fingerprint(text, "target"))


if __name__ == "__main__":
    unittest.main()
