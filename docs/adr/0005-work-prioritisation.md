# 0005. Work prioritisation

Status: Accepted
Date: 2026-08-24

## Context

The campaign queue (`docs/campaigns.md` Epochs 3-12) ranked overlay
campaigns by "expected exact bytes per unit time" and sent essentially all
of August's effort into overlays, while the resident segment's matched-count
grew by seven functions over the same period. `docs/acceleration-survey.md`
§2.1 measured why that was backwards: a masked-skeleton fingerprint scan
(opcode/funct/fmt kept, registers/immediates/jump-targets masked) against
the five reference builds found

| Region | Bytes | Skeleton-covered | Unambiguous donor |
|---|---:|---:|---:|
| resident game code | 451,616 | 97,976 (21.7%) | 239 hits, 87,368 B |
| libultra corridor | 30,960 | 24,932 (80.5%) | 85 hits |
| resident tail | 64,640 | 10,960 (17.0%) | 2 hits |
| all 106 overlays | 469,264 | 1,256 (0.3%) | 11 hits |

An 8-word n-gram kinship measure corroborates it: Mickey's resident game
code shares 31.7% of its masked 8-grams with JFG (higher than DKR-vs-JFG's
own 17.3%, i.e. Mickey's resident segment is closer to JFG than two
same-engine-family reference projects are to each other), while the
overlays sit at 7.9%, the level of an unrelated engine. The overlays are
new code; the resident segment is substantially JFG's engine.

86 of the resident segment's unambiguous donor hits, covering 27,960 bytes,
are not yet named in `symbol_addrs.us.txt` at all. Separately, the 50 KB
`n_audio` block (45 TUs, 10.9% of unmatched resident text) has matched
donor C in both PD and BK and has been sitting unblocked-in-principle since
2026-07-31, waiting only on the provenance ruling ADR 0008 makes.

## Decision

**Donor availability first, bytes second.** Rank queue items by whether a
matched or near-matched donor function exists in the reference farm, not by
raw byte count. Concretely, in order:

1. The resident segment before overlays: 21.7% skeleton-donor coverage
   there versus 0.3% in the overlays is a two-order-of-magnitude difference
   in expected cost per matched byte.
2. `n_audio` first of all: unblocked by ADR 0008, 50 KB, matched donor C in
   two reference projects, no naming obstacle remaining.
3. The 86 unnamed unambiguous resident donors, adopted as tier A/B names
   per `docs/modules.md` §1, with `PROVENANCE` notes, before further
   overlay work is queued.
4. Only after donor-backed resident work is exhausted does raw
   bytes-per-unit-time re-enter as the tiebreaker among remaining targets.

## Consequences

- `docs/campaigns.md`'s epoch planning re-ranks around this rule; existing
  overlay epochs are not invalidated, but new epoch selection follows donor
  availability first.
- The resident split gets drawn along JFG's own translation-unit boundaries
  where a donor names one (`gsSnd`, `n_csplayer`, `rcpFast3d`, `shadows`,
  `matrix`, `paths`), per ADR 0006's TU model.
- This does not change matching *standards* (ADR 0001/0002): a donor-backed
  candidate is still just a starting point to be matched against Mickey's
  own bytes, never an authority over them, per `docs/CLEANROOM.md`'s
  existing PROVENANCE rule.
