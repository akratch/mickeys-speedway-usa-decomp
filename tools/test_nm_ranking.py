#!/usr/bin/env python3
"""Focused tests for fail-closed ranking persistence and documentation."""

from __future__ import annotations

import json
from pathlib import Path
import tempfile
import unittest

import nm_ranking as ranking


def function_row(
    file_name: str,
    symbol: str,
    pct: float | None = None,
    *,
    category: str = "other",
    differing_words: int = 2,
) -> dict[str, object]:
    return {
        "file": file_name,
        "name": symbol,
        "overlay": None,
        "tu": "main",
        "size_bytes": 16,
        "objdiff_match_pct": pct,
        "differing_words": differing_words,
        "first_mismatch_offset": 4,
        "size_delta": 0,
        "category": category,
    }


def ranking_document(
    functions: list[dict[str, object]] | None = None,
    unresolved: list[list[object]] | None = None,
) -> dict[str, object]:
    function_rows = functions or []
    unresolved_rows = unresolved or []
    return {
        "queue_size": len(function_rows) + len(unresolved_rows),
        "resolved": len(function_rows),
        "unresolved": len(unresolved_rows),
        "objdiff_report_used": any(
            row["objdiff_match_pct"] is not None for row in function_rows
        ),
        "objdiff_match_pct_coverage": sum(
            row["objdiff_match_pct"] is not None for row in function_rows
        ),
        "functions": function_rows,
        "unresolved_functions": unresolved_rows,
    }


class PruneStaleTests(unittest.TestCase):
    def test_prunes_only_nonlive_exact_identities_and_normalizes_counts(self) -> None:
        keep = ("src/main/keep.c", "keep")
        pending = ("src/main/pending.c", "pending")
        document = ranking_document(
            [
                function_row(*keep, pct=91.5),
                function_row("src/main/matched.c", "matched", pct=100.0),
            ],
            [
                [[*pending], "compile failed"],
                [["src/main/resolved.c", "resolved"], "old failure"],
            ],
        )
        # Pruning repairs derived counts but still validates every row field.
        document["queue_size"] = 99
        document["resolved"] = 98
        document["unresolved"] = 1
        document["objdiff_match_pct_coverage"] = 98

        pruned, removed, unranked = ranking.prune_stale_document(
            document,
            {keep, pending, ("src/main/new.c", "new")},
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
        document = ranking_document()
        document["unresolved_functions"] = ["func (src/main/file.c): failed"]
        with self.assertRaisesRegex(
            ranking.RankingDocumentError,
            r"needs \[\[file, name\], diagnostic\] identity",
        ):
            ranking.prune_stale_document(document, set())

    def test_duplicate_identity_fails_closed(self) -> None:
        key = ("src/main/dup.c", "dup")
        document = ranking_document(
            [function_row(*key)], [[[*key], "also unresolved"]]
        )
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "duplicate function identities"
        ):
            ranking.prune_stale_document(document, {key})

    def test_atomic_writer_replaces_complete_json(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "ranking.json"
            path.write_text('{"old": true}\n', encoding="utf-8")
            path.chmod(0o640)
            document = ranking_document()
            ranking.write_json_atomic(path, document)
            self.assertEqual(json.loads(path.read_text()), document)
            self.assertEqual(path.stat().st_mode & 0o777, 0o640)
            self.assertEqual(list(path.parent.glob(".ranking.json.*.tmp")), [])


class ValidationTests(unittest.TestCase):
    def test_same_symbol_in_different_files_is_a_distinct_identity(self) -> None:
        document = ranking_document([
            function_row("src/main/one.c", "shared"),
            function_row("src/main/two.c", "shared"),
        ])
        ranking.validate_ranking_document(document)

    def test_duplicate_exact_identity_fails_closed(self) -> None:
        row = function_row("src/main/dup.c", "dup")
        document = ranking_document([row, dict(row)])
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "duplicate function identities"
        ):
            ranking.validate_ranking_document(document)

    def test_malformed_measurement_fails_closed(self) -> None:
        row = function_row("src/main/file.c", "symbol")
        row["differing_words"] = "two"
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "differing_words must be an integer"
        ):
            ranking.validate_ranking_document(ranking_document([row]))

    def test_location_inconsistent_with_exact_file_identity_fails_closed(self) -> None:
        row = function_row("src/overlays/o007/file.c", "symbol")
        row["overlay"] = 8
        row["tu"] = "overlays/o008"
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "overlay does not match its source path"
        ):
            ranking.validate_ranking_document(ranking_document([row]))

    def test_inconsistent_derived_count_fails_closed(self) -> None:
        document = ranking_document([
            function_row("src/main/file.c", "symbol")
        ])
        document["resolved"] = 0
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "resolved is 0, expected 1"
        ):
            ranking.validate_ranking_document(document)


class DocumentationTests(unittest.TestCase):
    def setUp(self) -> None:
        self.document = ranking_document(
            [
                function_row(
                    "src/main/a.c",
                    "same",
                    pct=87.25,
                    category="register-only",
                    differing_words=1,
                ),
                function_row(
                    "src/main/b.c",
                    "same",
                    category="size-mismatch",
                    differing_words=5,
                ),
            ],
            [[["src/main/pending.c", "pending"], "compile failed"]],
        )

    def test_render_is_stable_and_contains_complete_exact_identities(self) -> None:
        first = ranking.render_ranking_markdown(self.document)
        second = ranking.render_ranking_markdown(
            json.loads(json.dumps(self.document, sort_keys=True))
        )
        self.assertEqual(first, second)
        self.assertIn("**3 queued identities**", first)
        self.assertIn("`src/main/a.c` | `same`", first)
        self.assertIn("`src/main/b.c` | `same`", first)
        self.assertIn("`src/main/pending.c` | `pending` | compile failed", first)

    def test_generated_markers_preserve_authored_prose_byte_for_byte(self) -> None:
        original = (
            "authored before\n"
            f"{ranking.DOC_BEGIN}\nold generated\n{ranking.DOC_END}\n"
            "authored after\n"
        )
        replaced = ranking.replace_generated_markdown(original, "new generated\n")
        self.assertTrue(replaced.startswith("authored before\n"))
        self.assertTrue(replaced.endswith("\nauthored after\n"))
        self.assertIn("new generated\n", replaced)
        self.assertNotIn("old generated", replaced)

    def test_missing_or_duplicate_markers_fail_closed(self) -> None:
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "exactly one generated"
        ):
            ranking.replace_generated_markdown("no markers\n", "generated\n")
        duplicated = (
            f"{ranking.DOC_BEGIN}\n{ranking.DOC_BEGIN}\n{ranking.DOC_END}\n"
        )
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "exactly one generated"
        ):
            ranking.replace_generated_markdown(duplicated, "generated\n")

    def test_expected_document_detects_stale_generated_content(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "ranking.md"
            path.write_text(
                "authored\n"
                f"{ranking.DOC_BEGIN}\nstale\n{ranking.DOC_END}\n",
                encoding="utf-8",
            )
            current, expected = ranking.expected_document_text(
                self.document, path
            )
            self.assertNotEqual(current, expected)
            path.write_text(expected, encoding="utf-8")
            current, expected = ranking.expected_document_text(
                self.document, path
            )
            self.assertEqual(current, expected)


if __name__ == "__main__":
    unittest.main()
