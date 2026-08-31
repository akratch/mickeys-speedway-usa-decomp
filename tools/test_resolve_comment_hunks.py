#!/usr/bin/env python3
"""Regression tests for comment-only merge conflict resolution."""

from pathlib import Path
import sys
import tempfile
import unittest


TOOLS = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOLS))

import resolve_comment_hunks as resolver  # noqa: E402


class ResolveCommentHunksTests(unittest.TestCase):
    def resolve(self, text: str) -> tuple[int, str]:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "source.c"
            path.write_text(text)
            remaining = resolver.resolve(str(path))
            return remaining, path.read_text()

    def test_preserves_distinct_eof_plateau_blocks(self) -> None:
        conflict = "int value;\n" + "<" * 7 + " HEAD\n" + """\
/* PLATEAU-HANDOFF:first:start
 * score: 9/10 words
 * PLATEAU-HANDOFF:first:end
""" + "=" * 7 + "\n" + """\
/* PLATEAU-HANDOFF:second:start
 * score: 8/10 words
 * PLATEAU-HANDOFF:second:end
""" + ">" * 7 + " lane/second\n" + """\
 */
"""
        remaining, resolved = self.resolve(conflict)
        self.assertEqual(remaining, 0)
        self.assertNotIn("<" * 7, resolved)
        self.assertEqual(resolved.count("/* PLATEAU-HANDOFF:"), 2)
        self.assertEqual(resolved.count(" */"), 2)
        self.assertNotIn(":end */", resolved)
        self.assertIn("second:end\n */", resolved)
        self.assertLess(resolved.index("first:start"), resolved.index("second:start"))

    def test_replaces_same_symbol_plateau_with_incoming_revision(self) -> None:
        conflict = "<" * 7 + " HEAD\n" + """\
/* PLATEAU-HANDOFF:same:start
 * score: 8/10 words
 * PLATEAU-HANDOFF:same:end
""" + "=" * 7 + "\n" + """\
/* PLATEAU-HANDOFF:same:start
 * score: 9/10 words
 * PLATEAU-HANDOFF:same:end
""" + ">" * 7 + " lane/same\n" + """\
 */
"""
        remaining, resolved = self.resolve(conflict)
        self.assertEqual(remaining, 0)
        self.assertNotIn("8/10 words", resolved)
        self.assertIn("9/10 words", resolved)

    def test_preserves_existing_sequence_before_new_plateau(self) -> None:
        conflict = "<" * 7 + " HEAD\n" + """\
/* PLATEAU-HANDOFF:first:start
 * score: 9/10 words
 * PLATEAU-HANDOFF:first:end
 */

/* PLATEAU-HANDOFF:second:start
 * score: 8/10 words
 * PLATEAU-HANDOFF:second:end
""" + "=" * 7 + "\n" + """\
/* PLATEAU-HANDOFF:third:start
 * score: 7/10 words
 * PLATEAU-HANDOFF:third:end
""" + ">" * 7 + " lane/third\n" + """\
 */
"""
        remaining, resolved = self.resolve(conflict)
        self.assertEqual(remaining, 0)
        self.assertEqual(resolved.count("/* PLATEAU-HANDOFF:"), 3)
        self.assertEqual(resolved.count(" */"), 3)
        self.assertLess(resolved.index("first:start"), resolved.index("second:start"))
        self.assertLess(resolved.index("second:start"), resolved.index("third:start"))

    def test_preserves_unique_complete_blocks_and_replaces_overlap(self) -> None:
        conflict = "<" * 7 + " HEAD\n" + """\
/* PLATEAU-HANDOFF:keep:start
 * score: 9/10 words
 * PLATEAU-HANDOFF:keep:end
 */

/* PLATEAU-HANDOFF:update:start
 * score: 8/10 words
 * PLATEAU-HANDOFF:update:end
 */
""" + "=" * 7 + "\n" + """\
/* PLATEAU-HANDOFF:update:start
 * score: 10/10 words
 * PLATEAU-HANDOFF:update:end
 */

/* PLATEAU-HANDOFF:add:start
 * score: 7/10 words
 * PLATEAU-HANDOFF:add:end
 */
""" + ">" * 7 + " lane/update\n"
        remaining, resolved = self.resolve(conflict)
        self.assertEqual(remaining, 0)
        self.assertIn("keep:start", resolved)
        self.assertIn("add:start", resolved)
        self.assertNotIn("8/10 words", resolved)
        self.assertIn("10/10 words", resolved)
        self.assertEqual(resolved.count("/* PLATEAU-HANDOFF:"), 3)


if __name__ == "__main__":
    unittest.main()
