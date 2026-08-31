# The reference builds

Every tier-A name in this project comes from comparing Mickey's ROM bytes
against another game's *built objects*. That method is only as trustworthy as
the builds it reads, so this file records, for each reference decompilation:
where it came from, at which commit, which baserom it was built against and
whether that baserom's checksum was verified, what the build actually achieved,
and what it yielded when mined.

**None of this lives in this repository.** The checkouts, their baseroms and
their build outputs are at `~/Desktop/dev/decomp-refs/`; DKR v80 is additionally
cross-checked from `~/Desktop/dev/Diddy-Kong-Racing/`. They are outside the tree
and are never committed. `docs/CLEANROOM.md` is the governing document: these five named
published retail-derived decompilations are permitted sources for names and,
with point-of-use `PROVENANCE` disclosure, for adapted bodies. Nothing else is.

---

## What can and cannot be re-checked from this repository

**Read this before quoting any number below.** The repo URLs, pinned commits,
baserom checksums, build outcomes and object counts on this page describe an
out-of-tree build farm at `~/Desktop/dev/decomp-refs/`. No byte of it is in
this repository and none ever will be; `docs/CLEANROOM.md` requires that.

What is committed instead is the recipe and the checksums:

- **`tools/reference-builds.lock`** pins, per title, the repo URL, the commit,
  the baserom SHA1, the build outcome, the object count, and one aggregate
  digest over the objects that were mined.
- **`gmake reference-builds`** rebuilds the farm from those pins: clone at the
  pinned commit, stage the baserom out of your own ROM archive and check it
  against the locked SHA1 before using it, apply the macOS fixes each build
  needs, build.
- **`gmake check-reference-builds`** re-derives each title's digest from a farm
  on this machine and compares it to the lock. A farm that matches is the farm
  the 190 tier-A names were mined from. One that does not is a different farm,
  and this page's yield columns do not describe it.

The digest covers what the mining pass reads out of each object, not the object
file, because the object files are *not* byte-reproducible; see *Rebuilding
the farm* at the end of this page for the measurement and what follows from it.

Neither target can run in CI, for the reason `verify` and `check-fixtures`
cannot: they need retail ROMs, which this project does not ship and never will.
They are run on a machine that has a farm, and they are the only thing that
makes the pins above more than a transcription.

Three things on this page are still not checkable that way, and are labelled
where they appear: the **re-confirmation** column, which comes from the mining
run's logs and from nothing else; **Perfect Dark's baserom**, which upstream
publishes as an MD5 with no SHA1, so the check against upstream is an MD5 check
(the lock carries both); and, for the two titles that did not build to a
byte-perfect ROM, **whether the objects they did produce are all the objects a
complete build would have produced**. The digest proves a rebuild agrees with
this pass, not that this pass was complete.

The same distinction runs through the yield columns below: the *adoption* and
*corroboration* figures ARE recomputable from `symbol_addrs.us.txt` and
`mickey.us.yaml`, and `tools/check_derived_numbers.py` recomputes them on every
`gmake check-docs`. The *re-confirmation* figures are not, and are labelled.

## Three different things, counted separately

An earlier version of this page ran them together and produced a headline
("168 of 171 translation units adopted") that nothing in the repo supported.

- **Adopted**: the whole-`.text` match produced a **new named subsegment** in
  `mickey.us.yaml`, i.e. a measured file boundary this tree did not have. This
  is the yield. **Recomputable** from the `//` block headers in
  `symbol_addrs.us.txt`, each of which is cross-checked against the yaml.
- **Re-confirmed**: the match landed on a subsegment **Phase 1 had already
  named** from DKR. It adopted nothing and adds no row anywhere; its value is
  that Phase 1's map survives contact with four independent builds. **Not
  recomputable from this tree**: the figure comes from the run logs.
- **Corroborated**: an *adopted* TU whose bytes were also matched by a second
  title's object, recorded on that TU's `same bytes in:` line.
  **Recomputable.**

## Summary

