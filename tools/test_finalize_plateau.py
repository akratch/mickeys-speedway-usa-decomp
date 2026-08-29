#!/usr/bin/env python3
"""Source-level tests for safe plateau finalization."""

from __future__ import annotations

import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


TOOLS = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOLS))

import finalize_plateau as plateau  # noqa: E402


VALID_SOURCE = """#include \"common.h\"

#ifdef NON_MATCHING
void demo_symbol(int value) {
#ifdef EXTRA_PATH
    value++;
#endif
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/demo/demo_symbol.s")
#endif
"""


class GuardValidationTests(unittest.TestCase):
    def test_requires_exact_guarded_candidate_and_fallback(self) -> None:
        candidate = plateau.require_guarded_candidate(VALID_SOURCE, "demo_symbol")
        self.assertEqual(candidate.fallback, "asm/nonmatchings/main/demo/demo_symbol.s")

    def test_rejects_unguarded_candidate(self) -> None:
        source = "void demo_symbol(void) {}\n"
        with self.assertRaisesRegex(plateau.PlateauError, "not an unambiguous"):
            plateau.require_guarded_candidate(source, "demo_symbol")

    def test_rejects_fallback_for_a_different_symbol(self) -> None:
        source = VALID_SOURCE.replace("demo_symbol.s", "other_symbol.s")
        with self.assertRaisesRegex(plateau.PlateauError, "exactly one"):
            plateau.require_guarded_candidate(source, "demo_symbol")

    def test_accepts_generated_overlay_fallback_for_friendly_symbol(self) -> None:
        generated = VALID_SOURCE.replace(
            "demo_symbol.s", "func_overlay_040_F0000690_1886F40.s"
        )
        candidate = plateau.require_guarded_candidate(generated, "demo_symbol")
        self.assertTrue(candidate.fallback.endswith("func_overlay_040_F0000690_1886F40.s"))

    def test_rejects_non_assembly_fallback_with_matching_stem(self) -> None:
        source = VALID_SOURCE.replace("demo_symbol.s", "demo_symbol.c")
        with self.assertRaisesRegex(plateau.PlateauError, "exactly one"):
            plateau.require_guarded_candidate(source, "demo_symbol")

    def test_handoff_is_fixed_field_and_idempotent(self) -> None:
        metrics = plateau.Metrics("98/101 words", "0x8", 10, "+0xC", "allocator mismatch")
        plateau.require_guarded_candidate(VALID_SOURCE, "demo_symbol")
        once = plateau.update_source(
            VALID_SOURCE, "demo_symbol", plateau.source_handoff("demo_symbol", metrics)
        )
        plateau.require_guarded_candidate(once, "demo_symbol")
        twice = plateau.update_source(
            once, "demo_symbol", plateau.source_handoff("demo_symbol", metrics)
        )
        self.assertEqual(once, twice)
        self.assertEqual(once.count("PLATEAU-HANDOFF"), 2)
        self.assertIn(" * relocations: 10\n", once)
        self.assertNotIn("|", once)

    def test_handoff_appends_without_changing_existing_bytes_or_lines(self) -> None:
        metrics = plateau.Metrics("98/101 words", "0x8", 10, "+0xC")
        plateau.require_guarded_candidate(VALID_SOURCE, "demo_symbol")
        updated = plateau.update_source(
            VALID_SOURCE, "demo_symbol", plateau.source_handoff("demo_symbol", metrics)
        )
        self.assertTrue(updated.startswith(VALID_SOURCE))
        self.assertEqual(
            updated.splitlines()[:len(VALID_SOURCE.splitlines())],
            VALID_SOURCE.splitlines(),
        )
        self.assertGreater(updated.index("PLATEAU-HANDOFF"), updated.index("#endif"))

    def test_updates_one_symbol_in_a_multi_symbol_eof_suffix(self) -> None:
        first = plateau.Metrics("98/101 words", "0x8", 10, "+0xC")
        second = plateau.Metrics("7/8 words", "frameless", 0, "+0x4")
        plateau.require_guarded_candidate(VALID_SOURCE, "demo_symbol")
        with_first = plateau.update_source(
            VALID_SOURCE, "demo_symbol", plateau.source_handoff("demo_symbol", first)
        )
        with_both = plateau.update_source(
            with_first, "other_symbol", plateau.source_handoff("other_symbol", second)
        )
        revised = plateau.update_source(
            with_both,
            "demo_symbol",
            plateau.source_handoff(
                "demo_symbol", plateau.Metrics("99/101 words", "0x8", 10, "+0x10")
            ),
        )
        self.assertTrue(revised.startswith(VALID_SOURCE))
        self.assertIn("99/101 words", revised)
        self.assertNotIn("98/101 words", revised)
        self.assertIn("7/8 words", revised)
        self.assertEqual(revised.count("PLATEAU-HANDOFF:demo_symbol:start"), 1)
        self.assertEqual(revised.count("PLATEAU-HANDOFF:other_symbol:start"), 1)

    def test_refuses_legacy_inline_handoff_instead_of_moving_source(self) -> None:
        legacy = VALID_SOURCE.replace(
            "#ifdef NON_MATCHING\n",
            "#ifdef NON_MATCHING\n/* PLATEAU-HANDOFF\n * symbol: demo_symbol\n */\n",
        )
        plateau.require_guarded_candidate(legacy, "demo_symbol")
        with self.assertRaisesRegex(plateau.PlateauError, "move measured source lines"):
            plateau.update_source(
                legacy,
                "demo_symbol",
                plateau.source_handoff(
                    "demo_symbol", plateau.Metrics("98/101 words", "0x8", 10, "+0xC")
                ),
            )

    def test_markdown_handoff_replaces_its_own_block(self) -> None:
        first = plateau.Metrics("98/101 words", "0x8", 10, "+0xC")
        second = plateau.Metrics("99/101 words", "0x8", 10, "+0x10")
        text = "# Ledger\n"
        text = plateau.update_markdown(
            text, "demo_symbol", plateau.markdown_handoff("demo_symbol", "src/demo.c", first)
        )
        text = plateau.update_markdown(
            text, "demo_symbol", plateau.markdown_handoff("demo_symbol", "src/demo.c", second)
        )
        self.assertEqual(text.count("plateau-handoff:demo_symbol:start"), 1)
        self.assertIn("99/101 words", text)
        self.assertNotIn("98/101 words", text)


class FinalizeCommandTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.base = Path(self.temporary.name)
        self.repo = self.base / "repo"
        self.repo.mkdir()
        self.run_command("git", "init", "-q")
        self.run_command("git", "config", "user.email", "plateau@example.invalid")
        self.run_command("git", "config", "user.name", "Plateau Test")
        (self.repo / "src").mkdir()
        (self.repo / "docs").mkdir()
        (self.repo / "src" / "demo.c").write_text(VALID_SOURCE, encoding="utf-8")
        (self.repo / "docs" / "handoff.md").write_text("# Handoffs\n", encoding="utf-8")
        (self.repo / "other.txt").write_text("clean\n", encoding="utf-8")
        self.run_command("git", "add", ".")
        self.run_command("git", "commit", "-q", "-m", "initial")

        self.bin = self.base / "bin"
        self.bin.mkdir()
        self.gate_log = self.base / "gates.log"
        fake_gmake = self.bin / "gmake"
        fake_gmake.write_text(
            "#!/bin/sh\nprintf '%s\\n' \"$*\" >> \"$GATE_LOG\"\n",
            encoding="utf-8",
        )
        fake_gmake.chmod(0o755)
        self.env = os.environ.copy()
        self.env["PATH"] = f"{self.bin}{os.pathsep}{self.env['PATH']}"
        self.env["GATE_LOG"] = str(self.gate_log)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def run_command(self, *command: str, check: bool = True) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            command, cwd=self.repo, env=getattr(self, "env", None), text=True,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=check,
        )

    def finalize(self, *extra: str) -> subprocess.CompletedProcess[str]:
        command = (
            sys.executable,
            str(TOOLS / "finalize_plateau.py"),
            "demo_symbol",
            "src/demo.c",
            "--score", "98/101 words",
            "--frame", "0x8",
            "--relocations", "10",
            "--first-mismatch", "+0xC",
            *extra,
        )
        return self.run_command(*command, check=False)

    def test_without_commit_records_handoff_and_runs_only_source_gates(self) -> None:
        before = self.run_command("git", "rev-list", "--count", "HEAD").stdout.strip()
        result = self.finalize()
        after = self.run_command("git", "rev-list", "--count", "HEAD").stdout.strip()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(before, after)
        self.assertIn("commit: not requested", result.stdout)
        self.assertIn("PLATEAU-HANDOFF", (self.repo / "src" / "demo.c").read_text())
        self.assertTrue((self.repo / "src" / "demo.c").read_text().startswith(VALID_SOURCE))
        self.assertEqual(self.gate_log.read_text().splitlines(), ["cleanroom", "check-docs"])

    def test_explicit_commit_contains_only_named_source_and_doc(self) -> None:
        result = self.finalize(
            "--handoff-doc", "docs/handoff.md", "--summary", "one allocator web remains",
            "--commit",
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        changed = self.run_command(
            "git", "show", "--pretty=format:", "--name-only", "HEAD"
        ).stdout.splitlines()
        self.assertEqual(set(changed), {"src/demo.c", "docs/handoff.md"})
        self.assertIn("plateau-handoff:demo_symbol:start", (self.repo / "docs/handoff.md").read_text())

    def test_refuses_unrelated_dirt_before_writing(self) -> None:
        (self.repo / "other.txt").write_text("dirty\n", encoding="utf-8")
        before = (self.repo / "src" / "demo.c").read_text(encoding="utf-8")
        result = self.finalize()
        self.assertEqual(result.returncode, 2)
        self.assertIn("unrelated worktree/index dirt", result.stderr)
        self.assertEqual((self.repo / "src" / "demo.c").read_text(encoding="utf-8"), before)
        self.assertFalse(self.gate_log.exists())

    def test_rejects_unguarded_source_before_gates(self) -> None:
        (self.repo / "src" / "demo.c").write_text(
            "void demo_symbol(void) {}\n", encoding="utf-8"
        )
        result = self.finalize()
        self.assertEqual(result.returncode, 2)
        self.assertIn("not an unambiguous", result.stderr)
        self.assertFalse(self.gate_log.exists())

    def test_refuses_legacy_inline_handoff_before_gates(self) -> None:
        legacy = VALID_SOURCE.replace(
            "#ifdef NON_MATCHING\n",
            "#ifdef NON_MATCHING\n/* PLATEAU-HANDOFF\n * symbol: demo_symbol\n */\n",
        )
        (self.repo / "src" / "demo.c").write_text(legacy, encoding="utf-8")
        result = self.finalize()
        self.assertEqual(result.returncode, 2)
        self.assertIn("move measured source lines", result.stderr)
        self.assertEqual((self.repo / "src" / "demo.c").read_text(), legacy)
        self.assertFalse(self.gate_log.exists())


if __name__ == "__main__":
    unittest.main()
