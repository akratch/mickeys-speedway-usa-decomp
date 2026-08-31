#!/usr/bin/env python3
"""Unit tests for the match-regression merge guard."""

import unittest

import check_match_regression


class MixedOverlaySourcesTests(unittest.TestCase):
    def test_collects_only_explicit_mixed_tu_sources(self):
        atlas = {
            "modules": [
                {
                    "overlay": 56,
                    "mixed_tu_exact_c_ranges": [
                        {
                            "source": "overlays/o056/overlay_056",
                            "offset": "0x0",
                            "size": "0x5C",
                        }
                    ],
                },
                {"overlay": 57, "text_ownership": []},
            ]
        }

        self.assertEqual(
            check_match_regression.mixed_overlay_sources(atlas),
            {"src/overlays/o056/overlay_056.c"},
        )

    def test_ignores_malformed_rows_without_a_source(self):
        atlas = {
            "modules": [
                {"mixed_tu_exact_c_ranges": [{"offset": "0x0"}]},
            ]
        }

        self.assertEqual(check_match_regression.mixed_overlay_sources(atlas), set())


if __name__ == "__main__":
    unittest.main()
