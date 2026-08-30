#!/usr/bin/env python3
"""Focused synthetic tests for skeleton-scan target resolution."""

import sys
import unittest
from pathlib import Path
from unittest import mock


sys.path.insert(0, str(Path(__file__).resolve().parent))

import skeleton_scan  # noqa: E402


def mixed(start, end, *, label="exactFunction", source="overlays/o008/example"):
    return {
        "offset": hex(start),
        "end_offset": hex(end),
        "size": hex(end - start),
        "label": label,
        "source": source,
        "evidence": "synthetic linked exact proof",
    }


def atlas(*, ownership=None, mixed_ranges=None, duplicate_module=False):
    module = {
        "overlay": 8,
        "text_ownership": ownership or [],
        "mixed_tu_exact_c_ranges": mixed_ranges or [],
    }
    modules = [module, dict(module)] if duplicate_module else [module]
    return {"modules": modules}


def region(body, matched):
    return [{"overlay": 8, "rom_start": 0x1000, "body": body, "matched": matched}]


class OverlayTargetResolutionTests(unittest.TestCase):
    def resolve(self, state, body, matched, target="8:+0x0"):
        with mock.patch.object(
            skeleton_scan, "overlay_regions", return_value=region(body, matched)
        ):
            return skeleton_scan.resolve_target_bytes(target, {"atlas": state})

    def test_exact_mixed_range_overrides_coarse_ownership_at_same_start(self):
        body = bytes(range(64))
        state = atlas(mixed_ranges=[mixed(0, 8)])

        label, resolved = self.resolve(state, body, [(0, 64)])

        self.assertEqual("o008+0x0", label)
        self.assertEqual(body[:8], resolved)

    def test_falls_back_to_unique_text_ownership_start(self):
        body = bytes(range(64))

        label, resolved = self.resolve(atlas(), body, [(16, 32)], target="8:+0x10")

        self.assertEqual("o008+0x10", label)
        self.assertEqual(body[16:32], resolved)

    def test_duplicate_mixed_start_is_ambiguous(self):
        state = atlas(mixed_ranges=[mixed(0, 8), mixed(0, 12, label="duplicate")])

        with self.assertRaisesRegex(SystemExit, "ambiguous mixed-TU exact identity"):
            self.resolve(state, bytes(range(64)), [(0, 64)])

    def test_inconsistent_mixed_extent_is_not_function_sized(self):
        row = mixed(0, 8)
        row["size"] = "0xC"

        with self.assertRaisesRegex(SystemExit, "not one unambiguous function-sized range"):
            self.resolve(atlas(mixed_ranges=[row]), bytes(range(64)), [(0, 64)])

    def test_unaligned_mixed_extent_is_not_function_sized(self):
        row = mixed(0, 6)

        with self.assertRaisesRegex(SystemExit, "not one unambiguous function-sized range"):
            self.resolve(atlas(mixed_ranges=[row]), bytes(range(64)), [(0, 64)])

    def test_duplicate_overlay_module_is_ambiguous(self):
        with self.assertRaisesRegex(SystemExit, "ambiguous module identity"):
            self.resolve(atlas(duplicate_module=True), bytes(range(64)), [])


if __name__ == "__main__":
    unittest.main()
