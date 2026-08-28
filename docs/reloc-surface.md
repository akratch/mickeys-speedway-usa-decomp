# Synthesizing the overlay relocation surface

Implemented. Tool: `tools/reloc_surface.py`. Generated artifact:
`overlay_undefined_syms.us.txt`. Gates: `gmake overlay-syms` writes it,
`gmake check-overlay-syms` fails on drift.

Sections 1-4 are the model and the feasibility evidence (lane
`lane/reloc-synth`); section 5 is what the full implementation turned out to
need and what it measured (lane `lane/reloc-synth2`).

**Question.** Every matched overlay function today carries a hand-derived
`POSTPROCESS` rule and/or a hand-written line in `overlay_undefined_syms.us.txt`.
That bespoke work, not the C, is what gates the 279-candidate overlay pool. Can
the relocation surface for a promoted candidate be derived *mechanically* from
the candidate's object plus the shipped tables, with no per-function hand work?

**Verdict: yes.** `overlay_undefined_syms.us.txt` is now generated in full --
every value line and every alias line -- from `config/overlays.us.json` and the
compiled overlay objects, and the ROM stays byte-identical. 2,928
hand-maintained lines became 1,596 values plus 669 aliases over 630 objects,
with none of the hand file's 8 duplicate names and none of its 168 shadowed
assignments. The measurable overlay candidate pool went from 110/279 to
150/279, and a further 44 that used to be an opaque build failure now report an
exact `.text` size delta.

## 1. The model

`docs/overlays.md` §5.1-5.4 establishes that a module ships **unrelocated**:
`runlinkDownloadCode` patches each site named by the module's own `reloc1` /
`reloc2` tables after the DMA. What the ROM image therefore stores at a
relocation site is not an address but the record's **stored addend** - the value
the runtime adds its base to.

A C translation unit cannot express that. IDO emits an ordinary `R_MIPS_26` or
`R_MIPS_HI16` + `R_MIPS_LO16` reference to a symbol that has no address in this
build. The project's existing answer is a placeholder extern whose *value* is
supplied as a linker-script assignment, so the linked instruction word carries
exactly the addend the retail image carries. `overlay_undefined_syms.us.txt`
says so in its own header: "Raw overlay relocation addends used by adopted C."

The consequence this spike tests is that **the value is not a judgement call**.
For a candidate whose schedule agrees with the target at the site, it is
readable from the ROM:

| relocation | required symbol value |
|---|---|
| `R_MIPS_26` | `SYNTHETIC_VMA \| (stored_imm26 << 2)` |
| `R_MIPS_HI16` + `R_MIPS_LO16` | `(stored_hi << 16) + sign_extend16(stored_lo)` |
| `R_MIPS_32` | `stored_word` |

minus whatever addend the object's own instruction already carries. Subtracting
the object's addend is what lets one base symbol serve many struct-field
references: IDO puts the field offset in the instruction, so only the base
belongs in the linker script.

Three measured facts anchor the model, taken from the decoded tables at run
time (`tools/overlay_tables.py`):

- Every `SYMBOL` `R_MIPS_26` site in every module stores immediate zero. Those
  are the cross-module and resident calls the runtime resolves; the placeholder
  therefore has to link to the module base, which is what the existing
  `--redefine-sym <sym>=func_overlay_NNN_F0000000_...` rules do by hand.
- Every `LOCAL` and `SYMBOL` `R_MIPS_HI16` site stores immediate zero, and its
  `R_MIPS_LO16` partner stores the target's byte offset. That is why the
  hand-written aliases are named `D_<offset>` and are assigned that offset.
- `JUMP` `R_MIPS_26` sites store the target's module offset shifted right two,
  i.e. an ordinary intra-module `jal` at the synthetic VMA already produces the
  shipped word with no fixup at all.

## 2. Mapping an object offset to a module offset

The one thing the addend formula needs is where in the module the object's
`.text` sits. `config/overlays.us.json` answers it directly: it records one
contiguous `text_ownership` row per source file, so

    module_offset = row(source_stem).offset + object_text_offset

