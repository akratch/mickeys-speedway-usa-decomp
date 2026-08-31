#!/usr/bin/env python3
"""Synthetic tests for overlay-atlas exact-C release deltas."""

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))

import overlay_atlas  # noqa: E402


def ownership(start, end, *, exact, source="overlays/o001/example"):
    return {
        "offset": hex(start),
        "end_offset": hex(end),
        "size": hex(end - start),
        "type": "c",
        "source": source,
        "matched": True,
        "nonmatching": not exact,
    }


def module(overlay, rows, mixed=None):
    result = {"overlay": overlay, "text_ownership": rows}
    if mixed is not None:
        result["mixed_tu_exact_c_ranges"] = mixed
    return result


def atlas(modules):
    exact_bytes = 0
    for item in modules:
        exact_bytes += sum(
            int(row["size"], 0)
            for row in item["text_ownership"]
            if row["type"] == "c"
            and row["matched"] is True
            and row["nonmatching"] is False
        )
        exact_bytes += sum(
            int(row["size"], 0)
            for row in item.get("mixed_tu_exact_c_ranges", [])
        )
    return {
        "schema_version": 1,
        "totals": {"matched_overlay_c_bytes": exact_bytes},
        "modules": modules,
    }


class ExactCDeltaTests(unittest.TestCase):
    def test_reports_promotions_retractions_and_net_bytes(self):
        base = atlas(
            [
                module(1, [ownership(0x10, 0x20, exact=True)]),
                module(2, [ownership(0x30, 0x48, exact=False)]),
            ]
        )
        target = atlas(
            [
                module(1, [ownership(0x10, 0x20, exact=False)]),
                module(2, [ownership(0x30, 0x48, exact=True)]),
            ]
        )

        delta = overlay_atlas.compare_exact_c_atlases(base, target)

        self.assertEqual([row["key"] for row in delta["promotions"]], [
            "overlay:2:text:0x30"
        ])
        self.assertEqual([row["key"] for row in delta["retractions"]], [
            "overlay:1:text:0x10"
        ])
        self.assertEqual(delta["totals"]["promotion_bytes"], 0x18)
        self.assertEqual(delta["totals"]["retraction_bytes"], 0x10)
        self.assertEqual(delta["totals"]["net_exact_c_bytes"], 8)

    def test_mixed_tu_exact_range_is_a_promotion(self):
        container = ownership(0, 0x40, exact=False)
        base = atlas([module(7, [container])])
        exact_range = {
            "offset": "0x10",
            "end_offset": "0x28",
            "size": "0x18",
            "label": "overlay7Promoted",
            "source": "overlays/o007/overlay_007",
            "evidence": "synthetic exact proof",
        }
        target = atlas([module(7, [container], [exact_range])])

        delta = overlay_atlas.compare_exact_c_atlases(base, target)

        self.assertEqual(delta["promotions"][0]["kind"], "mixed_tu_range")
        self.assertEqual(delta["promotions"][0]["label"], "overlay7Promoted")
        self.assertEqual(delta["totals"]["net_exact_c_bytes"], 0x18)

    def test_duplicate_overlay_module_fails_closed(self):
        state = atlas([
            module(1, [ownership(0, 4, exact=True)]),
            module(1, [ownership(4, 8, exact=True)]),
        ])

        with self.assertRaisesRegex(
            overlay_atlas.AtlasDeltaError, "duplicate overlay 1"
        ):
            overlay_atlas.exact_c_index(state)

    def test_duplicate_overlay_offset_fails_closed(self):
        state = atlas([
            module(1, [
                ownership(0, 0x20, exact=True, source="overlays/o001/a"),
                ownership(0, 0x10, exact=True, source="overlays/o001/b"),
            ])
        ])

        with self.assertRaisesRegex(
            overlay_atlas.AtlasDeltaError, "ambiguous exact-C identity"
        ):
            overlay_atlas.exact_c_index(state)

    def test_overlapping_ranges_with_different_offsets_fail_closed(self):
        state = atlas([
            module(1, [
                ownership(0, 0x20, exact=True, source="overlays/o001/a"),
                ownership(0x10, 0x30, exact=True, source="overlays/o001/b"),
            ])
        ])

        with self.assertRaisesRegex(
            overlay_atlas.AtlasDeltaError, "overlapping exact-C identities"
        ):
            overlay_atlas.exact_c_index(state)

    def test_same_identity_with_changed_extent_fails_closed(self):
        base = atlas([module(1, [ownership(0x10, 0x20, exact=True)])])
        target = atlas([module(1, [ownership(0x10, 0x24, exact=True)])])

        with self.assertRaisesRegex(
            overlay_atlas.AtlasDeltaError, "ambiguous identity.*extent"
        ):
            overlay_atlas.compare_exact_c_atlases(base, target)

    def test_reviewed_whole_owner_to_mixed_ranges_reports_only_uncovered_bytes(self):
        source = "overlays/o001/example"
        base = atlas([module(1, [ownership(0, 0x40, exact=True, source=source)])])
        container = ownership(0, 0x40, exact=False, source=source)
        mixed = [
            {
                "offset": "0x0",
                "end_offset": "0x10",
                "size": "0x10",
                "label": "firstExact",
                "source": source,
            },
            {
                "offset": "0x30",
                "end_offset": "0x40",
                "size": "0x10",
                "label": "lastExact",
                "source": source,
            },
        ]
        target = atlas([module(1, [container], mixed)])

        delta = overlay_atlas.compare_exact_c_atlases(base, target)

        self.assertEqual(delta["promotions"], [])
        self.assertEqual(
            [
                (row["offset"], row["end_offset"], row["size"])
                for row in delta["retractions"]
            ],
            [(0x10, 0x30, 0x20)],
        )
        self.assertEqual(delta["totals"]["net_exact_c_bytes"], -0x20)

    def test_declared_total_must_match_exact_rows(self):
        state = atlas([module(1, [ownership(0, 8, exact=True)])])
        state["totals"]["matched_overlay_c_bytes"] = 12

        with self.assertRaisesRegex(
            overlay_atlas.AtlasDeltaError, "atlas declares 12"
        ):
            overlay_atlas.exact_c_index(state)

    def test_fractional_numeric_field_fails_closed(self):
        state = atlas([module(1, [ownership(0, 8, exact=True)])])
        state["modules"][0]["text_ownership"][0]["size"] = 8.5

        with self.assertRaisesRegex(
            overlay_atlas.AtlasDeltaError, "invalid size 8.5"
        ):
            overlay_atlas.exact_c_index(state)

    def test_missing_total_fails_closed(self):
        state = atlas([module(1, [ownership(0, 8, exact=True)])])
        del state["totals"]["matched_overlay_c_bytes"]

        with self.assertRaisesRegex(
            overlay_atlas.AtlasDeltaError, "lacks an exact-C byte total"
        ):
            overlay_atlas.exact_c_index(state)

    def test_orphan_mixed_tu_range_fails_closed(self):
        exact_range = {
            "offset": "0x10",
            "end_offset": "0x20",
            "size": "0x10",
            "label": "orphan",
            "source": "overlays/o001/example",
        }
        state = atlas([
            module(1, [ownership(0, 0x40, exact=True)], [exact_range])
        ])

        with self.assertRaisesRegex(
            overlay_atlas.AtlasDeltaError, "not inside exactly one nonmatching"
        ):
            overlay_atlas.exact_c_index(state)

    def test_json_result_uses_numeric_offsets_and_sizes(self):
        base = atlas([module(3, [ownership(4, 12, exact=False)])])
        target = atlas([module(3, [ownership(4, 12, exact=True)])])
        delta = overlay_atlas.compare_exact_c_atlases(base, target)

        decoded = json.loads(json.dumps(delta, sort_keys=True))

        self.assertEqual(decoded["schema_version"], 1)
        self.assertEqual(decoded["promotions"][0]["overlay"], 3)
        self.assertEqual(decoded["promotions"][0]["offset"], 4)
        self.assertEqual(decoded["promotions"][0]["size"], 8)


