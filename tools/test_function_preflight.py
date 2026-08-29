#!/usr/bin/env python3
"""Focused tests for function_preflight identity and fail-closed routing."""

from __future__ import annotations

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import function_preflight as fp  # noqa: E402


class SymbolResolutionTests(unittest.TestCase):
    def fixture(self, root: Path, alias_lines: str) -> Path:
        alias = root / "overlay_undefined_syms.us.txt"
        alias.write_text(alias_lines, encoding="utf-8")
        source = root / "src/overlays/o016/example.c"
        source.parent.mkdir(parents=True)
        source.write_text(
            "#ifdef NON_MATCHING\n"
            "void friendly(s32 value) { (void)value; }\n"
            "#else\n"
            '#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o016/example/'
            'func_overlay_016_F0000000_1.s")\n'
            "#endif\n",
            encoding="utf-8",
        )
        asm = root / "asm/nonmatchings/overlays/o016/example/func_overlay_016_F0000000_1.s"
        asm.parent.mkdir(parents=True)
        asm.write_text("glabel func_overlay_016_F0000000_1\n", encoding="utf-8")
        return alias

    def test_friendly_and_generated_names_resolve_to_same_nonmatching_candidate(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            alias = self.fixture(
                root,
                "func_overlay_016_F0000000_1 = friendly;\n",
            )
            friendly = fp.resolve("friendly", root=root, alias_path=alias)
            generated = fp.resolve(
                "func_overlay_016_F0000000_1", root=root, alias_path=alias
            )

        self.assertEqual(friendly.target_symbol, generated.target_symbol)
        self.assertEqual(friendly.candidate_symbol, "friendly")
        self.assertEqual(friendly.candidate_build_dir, "build_non_matching")
        self.assertEqual(
            friendly.translation_unit, "overlays/o016/example"
        )

    def test_ambiguous_friendly_alias_fails_closed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            alias = root / "aliases.txt"
            alias.write_text(
                "func_overlay_016_F0000000_1 = friendly;\n"
                "func_overlay_017_F0000000_2 = friendly;\n",
                encoding="utf-8",
            )
            with self.assertRaisesRegex(fp.PreflightError, "maps to 2"):
                fp._resolve_names("friendly", alias)

    def test_unresolved_runtime_identity_fails_closed(self) -> None:
        with self.assertRaisesRegex(fp.PreflightError, "ambiguous"):
            fp._identity_text(None)


class GeometryAndWorkbenchTests(unittest.TestCase):
    def test_equal_alias_geometries_are_one_unambiguous_range(self) -> None:
        class FakeElf:
            names = ["", ".text"]

            @staticmethod
            def symbols():
                return [
                    ("friendly", 0xF0000010, 0x20, 0, 1),
                    ("generated", 0xF0000010, 0x20, 0, 1),
                ]

        name, value, size, section = fp._symbol_geometry(
            FakeElf(), ("friendly", "generated")
        )
        self.assertEqual((name, value, size, section), ("friendly", 0xF0000010, 0x20, ".text"))

    def test_resident_boundary_reports_exact_gap_to_next_function(self) -> None:
        class FakeElf:
            names = ["", ".text"]

            @staticmethod
            def symbols():
                return [
                    ("current", 0x80001000, 0x20, 2, 1),
                    ("next", 0x80001028, 0x10, 2, 1),
                ]

        boundary = fp._resident_boundary(FakeElf(), 0x80001000, 0x20, ".text")
        self.assertEqual(boundary["next_symbol"], "next")
        self.assertEqual(boundary["padding_size"], 8)

    def test_workbench_extracts_only_scalar_score_context(self) -> None:
        resolution = fp.Resolution(
            "friendly",
            "generated",
            "friendly",
            Path("src/example.c"),
            "example",
            "build_non_matching",
            Path("build_non_matching/src/example.c.o"),
            Path("asm/generated.s"),
            "guarded",
        )
        payload = {
            "schema": "decomp-workbench-comparison-v1",
            "words": 3,
            "target_instructions": 10,
            "candidate_instructions": 10,
            "first_divergent_row": 2,
            "verdict": "register-mismatch",
            "target_frame_size": 32,
            "candidate_frame_size": 32,
            "diff_sites": [{"target": "ROM-derived listing is not propagated"}],
        }
        completed = subprocess.CompletedProcess(
            [], 0, stdout=json.dumps(payload), stderr=""
        )
        with mock.patch.object(fp, "_run", return_value=completed):
            report = fp._workbench(resolution)

        self.assertEqual(report["matched_words"], 7)
        self.assertEqual(report["first_mismatch"], "+0x8")
        self.assertNotIn("diff_sites", report)


if __name__ == "__main__":
    unittest.main()
