#!/usr/bin/env python3
"""Focused tests for fail-closed ranking persistence and documentation."""

from __future__ import annotations

import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

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


def queue_item(file_name: str, symbol: str) -> object:
    return ranking.pb.QueueItem(
        func=symbol,
        c_file=ranking.ROOT / file_name,
    )


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

    def test_transactional_writer_rolls_back_first_replacement(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            first = Path(directory) / "first.txt"
            second = Path(directory) / "second.txt"
            first.write_text("old first\n", encoding="utf-8")
            second.write_text("old second\n", encoding="utf-8")
            original_replace = Path.replace
            calls = 0

            def fail_second_replace(path: Path, target: Path) -> Path:
                nonlocal calls
                calls += 1
                if calls == 2:
                    raise OSError("injected replacement failure")
                return original_replace(path, target)

            with mock.patch.object(Path, "replace", fail_second_replace):
                with self.assertRaisesRegex(OSError, "injected"):
                    ranking.write_texts_transactionally([
                        (first, "new first\n"),
                        (second, "new second\n"),
                    ])
            self.assertEqual(first.read_text(encoding="utf-8"), "old first\n")
            self.assertEqual(second.read_text(encoding="utf-8"), "old second\n")
            self.assertEqual(list(Path(directory).glob(".*.tmp")), [])


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

    def test_schema_two_context_coverage_is_derived_and_validated(self) -> None:
        row = function_row("src/main/file.c", "symbol")
        row[ranking.SOURCE_CONTEXT_FIELD] = ranking.group_source_context("A" * 43)
        document = ranking.make_ranking_document(
            [row], [], objdiff_report_used=False
        )
        self.assertEqual(document["schema_version"], 2)
        self.assertEqual(document["source_context_coverage"], 1)
        document["source_context_coverage"] = 0
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "source_context_coverage"
        ):
            ranking.validate_ranking_document(document)

    def test_malformed_context_digest_fails_closed(self) -> None:
        row = function_row("src/main/file.c", "symbol")
        row[ranking.SOURCE_CONTEXT_FIELD] = "not-a-digest"
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "SHA-256 encoding"
        ):
            ranking.validate_ranking_document(ranking_document([row]))


class SourceContextTests(unittest.TestCase):
    def test_context_digest_is_cleanroom_safe_base64url(self) -> None:
        text = """#ifdef NON_MATCHING
void a(void) {}
#else
#pragma GLOBAL_ASM(\"asm/a.s\")
#endif
"""
        digest = ranking.source_context_digest(text, "a")
        self.assertIsNotNone(digest)
        self.assertRegex(
            digest or "", r"^(?:[A-Za-z0-9_-]{4}\.){10}[A-Za-z0-9_-]{3}$"
        )

    def test_legacy_hex_context_normalizes_without_changing_evidence(self) -> None:
        legacy = "00" * 32
        self.assertEqual(
            ranking.normalize_source_context_digest(legacy),
            ranking.group_source_context("A" * 43),
        )

    def test_ignores_comments_and_other_candidates_but_covers_shared_context(self) -> None:
        original = """extern int shared;
#ifdef NON_MATCHING
void a(void) { shared++; }
#else
#pragma GLOBAL_ASM("asm/a.s")
#endif
#ifdef NON_MATCHING
void b(void) { shared++; }
#else
#pragma GLOBAL_ASM("asm/b.s")
#endif
"""
        changed_other = original.replace(
            "void b(void) { shared++; }", "void b(void) { shared += 2; }"
        ).replace("extern int", "/* note */\nextern int")
        changed_shared = original.replace("extern int shared", "extern short shared")
        baseline = ranking.source_context_digest(original, "a")
        self.assertEqual(
            baseline, ranking.source_context_digest(changed_other, "a")
        )
        self.assertNotEqual(
            baseline, ranking.source_context_digest(changed_shared, "a")
        )

    def test_context_blame_supersedes_legacy_measurement_blame(self) -> None:
        lines = [
            ("1" * 40, '      "name": "func",'),
            ("1" * 40, '      "file": "src/main/file.c",'),
            ("2" * 40, '      "differing_words": 3,'),
            (
                "3" * 40,
                f'      "{ranking.SOURCE_CONTEXT_FIELD}": "'
                + ranking.group_source_context("A" * 43)
                + '",',
            ),
        ]
        with mock.patch.object(
            ranking, "blamed_source_lines", return_value=lines
        ):
            commits = ranking.ranking_evidence_commits("HEAD", "ranking.json")
        self.assertEqual(
            commits[("src/main/file.c", "func")], "3" * 40
        )