class NonmatchingSourceTests(unittest.TestCase):
    def test_guarded_candidate_is_nonmatching(self):
        self.assertTrue(
            overlay_atlas.is_nonmatching_text("#ifdef NON_MATCHING\nvoid f(void) {}\n#endif\n")
        )

    def test_bare_global_asm_is_nonmatching(self):
        self.assertTrue(
            overlay_atlas.is_nonmatching_text(
                '#pragma GLOBAL_ASM("asm/nonmatchings/example.s")\n'
            )
        )

    def test_exact_c_only_is_matching(self):
        self.assertFalse(overlay_atlas.is_nonmatching_text("void f(void) {}\n"))


class AtlasStateLoadingTests(unittest.TestCase):
    def test_loads_manifest_and_checkout_paths(self):
        state = atlas([module(1, [ownership(0, 4, exact=True)])])
        with tempfile.TemporaryDirectory() as temporary:
            tree = Path(temporary)
            manifest = tree / "config" / "overlays.us.json"
            manifest.parent.mkdir()
            manifest.write_text(json.dumps(state))

            from_manifest, manifest_info = overlay_atlas.load_atlas_state(
                str(manifest)
            )
            from_tree, tree_info = overlay_atlas.load_atlas_state(str(tree))

        self.assertEqual(from_manifest, state)
        self.assertEqual(from_tree, state)
        self.assertEqual(manifest_info["kind"], "manifest")
        self.assertEqual(tree_info["kind"], "tree")

    def test_loads_git_ref_without_checkout(self):
        state = atlas([module(9, [ownership(0x20, 0x30, exact=True)])])
        with tempfile.TemporaryDirectory() as temporary:
            repo = Path(temporary)
            subprocess.run(["git", "init", "-q", str(repo)], check=True)
            subprocess.run(
                ["git", "config", "user.name", "Atlas Test"], cwd=repo, check=True
            )
            subprocess.run(
                ["git", "config", "user.email", "atlas@example.invalid"],
                cwd=repo,
                check=True,
            )
            manifest = repo / "config" / "overlays.us.json"
            manifest.parent.mkdir()
            manifest.write_text(json.dumps(state))
            subprocess.run(["git", "add", str(manifest)], cwd=repo, check=True)
            subprocess.run(
                ["git", "commit", "-qm", "Synthetic atlas"], cwd=repo, check=True
            )

            loaded, info = overlay_atlas.load_atlas_state("HEAD", repo=repo)

        self.assertEqual(loaded, state)
        self.assertEqual(info["kind"], "git")
        self.assertRegex(info["resolved"], r"^[0-9a-f]{40}$")


if __name__ == "__main__":
    unittest.main()