| Title | Commit | Baserom SHA1 verified | Build outcome | Objects | Objects mined | Adopted TUs | Names adopted | Re-confirmed | Corroborations |
|---|---|---|---|---|---|---|---|---|---|
| Diddy Kong Racing | `38d7f9ba` | yes | **full match** (US v77) | 243 | 223 | (Phase 1: 80) | (Phase 1: 107 + 32 game) | — | — |
| Jet Force Gemini | `c82affff` | yes | **full match** | 772 | 391 | **84** | **192** | 84 | — |
| Perfect Dark | `169ed48b` | MD5 (no SHA1 published) | near-full: code links clean with authentic IDO, asset-compression bytes differ | 2546 | 467 | **3** | **3** | 52 | 25 |
| Banjo-Kazooie | `6eaae281` | yes | **full match** | 1232 | 1105 | **0** | **0** | 73 | 2 |
| Conker's Bad Fur Day | `3adf2291` | yes | partial: every source file compiles, final link blocked | 1446 | 705 | **0** | **0** | 65 | 8 |
| **Totals for this pass** | | | | | **2891** | **87** | **190** | *(not recomputable)* | **26 TUs** |

"Objects mined" counts objects with a non-empty `.text` after filtering out
asset blobs; see *Empty `.text` objects* below for why that filter exists.
Adopted TUs and names are attributed to the build whose object the symbol file
cites; a match present in several builds is credited once, so the columns sum
to the totals rather than double-counting.

**26 of the 87 adopted translation units were matched by more than one title**,
and the corroboration column counts, per title, how many of those 26 that title
matched. Four projects arriving independently at the same name for the same
bytes is the strongest thing a name in this tree can have behind it, but it is
a different claim from adoption and has its own column.

---

## Diddy Kong Racing

- Repo: https://github.com/akratch/Diddy-Kong-Racing.git (the pinned tooling
  branch of the published https://github.com/DavidSM64/Diddy-Kong-Racing
  decompilation)
- Pinned commit: `38d7f9ba39642e2b5311a76e0b83fb3fe2733262`
- Canonical checkout: `~/Desktop/dev/decomp-refs/diddy-kong-racing/`
- Version: US v77. Baserom SHA1
  `0cb115d8716dbbc2922fda38e533b9fe63bb9670`, matching the project's published
  v77 checksum.
- Build outcome: **full byte-perfect match**. The 243-object mining surface has
  digest `917ba733782e07382dd753b50b496c9f8647caec8695f7bca0359a19f0cd763b`.
- The adjacent working checkout's US v80 object surface is a secondary
  cross-check at the same commit: 243 objects, digest
  `91e8524065ba66c094c1107fa7c9ef0d9a9dff4026799a665b59e78dcf3c9243`.

Phase 1 used DKR before the farm was locked. Its yield was the libultra
corridor (107 function names, 80 subsegment boundaries) and 90 game-code match
candidates of which 32 were adopted. The pinned v77 rebuild now makes that
closest-lineage donor reproducible like the other references.

The overlay pass checks **both v77 and v80 before decompiling a module**. Each
revision gives the same result: one strong hit, one ambiguous hit, 104 negative
non-empty overlays, and the empty overlay 32. The strong hit is a unique,
relocation-free 64-byte `alSeqFileNew` at offset zero in Mickey overlay 5. The
overlay 46 hit is a short relocation-heavy shape that occurs three times in
the ROM and carries several unrelated DKR names; it is recorded and rejected
as identification. Negative results are useful here: they show that the games
share systems and source organization more heavily than they share unchanged
overlay object code.

Epoch 3 also checked DKR at source level after that exact scan. Its
`libultra/src/audio/bnkf.c` contains the same bank/instrument/sound/wavetable
patch graph as Mickey overlay 5 and identifies the required `-O3 -mips2` flag
family. DKR compiles the graph as one translation unit and inlines calls that
remain in Mickey, so only `alSeqFileNew` is claimed as an exact donor object;
the other five Mickey functions are separately bounded and independently
matched C using DKR as a source/flag crosswalk. This distinction is why the
object ledger stays sparse even when source similarity is highly productive.

DKR still provides semantic navigation where bytes drift. Mickey overlay 61's
`MSU-GHOST`, `LOAD GHOST`, `SAVE GHOST`, and Controller Pak UI strings point to
DKR's `src/save_data.c`, `src/racer.c`, and `src/menu.c`. This is a crosswalk,
not a match: names are adopted only after Mickey's own call/data flow proves
them. The exhaustive per-overlay evidence is in
`config/overlay-donors.us.json` and reproduced by
`gmake overlay-donors-scan-check`.

