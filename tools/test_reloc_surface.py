#!/usr/bin/env python3
"""Focused source-level regression tests for reloc_surface safety gates."""

from __future__ import annotations

import contextlib
import io
import json
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


if __name__ == "__main__":
    unittest.main()
