#!/usr/bin/env python3
"""
Unit tests for tools/flag_sweep.py's scoring function, score_words().

Deliberately synthetic word arrays only -- no compiler, no ROM, no
asm/nonmatchings. score_words() is a pure function of int sequences and a
mask dict, so this file exercises it without needing a baserom or a build,
and without touching anything ROM-derived.

Run with:  .venv/bin/python -m pytest tests/test_flag_sweep.py -q
       or: .venv/bin/python tests/test_flag_sweep.py
"""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))

from flag_sweep import Score, rank_key, score_words  # noqa: E402


class TestScoreWords(unittest.TestCase):
    def test_identical_arrays_are_exact(self):
        words = [0x27BDFFC0, 0xAFBF001C, 0x03E00008, 0x00000000]
        sc = score_words(words, list(words))
        self.assertTrue(sc.exact)
        self.assertEqual(sc.size_delta, 0)
        self.assertEqual(sc.diff_words, 0)
        self.assertIsNone(sc.first_mismatch)

    def test_single_word_difference(self):
        target = [0x11111111, 0x22222222, 0x33333333]
        candidate = [0x11111111, 0xDEADBEEF, 0x33333333]
        sc = score_words(target, candidate)
        self.assertFalse(sc.exact)
        self.assertEqual(sc.size_delta, 0)
        self.assertEqual(sc.diff_words, 1)
        self.assertEqual(sc.first_mismatch, 4)  # byte offset of word index 1

    def test_first_mismatch_is_the_earliest_one(self):
        target = [1, 2, 3, 4, 5]
        candidate = [1, 2, 30, 4, 50]
        sc = score_words(target, candidate)
        self.assertEqual(sc.diff_words, 2)
        self.assertEqual(sc.first_mismatch, 8)  # word index 2 -> byte 8

    def test_candidate_longer_counts_extra_words_and_size_delta(self):
        target = [1, 2, 3]
        candidate = [1, 2, 3, 4, 5]
        sc = score_words(target, candidate)
        self.assertFalse(sc.exact)
        self.assertEqual(sc.size_delta, 8)  # two extra words * 4 bytes
        self.assertEqual(sc.diff_words, 2)
        # the shared prefix agreed; first_mismatch lands at the first extra word
        self.assertEqual(sc.first_mismatch, 12)

    def test_candidate_shorter_counts_missing_words_and_size_delta(self):
        target = [1, 2, 3, 4, 5]
        candidate = [1, 2, 3]
        sc = score_words(target, candidate)
        self.assertFalse(sc.exact)
        self.assertEqual(sc.size_delta, -8)
        self.assertEqual(sc.diff_words, 2)

    def test_size_and_content_differ_together(self):
        target = [1, 2, 3]
        candidate = [1, 99, 3, 4]
        sc = score_words(target, candidate)
        self.assertEqual(sc.size_delta, 4)
        # one content mismatch (word 1) + one extra word (word 3)
        self.assertEqual(sc.diff_words, 2)
        self.assertEqual(sc.first_mismatch, 4)

    def test_mask_hides_relocation_site_low16(self):
        # A %hi/%lo pair: target has the real address baked in, the
        # candidate (an unlinked object) has a placeholder immediate. Same
        # opcode/base-register bits, different low 16 bits -> masked equal.
        target = [0x3C048006]  # lui $a0, 0x8006
        candidate = [0x3C040000]  # lui $a0, 0x0000 (unresolved)
        masks = {0: 0x0000FFFF}
        sc = score_words(target, candidate, masks)
        self.assertTrue(sc.exact)

    def test_mask_does_not_hide_a_real_opcode_difference(self):
        # Differ outside the masked low 16 bits (e.g. a different base
        # register or a different opcode entirely) -> still a mismatch.
        target = [0x3C048006]  # lui $a0, ...
        candidate = [0x3C058006]  # lui $a1, ... (same immediate, wrong reg)
        masks = {0: 0x0000FFFF}
        sc = score_words(target, candidate, masks)
        self.assertFalse(sc.exact)
        self.assertEqual(sc.diff_words, 1)

    def test_mask_applies_only_to_the_word_it_names(self):
        target = [0x3C048006, 0x11111111]
        candidate = [0x3C040000, 0x22222222]
        masks = {0: 0x0000FFFF}  # word 1 is not masked
        sc = score_words(target, candidate, masks)
        self.assertFalse(sc.exact)
        self.assertEqual(sc.diff_words, 1)
        self.assertEqual(sc.first_mismatch, 4)

    def test_empty_arrays_are_exact(self):
        sc = score_words([], [])
        self.assertTrue(sc.exact)
        self.assertEqual(sc.size_delta, 0)
        self.assertEqual(sc.diff_words, 0)


class TestRankKey(unittest.TestCase):
    def test_exact_beats_inexact_regardless_of_size(self):
        exact = Score(exact=True, size_delta=100, diff_words=0, first_mismatch=None)
        inexact = Score(exact=False, size_delta=0, diff_words=1, first_mismatch=40)
        self.assertLess(rank_key(exact), rank_key(inexact))

    def test_fewer_diff_words_ranks_better(self):
        a = Score(exact=False, size_delta=0, diff_words=1, first_mismatch=0)
        b = Score(exact=False, size_delta=0, diff_words=5, first_mismatch=0)
        self.assertLess(rank_key(a), rank_key(b))

    def test_smaller_size_delta_breaks_ties(self):
        a = Score(exact=False, size_delta=4, diff_words=2, first_mismatch=0)
        b = Score(exact=False, size_delta=40, diff_words=2, first_mismatch=0)
        self.assertLess(rank_key(a), rank_key(b))

    def test_later_first_mismatch_breaks_remaining_ties(self):
        # Matching for longer before diverging ranks better.
        early = Score(exact=False, size_delta=0, diff_words=1, first_mismatch=4)
        late = Score(exact=False, size_delta=0, diff_words=1, first_mismatch=40)
        self.assertLess(rank_key(late), rank_key(early))

    def test_sorting_a_mixed_list(self):
        scores = [
            Score(False, 0, 3, 0),
            Score(True, 0, 0, None),
            Score(False, 4, 1, 12),
            Score(False, 0, 1, 4),
        ]
        ordered = sorted(scores, key=rank_key)
        self.assertTrue(ordered[0].exact)
        # among the non-exact rows, diff_words=1 (offset 4 or size_delta 4)
        # beats diff_words=3
        self.assertEqual(ordered[-1].diff_words, 3)


if __name__ == "__main__":
    unittest.main()
