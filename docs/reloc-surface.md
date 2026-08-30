# Synthesizing the overlay relocation surface

Implemented. Tool: `tools/reloc_surface.py`. Generated artifact:
`overlay_undefined_syms.us.txt`. Gates: `gmake overlay-syms` writes it,
`gmake check-overlay-syms` fails on drift.

## Function-sized relocation comparison

Contributors can compare a full-TU candidate object's static relocations with
the target function's authenticated relocation surface without rebuilding a
second relocation decoder:

```sh
tools/reloc_surface.py compare overlay7DispatchSelection \
  --candidate-object build_non_matching/src/overlays/o007/overlay_007_tail.c.o
```

The default target context is `build/mickey.us.elf`; `--target-elf`,
`--candidate-symbol`, and `--target-symbol` cover scratch builds and the case
where extracted assembly keeps a generated name while C uses a friendly one.
Use `--json` for a compact machine-readable record and `--check` when a caller
needs non-exactness to return status 1.

The report gives the target count, candidate static count, exact
function-relative offset/type alignments, and exact stable-identity alignments.
Stable identities are runtime `(overlay, byte offset)` addresses, not source
names or the shared `0xF0000000` synthetic VMA. Overlay targets come from their
shipped module runtime tables. Resident targets instead come from the ordinary
canonical object's static `.rel.text`, because the resident runtime patch
table is sparse rather than a complete static-link surface. Before trusting
those tuples, the tool requires one canonical `build/src` object, exact
linked/object symbol size, and byte identity across the linked range after
masking only that object's relocation words. Each tuple must resolve to one
stable runtime identity, and its masked word is checked independently against
the resolved static-link value. JSON reports the selected path in
`target_surface_source`.

The command refuses duplicate or missing ownership, an overlay assertion that
disagrees with the atlas, a target span outside its owner, incomplete or
inconsistent runtime HI16/LO16 pairs, conflicting resident identities, a
missing canonical resident object, object/link disagreement outside relocation
words, and unresolved or duplicate resident tuples. A copied
scratch object whose path has lost `build*/src/...` context must pass its
canonical atlas key with `--source`; `--overlay` is only an assertion and
never selects between ambiguous owners.

### Shared identity normalization

`tools/reloc_identity.py` is the common identity layer used by both
`reloc_surface.py` and `function_preflight.py`. It parses GNU objdump relocation
rows into `(section, offset, type, symbol, addend)` tuples, parses successive
`objcopy --redefine-sym` operations, collapses transitive rename chains, and
applies relocation addends to stable `(overlay, byte offset)` identities.
Linker-script identifier aliases and postprocessed object names therefore use
one canonicalization path in both reports.

The layer fails closed: a rename cycle, conflicting original sources for one
destination, conflicting linker identities, malformed relocation rows, or
conflicting numeric assignments never receives a guessed identity. Exact
duplicate rename pairs are idempotent. The public comparison fields and human
report remain unchanged; `stable_identity_*` continues to describe identities
proved statically, while `effective_identity_*` may additionally include an
exact linked-ROM/runtime-table proof after canonical promotion.

### Canonical same-overlay generated-call boundaries

A guarded candidate can call another function in its own overlay by that
function's generated name even when no linker-alias row exists for the name.
The shared synthetic VMA still does not authenticate that call. The comparison
layer resolves this narrower case only for `R_MIPS_26`, and only when all of
the following canonical evidence agrees:

- the caller and encoded generated identity name the same overlay;
- one atlas module and one non-overlapping C `text_ownership` row exist, and
  the generated offset is exactly the row's start rather than merely somewhere
  inside a section or broad translation unit;
- the row's offsets and size agree with the module's exact text/ROM ownership;
- the tracked source has a fresh canonical object whose physical `.text`
  extent equals that owner and contains exactly one function symbol at object
  offset zero; and
- the linked ELF has one function symbol with the same generated name, encoded
  overlay offset, overlay section, and canonical-object symbol metadata.

Physical `.text` extent is the boundary authority. Metadata-only trimming may
leave the function symbol's pre-trim `st_size` larger than that extent; this is
accepted only when the canonical object and linked symbol retain the same size
and the physical section still equals the atlas owner. A smaller symbol,
another function in the object, or disagreement in value/section/size is a
conflict and is refused.

Cross-overlay names, non-call relocations, duplicate owners, missing or broad
boundaries, stale/missing source objects, unsafe source paths, and conflicting
object/linked symbols receive no inferred identity (or stop on contradiction).
Objcopy rename provenance is still propagated through the shared identity
layer, so a many-source destination remains ambiguous.

