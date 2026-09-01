#!/usr/bin/env python3
"""Focused synthetic tests for skeleton-scan target resolution."""

import sys
import tempfile
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


def owner(start=0, end=64, *, matched=True, source="overlays/o008/example"):
    return {
        "offset": hex(start),
        "end_offset": hex(end),
        "size": hex(end - start),
        "type": "c",
        "source": source,
        "matched": matched,
    }


def atlas(*, ownership=None, mixed_ranges=None, duplicate_module=False):
    module = {
        "overlay": 8,
        "text_ownership": [owner()] if ownership is None else ownership,
        "mixed_tu_exact_c_ranges": mixed_ranges or [],
    }
    modules = [module, dict(module)] if duplicate_module else [module]
    return {"modules": modules}


def region(body, matched):
    return [{"overlay": 8, "rom_start": 0x1000, "body": body, "matched": matched}]


class OverlayTargetResolutionTests(unittest.TestCase):
    def resolve(self, state, body, matched, target="8:+0x0", *, asm_root=None):
        kwargs = {} if asm_root is None else {"asm_root": asm_root}
        with mock.patch.object(
            skeleton_scan, "overlay_regions", return_value=region(body, matched)
        ):
            return skeleton_scan.resolve_target_bytes(
                target, {"atlas": state}, **kwargs
            )

    def fallback(self, root, offset, addresses, *, suffix="ABC", after=None):
        symbol = f"func_overlay_008_{skeleton_scan.SYNTHETIC_VMA + offset:08X}_{suffix}"
        path = root / "overlays/o008/example" / f"{symbol}.s"
        path.parent.mkdir(parents=True, exist_ok=True)
        rows = [
            f"    /* 1000 {address:08X} 00000000 */  synthetic"
            for address in addresses
        ]
        path.write_text(
            "\n".join([f"glabel {symbol}", *rows, f"endlabel {symbol}", *(after or [])])
            + "\n",
            encoding="utf-8",
        )
        return path

    def test_exact_mixed_range_overrides_coarse_ownership_at_same_start(self):
        body = bytes(range(64))
        state = atlas(mixed_ranges=[mixed(0, 8)])

        label, resolved = self.resolve(state, body, [(0, 64)])

        self.assertEqual("o008+0x0", label)
        self.assertEqual(body[:8], resolved)

    def test_falls_back_to_unique_text_ownership_start(self):
        body = bytes(range(64))

        state = atlas(ownership=[owner(16, 32)])
        label, resolved = self.resolve(state, body, [(16, 32)], target="8:+0x10")

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

    def test_overlapping_text_owners_block_raw_fallback(self):
        state = atlas(ownership=[owner(), owner(8, 32, source="overlays/o008/other")])
        with tempfile.TemporaryDirectory() as directory:
            asm_root = Path(directory)
            self.fallback(asm_root, 0x10, [0xF0000010, 0xF0000014])

            with self.assertRaisesRegex(SystemExit, "2 containing text_ownership rows"):
                self.resolve(
                    state, bytes(range(64)), [], target="8:+0x10", asm_root=asm_root
                )

    def test_atlas_absent_identity_uses_unique_raw_fallback_boundary(self):
        body = bytes(range(64))
        with tempfile.TemporaryDirectory() as directory:
            asm_root = Path(directory)
            self.fallback(
                asm_root,
                0x10,
                [0xF0000010, 0xF0000014, 0xF0000018],
                after=["    /* 100C F000001C 00000000 */  padding"],
            )

            label, resolved = self.resolve(
                atlas(), body, [], target="8:+0x10", asm_root=asm_root
            )

        self.assertEqual("o008+0x10", label)
        self.assertEqual(body[0x10:0x1C], resolved)

    def test_coarse_owner_start_does_not_override_shorter_fallback(self):
        body = bytes(range(64))
        with tempfile.TemporaryDirectory() as directory:
            asm_root = Path(directory)
            self.fallback(asm_root, 0, [0xF0000000, 0xF0000004])

            label, resolved = self.resolve(
                atlas(), body, [(0, 64)], target="8:+0x0", asm_root=asm_root
            )

        self.assertEqual("o008+0x0", label)
        self.assertEqual(body[:8], resolved)

    def test_duplicate_raw_fallback_boundaries_fail_closed(self):
        with tempfile.TemporaryDirectory() as directory:
            asm_root = Path(directory)
            addresses = [0xF0000010, 0xF0000014]
            self.fallback(asm_root, 0x10, addresses, suffix="AAA")
            self.fallback(asm_root, 0x10, addresses, suffix="BBB")

            with self.assertRaisesRegex(SystemExit, "raw skeleton ownership is ambiguous"):
                self.resolve(
                    atlas(), bytes(range(64)), [], target="8:+0x10", asm_root=asm_root
                )

    def test_noncontiguous_raw_fallback_boundary_fails_closed(self):
        with tempfile.TemporaryDirectory() as directory:
            asm_root = Path(directory)
            self.fallback(asm_root, 0x10, [0xF0000010, 0xF0000018])

            with self.assertRaisesRegex(SystemExit, "not one contiguous range"):
                self.resolve(
                    atlas(), bytes(range(64)), [], target="8:+0x10", asm_root=asm_root
                )

    def test_missing_atlas_and_fallback_identity_fails_closed(self):
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaisesRegex(SystemExit, "ownership cannot be proved"):
                self.resolve(
                    atlas(),
                    bytes(range(64)),
                    [],
                    target="8:+0x10",
                    asm_root=Path(directory),
                )