class RetainedDisplayTests(unittest.TestCase):
    def test_recovers_validated_rows_without_source_or_compile_state(self) -> None:
        document = ranking_document([
            function_row(
                "src/main/file.c",
                "func",
                category="register-only",
                differing_words=2,
                pct=98.5,
            )
        ])
        results = ranking.retained_results(document)
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0].name, "func")
        self.assertEqual(results[0].differing_words, 2)
        self.assertEqual(results[0].objdiff_match_pct, 98.5)

    def test_rejects_an_invalid_snapshot(self) -> None:
        document = ranking_document()
        document["queue_size"] = 99
        with self.assertRaises(ranking.RankingDocumentError):
            ranking.retained_results(document)

    def test_presentation_flag_alone_cannot_start_a_full_compile(self) -> None:
        with mock.patch.object(sys, "argv", ["nm_ranking.py", "--top", "1"]):
            with mock.patch.object(
                ranking,
                "missing_permuter_inputs",
                side_effect=AssertionError("compile path reached"),
            ):
                self.assertEqual(ranking.main(), 2)


class IncrementalRefreshTests(unittest.TestCase):
    def setUp(self) -> None:
        self.keep = ("src/main/keep.c", "keep")
        self.legacy = ("src/main/legacy.c", "legacy")
        self.stale = ("src/main/stale.c", "stale")
        self.pending = ("src/main/pending.c", "pending")
        self.new = ("src/main/new.c", "new")
        self.removed = ("src/main/removed.c", "removed")
        keep_row = function_row(*self.keep, differing_words=1)
        keep_row[ranking.SOURCE_CONTEXT_FIELD] = ranking.group_source_context("A" * 43)
        self.document = ranking_document(
            [
                keep_row,
                function_row(*self.legacy, differing_words=2),
                function_row(*self.stale, differing_words=3),
                function_row(*self.removed, differing_words=4),
            ],
            [[[*self.pending], "old compile failure"]],
        )
        self.items = [
            queue_item(*key)
            for key in (self.keep, self.legacy, self.stale, self.pending, self.new)
        ]
        self.contexts = {
            self.keep: ranking.group_source_context("A" * 43),
            self.legacy: ranking.group_source_context("B" * 43),
            self.stale: ranking.group_source_context("C" * 43),
            self.pending: ranking.group_source_context("D" * 43),
            self.new: ranking.group_source_context("E" * 43),
        }

    def test_plan_migrates_legacy_proof_and_bounds_only_compile_work(self) -> None:
        plan = ranking.plan_incremental_refresh(
            self.document,
            self.items,
            self.contexts,
            {
                self.legacy: ranking.group_source_context("B" * 43),
                self.stale: ranking.group_source_context("F" * 43),
            },
            limit=2,
        )
        self.assertEqual(set(plan.fresh_rows), {self.keep, self.legacy})
        self.assertEqual(
            [(item.rel_c_file, item.func) for item in plan.selected],
            [self.stale, self.pending],
        )
        self.assertEqual(plan.removed, [self.removed])
        self.assertEqual(set(plan.deferred_unresolved), {self.new})
        self.assertEqual(plan.stale_count, 2)
        self.assertEqual(plan.new_count, 1)

    def test_merge_preserves_fresh_and_deferred_rows_and_exact_live_queue(self) -> None:
        plan = ranking.plan_incremental_refresh(
            self.document,
            self.items,
            self.contexts,
            {self.legacy: ranking.group_source_context("B" * 43)},
            limit=1,
        )
        selected = plan.selected[0]
        result = ranking.FuncResult(
            name=selected.func,
            file=selected.rel_c_file,
            overlay=None,
            tu="main",
            size_bytes=16,
            differing_words=1,
            first_mismatch_offset=4,
            size_delta=0,
            category="register-only",
        )
        merged = ranking.merge_incremental_results(
            self.document,
            plan,
            [result],
            objdiff_report_used=False,
        )
        keys = {
            (row["file"], row["name"]) for row in merged["functions"]
        } | {
            tuple(row[0]) for row in merged["unresolved_functions"]
        }
        self.assertEqual(keys, set(self.contexts))
        self.assertEqual(merged["source_context_coverage"], 3)
        stale_row = next(
            row for row in merged["functions"] if row["name"] == "stale"
        )
        self.assertEqual(
            stale_row[ranking.SOURCE_CONTEXT_FIELD],
            ranking.group_source_context("C" * 43),
        )

    def test_cli_item_error_leaves_input_byte_identical(self) -> None:
        document = ranking_document(
            unresolved=[[[*self.pending], "old compile failure"]]
        )
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "ranking.json"
            original = json.dumps(document, indent=2) + "\n"
            path.write_text(original, encoding="utf-8")
            item = queue_item(*self.pending)
            argv = [
                "nm_ranking.py", "--refresh-stale", "--out", str(path),
                "--no-table", "--jobs", "1",
            ]
            with mock.patch.object(sys, "argv", argv), mock.patch.object(
                ranking.pb, "discover_queue", return_value=[item]
            ), mock.patch.object(
                ranking, "current_source_contexts",
                return_value={
                    self.pending: ranking.group_source_context("D" * 43)
                },
            ), mock.patch.object(
                ranking, "legacy_source_contexts", return_value={}
            ), mock.patch.object(
                ranking, "missing_permuter_inputs", return_value=[]
            ), mock.patch.object(
                ranking, "process_items",
                return_value=([], [(self.pending, "compile failed")]),
            ):
                self.assertEqual(ranking.main(), 2)
            self.assertEqual(path.read_text(encoding="utf-8"), original)


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