This closes evidence collection, not matching policy. It does not use runtime
row position to guess a callee, does not turn an unresolved identity into an
exact one, and does not relax promotion's independent offset/type, identity,
linked-range, overlay, and full-ROM requirements. On the current Overlay 22
initializer, it authenticates the generated call at function `+0x274` as the
uniquely owned Overlay 22 `+0xD30` boundary. Candidate identity resolution
therefore moves from 20/21 to 21/21 without changing the object or code bytes;
only 11/21 identities currently align with the target, so the function remains
non-exact.

Sections 1-4 describe the model and feasibility evidence; section 5 records
the requirements and measurements from the complete implementation.

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

Completeness is now enforced rather than assumed. Generation and audit stop
with the missing linker paths if any of those 630 objects is absent; a partial
build can no longer silently produce a plausible but incomplete tracked
surface.

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

### 5.6 A resident call is an ordinary addend with a name that is not free

`tools/promotion_trial.py` reported 15 candidates as `resident-symbol-missing`
and 4 as `relocation-truncated (R_MIPS_26)`. Both classes have one cause, and
it is not the addend model.

**What the ROM stores.** Section 5.2's census says a module's `SYMBOL`
`R_MIPS_26` record stores immediate zero, and it makes no distinction between a
target in another module and a target in the resident segment. Measured at the
sites themselves, that holds: overlay 49's three resident calls -- the ones the
Makefile rebinds by hand to `overlay65UpdateReloc` -- are `SYMBOL` `R_MIPS_26`
records at module offsets `0x218`, `0x2c0` and `0x314`, and each stores
immediate zero. The shipped word is `jal 0`; the runtime patches it, exactly as
it patches a cross-module call. The trampoline encoding `0C00CCE8` that 370 of
the 375 *`mainRelocTable`* entries carry (§5.3) belongs to the **resident**
segment's calls *into* overlays, not to a module's calls out of one, and does
not appear at these sites. A resident **data** reference is the other measured
case and needs no patch at all: its `HI16`/`LO16` pair stores the real resident
address, which is why the surface already carries lines like
`D_80000040 = 0x80000040` and why they are correct as they stand.

So a resident call wants the same value every other cross-module call wants,
`0xF0000000`. Nothing about the addend is special.

**What is special is the name.** Adopted C spells the call with splat's
resident auto-name, `func_80029FE4`. That name is global and shared with the
resident segment. A value line for it does not give the overlay an addend, it
*moves the resident function for every resident caller*: assigning
`func_80034448 = 0xf0000000` turns `models.c`, `level.c`, `menu.c`,
`texLoadTextureAddr.c` and four asm objects into `relocation truncated to fit:
R_MIPS_26`. That is the whole of the `relocation-truncated` class, and it is
also why the earlier attempt to rename these placeholders to the *real*
resident symbols failed the other way round: a `jal` from the module's
synthetic `0xF0000000` VMA cannot reach a `0x8…` address either. Neither the
overlay's name nor the resident's name can carry both meanings.

The auto-name shape is not the defining property, either. `overlay5InitializeAudio` calls `alHeapDBAlloc`, `osCreateMesgQueue` and `n_alCSPSetMessageQ` --
ordinary libultra globals -- and valuing those breaks `audio_manager_1050.c`
and the whole libultra link the same way. What the two shapes have in common is
that the **resident side of the link owns the name**, so that is what the
generator measures: `resident_defined_names()` collects the global symbols
defined by every non-overlay object `mickey.us.ld` names, plus the names
`undefined_funcs_auto`, `undefined_syms_auto` and `libultra_undefined_syms`
assign -- 4,886 of them, available before the link and without it. A call to
another *module's* function is deliberately excluded: that is the
generated-identity alias case of §5.3 and must keep its own name.

**The fix is the rebind the tree already does by hand.** Overlay 49's
POSTPROCESS rule rebinds the relocation at `0x218` from `func_800254FC` to
`overlay65UpdateReloc`, a per-module placeholder, and values *that*. The
generator now derives the same thing: `reloc_surface.py generate` renames every
undefined `R_MIPS_26` relocation against a resident-owned name to
`<name>_o<NNN>Reloc` in the object, then values the alias from the site like
any other placeholder. The resident name keeps its real address; the overlay
keeps its stored addend.

Three properties make it safe to run inside `generate --write`:

