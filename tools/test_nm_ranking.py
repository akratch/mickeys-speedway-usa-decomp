#!/usr/bin/env python3
"""Focused tests for fail-closed nonmatching-ranking pruning."""

from __future__ import annotations

import json
from pathlib import Path
import tempfile
import unittest

import nm_ranking as ranking


def function_row(
    file_name: str, symbol: str, pct: float | None = None
) -> dict[str, object]:
    return {"file": file_name, "name": symbol, "objdiff_match_pct": pct}


class PruneStaleTests(unittest.TestCase):
    def test_prunes_only_nonlive_exact_identities_and_normalizes_counts(self) -> None:
        keep = ("src/main/keep.c", "keep")
        pending = ("src/main/pending.c", "pending")
        document = {
            "queue_size": 99,
            "resolved": 98,
            "unresolved": 1,
            "objdiff_match_pct_coverage": 98,
            "functions": [
                function_row(*keep, pct=91.5),
                function_row("src/main/matched.c", "matched", pct=100.0),
            ],
            "unresolved_functions": [
                [[*pending], "compile failed"],
                [["src/main/resolved.c", "resolved"], "old failure"],
            ],
        }

        pruned, removed, unranked = ranking.prune_stale_document(
            document, {keep, pending, ("src/main/new.c", "new")}
        )

        self.assertEqual([row["name"] for row in pruned["functions"]], ["keep"])
        self.assertEqual(
            pruned["unresolved_functions"], [[[*pending], "compile failed"]]
        )
        self.assertEqual(pruned["queue_size"], 2)
        self.assertEqual(pruned["resolved"], 1)
        self.assertEqual(pruned["unresolved"], 1)
        self.assertEqual(pruned["objdiff_match_pct_coverage"], 1)
        self.assertEqual(
            removed,
            [
                ("src/main/matched.c", "matched"),
                ("src/main/resolved.c", "resolved"),
            ],
        )
        self.assertEqual(unranked, [("src/main/new.c", "new")])
        self.assertEqual(document["queue_size"], 99)

    def test_unidentified_unresolved_row_fails_closed(self) -> None:
        document = {
            "functions": [],
            "unresolved_functions": ["func (src/main/file.c): failed"],
        }
        with self.assertRaisesRegex(
            ranking.RankingDocumentError,
            r"needs \[\[file, name\], diagnostic\] identity",
        ):
            ranking.prune_stale_document(document, set())

    def test_duplicate_identity_fails_closed(self) -> None:
        key = ("src/main/dup.c", "dup")
        document = {
            "functions": [function_row(*key)],
            "unresolved_functions": [[[*key], "also unresolved"]],
        }
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "duplicate function identities"
        ):
            ranking.prune_stale_document(document, {key})

    def test_atomic_writer_replaces_complete_json(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "ranking.json"
            path.write_text('{"old": true}\n', encoding="utf-8")
            path.chmod(0o640)
            ranking.write_json_atomic(path, {"queue_size": 0})
            self.assertEqual(json.loads(path.read_text()), {"queue_size": 0})
            self.assertEqual(path.stat().st_mode & 0o777, 0o640)
            self.assertEqual(list(path.parent.glob(".ranking.json.*.tmp")), [])


if __name__ == "__main__":
    unittest.main()