with no name convention, no heuristic, and - importantly - no linked ELF. The
link is exactly what is missing when a promotion fails to resolve, so an earlier
draft that read `build/mickey.us.elf` was wrong in the only case that matters.

A site the module's relocation table does not name is not a relocation site in
the shipped image; reading an addend there reads an instruction word. When any
site for a symbol *is* corroborated by the table, the tool ignores the ones that
are not. That is the ROM's own statement, not a heuristic, and it is what makes
the procedure tolerant of a candidate whose schedule diverges away from the
sites in question.

## 3. Evidence

### 3.1 Replaying the hand-derived surface

`tools/reloc_surface.py --audit` runs the procedure over every overlay object
the Makefile links and scores the synthesized values against the tracked file:

    tracked-value replay: 1773/1773 agree (100.0%), 0 disagree,
                          982 not tracked, 0 unresolved, 27 stale objects skipped

That is the honest test the spike was asked for, at far greater scale than three
hand-picked functions: every symbol value in `overlay_undefined_syms.us.txt`
that the current build actually exercises - including the heaviest bespoke
rules, `overlay_008.c.o` (32 values behind 46 hand-written `--redefine-sym`
arguments), `overlay_001_tail.c.o` (62), `overlay50Initialize.c.o` (17), and
`overlay_009.c.o` (6 plus a 40-line filter spec) - is reproduced exactly from
the baserom and the atlas, with zero refusals.

### 3.2 Replaying the link itself

The 982 symbols the tool values that are *not* in the tracked file are ones the
link defines by other means: other overlays' functions, resident functions, and
absolute aliases from `undefined_syms_auto.us.txt`. Compared against the linked
ELF's own symbol values:

    linked-ELF cross-check: 979 agree, 3 disagree, 0 undefined

The three are `D_80000039`/`D_8000003A`/`D_8000003B` in
`overlay41UpdateColorRecords.c.o`, referenced only by an `R_MIPS_LO16` with no
`R_MIPS_HI16` partner. A lone `LO16` site does not observe the upper half, so
the tool reports the low half. The emitted instruction word is identical either
way; the value differs, the ROM does not.

Total independent agreement: **2752 relocation-symbol values reproduced, three
partial in an unobservable half, none wrong.**

### 3.3 Unblocking candidates that could not link

`tools/promotion_trial.py` classifies all 279 overlay candidates:

| class | count |
|---|---:|
| `text-differs` (already links) | 110 |
| `build-error`: undefined reference to a placeholder | 100 |
| `build-error`: `cannot grow .text` | 31 |
| `build-error`: `refusing to trim nonzero bytes` | 22 |
| `build-error`: digest / truncated relocation | 5 |
| `build-error`: other | 11 |

The undefined-reference class is the one this procedure addresses. Nineteen were
tried end to end - promote, compile, synthesize, rename the undefined symbols to
per-object aliases, emit the assignments, relink, diff the ROM:

| result | count | functions |
|---|---:|---|
| links, ROM produced | 14 | `overlay1ActivateObject`, `overlay1AdvanceGauge`, `overlay2QueryNode`, `overlay5InitializeAudio`, `overlay13DrawActive`, `overlay13UpdateRecord`, `overlay17CalculateEndpoints`, `overlay20UpdateObjectResource`, `func_overlay_002_F0000C90_1857A88`, `func_overlay_014_F00009F4_18702CC`, `func_overlay_022_F0000A7C_1878B84`, `func_overlay_022_F0000D30_1878E38`, `func_overlay_027_F0000624_187BFFC`, `func_overlay_029_F00010C4_187E374` |
| still fails | 5 | `func_overlay_007_F0000324_185C1AC`, `overlay15InitStars`, `overlay1InterpolatePath`, `overlay1MeasureCurves`, `overlay27UpdateCoordinates` |

