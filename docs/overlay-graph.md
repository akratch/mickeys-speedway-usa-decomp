# Structural overlay-to-module matching against Jet Force Gemini

Byte identity between Mickey's 107 overlays and JFG's 157 modules found only
three shared symbols (`docs/acceleration-survey.md` section 6): `alSeqFileNew`,
`refractOutput`, and `osRamTest4_6105`/`osRamTest3_6105`. JFG's overlay code
was recompiled from a different source revision, so byte comparison mostly
returns nothing. What the two games share instead is *structure*: they are
built from the same runtime linker (`include/game/runlink.h`), the same
overlay header and relocation-table layout, and, per the acceleration survey's
n-gram measurement, a resident engine that is 31.7% kin. This report asks
whether that shared structure is enough to line up modules by their shape —
size, function count, and cross-overlay call graph — rather than their bytes.

`tools/overlay_graph_match.py` builds the feature vectors, scores every
Mickey-overlay/JFG-module pair, and writes `config/overlay-graph.us.json`.
Re-run it with `--write` to refresh the report, `--check` to verify it is
current, and `--self-test` to run the calibration below on its own.

## 1. What was measured, and from where

**Mickey side** — entirely from the existing atlas (`config/overlays.us.json`,
built by `tools/overlay_atlas.py` from the ROM's own header/relocation
tables; no new ROM reads):

- `text_size`, `data_rodata size`, `bss_size` per overlay.
- function count: the number of `text_ownership` entries, both matched (`c`,
  a real named/sized C function) and unmatched (`asm`, an unresolved chunk
  that may bundle more than one function — an upper bound, not an exact
  count).
- `out_degree` / `out_relocations`: distinct overlays this one's `imports`
  list names, and the total relocation count across them.
- `in_degree` / `in_relocations`: inverted from every other overlay's
  `imports`, plus the atlas's own `cross_overlay_inbound_relocations`.

**JFG side** — entirely from JFG's own published decomp repository at
`~/Desktop/dev/decomp-refs/jfg` (override with `JFG_ROOT`), a permitted
source under `docs/CLEANROOM.md`:

- `jfg_us_syms_full.txt`: 157 `Module N` sections, each a list of
  `+offset name` symbols (a name and a location, not a body).
- `build/jfg.us.map`: JFG's own linker map, giving `.text`/`.data`/`.bss`
  sizes per `overlays/oN/overlay_N.c.o` object. This is JFG's build output
  from JFG's own source, not anything derived from Mickey's ROM.
- `overly_refs.txt`: JFG's published cross-overlay call graph
  (`Overlay N -> [...]`/`Overlay N <- [...]`, both directions, with counts),
  covering 76 of the 157 modules (the rest call nothing outside themselves).