The Epoch 3 reproducibility pass ran that scan from the pinned farm rather than
accepting the committed report. It reproduced all 107 rows for DKR v77, DKR
v80, and JFG after the derived atlas fingerprint was refreshed.

In the resident re-run, DKR whole-matches 84 of the corridor's 95 named
subsegments. Every name it supports is already in the tree, and the 16 corridor
translation units it does *not* whole-match are exactly the ones Jet Force
Gemini does. See `docs/modules.md` §4.1.

## Jet Force Gemini: the highest-value reference

- Repo: https://github.com/Ryan-Myers/Jet-Force-Gemini
- Pinned commit: `c82affffe8f11cb5b440cfa918f4582ad8573279`
- Baserom: `Jet Force Gemini (USA).z64`, SHA1
  `493ced9008dbe932d6e91179b68e8630cf23a023`; matches the repo's documented US
  SHA1 exactly.
- Build outcome: **full byte-perfect match**. `build/jfg.us.z64` SHA1 equals the
  baserom's; CRC1/CRC2 both good.
- Objects: 772 total (178 libultra, 214 game code, 213 raw non-matching asm,
  167 asset blobs). 391 mined after filtering.

**Why it was worth more than the other non-DKR donors put together.** JFG and Mickey share
an engine lineage, a four-module layout, the runtime linker and, it turns
out, a libultra build. Of the 87 translation units this pass adopted, **84 are
JFG's**, carrying 187 of the 190 names, and they include all sixteen of the corridor drift runs DKR could not explain,
the entire `n_audio` synthesis library, the Controller Pak filesystem, the
exception-handler island, and five pieces of shared engine code (`gsSnd`,
`lights2`, `trapDanglingJump`, `refractOutputAssembler`, `osBootRamTest`, plus
three maths TUs).

JFG's objects carry a quirk worth knowing: `GLOBAL_ASM`-wrapped functions emit
a second symbol suffixed `.NON_MATCHING` at the same address, sometimes with a
slightly different size. The mining scripts fold those onto the base name and
keep the larger size; the symbol file never contains a `.NON_MATCHING` row.

Across Mickey's overlays, JFG yields five strong locations and five ambiguous
ones. Three strong results carry only generated placeholder names (overlays 5,
14, and 16) and are not adopted. The useful named results are `refractOutput`
at overlay 49 text offset `0x354` and the entire 48-byte overlay 107 matching
JFG's `osRamTest4_6105` object. The latter is both a whole-section and named
function match; those are two views of one byte range, not two independent
pieces of evidence.

## Perfect Dark

- Repo: https://github.com/n64decomp/perfect_dark
- Pinned commit: `169ed48bdcbfb3b568b028bd5bebb27680073514`
- Baserom: `Perfect Dark (USA) (Rev 1).z64`, MD5
  `e03b088b6ac9e0080440efed07c1e40f`; matches the repo's documented
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
- **Yield: 3 adopted translation units, 3 names**. `osGbpakCheckConnector`,
  `osGbpakGetStatus`, and `__osGbpakSelectBank` at ROM 0x6B3D0-0x6C040.
  Small, and not replaceable: PD is the reference whose mined objects adopted
  these three Transfer Pak translation units. PD also **corroborates 25** of the
  87 adopted TUs and **re-confirms 52** subsegments Phase 1 had already named.

## Banjo-Kazooie

- Repo: https://github.com/n64decomp/banjo-kazooie
- Pinned commit: `6eaae281481c9e4b367dc161faabfc3c79fe8733`
- Baserom: `Banjo-Kazooie (USA).z64`, SHA1
  `1fe1632098865f639e22c11b9a81ee8f29c75d7a`; matches the repo's documented
  `baserom.us.v10.z64` checksum exactly.
- Build outcome: **full byte-perfect match**.
- Objects: 1232 total (737 game/boot, 495 libultra via `lib/ultralib`).
  1105 mined.
