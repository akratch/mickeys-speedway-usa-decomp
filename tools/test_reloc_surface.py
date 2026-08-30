#!/usr/bin/env python3
"""Focused source-level regression tests for reloc_surface safety gates."""

from __future__ import annotations

import contextlib
import io
import json
import os
import struct
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import reloc_surface as rs  # noqa: E402


class LinkedObjectCompletenessTests(unittest.TestCase):
    def test_missing_linker_object_fails_closed(self):
        with tempfile.TemporaryDirectory() as td:
            repo = Path(td)
            (repo / "mickey.us.ld").write_text(
                "build/src/overlays/o001/present.c.o(.text)\n"
                "build/src/overlays/o002/missing.c.o(.text)\n"
            )
            present = repo / "build/src/overlays/o001/present.c.o"
            present.parent.mkdir(parents=True)
            present.write_bytes(b"placeholder")

            with mock.patch.object(rs, "REPO", repo):
                with self.assertRaises(SystemExit) as caught:
                    rs.linked_overlay_objects()

            message = str(caught.exception)
            self.assertIn("complete linker object set", message)
            self.assertIn("build/src/overlays/o002/missing.c.o", message)

    def test_complete_linker_object_set_is_returned_in_order(self):
        with tempfile.TemporaryDirectory() as td:
            repo = Path(td)
            rels = [
                "build/src/overlays/o002/second.c.o",
                "build/src/overlays/o001/first.c.o",
            ]
            (repo / "mickey.us.ld").write_text(
                "\n".join(rel + "(.text)" for rel in rels) + "\n"
            )
            for rel in rels:
                path = repo / rel
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(b"placeholder")

            with mock.patch.object(rs, "REPO", repo):
                got = rs.linked_overlay_objects()

            self.assertEqual([(2, repo / rels[0]), (1, repo / rels[1])], got)


class ResidentRebindSafetyTests(unittest.TestCase):
    def test_dry_run_reports_plan_without_objcopy(self):
        objects = [(40, Path("build/src/overlays/o040/example.c.o"))]
        with mock.patch.object(rs, "resident_defined_names", return_value=set()):
            with mock.patch.object(
                rs,
                "resident_call_aliases",
                return_value=({"func_80000000": "func_80000000_o040Reloc"}, [], []),
            ):
                refusals, notes, planned = rs.rebind_resident_calls(
                    objects,
                    b"",
                    {40: []},
                    [None] * 40,
                    [],
                    records_cache={40: []},
                    apply=False,
                )

        self.assertEqual([], refusals)
        self.assertEqual([], notes)
        self.assertEqual(
            [("example.c.o", "func_80000000", "func_80000000_o040Reloc")],
            planned,
        )

    def test_check_mode_refuses_pending_rebind(self):
        with tempfile.TemporaryDirectory() as td:
            repo = Path(td)
            config = repo / "config"
            config.mkdir()
            (config / "overlays.us.json").write_text(json.dumps({"modules": []}))
            rom = repo / "rom.z64"
            out = repo / "surface.txt"
            rom.write_bytes(b"")
            out.write_text("tracked\n")
            diag = {
                "pending_rebinds": [
                    ("example.c.o", "func_80000000", "func_80000000_o040Reloc")
                ]
            }

            stderr = io.StringIO()
            with mock.patch.object(rs, "REPO", repo):
                with mock.patch.object(rs, "generate", return_value=("generated\n", diag)) as gen:
                    with contextlib.redirect_stderr(stderr):
                        status = rs.cmd_generate(
                            ["--check", "--rom", str(rom), "--out", str(out)]
                        )

            self.assertEqual(1, status)
            self.assertFalse(gen.call_args.kwargs["mutate_objects"])
            self.assertIn("read-only generation will not modify", stderr.getvalue())


