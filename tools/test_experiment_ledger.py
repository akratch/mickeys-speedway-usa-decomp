#!/usr/bin/env python3
"""Focused regression tests for tools/experiment_ledger.py."""

from __future__ import annotations

from contextlib import redirect_stderr, redirect_stdout
import hashlib
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

import experiment_ledger as ledger


class ExperimentLedgerTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        (self.root / "src/main").mkdir(parents=True)
        (self.root / "src/main/example.c").write_text(
            "void func_80001234(void) {}\nvoid second_symbol(void) {}\n", encoding="utf-8"
        )
        self.path = ledger.resolve_ledger_path(self.root, "build/experiments.jsonl")

    def record(self, **changes: object) -> dict[str, object]:
        record: dict[str, object] = {
            "schema_version": ledger.SCHEMA_VERSION,
            "timestamp": "2026-08-30T12:00:00Z",
            "symbol": "func_80001234",
            "source": "src/main/example.c",
            "hypothesis": "Narrow local lifetime around the conditional",
            "candidate_words": 80,
            "target_words": 77,
            "raw_differences": 12,
            "relocation_masked_differences": 10,
            "frame": 48,
            "candidate_relocations": 7,
            "target_relocations": 7,
            "relocation_identities": 6,
            "first_raw_mismatch": 8,
            "first_masked_mismatch": 32,
            "verdict": "attempt",
        }
        record.update(changes)
        return record

    def preflight_report(self, **changes: object) -> dict[str, object]:
        report: dict[str, object] = {
            "schema": ledger.PREFLIGHT_SCHEMA,
            "requested_symbol": "func_80001234",
            "target_symbol": "func_80001234",
            "candidate_symbol": "func_80001234",
            "source": "src/main/example.c",
            "candidate_object": "build_non_matching/src/main/example.c.o",
            "workbench": {
                "candidate_words": 80,
                "target_words": 77,
                "differing_words": 12,
                "first_mismatch": "+0x8",
                "candidate_frame": 48,
            },
            "relocation_comparison": {
                "status": "partial",
                "diagnostic": "additive future detail is ignored",
                "candidate_record_count": 7,
                "target_record_count": 7,
                "effective_identity_alignment_count": 6,
            },
        }
        report.update(changes)
        return report

    def write_preflight(
        self, report: dict[str, object], name: str = "preflight.json"
    ) -> Path:
        path = self.root / "build" / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(report), encoding="utf-8")
        return path

    def test_append_preserves_existing_bytes_and_schema(self) -> None:
        first = ledger.append_record(self.path, self.record())
        prefix = self.path.read_bytes()
        inode = self.path.stat().st_ino
        second_record = self.record(
            timestamp="2026-08-30T12:01:00Z",
            raw_differences=9,
            relocation_masked_differences=7,
            first_raw_mismatch=16,
            first_masked_mismatch=36,
            verdict="improved",
        )
        second = ledger.append_record(self.path, second_record)
        payload = self.path.read_bytes()

        self.assertTrue(payload.startswith(prefix))
        self.assertEqual(inode, self.path.stat().st_ino)
        self.assertEqual([first, second], ledger.load_records(self.path))
        self.assertEqual(2, payload.count(b"\n"))
        self.assertEqual(ledger.SCHEMA_VERSION, json.loads(prefix)["schema_version"])

    def test_new_append_uses_one_write_and_fsyncs_file_and_directory(self) -> None:
        with mock.patch.object(ledger.os, "write", wraps=ledger.os.write) as write_call:
            with mock.patch.object(ledger.os, "fsync", wraps=ledger.os.fsync) as fsync_call:
                ledger.append_record(self.path, self.record())
        write_call.assert_called_once()
        self.assertEqual(2, fsync_call.call_count)

    def test_append_refuses_to_extend_corrupt_or_truncated_journal(self) -> None:
        self.path.parent.mkdir(parents=True)
        self.path.write_text('{"schema_version":1}', encoding="utf-8")
        before = self.path.read_bytes()
        with self.assertRaisesRegex(ledger.LedgerError, "truncated"):
            ledger.append_record(self.path, self.record())
        self.assertEqual(before, self.path.read_bytes())

    def test_strict_symbol_source_and_schema_validation(self) -> None:
        invalid_symbols = (
            "",
            "9bad",
            "func-name",
            "func_1;drop",
            "while",
            "caf\N{LATIN SMALL LETTER E WITH ACUTE}",
        )
        for symbol in invalid_symbols:
            with self.subTest(symbol=symbol), self.assertRaises(ledger.LedgerError):
                ledger.validate_record(self.record(symbol=symbol))

        unknown = self.record(target_instruction="prohibited")
        with self.assertRaisesRegex(ledger.LedgerError, "unknown target_instruction"):
            ledger.validate_record(unknown)

        with self.assertRaisesRegex(ledger.LedgerError, "not named by source"):
            ledger.require_source_ownership(
                self.root, "src/main/example.c", "missing_symbol"
            )

        with self.assertRaisesRegex(ledger.LedgerError, "absolute/traversal"):
            ledger.validate_record(self.record(source="../src/main/example.c"))

    def test_metric_invariants_and_exact_verdict(self) -> None:
        with self.assertRaisesRegex(ledger.LedgerError, "cannot exceed raw"):
            ledger.validate_record(self.record(relocation_masked_differences=13))
        with self.assertRaisesRegex(ledger.LedgerError, "aligned byte offset"):
            ledger.validate_record(self.record(first_raw_mismatch=3))
        with self.assertRaisesRegex(ledger.LedgerError, "must be null"):
            ledger.validate_record(
                self.record(
                    raw_differences=0,
                    relocation_masked_differences=0,
                    first_raw_mismatch=8,
                    first_masked_mismatch=None,
                )
            )
        with self.assertRaisesRegex(ledger.LedgerError, "exact verdict requires"):
            ledger.validate_record(self.record(verdict="exact"))

        exact = self.record(
            candidate_words=77,
            raw_differences=0,
            relocation_masked_differences=0,
            first_raw_mismatch=None,
            first_masked_mismatch=None,
            relocation_identities=7,
            verdict="exact",
        )
        self.assertEqual("exact", ledger.validate_record(exact)["verdict"])

    def test_best_prefers_exact_then_masked_raw_and_latest(self) -> None:
        records = [
            self.record(timestamp="2026-08-30T12:00:00Z", raw_differences=8,
                        relocation_masked_differences=5, first_masked_mismatch=16),
            self.record(timestamp="2026-08-30T12:01:00Z", raw_differences=9,
                        relocation_masked_differences=4, first_masked_mismatch=20,
                        verdict="plateau"),
            self.record(timestamp="2026-08-30T12:02:00Z", raw_differences=9,
                        relocation_masked_differences=4, first_masked_mismatch=20,
                        verdict="improved"),
        ]
        self.assertEqual("2026-08-30T12:02:00Z", ledger.best_records(records)[0]["timestamp"])

        exact = self.record(
            timestamp="2026-08-30T12:03:00Z",
            candidate_words=77,
            raw_differences=0,
            relocation_masked_differences=0,
            first_raw_mismatch=None,
            first_masked_mismatch=None,
            relocation_identities=7,
            verdict="exact",
        )
        self.assertEqual("exact", ledger.best_records(records + [exact])[0]["verdict"])

    def test_best_groups_symbols_and_summary_counts(self) -> None:
        records = [
            self.record(),
            self.record(symbol="second_symbol", timestamp="2026-08-30T12:01:00Z",
                        raw_differences=3, relocation_masked_differences=2,
                        first_raw_mismatch=12, first_masked_mismatch=16,
                        verdict="improved"),
        ]
        self.assertEqual(["func_80001234", "second_symbol"],
                         [record["symbol"] for record in ledger.best_records(records)])
        summary = ledger.summarize_records(records)
        self.assertEqual(2, summary["records"])
        self.assertEqual(2, summary["symbols"])
        self.assertEqual({"attempt": 1, "improved": 1}, summary["verdicts"])

    def test_refuses_instruction_text_words_and_paths_without_appending(self) -> None:
        unsafe = (
            "Try addiu before the branch",
            "Candidate contains 0x27bdffe0",
            r"Candidate bytes are \x27\xbd\xff\xe0",
            "Candidate decimal word is 666894304",
            "Try or $8, $9, $10",
            "Compare against /private/object.o",
            r"Compare against C:\\private\\object.o",
            "Use src/main/private.c as a donor",
        )
        for hypothesis in unsafe:
            with self.subTest(hypothesis=hypothesis):
                with self.assertRaises(ledger.LedgerError):
                    ledger.append_record(self.path, self.record(hypothesis=hypothesis))
        self.assertFalse(self.path.exists())

    def test_refuses_absolute_and_traversing_artifact_paths(self) -> None:
        digest = hashlib.sha256(b"candidate").hexdigest()
        for path in ("/tmp/candidate.o", "build/../candidate.o", "tmp/candidate.o"):
            with self.subTest(path=path), self.assertRaises(ledger.LedgerError):
                ledger.validate_record(self.record(artifacts=[{"path": path, "sha256": digest}]))
        valid = ledger.validate_record(
            self.record(artifacts=[{"path": "build/wb/candidate.o", "sha256": digest}])
        )
        self.assertEqual(digest, valid["artifacts"][0]["sha256"])

        nonmatching = ledger.validate_record(
            self.record(
                artifacts=[
                    {"path": "build_non_matching/src/main/example.c.o", "sha256": digest}
                ]
            )
        )
        self.assertEqual("build_non_matching/src/main/example.c.o",
                         nonmatching["artifacts"][0]["path"])

    def test_ledger_destination_is_untracked_and_symlink_free(self) -> None:
        with self.assertRaisesRegex(ledger.LedgerError, "below build"):
            ledger.resolve_ledger_path(self.root, "docs/experiments.jsonl")
        with self.assertRaisesRegex(ledger.LedgerError, "normalized, relative"):
            ledger.resolve_ledger_path(self.root, self.root / "build/experiments.jsonl")

        (self.root / "build").symlink_to(self.root / "elsewhere", target_is_directory=True)
        with self.assertRaisesRegex(ledger.LedgerError, "symlinks"):
            ledger.resolve_ledger_path(self.root, "build/experiments.jsonl")

    def test_cli_append_list_best_and_summarize_round_trip(self) -> None:
        commands = [
            [
                "--ledger", "build/cli.jsonl", "append", "func_80001234",
                "--source", "src/main/example.c",
                "--hypothesis", "Narrow local lifetime around the conditional",
                "--candidate-words", "80", "--target-words", "77",
                "--raw-differences", "12", "--masked-differences", "10",
                "--frame", "0x30", "--candidate-relocations", "7",
                "--target-relocations", "7", "--relocation-identities", "6",
                "--first-raw-mismatch", "+0x8",
                "--first-masked-mismatch", "+0x20", "--verdict", "attempt",
            ],
            ["--ledger", "build/cli.jsonl", "list", "--json"],
            ["--ledger", "build/cli.jsonl", "best", "func_80001234", "--json"],
            ["--ledger", "build/cli.jsonl", "summarize", "--json"],
        ]
        outputs: list[str] = []
        with mock.patch.object(ledger, "project_root", return_value=self.root):
            for command in commands:
                stream = io.StringIO()
                with redirect_stdout(stream):
                    self.assertEqual(0, ledger.main(command))
                outputs.append(stream.getvalue())

        self.assertEqual("func_80001234", json.loads(outputs[1])[0]["symbol"])
        self.assertEqual("attempt", json.loads(outputs[2])[0]["verdict"])
        self.assertEqual(1, json.loads(outputs[3])["records"])

    def test_cli_ingests_current_preflight_and_hashes_candidate_object(self) -> None:
        candidate = self.root / "build_non_matching/src/main/example.c.o"
        candidate.parent.mkdir(parents=True)
        candidate.write_bytes(b"candidate object bytes")
        self.write_preflight(self.preflight_report())

        command = [
            "--ledger", "build/ingested.jsonl", "append", "func_80001234",
            "--preflight-json", "build/preflight.json",
            "--hypothesis", "Narrow local lifetime around the conditional",
            "--verdict", "attempt", "--json",
        ]
        output = io.StringIO()
        with mock.patch.object(ledger, "project_root", return_value=self.root):
            with redirect_stdout(output):
                self.assertEqual(0, ledger.main(command))

        record = json.loads(output.getvalue())[0]
        self.assertEqual(80, record["candidate_words"])
        self.assertEqual(12, record["raw_differences"])
        self.assertEqual(12, record["relocation_masked_differences"])
        self.assertEqual(8, record["first_raw_mismatch"])
        self.assertEqual(8, record["first_masked_mismatch"])
        self.assertEqual(48, record["frame"])
        self.assertEqual(7, record["candidate_relocations"])
        self.assertEqual(6, record["relocation_identities"])
        self.assertEqual(
            hashlib.sha256(candidate.read_bytes()).hexdigest(),
            record["artifacts"][0]["sha256"],
        )
        self.assertEqual(
            "build_non_matching/src/main/example.c.o",
            record["artifacts"][0]["path"],
        )

    def test_partial_preflight_accepts_only_absent_evidence_overrides(self) -> None:
        candidate = self.root / "build/src/main/example.c.o"
        candidate.parent.mkdir(parents=True)
        candidate.write_bytes(b"candidate")
        report = self.preflight_report(
            candidate_object="build/src/main/example.c.o",
            workbench={
                "candidate_words": 80,
                "target_words": 77,
                "differing_words": 12,
                "relocation_masked_differing_words": 4,
                "first_mismatch": "+0x8",
                "candidate_frame": None,
                "status": "partial",
                "diagnostic": {"reason": "frame unavailable"},
            },
            relocation_comparison={
                "status": "partial",
                "candidate_record_count": 7,
            },
        )
        self.write_preflight(report)
        command = [
            "--ledger", "build/partial.jsonl", "append", "func_80001234",
            "--preflight-json", "build/preflight.json",
            "--hypothesis", "Narrow local lifetime around the conditional",
            "--frame", "0x30", "--target-relocations", "8",
            "--relocation-identities", "5", "--first-masked-mismatch", "+0x20",
            "--verdict", "attempt", "--json",
        ]
        output = io.StringIO()
        with mock.patch.object(ledger, "project_root", return_value=self.root):
            with redirect_stdout(output):
                self.assertEqual(0, ledger.main(command))
        record = json.loads(output.getvalue())[0]
        self.assertEqual(7, record["candidate_relocations"])
        self.assertEqual(8, record["target_relocations"])
        self.assertEqual(5, record["relocation_identities"])
        self.assertEqual(4, record["relocation_masked_differences"])
        self.assertEqual(32, record["first_masked_mismatch"])

        before = (self.root / "build/partial.jsonl").read_bytes()
        conflicting = command[:-3] + ["--candidate-words", "79"] + command[-3:]
        with mock.patch.object(ledger, "project_root", return_value=self.root):
            with redirect_stderr(io.StringIO()), redirect_stdout(io.StringIO()):
                self.assertEqual(2, ledger.main(conflicting))
        self.assertEqual(before, (self.root / "build/partial.jsonl").read_bytes())

    def test_duplicate_candidate_artifact_is_rejected_without_mutation(self) -> None:
        candidate = self.root / "build/src/main/example.c.o"
        candidate.parent.mkdir(parents=True)
        candidate.write_bytes(b"same compiler output")
        report = self.preflight_report(candidate_object="build/src/main/example.c.o")
        self.write_preflight(report)
        base = [
            "--ledger", "build/duplicates.jsonl", "append", "func_80001234",
            "--preflight-json", "build/preflight.json", "--verdict", "attempt",
        ]
        with mock.patch.object(ledger, "project_root", return_value=self.root):
            with redirect_stdout(io.StringIO()):
                self.assertEqual(
                    0,
                    ledger.main(base + [
                        "--hypothesis", "Narrow local lifetime around the conditional"
                    ]),
                )

        journal = self.root / "build/duplicates.jsonl"
        before = journal.read_bytes()
        inode = journal.stat().st_ino
        error = io.StringIO()
        with mock.patch.object(ledger, "project_root", return_value=self.root):
            with redirect_stderr(error), redirect_stdout(io.StringIO()):
                self.assertEqual(
                    2,
                    ledger.main(base + [
                        "--hypothesis", "Extend the local lifetime across the conditional"
                    ]),
                )
        self.assertIn("duplicate candidate artifact", error.getvalue())
        self.assertEqual(before, journal.read_bytes())
        self.assertEqual(inode, journal.stat().st_ino)

    def test_preflight_refuses_nonlocal_candidate_object_without_appending(self) -> None:
        self.write_preflight(self.preflight_report(candidate_object="../private.o"))
        command = [
            "--ledger", "build/invalid.jsonl", "append", "func_80001234",
            "--preflight-json", "build/preflight.json",
            "--hypothesis", "Narrow local lifetime around the conditional",
            "--verdict", "attempt",
        ]
        with mock.patch.object(ledger, "project_root", return_value=self.root):
            with redirect_stderr(io.StringIO()), redirect_stdout(io.StringIO()):
                self.assertEqual(2, ledger.main(command))
        self.assertFalse((self.root / "build/invalid.jsonl").exists())


if __name__ == "__main__":
    unittest.main()