class ResidentOwnerMetadataTests(unittest.TestCase):
    def test_func_80010b4c_metadata_supplies_exact_resident_extent(self):
        vram = 0x80010B4C
        size = 0xA98
        rom_offset = skeleton_scan.resident_vram_to_rom(vram)
        synthetic = bytes(rom_offset) + bytes([0xB4]) * size

        with mock.patch.object(skeleton_scan, "load_rom", return_value=synthetic):
            label, resolved = skeleton_scan.resolve_target_bytes(
                "0x80010B4C", {}
            )

        self.assertEqual("vram:0x80010B4C", label)
        self.assertEqual(size, len(resolved))
        self.assertEqual(bytes([0xB4]) * size, resolved)

    def test_func_80011cdc_metadata_supplies_exact_resident_extent(self):
        vram = 0x80011CDC
        size = 0x558
        rom_offset = skeleton_scan.resident_vram_to_rom(vram)
        synthetic = bytes(rom_offset) + bytes([0xA5]) * size

        with mock.patch.object(skeleton_scan, "load_rom", return_value=synthetic):
            label, resolved = skeleton_scan.resolve_target_bytes(
                "0x80011CDC", {}
            )

        self.assertEqual("vram:0x80011CDC", label)
        self.assertEqual(size, len(resolved))
        self.assertEqual(bytes([0xA5]) * size, resolved)

    def test_func_8001398c_metadata_supplies_exact_resident_extent(self):
        vram = 0x8001398C
        size = 0x528
        rom_offset = skeleton_scan.resident_vram_to_rom(vram)
        synthetic = bytes(rom_offset) + bytes([0x5A]) * size

        with mock.patch.object(skeleton_scan, "load_rom", return_value=synthetic):
            label, resolved = skeleton_scan.resolve_target_bytes(
                "0x8001398C", {}
            )

        self.assertEqual("vram:0x8001398C", label)
        self.assertEqual(size, len(resolved))
        self.assertEqual(bytes([0x5A]) * size, resolved)

    def test_func_8001291c_metadata_supplies_exact_resident_extent(self):
        vram = 0x8001291C
        size = 0x890
        rom_offset = skeleton_scan.resident_vram_to_rom(vram)
        synthetic = bytes(rom_offset) + bytes([0x3C]) * size

        with mock.patch.object(skeleton_scan, "load_rom", return_value=synthetic):
            label, resolved = skeleton_scan.resolve_target_bytes(
                "0x8001291C", {}
            )

        self.assertEqual("vram:0x8001291C", label)
        self.assertEqual(size, len(resolved))
        self.assertEqual(bytes([0x3C]) * size, resolved)

    def test_func_8000e920_metadata_supplies_exact_resident_extent(self):
        vram = 0x8000E920
        size = 0x878
        rom_offset = skeleton_scan.resident_vram_to_rom(vram)
        synthetic = bytes(rom_offset) + bytes([0x3C]) * size

        with mock.patch.object(skeleton_scan, "load_rom", return_value=synthetic):
            label, resolved = skeleton_scan.resolve_target_bytes(
                "0x8000E920", {}
            )

        self.assertEqual("vram:0x8000E920", label)
        self.assertEqual(size, len(resolved))
        self.assertEqual(bytes([0x3C]) * size, resolved)


if __name__ == "__main__":
    unittest.main()
