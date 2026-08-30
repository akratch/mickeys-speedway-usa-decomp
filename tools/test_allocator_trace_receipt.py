#!/usr/bin/env python3
"""Focused tests for allocator_trace_receipt.py."""

from __future__ import annotations

import argparse
import json
import sys
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import allocator_trace_receipt as receipt


SYMBOLS = [
    receipt.FunctionSymbol("helper_a", 0x00, 16, "1"),
    receipt.FunctionSymbol("target", 0x10, 32, "1"),
    receipt.FunctionSymbol("target_alias", 0x10, 32, "1"),
    receipt.FunctionSymbol("helper_b", 0x30, 8, "1"),
]

INDEX = """
[CDX] procindex proc=0 decisions=1
[CDX] procindex proc=1 decisions=2
[CDX] procindex proc=2 decisions=0
"""

DETAIL = """
[CDX] p1dec proc=1 web=7 class=1 decision=color bestcolor=3 bestreg=a0 forced=-2
[CDX] p2dec proc=1 web=8 class=2 decision=no-color bestreg=? forced=-2
"""


class AllocatorTraceReceiptTests(unittest.TestCase):
    def test_maps_alias_group_by_named_ucode_order(self) -> None:
        proc, group, count = receipt.map_symbol_to_procedure(
            "target_alias",
            SYMBOLS,
            receipt.parse_proc_index(INDEX),
            ["helper_a", "target_alias", "helper_b"],
        )
        self.assertEqual(proc, 1)
        self.assertEqual(group.names, ("target", "target_alias"))
        self.assertEqual(group.value, 0x10)
        self.assertEqual(group.end, 0x30)
        self.assertEqual(count, 3)

    def test_mapping_fails_closed_when_procedure_count_disagrees(self) -> None:
        short_index = receipt.parse_proc_index(
            "[CDX] procindex proc=0 decisions=1\n"
            "[CDX] procindex proc=1 decisions=2\n"
        )
        with self.assertRaisesRegex(receipt.ReceiptError, "cannot prove"):
            receipt.map_symbol_to_procedure(
                "target", SYMBOLS, short_index, ["helper_a", "target", "helper_b"]
            )

    def test_ucode_ent_name_payload_is_authoritative(self) -> None:
        payload = b"target\0\0"
        words = tuple(
            int.from_bytes(payload[index : index + 4], "big")
            for index in range(0, len(payload), 4)
        )
        records = [
            SimpleNamespace(name="ent", index=10, words=(0, 0, 0, 0)),
            SimpleNamespace(
                name="comm", index=11, words=(0, 0, 0, 0, 6, 0, *words)
            ),
        ]
        self.assertEqual(
            receipt.procedure_names_from_ucode_records(records), ["target"]
        )

    def test_detail_count_must_match_index(self) -> None:
        with self.assertRaisesRegex(receipt.ReceiptError, "requires 2"):
            receipt.parse_cdx_decisions(DETAIL.splitlines()[0], 1, 2)

    def test_decision_summary_separates_integer_and_fp_pools(self) -> None:
        rows = receipt.parse_cdx_decisions(DETAIL, 1, 2)
        summary = receipt.summarize_decisions(rows)
        self.assertEqual(summary["integer_pool"]["decisions"], 1)
        self.assertEqual(summary["integer_pool"]["registers"], {"a0": 1})
        self.assertEqual(summary["fp_pool"]["decisions"], 1)
        self.assertEqual(summary["fp_pool"]["outcomes"], {"no-color": 1})

    def test_ugen_summary_keeps_only_result_lanes(self) -> None:
        summary = receipt.summarize_ugen_results(
            "DKWB-FREELIST ALLOC_GP reg=96 emitted=1 line=2\n"
            "DKWB-FREELIST ALLOC_GP_RESULT reg=14 emitted=1 line=2\n"
            "DKWB-FREELIST ALLOC_FP_RESULT reg=36 emitted=2 line=3\n"
        )
        self.assertEqual(summary["integer_temps"]["registers"], {"t6": 1})
        self.assertEqual(summary["fp_temps"]["registers"], {"$f4": 1})

    def test_receipt_is_compact_and_omits_raw_trace_rows(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            candidate = root / "candidate.o"
            traced = root / "traced.o"
            index = root / "index.log"
            detail = root / "detail.log"
            ucode = root / "candidate.B"
            candidate.write_bytes(b"candidate")
            traced.write_bytes(b"traced")
            index.write_text(INDEX, encoding="utf-8")
            detail.write_text(DETAIL, encoding="utf-8")
            ucode.write_bytes(b"ucode")
            args = argparse.Namespace(
                symbol="target",
                candidate_object=str(candidate),
                traced_object=str(traced),
                index_trace=str(index),
                ucode_stream=str(ucode),
                uopt_trace=str(detail),
                ugen_trace=None,
                attempts=4,
                budget=10,
                objdump="objdump",
                workbench="workbench",
                json=False,
            )
            fidelity = {
                "pass": True,
                "gates": {
                    ".text": True,
                    ".rodata": True,
                    ".data": True,
                    "relocations": True,
                    "symbols": True,
                },
                "file_identical": False,
                "stock": {"file_sha256": "a" * 64},
                "instrumented": {"file_sha256": "b" * 64},
            }
            with mock.patch.object(
                receipt,
                "read_object_symbols",
                return_value=SYMBOLS,
            ), mock.patch.object(
                receipt,
                "read_ucode_procedure_names",
                return_value=["helper_a", "target", "helper_b"],
            ), mock.patch.object(receipt, "run_fidelity_gate", return_value=fidelity):
                result = receipt.build_receipt(args)

        rendered = receipt.render_text(result)
        self.assertEqual(result["procedure"]["ordinal"], 1)
        self.assertEqual(result["attempts"]["remaining"], 6)
        self.assertEqual(result["ugen"]["status"], "not-provided")
        self.assertEqual("unavailable", result["trace_summary"]["frame"]["status"])
        self.assertEqual(
            "unavailable", result["comparison"]["first_divergence"]["status"]
        )
        self.assertNotIn("[CDX]", rendered)
        self.assertNotIn("web=", rendered)
        self.assertLessEqual(len(rendered.splitlines()), 9)

    def test_multifunction_object_refuses_unscoped_ugen_trace(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            paths = {
                name: root / name
                for name in (
                    "candidate.o",
                    "traced.o",
                    "index.log",
                    "detail.log",
                    "candidate.B",
                    "ugen.log",
                )
            }
            paths["candidate.o"].write_bytes(b"candidate")
            paths["traced.o"].write_bytes(b"traced")
            paths["index.log"].write_text(INDEX, encoding="utf-8")
            paths["detail.log"].write_text(DETAIL, encoding="utf-8")
            paths["candidate.B"].write_bytes(b"ucode")
            paths["ugen.log"].write_text(
                "DKWB-FREELIST ALLOC_GP_RESULT reg=14 emitted=1 line=2\n",
                encoding="utf-8",
            )
            args = argparse.Namespace(
                symbol="target",
                candidate_object=str(paths["candidate.o"]),
                traced_object=str(paths["traced.o"]),
                index_trace=str(paths["index.log"]),
                ucode_stream=str(paths["candidate.B"]),
                uopt_trace=str(paths["detail.log"]),
                ugen_trace=str(paths["ugen.log"]),
                attempts=1,
                budget=1,
                objdump="objdump",
                workbench="workbench",
                json=False,
            )
            fidelity = {
                "pass": True,
                "gates": {},
                "file_identical": True,
                "stock": {"file_sha256": "a" * 64},
                "instrumented": {"file_sha256": "a" * 64},
            }
            with mock.patch.object(
                receipt,
                "read_object_symbols",
                return_value=SYMBOLS,
            ), mock.patch.object(
                receipt,
                "read_ucode_procedure_names",
                return_value=["helper_a", "target", "helper_b"],
            ), mock.patch.object(receipt, "run_fidelity_gate", return_value=fidelity):
                with self.assertRaisesRegex(receipt.ReceiptError, "no compiled-procedure identity"):
                    receipt.build_receipt(args)

    def test_stack_home_summary_preserves_explicit_width_access_and_source(self) -> None:
        detail = (
            "[CDX] webdetail phase=p2 proc=2 role=target web=15 sym=0 "
            "type=3 dtype=13 virtual_offset=-32 final_offset=24 width=4 "
            "access=load-store line=617 file=/private/workstation/candidate.c\n"
            "[CDX] p2dec phase=p2 proc=2 web=15 class=1 bestcolor=1 "
            "decision=color\n"
        )
        summary = receipt.summarize_stack_homes(
            detail, procedure=2, symbol="func_80050E9C"
        )
        self.assertEqual("available", summary["status"])
        self.assertEqual(
            {
                "offset": 24,
                "offset_basis": "final-frame",
                "width_bytes": 4,
                "access_class": "load-store",
                "kind": "compiler-temporary",
                "source": {
                    "procedure": "func_80050E9C",
                    "file": "candidate.c",
                    "line": 617,
                },
            },
            summary["homes"][0],
        )

    def test_func_80050e9c_style_extra_temp_birth_names_next_lever(self) -> None:
        candidate = receipt.summarize_ugen_results(
            "DKWB-FREELIST ALLOC_GP_RESULT reg=14 emitted=40 line=617\n"
            "DKWB-FREELIST ALLOC_GP_RESULT reg=15 emitted=41 line=617\n"
            "DKWB-FREELIST FREE reg=14 emitted=42 line=618\n",
            procedure="func_80050E9C",
        )
        target_events = {
            "status": "available",
            "events": [
                candidate["events"][0],
                {**candidate["events"][2], "ordinal": 1},
            ],
        }
        comparison = receipt.compare_trace_summary(
            frame={"candidate_bytes": 32, "target_bytes": 32},
            stack_homes={"status": "available", "homes": []},
            temp_events={"status": "available", "events": candidate["events"]},
            target={
                "frame_size_bytes": 32,
                "stack_homes": {"status": "available", "homes": []},
                "temp_events": target_events,
            },
        )
        first = comparison["first_divergence"]
        self.assertEqual("extra-temp-birth", first["mechanism"])
        self.assertEqual(617, first["evidence"]["candidate_event"]["source"]["line"])
        self.assertIn("attributed expression", first["next_lever"])

    def test_stack_home_displacement_precedes_later_allocator_fields(self) -> None:
        comparison = receipt.compare_trace_summary(
            frame={"candidate_bytes": 32, "target_bytes": 32},
            stack_homes={
                "status": "available",
                "homes": [
                    {
                        "offset": 24,
                        "width_bytes": 4,
                        "access_class": "store",
                    }
                ],
            },
            temp_events={"status": "available", "events": []},
            target={
                "frame_size_bytes": 32,
                "stack_homes": {
                    "status": "available",
                    "homes": [
                        {
                            "offset": 28,
                            "width_bytes": 4,
                            "access_class": "store",
                        }
                    ],
                },
                "temp_events": {"status": "available", "events": []},
            },
        )
        first = comparison["first_divergence"]
        self.assertEqual("stack-home-displacement", first["mechanism"])
        self.assertEqual(28, first["evidence"]["target_offset"])
        self.assertEqual(24, first["evidence"]["candidate_offset"])

    def test_no_divergence_requires_every_field_to_be_available_and_equal(self) -> None:
        event = {
            "ordinal": 0,
            "temp_class": "integer",
            "lifecycle": "birth",
            "ring_action": "pop",
            "source": {"procedure": "target", "line": 12},
        }
        homes = [
            {"offset": 16, "width_bytes": 4, "access_class": "load-store"}
        ]
        comparison = receipt.compare_trace_summary(
            frame={"candidate_bytes": 24, "target_bytes": 24},
            stack_homes={"status": "available", "homes": homes},
            temp_events={"status": "available", "events": [event]},
            target={
                "frame_size_bytes": 24,
                "stack_homes": {"status": "available", "homes": homes},
                "temp_events": {"status": "available", "events": [event]},
            },
        )
        self.assertEqual(
            "no-divergence", comparison["first_divergence"]["status"]
        )

    def test_missing_source_line_is_unavailable_not_a_guessed_divergence(self) -> None:
        candidate_event = {
            "ordinal": 0,
            "temp_class": "integer",
            "lifecycle": "birth",
            "ring_action": "pop",
            "source": {"procedure": "target", "line": None},
        }
        target_event = {
            **candidate_event,
            "source": {"procedure": "target", "line": 17},
        }
        comparison = receipt.compare_trace_summary(
            frame={"candidate_bytes": 24, "target_bytes": 24},
            stack_homes={"status": "available", "homes": []},
            temp_events={"status": "available", "events": [candidate_event]},
            target={
                "frame_size_bytes": 24,
                "stack_homes": {"status": "available", "homes": []},
                "temp_events": {"status": "available", "events": [target_event]},
            },
        )
        self.assertEqual("partial", comparison["fields"]["temp_events"]["status"])
        self.assertEqual(
            "unavailable", comparison["first_divergence"]["status"]
        )

    def test_malformed_and_truncated_ugen_traces_fail_closed(self) -> None:
        with self.assertRaisesRegex(receipt.ReceiptError, "malformed reg"):
            receipt.parse_ugen_events(
                "DKWB-FREELIST ALLOC_GP_RESULT reg=oops emitted=1 line=2\n",
                procedure="target",
            )
        with self.assertRaisesRegex(receipt.ReceiptError, "no ALLOC_GP_RESULT"):
            receipt.parse_ugen_events(
                "DKWB-FREELIST ALLOC_GP reg=96 emitted=1 line=2\n",
                procedure="target",
            )
        with self.assertRaisesRegex(receipt.ReceiptError, "malformed DKWB"):
            receipt.parse_ugen_events(
                "DKWB-FREELIST ALLOC_GP_RESULT\n", procedure="target"
            )
        with self.assertRaisesRegex(receipt.ReceiptError, "malformed structured"):
            receipt.parse_ugen_events(
                "DKWB-FREELIST ALLOC_GP_RESULT reg=14 trailing line=2\n",
                procedure="target",
            )

    def test_diagnostics_and_structured_output_redact_host_paths_and_raw_rows(self) -> None:
        redacted = receipt.redact_diagnostic(
            "could not decode /private/workstation/project/candidate.o"
        )
        self.assertNotIn("/private/workstation", redacted)
        self.assertIn("<redacted-path>", redacted)
        summary = receipt.summarize_ugen_results(
            "DKWB-FREELIST ALLOC_GP_RESULT reg=14 emitted=1 line=617\n",
            procedure="target",
        )
        rendered = json.dumps(summary, sort_keys=True, separators=(",", ":"))
        self.assertNotIn("DKWB-FREELIST", rendered)
        self.assertNotIn("emitted", rendered)
        self.assertNotIn("/Users", rendered)

    def test_structured_json_is_deterministic(self) -> None:
        candidate = receipt.summarize_ugen_results(
            "DKWB-FREELIST ALLOC_GP_RESULT reg=15 emitted=2 line=8\n"
            "DKWB-FREELIST ALLOC_GP_RESULT reg=14 emitted=1 line=7\n",
            procedure="target",
        )
        payload = {
            "frame": {"candidate_bytes": 32, "target_bytes": 32},
            "stack_homes": {"status": "available", "homes": []},
            "temp_events": {"status": "available", "events": candidate["events"]},
        }
        first = json.dumps(payload, indent=2, sort_keys=True)
        second = json.dumps(payload, indent=2, sort_keys=True)
        self.assertEqual(first, second)
        self.assertEqual([0, 1], [row["ordinal"] for row in candidate["events"]])

    def test_hash_bound_workbench_frame_summary_is_admitted(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            candidate = root / "candidate.o"
            summary = root / "summary.json"
            candidate.write_bytes(b"candidate")
            digest = receipt.sha256_file(candidate)
            summary.write_text(
                json.dumps(
                    {
                        "schema": receipt.WORKBENCH_SUMMARY_SCHEMA,
                        "symbol": {
                            "requested": "target",
                            "target": "target",
                            "candidate": "target",
                        },
                        "comparison": {
                            "target_frame_bytes": 32,
                            "candidate_frame_bytes": 32,
                        },
                        "provenance": {"candidate_object_sha256": digest},
                    }
                ),
                encoding="utf-8",
            )
            frame = receipt.load_workbench_summary(
                str(summary), symbol="target", candidate_sha256=digest
            )
        self.assertEqual(32, frame["candidate_bytes"])
        self.assertEqual(32, frame["target_bytes"])
        self.assertEqual("hash-bound-workbench-summary", frame["evidence"])

    def test_target_evidence_rejects_truncation_and_unsupported_fields(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            truncated = root / "truncated.json"
            truncated.write_text('{"schema":', encoding="utf-8")
            with self.assertRaisesRegex(receipt.ReceiptError, "malformed or truncated"):
                receipt.load_target_evidence(str(truncated), symbol="target")
            unsupported = root / "unsupported.json"
            unsupported.write_text(
                json.dumps(
                    {
                        "schema": receipt.TARGET_EVIDENCE_SCHEMA,
                        "symbol": "target",
                        "frame_size_bytes": 32,
                        "raw_trace": "must not be admitted",
                    }
                ),
                encoding="utf-8",
            )
            with self.assertRaisesRegex(receipt.ReceiptError, "unsupported fields"):
                receipt.load_target_evidence(str(unsupported), symbol="target")

    def test_fidelity_failure_is_bounded_and_does_not_echo_tool_output(self) -> None:
        process = SimpleNamespace(
            returncode=1,
            stdout=json.dumps({"pass": False, "gates": {".text": False}}),
            stderr="/private/workstation/raw instruction listing",
        )
        with mock.patch.object(receipt, "run_command", return_value=process):
            with self.assertRaises(receipt.ReceiptError) as raised:
                receipt.run_fidelity_gate(
                    Path("stock.o"),
                    Path("traced.o"),
                    workbench="workbench",
                    objdump="objdump",
                )
        message = str(raised.exception)
        self.assertIn("fidelity gate failed", message)
        self.assertNotIn("/Users", message)
        self.assertNotIn("instruction listing", message)


if __name__ == "__main__":
    unittest.main()