- **Yield: zero adopted names, and that is the result.** BK **corroborates 2**
  of the 87 adopted TUs and **re-confirms 73** subsegments Phase 1 had already
  named (not nothing), but every whole-TU match it produced was either already
  named from DKR or JFG, or was rejected: `src/SM/code_46C0.c.o` at 0x4FC20 offers
  only the placeholder `func_8038AAB0` at an address already rejected in Phase
  1, and `src/mgu/mtxxfmf.o` at 0x2A2B0 is a clean match whose boundary is
  deliberately not declared (see `docs/modules.md` §4.3). BK's `ultralib` is a
  different, later ultralib than Mickey's; its game code is a different engine.

## Conker's Bad Fur Day

- Repo: https://github.com/mkst/conker
- Pinned commit: `3adf229175c037c771f251f169f9dd80ca306924`
- Baserom: `Conker's Bad Fur Day (USA).z64`, SHA1
  `4cbadd3c4e0729dec46af64ad018050eada4f47a`; matches the repo's documented
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
- **Yield: zero adopted names.** Conker **corroborates 8** of the 87 adopted
  TUs, **re-confirms 65** subsegments Phase 1 had already named, and supplies
  **three** of the seven rejections (`docs/modules.md` §4.3):
  `src/init_3920.c.o` at 0x20008 places only a placeholder;
  `src/libultra/audio/n_synaddplayer.c.o` at 0x63B40 is subsumed by the whole
  TU at 0x63AD0; and `asm/libultra/libc/ldiv.s.o` at 0x76C80 is a partial
  extraction subsumed by the whole `ldiv` TU already matched at 0x76B80. Its
  own audio objects carry placeholder file names (`init_17870.c`) where JFG
  carries real ones, which is why JFG is cited even where both match.

---

## Empty `.text` objects: a filter the mining pass must keep

Several farm builds contain objects that are valid ELF files with an
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
gmake overlay-donors-scan-check   # DKR v77/v80 + JFG against all overlays
```

`<ref-build-dir>` must contain only compiled code objects. The tool globs
`**/*.o` under one directory and has no exclude option, so restricting a run to
"libultra only" means building a filtered tree of symlinks outside the repo
first, noted in `docs/workbench-improvement-log.md` as friction worth fixing.
The overlay scanner defaults to the farm paths documented above; alternate
object roots can be supplied with `DKR_V77_OBJECTS`, `DKR_V80_OBJECTS`, and
`JFG_OBJECTS`.

## Rebuilding the farm

```sh
gmake reference-builds                      # all five, from the locked pins
gmake reference-builds REFS_ARGS=jfg        # one title
gmake check-reference-builds                # compare a farm to the lock
```

`tools/setup_reference_builds.sh` carries the per-title toolchain surgery each
of these builds needed on macOS, with the reason for each in a comment: Apple's
standalone `cpp`, which fails on `-o` and fails *silently* when piped into the
assembler; the Linux-only IDO binaries every one of these repos ships or builds,
replaced by decompals' macOS build of the same release, checksummed on download
because a truncated copy of that exact tarball once produced a `cc` that was the
right size and died on every invocation; Perfect Dark's `armips` and its
committed Linux `gzip`; Banjo-Kazooie's missing `mips-linux-gnu-gcc` and its
extraction/compile race under `-j`; the three assembler macros newer splat emits
and older `macro.inc` files do not define. The baseroms come from your own
archive and are checked against the lock's SHA1 before use.

**The objects are not byte-reproducible, and the digest is not over the whole
object.** IDO writes an `.mdebug` section into every object it compiles carrying
the absolute source path, the build host's name and a build timestamp, so the
same commit built in a different directory produces different object *files*.
Measured, not assumed: a fresh clone of Jet Force Gemini at `c82affff`, built by
this script into a scratch directory, rebuilt the ROM byte-for-byte identical to
the baserom and produced the same 772 objects, of which **279 differed
byte-wise** from the farm's, while all 772 had an identical `.text` and an
identical set of `.text` symbols.

So the digest covers each object's **mining surface** instead: the bytes of
`.text`, and the name, value and size of every symbol in it. That is exactly and
only what `tools/find_known_objects.py` compares against Mickey's ROM. It is
stable across rebuilds (the same JFG rebuild reproduced the locked digest
exactly) and it still moves if the compiler, the flags or the sources move. A
farm that matches the lock is a farm that mines to the same names.

`tools/reference_build_digest.py` computes it, in stdlib Python off the ELF
section headers, so it runs wherever a farm is and does not need a MIPS
cross-toolchain to check one.
