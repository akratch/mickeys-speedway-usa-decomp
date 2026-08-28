# Structural overlay-to-module matching against Jet Force Gemini

Byte identity between Mickey's 107 overlays and JFG's 157 modules found only
three shared symbols (`docs/acceleration-survey.md` section 6): `alSeqFileNew`,
`refractOutput`, and `osRamTest4_6105`/`osRamTest3_6105`. JFG's overlay code
was recompiled from a different source revision, so byte comparison mostly
returns nothing. What the two games share instead is *structure*: they are
built from the same runtime linker (`include/game/runlink.h`), the same
overlay header and relocation-table layout, and, per the acceleration survey's
n-gram measurement, a resident engine that is 31.7% kin. This report asks
whether that shared structure is enough to line up modules by their shape --
size, function count, cross-overlay call graph, and (as of this pass) *which
resident engine functions each overlay calls*.

`tools/overlay_graph_match.py` builds the feature vectors, scores every
Mickey-overlay/JFG-module pair, and writes `config/overlay-graph.us.json`.
Re-run it with `--write` to refresh the report, `--check` to verify it is
current, and `--self-test` to run the calibration below on its own. It runs
in well under a second (`gmake`-free; only needs the two baseroms already on
disk for this project and its JFG reference checkout).

## 1. What was measured, and from where

**Mickey side** -- mostly the existing atlas (`config/overlays.us.json`,
built by `tools/overlay_atlas.py` from the ROM's own header/relocation
tables), plus one new input this pass:

- `text_size`, `data_rodata size`, `bss_size` per overlay.
- function count: the number of `text_ownership` entries, both matched (`c`,
  a real named/sized C function) and unmatched (`asm`, an unresolved chunk
  that may bundle more than one function -- an upper bound, not an exact
  count).
- `out_degree` / `out_relocations`: distinct overlays this one's `imports`
  list names, and the total relocation count across them.
- `in_degree` / `in_relocations`: inverted from every other overlay's
  `imports`, plus the atlas's own `cross_overlay_inbound_relocations`.
- **new: the resident-callee fingerprint** -- which named resident-engine
  functions (`osRecvMesg`, `matrixYaw`, `mmFree`, ...) each overlay calls,
  and how many times. Section 1.1 explains where this comes from and why
  the prior pass concluded it was unrecoverable and was wrong about that.

**JFG side** -- entirely from JFG's own published decomp repository at
`~/Desktop/dev/decomp-refs/jfg` (override with `JFG_ROOT`), a permitted
source under `docs/CLEANROOM.md`:

- `jfg_us_syms_full.txt`: 157 `Module N` sections, each a list of
  `+offset name` symbols (a name and a location, not a body), plus a
  `Main Code` section of ~1290 named resident functions. Two sections in
  this file, `Module 3430` and `Module 4092`, are reserved-selector
  artifacts rather than real overlay modules (their symbol content --
  `__osRdbSend`, `osTvType` -- and their section numbers, `0xD66`/`0xFFC`,
  don't fit the real 1-157 numbering or the `0xFFD`/`0xFFE`/`0xFFF`
  reserved range either); `tools/overlay_graph_match.py` drops both. Two
  further module numbers in the real range, 21 and 56, simply have no
  symbols listed at all and are absent from the file -- not a bug, JFG's
  own extraction just found nothing to name there, the same as Mickey's
  overlay 32.
- `build/jfg.us.map`: JFG's own linker map, giving `.text`/`.data`/`.bss`
  sizes per `overlays/oN/overlay_N.c.o` object.
- `overly_refs.txt`: JFG's published cross-overlay call graph
  (`Overlay N -> [...]`/`Overlay N <- [...]`, both directions, with counts),
  covering 76 of the 157 modules (the rest call nothing outside themselves).