Every one of the 14 produced **zero out-of-range differing bytes**: the
synthesized surface disturbs nothing outside the promoted function. The
remaining in-range word counts range from 7 to 196 and are genuine codegen
differences - which is the point. Those candidates now have a linked-ROM oracle
and an exact in-range word count where before they had a link error.

For completeness, the five candidates named in the spike brief
(`overlay7DispatchSelection`, `overlay8ScaleOutputs`, `overlay18Load`,
`overlay20BuildTileCommands`, `overlay1CloneRecord`) already link on the
existing surface: promoted, they build first time and reproduce the trial's
2/2/2/4/3 in-range words with no synthesis at all. Their residual is codegen,
not relocation surface. The blocked pool is the 100 undefined-reference
candidates, not these.

## 4. Limits

The five failures are three distinct classes, and none of them contradicts the
addend model.

1. **Schedule divergence at the site** (`func_overlay_007_F0000324_185C1AC`,
   `overlay15InitStars`). The candidate's instructions differ *at* the
   placeholder's own sites, so no consistent addend exists and the tool refuses
   rather than inventing one. It reports the conflicting values per symbol,
   which localizes the divergence. This is the model's stated precondition: the
   addend is only readable where the schedule agrees.
2. **Alias-block coupling** (`overlay1InterpolatePath`, `overlay1MeasureCurves`).
   `overlay_undefined_syms.us.txt` also carries 624 hand-written *alias* lines
   of the form `func_overlay_001_F0000CA8_184D088 = overlay1InterpolatePath;`
   so that other TUs' generated assembly can reach a friendly name. Promoting
   one function in a shared TU changes which symbols that TU defines and can
   strand such a line. A synthesizer that owns only the value lines cannot fix
   this; it has to own the alias block too, which is mechanical from
   `text_ownership`. It does now: §5.3.
3. **Relocation sites outside `.text`** (`overlay27UpdateCoordinates`). The
   prototype scans `.rel.text` only. A jump table or initialized pointer in
   `.data` carries the same kind of site and needs the same treatment against
   the `data_rodata` range. In the full run no candidate failed this way -- the
   class the trial reports for `overlay27UpdateCoordinates` is now
   `schedule-divergence-at-site` -- but the generator still scans `.rel.text`
   only, so the limit stands.

Two further limits are worth recording because they are invisible until they
bite:

- A lone `R_MIPS_LO16` determines only the low half of its symbol's value
  (§3.2). Byte-identical output, non-canonical symbol value.
- `overlay_undefined_syms.us.txt` currently assigns eight symbol names twice
  (`gOverlay100Entries`, `gOverlay1ModeObject`, `gOverlay1SubmitArg5`,
  `gOverlay4Groups`, `gOverlay59Entries`, `gOverlay77CallbackArgument`,
  `gOverlay77Handle`, `gOverlay77Selection`). `ld` takes the last, and so does
  the tool, so the audit is consistent - but a generated file would not have
  shadowed lines at all.

## 5. The full implementation

The spike proved the addend model. Building the generator on top of it turned
up three corrections to that model, all of them cases the spike's sample had
not exercised.

### 5.1 The object list comes from the linker script

The spike filtered build artifacts by whether the Makefile mentioned the
object's name, to skip stale ones. That silently dropped the **21 overlay
objects that reach the link through a pattern rule** and are never named
literally -- overlay 34's, 35's, 94's, 104's and 105's among them, which is
exactly why a handful of tracked values looked unreproducible. `mickey.us.ld`
names every input object explicitly, so it is the authoritative list: 630
objects, not 609. Replaying the historic hand-maintained file against the
complete list scores **1,840/1,840 values agree, 0 disagree**, up from the
spike's 1,773 on the incomplete one.

### 5.2 A value line is also needed for symbols this build *defines*

The spike only valued **undefined** symbols. The hand-maintained file also
assigned symbols the C defines -- `gOverlay77PositiveDivisor`,
`gOverlay57Countdown`, `gOverlay79RaceFlags` -- and dropping those changed ten
instruction words in overlay 77.

