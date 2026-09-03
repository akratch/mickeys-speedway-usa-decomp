#!/usr/bin/env python3
"""Focused regression tests for tools/experiment_ledger.py."""

from __future__ import annotations

from contextlib import redirect_stderr, redirect_stdout
import hashlib
import io
import json
import os
from pathlib import Path
import struct
import tempfile
import unittest
from unittest import mock

import experiment_ledger as ledger


def _align(value: int, alignment: int = 4) -> int:
    return (value + alignment - 1) & -alignment


def candidate_elf(
    *,
    function_text: bytes | None = None,
    unrelated_text: bytes = b"unrelated function",
    relocation_target: str = "external_target",
    relocation_type: int = ledger.R_MIPS_26,
    relocation_offsets: tuple[int, ...] = (4, 8, 12, 16, 20, 24, 28),
    relocation_types: tuple[int, ...] | None = None,
    function_size: int | None = None,
    overlapping_symbol: bool = False,
    duplicate_definition: bool = False,
) -> bytes:
    """Build one minimal big-endian MIPS ELF32 object without invoking a compiler."""

    if function_text is None:
        body = bytearray(80 * 4)
        for offset in relocation_offsets:
            struct.pack_into(">I", body, offset, 0x0C000000)
        function_text = bytes(body)
    declared_size = len(function_text) if function_size is None else function_size
    text = function_text + unrelated_text
    types = (
        (relocation_type,) * len(relocation_offsets)
        if relocation_types is None
        else relocation_types
    )
    if len(types) != len(relocation_offsets):
        raise ValueError("relocation_types must align with relocation_offsets")

    symbol_names = ["", "func_80001234", "second_symbol", relocation_target]
    if overlapping_symbol:
        symbol_names.append("overlapping_symbol")
    strtab = bytearray(b"\0")
    string_offsets: dict[str, int] = {"": 0}
    for name in symbol_names[1:]:
        string_offsets[name] = len(strtab)
        strtab.extend(name.encode("ascii") + b"\0")

    symbols = [struct.pack(">IIIBBH", 0, 0, 0, 0, 0, 0)]
    symbols.append(
        struct.pack(
            ">IIIBBH", string_offsets["func_80001234"], 0, declared_size, 0x12, 0, 1
        )
    )
    symbols.append(
        struct.pack(
            ">IIIBBH",
            string_offsets["second_symbol"],
            len(function_text),
            len(unrelated_text),
            0x12,
            0,
            1,
        )
    )
    target_index = len(symbols)
    symbols.append(
        struct.pack(
            ">IIIBBH", string_offsets[relocation_target], 0, 0, 0x10, 0, 0
        )
    )
    if overlapping_symbol:
        symbols.append(
            struct.pack(
                ">IIIBBH", string_offsets["overlapping_symbol"], 4, 8, 0x12, 0, 1
            )
        )
    if duplicate_definition:
        symbols.append(
            struct.pack(
                ">IIIBBH",
                string_offsets["func_80001234"],
                0,
                declared_size,
                0x12,
                0,
                1,
            )
        )
    symtab = b"".join(symbols)
    relocations = b"".join(
        struct.pack(">II", offset, (target_index << 8) | row_type)
        for offset, row_type in zip(relocation_offsets, types)
    )

    section_names = b"\0.text\0.rel.text\0.symtab\0.strtab\0.shstrtab\0"
    section_name_offsets = {
        name: section_names.index(name.encode("ascii"))
        for name in (".text", ".rel.text", ".symtab", ".strtab", ".shstrtab")
    }
    cursor = 52
    content: list[tuple[int, bytes]] = []
    for blob in (text, relocations, symtab, bytes(strtab), section_names):
        cursor = _align(cursor)
        content.append((cursor, blob))
        cursor += len(blob)
    section_offset = _align(cursor)
    output = bytearray(section_offset + 6 * 40)
    ident = b"\x7fELF\x01\x02\x01" + b"\0" * 9
    struct.pack_into(
        ">16sHHIIIIIHHHHHH",
        output,
        0,
        ident,
        ledger.ET_REL,
        8,
        1,
        0,
        0,
        section_offset,
        0,
        52,
        0,
        0,
        40,
        6,
        5,
    )
    for offset, blob in content:
        output[offset : offset + len(blob)] = blob
    text_row, reloc_row, symtab_row, strtab_row, names_row = content
    headers = [
        (0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
        (section_name_offsets[".text"], 1, 6, 0, text_row[0], len(text), 0, 0, 4, 0),
        (
            section_name_offsets[".rel.text"],
            ledger.SHT_REL,
            0,
            0,
            reloc_row[0],
            len(relocations),
            3,
            1,
            4,
            8,
        ),
        (
            section_name_offsets[".symtab"],
            ledger.SHT_SYMTAB,
            0,
            0,
            symtab_row[0],
            len(symtab),
            4,
            1,
            4,
            16,
        ),
        (section_name_offsets[".strtab"], 3, 0, 0, strtab_row[0], len(strtab), 0, 0, 1, 0),
        (
            section_name_offsets[".shstrtab"],
            3,
            0,
            0,
            names_row[0],
            len(section_names),
            0,
            0,
            1,
            0,
        ),
    ]
    for index, header in enumerate(headers):
        struct.pack_into(">10I", output, section_offset + index * 40, *header)
    return bytes(output)


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
            "schema_version": ledger.LEGACY_SCHEMA_VERSION,
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
                "candidate_identity_resolved_count": 7,
                "candidate_identity_unresolved_records": [],
            },
            "preflight": {
                "status": "complete",
                "counts": {
                    "candidate_static_relocations": 7,
                    "candidate_identities_resolved": 7,
                    "candidate_identities_unresolved": 0,
                },
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

    def write_candidate(
        self,
        payload: bytes | None = None,
        relative: str = "build/src/main/example.c.o",
    ) -> Path:
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(candidate_elf() if payload is None else payload)
        return path

    def preflight_command(
        self,
        *,
        ledger_path: str = "build/fingerprints.jsonl",
        hypothesis: str = "Narrow local lifetime around the conditional",
    ) -> list[str]:
        return [
            "--ledger",
            ledger_path,
            "append",
            "func_80001234",
            "--preflight-json",
            "build/preflight.json",
            "--hypothesis",
            hypothesis,
            "--verdict",
            "attempt",
            "--json",
        ]

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
        self.assertEqual(
            ledger.LEGACY_SCHEMA_VERSION, json.loads(prefix)["schema_version"]
        )

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
            "Candidate bytes are " + r"\x27" + r"\xbd" + r"\xff" + r"\xe0",
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
        candidate.write_bytes(candidate_elf())
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
        self.assertEqual(ledger.SCHEMA_VERSION, record["schema_version"])
        self.assertEqual(
            {
                "algorithm",
                "sha256",
                "size",
                "relocations",
            },
            set(record["candidate_fingerprint"]),
        )
        self.assertEqual(320, record["candidate_fingerprint"]["size"])
        self.assertEqual(7, record["candidate_fingerprint"]["relocations"])

    def test_partial_preflight_accepts_only_absent_evidence_overrides(self) -> None:
        candidate = self.root / "build/src/main/example.c.o"
        candidate.parent.mkdir(parents=True)
        candidate.write_bytes(candidate_elf())
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
                "candidate_identity_resolved_count": 7,
                "candidate_identity_unresolved_records": [],
            },
            preflight={
                "status": "complete",
                "counts": {
                    "candidate_static_relocations": 7,
                    "candidate_identities_resolved": 7,
                    "candidate_identities_unresolved": 0,
                },
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

    def test_duplicate_candidate_fingerprint_is_rejected_without_mutation(self) -> None:
        candidate = self.root / "build/src/main/example.c.o"
        candidate.parent.mkdir(parents=True)
        candidate.write_bytes(candidate_elf())
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
        self.assertIn("duplicate candidate function fingerprint", error.getvalue())
        self.assertEqual(before, journal.read_bytes())
        self.assertEqual(inode, journal.stat().st_ino)

    def test_unrelated_translation_unit_change_keeps_function_fingerprint(self) -> None:
        candidate = self.write_candidate(candidate_elf(unrelated_text=b"first unrelated body"))
        first_object_digest = hashlib.sha256(candidate.read_bytes()).hexdigest()
        self.write_preflight(
            self.preflight_report(candidate_object="build/src/main/example.c.o")
        )
        command = self.preflight_command()
        first_output = io.StringIO()
        with mock.patch.object(ledger, "project_root", return_value=self.root):
            with redirect_stdout(first_output):
                self.assertEqual(0, ledger.main(command))
        first_record = json.loads(first_output.getvalue())[0]

        candidate.write_bytes(candidate_elf(unrelated_text=b"second unrelated body"))
        second_object_digest = hashlib.sha256(candidate.read_bytes()).hexdigest()
        self.assertNotEqual(first_object_digest, second_object_digest)
        self.write_preflight(
            self.preflight_report(candidate_object="build/src/main/example.c.o")
        )
        journal = self.root / "build/fingerprints.jsonl"
        before = journal.read_bytes()
        error = io.StringIO()
        with mock.patch.object(ledger, "project_root", return_value=self.root):
            with redirect_stderr(error), redirect_stdout(io.StringIO()):
                self.assertEqual(
                    2,
                    ledger.main(
                        self.preflight_command(
                            hypothesis="Extend the local lifetime across the conditional"
                        )
                    ),
                )
        self.assertIn("duplicate candidate function fingerprint", error.getvalue())
        self.assertEqual(before, journal.read_bytes())
        self.assertNotIn(str(self.root), json.dumps(first_record))

    def test_function_or_relocation_semantics_change_the_fingerprint(self) -> None:
        baseline = candidate_elf()
        changed_body = bytearray(baseline)
        # Locate the owned .text byte through the parser's own ELF section geometry.
        section_offset = struct.unpack_from(">I", baseline, 0x20)[0]
        text_file_offset = struct.unpack_from(">I", baseline, section_offset + 40 + 16)[0]
        changed_body[text_file_offset + 100] ^= 1

        baseline_fingerprint = ledger._fingerprint_candidate_function(
            baseline, "func_80001234", 80, 7
        )
        body_fingerprint = ledger._fingerprint_candidate_function(
            bytes(changed_body), "func_80001234", 80, 7
        )
        target_fingerprint = ledger._fingerprint_candidate_function(
            candidate_elf(relocation_target="different_target"),
            "func_80001234",
            80,
            7,
        )

        addend_body = bytearray(80 * 4)
        for offset in (4, 8, 12, 16, 20, 24, 28):
            struct.pack_into(">I", addend_body, offset, 0x0C000000)
        struct.pack_into(">I", addend_body, 4, 0x0C000001)
        addend_fingerprint = ledger._fingerprint_candidate_function(
            candidate_elf(function_text=bytes(addend_body)),
            "func_80001234",
            80,
            7,
        )

        fingerprints = {
            baseline_fingerprint["sha256"],
            body_fingerprint["sha256"],
            target_fingerprint["sha256"],
            addend_fingerprint["sha256"],
        }
        self.assertEqual(4, len(fingerprints))

    def test_hi16_lo16_addends_are_paired_and_unrelated_bytes_are_ignored(self) -> None:
        function = bytearray(80 * 4)
        struct.pack_into(">I", function, 4, 0x3C010001)
        struct.pack_into(">I", function, 8, 0x24218000)
        baseline = candidate_elf(
            function_text=bytes(function),
            relocation_offsets=(4, 8),
            relocation_types=(ledger.R_MIPS_HI16, ledger.R_MIPS_LO16),
        )
        unrelated = candidate_elf(
            function_text=bytes(function),
            unrelated_text=b"different sibling",
            relocation_offsets=(4, 8),
            relocation_types=(ledger.R_MIPS_HI16, ledger.R_MIPS_LO16),
        )
        changed_addend = bytearray(function)
        struct.pack_into(">I", changed_addend, 8, 0x24218004)
        changed = candidate_elf(
            function_text=bytes(changed_addend),
            relocation_offsets=(4, 8),
            relocation_types=(ledger.R_MIPS_HI16, ledger.R_MIPS_LO16),
        )
        baseline_digest = ledger._fingerprint_candidate_function(
            baseline, "func_80001234", 80, 2
        )["sha256"]
        self.assertEqual(
            baseline_digest,
            ledger._fingerprint_candidate_function(
                unrelated, "func_80001234", 80, 2
            )["sha256"],
        )
        self.assertNotEqual(
            baseline_digest,
            ledger._fingerprint_candidate_function(
                changed, "func_80001234", 80, 2
            )["sha256"],
        )

        unpaired = candidate_elf(
            function_text=bytes(function),
            relocation_offsets=(4,),
            relocation_types=(ledger.R_MIPS_HI16,),
        )
        with self.assertRaisesRegex(ledger.LedgerError, "unpaired MIPS HI16"):
            ledger._fingerprint_candidate_function(
                unpaired, "func_80001234", 80, 1
            )

    def test_fingerprint_fails_closed_on_boundary_and_relocation_ambiguity(self) -> None:
        rela_object = bytearray(candidate_elf())
        section_offset = struct.unpack_from(">I", rela_object, 0x20)[0]
        struct.pack_into(">I", rela_object, section_offset + 2 * 40 + 4, ledger.SHT_RELA)
        cases = {
            "word geometry": (candidate_elf(), 79, 7),
            "overlaps": (candidate_elf(overlapping_symbol=True), 80, 7),
            "duplicate definition": (
                candidate_elf(duplicate_definition=True),
                80,
                7,
            ),
            "relocation count": (candidate_elf(), 80, 6),
            "unsupported relocation": (
                candidate_elf(relocation_type=127),
                80,
                7,
            ),
            "RELA relocation section": (bytes(rela_object), 80, 7),
            "not ELF": (b"not an object", 80, 7),
        }
        for label, (payload, words, relocations) in cases.items():
            with self.subTest(label=label), self.assertRaises(ledger.LedgerError):
                ledger._fingerprint_candidate_function(
                    payload, "func_80001234", words, relocations
                )

    def test_preflight_unresolved_or_stale_evidence_refuses_without_journal(self) -> None:
        candidate = self.write_candidate()
        unresolved = self.preflight_report(
            candidate_object="build/src/main/example.c.o",
            relocation_comparison={
                "candidate_record_count": 7,
                "target_record_count": 7,
                "effective_identity_alignment_count": 6,
                "candidate_identity_resolved_count": 6,
                "candidate_identity_unresolved_records": [
                    {"offset": 4, "rtype": ledger.R_MIPS_26}
                ],
            },
            preflight={
                "status": "partial",
                "counts": {
                    "candidate_static_relocations": 7,
                    "candidate_identities_resolved": 6,
                    "candidate_identities_unresolved": 1,
                },
            },
        )
        self.write_preflight(unresolved)
        error = io.StringIO()
        with mock.patch.object(ledger, "project_root", return_value=self.root):
            with redirect_stderr(error), redirect_stdout(io.StringIO()):
                self.assertEqual(2, ledger.main(self.preflight_command()))
        self.assertIn("every static relocation identity", error.getvalue())
        self.assertFalse((self.root / "build/fingerprints.jsonl").exists())

        self.write_preflight(
            self.preflight_report(candidate_object="build/src/main/example.c.o")
        )
        report_mtime = (self.root / "build/preflight.json").stat().st_mtime_ns
        candidate.write_bytes(candidate_elf(unrelated_text=b"newer object"))
        # Some filesystems have coarse mtimes; force the ordering under test.
        os.utime(candidate, ns=(report_mtime + 1_000_000_000,) * 2)
        self.assertGreater(candidate.stat().st_mtime_ns, report_mtime)
        with mock.patch.object(ledger, "project_root", return_value=self.root):
            with redirect_stderr(error), redirect_stdout(io.StringIO()):
                self.assertEqual(2, ledger.main(self.preflight_command()))
        self.assertFalse((self.root / "build/fingerprints.jsonl").exists())

    def test_schema_v1_journal_remains_readable_and_guards_legacy_duplicates(self) -> None:
        candidate = self.write_candidate()
        digest = hashlib.sha256(candidate.read_bytes()).hexdigest()
        legacy = self.record(
            artifacts=[{"path": "build/src/main/example.c.o", "sha256": digest}]
        )
        ledger.append_record(self.path, legacy)
        self.assertEqual(
            ledger.LEGACY_SCHEMA_VERSION,
            ledger.load_records(self.path)[0]["schema_version"],
        )

        self.write_preflight(
            self.preflight_report(candidate_object="build/src/main/example.c.o")
        )
        error = io.StringIO()
        command = self.preflight_command(ledger_path="build/experiments.jsonl")
        with mock.patch.object(ledger, "project_root", return_value=self.root):
            with redirect_stderr(error), redirect_stdout(io.StringIO()):
                self.assertEqual(2, ledger.main(command))
        self.assertIn("duplicate legacy candidate artifact", error.getvalue())
        self.assertEqual(1, len(ledger.load_records(self.path)))

    def test_schema_v2_requires_consistent_bounded_fingerprint_metadata(self) -> None:
        fingerprint = ledger._fingerprint_candidate_function(
            candidate_elf(), "func_80001234", 80, 7
        )
        current = self.record(
            schema_version=ledger.SCHEMA_VERSION,
            candidate_fingerprint=fingerprint,
        )
        self.assertEqual(
            fingerprint, ledger.validate_record(current)["candidate_fingerprint"]
        )
        with self.assertRaisesRegex(ledger.LedgerError, "must equal candidate_words"):
            ledger.validate_record(
                dict(
                    current,
                    candidate_fingerprint=dict(fingerprint, size=316),
                )
            )
        with self.assertRaisesRegex(ledger.LedgerError, "must equal candidate_relocations"):
            ledger.validate_record(
                dict(
                    current,
                    candidate_fingerprint=dict(fingerprint, relocations=6),
                )
            )
        with self.assertRaisesRegex(ledger.LedgerError, "missing candidate_fingerprint"):
            without_fingerprint = dict(current)
            without_fingerprint.pop("candidate_fingerprint")
            ledger.validate_record(without_fingerprint)

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
