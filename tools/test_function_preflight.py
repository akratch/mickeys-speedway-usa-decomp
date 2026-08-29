#!/usr/bin/env python3
"""Focused tests for function_preflight identity and fail-closed routing."""

from __future__ import annotations

import contextlib
import io
import json
import os
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


class FreshnessTests(unittest.TestCase):
    def resolution(self, root: Path) -> fp.Resolution:
        source = root / "src/example.c"
        source.parent.mkdir(parents=True)
        source.write_text("void friendly(void) {}\n", encoding="utf-8")
        candidate = root / "build_non_matching/src/example.c.o"
        candidate.parent.mkdir(parents=True)
        candidate.write_bytes(b"object")
        return fp.Resolution(
            "friendly",
            "generated",
            "friendly",
            source,
            "example",
            "build_non_matching",
            candidate,
            root / "asm/generated.s",
            "guarded",
        )

    def test_make_graph_staleness_fails_before_comparison(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            resolution = self.resolution(root)
            completed = subprocess.CompletedProcess([], 1, stdout="", stderr="")
            with mock.patch.object(fp, "_run", return_value=completed):
                with self.assertRaisesRegex(fp.PreflightError, "according to the Make"):
                    fp._require_fresh_target(
                        resolution.candidate_object,
                        label="candidate object",
                        non_matching=True,
                        build_logic_inputs=(),
                    )

    def test_newer_recipe_or_config_fails_even_when_make_would_not(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            resolution = self.resolution(root)
            config = root / "config/normalizations/example.mk"
            config.parent.mkdir(parents=True)
            config.write_text("# flags changed\n", encoding="utf-8")
            object_time = resolution.candidate_object.stat().st_mtime_ns
            os.utime(config, ns=(object_time + 1_000_000, object_time + 1_000_000))

            with self.assertRaisesRegex(fp.PreflightError, "newer build recipe/config"):
                fp._require_fresh_target(
                    resolution.candidate_object,
                    label="candidate object",
                    non_matching=True,
                    build_logic_inputs=(config,),
                )

    def test_fresh_nonmatching_object_queries_its_actual_build_mode(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            resolution = self.resolution(root)
            completed = subprocess.CompletedProcess([], 0, stdout="", stderr="")
            with mock.patch.object(fp, "_run", return_value=completed) as run:
                fp._require_fresh_target(
                    resolution.candidate_object,
                    label="candidate object",
                    non_matching=True,
                    build_logic_inputs=(),
                )

            command = run.call_args.args[0]
            self.assertIn("NON_MATCHING=1", command)
            self.assertEqual(command[-1], fp._relative(resolution.candidate_object))

    def test_make_query_error_fails_closed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            resolution = self.resolution(root)
            completed = subprocess.CompletedProcess(
                [], 2, stdout="", stderr="dependency parse failed\n"
            )
            with mock.patch.object(fp, "_run", return_value=completed):
                with self.assertRaisesRegex(fp.PreflightError, "could not prove"):
                    fp._require_fresh_target(
                        resolution.candidate_object,
                        label="candidate object",
                        non_matching=True,
                        build_logic_inputs=(),
                    )

    def test_build_runs_split_and_target_as_separate_make_invocations(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            resolution = self.resolution(root)
            completed = subprocess.CompletedProcess([], 0, stdout="", stderr="")
            with mock.patch.object(fp, "_run", return_value=completed) as run:
                fp._build_target(
                    resolution.candidate_object,
                    non_matching=True,
                    label="candidate",
                )

            commands = [call.args[0] for call in run.call_args_list]
            self.assertEqual(len(commands), 2)
            self.assertEqual(commands[0][-1], "build_non_matching/.splat-stamp")
            self.assertEqual(commands[1][-1], fp._relative(resolution.candidate_object))
            self.assertIn("NON_MATCHING=1", commands[0])
            self.assertIn("NON_MATCHING=1", commands[1])


class RelocationEvidenceTests(unittest.TestCase):
    def test_resident_reports_eight_static_tuples_and_zero_runtime_records(self) -> None:
        resolution = fp.Resolution(
            "func_8002BB40",
            "func_8002BB40",
            "func_8002BB40",
            Path("src/main/memory.c"),
            "main/memory",
            "build",
            Path("build/src/main/memory.c.o"),
            Path("asm/unused.s"),
            "ordinary C",
        )
        static_records = [
            fp.rs.SurfaceRecord(0x14, fp.rs.R_MIPS_HI16, (0, 0xD1D60)),
            fp.rs.SurfaceRecord(0x18, fp.rs.R_MIPS_LO16, (0, 0xD1D60)),
            fp.rs.SurfaceRecord(0x28, fp.rs.R_MIPS_HI16, (0, 0xD1810)),
            fp.rs.SurfaceRecord(0x2C, fp.rs.R_MIPS_LO16, (0, 0xD1810)),
            fp.rs.SurfaceRecord(0x08, fp.rs.R_MIPS_HI16, (0, 0x79E20)),
            fp.rs.SurfaceRecord(0x64, fp.rs.R_MIPS_LO16, (0, 0x79E20)),
            fp.rs.SurfaceRecord(0x40, fp.rs.R_MIPS_HI16, (0, 0xD1810)),
            fp.rs.SurfaceRecord(0x44, fp.rs.R_MIPS_LO16, (0, 0xD1810)),
        ]

        with mock.patch.object(
            fp.rs, "_resident_target_records", return_value=static_records
        ) as authenticate:
            evidence = fp._relocation_evidence(
                resolution,
                {"kind": "resident"},
                [],
                mock.sentinel.target_elf,
                "func_8002BB40",
                0x8002BB40,
                0x120,
                ".main",
            )

        self.assertEqual(8, len(evidence["target_static_relocations"]))
        self.assertEqual([], evidence["resident_runtime_records"])
        self.assertEqual([], evidence["runtime_overlay_records"])
        self.assertEqual(
            ["+0x14", "+0x18", "+0x28", "+0x2C", "+0x8", "+0x64", "+0x40", "+0x44"],
            [row["offset"] for row in evidence["target_static_relocations"]],
        )
        authenticate.assert_called_once_with(
            resolution.candidate_object,
            "main/memory",
            mock.sentinel.target_elf,
            "func_8002BB40",
            0x8002BB40,
            0x120,
            ".main",
            fp.ALIASES,
            [],
        )

        report = {
            "target_symbol": "func_8002BB40",
            "candidate_symbol": "func_8002BB40",
            "source": "src/main/memory.c",
            "candidate_build_dir": "build",
            "candidate_signature": "s32 func_8002BB40(void)",
            "owned_size": 0x120,
            "context": {
                "kind": "resident",
                "vram_start": 0x8002BB40,
                "vram_end": 0x8002BC60,
                "rom_start": 0x2C740,
                "rom_end": 0x2C860,
                "next_symbol": None,
            },
            "exports": [],
            "callers": [],
            "inbound_references": [],
            **evidence,
            "relocation_comparison": {
                "target_surface_source": "resident-canonical-static-object",
                "target_record_count": 8,
                "offset_type_alignment_count": 8,
                "stable_identity_alignment_count": 8,
            },
            "workbench": {
                "matched_words": 72,
                "target_words": 72,
                "candidate_words": 72,
                "differing_words": 0,
                "first_mismatch": None,
                "verdict": "exact",
                "target_frame": 0,
                "candidate_frame": 0,
            },
        }
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            fp._render_human(report)
        rendered = output.getvalue()
        self.assertIn("target static relocations: 8", rendered)
        self.assertIn("resident runtime records: 0", rendered)
        self.assertIn("runtime overlay records: not applicable", rendered)
        self.assertIn("offset/type=8/8 identity=8/8", rendered)

    def test_overlay_runtime_records_are_not_called_static_target_records(self) -> None:
        resolution = fp.Resolution(
            "friendly", "generated", "friendly", Path("src/example.c"),
            "example", "build_non_matching", Path("build/example.o"),
            Path("asm/generated.s"), "guarded",
        )
        runtime = [fp.rs.SurfaceRecord(0x10, fp.rs.R_MIPS_26, (7, 0x20), 0)]

        evidence = fp._relocation_evidence(
            resolution, {"kind": "overlay"}, runtime,
            mock.sentinel.target_elf, "generated", 0xF0000020, 0x40, ".ovl",
        )

        self.assertEqual([], evidence["target_static_relocations"])
        self.assertEqual(1, len(evidence["runtime_overlay_records"]))
        self.assertEqual([], evidence["resident_runtime_records"])
if __name__ == "__main__":
    unittest.main()
