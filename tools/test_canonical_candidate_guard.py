#!/usr/bin/env python3

import tempfile
import unittest
from pathlib import Path

import canonical_candidate_guard as guard


class CanonicalCandidateGuardTests(unittest.TestCase):
    def test_receipt_selects_only_changed_or_missing_objects(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = root / "build" / "receipt.json"
            first = root / "build" / "first.o"
            second = root / "build" / "second.o"
            first.parent.mkdir()
            first.write_bytes(b"canonical-first")
            second.write_bytes(b"canonical-second")

            self.assertEqual(
                guard.dirty_objects(manifest, [first, second]), [first, second]
            )
            guard.write_receipt(manifest, [first, second])
            self.assertEqual(guard.dirty_objects(manifest, [first, second]), [])

            first.write_bytes(b"candidate-first")
            second.unlink()
            self.assertEqual(
                guard.dirty_objects(manifest, [first, second]), [first, second]
            )

    def test_malformed_receipt_fails_safe(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = root / "receipt.json"
            obj = root / "candidate.o"
            manifest.write_text("not json", encoding="utf-8")
            obj.write_bytes(b"object")
            self.assertEqual(guard.dirty_objects(manifest, [obj]), [obj])

    def test_write_rejects_missing_objects(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with self.assertRaises(SystemExit):
                guard.write_receipt(root / "receipt.json", [root / "missing.o"])


if __name__ == "__main__":
    unittest.main()
