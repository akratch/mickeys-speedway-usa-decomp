#!/usr/bin/env python3
"""Focused pass-side fixtures for clean-room decoder discriminators."""

import unittest

import cleanroom_detectors as D


class DecoderExclusionTests(unittest.TestCase):
    def test_address_shaped_identifiers_are_not_numeric_tokens(self) -> None:
        text = "\n".join(
            [
                "extern void func_80012340(void);",
                "extern int D_80081898;",
                "O101TailAB4CNode *gO101TailAB4CNode;",
            ]
            * 80
        )

        by_stage = D.normalize_words_by_stage(text)

        self.assertEqual(by_stage["hex-run"], [])
        self.assertEqual(by_stage["halves-pair"], [])

    def test_consecutive_source_lines_are_not_base_n_blocks(self) -> None:
        text = "\n".join(
            [
                "rumbleTick(updateRate);",
                "osContStartReadData(&D_800CF340);",
                "mmFree(D_800C95A8);",
                "mmFree(D_800C9D2C);",
            ]
        )

        by_stage = D.normalize_words_by_stage(text)

        self.assertEqual(by_stage["base-block"], [])

    def test_supported_numeric_token_shapes_remain_decodable(self) -> None:
        by_stage = D.normalize_words_by_stage(
            r"0x27bdffe0u 27bd_ffe0L \u27bd\uffe0 0x27bdffe0-0x27bdffe4."
        )

        self.assertIn(0x27BDFFE0, by_stage["hex-run"])
        self.assertGreaterEqual(by_stage["hex-run"].count(0x27BDFFE0), 3)

    def test_unprefixed_decimal_is_not_also_decoded_as_hex(self) -> None:
        text = "timestamp 1788409073 recorded"
        by_stage = D.normalize_words_by_stage(text)

        self.assertEqual(by_stage["hex-run"], [])
        self.assertEqual(by_stage["dec-token"], [int(text.split()[1])])


if __name__ == "__main__":
    unittest.main()