- A function's size on the JFG side is the delta to the next symbol's offset
  within its module (or to the module's text-size bound for the last one) —
  arithmetic on published offsets, not disassembly.

Two things the task description asked for turned out not to exist in the
data actually available and are recorded as gaps rather than silently
skipped:

- **Resident-import fingerprints** (which `n_al*`/`gu*`/`os*` functions a
  module calls) are not recoverable for Mickey without reading instruction
  bytes. Mickey's runtime linker only patches *overlay-to-overlay* calls
  through `RelocTableEntry`/`TrapDanglingJump` (`runlink.h`); a call from an
  overlay into the fixed-address resident segment needs no runtime patch and
  so leaves no metadata trace outside the instruction stream itself, which
  this project's rules forbid tracking. `imports` in the atlas is
  overlay-to-overlay only. This feature is dropped rather than faked.
- **`alSeqFileNew`** (one of the three byte-identical anchors) is JFG resident
  library code, not inside any of the 157 `Module N` sections, so it cannot
  anchor a module-to-module pair. It is recorded in the report under
  `calibration.known_anchors.unanchorable` rather than silently dropped.

## 2. Scoring

Cosine similarity over the raw log-scaled feature vector was tried first and
discarded: most overlays are small (median text size is a few hundred bytes),
so most feature dimensions are near zero on both sides, and cosine similarity
between two mostly-zero vectors collapses toward 1.0 regardless of whether the
one dimension that actually differs is a good match or a terrible one. The
shipped scorer instead uses **ratio similarity** (`min(a,b)/max(a,b)`, in
`[0, 1]`) per feature, which stays sensitive to real differences even when
most other dimensions agree at zero:

```
score = 0.5 * size_score + 0.3 * function_count_ratio + 0.2 * degree_shape

size_score = (2*text_ratio + data_ratio + bss_ratio) / 4   # text weighted double: present and nonzero almost everywhere
degree_shape = mean(in_degree ratio, out_degree ratio)      # coarse call-graph "busyness" match only
```

Neighbour *identity* (which specific overlays two modules each call) is not
compared — overlay numbers are unrelated across the two games and there is no
anchor set large enough to seed a real subgraph match, so `degree_shape` stays
a profile comparison (how many neighbours, not which ones).

A pair is flagged **confident** only when the top score clears 0.70 **and**
leads the runner-up by at least 25% of its own value, and never for a
genuinely empty Mickey overlay (overlay 32, 0 bytes): `ratio_similarity(0,
0) == 1.0` on every feature, so an empty module trivially "matches" any other
near-empty module, and that would be reported as confident for the wrong
reason if not excluded explicitly.

## 3. Calibration

### 3.1 The two anchorable byte-identical matches

| Mickey overlay | Symbol | JFG module | Method's rank for the pair (of 157) | Method's actual top-1 pick |
|---|---|---|---|---|
| 107 | `osRamTest4_6105` | 156 | **1** | 156 (correct) |
| 49 | `refractOutput` | 2 | 115 | 72 |

Overlay 107 is a clean win: both sides are a 0x30-byte, one-function module,
and the ratio-similarity scorer finds it immediately. Overlay 49 is a real
miss, and an instructive one: Mickey's overlay 49 is 896 bytes with one
matched function (`refractOutput` alone), while JFG's module 2 is 2128 bytes
with 6 functions (the whole refract subsystem). Mickey's build evidently
regrouped this donor function into a much smaller overlay than the one JFG
shipped it in, so a size/count profile has no way to find it — the two
modules do not have similar shapes even though they share one function. This
is a structural limit of the method, not a bug: shape matching can only
recover pairs where the *module boundary* survived the port, and this one
did not.

### 3.2 Self-test: JFG-US vs. JFG-kiosk

JFG-US and JFG-kiosk are the same game at two close revisions — true module
identity (module N ↔ module N) is verifiable directly from the symbol lists:
of a 20-module sample, 14 have byte-for-byte identical function-name sets and
the rest differ by one or two functions. This is the easiest possible version
of the module-matching problem and is the ceiling for what a size/count/degree
profile can do before even reaching the much harder Mickey-vs-JFG case
(different game, different revision, and — per 3.1 — module boundaries that
can move).

The kiosk side of this self-test is deliberately weaker than the full method:
JFG's kiosk build has no map file or cross-overlay reference dump in this
checkout, so `text_size` there is only approximated (the last symbol's own
offset, not a true section-size bound) and `data_size`/`bss_size`/degree
features are all zero. The result is a lower bound on the method's ceiling,
not a fair fight.

```
pairs tested:          156
top-1 accuracy:         9.6%
top-3 accuracy:        16.7%
confident-flagged:         0
```

Manual inspection of the misses (`tools/overlay_graph_match.py --self-test`
prints the raw numbers; see the worked examples in the source's
`run_self_test` docstring) shows the failures cluster on modules with 4-6
functions, where a dozen-plus *different* modules share that same small
function count and the noisy kiosk `text_size` approximation is not enough to
break the tie. Larger, more distinctive modules (module 3, 67 functions;
module 6, 22 functions) are found correctly even under this weakened
self-test.

### 3.3 What this means for the main result

Given a 9.6% same-game top-1 ceiling and a genuine miss on one of the two
anchorable byte-identical pairs, the Mickey-vs-JFG run was expected to — and
did — clear the confidence bar for **zero** of Mickey's 107 overlays. That is
the honest output of `tools/overlay_graph_match.py --write`, and
`config/overlay-graph.us.json`'s `totals.confident_matches` is `0`.

## 4. What's in the report

`config/overlay-graph.us.json` (schema `1`) has, for every one of Mickey's
107 overlays: its own size/count/degree features, `confident` (always
`false` in this run), and its top-3 JFG module candidates by score with each
candidate's text size and function count so a reader can sanity-check the
pairing by eye. `calibration` carries the two anchor results and the
self-test above so the confidence bar's provenance travels with the data
rather than living only in this document.

No candidate in the report reaches the tier this project's naming convention
(`docs/modules.md` section 1) requires to propose a name — every candidate
here is, at best, Tier D structural inference with a weak, uncalibrated
scoring function behind it, well under the individually-argued sub-threshold
bar in section 1.2. **This report proposes zero overlay names.**
`propose_names()` in the tool exists and is exercised (it produces
rank-order function-offset/size pairings for a confident match, each
explicitly tagged Tier D), but it never fires in this run because no match
qualifies as confident, which is the correct behavior given section 3.

### 4.1 Leads, not proposals

For a future pass — most usefully, one that has the resident-donor lane's
finished symbol names to add real resident-call fingerprints, or one that
extracts JFG's own `.data`/`.bss` sizes with less noise — the highest-scoring
pairs from this run are the overlays most likely to reward a second look.
These are explicitly **not** proposals and carry no evidence tier:

| Mickey overlay | text | functions | JFG module (top-1) | score |
|---|---|---|---|---|
| 39 | 368 | 3 | 45 | 1.000 |
| 93 | 240 | 2 | 134 | 1.000 |
| 64 | 1680 | 1 | 65 | 0.998 |
| 24 | 1056 | 4 | 20 | 0.996 |
| 78 | 176 | 2 | 78 | 0.979 |
| 91 | 1408 | 4 | 96 | 0.975 |
| 71 | 2896 | 5 | 76 | 0.967 |
| 31 | 3920 | 8 | 29 | 0.963 |

A score of 1.000 here means the two modules' *sizes and counts happen to
agree exactly* — with a 9.6% same-game top-1 ceiling (section 3.2), that is
weak evidence on its own and should not be read as "these are the same
module." It is a shortlist for someone with more evidence (a donor-object
byte scan, or a resident-call fingerprint) to check next, nothing more.

## 5. Handoff

- No overlay names are proposed by this lane; `docs/modules.md` /
  `symbol_addrs.us.txt` are unchanged.
- The two byte-identical anchors already known before this report
  (`docs/acceleration-survey.md` section 6) are the only Mickey-JFG overlay
  correspondences this report can state with confidence: overlay 107 ↔ JFG
  module 156 (`osRamTest4_6105`) and, on shared code only, overlay 49 ↔ JFG
  module 2 (`refractOutput`) — the latter without module-level structural
  agreement (section 3.1).
- The section 4.1 shortlist is a starting point for the next attempt, not a
  result to build on directly.
- The biggest lever for a future run is closing the resident-import gap
  (section 1): if a later lane's donor-object scan (`overlay_donor_scan.py`)
  or the resident-donors lane's growing `symbol_addrs.us.txt` puts real
  names on more of Mickey's resident functions, this tool's `imports`
  feature could in principle be extended to a genuine resident-call
  fingerprint — but only from data already in the atlas or a permitted
  source; nothing here should ever be extended by reading instruction bytes.
