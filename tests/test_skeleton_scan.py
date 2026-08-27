#!/usr/bin/env python3
"""Unit tests for tools/skeleton_scan.py's masking function.

Uses only synthetic, hand-built instruction words -- nothing derived from the
baserom or any reference build. See docs/CLEANROOM.md: nothing ROM-derived is ever
tracked in git, including in test fixtures.

Run: python3 -m unittest tests/test_skeleton_scan.py -v
"""
import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))

import skeleton_scan as ss  # noqa: E402


def word(op, rest_bits):
    """Build a synthetic big-endian MIPS-shaped word: op in bits 31-26,
    remaining 26 bits supplied directly (already positioned by the caller).
    Not a real instruction encoding from any ROM -- just enough bit layout
    to exercise mask_word's field extraction."""
    return ((op & 0x3F) << 26) | (rest_bits & 0x3FFFFFF)


class TestMaskWord(unittest.TestCase):
    def test_special_keeps_funct_only(self):
        # op=0 (SPECIAL): funct field is bits 5-0. Two words with the same
        # funct but different rd/rs/rt/shamt must mask identically.
        w1 = word(0, (1 << 21) | (2 << 16) | (3 << 11) | (4 << 6) | 0x20)  # funct 0x20 (add)
        w2 = word(0, (7 << 21) | (8 << 16) | (9 << 11) | (0 << 6) | 0x20)
        self.assertEqual(ss.mask_word(w1), ss.mask_word(w2))
        # Different funct must mask differently.
        w3 = word(0, (1 << 21) | (2 << 16) | (3 << 11) | (4 << 6) | 0x22)  # funct 0x22 (sub)
        self.assertNotEqual(ss.mask_word(w1), ss.mask_word(w3))

    def test_regimm_keeps_rt_suboppcode(self):
        # op=1 (REGIMM): rt (bits 20-16) selects bltz/bgez/etc; rs and the
        # immediate must not affect the mask.
        w1 = word(1, (5 << 21) | (0 << 16) | 0x1234)   # rt=0 (bltz)
        w2 = word(1, (9 << 21) | (0 << 16) | 0x0001)
        self.assertEqual(ss.mask_word(w1), ss.mask_word(w2))
        w3 = word(1, (5 << 21) | (1 << 16) | 0x1234)   # rt=1 (bgez)
        self.assertNotEqual(ss.mask_word(w1), ss.mask_word(w3))

    def test_cop1_arithmetic_keeps_fmt_and_funct(self):
        # op=0x11 (COP1), fmt in bits 25-21 >= 0x10 selects single/double
        # arithmetic; funct (bits 5-0) selects add/sub/mul/etc.
        fmt_single = 0x10
        w1 = word(0x11, (fmt_single << 21) | (1 << 16) | (2 << 11) | (3 << 6) | 0x00)  # add.s
        w2 = word(0x11, (fmt_single << 21) | (9 << 16) | (8 << 11) | (7 << 6) | 0x00)
        self.assertEqual(ss.mask_word(w1), ss.mask_word(w2))
        w3 = word(0x11, (fmt_single << 21) | (1 << 16) | (2 << 11) | (3 << 6) | 0x01)  # sub.s
        self.assertNotEqual(ss.mask_word(w1), ss.mask_word(w3))
        fmt_double = 0x11
        w4 = word(0x11, (fmt_double << 21) | (1 << 16) | (2 << 11) | (3 << 6) | 0x00)  # add.d
        self.assertNotEqual(ss.mask_word(w1), ss.mask_word(w4))

    def test_cop1_move_keeps_fmt_only(self):
        # fmt < 0x10 is the mfc1/mtc1/bc1 family: only fmt distinguishes them,
        # the rest of the word (rt, fs, etc.) is masked away.
        fmt_mtc1 = 0x04
        w1 = word(0x11, (fmt_mtc1 << 21) | (1 << 16) | (2 << 11))
        w2 = word(0x11, (fmt_mtc1 << 21) | (9 << 16) | (8 << 11))
        self.assertEqual(ss.mask_word(w1), ss.mask_word(w2))
        fmt_mfc1 = 0x00
        w3 = word(0x11, (fmt_mfc1 << 21) | (1 << 16) | (2 << 11))
        self.assertNotEqual(ss.mask_word(w1), ss.mask_word(w3))

    def test_ordinary_opcode_keeps_op_only(self):
        # Any other primary opcode (e.g. addiu=0x09): immediates, rs, rt are
        # all masked away, only the opcode field survives.
        op = 0x09
        w1 = word(op, (1 << 21) | (2 << 16) | 0x0001)
        w2 = word(op, (31 << 21) | (30 << 16) | 0x7FFF)
        self.assertEqual(ss.mask_word(w1), ss.mask_word(w2))
        w3 = word(op + 1, (1 << 21) | (2 << 16) | 0x0001)
        self.assertNotEqual(ss.mask_word(w1), ss.mask_word(w3))

    def test_mask_word_fits_in_16_bits(self):
        for op in range(0, 0x40):
            for rest in (0, 0x3FFFFFF, 0x1234567):
                m = ss.mask_word(word(op, rest))
                self.assertGreaterEqual(m, 0)
                self.assertLess(m, 0x10000)


class TestMaskedBytes(unittest.TestCase):
    def test_masks_each_word_to_two_bytes(self):
        words = [word(0x09, (1 << 21) | 0x0001), word(0x09, (2 << 21) | 0x0002)]
        data = b"".join(struct.pack(">I", w) for w in words)
        mb = ss.masked_bytes(data)
        self.assertEqual(len(mb), 4)  # 2 words -> 2 * 2 bytes
        # Same opcode in both words -> identical masked halves.
        self.assertEqual(mb[0:2], mb[2:4])

    def test_drops_trailing_partial_word(self):
        w = word(0x09, 0x1234)
        data = struct.pack(">I", w) + b"\x01\x02"  # 6 bytes: one full word + 2 stray bytes
        mb = ss.masked_bytes(data)
        self.assertEqual(len(mb), 2)

    def test_register_allocation_is_invisible(self):
        # The whole point of masking: two "functions" that differ only in
        # register choice must produce byte-identical masked skeletons.
        def make(regs):
            r0, r1, r2 = regs
            return b"".join(struct.pack(">I", w) for w in [
                word(0x09, (r0 << 21) | (r1 << 16) | 0x0010),   # addiu
                word(0, (r1 << 21) | (r2 << 16) | (r0 << 11) | 0x20),  # add (SPECIAL/funct)
                word(0x23, (r0 << 21) | (r2 << 16) | 0x0004),   # lw
            ])
        fn_a = make((4, 5, 6))
        fn_b = make((16, 17, 18))
        self.assertEqual(ss.masked_bytes(fn_a), ss.masked_bytes(fn_b))

    def test_different_shape_diverges(self):
        def make(funct):
            return struct.pack(">I", word(0, (4 << 21) | (5 << 16) | (6 << 11) | funct))
        self.assertNotEqual(ss.masked_bytes(make(0x20)), ss.masked_bytes(make(0x22)))


if __name__ == "__main__":
    unittest.main()