class FunctionSurfaceComparisonTests(unittest.TestCase):
    GENERATED_O22 = "func_overlay_022_F0000D30_1878E38"

    class BoundaryElf:
        def __init__(self, *, path=Path("missing"), names=(), symbols=(),
                     relocations=(), text=b""):
            self.path = Path(path)
            self.names = list(names)
            self._symbols = list(symbols)
            self._relocations = list(relocations)
            self._text = bytes(text)

        def section(self, name):
            if name == ".text" and ".text" in self.names:
                return self.names.index(".text"), object()
            return None, None

        def section_bytes(self, name):
            return self._text if name == ".text" else b""

        def symbols(self):
            return list(self._symbols)

        def relocations(self, target=r"\.text"):
            return list(self._relocations)

    def o22_atlas(self, ownership=None):
        if ownership is None:
            ownership = [{
                "offset": "0xD30",
                "end_offset": "0xE9C",
                "size": "0x16C",
                "type": "c",
                "source": "overlays/o022/overlay22RemoveObject",
            }]
        return {
            "modules": [{
                "overlay": 22,
                "identity": "overlay:22",
                "synthetic_vma": "0xF0000000",
                "rom": {"start": "0x1878108"},
                "sections": {
                    "text": {
                        "start": "0x1878108",
                        "end": "0x1878FA8",
                        "size": "0xEA0",
                    }
                },
                "text_ownership": ownership,
            }]
        }

    def o22_boundary_fixture(self, root, *, rtype=None, linked_value=None):
        source = root / "src/overlays/o022/overlay22RemoveObject.c"
        source.parent.mkdir(parents=True, exist_ok=True)
        source.write_text("void placeholder(void) {}\n", encoding="utf-8")
        obj = root / "build/src/overlays/o022/overlay22RemoveObject.c.o"
        obj.parent.mkdir(parents=True, exist_ok=True)
        obj.write_bytes(b"ELF placeholder")
        generated = self.GENERATED_O22
        candidate = self.BoundaryElf(
            names=["", ".text"],
            symbols=[(generated, 0, 0, 0, rs.SHN_UNDEF)],
            relocations=[(
                ".text", 0x274,
                rs.R_MIPS_26 if rtype is None else rtype,
                0,
            )],
            text=b"\0" * 0x2B0,
        )
        canonical = self.BoundaryElf(
            path=obj,
            names=["", ".text"],
            # Metadata keeps the pre-trim size; physical .text is the exact
            # atlas-owned function boundary.
            symbols=[(generated, 0, 0x1B0, rs.STT_FUNC, 1)],
            text=b"\0" * 0x16C,
        )
        target = self.BoundaryElf(
            names=["", ".overlay_022"],
            symbols=[(
                generated,
                rs.SYNTHETIC_VMA + 0xD30 if linked_value is None else linked_value,
                0x1B0,
                rs.STT_FUNC,
                1,
            )],
        )
        return candidate, canonical, target, source, obj

    def test_o22_generated_call_uses_unique_owned_function_boundary(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            candidate, canonical, target, _source, _obj = (
                self.o22_boundary_fixture(root)
            )
            resolved, ambiguous = rs._stable_overlay_call_identities(
                root / "missing-aliases.txt",
                candidate,
                22,
                target,
                self.o22_atlas(),
                0,
                0x2B0,
                root=root,
                elf_loader=lambda _path: canonical,
            )

        self.assertEqual({self.GENERATED_O22: (22, 0xD30)}, resolved)
        self.assertEqual(set(), ambiguous)

    def test_generated_call_rejects_ambiguous_ownership(self):
        owner = self.o22_atlas()["modules"][0]["text_ownership"][0]
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            candidate, canonical, target, _source, _obj = (
                self.o22_boundary_fixture(root)
            )
            with self.assertRaisesRegex(
                rs.SurfaceComparisonError, "ambiguous overlapping atlas owners"
            ):
                rs._stable_overlay_call_identities(
                    root / "missing-aliases.txt", candidate, 22, target,
                    self.o22_atlas([dict(owner), dict(owner)]), 0, 0x2B0,
                    root=root, elf_loader=lambda _path: canonical,
                )

    def test_generated_call_rejects_cross_overlay_and_non_call(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            candidate, canonical, target, _source, _obj = (
                self.o22_boundary_fixture(root)
            )
            cross, _ = rs._stable_overlay_call_identities(
                root / "missing-aliases.txt", candidate, 21, target,
                self.o22_atlas(), 0, 0x2B0, root=root,
                elf_loader=lambda _path: canonical,
            )
            non_call_candidate, canonical, target, _source, _obj = (
                self.o22_boundary_fixture(root, rtype=rs.R_MIPS_HI16)
            )
            non_call, _ = rs._stable_overlay_call_identities(
                root / "missing-aliases.txt", non_call_candidate, 22, target,
                self.o22_atlas(), 0, 0x2B0, root=root,
                elf_loader=lambda _path: canonical,
            )

        self.assertEqual({}, cross)
        self.assertEqual({}, non_call)

    def test_generated_call_rejects_section_only_and_missing_boundary(self):
        broad = [{
            "offset": "0x0",
            "end_offset": "0xEA0",
            "size": "0xEA0",
            "type": "c",
            "source": "overlays/o022/overlay_022",
        }]
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            candidate, canonical, target, _source, _obj = (
                self.o22_boundary_fixture(root)
            )
            section_only, _ = rs._stable_overlay_call_identities(
                root / "missing-aliases.txt", candidate, 22, target,
                self.o22_atlas(broad), 0, 0x2B0, root=root,
                elf_loader=lambda _path: canonical,
            )
            missing, _ = rs._stable_overlay_call_identities(
                root / "missing-aliases.txt", candidate, 22, target,
                self.o22_atlas([]), 0, 0x2B0, root=root,
                elf_loader=lambda _path: canonical,
            )

        self.assertEqual({}, section_only)
        self.assertEqual({}, missing)

    def test_generated_call_rejects_stale_object(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            candidate, canonical, target, source, obj = (
                self.o22_boundary_fixture(root)
            )
            newer = obj.stat().st_mtime + 5
            os.utime(source, (newer, newer))
            resolved, _ = rs._stable_overlay_call_identities(
                root / "missing-aliases.txt", candidate, 22, target,
                self.o22_atlas(), 0, 0x2B0, root=root,
                elf_loader=lambda _path: canonical,
            )

        self.assertEqual({}, resolved)

    def test_generated_call_refuses_conflicting_linked_symbol(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            candidate, canonical, target, _source, _obj = (
                self.o22_boundary_fixture(
                    root, linked_value=rs.SYNTHETIC_VMA + 0xE00
                )
            )
            with self.assertRaisesRegex(
                rs.SurfaceComparisonError,
                "linked symbol conflicts with canonical overlay ownership",
            ):
                rs._stable_overlay_call_identities(
                    root / "missing-aliases.txt", candidate, 22, target,
                    self.o22_atlas(), 0, 0x2B0, root=root,
                    elf_loader=lambda _path: canonical,
                )

    def test_redefine_alias_propagates_stable_identity(self):
        class FakeElf:
            def __init__(self, target=False):
                self.target = target

            def section(self, name):
                self.assert_name = name
                return 1, object()

            def symbols(self):
                old = (
                    ("func_80005750", 0x80005750, 4, 0, 1)
                    if self.target
                    else ("func_80005750", 0, 0, 0, rs.SHN_UNDEF)
                )
                return [
                    old,
                    ("func_80005750_o001Reloc", 0, 0, 0, rs.SHN_UNDEF),
                ]

        resolved, ambiguous = rs._stable_symbol_identities(
            Path("missing"), FakeElf(), 1, 0x3578, FakeElf(target=True),
            {"func_80005750_o001Reloc": "func_80005750"},
        )
        self.assertEqual(
            resolved["func_80005750_o001Reloc"], (0, 0x5300),
        )
        self.assertNotIn("func_80005750_o001Reloc", ambiguous)

    def test_transitive_redefine_alias_propagates_stable_identity(self):
        class FakeElf:
            def __init__(self, target=False):
                self.target = target

            def section(self, _name):
                return 1, object()

            def symbols(self):
                original = (
                    ("func_80005750", 0x80005750, 4, 0, 1)
                    if self.target
                    else ("func_80005750", 0, 0, 0, rs.SHN_UNDEF)
                )
                return [
                    original,
                    ("middle", 0, 0, 0, rs.SHN_UNDEF),
                    ("final", 0, 0, 0, rs.SHN_UNDEF),
                ]

        resolved, ambiguous = rs._stable_symbol_identities(
            Path("missing"),
            FakeElf(),
            1,
            0x3578,
            FakeElf(target=True),
            {"middle": "func_80005750", "final": "func_80005750"},
        )
        self.assertEqual((0, 0x5300), resolved["final"])
        self.assertNotIn("final", ambiguous)

    def test_exact_surface_counts_shape_and_identity(self):
        target = [
            rs.SurfaceRecord(0x10, rs.R_MIPS_26, (0, 0x1234)),
            rs.SurfaceRecord(0x28, rs.R_MIPS_HI16, (7, 0x1BA8)),
            rs.SurfaceRecord(0x2C, rs.R_MIPS_LO16, (7, 0x1BA8)),
        ]
        result = rs.compare_record_sets(target, list(target))

        self.assertEqual(3, result["target_runtime_record_count"])
        self.assertEqual(3, result["candidate_record_count"])
        self.assertEqual(3, result["offset_type_alignment_count"])
        self.assertEqual(3, result["stable_identity_alignment_count"])
        self.assertTrue(result["offset_type_exact"])
        self.assertTrue(result["stable_identity_exact"])

    def test_shifted_surface_does_not_count_as_aligned(self):
        target = [rs.SurfaceRecord(0x10, rs.R_MIPS_26, (0, 0x1234))]
        candidate = [rs.SurfaceRecord(0x14, rs.R_MIPS_26, (0, 0x1234))]

        result = rs.compare_record_sets(target, candidate)

        self.assertEqual(0, result["offset_type_alignment_count"])
        self.assertEqual(0, result["stable_identity_alignment_count"])
        self.assertFalse(result["offset_type_exact"])
        self.assertFalse(result["stable_identity_exact"])

    def test_wrong_identity_preserves_shape_but_not_identity(self):
        target = [rs.SurfaceRecord(0x10, rs.R_MIPS_26, (7, 0xCCC))]
        candidate = [rs.SurfaceRecord(0x10, rs.R_MIPS_26, (59, 0x70))]

        result = rs.compare_record_sets(target, candidate)

        self.assertEqual(1, result["offset_type_alignment_count"])
        self.assertEqual(0, result["stable_identity_alignment_count"])
        self.assertTrue(result["offset_type_exact"])
        self.assertFalse(result["stable_identity_exact"])

    def test_ambiguous_overlay_owner_fails_closed(self):
        row = {
            "type": "c",
            "source": "overlays/o007/example",
            "offset": "0x0",
            "end_offset": "0x20",
            "size": "0x20",
        }
        atlas = {
            "modules": [
                {"overlay": 7, "text_ownership": [dict(row)]},
                {"overlay": 8, "text_ownership": [dict(row)]},
            ]
        }

        with self.assertRaisesRegex(rs.SurfaceComparisonError,
                                    "2 overlay text owners"):
            rs.resolve_overlay_ownership(
                Path("build/src/overlays/o007/example.c.o"), atlas)


class ResidentTargetRangeTests(unittest.TestCase):
    class FakeElf:
        def __init__(self, path, section_name, section_address, section_data,
                     symbols, relocations=()):
            self.path = Path(path)
            self.names = ["", section_name]
            self._section_name = section_name
            self._section_address = section_address
            self._section_data = bytes(section_data)
            self._symbols = list(symbols)
            self._relocations = list(relocations)

        def section(self, name):
            if name != self._section_name:
                return None, None
            return 1, (0, 0, 0, self._section_address, 0,
                       len(self._section_data), 0, 0, 0, 0)

        def section_bytes(self, name):
            return self._section_data if name == self._section_name else b""

        def symbols(self):
            return list(self._symbols)

        def relocations(self, target=r"\.text"):
            return list(self._relocations)

    def _fixture(self, repo):
        candidate = repo / "build_non_matching/src/main/fx.c.o"
        target_path = repo / "build/src/main/fx.c.o"
        candidate.parent.mkdir(parents=True)
        target_path.parent.mkdir(parents=True)
        candidate.write_bytes(b"candidate")
        target_path.write_bytes(b"target")

        object_start = 0x100
        size = 0x190
        names = [
            ("func_800498FC", object_start, size, 2, 1),
            ("D_800D5F58", 0, 0, 0, rs.SHN_UNDEF),
            ("func_80021FB0", 0, 0, 0, rs.SHN_UNDEF),
            ("camGetMode", 0, 0, 0, rs.SHN_UNDEF),
        ]
        relative = [
            (0x2C, rs.R_MIPS_HI16, 1),
            (0x30, rs.R_MIPS_LO16, 1),
            (0x88, rs.R_MIPS_26, 2),
            (0x9C, rs.R_MIPS_26, 3),
            (0xC4, rs.R_MIPS_26, 2),
        ]
        relocations = [
            (".text", object_start + offset, rtype, symbol_index)
            for offset, rtype, symbol_index in relative
        ]
        object_data = bytearray(object_start + size)
        linked_start = 0x80049800
        linked_value = 0x800498FC
        linked_data = bytearray(0x300)
        linked_offset = linked_value - linked_start
        linked_data[linked_offset:linked_offset + size] = object_data[
            object_start:object_start + size]
        addresses = {
            1: 0x800D5F58,
            2: 0x80021FB0,
            3: 0x80012340,
        }
        for offset, rtype, symbol_index in relative:
            address = addresses[symbol_index]
            if rtype == rs.R_MIPS_26:
                word = (0x03 << 26) | ((address >> 2) & 0x03FFFFFF)
            elif rtype == rs.R_MIPS_HI16:
                word = (0x0F << 26) | (((address + 0x8000) >> 16) & 0xFFFF)
            else:
                word = (0x0D << 26) | (address & 0xFFFF)
            struct.pack_into(">I", linked_data, linked_offset + offset, word)

        target_object = self.FakeElf(
            target_path, ".text", 0, object_data, names, relocations)
        linked_symbols = [
            ("func_800498FC", linked_value, size, 2, 1),
            ("D_800D5F58", 0x800D5F58, 4, 1, 1),
            ("func_80021FB0", 0x80021FB0, 4, 2, 1),
            ("camGetMode", 0x80012340, 4, 2, 1),
        ]
        linked = self.FakeElf(
            repo / "build/mickey.us.elf", ".main", linked_start,
            linked_data, linked_symbols)
        return candidate, target_path, target_object, linked, linked_value, size

    def test_func_800498fc_boundary_yields_five_static_tuples(self):
        with tempfile.TemporaryDirectory() as td:
            repo = Path(td)
            candidate, target_path, target_object, linked, value, size = \
                self._fixture(repo)

            def open_elf(path):
                self.assertEqual(target_path, Path(path))
                return target_object

            with mock.patch.object(rs, "REPO", repo), \
                    mock.patch.object(rs, "Elf", side_effect=open_elf):
                records = rs._resident_target_records(
                    candidate, "main/fx", linked, "func_800498FC",
                    value, size, ".main", repo / "missing-values.txt")

            self.assertEqual([0x2C, 0x30, 0x88, 0x9C, 0xC4],
                             [record.offset for record in records])
            self.assertTrue(all(record.identity is not None for record in records))
            self.assertEqual(records[0].identity, records[1].identity)

    def test_conflicting_source_assertion_fails_closed(self):
        with tempfile.TemporaryDirectory() as td:
            repo = Path(td)
            candidate, *_rest = self._fixture(repo)
            with mock.patch.object(rs, "REPO", repo):
                with self.assertRaisesRegex(rs.SurfaceComparisonError,
                                            "source is ambiguous"):
                    rs.resolve_resident_target_object(candidate, "main/other")

    def test_sparse_runtime_identity_overrides_matching_static_tuple(self):
        with tempfile.TemporaryDirectory() as td:
            repo = Path(td)
            candidate, _target_path, target_object, linked, value, size = \
                self._fixture(repo)
            runtime = [
                rs.SurfaceRecord(0x88, rs.R_MIPS_26, (7, 0x1234), 0)
            ]
            with mock.patch.object(rs, "REPO", repo), \
                    mock.patch.object(rs, "Elf", return_value=target_object):
                records = rs._resident_target_records(
                    candidate, None, linked, "func_800498FC", value, size,
                    ".main", repo / "missing-values.txt", runtime)

            by_shape = {(record.offset, record.rtype): record for record in records}
            self.assertEqual((7, 0x1234),
                             by_shape[(0x88, rs.R_MIPS_26)].identity)
            self.assertEqual(5, len(records))

    def test_missing_canonical_target_object_fails_closed(self):
        with tempfile.TemporaryDirectory() as td:
            repo = Path(td)
            candidate = repo / "build_non_matching/src/main/fx.c.o"
            candidate.parent.mkdir(parents=True)
            candidate.write_bytes(b"candidate")
            with mock.patch.object(rs, "REPO", repo):
                with self.assertRaisesRegex(rs.SurfaceComparisonError,
                                            "missing canonical resident"):
                    rs.resolve_resident_target_object(candidate)

    def test_nonrelocation_byte_mismatch_fails_closed(self):
        with tempfile.TemporaryDirectory() as td:
            repo = Path(td)
            candidate, target_path, target_object, linked, value, size = \
                self._fixture(repo)
            linked_data = bytearray(linked._section_data)
            linked_data[value - linked._section_address + 4] = 1
            bad_linked = self.FakeElf(
                linked.path, ".main", linked._section_address, linked_data,
                linked.symbols())

            with mock.patch.object(rs, "REPO", repo), \
                    mock.patch.object(rs, "Elf", return_value=target_object):
                with self.assertRaisesRegex(
                        rs.SurfaceComparisonError, "outside relocation words"):
                    rs._resident_target_records(
                        candidate, None, bad_linked, "func_800498FC",
                        value, size, ".main", repo / "missing-values.txt")


if __name__ == "__main__":
    unittest.main()