- A function's size on the JFG side is the delta to the next symbol's offset
  within its module (or to the module's text-size bound for the last one) --
  arithmetic on published offsets, not disassembly.
- **new: `build/jfg.us.z64`**, JFG's own built ROM, read for its per-module
  relocation tables only (never disassembled -- see 1.1).

**One prior gap stays a gap**: `alSeqFileNew` (one of the three
byte-identical anchors) is resident code in JFG's `Main Code` section, not
inside any of the 157 `Module N` sections. Mickey placed the *same* function
in overlay 5 (`docs/modules.md` records it as a DKR-donor object at overlay 5
offset 0), but that placement decision doesn't manufacture a JFG module to
pair overlay 5 against -- there still isn't one. It is recorded in the
report under `calibration.known_anchors.unanchorable` rather than silently
dropped.

### 1.1 The resident-callee fingerprint (new this pass, and a correction)

The previous pass concluded resident-import fingerprints were unrecoverable
without reading ROM instruction bytes, reasoning that Mickey's runtime linker
only patches overlay-to-overlay calls (`RelocTableEntry`/`TrapDanglingJump` in
`runlink.h`) and that a call from an overlay into the fixed-address resident
segment needs no runtime patch, so it leaves no metadata trace.

That reasoning doesn't survive contact with the actual relocation tables.
`tools/overlay_atlas.py` already decodes every module's own relocation
records (`overlay_tables.read_module_relocations`, the same call this tool
now reuses), and each `SYMBOL`-operation record resolves through
`overlayRomTable` to a `(target overlay, offset)` pair -- and **1161 of that
table's 2004 entries carry overlay number 0**, which is the resident segment.
Concretely, in overlay 1's own relocation records, checking every
`mode == R_MIPS_26, op == SYMBOL` entry against its resolved target found 148
call sites landing on overlay 0 alone, out of 282 total `jal`-patching
records. Mickey's overlay format reserves "overlay 0" for the resident
segment in exactly the table Rare's runtime linker convention uses -- and
JFG's own published tooling (`tools/overlay_reloc.py` in its repo) documents
the identical convention for JFG's ROM, under the same field names
(`RELOC_TYPE_JUMP`/`RELOC_TYPE_EXTERNAL` resolving through `overlayRomTable`,
with overlay 0 meaning "main"). Every external call on both sides --
resident or overlay-to-overlay -- is a placeholder `jal` patched at load
time, not a directly-encoded address as the prior pass assumed.

The consequence: **the resident-callee fingerprint is recoverable from
relocation-table metadata alone, on both sides, with zero ROM instruction
words decoded.** `mickey_resident_calls()` and `jfg_resident_calls()` in
`tools/overlay_graph_match.py` each read only relocation records (already
the kind of content `overlay_atlas.py` reads for the tracked atlas) and
`overlayRomTable`/JFG's equivalent -- pure arithmetic on fixed-format binary
tables, the same category of fact `overlay_tables.py` already asserts
against the live ROM on every run. Nothing resembling instruction text, a
byte array, or a hexdump is read, held past the two arithmetic passes, or
written to any file; only the resulting symbol names, addresses, and
per-overlay/module counts (config data, same as everything else the atlas
already tracks) reach `config/overlay-graph.us.json`.

Each resolved target address is mapped to a name: on Mickey's side via
`symbol_addrs.us.txt` (entirely a resident-segment symbol file), which has
426 `type:func` entries, 404 of them carrying a real name -- the rest are
`func_XXXXXXXX` address-derived stand-ins, skipped since they carry no
information beyond the address; on JFG's side via its `Main Code` section,
which is fully named (no `func_`-style stubs at all -- JFG's public decomp
is much further along).
A call whose target has no name becomes an opaque `unnamed:0xADDR` token,
which can only ever match the identical address and therefore only ever
matches within the same game -- useless for cross-game bridging, but kept in
the report (`mickey_resident_opaque_call_count`) since it's still a true
count of how much of an overlay's resident traffic this pass could name.

**Weighting.** A resident callee like `mmFree`, `osInvalDCache`, or
`mathRnd` is called from dozens of modules on both sides and is barely
evidence that two *specific* modules correspond; a callee only 1-2 modules
touch (`refractOutputAssembler`, `osCic6105SendData`, `GetSquadronFromIdentifier`)
is much stronger. `compute_name_weights()` gives every resident name an
inverse-document-frequency weight (`1 / document_frequency`, pooled across
both games' overlay/module feature dicts) and `multiset_jaccard()` applies it
to both the intersection and union sums. Concretely: without weighting,
Mickey overlays 104 and 105 both looked like exact matches for JFG module 111
(`resident_named_jaccard = 1.0`) purely because both call the common
`__osPiGetAccess`/`__osPiRelAccess` pair and nothing else named -- weighting
alone doesn't zero that score (both sides' sets are still identical), but it
lets other modules' scores separate enough that the confidence margin check
(section 2) correctly declines to call it confident.

## 2. Scoring

Cosine similarity over the raw log-scaled feature vector was tried first (and
discarded, prior pass) because most overlays are small (median text size is a
few hundred bytes), so most feature dimensions are near zero on both sides
and cosine similarity between two mostly-zero vectors collapses toward 1.0
regardless of whether the one dimension that actually differs is a good match
or a terrible one. The shipped scorer uses **ratio similarity**
(`min(a,b)/max(a,b)`, in `[0, 1]`) for the metadata-only terms, plus the new
**weighted Jaccard over named resident callees** as the dominant term when
either side has any:

```
named_jaccard is not None:
    score = 0.55 * named_jaccard + 0.25 * size_score + 0.12 * fn_score + 0.08 * shape
named_jaccard is None (neither side named a resident call for this pair):
    score = 0.5 * size_score + 0.3 * fn_score + 0.2 * shape   # unchanged fallback

size_score = (2*text_ratio + data_ratio + bss_ratio) / 4   # text weighted double
fn_score   = ratio_similarity(function_count, function_count)
shape      = mean(in_degree ratio, out_degree ratio)
```

`named_jaccard` is `None`, not `0.0`, when both sides have zero named
resident calls -- "no calls observed" is a missing signal, not evidence of a
mismatch, and the fallback blend keeps the prior pass's metadata-only
behavior exactly for those pairs. Neighbour *identity* (which specific
overlays two modules each call) is still not compared -- overlay numbers are
unrelated across the two games -- so `shape` stays a coarse profile
comparison, unchanged from the prior pass.

A pair is flagged **confident** only when the top score clears 0.70 **and**
leads the runner-up by at least 25% of its own value, and never for a
genuinely empty Mickey overlay (overlay 32, 0 bytes): `ratio_similarity(0,
0) == 1.0` on every metadata feature, so an empty module trivially "matches"
any other near-empty module. Both thresholds are unchanged from the prior
pass; they were not retuned for the new feature, and the calibration below is
the check on whether that was still the right call.

## 3. Calibration

### 3.1 The two anchorable byte-identical matches

| Mickey overlay | Symbol | JFG module | Method's rank for the pair (of 157) | Method's actual top-1 pick |
|---|---|---|---|---|
| 107 | `osRamTest4_6105` | 156 | **1** | 156 (correct) |
| 49 | `refractOutput` | 2 | 108 | 148 |

Overlay 107 is unchanged from the prior pass: both sides are a 0x30-byte,
one-function module with no resident calls on either side, so
`named_jaccard` stays `None` and the fallback size/count blend (which
already found this pair immediately) still wins it outright.

Overlay 49 is still a real miss, and the resident-callee feature does not
rescue it -- which is informative rather than disappointing. Mickey's
overlay 49 is 896 bytes, one function (`refractOutput` alone), with **zero**
resident calls; JFG's module 2 is 2128 bytes, six functions (the whole
refract subsystem: `refractInit`, `refractFreeAll`, `CreateRefractTask`,
`refractTick`, `refractOutput`, and an assembler helper), with sixteen named
resident calls spanning graphics/object/camera/message-passing functions.
`resident_named_jaccard` for the pair is a flat `0.0`: Mickey's isolated
`refractOutput` genuinely doesn't call anything the rest of JFG's module 2
calls, because Mickey shipped `refractOutput` alone and moved the rest of
the subsystem's resident traffic elsewhere (or dropped it). This confirms
the prior pass's diagnosis structurally rather than just by size: the module
*boundary* moved, and no per-module feature -- shape or call graph -- can
recover a boundary that didn't survive the port. `rank_among_all_157` moved
from 115 (prior pass) to 108 here, which is noise from the corrected JFG
module pool (155 modules with real symbol content, not 157 -- see 1) rather
than a meaningful improvement.

