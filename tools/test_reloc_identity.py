#!/usr/bin/env python3
"""Focused tests for the shared relocation identity helpers."""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import reloc_identity as ri  # noqa: E402


class ObjdumpRelocationTests(unittest.TestCase):
    def test_sections_types_symbols_and_addends_are_canonicalized(self) -> None:
        text = """
RELOCATION RECORDS FOR [.text]:
OFFSET           TYPE              VALUE
00000010 R_MIPS_26        target
00000024 R_MIPS_HI16      global_data+0x18
00000028 R_MIPS_LO16      global_data - 4

RELOCATION RECORDS FOR [.data]:
00000000 R_MIPS_32        ignored+0x8
"""
        records = ri.parse_objdump_relocations(text, section=".text")
        self.assertEqual(
            records,
            [
                ri.ObjdumpRelocation(".text", 0x10, "R_MIPS_26", "target", 0),
                ri.ObjdumpRelocation(
                    ".text", 0x24, "R_MIPS_HI16", "global_data", 0x18
                ),
                ri.ObjdumpRelocation(
                    ".text", 0x28, "R_MIPS_LO16", "global_data", -4
                ),
            ],
        )

    def test_malformed_data_row_fails_closed(self) -> None:
        text = "RELOCATION RECORDS FOR [.text]:\n00000010 not-a-relocation\n"
        with self.assertRaisesRegex(ri.RelocationIdentityError, "cannot parse"):
            ri.parse_objdump_relocations(text, section=".text")


class ObjcopyAliasTests(unittest.TestCase):
    def test_transitive_aliases_retain_original_source(self) -> None:
        command = (
            "$(OBJCOPY) --redefine-sym base=middle x.o && "
            "tools/binutils/mips64-elf-objcopy --redefine-sym=middle=final x.o"
        )
        pairs = ri.parse_objcopy_redefine_pairs(command)
        closure = ri.canonicalize_redefine_aliases(pairs)
        self.assertEqual({"middle": "base", "final": "base"}, closure.resolved)
        self.assertEqual(frozenset(), closure.ambiguous)

    def test_duplicate_pair_is_idempotent(self) -> None:
        closure = ri.canonicalize_redefine_aliases(
            [("source", "alias"), ("source", "alias")]
        )
        self.assertEqual({"alias": "source"}, closure.resolved)
        self.assertEqual((("source", "alias"),), closure.duplicates)
        self.assertEqual((), closure.conflicts)

    def test_self_alias_is_idempotent(self) -> None:
        closure = ri.canonicalize_redefine_aliases([("same", "same")])
        self.assertEqual({"same": "same"}, closure.resolved)
        self.assertEqual(frozenset(), closure.ambiguous)
        self.assertEqual((), closure.cycles)

    def test_conflicting_sources_leave_destination_unresolved(self) -> None:
        closure = ri.canonicalize_redefine_aliases(
            [("first", "shared"), ("second", "shared"), ("shared", "final")]
        )
        self.assertNotIn("shared", closure.resolved)
        self.assertNotIn("final", closure.resolved)
        self.assertEqual(frozenset({"shared", "final"}), closure.ambiguous)
        self.assertIn(("shared", ("first", "second")), closure.conflicts)

    def test_cycle_and_downstream_alias_fail_closed(self) -> None:
        closure = ri.canonicalize_redefine_aliases(
            [("first", "second"), ("second", "first"), ("second", "after")]
        )
        self.assertEqual({}, closure.resolved)
        self.assertEqual(frozenset({"first", "second", "after"}), closure.ambiguous)
        self.assertEqual(1, len(closure.cycles))
        self.assertEqual({"first", "second"}, set(closure.cycles[0]))


class EffectiveIdentityTests(unittest.TestCase):
    def test_linker_and_transitive_objcopy_aliases_share_one_identity(self) -> None:
        resolution = ri.resolve_identities(
            {"generated": {(16, 0x1E0)}},
            equality_aliases=[("generated", "friendly")],
            redefine_aliases=[("friendly", "proxy"), ("proxy", "postprocessed")],
        )
        self.assertEqual((16, 0x1E0), resolution.resolved["friendly"])
        self.assertEqual((16, 0x1E0), resolution.resolved["postprocessed"])
        self.assertEqual(
            (16, 0x1F8),
            ri.effective_identity(resolution.resolved["postprocessed"], 0x18),
        )

    def test_conflicting_linker_identities_propagate_ambiguity(self) -> None:
        resolution = ri.resolve_identities(
            {"generated_a": {(1, 0x20)}, "generated_b": {(2, 0x20)}},
            equality_aliases=[
                ("generated_a", "friendly"),
                ("generated_b", "friendly"),
            ],
            redefine_aliases=[("friendly", "proxy")],
        )
        self.assertIn("friendly", resolution.ambiguous)
        self.assertIn("proxy", resolution.ambiguous)
        self.assertNotIn("proxy", resolution.resolved)

    def test_effective_identity_summary_preserves_compatibility_fields(self) -> None:
        target = [ri.RelocationRecord(0x10, 4, (7, 0x100))]
        candidate = [ri.RelocationRecord(0x10, 4, None)]
        comparison = ri.compare_records(target, candidate)
        result = ri.augment_effective_identity(comparison, linked_exact=True)
        self.assertEqual(1, result["target_runtime_record_count"])
        self.assertEqual(0, result["stable_identity_alignment_count"])
        self.assertEqual(1, result["linked_runtime_identity_alignment_count"])
        self.assertEqual(1, result["effective_identity_alignment_count"])
        self.assertTrue(result["effective_identity_exact"])

    def test_comparison_reports_unresolved_candidate_sites_without_identity(self) -> None:
        target = [
            ri.RelocationRecord(0x10, 4, (7, 0x100)),
            ri.RelocationRecord(0x08, 5, (0, 0x200)),
        ]
        candidate = [
            ri.RelocationRecord(0x10, 4, None),
            ri.RelocationRecord(0x08, 5, None),
        ]

        comparison = ri.compare_records(target, candidate)

        self.assertEqual(
            [
                {"offset": 0x08, "rtype": 5},
                {"offset": 0x10, "rtype": 4},
            ],
            comparison["candidate_identity_unresolved_records"],
        )
        self.assertTrue(
            all(
                "identity" not in row
                for row in comparison["candidate_identity_unresolved_records"]
            )
        )


if __name__ == "__main__":
    unittest.main()