- **It is a no-op on the matching tree.** No overlay object in the current
  build carries an `R_MIPS_26` against a resident auto-name -- every matched
  resident call is already rebound by a hand-written rule -- so the generated
  block is byte-for-byte what it was, `--audit` stays at 100%, and
  `check-overlay-syms` reports no drift.
- **Checks are read-only.** The alias no longer matches the resident-name
  pattern, so a second write pass renames nothing and values the alias from the
  same site. `generate --check` never invokes `objcopy`; if an object still
  needs a resident-call rebind, the check names the object and required
  `POSTPROCESS` mapping and fails. `--compare` is also read-only.
- **It names what it cannot read, and a refusal is total.** Only `R_MIPS_26`
  sites are aliased. A resident symbol reached by *both* a call and a data
  reference is refused -- one placeholder cannot carry two different addends --
  as is a call in an object with no `text_ownership` row, which has no module
  offset. A refused name must then get **no value line under its global name
  either**, and that is the trap that made the first cut of this change worse
  than the problem it fixed: `synthesize()` still reads an addend for the
  refused symbol from its corroborated sites, and emitting
  `func_8002A8C0 = 0xf0000000` to help one overlay produced thirty-odd
  `relocation truncated to fit` errors in `shadows.c`, `camera.c`,
  `charControl.c` and `track.c`. `generate()` now drops a refused resident name
  from every object's values, the reason is reported once, and the trial
  classes the candidate `resident-call-unreadable`.

  **Corroboration is a note, not a refusal.** A call site the module's table
  does not name is where the candidate's schedule has diverged. Under the
  alias, an addend read there can only produce a differing word *inside* the
  promoted function -- which is the measurement the trial exists to take -- so
  the generator emits the value and a `/* NOTE … */` line naming the offsets
  rather than throwing the symbol away. This was measured, not assumed: the
  strict version cost `overlay34SortAndDraw` its 168-word reading and returned
  it to a bare build failure. Notes are printed under their own marker so a
  caller matching `UNRESOLVED` does not read a measurable candidate as a
  failure.


#### What it measured

Re-trialled: the 56 overlay candidates whose `NON_MATCHING` body names a
resident target (splat auto-name or `D_8…`), which is a superset of the 15
`resident-symbol-missing` and every `relocation-truncated` case among them.
**`resident-symbol-missing` and `resident-call-unreadable` are now zero.** The
15 the lane was pointed at:

| candidate | before | after |
|---|---|---|
| `overlay1UpdateAimedTransient` | `resident-symbol-missing` | `text-differs` 59 words (2 out of range) |
| `overlay4UpdateObjectMotion` | `resident-symbol-missing` | `text-differs` 15 |
| `overlay5InitializeAudio` | `resident-symbol-missing` | `text-differs` 22 |
| `func_overlay_011_F0001E4C_186A694` | `resident-symbol-missing` | `schedule-divergence-at-site` |
| `func_overlay_011_F00022E8_186AB30` | `resident-symbol-missing` | `schedule-divergence-at-site` |
| `overlay11UpdateMenu` | `resident-symbol-missing` | `text-differs` 4 |
| `func_overlay_026_F0000D24_187B11C` | `resident-symbol-missing` | `text-differs` 39 |
| `func_overlay_027_F0000064_187BA3C` | `resident-symbol-missing` | `schedule-divergence-at-site` |
| `func_overlay_029_F00005C4_187D874` | `resident-symbol-missing` | `schedule-divergence-at-site` |
| `overlay34CreateRecord` | `resident-symbol-missing` | `text-differs` 36 (4 out of range) |
| `overlay37RenderEffect` | `resident-symbol-missing` | `text-differs` 138 |
| `func_overlay_041_F0000854_1887B8C` | `resident-symbol-missing` | `rom-size` |
| `func_overlay_046_F0000120_188E518` | `resident-symbol-missing` | `rom-size` |
| `func_overlay_071_F0000278_18C9D98` | `resident-symbol-missing` | `text-differs` 28 |
| `overlay94UpdateController` | `resident-symbol-missing` | `text-differs` 13 |

Nine now carry an in-range word count and a linked-ROM oracle. Four are
`schedule-divergence-at-site`, which is the honest answer and a codegen problem:
`func_overlay_029`'s two remaining refusals are `ext_o0_2a4c0` and
`ext_o0_6ec00`, each demanding two and three different addends from sites that
disagree. Two moved to `rom-size`, which is a different scaffolding question.