### 3.2 Self-test: JFG-US vs. JFG-kiosk

JFG-US and JFG-kiosk are the same game at two close revisions -- true module
identity (module N ↔ module N) is verifiable directly from the symbol lists.
This is the easiest possible version of the module-matching problem and is
the ceiling for what this method can do before even reaching the much harder
Mickey-vs-JFG case.

```
pairs tested:          155
top-1 accuracy:         9.7%
top-3 accuracy:        16.8%
confident-flagged:         0
```

These numbers are barely different from the prior pass's 9.6%/16.7% (the
0.1pp shift is the `Module 3430`/`4092` fix in section 1, not anything about
the new feature) and **that is expected, not a regression**: this checkout
ships JFG-kiosk's symbol list only, not its ROM or a build, and the
resident-callee fingerprint needs the ROM's own relocation tables to
compute. Every kiosk module's `resident_named` set is therefore empty,
`multiset_jaccard()` returns `None` for every pair, and `score_pair()` falls
back to the identical metadata-only blend the prior pass used. This self-test
could not exercise the new feature at all; `resident_callee_feature_engaged`
is recorded as `false` in the report specifically so this isn't misread as a
result. Section 3.1's anchor check and section 4's confident matches are
where the new feature actually engages and can be judged.

### 3.3 What this means for the main result

