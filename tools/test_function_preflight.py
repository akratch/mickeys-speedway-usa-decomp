#!/usr/bin/env python3
"""Focused tests for function_preflight identity and fail-closed routing."""

from __future__ import annotations

import contextlib
import dataclasses
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

    def test_candidate_redefine_aliases_preserve_sources(self) -> None:
        command = (
            "tools/binutils/mips64-elf-objcopy "
            "--redefine-sym func_80005750=func_80005750_o001Reloc "
            "--redefine-sym=mathRnd=mathRnd_o001Reloc "
            "--redefine-sym first=sharedProxy && "
            "tools/binutils/mips64-elf-objcopy --redefine-sym second=sharedProxy "
            "build/src/example.c.o"
        )
        with mock.patch.object(
            fp.pa, "run_make_database", return_value="database"
        ), mock.patch.object(
            fp.pa, "postprocess_commands",
            return_value={"build/src/example.c.o": command},
        ):
            aliases = fp._candidate_redefine_aliases(
                fp.REPO / "build/src/example.c.o"
            )
        self.assertEqual(
            aliases,
            {
                "func_80005750_o001Reloc": "func_80005750",
                "mathRnd_o001Reloc": "mathRnd",
            },
        )

    def test_candidate_redefine_aliases_collapse_transitive_chain(self) -> None:
        command = (
            "tools/binutils/mips64-elf-objcopy --redefine-sym original=proxy "
            "build/src/example.c.o && "
            "tools/binutils/mips64-elf-objcopy --redefine-sym proxy=final "
            "build/src/example.c.o"
        )
        with mock.patch.object(
            fp.pa, "run_make_database", return_value="database"
        ), mock.patch.object(
            fp.pa, "postprocess_commands",
            return_value={"build/src/example.c.o": command},
        ):
            aliases = fp._candidate_redefine_aliases(
                fp.REPO / "build/src/example.c.o"
            )
        self.assertEqual(
            {"proxy": "original", "final": "original"}, aliases
        )

    def test_candidate_redefine_alias_cycle_fails_closed(self) -> None:
        command = (
            "$(OBJCOPY) --redefine-sym first=second build/src/example.c.o && "
            "$(OBJCOPY) --redefine-sym second=first build/src/example.c.o"
        )
        with mock.patch.object(
            fp.pa, "run_make_database", return_value="database"
        ), mock.patch.object(
            fp.pa, "postprocess_commands",
            return_value={"build/src/example.c.o": command},
        ):
            with self.assertRaisesRegex(fp.PreflightError, "contain a cycle"):
                fp._candidate_redefine_aliases(
                    fp.REPO / "build/src/example.c.o"
                )

    def test_promoted_overlay_resolves_from_exact_atlas_owner_without_asm(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            alias = root / "aliases.txt"
            alias.write_text(
                "func_overlay_016_F00001E0_1873678 = friendly;\n",
                encoding="utf-8",
            )
            source = root / "src/overlays/o016/friendly.c"
            source.parent.mkdir(parents=True)
            source.write_text("void friendly(void) {}\n", encoding="utf-8")
            atlas = root / "atlas.json"
            atlas.write_text(
                json.dumps(
                    {
                        "modules": [
                            {
                                "overlay": 16,
                                "text_ownership": [
                                    {
                                        "offset": "0x1E0",
                                        "end_offset": "0x220",
                                        "size": "0x40",
                                        "type": "c",
                                        "source": "overlays/o016/friendly",
                                        "matched": True,
                                        "nonmatching": False,
                                    }
                                ],
                            }
                        ]
                    }
                ),
                encoding="utf-8",
            )
            symbols = root / "symbols.txt"
            symbols.write_text("", encoding="utf-8")

            friendly = fp.resolve(
                "friendly",
                root=root,
                alias_path=alias,
                atlas_path=atlas,
                symbol_path=symbols,
            )
            generated = fp.resolve(
                "func_overlay_016_F00001E0_1873678",
                root=root,
                alias_path=alias,
                atlas_path=atlas,
                symbol_path=symbols,
            )

        self.assertEqual("post_promotion", friendly.resolution_mode)
        self.assertIsNone(friendly.target_asm)
        self.assertEqual("build", friendly.candidate_build_dir)
        self.assertEqual(0xF00001E0, friendly.expected_value)
        self.assertEqual(0x40, friendly.expected_size)
        self.assertEqual(friendly.target_symbol, generated.target_symbol)
        self.assertEqual(friendly.candidate_symbol, generated.candidate_symbol)

    def test_promoted_mixed_tu_overlay_uses_function_sized_exact_range(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            alias = root / "aliases.txt"
            alias.write_text(
                "func_overlay_025_F0000000_1879C88 = friendly;\n",
                encoding="utf-8",
            )
            source = root / "src/overlays/o025/overlay_025.c"
            source.parent.mkdir(parents=True)
            source.write_text(
                "void friendly(void) {}\n"
                "#ifdef NON_MATCHING\n"
                "void another(void) {}\n"
                "#else\n"
                '#pragma GLOBAL_ASM("asm/nonmatchings/x/another.s")\n'
                "#endif\n",
                encoding="utf-8",
            )
            atlas = root / "atlas.json"
            atlas.write_text(
                json.dumps(
                    {
                        "modules": [
                            {
                                "overlay": 25,
                                "text_ownership": [
                                    {
                                        "offset": "0x0",
                                        "end_offset": "0x608",
                                        "size": "0x608",
                                        "type": "c",
                                        "source": "overlays/o025/overlay_025",
                                        "matched": True,
                                        "nonmatching": True,
                                    }
                                ],
                                "mixed_tu_exact_c_ranges": [
                                    {
                                        "offset": "0x0",
                                        "end_offset": "0x17C",
                                        "size": "0x17C",
                                        "label": "friendly",
                                        "source": "overlays/o025/overlay_025",
                                        "evidence": "linked exact",
                                    }
                                ],
                            }
                        ]
                    }
                ),
                encoding="utf-8",
            )

            resolution = fp.resolve(
                "friendly",
                root=root,
                alias_path=alias,
                atlas_path=atlas,
                symbol_path=root / "unused-symbols.txt",
            )

        self.assertEqual(0x17C, resolution.expected_size)
        self.assertIn("mixed_tu_exact_c_ranges", resolution.identity_evidence)

    def test_promoted_exact_tu_uses_next_export_as_function_boundary(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            alias = root / "aliases.txt"
            alias.write_text(
                "func_overlay_049_F0000000_1896410 = friendly;\n",
                encoding="utf-8",
            )
            source = root / "src/overlays/o049/overlay_049.c"
            source.parent.mkdir(parents=True)
            source.write_text("void friendly(void) {}\n", encoding="utf-8")
            atlas = root / "atlas.json"
            atlas.write_text(
                json.dumps(
                    {
                        "modules": [
                            {
                                "overlay": 49,
                                "text_ownership": [
                                    {
                                        "offset": "0x0",
                                        "end_offset": "0x374",
                                        "size": "0x374",
                                        "type": "c",
                                        "source": "overlays/o049/overlay_049",
                                        "matched": True,
                                        "nonmatching": False,
                                    }
                                ],
                                "exports": [
                                    {"rom_table_index": 1415, "offset": "0x0"},
                                    {"rom_table_index": 1433, "offset": "0x1F4"},
                                    {"rom_table_index": 1397, "offset": "0x354"},
                                ],
                            }
                        ]
                    }
                ),
                encoding="utf-8",
            )

            resolution = fp.resolve(
                "friendly",
                root=root,
                alias_path=alias,
                atlas_path=atlas,
                symbol_path=root / "unused-symbols.txt",
            )

        self.assertEqual(0x1F4, resolution.expected_size)
        self.assertIn("export boundary", resolution.identity_evidence)

    def test_promoted_resident_requires_matched_c_symbol_geometry(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            alias = root / "aliases.txt"
            alias.write_text("", encoding="utf-8")
            source = root / "src/main/memory.c"
            source.parent.mkdir(parents=True)
            source.write_text("void func_8002BB40(void) {}\n", encoding="utf-8")
            symbols = root / "symbols.txt"
            symbols.write_text(
                "func_8002BB40 = 0x8002BB40; // type:func size:0x120 matched C\n",
                encoding="utf-8",
            )

            resolution = fp.resolve(
                "func_8002BB40",
                root=root,
                alias_path=alias,
                atlas_path=root / "unused-atlas.json",
                symbol_path=symbols,
            )

        self.assertEqual("post_promotion", resolution.resolution_mode)
        self.assertEqual(0x8002BB40, resolution.expected_value)
        self.assertEqual(0x120, resolution.expected_size)

    def test_missing_fallback_does_not_promote_guarded_nonmatching_source(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            alias = root / "aliases.txt"
            alias.write_text(
                "func_overlay_016_F00001E0_1873678 = friendly;\n",
                encoding="utf-8",
            )
            source = root / "src/overlays/o016/friendly.c"
            source.parent.mkdir(parents=True)
            source.write_text(
                "#ifdef NON_MATCHING\n"
                "void friendly(void) {}\n"
                "#else\n"
                '#pragma GLOBAL_ASM("asm/nonmatchings/x/'
                'func_overlay_016_F00001E0_1873678.s")\n'
                "#endif\n",
                encoding="utf-8",
            )

            with self.assertRaisesRegex(fp.PreflightError, "not one unconditional"):
                fp.resolve(
                    "friendly",
                    root=root,
                    alias_path=alias,
                    atlas_path=root / "unused-atlas.json",
                    symbol_path=root / "unused-symbols.txt",
                )

    def test_untracked_ordinary_c_is_not_assumed_promoted(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            alias = root / "aliases.txt"
            alias.write_text("", encoding="utf-8")
            source = root / "src/main/example.c"
            source.parent.mkdir(parents=True)
            source.write_text("void example(void) {}\n", encoding="utf-8")
            symbols = root / "symbols.txt"
            symbols.write_text(
                "example = 0x80001000; // type:func size:0x20 tier-D\n",
                encoding="utf-8",
            )

            with self.assertRaisesRegex(fp.PreflightError, "matched-C"):
                fp.resolve(
                    "example",
                    root=root,
                    alias_path=alias,
                    atlas_path=root / "unused-atlas.json",
                    symbol_path=symbols,
                )


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

    def test_consolidated_tu_escape_reports_target_and_candidate_drift(self) -> None:
        class FakeElf:
            names = ["", ".text"]

            @staticmethod
            def symbols():
                return [
                    (
                        "func_overlay_009_F00010B4_186772C",
                        0x10DC,
                        0x468,
                        2,
                        1,
                    )
                ]

        resolution = fp.Resolution(
            "func_overlay_009_F00010B4_186772C",
            "func_overlay_009_F00010B4_186772C",
            "func_overlay_009_F00010B4_186772C",
            fp.REPO / "src/overlays/o009/overlay_009.c",
            "overlays/o009/overlay_009",
            "build_non_matching",
            fp.REPO / "build_non_matching/src/overlays/o009/overlay_009.c.o",
            fp.REPO / "asm/target.s",
            "guarded",
        )
        atlas = {
            "modules": [
                {
                    "overlay": 9,
                    "text_ownership": [
                        {
                            "offset": "0x0",
                            "end_offset": "0x1520",
                            "size": "0x1520",
                            "type": "c",
                            "source": "overlays/o009/overlay_009",
                        }
                    ],
                }
            ]
        }

        with mock.patch.object(fp.rs, "Elf", return_value=FakeElf()):
            message = fp._surface_error_diagnostic(
                fp.rs.SurfaceComparisonError("candidate function escapes TU ownership"),
                resolution,
                atlas,
                0xF00010B4,
                0x468,
            )

        self.assertIn("TU .text+0x10DC..+0x1544", message)
        self.assertIn("target belongs at TU+0x10B4..+0x151C", message)
        self.assertIn("shifted this function by +0x28", message)
        self.assertIn("exceeds the owner by 0x24", message)
        self.assertIn("will not reinterpret bytes outside the atlas owner", message)

    def test_consolidated_alias_error_reports_compiled_identity_drift(self) -> None:
        class FakeElf:
            names = ["", ".text"]

            @staticmethod
            def section(name):
                if name != ".text":
                    raise ValueError(name)
                return 1, {"size": 0x4664}

            @staticmethod
            def symbols():
                return [
                    ("aimed", 0x37D4, 0x3E4, 2, 1),
                    ("overlay1ReadSelection", 0x2ECC, 0xD4, 2, 1),
                ]

        resolution = fp.Resolution(
            "aimed",
            "func_overlay_001_F0006D4C_185312C",
            "aimed",
            fp.REPO / "src/overlays/o001/overlay_001_tail.c",
            "overlays/o001/overlay_001_tail",
            "build_non_matching",
            fp.REPO / "build_non_matching/src/overlays/o001/overlay_001_tail.c.o",
            fp.REPO / "asm/target.s",
            "guarded",
        )
        atlas = {
            "modules": [
                {
                    "overlay": 1,
                    "text_ownership": [
                        {
                            "offset": "0x3578",
                            "end_offset": "0x7BDC",
                            "size": "0x4664",
                            "type": "c",
                            "source": "overlays/o001/overlay_001_tail",
                        }
                    ],
                }
            ]
        }
        with tempfile.TemporaryDirectory() as directory:
            aliases = Path(directory) / "aliases.txt"
            aliases.write_text(
                "func_overlay_001_F0006424_1852804 = overlay1ReadSelection;\n",
                encoding="utf-8",
            )
            error = fp.rs.SurfaceComparisonError(
                "candidate relocation symbol overlay1ReadSelection "
                "has ambiguous runtime identity"
            )
            with mock.patch.object(fp.rs, "Elf", return_value=FakeElf()):
                message = fp._surface_error_diagnostic(
                    error,
                    resolution,
                    atlas,
                    0xF0006D4C,
                    0x3E4,
                    alias_path=aliases,
                )

        self.assertIn("tracked alias identity is overlay:1:+0x6424", message)
        self.assertIn("lands at overlay:1:+0x6444", message)
        self.assertIn("TU .text+0x2ECC", message)
        self.assertIn("disagreement +0x20", message)
        self.assertIn("candidate prefix-layout drift", message)

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
        with mock.patch.object(fp, "_run", return_value=completed) as run:
            report = fp._workbench(resolution)

        self.assertEqual(
            [str(fp.WB_COMPARE), "--no-build", "friendly"],
            run.call_args.args[0][:3],
        )
        self.assertEqual(report["matched_words"], 7)
        self.assertEqual(report["first_mismatch"], "+0x8")
        self.assertNotIn("diff_sites", report)

    def test_promoted_workbench_uses_friendly_symbol_and_rom_oracle(self) -> None:
        resolution = fp.Resolution(
            "generated",
            "generated",
            "friendly",
            Path("src/example.c"),
            "example",
            "build",
            Path("build/src/example.c.o"),
            None,
            "promoted",
            resolution_mode="post_promotion",
            expected_value=0xF0000020,
            expected_size=0x20,
        )
        payload = {
            "schema": "decomp-workbench-comparison-v1",
            "words": 0,
            "target_instructions": 8,
            "candidate_instructions": 8,
            "first_divergent_row": None,
            "verdict": "exact",
        }
        completed = subprocess.CompletedProcess(
            [], 0, stdout=json.dumps(payload), stderr=""
        )
        with mock.patch.object(fp, "_run", return_value=completed) as run:
            report = fp._workbench(resolution)

        command = run.call_args.args[0]
        self.assertEqual(command[1:3], ["--rom", "friendly"])
        self.assertEqual("rom", report["comparison_mode"])

    def test_promoted_geometry_must_equal_tracked_range(self) -> None:
        resolution = fp.Resolution(
            "friendly",
            "generated",
            "friendly",
            Path("src/example.c"),
            "example",
            "build",
            Path("build/src/example.c.o"),
            None,
            "promoted",
            resolution_mode="post_promotion",
            expected_value=0xF0000020,
            expected_size=0x20,
        )
        with self.assertRaisesRegex(fp.PreflightError, "linked geometry disagrees"):
            fp._require_tracked_geometry(resolution, 0xF0000020, 0x24)

    def test_promoted_mode_accepts_exact_relocation_shape_with_proxy_identities(self) -> None:
        resolution = fp.Resolution(
            "friendly", "generated", "friendly", Path("src/example.c"),
            "example", "build", Path("build/src/example.c.o"), None,
            "promoted", resolution_mode="post_promotion",
        )
        comparison = {
            "candidate_record_count": 9,
            "candidate_identity_resolved_count": 4,
            "target_record_count": 9,
            "offset_type_exact": True,
        }
        fp._require_static_relocation_evidence(resolution, comparison)

    def test_fallback_mode_still_rejects_unresolved_relocation_identity(self) -> None:
        resolution = fp.Resolution(
            "friendly", "generated", "friendly", Path("src/example.c"),
            "example", "build_non_matching",
            Path("build_non_matching/src/example.c.o"), Path("target.s"),
            "guarded fallback",
        )
        comparison = {
            "candidate_record_count": 3,
            "candidate_identity_resolved_count": 2,
            "target_record_count": 3,
            "offset_type_exact": True,
        }
        with self.assertRaisesRegex(fp.PreflightError, "unresolved at 1"):
            fp._require_static_relocation_evidence(resolution, comparison)

    def test_promoted_mode_rejects_relocation_shape_drift(self) -> None:
        resolution = fp.Resolution(
            "friendly", "generated", "friendly", Path("src/example.c"),
            "example", "build", Path("build/src/example.c.o"), None,
            "promoted", resolution_mode="post_promotion",
        )
        comparison = {
            "candidate_record_count": 9,
            "candidate_identity_resolved_count": 9,
            "target_record_count": 8,
            "offset_type_exact": False,
        }
        with self.assertRaisesRegex(fp.PreflightError, "shape disagrees"):
            fp._require_static_relocation_evidence(resolution, comparison)

    def test_promoted_linked_exact_completes_runtime_identity_proof(self) -> None:
        resolution = fp.Resolution(
            "friendly", "generated", "friendly", Path("src/example.c"),
            "example", "build", Path("build/src/example.c.o"), None,
            "promoted", resolution_mode="post_promotion",
        )
        comparison = {
            "candidate_record_count": 13,
            "target_record_count": 13,
            "offset_type_exact": True,
            "stable_identity_alignment_count": 8,
            "stable_identity_exact": False,
        }
        workbench = {
            "differing_words": 0,
            "target_words": 133,
            "candidate_words": 133,
        }
        result = fp._augment_runtime_identity_evidence(
            resolution, comparison, workbench
        )
        self.assertEqual(5, result["linked_runtime_identity_alignment_count"])
        self.assertEqual(13, result["effective_identity_alignment_count"])
        self.assertTrue(result["effective_identity_exact"])
        self.assertEqual(
            "static-plus-runtime-table-and-linked-rom",
            result["identity_proof_mode"],
        )

    def test_unpromoted_exact_words_do_not_replace_static_identity(self) -> None:
        resolution = fp.Resolution(
            "friendly", "generated", "friendly", Path("src/example.c"),
            "example", "build_non_matching",
            Path("build_non_matching/src/example.c.o"), Path("target.s"),
            "guarded",
        )
        comparison = {
            "candidate_record_count": 3,
            "target_record_count": 3,
            "offset_type_exact": True,
            "stable_identity_alignment_count": 2,
            "stable_identity_exact": False,
        }
        workbench = {
            "differing_words": 0,
            "target_words": 20,
            "candidate_words": 20,
        }
        result = fp._augment_runtime_identity_evidence(
            resolution, comparison, workbench
        )
        self.assertEqual(0, result["linked_runtime_identity_alignment_count"])
        self.assertEqual(2, result["effective_identity_alignment_count"])
        self.assertFalse(result["effective_identity_exact"])
        self.assertEqual("partial-static", result["identity_proof_mode"])


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

    def test_build_logic_inputs_include_make_policy_fragments(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            makefile = root / "Makefile"
            overlay_policy = root / "mk/overlays.mk"
            normalization = root / "config/normalizations/example.mk"
            for path in (makefile, overlay_policy, normalization):
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text("# build logic\n", encoding="utf-8")

            self.assertEqual(
                (makefile, overlay_policy, normalization),
                fp._build_logic_inputs(root),
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

    def test_newer_build_policy_forces_complete_target_dependency_graph(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            resolution = self.resolution(root)
            policy = root / "mk/flags.mk"
            policy.parent.mkdir(parents=True)
            policy.write_text("# changed flags\n", encoding="utf-8")
            object_time = resolution.candidate_object.stat().st_mtime_ns
            os.utime(policy, ns=(object_time + 1_000_000, object_time + 1_000_000))
            completed = subprocess.CompletedProcess([], 0, stdout="", stderr="")
            with mock.patch.object(
                fp, "_build_logic_inputs", return_value=(policy,)
            ), mock.patch.object(fp, "_run", return_value=completed) as run:
                fp._build_target(
                    resolution.candidate_object,
                    non_matching=True,
                    label="candidate",
                )

            commands = [call.args[0] for call in run.call_args_list]
            self.assertNotIn("--always-make", commands[0])
            self.assertIn("--always-make", commands[1])
            self.assertIn("--assume-old=.venv/bin/python", commands[1])
            self.assertEqual(commands[1][-1], fp._relative(resolution.candidate_object))

    def test_resolve_wb_refreshes_by_default_and_no_build_is_explicit(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            resolution = self.resolution(Path(directory))
            resolution.target_asm.parent.mkdir(parents=True)
            resolution.target_asm.write_text("glabel generated\n", encoding="utf-8")
            with mock.patch.object(
                fp, "resolve", return_value=resolution
            ), mock.patch.object(fp, "_build") as build, mock.patch.object(
                fp, "require_fresh_evidence"
            ) as freshness:
                with contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(0, fp.main(["friendly", "--resolve-wb"]))
            build.assert_called_once_with(resolution)
            freshness.assert_called_once_with(resolution)

            with mock.patch.object(
                fp, "resolve", return_value=resolution
            ), mock.patch.object(fp, "_build") as build, mock.patch.object(
                fp, "require_fresh_evidence"
            ) as freshness:
                with contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(
                        0,
                        fp.main(["friendly", "--resolve-wb", "--no-build"]),
                    )
            build.assert_not_called()
            freshness.assert_called_once_with(resolution)

    def test_nonmatching_candidate_build_precedes_final_canonical_build(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            resolution = self.resolution(Path(directory))
            with mock.patch.object(fp, "_build_target") as build_target:
                fp._build(resolution)

            self.assertEqual(
                [
                    mock.call(
                        resolution.candidate_object,
                        non_matching=True,
                        label="candidate",
                    ),
                    mock.call(fp.TARGET_ELF, non_matching=False, label="canonical"),
                ],
                build_target.call_args_list,
            )

    def test_ordinary_candidate_is_supplied_by_canonical_link_build(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            resolution = dataclasses.replace(
                self.resolution(Path(directory)), candidate_build_dir="build"
            )
            with mock.patch.object(fp, "_build_target") as build_target:
                fp._build(resolution)

            build_target.assert_called_once_with(
                fp.TARGET_ELF, non_matching=False, label="canonical"
            )


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
            "resolution_mode": "post_promotion",
            "identity_evidence": "symbol_addrs matched-C function row",
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
                "comparison_mode": "rom",
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
