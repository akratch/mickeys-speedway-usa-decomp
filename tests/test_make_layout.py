#!/usr/bin/env python3
"""Keep the host build graph separate from per-overlay object policy."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parent.parent
MAKEFILE = ROOT / "Makefile"
OVERLAY_POLICY = ROOT / "mk" / "overlays.mk"


class MakeLayoutTests(unittest.TestCase):
    def test_root_includes_overlay_policy_once(self) -> None:
        text = MAKEFILE.read_text(encoding="utf-8")
        self.assertEqual(1, text.count("include mk/overlays.mk"))
        self.assertNotRegex(
            text,
            re.compile(
                r"^\$\(BUILD_DIR\)/\$\(SRC_DIR\)/overlays/",
                re.MULTILINE,
            ),
        )

    def test_overlay_policy_contains_no_link_graph(self) -> None:
        text = OVERLAY_POLICY.read_text(encoding="utf-8")
        self.assertIn(
            "$(BUILD_DIR)/$(SRC_DIR)/overlays/%.c.o: MIPSISET :=",
            text,
        )
        self.assertNotIn("$(TARGET).elf:", text)
        self.assertNotIn("mips64-elf-ld", text)

    def test_root_owns_the_single_final_link(self) -> None:
        text = MAKEFILE.read_text(encoding="utf-8")
        self.assertEqual(1, len(re.findall(r"^\$\(TARGET\)\.elf:", text, re.MULTILINE)))


if __name__ == "__main__":
    unittest.main()
