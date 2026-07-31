# The reference builds

Every tier-A name in this project comes from comparing Mickey's ROM bytes
against another game's *built objects*. That method is only as trustworthy as
the builds it reads, so this file records, for each reference decompilation:
where it came from, at which commit, which baserom it was built against and
whether that baserom's checksum was verified, what the build actually achieved,
and what it yielded when mined.

**None of this lives in this repository.** The checkouts, their baseroms and
their build outputs are at `~/Desktop/dev/decomp-refs/` (and, for Diddy Kong
Racing, `~/Desktop/dev/Diddy-Kong-Racing/`), outside the tree, and are never
committed. `docs/CLEANROOM.md` is the governing document: these five named
published retail-derived decompilations are permitted sources for names and —
with point-of-use `PROVENANCE` disclosure — for adapted bodies. Nothing else is.

---

## Summary

| Title | Commit | Baserom SHA1 verified | Build outcome | Objects | Objects mined | Whole-TU matches | Names adopted |
|---|---|---|---|---|---|---|---|
| Diddy Kong Racing | (Phase 1) | yes | full match | — | 169 libultra + game | 84 in corridor | 107 corridor + 90 game-code candidates, 32 adopted |
| Jet Force Gemini | `c82affff` | yes | **full match** | 772 | 391 | **168** | **187** |
| Perfect Dark | `169ed48b` | MD5 (no SHA1 published) | near-full: code links clean with authentic IDO, asset-compression bytes differ | 2546 | 467 | 3 | 3 |
| Banjo-Kazooie | `6eaae281` | yes | **full match** | 1232 | 1105 | 0 | 0 |
| Conker's Bad Fur Day | `3adf2291` | yes | partial: every source file compiles, final link blocked | 1446 | 705 | 0 | 0 |

"Objects mined" counts objects with a non-empty `.text` after filtering out
asset blobs — see *Empty `.text` objects* below for why that filter exists.
"Whole-TU matches" and "Names adopted" are this pass's figures, attributed to
the reference build actually cited on each row; a match present in several
builds is credited to the one whose object the symbol file names, so the
per-title numbers sum to the total rather than double-counting.

100 of the 171 adopted translation units were matched by more than one title.
Those extra matches are recorded as corroboration in `symbol_addrs.us.txt`
rather than as yield, and they are worth more than the column suggests: four
projects arriving independently at the same name for the same bytes is the
strongest thing a name in this tree can have behind it.

---

## Diddy Kong Racing

- Repo: https://github.com/DavidSM64/Diddy-Kong-Racing
- Checkout: `~/Desktop/dev/Diddy-Kong-Racing/`
- Built and mined in Phase 1 (Tasks B and D); this file documents it for
  completeness rather than re-deriving it.
- Yield in Phase 1: the libultra corridor (107 function names, 80 subsegment
  boundaries) and 90 game-code match candidates of which 32 were adopted.
- **Yield when re-run in this pass: superseded, not extended.** DKR whole-matches
  84 of the corridor's 95 named subsegments. Every name it supports is already
  in the tree, and the 16 corridor translation units it does *not* whole-match
  are exactly the ones Jet Force Gemini does. See `docs/modules.md` §4.1.

## Jet Force Gemini — the highest-value reference

- Repo: https://github.com/Ryan-Myers/Jet-Force-Gemini
- Pinned commit: `c82affffe8f11cb5b440cfa918f4582ad8573279`
- Baserom: `Jet Force Gemini (USA).z64`, SHA1
  `493ced9008dbe932d6e91179b68e8630cf23a023` — matches the repo's documented US
  SHA1 exactly.
- Build outcome: **full byte-perfect match**. `build/jfg.us.z64` SHA1 equals the
  baserom's; CRC1/CRC2 both good.
- Objects: 772 total (178 libultra, 214 game code, 213 raw non-matching asm,
  167 asset blobs). 391 mined after filtering.

**Why it is worth more than the other three put together.** JFG and Mickey share
an engine lineage, a four-module layout and the runtime linker — and, it turns
out, a libultra build. Of the 171 whole-TU matches this pass adopted, 168 are
JFG's, including all sixteen of the corridor drift runs DKR could not explain,
the entire `n_audio` synthesis library, the Controller Pak filesystem, the
exception-handler island, and five pieces of shared engine code (`gsSnd`,
`lights2`, `trapDanglingJump`, `refractOutputAssembler`, `osBootRamTest`, plus
three maths TUs).

JFG's objects carry a quirk worth knowing: `GLOBAL_ASM`-wrapped functions emit
a second symbol suffixed `.NON_MATCHING` at the same address, sometimes with a
slightly different size. The mining scripts fold those onto the base name and
keep the larger size; the symbol file never contains a `.NON_MATCHING` row.

## Perfect Dark

- Repo: https://github.com/n64decomp/perfect_dark
- Pinned commit: `169ed48bdcbfb3b568b028bd5bebb27680073514`
- Baserom: `Perfect Dark (USA) (Rev 1).z64`, MD5
  `e03b088b6ac9e0080440efed07c1e40f` — matches the repo's documented
  `ntsc-final` MD5. **The repo publishes an MD5 and no SHA1**, so the check is
  an MD5 check; the file's SHA1 is `af8788ac4d1a57260eae9c53ffe851fcf2a3319b`
  and is recorded here so a future run can compare against *this* pass rather
  than re-deriving trust from upstream.