`relocation-truncated (R_MIPS_26)` also disappears: the one case in this set,
`func_overlay_066_F00004E0_18C6948`, is now `text-differs` at 181 words. The
class is closed by construction -- it existed only because a resident-owned
name was being assigned -- but only the candidates whose source names a
resident target were re-trialled here, so a full `--overlays-only` sweep is what
would confirm the other three.

Two side effects worth recording. `overlay34SortAndDraw`, which is *not* one of
the 15, went from `text-differs` 168 to `resident-call-unreadable` and back to
168 across the two refusal policies -- it is the measurement that settled §5.6's
note-versus-refusal question. And `func_overlay_057_*`'s divergence report now
names `func_800508B4_o057Reloc` rather than a bare placeholder, which is the
aliasing showing its working.

#### A stale log is a wrong class

Six of the candidates above stayed in `resident-symbol-missing` after the
surface had already valued them, because `promotion_trial.py` classified from
`log1 + log2`. The first pass links against the *stale* surface -- that is the
whole point of the two-pass integration (§5.5) -- so its `undefined reference`
lines survive into the concatenation and `UNDEF_RE` reported symbols the second
link resolved perfectly well. Markers still come from both passes, since a
POSTPROCESS marker is printed by the compile; every *link* diagnostic now comes
from the second pass alone.

## 6. What the trial measures now

`tools/promotion_trial.py --overlays-only`, all 279 overlay candidates, lane
the full implementation:

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

Overlay 40 `+0x690` (`overlay40FadeRecords`) owns five HI16/LO16 pairs for
timer, current, target, duration, and output. The bounded output-origin C emits
all ten records and measures 98/101 words with an exact `0x8` frame. Its three
residual words are one `v0`/`v1` globalcolor outcome; 119 flag configurations
were nonexact, so current linked equality remains fallback-only.

Resident `func_80028FCC` owns three `R_MIPS_26` records to `func_80028FB8` at
`+0x14/+0x30/+0x4C`, all exact in fresh configured full-TU C under
`-Wo,-Olimit,100`. The retained spelling is 17/27 words with exact `0x6C`
boundary/frame `0x18` and first mismatch `+0x1C`. ORT 663 exports the function,
but exhaustive relocation, direct-call, and pointer scans authenticate no
caller. The shared-result probe regressed to 25 words and moved the latter two
calls; linked equality remains fallback-only.

Overlay 74 `+0xB8` (`overlay74Update`) owns eight runtime-authenticated records:
four calls at `+0x70/+0x128/+0x134/+0x178` and two HI16/LO16 pairs at
`+0xE8/+0xEC` and `+0x13C/+0x140`. Policy-clean configured C emits all eight at
those exact offsets and types. It remains nonexact at 39 relocation-masked
words because its frame is `0x70` versus target `0x60`, cascading through the
integer allocation. ORT 1285 exports the owned `+0xB8..+0x248` range; the sole
authenticated inbound is `func_8000AEEC+0x34C`. Following padding is separately
assembly-owned, and linked equality proves fallback only.

Resident `debug_text_width` owns five exact records: calls at `+0x18/+0x30` to
`sprintfSetSpacingCodes`, `+0x28` to `vsprintf`, and a HI16/LO16 pair at
`+0x4C/+0x50` to `D_8007CE98`. Fresh configured full-TU C emits all five at
59/66 words under canonical flags; all 119 flag rows were nonexact. ORT 862
exports it, but exhaustive relocation, direct-call, pointer, and source scans
find no caller. There is no target padding; linked equality proves fallback
only.

Eleven of these were not measurable before this lane. They are the sweep's next
targets: within eight words is the range where the permuter closes candidates.

Overlay 34 `+0x40C` (`overlay34UpdateRecords`) owns seven exact runtime
records. Three LOCAL HI16/LO16 pairs resolve to the active count, pointer array,
and float parameter; the LOCAL JUMP resolves to `overlay34RemoveRecord` at
module `+0x2C8`. Configured C is exact at 77 words with a `0x30` frame, all
seven records agree by offset/type/effective identity, and the linked owner,
complete overlay, and full ROM are exact.

## 7. What is still hand-written

- **Section externalization.** `externalize_elf_section.py` takes either the
  expected payload as a hex literal or its SHA-256 digest in the Makefile,
  which is the one part of the machinery not derivable from addresses alone.
  The digest form keeps larger private literal pools fail-closed without
  embedding their bytes in tracked text.
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