The reason is the same one §1 gives. The module's data is placed by the
*runtime*, not by this link, so a reference to it must carry the stored addend
(its offset within the module's data region) rather than whatever address `ld`
happens to give the definition. A linker-script assignment overrides the
definition, which is what makes that expressible at all.

The one reference that needs no assignment is an **intra-module call**: a JUMP
record stores the target's module offset shifted right two, which is exactly
what the assembler emits for that symbol at the synthetic VMA. So the rule is:

> value every symbol a relocation names, except one defined in this module's
> own `.text`.

That is derivable without a link -- the module's own objects say which names
they define in `.text` -- and it covers resident functions, other modules'
functions, and this module's own data uniformly.

### 5.3 An aliased identity must not also carry a value

`func_overlay_045_F000000C_188C464` is both a cross-module call placeholder
(wants the addend `0xF0000000`) and the generated identity of
`overlay45CreateDescriptor` (wants the real address). The hand file carried
both lines and `ld` silently took the last, which happened to be the alias.
The generated block emits the alias only, so the file has **no duplicate
names**: the 8 the hand file assigned twice, and the 168 values it shadowed
with an alias, are simply not written.

### 5.4 The `.text` extent, as a report instead of a failure

`trim_elf_section.py $@ .text <size>` appears in 588 Makefile rules and the
size is the `text_ownership` row's own extent. A promoted candidate whose
codegen is a different size therefore trips the trim guard at *compile* time,
and the build dies before the link -- which is why 53 of the 279 candidates
used to report nothing but `cannot grow .text` or `refusing to trim nonzero
bytes`.

`tools/postprocess_guard.py` gives every digest-guarded POSTPROCESS pass a
report-and-skip mode, enabled by `PROMOTION_TRIAL` in the environment
(`gmake PROMOTION_TRIAL=1`, or `tools/promotion_trial.py`, which sets it for
its own builds). The guard prints one marker line and skips its pass:

    PROMOTION-TRIAL: text-size-differs (+24 bytes): ... refusing to trim
    nonzero bytes from .text

The resulting ROM is not a valid build -- a skipped normalization leaves the
object un-normalized -- and is never verified; the trial classifies from the
marker. **The normal build never sets the variable**, so `gmake` and
`gmake verify` are untouched, and a usage error is never skipped, so a harness
bug cannot hide behind a green build.

### 5.5 Integration: the window between compile and link

The surface can only be derived once the candidate's object exists and before
the link resolves it -- a window a plain `gmake` does not offer.
`tools/promotion_trial.py` therefore builds, regenerates
`overlay_undefined_syms.us.txt` from the objects on disk, and builds again; the
second pass relinks only. Both the source file and the surface are restored
afterwards.

## 6. What the trial measures now

`tools/promotion_trial.py --overlays-only`, all 279 overlay candidates, lane
`lane/reloc-synth2`:

| class | before | after |
|---|---:|---:|
| `text-differs` -- links, N words differ in range | 110 | **150** |
| `text-size-differs` -- links, with an exact size delta | 0 | **44** |
| `build-error` | 169 | **85** |

Nothing came out `exact` or `text-exact`; every candidate that links has real
codegen work left, which is the point -- what changed is that 194 of 279 now
carry a number instead of a build failure.

The 150 that link, by in-range words:

| words | 1-2 | 3-4 | 5-8 | 9-16 | 17-32 | 33-64 | 65+ |
|---|---:|---:|---:|---:|---:|---:|---:|
| candidates | 7 | 11 | 13 | 30 | 22 | 27 | 40 |

The 44 size reports, by delta: 24 are shorter than the module owns and 19
longer (one guard reports a non-size digest); **18 are within ±16 bytes**, the
range where a codegen nudge is plausible. The extremes (`-472`, `+456`) are
candidates whose shape is wrong, not their scheduling.

The 85 remaining build errors, by cause -- each needs different work, which is
why they are named rather than pooled:

| cause | count | what it means |
|---|---:|---|
| `schedule-divergence-at-site` | 49 | the candidate's instructions differ *at* a placeholder's own sites, so no consistent addend exists. §4.1; the synthesizer reports the conflict rather than inventing a value |
| `resident-symbol-missing` | 15 | an undefined splat auto-name in the resident address space. Not a relocation-surface problem: the tree does not define that function yet |
| `rom-size` | 14 | the image's own size moved and no guard marker explains it |
| `relocation-truncated` | 4 | the reference does not fit its field at the synthesized value |
| `unresolved-placeholder` | 3 | undefined with none of the above |

`schedule-divergence-at-site` is now the dominant class, and it is the honest
one: it says the candidate does not yet agree with the target *where the
relocation table can see it*, which is a codegen problem, not a scaffolding
problem. The spike's alias-coupling and non-`.text` failure classes are gone --
`overlay1InterpolatePath` and `overlay1MeasureCurves` link now, because the
generator owns the alias block (§5.3).

### 6.1 The 31 candidates within 8 in-range words

    1  overlay80InitializeContact          o80    4  overlay1FindNextAngle              o1
    1  overlay97InitScale                  o97    4  overlay1FindPreviousAngle          o1
    2  overlay18Load                       o18    4  overlay20BuildTileCommands         o20
    2  overlay7DispatchSelection           o7     4  overlay3FindClosestObject          o3
    2  overlay84AdvanceCurrent             o84    4  overlay43FilterImage               o43
    2  overlay8ScaleOutputs                o8     4  overlay62Update                    o62
    2  overlay99RenderSegments             o99    4  overlay84LoadCurrent               o84
    3  overlay101DrawClock                 o101   6  func_overlay_022_F0000000_1878108  o22
    3  overlay1CloneRecord                 o1     6  func_overlay_041_F0001650_1888988  o41
    3  overlay40FadeRecords                o40    6  overlay68PromoteSecondary          o68
    4  func_overlay_014_F0000000_186F8D8   o14    6  overlay74Update                    o74
                                                  6  overlay7CommitSelection            o7
    7  func_overlay_022_F0000A7C_1878B84   o22    8  func_overlay_009_F0000540_1866BB8  o9
    7  func_overlay_038_F0000000_1885D10   o38    8  func_overlay_073_F0000000_18CAAC0  o73
    7  overlay14ResetMode                  o14    8  overlay1AssignRecordIndex          o1
                                                  8  overlay1ResolvePathPoint           o1
                                                  8  overlay20UpdateObjectResource      o20

Eleven of these were not measurable before this lane. They are the sweep's next
targets: within eight words is the range where the permuter closes candidates.

## 7. What is still hand-written

- **Section externalization.** `externalize_elf_section.py` takes the expected
  payload as a hex literal in the Makefile, which is the one part of the
  machinery not derivable from addresses alone. Five rules use it.
- **The `POSTPROCESS` trim sizes themselves.** They are the ownership row's
  extent and could be emitted from the atlas rather than written out per file;
  this lane made the *failure* derivable, not yet the rule.
- **Relocation filters and instruction normalizations.** Genuinely per-function
  reviewed assertions; nothing here suggests they are mechanical.
- **A lone `R_MIPS_LO16`** still determines only the low half of its symbol's
  value (§3.2). Byte-identical output, non-canonical symbol value.

## 8. Cleanroom note

`tools/reloc_surface.py` reads the baserom and `config/overlays.us.json` at run
time and emits only symbol names and the addresses/values the link already
requires. It writes no extracted data, embeds no ROM bytes, and prints no
instruction text. This document quotes no ROM words.

The generated `overlay_undefined_syms.us.txt` is tracked, and is the same class
of content as the hand-maintained file it replaces: symbol names and the
relocation addends the link requires, in the same two line forms. It is
smaller than what it replaced (2,265 lines against 2,928) and carries no
instruction text, so the clean-room detectors see strictly less than before.