- Build outcome: **near-full**. The full link succeeds with zero undefined
  symbols and a ROM is produced, but roughly 800 of ~2044 packed asset blobs
  differ from retail because of host `gzip`/deflate-stream differences. That is
  an asset-compression difference, not a code-compilation one: every C and asm
  object was compiled by the authentic IDO 5.3/7.1, which is the only property
  mining depends on.
- Objects: 2546 total (157 libultra, 100 engine lib, 232 game, 2044 assets,
  13 misc). 467 mined.
- **Yield: 3 translation units, 3 names** — `osGbpakCheckConnector`,
  `osGbpakGetStatus`, `__osGbpakSelectBank` at ROM 0x6B3D0-0x6C040. Small, and
  not replaceable: PD is the only one of the five references whose libultra
  contains the Transfer Pak driver at all. PD's objects also independently
  corroborate 77 of the translation units cited to JFG or already in the tree.

## Banjo-Kazooie

- Repo: https://github.com/n64decomp/banjo-kazooie
- Pinned commit: `6eaae281481c9e4b367dc161faabfc3c79fe8733`
- Baserom: `Banjo-Kazooie (USA).z64`, SHA1
  `1fe1632098865f639e22c11b9a81ee8f29c75d7a` — matches the repo's documented
  `baserom.us.v10.z64` checksum exactly.
- Build outcome: **full byte-perfect match**.
- Objects: 1232 total (737 game/boot, 495 libultra via `lib/ultralib`).
  1105 mined.
- **Yield: zero adopted names, and that is the result.** BK's objects
  corroborate 75 of the translation units adopted here, which is not nothing —
  but every whole-TU match it produced was either already named from DKR or
  JFG, or was rejected: `src/SM/code_46C0.c.o` at 0x4FC20 offers
  only the placeholder `func_8038AAB0` at an address already rejected in Phase
  1, and `src/mgu/mtxxfmf.o` at 0x2A2B0 is a clean match whose boundary is
  deliberately not declared (see `docs/modules.md` §4.3). BK's `ultralib` is a
  different, later ultralib than Mickey's; its game code is a different engine.

## Conker's Bad Fur Day

- Repo: https://github.com/mkst/conker
- Pinned commit: `3adf229175c037c771f251f169f9dd80ca306924`
- Baserom: `Conker's Bad Fur Day (USA).z64`, SHA1
  `4cbadd3c4e0729dec46af64ad018050eada4f47a` — matches the repo's documented
  `conker.us.sha1` exactly.
- Build outcome: **partial**. Every one of the 1446 source files compiles to an
  object with the authentic IDO 5.3, but the final link fails on `_asmpp_funcN`
  placeholder-symbol collisions across a few debugger translation units and on
  forward references into not-yet-decompiled functions. The project is ~23%
  decompiled, so most of its "game" objects are extracted assembly. **The
  objects are the deliverable for mining and they are complete**; the failed
  link does not affect them, because a whole-`.text` comparison never links.
- Objects: 1446 total (146 libultra, 208 game C, 1091 raw asm, 1 asset).
  705 mined.
- **Yield: zero adopted names.** Conker's objects corroborate 73 of the
  translation units adopted here, and it supplies two rejections: `src/init_3920.c.o` at 0x20008 places only a
  placeholder, and `asm/libultra/libc/ldiv.s.o` at 0x76C80 is a partial
  extraction subsumed by the whole `ldiv` TU already matched at 0x76B80. Its
  own audio objects carry placeholder file names (`init_17870.c`) where JFG
  carries real ones, which is why JFG is cited even where both match.

---

## Empty `.text` objects — a filter the mining pass must keep

Three of the four farm builds contain objects that are valid ELF files with an
**empty or absent `.text` section**: 14 in JFG, 29 in Perfect Dark, 110 in
Banjo-Kazooie, 1 in Conker. Most are legitimately data-only.
Some are not: the farm build hit a macOS-specific bug in which Apple's
standalone `/usr/bin/cpp` fails when invoked as `cpp -P file -o out`, and where
the failure was piped into the assembler (`cpp -P $< | as`) it produced a
*valid-looking, empty* object rather than an error.

Those objects were fixed in the farm before the inventories were taken, but the
class of failure is silent by construction, so the mining scripts exclude any
object whose `.text` is empty rather than trusting the build. An empty `.text`
contributes no comparison, so excluding it costs nothing; including one would
inflate the "objects scanned" figure and hide a broken build behind a clean
run. If a future pass reports a sharply lower match count for a title, check
this first.

## Reproducing a run

```sh
tools/find_known_objects.py <ref-build-dir> --start 0x1000 --end 0x86640 \
    --sections --rom-occ          # whole-TU matches: file boundaries
tools/find_known_objects.py <ref-build-dir> --start 0x1000 --end 0x86640 \
    --rom-occ                     # per-function matches
```

`<ref-build-dir>` must contain only compiled code objects. The tool globs
`**/*.o` under one directory and has no exclude option, so restricting a run to
"libultra only" means building a filtered tree of symlinks outside the repo
first — noted in `docs/workbench-improvement-log.md` as friction worth fixing.

The exact commands that produced each farm build, and the per-title toolchain
surgery each one needed on macOS, are not reproduced here: they are properties
of those projects and this environment, not of Mickey, and they live with the
farm.