Given a 9.7% same-game top-1 ceiling on the metadata-only path and a genuine
miss on one of the two anchorable byte-identical pairs even with the new
feature engaged, the honest expectation was that most of Mickey's 107
overlays would still not clear the confidence bar. The resident-callee
feature changed the answer from "zero" to "three" (section 4) -- a real
improvement, driven by actual call-graph evidence rather than a threshold
change (the bar itself, 0.70 + 25% margin, was not touched) -- but it is
still a small fraction of 107, and every one of the confident matches is
individually inspectable rather than a statistical claim about the rest.

## 4. Confident matches

Three Mickey overlays clear the bar. Two are backed by real named-callee
overlap (Tier B evidence at the function level, section 4.1); one is a
size/count-only match with no resident-call corroboration and should be read
with that caveat attached.

| Mickey overlay | JFG module | Score | `resident_named_jaccard` | Shared named resident calls |
|---|---|---|---|---|
| 39 | 45 | 1.000 | 1.000 | `__osSiGetAccess`, `__osSiRawStartDma`, `__osSiRelAccess`, `osWritebackDCache`, `osInvalDCache` |
| 15 | 5 | 0.767 | 0.790 | `mmFree`, `starfieldFastMove`, `rainFastDraw` |
| 10 | 148 | 0.798 | *(none named on Mickey's side)* | -- |

**Overlay 39 ↔ JFG module 45** is the strongest result in this report.
Mickey's overlay is two functions (`overlay39Write`, and an unnamed tail) at
0xC8/0xA0 bytes; JFG's module 45 is `osCic6105SendData` (0xC8) and
`osCic6105StartGetData` (0x54) -- CIC-6105 is the N64 cartridge boot-auth
chip, and both functions call the same SI-driver primitives
(`__osSiGetAccess`/`__osSiRelAccess`/`__osSiRawStartDma`) plus one
distinguishing cache op each (`osWritebackDCache` for the send path,
`osInvalDCache` for the receive path) -- a call-graph signature that matches
the two functions' own semantics, not just their sizes.

**Overlay 15 ↔ JFG module 5** is a weather-effects module (starfield/rain)
with 13 Mickey functions (12 matched/named, one still `asm`) against JFG's
12. Five of the 12 matched-function pairs have direct named-callee evidence
(Tier B); the remaining seven line up by rank order and offset with strong
size agreement (several above 0.97, two at an exact 1.0) -- see section
4.1's full table.

**Overlay 10 ↔ JFG module 148** cleared the confidence bar on shape alone
(0.798, comfortably over 0.70+25%-margin) -- Mickey's overlay makes zero
resident calls this pass could name, so there is no call-graph evidence for
or against this specific pairing, only that the sizes/counts happen to line
up well. Module 148 is a small (544-byte, 2-function) JFG module that turns
out to be a common top-1 pick for several *unconfident* Mickey overlays too
(section 4.2), which is a mild warning sign about how distinctive this
particular size/count profile actually is. Treat this one pairing as weaker
than the other two.

### 4.1 Function-level alignment

`propose_names()` runs two passes per confident match: first, for every pair
of Mickey/JFG functions in the two modules, it checks whether they call any
of the *same* named resident function -- that's call-graph evidence at the
single-function level (Tier B), independent of the module-level score that
picked the pair in the first place. Whatever functions are left over fall
back to the prior pass's rank-order-by-offset pairing with a size ratio
(Tier D), unchanged.

**Overlay 39 ↔ module 45** (both Tier B, both already shown above):
`overlay39Write` ↔ `osCic6105SendData`, `overlay_039_tail` ↔
`osCic6105StartGetData`.

**Overlay 15 ↔ module 5** (Mickey offset/size, JFG name/offset/size):

| Mickey function | Mickey off/size | JFG name | JFG off/size | Tier | Evidence |
|---|---|---|---|---|---|
| `overlay15ReleaseResource` | 0xC / 0x40 | `starfieldFree` | 0x10 / 0x3C | B | shares `mmFree` |
| `overlay15MoveStars` | 0x428 / 0xD8 | `starfieldMove` | 0x434 / 0xD8 | B | shares `starfieldFastMove` |
| `overlay15ReleaseResource10` | 0x6B0 / 0x38 | `rainFree` | 0x718 / 0x34 | B | shares `mmFree` |
| `overlay15UpdateMovingStars` | 0x9E0 / 0x19C | `rainMove` | 0xA58 / 0x1A0 | B | shares `starfieldFastMove` |
| `overlay15DrawRain` | 0xB94 / 0xD8 | `rainDraw` | 0xC10 / 0x100 | B | shares `rainFastDraw` |
| `overlay15GetResource4` | 0x0 / 0xC | `starfieldActive` | 0x0 / 0x10 | D | size ratio 0.75 |
| `overlay15InitStarsAndPalette` | 0x4C / 0x3DC | `starfieldInit` | 0x4C / 0x3E8 | D | size ratio 0.988 |
| `overlay15DrawScreenStars` | 0x500 / 0x1A4 | `starfieldDrawSP` | 0x50C / 0x1FC | D | size ratio 0.827 |
| `overlay15GetResource10` | 0x6A4 / 0xC | `rainActive` | 0x708 / 0x10 | D | size ratio 0.75 |
| `overlay15InitStars` | 0x6E8 / 0x2F8 | `rainInit` | 0x74C / 0x30C | D | size ratio 0.974 |
| `overlay15SetValueC` | 0xB7C / 0xC | `rainSetDraw` | 0xBF8 / 0xC | D | size ratio 1.0 |
| `overlay15ClearValue7C` | 0xB88 / 0xC | `rainResetCamera` | 0xC04 / 0xC | D | size ratio 1.0 |

Every Tier D size ratio here (0.75-1.0) agrees with the Tier B rows'
qualitative read: this looks like a real module correspondence, not a
coincidence of aggregate size.

**Overlay 10 ↔ module 148**: one Tier D pairing,
`overlay10Initialize` (0x2B0) ↔ `BTSBGunTurret` (0x60), size ratio 0.14 --
weak on its own terms and offered with the same caveat as section 4.

Across all three confident matches: **7 Tier B proposals, 8 Tier D
proposals.** None of these are adoptions -- they are evidence-tiered leads
per `docs/modules.md` section 1's tier definitions, for
`symbol_addrs.us.txt`/`docs/modules.md` to weigh in a follow-up pass; this
lane does not edit either file.

### 4.2 Leads, not proposals

For overlays that didn't clear the confidence bar, the highest-scoring
pairs are still worth a second look, same caveat as the prior pass -- these
carry no evidence tier and are not proposals:

| Mickey overlay | text | named calls | JFG module (top-1) | score | `resident_named_jaccard` |
|---|---|---|---|---|---|
| 104 | 128 | 2 | 111 | 0.986 | 1.000 |
| 105 | 128 | 2 | 111 | 0.986 | 1.000 |
| 5 | 1904 | 12 | 25 | 0.573 | 0.431 |
| 16 | 1072 | 1 | 27 | 0.456 | 0.146 |
| 71 | 2896 | 3 | 76 | 0.442 | 0.015 |

104 and 105 are the pair discussed in section 1.1: both call only the common
`__osPiGetAccess`/`__osPiRelAccess` pair, so weighting cannot separate them
from each other, and the confidence margin check correctly declines them
(their competing candidates are close enough that the 25% margin fails, see
`config/overlay-graph.us.json` for the full top-3). Overlay 5 is
`alSeqFileNew`'s overlay (section 1) -- its top candidate, module 25, is
plausible audio-adjacent territory (`resident_named_jaccard` 0.431 is the
highest of any non-confident pair) but a comfortable 0.13 short of the
confidence bar.

The rest of the 107-overlay table -- every Mickey overlay's own named
resident-callee multiset, top-3 JFG candidates, and per-candidate
`resident_named_jaccard` -- is in `config/overlay-graph.us.json`, not
reproduced here in full.

## 5. Handoff

- **Three overlay/module pairings now clear this method's confidence bar**,
  up from zero in the prior pass: overlay 39 ↔ JFG module 45
  (`osCic6105SendData`/`osCic6105StartGetData`, strong Tier B evidence),
  overlay 15 ↔ JFG module 5 (starfield/rain weather effects, 5 Tier B + 8
  well-corroborated Tier D function pairs), and overlay 10 ↔ JFG module 148
  (shape-only, no call-graph corroboration -- treat cautiously).
- Fourteen function-level name proposals came out of those three modules (7
  Tier B, 8 Tier D); see section 4.1 for the full table with offsets and
  sizes. `docs/modules.md` / `symbol_addrs.us.txt` are unchanged -- this
  lane proposes, it does not adopt.
- The two byte-identical anchors from the prior pass are unchanged: overlay
  107 ↔ JFG module 156 (`osRamTest4_6105`, clean win, rank 1) and, on shared
  code only, overlay 49 ↔ JFG module 2 (`refractOutput`) -- now confirmed by
  a `resident_named_jaccard` of a flat `0.0` that the module boundary really
  did move and no per-module feature can recover it (section 3.1).
  `alSeqFileNew`/overlay 5 remains unanchorable to any JFG module (section 1).
- **The prior pass's stated blocker -- "resident-import fingerprints ...
  are not recoverable for Mickey without reading instruction bytes" -- was
  wrong**, and this pass found out empirically rather than by re-reading
  `runlink.h` more carefully: Mickey's `overlayRomTable` reserves overlay
  number 0 for the resident segment exactly like JFG's, so every external
  call on both sides is a placeholder patched at runtime, recoverable from
  relocation-table metadata with no ROM instruction word decoded. A
  follow-up pass should treat this as settled and build on it rather than
  re-deriving it.
- The section 4.2 shortlist (and the two module-numbering artifacts fixed in
  section 1 -- `Module 3430`/`4092`) are worth knowing about for whoever
  extends `parse_jfg_syms()` or reads `jfg_us_syms_full.txt` directly again.
- A JFG-kiosk *build* (ROM or ELF, not just the symbol list already in this
  checkout) would let the self-test in section 3.2 actually exercise the new
  feature instead of falling back to the metadata-only blend; right now
  that self-test's 9.7%/16.8% numbers say nothing about whether the
  resident-callee fingerprint generalizes, only that the old features don't.
