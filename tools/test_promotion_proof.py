#!/usr/bin/env python3
"""Focused tests for the compact post-promotion proof policy."""

from __future__ import annotations

import copy
import contextlib
import io
import json
import subprocess
import sys
import unittest
from pathlib import Path


TOOLS = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOLS))
import promotion_proof as proof  # noqa: E402


def exact_report() -> dict[str, object]:
    return {
        "schema": "mickey-function-evidence-preflight-v1",
        "requested_symbol": "friendly",
        "candidate_symbol": "friendly",
        "linked_symbol": "friendly",
        "resolution_mode": "post_promotion",
        "workbench": {
            "comparison_mode": "rom",
            "differing_words": 0,
            "target_words": 32,
            "candidate_words": 32,
            "first_mismatch": None,
            "verdict": "exact",
            "target_frame": 48,
            "candidate_frame": 48,
        },
        "relocation_comparison": {
            "target_record_count": 3,
            "candidate_record_count": 3,
            "offset_type_exact": True,
            "effective_identity_alignment_count": 3,
            "effective_identity_exact": True,
            "identity_proof_mode": "static-plus-runtime-table-and-linked-rom",
        },
    }


class ReportPolicyTests(unittest.TestCase):
    def test_exact_post_promotion_report_becomes_compact_receipt(self) -> None:
        receipt = proof.validate_report("friendly", exact_report())

        self.assertEqual("mickey-promotion-proof-v1", receipt["schema"])
        self.assertEqual("exact", receipt["verdict"])
        self.assertEqual(32, receipt["exact_words"])
        self.assertEqual(48, receipt["frame_size"])
        self.assertEqual(3, receipt["exact_relocations"])

    def test_leaf_function_with_no_frame_or_relocations_can_be_exact(self) -> None:
        report = exact_report()
        report["workbench"]["target_frame"] = None
        report["workbench"]["candidate_frame"] = None
        report["relocation_comparison"].update(
            {
                "target_record_count": 0,
                "candidate_record_count": 0,
                "effective_identity_alignment_count": 0,
                "identity_proof_mode": "static",
            }
        )

        receipt = proof.validate_report("friendly", report)

        self.assertIsNone(receipt["frame_size"])
        self.assertEqual(0, receipt["exact_relocations"])

    def test_workbench_native_exact_word_verdict_is_accepted(self) -> None:
        report = exact_report()
        report["workbench"]["verdict"] = "instruction-words-identical"
        self.assertEqual("exact", proof.validate_report("friendly", report)["verdict"])

    def test_fallback_mode_is_not_a_promotion_proof(self) -> None:
        report = exact_report()
        report["resolution_mode"] = "fallback"
        with self.assertRaisesRegex(proof.ProofError, "not post-promotion"):
            proof.validate_report("friendly", report)

    def test_every_exactness_surface_fails_closed(self) -> None:
        mutations = (
            ("linked words", lambda row: row["workbench"].update(differing_words=1)),
            (
                "exact-word verdict",
                lambda row: row["workbench"].update(verdict="register-mismatch"),
            ),
            ("frame", lambda row: row["workbench"].update(candidate_frame=32)),
            (
                "relocation counts",
                lambda row: row["relocation_comparison"].update(candidate_record_count=2),
            ),
            (
                "offsets/types",
                lambda row: row["relocation_comparison"].update(offset_type_exact=False),
            ),
            (
                "identities",
                lambda row: row["relocation_comparison"].update(
                    effective_identity_exact=False
                ),
            ),
        )
        for message, mutate in mutations:
            with self.subTest(message=message):
                report = copy.deepcopy(exact_report())
                mutate(report)
                with self.assertRaisesRegex(proof.ProofError, message):
                    proof.validate_report("friendly", report)


class CommandTests(unittest.TestCase):
    def test_preflight_command_forwards_no_build_and_parses_json(self) -> None:
        calls: list[tuple[list[str], dict[str, object]]] = []

        def runner(command: list[str], **kwargs: object) -> subprocess.CompletedProcess[str]:
            calls.append((command, kwargs))
            return subprocess.CompletedProcess(command, 0, json.dumps(exact_report()), "")

        report = proof.run_preflight("friendly", no_build=True, runner=runner)

        self.assertEqual(exact_report(), report)
        self.assertEqual(
            [sys.executable, str(proof.PREFLIGHT), "friendly", "--json", "--no-build"],
            calls[0][0],
        )
        self.assertEqual(proof.REPO, calls[0][1]["cwd"])

    def test_canonical_commands_are_explicitly_niced_and_two_job(self) -> None:
        commands = [command for _label, command in proof.canonical_commands()]

        self.assertEqual(
            [
                ["nice", "-n", "10", "gmake", "-j2", "verify"],
                ["nice", "-n", "10", "gmake", "-j2", "check-overlay-syms"],
            ],
            commands,
        )

    def test_canonical_proofs_stop_on_first_failure(self) -> None:
        calls: list[list[str]] = []

        def runner(command: list[str], **_kwargs: object) -> subprocess.CompletedProcess[str]:
            calls.append(command)
            return subprocess.CompletedProcess(command, 1)

        with contextlib.redirect_stdout(io.StringIO()):
            with self.assertRaisesRegex(proof.ProofError, "full-ROM identity"):
                proof.run_canonical_proofs(runner=runner)
        self.assertEqual(1, len(calls))


if __name__ == "__main__":
    unittest.main()
