#!/usr/bin/env python3
"""Focused tests for allocator_trace_receipt.py."""

from __future__ import annotations

import argparse
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


if __name__ == "__main__":
    unittest.main()
