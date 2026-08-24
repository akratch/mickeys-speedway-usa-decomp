# The module map

What lives where in Mickey's Speedway USA (USA), and how each claim was
established. This file is the naming convention later phases follow, plus the
address map those names hang on.

Two rules govern everything below.

1. **Every claim carries its evidence.** A range with no stated method is a bug
   in this document, not a fact about the ROM.
2. **Evidence tiers are never mixed.** A byte-identical match and a plausible
   reading of a disassembly are labelled separately here and in
   `symbol_addrs.us.txt`, and the labels are load-bearing.

All ROM offsets are byte offsets into `baseroms/mickey.us.z64`
(SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548`). For the resident segment,
`VRAM = ROM + 0x7FFFF400`.

---

## 1. Naming convention

### 1.1 The four tiers

| Tier | What it is | What may be adopted | Where it is used |
|---|---|---|---|
| **A — byte-identity** | The ROM's bytes are identical, under relocation masking, to a named symbol in a permitted decomp's *built* objects | The reference build's name, verbatim | the libultra corridor, DKR game code |
| **B — call graph** | The function's role is pinned by who calls it, with what, and what the caller does with the result | An invented descriptive name, or a public decomp's name where the role matches exactly | `piacs`/`siacs`, the linker cluster |
| **C — string correspondence** | The function builds the address of a distinctive string literal, and a public decomp of the same engine uses that literal in a function whose job matches | That project's name | `diRcp*`, `diCpu*`, `gsSndp*`, `osScGetTaskType` |
| **D — structural inference** | Read off the disassembly and nothing else | An invented descriptive name | `SetLinkSlot`, `ReleaseUnusedLinkSlots` |

Tier A is the only tier that is a *measurement*. B, C and D are arguments, and
an argument can be wrong; the tiers are kept apart so a reader can tell, at the
point of use, how much a given name is worth.

### 1.2 Tier A: the adoption threshold

A byte-identical match is adopted as a name only when **all three** hold:

- **at least 6 unmasked instruction words** compared. Masked words are the ones
  the reference object's own relocation records say the linker patches; they
  prove nothing.
- **unique across the whole 32MB image.** Not unique within the window that was
  scanned, a distinction that has already cost one wrong claim. `occ` in
  `tools/find_known_objects.py` is window-scoped; **`--rom-occ` computes the
  figure this bullet asks for**, and every name adopted since it existed
  carries a measured `romocc`. Where a comparison has no run of two consecutive
  unmasked words to anchor a whole-image search on, the tool prints `romocc=?`
  and the row is not adoptable on uniqueness grounds.
- **exactly one candidate name** for those bytes. Several different functions
  in one reference build routinely compile to identical instructions; when they
  do, byte comparison cannot say which one this is, and the row is noise.

**Six adopted symbols fall below the 6-word bar, resting on four distinct
arguments.** Every one is disclosed at its point of use in
`symbol_addrs.us.txt`, and each rests on something a word count does not
measure:

| Symbol | Words | What carries it instead |
|---|---|---|
| `osScGetCmdQ` | 2 | The **pair**: two *different* immediates (`addiu v0,a0,0x78`, `addiu v0,a0,0x40`), 8 bytes apart, in the reference's order, each ROM-wide unique |
| `osScGetInterruptQ` | 2 | as above |
| `stack_pointer` | 2 real | Semantically unambiguous on its own: the whole body is `jr ra` / `move v0,sp` |
| `__osGetCause` | 2 real of 4 | Same argument as `stack_pointer`: the body is `mfc0 v0, Cause` / `jr ra`, and **those two words occur exactly once in the 32MB image**, unlike the rejected `leointerrupt`, whose body alone occurs 39 times, so the padding is doing none of the work. Three reference builds name those bytes, all identically |
| `__osExceptionPreamble` | 2 unmasked of 4 | Shape: `lui k0,0x8005` / `addiu k0,-4032` / `jr k0` jumps to the very next instruction |
| `__osPopThread` | 4 unmasked | Island context: three other matches already establish that this is `exceptasm.s` |

Six rows, four arguments: `osScGetCmdQ` and `osScGetInterruptQ` share one, and
`__osGetCause` reuses `stack_pointer`'s. A pass that adds a fifth argument
should be treated as suspect; exceptions are meant to stay rare enough to
enumerate.

Passing the threshold is not the same as the name being *right about Mickey*.
Tier A establishes that the routine is the same routine; the reference build's
name may still describe that project's use of it. `weather_clip_planes` and
`audspat_jingle_off` are carried because renaming shared code would hide the
sharing, and are flagged where they appear.

**A whole-`.text` match outranks a standalone function match.** When both are
available for one address, the whole-object match wins and the standalone one
is recorded as noise; see the `__osPfsGetInitData` and `__osPiGetAccess`
collisions in `symbol_addrs.us.txt`.

### 1.3 When a public decomp's name is adopted, and how it is disclosed

`docs/CLEANROOM.md` permits public retail-derived decompilations (DKR, JFG, PD,
BK, Conker) as sources. This project reads **all five**, and
`docs/references.md` records each one's repo URL, pinned commit, verified
baserom checksum, build outcome and match yield. The two that supply most of
the tree are:

- **Diddy Kong Racing**, for tier A, its *built objects only*: compiled bytes
  plus symbol and relocation tables. That restriction is what makes tier A
  evidence rather than transcription, and it is a claim about the method, not
  about what a human was allowed to read: both projects' sources are permitted
  reading under `docs/CLEANROOM.md`, and DKR's was grepped for string literals
  during the tier-C work. It produced **no adopted name**; every DKR name in
  the tree came out of a built object.
- **Jet Force Gemini**: two uses, not to be conflated. Its *published source
  text* was read to answer "what did that project call the function that does
  this", and (in `src/main/runlink.c`) for adapted function bodies, disclosed
  at the point of use. Separately, its *built objects* are the largest single
  source of tier-A names in the tree: **84 of the 87 translation units that
  pass adopted are JFG's, carrying 187 of its 190 names**, on the same
  built-objects-only basis as DKR's.
- **Perfect Dark, Banjo-Kazooie and Conker**: built objects only. PD
  contributed three names (the Transfer Pak driver, which no other reference
  build contains). BK and Conker contributed **none**, and that documented zero
  is in `docs/references.md` alongside what they did contribute:
  independent corroboration of **2** and **8** of the adopted translation units,
  and re-confirmation of **73** and **65** subsegments Phase 1 had already
  named. Those are two different measurements and they are not summed; see
  `docs/references.md` §"Three different things, counted separately", and note
  that only the corroboration figures are recomputable from this tree. None of
  the three had its source read for any purpose.

Nothing in this tree was independently derived from Mickey's bytes and then
found to agree with a reference project. **The names are borrowed.** What the
byte comparison establishes is that the routine is the same routine; the name
comes from the other project, and that is what tier A means here.

Every borrowed body in this tree carries an explicit `PROVENANCE` disclosure at
the point of use, and so does every block of borrowed names.
`src/main/runlink.c` opens with one, because the bodies in it are adapted from
JFG's `runLink.c`; the tier-C block in `symbol_addrs.us.txt` carries another;
each `src/libultra/*.c` carries a one-line note that its body is SDK libultra
source as published in public decomp trees. The rule is not scoped to JFG:
**the provenance line goes in before the body.** Byte-identity against Mickey's
ROM is what makes the borrowing sound, not a reason to leave it unmentioned.

Where Mickey's ROM and a public decomp disagree, **Mickey wins and the
divergence is written down**. Current divergences: `MipsInstruction`'s field
order, `runlinkCallResumeFunction`'s do/while-15 scan, and `RelocTableEntry`,
whose layout is not JFG's at all (see §5.2).

### 1.4 Casing

Follow the surrounding code; do not impose a house style on borrowed names.

| Kind | Style | Examples |
|---|---|---|
| libultra | whatever the SDK calls it | `osRecvMesg`, `__osViSwapContext` |
| Engine code shared with DKR/JFG | that project's spelling, unchanged | `mtxf_transform_point`, `diRcpPrintDL`, `gzip_inflate_codes` |
| The runtime linker | JFG's `PascalCase` / `runlinkCamelCase` mix, unchanged | `ResolveRelocAddress`, `runlinkDownloadCode` |
| Mickey-specific inventions | `PascalCase` for functions, `lowerCamel` for data | `SetLinkSlot`, `overlayCount` |
| Structs / types | `PascalCase` | `OverlayHeader`, `RelocTableEntry` |
| Macros | `SCREAMING_SNAKE` with a family prefix | `RELOC_TYPE_*`, `SC_TASK_*` |

The mix is deliberate and is not worth normalising. A name spelled exactly as
DKR or JFG spells it can be grepped against that project; a name that has been
tidied cannot.

### 1.5 What must not happen

- No name without a stated tier and stated evidence.
- No name for a function whose C is parked non-matching; it would put a second
  evidence tier into the symbol file. Such names live in the source file's
  comments only.
- No name inherited from a reference build's *address placeholder*. Importing
  one would assert an address that does not exist in this game. Two such cases,
  different in kind: `func_80070058` (DKR's `math_util.s`) matched at ROM
  `0x2A90C` and is left unnamed with the reason recorded in
  `symbol_addrs.us.txt`; `func_800676F8` (JFG's `diCpu.c`) is what the string
  evidence for Mickey's `0x80045BBC` pointed at, so that address keeps its own
  `func_` name. Beyond those, **37 placeholder addresses fall inside the
  translation units the cross-title pass matched** (mostly inside JFG's large
  `n_audio` and `gsSnd` objects, which that project has matched but not yet
  named), and none was imported. The rule is applied by the generator, not by
  judgement, which is why the number can be this large without being a risk.

---

## 2. Top-level ROM map

| Range | Size | What | Evidence |
|---|---|---|---|
| `0x000000`–`0x000040` | 0x40 | header | splat `header` segment |
| `0x000040`–`0x001000` | 0xFC0 | IPL3 boot | standard N64 layout |
| `0x001000`–`0x086640` | 0x85640 | **resident segment** (code + data + rodata) | §3 |
| `0x086640`–`0x087000` | 0x9C0 | table of ROM offsets (unidentified) | entropy transition; still `bin` |
| `0x087000`–`0x16B0000` | 22.16 MiB | compressed assets | entropy 7.1–8.0 across the band |
| `0x16B0000`–`0x1848B70` | 1.60 MiB | unclassified | Nothing reaches it: the resident segment builds four address literals in `0x1600000`–`0x18FFFFF` and they are the four block bases below. Entropy 5.5–6.4 with a 0.35 MiB flat run |
| `0x1848B70`–`0x184C3E0` | 0x3870 | **the three overlay tables**, flat and uncompressed | §5.3 |
| `0x184C3E0`–`0x18F1FE0` | 0.65 MiB | **107 overlay module images** | §5.3 |
| `0x18F1FE0`–`0x2000000` | 7.06 MiB | `0xFF` fill | verified byte by byte |

The resident segment's end is derived, not guessed: the entrypoint zeroes BSS
from VRAM `0x80085A40`, and `0x80085A40 - 0x80000400 + 0x1000 = 0x86640`, so
the ROM only needs to supply bytes up to there.

### 2.1 The build stamp

ROM `0x7AD00` holds three consecutive string pointers: to `"1.1153"`,
`"18/08/00 13:08"` and `"pmountain"` at `0x80081A80`/`0x80081A88`/`0x80081A98`.
A version, a build timestamp and a build-host or branch tag. 18 August 2000 sits
about three months before the game's release, which is consistent with a
release-candidate build. Nothing in the resident segment's disassembly builds
the address of this triple, so whatever prints it is in an overlay.

---

## 3. The resident segment (`main`)

ROM `0x1000`–`0x86640`, VRAM `0x80000400`–`0x80085A40`, plus `0x52D10` of BSS.
Always present; the boot code jumps straight into it at `0x800211A0`, and
`mainproc` is byte-identical to DKR's at exactly that address.

Named anchors, in address order. **Tier A** rows are byte-identical to a
reference build's objects (DKR's, JFG's, Perfect Dark's, Banjo-Kazooie's or
Conker's; see `docs/references.md`); **tier C** rows are string-correspondence
with JFG; everything else is noted inline. Ranges without a named anchor are
omitted rather than guessed at. 171 translation units are matched whole across
the segment, carrying 190 function names.

| ROM | VRAM | Anchor | Tier | What it establishes |
|---|---|---|---|---|
| `0x1000` | `0x80000400` | `entrypoint` | A | The reset vector's target |
| `0x1AE60`–`0x1BE50` | `0x8001A260` | `main/lights2` | A | **Measured file boundary**: JFG's whole 0xFF0 `hasm/lights2.s`, 9 routines: the lighting pipeline, a starfield mover, a CPU line rasteriser, a rain draw. The first anchor anywhere in `0x16140`–`0x1C790` |
| `0x31C4` | `0x800025C4` | `audspat_jingle_off` | A | Spatial audio, and the thinnest row adopted |
| `0xC9B4`, `0xF520` | — | `"track/track.c"` asserts | — | **`track` code is partly resident** |
| `0x21DA0` | `0x800211A0` | `mainproc`, `thread1_main` | A | `main.c` proper, at the boot target |
| `0x27BB4`, `0x28BB8` | — | `"main/main.c"` asserts | — | **`main` code is resident** |
| `0x29FD0` | `0x800293D0` | `"x = %5d"` … `"a = %3.1f"` | — | On-screen coordinate readout |
| `0x2A250`–`0x2AE44` | `0x80029650` | 11 named `math_util.s` routines | A | Matrix / vector / RNG library. 13 routines matched: 11 named here, `rand_range` already carried as `mathRnd`, and `func_80070058` left unnamed as a placeholder |
| `0x2B650`–`0x2BCD0` | `0x8002AA50` | `main/matrix` | — | The parked float TU (§6.2) |
| `0x2C860` | `0x8002BC60` | `align16`/`align8`/`align4` | A | The allocator |
| `0x2F400`–`0x323A0` | `0x8002E800` | `osScGetCmdQ`, `osScGetInterruptQ`, `osScGetTaskType` | A + C | **The game scheduler** |
| `0x316E8` | `0x80030AE8` | `"SP CRASHED"`, `"Version %s"` | — | The frame loop / RCP watchdog |
| `0x323A0`–`0x323E0` | `0x800317A0` | `main/rsp_segment` | A | Measured file boundary (whole `.text`) |
| `0x323E0`–`0x33FA0` | `0x800317E0` | `main/runlink` | A/B/C | **The runtime overlay linker** (§5) |
| `0x33FA0`–`0x34180` | `0x800333A0` | `main/trapDanglingJump` | A | The overlay call trampoline. **Measured file boundary**: JFG's whole 0x1E0 `hasm/ido/trapDanglingJump.s`. Was named at tier B from Mickey's call graph alone; the bytes agree |
| `0x342A8` | `0x800336A8` | `"Ntsc LowRes"` … | — | Video-mode table (15 entries) |
| `0x39A1C` | `0x80038E1C` | `"front/front.c"` asserts | — | **`front` code is partly resident** |
| `0x3B1A0` | `0x8003A5A0` | `"UNKNOWN TRACK"` | — | Track selection |
| `0x3B57C` | `0x8003A97C` | `weather_clip_planes` | A | |
| `0x3D5F0`–`0x43470` | `0x8003C9F0` | `main/particles` | A + B + D | 44-function resident particle TU; §3.4 |
| `0x43470`–`0x45760` | `0x80042870` | `main/diprint` | A + B + C | 19-function formatting/debug-text TU; §3.4 |
| `0x459C0`–`0x467BC` | `0x80044DC0` | `diRcpPrintDL`, `diRcpMoveWd`, `diRcpStrName`, `diRcpOtherMode`, `diRcpGeometryMode` | C | **The display-list disassembler**, a full GBI pretty-printer left in the retail build |
| `0x467BC`–`0x47A60` | `0x80045BBC` | `diCpuReportWatchpoint`, plus the memory/module debug pages and the register-dump crash reporter | C | **The debug monitor**, also left in |
| `0x47A60`–`0x47A70` | `0x80046E60` | `main/get_stack_pointer` | A | Measured file boundary |
| `0x4E378` | `0x8004D778` | `byteswap32` | A | |
| `0x4EA60`–`0x4F4D4` | `0x8004DE60` | `main/gzip_asm` | A | **Measured file boundary**: DKR's whole 0xA74 inflate core, in one piece |
| `0x4FC30`–`0x505E0` | `0x8004F030` | `libultra/exceptasm` | A | **Measured file boundary**, 9 routines including `__osException` and `__osDispatchThread`; §4.2. `0x4FC20` before it is the **rejected** `io/leointerrupt` match, and `0x505E0`–`0x506D0` after it is a separate unknown |
| `0x50820`–`0x50C00` | `0x8004FC20` | `main/refractOutputAssembler` | A | Measured file boundary (JFG) |
| `0x59B90`–`0x59BF0` | `0x80058F90` | `main/osBootRamTest` | A | Measured file boundary (JFG): the IPL3 6105 RAM test |
| `0x5C310`–`0x5E6B0` | `0x8005B710` | `main/gsSnd` | A | **The sound player**, 0x23A0 in one piece, 22 named functions. Two of those names were predicted at tier C from error strings and fall inside this TU at exactly the predicted addresses |
| `0x5E6B0`–`0x6AF90` | `0x8005DAB0` | libultra's `n_audio` synthesis library | A | 45 consecutive measured file boundaries, 106 names, plus two JFG maths TUs interleaved (`math_atan`, `math_acosf`); a third, `math_arc`, begins at `0x6AF90` immediately after. §4.2 |
| `0x6B3D0`–`0x6F3E0` | `0x8006A7D0` | Transfer Pak, Rumble Pak, Controller Pak filesystem | A | 18 measured file boundaries, 34 names. The Transfer Pak three come from **Perfect Dark**, the only reference build that has them; §4.2 |
| `0x6F420`–`0x76D10` | `0x8006E820` | the libultra corridor | A | §4.1 |
| `0x76D10`–`0x76E60` | — | non-resident text | — | Indexes off `$at`, loads from address 0; relocated before it runs. Still `bin` |
| `0x76E60`–`0x81590` | `0x80076260` | `.data`, mostly undifferentiated; selected SDK tails are TU-owned | — | §6.3 |
| `0x81590`–`0x86640` | `0x80080990` | `.rodata`, mostly anonymous; selected SDK tails are TU-owned | — | §6.3 |

### 3.1 Which modules are resident

The ROM carries `__FILE__` path strings from `assert`-style call sites, and
where those strings are *referenced from* is direct evidence of where a
module's code lives.

| Module | Path string copies | Referenced from | Conclusion |
|---|---|---|---|
| `main` | 6, at `0x80081B0C`–`0x80081B48` | resident: `0x80026FB4`, `0x80027FB8` | Fully resident |
| `track` | 14, at `0x80081540`–`0x80081610` | resident: `0x8000BDB4`, `0x8000E920` | **Partly resident** |
| `front` | 2, at `0x800826C0`, `0x800826D0` | resident: `0x80038E1C` | **Partly resident** |
| `clone` | 2, at ROM `0x188B4D0`, `0x188B4E0` | not referenced from the resident segment at all | **Overlay-only**: both strings sit in **overlay 43**'s `.data`, at offsets `0x1500`/`0x1510` from that module's base (§5.3) |

`main` is the permanently resident module; `front` and `track` straddle the
boundary, with resident stubs or shared helpers that carry their own assert
strings; `clone` exists only inside the overlay region. The scheduler's task
taxonomy agrees independently: `SC_TASK_CLONE` is one of its seven task types
(`include/game/sched.h`), so `clone` is a task, i.e. something scheduled rather
than something always present.

`main`, `front`, `track` and `clone` are **source-file names, not overlay
boundaries**. The ROM's overlay segmentation is 107 modules (§5.3); `clone`'s
two `__FILE__` copies land in one of them, and nothing says a source module maps
to one overlay.

### 3.2 What the debug content says about this build

Two substantial debug subsystems survive into the retail ROM: a complete GBI
display-list disassembler (`0x459C0`–`0x467BC`, every `G_*` and `RM_*` name
spelled out) and a debug monitor with memory-region pages, a module list and a
full register-dump crash reporter (`0x467BC`–`0x47A60`). Together that is
roughly 8KB of code plus 4KB of strings, and it is why so much of the resident
segment can be identified from strings alone. It also means the *linker* is
observable from the outside: the crash reporter calls `runlinkGetAddressInfo`
to turn a faulting address into "Module %d at %08x".

### 3.3 Resident TU map from skeleton donors

A masked-instruction-skeleton scan (registers, immediates and jump targets
masked; opcode/funct/fmt kept, so same-source-same-compiler code matches
regardless of register allocation) was run against every function of at
least 10 words in Jet Force Gemini's and Banjo-Kazooie's built objects, over
the still-unnamed code of ROM `0x1000`–`0x6F420`. It found 88 unambiguous
hits (one candidate reference name apiece); two, both inside ROM
`0x76D10`–`0x86990` (the `.data`/`.rodata` tail, not code), turned out to be
JFG float-literal symbols placed inside `.text` and were discarded as
meaningless there. Four more (`0x8001A2C4`, `0x8001A4BC`, `0x8001A774`,
`0x8001A9A4`) land inside the already-measured `main/lights2` whole-file
boundary (§3, table) and are not new. Eleven did not survive independent
re-verification with `tools/find_known_objects.py` against the same
reference build (real relocation-record masking, not the coarse opcode-class
mask): no exact byte match exists at that address once actual register
allocation is compared, so the coarse scan's hit there is a same-shape
coincidence, not a same-source one. They are not adopted:
`Sinf`, `fmvInit`, `camStopShakes`, `camSetZoom`, PD's `osCreatePiManager`,
and six JFG `func_`-placeholder hits.

The remaining 71 were independently re-verified byte-for-byte (masked words
under real relocation records, `romocc` computed): 68 clear the tier-A bar in
full (docs/modules.md 1.2) and are adopted, one at `0x8002D824` fails on
uniqueness (Banjo-Kazooie's `unallocUnusedBlock`, `romocc=4`) and two more
fall short of the 6-unmasked-word floor or leave `romocc` unresolved
(`texLoadTextureAddr` at 5 words; `viFrameRateReset`, `romocc=?`) -- all three
left unnamed, consistent with 1.2's "not adoptable on uniqueness grounds" for
an unresolved `romocc`. Four further hits (`matrix_RPY_XYZ`,
`matrix_XYZ_YPR_SCL`, `matrix_XYZ_YPR`, `matrixTransposeVectorMultiply`, all
inside `main/matrix`) clear the bar but are **not** written into
`symbol_addrs.us.txt`: their C is parked non-matching, and 1.5 forbids naming
a symbol whose C is not in the ROM. They are recorded instead in §6.2.

Of the 68 adopted, 46 carry a real JFG/BK function name (adopted verbatim,
`symbol_addrs.us.txt`); 22 are JFG placeholder names (`func_8xxxxxxx`), which
1.5 forbids importing, so Mickey's own `func_<VRAM>` stands and the comment
records only the donor translation unit.

**What this adds to the TU picture**, one row per donor TU, functions found
in each and the ROM span of just those functions (not a boundary claim --
see the caveat below):

| Donor TU | Functions found | ROM span of finds | Status |
|---|---|---|---|
| `libultra/n_csplayer.c.o` | 5 | `0x5E970`–`0x61828` | Inside the already-measured `libultra/n_csplayer` boundary (§ table); corroborates it |
| `gsSnd.c.o` | 6 | `0x5C578`–`0x5DFA4` | Inside the already-measured `main/gsSnd` boundary; corroborates it |
| `libultra/n_drvrNew.c.o` | 1 | `0x659C0` | At the exact start of the already-measured `libultra/n_drvrNew` boundary; corroborates it |
| `libultra/n_env.c.o` | 1 | `0x6910C` | Inside the already-measured `libultra/n_env` boundary; corroborates it |
| `libultra/n_load.c.o` | 1 | `0x6A634` | Inside the already-measured `libultra/n_load` boundary; corroborates it |
| `hasm/ido/math_util.s.o` | 15 | `0x2A9E4`–`0x2B644` | Inside the already-measured `main/math_util` boundary; corroborates it |
| `src/menu.c.o` | 6 | `0x3A184`–`0x3B008` | Inside yaml's unnamed `0x37D50`–`0x3B480` block. No whole-`.text` match found, so no boundary is claimed |
| `src/gameVi.c.o` | 4 | `0x34B68`–`0x34E60` | Inside yaml's unnamed `0x34180`–`0x37D50` block. No whole-`.text` match; no boundary claimed |
| `src/anim.c.o` | 3 | `0x50D7C`–`0x51D28` | Inside yaml's unnamed `0x50C00`–`0x58570` block. No whole-`.text` match; no boundary claimed |
| `src/models.c.o` | 3 | `0x20020`–`0x21710` | Inside yaml's unnamed `0x20020`–`0x21DA0` block, starting exactly at its boundary. No whole-`.text` match; no boundary claimed |
| `src/font.c.o` | 2 | `0x4BC70`–`0x4C884` | Inside yaml's unnamed `0x4BC40`–`0x4EA60` block. No whole-`.text` match; no boundary claimed |
| `src/audio_manager_4C50.c.o` | 2 | `0x45F0`–`0x4F3C` | Starts exactly at yaml's `0x45F0` boundary; ends inside the unnamed `0x4F40`–`0xC950` block. No whole-`.text` match; no boundary claimed |
| `src/audio_manager_1050.c.o` | 3 | `0x12BC`–`0x22C8` | Inside yaml's unnamed `0x1050`–`0x45F0` block. Wide span for 3 hits -- other code plainly sits between them; no boundary claimed |
| `src/charControl.c.o` | 2 | `0x1CED4`–`0x1FFAC` | Inside yaml's unnamed `0x1C790`–`0x20020` block. No boundary claimed |
| `src/camera.c.o` | 2 | `0x23360`, `0x5B778` | 230KB apart -- evidently not one placed TU here; treat as two independent identifications, not a span |
| `src/memory.c.o` | 2 | `0x2BCD0`–`0x2C3AC` | Starts exactly at yaml's `0x2BCD0` boundary (end of `main/matrix`); the already-named `align16`/`align8`/`align4` (tier A, `memory.c.o`) sit at `0x2C860`, past this span. Consistent with one TU, no boundary claimed |
| `src/shadows_214A0.c.o` | 2 | `0x18FF0`–`0x19144` | Inside yaml's unnamed `0x18FF0`–`0x1AE60` block, starting exactly at its boundary. No boundary claimed |
| `src/saves.c.o`, `src/rcpFast3d.c.o`, `src/track.c.o`, `src/textures.c.o`, `src/diCpu.c.o`, `src/objects.c.o`, `libultra/src/flash/flashreadid.c.o`, `us.v10/src/core1/code_1D00.c.o` (BK) | 1 each | single points | Isolated identifications, no span to claim |

**Why no new `mickey.us.yaml` split accompanies this table.** §1's "measured
file boundary" tier requires a whole-`.text` match; this pass only matched
individual functions (`tools/find_known_objects.py --sections` found no
whole-object match for any of the not-yet-named TUs above). Asserting a yaml
`asm`/`c` split from function-level hits alone would claim more than was
measured, exactly the mistake 1.2's uniqueness clause exists to prevent one
level up. The already-measured TUs above (`n_csplayer`, `gsSnd`, `n_drvrNew`,
`n_env`, `n_load`, `math_util`) needed no new split; they already have one.

### 3.4 Particle and debug-print translation units

ROM `0x3D5F0`–`0x45760` is now split into two aligned resident C
subsegments. The complete per-function census, including exact sizes and the
evidence tier on every adopted name, is in `symbol_addrs.us.txt`; the source
files keep unresolved functions as `GLOBAL_ASM`, so the split itself claims no
new matched bytes.

| Mickey TU | ROM / VRAM | Functions | Evidence |
|---|---|---:|---|
| `main/particles` | `0x3D5F0`–`0x43470` / `0x8003C9F0` | 44 | **A:** DKR's built `particles.c.o` identifies `reset_particles` byte-for-byte. **B:** the internal call graph and external particle callers. **D:** the full function order and masked-skeleton sequence track JFG's 42-function `particles.c.o` from `partFreeLib` through `partNullifyCircularParticleParents`; Mickey inserts two extra 12-byte state setters before `partUpdateTriggers`, after which the sequences reconverge. |
| `main/diprint` | `0x43470`–`0x45760` / `0x80042870` | 19 | **A:** DKR objects identify `strcpy`, `memset`, and `sprintf` exactly. **B:** `diPrintf` brackets `vsprintf` with `sprintfSetSpacingCodes`, `diPrintfAll` drives the parse/background/character/bounds/origin helpers, and later `diRcp*` routines call `sprintf`. **C:** `_itoa` owns both digit alphabets and `vsprintf` owns `(null)` and `(nil)`. The order matches JFG's `diprint.c.o`, with DKR's `debug_text_width` inserted between `diPrintfSetXY` and `debug_text_parse`. |

**PROVENANCE.** The `part*`, `diPrintf*`, and `debug_text_*` names and both TU
attributions are adapted from Jet Force Gemini's public `src/particles.c` and
`src/diprint.c` plus their built objects. Diddy Kong Racing's public
`src/printf.c` supplies `debug_text_width`, while its built
`unused_string.c.o`, `printf.c.o`, and `particles.c.o` supply the tier-A rows
stated above. JFG address-placeholder names are not imported: Mickey's own
`func_<VRAM>` names remain. Mickey's bytes and call graph decide every
disagreement.

No function in either range uses an odd single-precision floating-point
register. None is therefore classified as handwritten assembly by §6.2's
criterion.

---

## 4. libultra

### 4.1 The corridor: ROM `0x6F420`–`0x76D10`

VRAM `0x8006E820`–`0x80076110`, `0x78F0` bytes. **95 named subsegments, every
one of them a measured whole-`.text` file boundary, and 123 named functions**,
all tier A. The yaml carries the boundary argument at both ends and
`symbol_addrs.us.txt` carries the per-function names.

**Where the drift went.** The first sweep, against DKR's built libultra alone,
named 80 subsegments and 107 functions and left `0x1AE0` (22.2% of the
corridor, in ten runs) unnamed: libultra-shaped code not byte-identical to
DKR's build, with a best-alignment fuzzy scan at 35% tolerance returning zero
candidates. Those runs are not drifted copies of DKR's libultra; they are a
*different build*. Run the finder over Jet Force Gemini's libultra and **eight
of the ten runs fall**, in fifteen whole-`.text` matches.

The remaining unnamed code is `0xB50`, **9.4% of the corridor**, in two
contiguous runs:

| ROM | Size | Note |
|---|---|---|
| `0x70AF0`–`0x70E20` | `0x330` | Between `dpsetstat` and `pfsdeletefile` |
| `0x74090`–`0x748B0` | `0x820` | Between `timerintr` and `vigetcurrcontext` |

Neither matches any object in any of the five reference builds, whole or
per-function: five negatives, two of them from byte-perfect builds.

**Read these two runs rather than mining them further.** "Unnamed code inside
the corridor is libultra-shaped" was a fair assumption at 78% identified
against a single build; at 90.6% against five it is carrying more weight than
it has earned, and the plainest reading of `0xB50` that five libultra builds do
not contain is that some of it **is not libultra**. Two more reference builds
would be a sixth and seventh negative; a disassembly would be an answer.
Disassemble `0x70AF0`–`0x70E20` and `0x74090`–`0x748B0` before running the
finder over anything else.

`__osPiGetAccess` ("the same 17 instructions as DKR's, scheduled differently")
is named from JFG's whole `io/piacs.c` TU at `0x80071B80`. `libultra/piacs`
was the one corridor subsegment named without a measured boundary; JFG measures
it, which is why this section says 95 of 95.

### 4.2 libultra outside the corridor

**The corridor is where libultra is contiguous, not where libultra exists.** A
whole-image sweep against four more reference builds found **68 translation
units of libultra below the corridor**, in three blocks.

| ROM | VRAM | What | TUs | Names | From |
|---|---|---|---|---|---|
| `0x4FC30`–`0x505E0` | `0x8004F030` | `os/exceptasm.s`, the exception handler and thread dispatcher | 1 | 9 | JFG |
| `0x5E6B0`–`0x6AF90` | `0x8005DAB0` | the `n_audio` synthesis library (45 TUs) with two JFG maths TUs interleaved | 47 | 106 | JFG |
| `0x6B3D0`–`0x6F3E0` | `0x8006A7D0` | Transfer Pak, Rumble Pak, Controller Pak filesystem | 18 | 34 | JFG + PD |

Plus the two scheduler accessors at `0x30F10`/`0x30F18` (`osScGetCmdQ`,
`osScGetInterruptQ`, `sc/sched.c`), which remain the only libultra named below
the corridor that is *not* part of a measured TU.

**The exception island is stock libultra.** The whole `0x9B0` `.text` of JFG's
built `os/exceptasm.s.o` matches ROM `0x4FC30` in one piece, 76 of 620 words
masked, ROM-wide unique, including `__osException`, `__osEnqueueThread`,
`__osDispatchThread` and `__osEnqueueAndYield`, which match nothing in DKR's
build at all. The TU ends at `0x505E0`, not `0x506D0`; the `0xF0` bytes between
are a separate, still-unidentified run, split off in the yaml. The code stays
`asm`: it is hand-written assembly, and identifying it did not require
decompiling it.

**The corridor's lower boundary.** The yaml once justified `0x6F420` partly on
"the function ending at `0x6F3DC` is game-shaped (saves s5/s6)". That function
is `corrupted`, the last routine of libultra's own `io/pfschecker.c`: the
observation was accurate, the inference from it was not. libultra is contiguous
from `0x6B3D0` to `0x76D10` apart from `0x1120` of unnamed code, of which
`0xB50` is the corridor drift above, `0x590` is between the Transfer Pak
routines, and `0x40` is the hole at `0x6F3E0`–`0x6F420`.

### 4.3 What was rejected, and why

Ten candidates below the corridor have been rejected or reconsidered; all are
listed, because a rejection is a result and because two of them proved
revisable once more reference builds existed.

**Reversed**: the reason held against the evidence then available and does not
hold against the evidence now:

| ROM | Was | Now |
|---|---|---|
| `0x61990` | `alCSPGetState`, rejected: a one-line accessor whose ROM-wide uniqueness is an accident of padding | `n_alCSPGetState`, adopted: the address falls inside a whole-`.text` match (`0x20`, 0 masked, `romocc=1`), so what places it is the translation unit, not the accessor's own bytes. The `n_` prefix is also new information: this is the n_audio variant |
| `0x620E0` | `alCSeqGetTicks`, same objection | `n_alCSeqGetTicks`, adopted, inside `n_cseq.c`'s whole `0x9D0` TU |

**Standing:**

- `io/leointerrupt` (`0x4FC20`): whole `.text` matched, ROM-wide unique, and
  the entire body is `jr ra` / `move v0,zero`. The uniqueness is an accident of
  8 bytes of padding, and that is measured rather than asserted: the section
  occurs once in the image, the two-instruction body alone occurs **39 times**.
  Banjo-Kazooie also matches here and offers only the placeholder
  `func_8038AAB0`.
- `__osDisableInt`/`__osRestoreInt` (`0x2A25C`/`0x2A288`): `romocc=2`, and the
  bytes are the tail of the larger, ROM-wide-unique
  `interrupts_disable`/`interrupts_enable` in DKR's `math_util.s`, which is why
  they matched at an offset of `0xC`. Not, as once read, a second copy of
  `os/interrupt.s`.

**From the cross-title pass:**

| ROM | Candidate | Ground |
|---|---|---|
| `0x20008` | Conker `src/init_3920.c.o` | Places only `func_10003920`, a reference-build address placeholder (§1.5) |
| `0x2C8AC` | PD `game/stubs/game_11eff0.o` | `stub0f11eff0`, an 8-byte stub. Semantically vacuous, same ground as `leointerrupt` |
| `0x58E40` | ten objects across four titles | A `0x10` `return x->field`. **Ten candidate names**, `romocc=?`. Fails §1.2's one-candidate rule outright, and the `?` is the tool declining to answer rather than a `1` |
| `0x63B40`, `0x68110`, `0x76C80` | Conker `n_synaddplayer`, PD `n_save`, Conker `ldiv` | **Subsumed.** Each is a fragment of a TU already matched whole at a lower address. §1.2's "a whole-`.text` match outranks a standalone function match" |
| `0x2A2B0` | BK `src/mgu/mtxxfmf.o` → `guMtxXFMF` | A clean whole-object match (`0xA0`, 0 masked, `romocc=1`), and its **boundary is still not declared**. The surrounding evidence is stronger and says something different: `0x2A250`–`0x2AE44` is thirteen routines matched against DKR's *one* hand-written `math_util.s` object, in DKR's order. Mickey inlines the SDK's `guMtxXFMF` into a hand-written maths file, exactly as DKR does, so declaring a TU boundary here would assert a file that is not there. The identification is recorded in `symbol_addrs.us.txt` §6; the address keeps `mtxf_transform_point` |

### 4.4 n_audio: matching progress

Per docs/acceleration-survey.md §13.3's ruling, `n_audio` is unblocked:
JFG's fully-matched naudio tree (`libultra/src/naudio/`, commit
`c75c270`, "Finish matching libultra naudio files") supplies matched C
bodies with `PROVENANCE` notes, using JFG's headers (imported under
`include/n_audio/**` and `include/n_libaudio.h`) and the bare-`OPT_FLAGS`
(`-g`, no `-O`) + `-mips2 -32` flag group already measured on
`n_cspsetvol`.

25 of the 45 `n_audio` TUs are matched (`n_cspsetvol`, `cents2ratio`
adopted before this pass; 23 more from it): every masked=0/1/2 TU (the
thin `N_ALEvent` posters and one-line accessors), `n_sl` (which places
the driver singletons `n_alGlobals`/`n_syn` — VRAM `0x80080160`/
`0x80080164`, measured directly off a built candidate diffed against the
ROM with `tools/wb_compare.sh --rom n_alInit --show-diff` and carved out
of the resident `.data` band in `mickey.us.yaml`, since almost everything
else in the library reads `n_syn`), the seven `ALParam`-update setters
that funnel through it, `n_synallocfx`, `n_alcspchan` (needs
`-DRAREDIFFS` for Rare's added MIDI control-change codes), and
`n_syngetfxref`.

**Plateaus, each with a first mismatch:**

- `n_synsetfxparam` (masked=6): `n_alSynSetFXParam` alone matches;
  its sibling `n_alSynSetOutputLPParam` in the same TU pulls a `0.1f`
  float literal from the wrong offset in the still-undifferentiated
  `.rodata` pool, off by `0x20` — a `.rodata`-ordering question beyond
  this pass.
- `n_resample` (masked=8): `n_alResamplePull`'s tail diverges
  structurally from the ROM (an extra `jal` the real function doesn't
  have); needs a closer read of the loop/branch shape before another
  attempt.

Remaining unmatched, roughly by size: `n_synthesizer` (masked=173,
`0xAD0`), `n_csplayer` (masked=154, `0x3220`), `n_reverb` (masked=60,
DSP-heavy, deferred per plan), `n_env` (masked=59), `alsurround`
(masked=39), `n_event`/`n_drvrNew` (masked=34 each), `n_synaddplayer`
(masked=24), `n_mainbus`/`n_synallocvoice` (masked=22/23), `n_cseq`
(masked=15), `n_seqplayer` (masked=14, the 15-function DSP-heavy TU,
deferred per plan), `n_alLPFilter` (masked=13), `n_auxbus` (masked=7),
`n_load` (masked=4, DSP-heavy ADPCM decoder), and `n_synsetvol`/
`n_synstartvoiceparam`/`n_synallocvoice` (masked=5) not yet attempted.

---

## 5. The overlay system

### 5.1 What runs it

The resident segment carries a complete Rare/DKR-lineage runtime linker at ROM
`0x323E0`–`0x33FA0`, plus its trampoline at `0x33FA0`. Six of its functions are
decompiled and byte-matched; four more are named from Mickey's call graph. The
mechanism, entirely from Mickey's own disassembly:

1. A call to a function that lives in a not-yet-loaded overlay is assembled as
   a `jal` to **`TrapDanglingJump`** (`0x800333A0`).
2. `TrapDanglingJump` saves every argument register (`a0`–`a3`, `f12`–`f15`,
   `v0`, `v1`) and computes `ra - 8`, the address of the `jal` that reached it.
3. It searches **`mainRelocTable`** (8-byte entries) for the entry whose call
   site is that address, which yields an index into **`overlayRomTable`**.
4. That 4-byte entry splits into a 12-bit overlay number and a 20-bit offset
   (`RomTableEntry`).
5. **`runlinkDownloadCode`** (`0x80031C78`) loads the overlay and relocates it,
   calling `ProcessRelocationEntry` per record and finishing with
   `osInvalICache`, genuine self-modifying code.
6. The trampoline recomputes `overlayTable[n].vramBase + offset`, restores the
   arguments and `jr`s to the real function. The caller never knows.

Step 0 is `runlinkInit` (`0x800328CC`), which runs once at boot, out of
`func_80026E4C`, immediately before the first `TrapDanglingJump`: it allocates
and DMA-copies the three tables out of ROM and synthesizes `overlayTable[0]`
for the resident module itself. §5.3.

`runlinkGetAddressInfo` (`0x800331E4`) is the inverse, and is what the debug
monitor uses to print "Module %d at %08x". Its fourth parameter is an optional
symbol-name out-pointer filled by **`GetSymbolName`** (`0x800317E0`), which in
this retail build is four instructions that spill their argument to the stack,
never read it back, and return the constant string `"unknown"`. The ROM-side
symbol table the mechanism is built around is simply absent from the shipped
image.

### 5.2 The tables

Named from stride and use in Mickey's disassembly (`symbol_addrs.us.txt`), all
six in BSS, all six written by `runlinkInit` and by nothing else:

| Symbol | VRAM | Element | Stride | Filled from |
|---|---|---|---|---|
| `overlayTable` | `0x800D2D90` | `OverlayHeader` | `0x20` | ROM `0x184B680`, at `+0x20` |
| `mainRelocTable` | `0x800D2D94` | `RelocTableEntry` | `0x8` | ROM `0x1848B74` |
| `overlayRomTable` | `0x800D2D98` | `RomTableEntry` | `0x4` | ROM `0x1849730` |
| `overlayCount` | `0x800D2D9C` | count = 108 | — | `(0xD60 >> 5) + 1` |
| `mainRelocTableCount` | `0x800D2DA0` | count = 375 | — | the word at ROM `0x1848B70` |
| `linkSlotTable` | `0x800D2E48` | `LinkSlot` | `0x2` | allocated and zeroed |

`overlayCount` is 108 and there are 107 headers in ROM: entry 0 is synthesized
for the resident module, so overlay *n* is ROM header *n - 1*.

`overlayCount` bounds **both** the overlay table and the link-slot table, so
there is exactly one link slot per overlay. That is the best available
evidence for what `LinkSlot`'s two fields mean, and still not enough to promote
them out of inference.

`RelocTableEntry` (`include/game/runlink.h`) is **not JFG's layout**: Mickey
puts the ROM-table index first and packs the call site as a 24-bit offset from
`0x80000450` in the high bits of the second word. Derived from Mickey's ROM;
only the type's name is borrowed.

### 5.3 Where the tables are in ROM

Four blocks, stored flat and uncompressed, back to back:

| ROM | Size | Contents |
|---|---|---|
| `0x1848B70` | `0x4` | `u32 mainRelocTableCount` = **375** |
| `0x1848B74` | `0xBB8` | `RelocTableEntry[375]`, then four bytes of pad |
| `0x1849730` | `0x1F50` | `RomTableEntry[2004]` |
| `0x184B680` | `0xD60` | `OverlayHeader[107]` |
| `0x184C3E0` | `0xA5C00` | **107 overlays**, each image `[.text][.data][relocTable1][relocTable2]` |

`runlinkInit` (`0x800328CC`) computes each block's size as the difference of two
ROM address literals, allocates, and DMA-copies it through `func_8002E3E0`,
which is `osInvalDCache` plus a loop of `osPiStartDma` and `osRecvMesg` in
`0x400`-byte chunks. **The one thing that hid this layout is a single word**:
the relocation block opens with its own entry count, and the initializer sets
`mainRelocTable = copy + 4`, so decoding 8-byte entries from `0x1848B70` reads
every entry one word out of phase and swaps its two fields.

The three table blocks remain `bin` segments in `mickey.us.yaml`.
`gmake overlay-tables` (`tools/overlay_tables.py`) decodes them and the module
images into a 107-row map, re-asserting the five checks below against the ROM
on every run.

**What makes the layout a measurement rather than a reading.**

- For all 107 modules, with zero mismatches,
  `next.romAddress - this.romAddress == textSize + dataSize + relocTableSize +
  relocTableSize2`. The last module ends at `0x18F1FE0`, byte-exact with the
  `rom_fill` boundary the ROM map already had. No rodata term fits that sum,
  which is what makes `OverlayHeader[0x10]` `bssSize` (`include/game/runlink.h`).
- 370 of the 375 relocation entries decode to a call site whose ROM word is
  literally `0C00CCE8`, i.e. `jal TrapDanglingJump`. That is the same 370 as the
  `jal TrapDanglingJump` sites in `asm/`, and it settles §5.1's mechanism from
  the shipped bytes: an unloaded call site *is* a real `jal` in the ROM image.
  The five that are not are two `HI16`/`LO16` `SYMBOL` pairs patching a
  `lui`/`addiu` of `0x800D2DC4` and one `R_MIPS_32` patching a data word, which
  is what their flags bytes say they are and what a `jal` test correctly
  rejects.
- `clone/clone.c`'s two `__FILE__` strings (§3.1, located from resident rodata
  and independently of any of this) land at offsets `0x1500`/`0x1510` inside
  **overlay 43's `.data`** under this arithmetic, with nothing fitted.
- The maximum `romTableIndex` any relocation entry uses is 1473, inside
  `RomTableEntry[2004]`; the maximum real overlay number in `overlayRomTable` is
  107, and its reserved selectors `0xFFF` and `0xFFD` appear 136 and 97 times,
  confirming `RomTableEntry`'s 12/20 bitfield split.
- Every one of the 107 headers has `vramBase == 0` and a strictly monotonic
  `romAddress`: the modules ship unrelocated and are placed at load time.

**Nothing in this path decompresses.** `runlinkInit` and `runlinkDownloadCode`
both copy through `osPiStartDma`, and neither call graph reaches `gzip_asm`
(`0x4EA60`), the resident decompressor the asset loader uses. The block bytes
decode as the documented structs directly.

The per-module relocation tables are `RelocationEntry[]` as
`include/game/runlink.h` documents, `relocTableSize`/`relocTableSize2` bytes
each. Across all 107 modules, table 1 is 6943 records that are overwhelmingly
`SYMBOL` (`symbolIndex` indexes `overlayRomTable`) and table 2 is 11599 records
that are overwhelmingly `LOCAL` (`symbolIndex` is a byte offset from the
module's own base). **That division of labour is inferred from the flag census,
not proven**; `runlinkDownloadCode` is what would prove it.

### 5.4 The canonical overlay work surface

That split is now complete. `config/overlays.us.json`, generated by
`tools/overlay_atlas.py`, is the canonical projection of the shipped tables.
It records every module's ROM and section ranges, BSS size, entry points,
exports, resident callers, imports, relocation census, graph edges, and a
transparent campaign priority. `gmake overlay-atlas` regenerates both the JSON
and the marked yaml block in memory and fails on drift; only
`gmake overlay-atlas-write` updates them.

The measured totals are:

| Surface | Total |
|---|---:|
| headers / non-empty modules | 107 / 106 (overlay 32 is empty) |
| text | 469,264 bytes |
| initialized `data_rodata` | 61,312 bytes |
| BSS | 77,680 bytes |
| module relocation records | 18,542 |
| cross-overlay relocations / directed edges | 608 / 97 |

The header has one initialized-data size, not separate `.data` and `.rodata`
sizes. The atlas therefore calls that range `data_rodata`; inventing a split
would turn a missing fact into false precision. Each non-empty module is a
Splat `code` segment with text emitted as assembly and the initialized and
relocation tails preserved as binary until their internal ownership is known.

All shipped `vramBase` values are zero, so the build uses `0xF0000000` as a
**synthetic link VMA**, never as a claim about a runtime load address. Every
module shares one `exclusive_ram_id`: Splat uses that ID to hide symbols from
other mutually exclusive segments. Giving every overlay a distinct ID leaks
one overlay's local labels into another. `subalign: 1` is equally deliberate;
the relocation tails are only 8-byte aligned, and the linker's default
16-byte input alignment inserts bytes that do not exist in the ROM.

The generated identity is `(overlay, section, byte offset)`, reflected in
labels such as `func_overlay_061_...`; a bare synthetic VMA is not unique
because all modules overlap there. This representation links all 106
non-empty modules and reproduces the full US ROM byte-for-byte while leaving
the progress denominator unchanged.

`config/overlay-donors.us.json` is the companion evidence ledger. It checks
every overlay against pinned DKR v77, DKR v80, and JFG object builds, recording
strong, ambiguous, empty, and negative results. The first reusable findings
are DKR's unique `alSeqFileNew` at overlay 5 offset zero; JFG's named
`refractOutput` in overlay 49; and JFG's whole-text `osRamTest4_6105` match for
overlay 107. Placeholder-only matches remain placeholders. Overlay 61 has a
semantic DKR crosswalk to ghost/Controller Pak code, but no exact-byte match,
so it is explicitly a workflow lead rather than adopted identification.

Four pilot shapes are retained in the atlas: overlay 107 (whole-module donor),
103 (text plus one relocation table), 76 (text, initialized data, BSS, and both
relocation tables), and 61 (DKR semantic crosswalk). Together they exercise
every structural case without pretending that generated assembly is C.

### 5.5 Epoch 3 reviewed maps and matched ownership

The first overlay matching tranche owns exactly 2,000 text bytes. Ownership is
stored as explicit half-open offset ranges in the atlas, rather than inferred
from synthetic ELF symbols:

| Overlay | Matched ranges | Bytes | Evidence / disposition |
|---:|---|---:|---|
| 5 | `0x000`–`0x2E4` | 740 | DKR exact `alSeqFileNew`; DKR `bnkf.c` source/flag crosswalk for the remaining audio-bank patcher |
| 6 | `0x000`–`0x01C` | 28 | complete dependency-free neighborhood; three function boundaries, four padding bytes excluded |
| 14 | `0xB40`–`0xB5C` | 28 | exact JFG placeholder body, retained under a neutral Mickey name |
| 49 | `0x354`–`0x374` | 32 | exact named JFG `refractOutput`; the three alignment nops are still assembly |
| 72 | `0x000`–`0x0B4` | 180 | independently matched initializer after negative exact scans |
| 76 | `0x000`–`0x114` | 276 | complete three-function structural pilot; 12 padding bytes excluded |
| 78 | `0x000`–`0x0A8` | 168 | complete two-function leaf; eight padding bytes excluded |
| 93 | `0x000`–`0x01C` | 28 | initializer only; update remains assembly |
| 102–105 | function text | 472 | four one-function leaves; eight combined padding bytes excluded; exact donor scans negative |
| 106 | `0x000`–`0x008` | 8 | no-relocation constant-return leaf; eight padding bytes excluded |
| 107 | `0x000`–`0x028` | 40 | exact named JFG `osRamTest4_6105` donor; eight padding bytes excluded |
| **total** | | **2,000** | matched C only; no generated-assembly or padding credit |

Overlay 5 is compiled at `-O3 -mips2 -32`. DKR established both the shared
audio-bank source family and that optimisation level, but Mickey retains calls
that DKR's whole `bnkf.c` translation unit inlines. The six measured Mickey
source boundaries preserve those calls. The other adopted overlay translation
units use `-O2 -mips2 -32`. Where a source boundary is not 16-byte aligned,
`tools/trim_elf_section.py` removes only IDO's zero object padding and refuses
to discard a nonzero byte.

The four required pilot reviews are:

- **Overlay 107:** one function at `+0x000`, export table index 2001, no header
  entry point, resident caller, relocation, initialized data, BSS, or import.
  Its `0x28` function text is `osRamTest4_6105`; the final eight bytes are
  alignment padding.
- **Overlay 103:** one function at `+0x000`, export index 1356 and resident call
  site ROM `0x27138`; no header entry point, initialized data, BSS, or overlay
  import. Its five primary-table relocations are one resident `R_MIPS_26` call
  and two `HI16`/`LO16` pairs for reserved symbol `0xFFD` addend `0x14A4`.
  Its `0x6C` function text is matched C; the final four bytes are padding.
- **Overlay 76:** functions at `+0x000`, `+0x038`, and `+0x0D0`, exported at
  table indices 1240, 1286, and 1332 and called from resident ROM `0xB9E4`,
  `0xBE4C`, and `0x1C5E0`. Two primary-table `R_MIPS_26` records are the
  resident sound and random calls; twelve local `HI16`/`LO16` records prove the
  16-byte initialized range and 32-byte status BSS. It has no overlay imports,
  and its `0x114` function text is matched C; the final 12 bytes are padding.
- **Overlay 61:** thirteen reviewed boundaries at `+0x000`, `+0x1C0`,
  `+0x1DC`, `+0x3C0`, `+0x7C4`, `+0x968`, `+0xB84`, `+0x1578`, `+0x1648`,
  `+0x17B8`, `+0x18A0`, `+0x19B0`, and `+0x1A6C`. Its three exports are
  `+0x968`, `+0xB84`, and `+0x1578` (indices 1894–1896); it has no resident
  inbound call. Its 549 relocations comprise 160 primary and 389 secondary
  records: 160 `SYMBOL`, 373 `LOCAL`, 16 `JUMP`, with 19 `R_MIPS_32`, 158
  `R_MIPS_26`, and 186 each `HI16`/`LO16`. Fifty-four imports target overlay
  45 and two target overlay 68. The `0x2A0` initialized range and `0x5E0` BSS
  remain raw. Exact C now owns the contiguous `+0x000..+0xB84` prefix,
  `+0x1578..+0x1648`, and `+0x17B8..+0x1A84`; the
  intervening ranges remain assembly, followed by twelve bytes of padding.
  DKR's `save_data.c`,
  `racer.c`, and `menu.c` are a semantic navigation crosswalk only: the ghost
  and Controller Pak strings do not by themselves prove function names.

The closed dependency neighborhood is overlay 6. Its header init is `+0x000`,
its additional exports are `+0x008` and `+0x010`, and resident call sites at
ROM `0x3344`/`0x3A48` reach the latter two. It has no relocation table, data,
BSS, or imports, so all directly required symbols are local and the complete
`0x20` text can be owned without pulling in another module.

### 5.6 Rejected and deferred overlay candidates

The tranche deliberately records near misses so a later pass does not repeat
them:

- JFG's placeholder-only hits in overlays 5, 14, and 16 prove byte reuse but
  do not supply names. Overlay 14's exact body was adopted under a neutral
  name; overlay 5's placeholder region after `+0x2E4` and all of overlay 16
  remain assembly. A source reconstruction of the overlay 16 hit did not
  reproduce the object.
- Overlay 93's initially rejected donor-shaped update was revisited from
  Mickey's instructions. `-Wab,-r4300_mul` supplies the required R4300 hazard
  schedule, and the complete `0x0F0` text is now matched C.
- Overlay 39's earlier prefix register-allocation mismatch was closed with an
  otherwise-unused volatile reservation; the complete module is now matched
  C except padding. Overlay 95's main body is likewise exact and closes that
  module. Overlay 85's first 192 bytes are exact with
  `-Wab,-r4300_mul`; its 476-byte tail remains deferred on register colouring.
- Negative DKR v77/v80 and JFG scans for overlays 72, 76, 78, 93, and 102–106
  are retained as useful evidence: their matching C was derived from Mickey's
  own instructions and relocations, not attributed to a donor.

### 5.7 Epoch 4 tranche-B ownership and hub maps

The completed tranche adds 4,100 matched overlay bytes, raising explicit
overlay C ownership from 2,000 to 6,100 bytes:

| Overlay | New matched ranges | Bytes | Disposition |
|---:|---|---:|---|
| 1 | `0x0E4`–`0x154`, `0x69A0`–`0x6A14`, `0x6B28`–`0x6B6C` | 296 | wrapped offset plus two state initializers |
| 5 | `0x6C0`–`0x764` | 164 | object/player constructor tail; padding remains assembly |
| 21 | `0x000`–`0x10C` | 268 | plane/geometry registration routine |
| 23 | `0x208`–`0x468` | 608 | initializer and update; prefix/tail remain assembly |
| 24 | `0x000`–`0x01C` | 28 | initializer |
| 25 | `0x588`–`0x608` | 128 | vector copy and state-flag selection |
| 27 | `0x000`–`0x064` | 100 | initializer |
| 39 | `0x0C8`–`0x168` | 160 | reset and readback exports; prefix remains assembly |
| 56 | `0x0B8`–`0x10C`, `0xAB4`–`0xAF4` | 148 | integer time split and packed-colour unpack |
| 67 | `0x000`–`0x14C` | 332 | module complete except padding |
| 69 | `0x000`–`0x04C` | 76 | initializer |
| 72 | `0x0B4`–`0x168` | 180 | update tail; module complete except padding |
| 74 | `0x000`–`0x0B8` | 184 | initializer; update remains assembly |
| 77 | `0x3B8`–`0x430` | 120 | selection and callback tail exports |
| 81 | `0x000`–`0x0CC`, `0x0CC`–`0x220`, `0x220`–`0x34C` | 844 | full module except four-byte padding; 556 bytes are beyond the checkpoint |
| 88 | `0x000`–`0x04C` | 76 | initializer |
| 92 | `0x000`–`0x068` | 104 | initializer |
| 93 | `0x01C`–`0x0EC` | 208 | update; module complete except padding |
| 95 | `0x000`–`0x00C` | 12 | empty callback boundary |
| 97 | `0x1A8`–`0x1E8` | 64 | radius-squared initializer |
| **tranche total** | | **4,100** | matched C only; no padding/generated-assembly credit |

The overlay 81 row shows its complete `0x34C` C surface for clarity; its
epoch contribution is 844 bytes, of which the original checkpoint already
reported 288. Overlays 67, 81, and 93 have no cross-overlay imports and retain
only alignment padding. Together with complete dependency-free overlays
102–107, the exit state contains nine dependency-free leaves. Overlay 72 is
also complete except padding but has one recorded import from overlay 8, so it
is not included in that count.

All new code uses measured `-O2 -mips2 -32`. Overlay 39 additionally needs
`-Wo,-loopunroll,0`; overlays 21, 23, 74, 81, 93, and 97 use
`-Wab,-r4300_mul` where their instruction schedules require it. No new
initialized-data or BSS slice was claimed: partial-module data remains raw,
and the absolute symbols used by C are relocation addends rather than invented
ownership boundaries.

Fresh pinned DKR v77/v80 and JFG object scans reproduce the donor ledger and
are negative for new exact attribution. DKR source was also searched first for
each routine's constants, structure effects, and control shape. The known JFG
overlay 5 placeholder at `+0x2E4` remains assembly: its shared instructions do
not reproduce Mickey's stack frame from plausible C.

#### Overlay 61's overlay 45/68 API surface

Relocation decoding accounts for all 56 cross-overlay records, with no
unresolved target:

| Target | Overlay 61 call sites | Count | Proved neutral contract |
|---|---|---:|---|
| overlay 45 `+0x00C` | `+0x994` through `+0xB14`, thirteen calls | 13 | accepts a data pointer plus numeric descriptor fields, allocates/initializes a linked resource descriptor and returns its handle (or zero) |
| overlay 45 `+0x270` | `+0x1590` through `+0x1620`, twelve-byte stride | 13 | finds a supplied descriptor in overlay 45's linked list, unlinks it through field `+0x30`, and releases storage at `+0x2C` |
| overlay 45 `+0x1BE0` | 28 sites from `+0xBB8` through `+0x1358` | 28 | null-guarded byte setter for descriptor field `+0x1D` |
| overlay 68 `+0x000` | `+0x1708`, `+0x1724` | 2 | returns constant `0x2EF0`; the paired calls clamp a `CHAR`-tagged payload-copy length to that bound |

Epoch 5's complete inbound census groups all 298 overlay 45 relocations and all
six overlay 68 relocations by target and caller. Counts are relocation records,
not inferred source calls:

| Overlay 45 target | Inbound total | Caller overlays (count) |
|---:|---:|---|
| `+0x000` | 1 | 10 (1) |
| `+0x00C` | 71 | 11 (35), 47 (5), 48 (1), 50 (1), 52 (1), 54 (1), 57 (12), 61 (13), 62 (1), 63 (1) |
| `+0x270` | 50 | 11 (10), 47 (5), 48 (2), 50 (2), 52 (2), 54 (2), 57 (12), 61 (13), 62 (1), 63 (1) |
| `+0x314` | 73 | 47 (8), 57 (65) |
| `+0x640` | 1 | 47 (1) |
| `+0x1BE0` | 84 | 11 (14), 48 (2), 50 (3), 52 (3), 54 (3), 57 (26), 61 (28), 62 (3), 63 (2) |
| `+0x1BF4` | 18 | 11 (14), 47 (4) |
| **total** | **298** | |

| Overlay 68 target | Inbound total | Caller overlays (count) |
|---:|---:|---|
| `+0x000` | 2 | 61 (2) |
| `+0x4E4` | 1 | 1 (1) |
| `+0x146C` | 3 | 57 (1), 58 (1), 60 (1) |
| **total** | **6** | |

The converted overlay 61 edges have the following ABI and field-level map.
No stack arguments occur on these four calls:

| Target | Arguments / return | Nullability and observed effects |
|---|---|---|
| overlay 45 `+0x00C` | `a0` text/data pointer, `a1` signed width, `a2` signed height, `a3` flags; nullable descriptor in `v0` | Allocation failure returns zero. Success initializes allocation/list links `+0x2C/+0x30`, flags `+0x08`, dimensions `+0x18/+0x1A`, count `+0x1C`, mode `+0x1D`, element/string pointers `+0x24/+0x28`, and per-element records; overlay 61 stores each return in handles `+0x58..+0x88`. |
| overlay 45 `+0x270` | nullable descriptor in `a0`; no used return | A null argument is a no-op. Otherwise it searches the global list, rewrites the head or predecessor `+0x30` link, and frees the target allocation at `+0x2C`. |
| overlay 45 `+0x1BE0` | nullable descriptor in `a0`, mode in `a1`; no used return | A null argument is a no-op; otherwise the low byte of `a1` is stored at descriptor `+0x1D`. |
| overlay 68 `+0x000` | no consumed argument; `0x2EF0` in `v0` | Pure constant accessor. Overlay 61 compares its pending copy length with the return and clamps before the resident copy path. |

Overlay 45 itself has ten exports, one resident caller, no imports, 298
cross-overlay inbound relocations, `0x80` initialized bytes, and `0x10` BSS.
Overlay 68 has eighteen exports, fifteen resident call sites, no imports, six
cross-overlay inbound relocations, `0x40` initialized bytes, and no BSS. Those
whole-module facts and the field effects above are enough to type overlay 61's
edges; neither hub's bulk text is claimed as C or given a stronger semantic
name.

#### Overlay 61 export `+0x968`

The `0x21C`-byte export is a module resource/configuration initializer. After
two resident calls, it invokes overlay 45 `+0x00C` thirteen times with local
data records at `+0xD0`, `+0xDC`, `+0xE8`, `+0xF4`, `+0x100`, `+0x10C`,
`+0x114`, `+0x120`, `+0x12C`, `+0x130`, `+0x134`, `+0x13C`, and `+0x144`.
The returned handles are retained in module globals `+0x58` through `+0x88`.
It then copies two resident configuration words (`+0x10` and `+0x14`) into
module globals `+0x08` and `+0x30`, initializes state words `+0xA4`–`+0xB0`
to `2, 0, 2, 7`, and finishes with a resident call using argument zero.

That narrative proves resource allocation/registration and state setup. It
does not prove DKR's ghost or Controller Pak identities, so those remain a
semantic navigation crosswalk and no borrowed name is adopted.

#### Overlay 61 export `+0xB84`

The `0x9F4`-byte export is a nine-state controller/menu state machine. It
begins by calling local `+0x000` to derive four directional/confirm/cancel-like
outputs, clears two emission markers, and uses overlay 45 `+0x1BE0` 28 times
to reset or enable thirteen descriptor handles. The initialized-data jump
table at `+0x278` maps state word `+0xA4` to cases 0 through 8 at `+0xCB0`,
`+0x00000D98`, `+0x00000DE8`, `+0x00001080`, `+0x00001168`, `+0x00001254`, `+0x00001334`, `+0x00001414`,
and `+0x1428`.

- State 0 selects one of handles `+0x6C/+0x70/+0x74` from selection word
  `+0xA8`, enables `+0x80/+0x84`, and on confirm/cancel copies the saved
  `+0xAC/+0xB0` state before emitting resident actions `0xC/0xD`.
- State 1 enables `+0x68`; either decision advances to state 2 with action
  `0xC`. State 2 performs the resident query at `+0x2D0D8`. Success installs
  `(state, selection, back, next) = (0, result, 2, 7)`. Failure resets the
  three counters through exact helper `+0x1C0`, scans sixteen records at
  `+0xC0`, probes them through local `+0x18A0`, and registers modes 4/6 via
  local `+0x1DC`. Optional object sizing through `+0x1A6C` is rounded to the
  next `0x100` before mode 5 is registered; a resident `+0x2D408` result is
  retained at global `+0x94` and registered as mode 7. The case ends in
  state 3.
- State 3 handles cancel to state 7/action `0xD`, confirm through the selected
  record's `+0xD0` field/action `0xC`, and wraps counter `+0x9C` against count
  `+0x98` on signed vertical input with action `0xF`.
- States 4, 5, and 6 enable descriptor sets `+0x5C/+0x78/+0x7C`,
  `+0x60/+0x78/+0x7C`, and `+0x64/+0x78/+0x7C`. Their confirm paths use local
  helpers `+0x1648`, `+0x19B0` then `+0x17B8`, and resident `+0x2D51C`;
  success installs tuples `(0,result,4,2)`, `(0,result,5,2)`, or
  `(0,result,6,2)`, while failure returns to state 1. Cancel returns to the
  appropriate prior state.
- State 7 calls resident `+0x288E0(0)` and advances to state 8. State 8 enters
  the common tail directly. The tail optionally calls local `+0x7C4`, invokes
  resident `+0x36354` twice for configuration, scales/truncates floats by
  `0x47800000` into config `+0x08` and global `+0x38`, and emits either marked
  parameter block through resident `+0x2F1C8`.

Its direct dependency list is complete: overlay 45 `+0x1BE0` (28 calls);
resident `+0xB44` (13), `+0x2D0D8` (1), `+0x2D408` (1), `+0x2D51C` (1),
`+0x288E0` (1), `+0x36354` (2), and `+0x2F1C8` (2); local `+0x000`,
`+0x1C0`, `+0x1DC` (5), `+0x7C4`, `+0x1648`, `+0x17B8`, `+0x18A0`,
`+0x19B0`, and `+0x1A6C`; plus five reserved-symbol HI/LO pairs to
overlay 4093 `+0x1494` and three to overlay 4095 `+0x4D700`. The trace proves
the state and descriptor effects above, not a ghost or Controller Pak name.

#### Rejected/deferred candidates

Near matches for overlay 24's update/render routines, overlay 60's quad draw,
overlay 83's mesh draw, and initializers in overlays 82, 84, and 91 were
removed from source and atlas ownership. They had plausible control flow or
exact footprint but unresolved register allocation/scheduling differences.
Overlay 1 `+0x154` likewise remains assembly after a two-register allocation
swap. These ranges receive no progress credit and are the explicit next
compiler-level boundaries rather than parked non-matching C.

---

### 5.8 Epoch 5 execution ledger

Epoch 5's exact-only pass currently contributes **6,376 overlay text bytes**.
This raises overlay C ownership from 6,100 to 12,476 bytes. The mandatory
1,344-byte semantic spine is complete, and overlay 61 `+0x1C0` contributes an
additional 28-byte helper proved by the `+0xB84` trace. No padding, data, BSS,
generated assembly, exact-size-only body, or semantic near match is included.

| Overlay | Newly matched ranges | Bytes | DKR-first result |
|---:|---|---:|---|
| 37 | `+0x000..+0x088`, `+0x4F4..+0x558` | 236 | negative |
| 39 | `+0x000..+0x0C8` | 200 | negative; module closed with its existing tail |
| 40 | `+0x2E4..+0x534` | 592 | timed interpolation is semantically related to DKR weather shifting; draw code has DKR menu-border/projection leads, but neither is an exact donor; `+0x0E8` remains assembly after a two-instruction startup scheduling mismatch |
| 42 | `+0x000..+0x0F4`, `+0x6A4..+0x700` | 336 | semantic display-list lifecycle / framebuffer swap; the unresolved renderer resembles `screenimage_draw` |
| 45 | `+0x00C..+0x314`, `+0x1BE0..+0x1BF4` | 796 | negative for allocator, release, and setter shapes |
| 56 | `+0x000..+0x0B8`, `+0x10C..+0x1A0` | 332 | generic setter/viewport idioms; unresolved minimap has a strong DKR HUD semantic lead |
| 61 | `+0x1C0..+0x1DC`, `+0x968..+0xB84` | 568 | negative; nearby DKR ghost/Controller Pak code is navigation only |
| 68 | `+0x000..+0x008` | 8 | negative |
| 75 | `+0x000..+0x214`, `+0x6D4..+0x6F8` | 568 | initializer exact-negative; leaf has only generic reference-array assignment similarity |
| 77 | `+0x000..+0x130` | 304 | DKR scenery/object radius clamp and divide is a strong semantic source lead |
| 82 | `+0x498..+0x4CC` | 52 | negative except generic setter reuse |
| 85 | `+0x000..+0x0C0` | 192 | negative |
| 95 | `+0x00C..+0x1D8` | 460 | DKR HUD audio has a related timed handle/volume ramp, not an exact donor; module closed |
| 96 | `+0x57C..+0x5C8` | 76 | negative |
| 97 | `+0x130..+0x1A8`, `+0x1E8..+0x3F4`, `+0x420..+0x508`, `+0x748..+0xA54` | 1,656 | negative for adopted bodies; DKR `obj_init_scenery` is a semantic lead for the unresolved scale initializer |
| **total** | | **6,376** | no newly adopted exact object donor |

The closure cohort supplied 2,864 bytes, the semantic spine and its extra
overlay 61 helper supplied 1,372, and the measured fallback ladder supplied
2,140: overlay 40 (592), overlay 56 (332), overlay 42 (336), overlay 37
(236), overlay 96 (76), and overlay 75 (568). These substitutions replace
blocked primary boundaries explicitly; they do not imply that the primary
modules closed.

Only overlays 39 and 95 reached full cohort closure. The other seven closure
targets were bounded and left as assembly where exactness failed: overlay 74
has one 400-byte allocator/scheduler mismatch; overlay 85 has a 476-byte
timer/trigger register-colouring mismatch; overlay 23 retains indivisible
520- and 256-byte bodies; overlay 77 retains one 648-byte projectile body;
overlay 24 retains 616- and 400-byte bodies; overlay 82 retains its 64-byte
initializer and 1,112-byte main routine; and overlay 97 retains 304-, 44-, and
576-byte bodies. The 44-byte overlay 97 state helper is the narrowest miss:
10 of 11 words match, but IDO saves `a1` rather than the target's `a2` in the
call delay slot, so it receives no credit.

Compiler settings remain narrow. All adopted game C uses `-O2 -mips2 -32`;
overlay 39's prefix additionally uses `-Wo,-loopunroll,0`, while overlay 77's
initializer, overlay 75's initializer, overlay 85's configuration routine,
and the affected overlay 97 math bodies use
`-Wab,-r4300_mul`. Every absolute symbol in partial modules is a measured raw
relocation addend. Nonexact experiment sources, Make rules, and placeholder
aliases are removed after each bounded pass.

The hard 8,192-byte and six-module closure exits are therefore **not yet
satisfied**: 1,816 more exact bytes and four more cohort closures are required.
The semantic-spine, second-overlay-61-function, `+0xB84` trace, inbound
histogram, and ABI-map exits are satisfied. This is an execution checkpoint,
not an Epoch 5 completion claim.

### 5.9 Epoch 6 exact-leaf recovery ledger

Epoch 6 adds **1,040 exact non-padding overlay text bytes**, raising overlay C
ownership from 12,476 to **13,516 bytes** and whole-program resolved code from
55,128 to **56,168 / 949,944 bytes (5.91%)**. Every range below was compiled
with the overlay default `-O2 -mips2 -32` except the measured overlay 97
direction initializer noted below, linked with its measured addends, and
compared in the complete US ROM. No generated alignment padding is credited.

| Overlay | Newly matched ranges | Bytes | DKR-first result |
|---:|---|---:|---|
| 8 | `+0x0000..+0x0008`, `+0x49A4..+0x49E8` | 76 | generic no-op, buffer, and scalar access patterns only; no donor |
| 9 | `+0x10A4..+0x10B4` | 16 | generic no-op only; no donor |
| 14 | `+0x31C..+0x328`, `+0xACC..+0xAF8` | 56 | generic flag access/reset patterns only; no donor |
| 15 | `+0x000..+0x00C`, `+0x6A4..+0x6B0`, `+0xB7C..+0xB94` | 48 | generic resource/scalar accessors only; no donor |
| 34 | `+0x0C8..+0x0D4` | 12 | generic scalar setter only; no donor |
| 41 | `+0x172C..+0x1740` | 20 | generic no-op only; no donor |
| 45 | `+0x1BF4..+0x1C1C` | 40 | generic null-guarded byte setters only; no donor |
| 61 | `+0x1A6C..+0x1A84` | 24 | negative for the record-size calculation |
| 66 | `+0x034..+0x040` | 12 | generic accessor only; no donor |
| 79 | `+0x1280..+0x1290`, `+0x147C..+0x149C` | 48 | generic object-state assignments only; no donor |
| 84 | `+0x00000DBC..+0x00000DD0`, `+0x00001004..+0x00001060`, `+0x00001294..+0x000012B4`, `+0x00001350..+0x000013BC` | 252 | generic state accessors/setters only; no donor |
| 86 | `+0x444..+0x474` | 48 | generic object initializer only; no donor |
| 94 | `+0x55C..+0x568` | 12 | generic scalar setter only; no donor |
| 97 | `+0x000..+0x130`, `+0x3F4..+0x420` | 348 | DKR `obj_init_scenery` supports only the direction routine's scale-prefix semantics; no exact donor or name evidence |
| 101 | `+0x1BB4..+0x1BD0` | 28 | generic three-global reset only; no donor |
| **total** | | **1,040** | pinned DKR v77/v80 ledger remains exact-donor negative for every adopted module |

Two old compiler blockers were closed rather than merely bypassed. Overlay 97
`+0x000` required preserving explicit unsigned-byte angle masks plus
`-Wab,-r4300_mul`; `+0x3F4` required the original three-argument ABI shape,
whose otherwise-unused third argument produces the target `a2` home store in
the call delay slot. The latter was Epoch 5's one-word-near function.

The remaining overlay 82 initializer experiment was removed. IDO consistently
folds the target's explicit `li`/`sll`/`addu` address calculation into one
`addiu`; volatile variants spill and are farther away. It stays assembly and
receives no credit. A stale generated object initially exposed why linked
validation is mandatory: its untrimmed alignment tail shifted the next
subsegment, while rebuilding with the committed exact-size trim restored the
full ROM. The final image is byte-identical at SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`.

The fresh 107-overlay scan covers both pinned local DKR revisions and JFG.
DKR v77/v80 each remain `1 strong / 1 ambiguous / 104 none / 1 empty`; none of
the Epoch 6 targets is the strong or ambiguous entry. Source-level similarities
above are deliberately classified as semantic or generic and do not promote
DKR names.

### 5.10 Epoch 7 leaf and wrapper ledger

Epoch 7 adds **508 exact non-padding bytes**, raising overlay C from 13,516 to
**14,024 bytes** and whole resolved code from 56,168 to **56,676 bytes**.

| Overlay | Newly matched ranges | Bytes | DKR-first result |
|---:|---|---:|---|
| 1 | `+0x00005BA4..+0x00005BC0`, `+0x00006788..+0x000067C0`, `+0x00007FCC..+0x00008008` | 144 | generic state initialization, byte copy, and predicates only |
| 14 | `+0xB34..+0xB40` | 12 | generic scalar getter only |
| 15 | `+0x00C..+0x04C` | 64 | generic resource release only |
| 20 | `+0xE0C..+0xE28` | 28 | generic nested-state mark only |
| 33 | `+0x17C..+0x19C`, `+0x708..+0x728` | 64 | generic zero-argument wrappers only |
| 36 | `+0x1470..+0x14B0` | 64 | generic mode wrappers only |
| 46 | `+0x112C..+0x1150` | 36 | generic submit wrapper only |
| 65 | `+0xBC0..+0xBF0` | 48 | generic double-release wrapper only |
| 101 | `+0xCEA8..+0xCED8` | 48 | byte-string loop; no donor |
| **total** | | **508** | no exact DKR donor or adopted DKR name |

Every row uses the overlay default `-O2 -mips2 -32`. The relocation-bearing
wrappers were verified in the complete linked image, not merely by comparing
unresolved objects. The adjacent overlay 101 `+0xCED8..+0xCEE0` and overlay 33
`+0x728..+0x730` zero tails are padding and receive no credit.

### 5.11 Epoch 8 accessor and resource-wrapper ledger

Epoch 8 adds **436 exact non-padding bytes**, raising overlay C from 14,024 to
**14,460 bytes** and whole resolved code from 56,676 to **57,112 bytes**.

| Overlay | Newly matched ranges | Bytes | DKR-first result |
|---:|---|---:|---|
| 15 | `+0x6B0..+0x6E8` | 56 | generic resource release only |
| 63 | `+0x74C..+0x77C` | 48 | generic double-release wrapper only |
| 84 | `+0x000..+0x048`, `+0xC74..+0xC9C`, `+0xFC4..+0x1004`, `+0x12B4..+0x1350` | 332 | generic object-state access and initialization only |
| **total** | | **436** | no exact DKR donor or adopted DKR name |

Overlay 84's two longer queries required sequential early returns to preserve
the target branch-likely duplication. A 72-byte overlay 65 reset reached the
correct loop under `-Wo,-loopunroll,0` but retained a different relocation and
register schedule; it was removed. No nonexact Epoch 8 source or alias remains.

### 5.12 Epoch 9 overlay 68 lifecycle ledger

Epoch 9 adds **524 exact non-padding bytes**, raising overlay C from 14,460 to
**14,984 bytes** and whole resolved code from 57,112 to
**57,636 / 949,944 bytes (6.07%)**.

| Overlay | Newly matched ranges | Bytes | DKR-first result |
|---:|---|---:|---|
| 68 | `+0x008..+0x148`, `+0x484..+0x51C`, `+0x1438..+0x146C` | 524 | lifecycle/allocation semantics are generic; no exact donor |
| **total** | | **524** | pinned DKR v77/v80 row remains `none` |

The first range contains two 108-byte allocators followed by two 52-byte
release wrappers. The middle range clears a nested flag, closes an active
entry while advancing its generation, and arms a timer. The last range is a
third release wrapper. All calls and zero-addend globals were checked after
linking, and the complete image retains SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`.

Epochs 7–9 contribute **1,468 bytes** together. The original Epoch 5 byte exit
is now satisfied at **8,884 / 8,192**, 692 bytes over target. Its separate
module-closure exit remains **2 / 6**, so four cohort closures still block the
campaign's final completion claim.

### 5.13 Epoch 10 execution checkpoint

Epoch 10's current checkpoint adds **16,708 exact non-padding bytes**, raising
overlay C from 14,984 to **31,692 bytes** and whole-program resolved code from
57,636 to **74,344 / 949,944 bytes (7.83%)**. This remains an active-campaign
checkpoint rather than the 10.00% exit.

| Newly matched ranges | Role | Bytes |
|---|---|---:|
| `+0x108..+0x164` | find the requested occurrence in a linked entry chain | 92 |
| `+0x164..+0x258`, `+0x344..+0x438`, `+0x948..+0xA3C`, `+0xB28..+0xC1C`, `+0x11E0..+0x12D4` | timed two-coordinate interpolators | 1,220 |
| `+0x258..+0x344`, `+0x438..+0x524` | allocate and schedule two-coordinate interpolation | 472 |
| `+0x524..+0x5E0` | activate/advance slot state | 188 |
| `+0x668..+0x708`, `+0x7D8..+0x878` | timed byte interpolators | 320 |
| `+0x708..+0x7D8`, `+0x878..+0x948` | allocate and schedule byte interpolation | 416 |
| `+0xD08..+0xD80` | timed scalar interpolation | 120 |
| `+0xE54..+0xEF4` | timed delta interpolation | 160 |
| `+0xFF4..+0x110C` | timed unsigned-byte interpolation | 280 |
| `+0x13C0..+0x1558` | four-channel color interpolation | 408 |
| `+0x1868..+0x1970` | timed global coordinate interpolation | 264 |
| `+0x2CE4..+0x2DC0` | construct four intensity/color words | 220 |
| **total** | **20 functions** | **4,160** |

The next scheduler/lifecycle wave adds another 4,864 bytes:

| Newly matched ranges | Role | Bytes |
|---|---|---:|
| overlay 101 `+0x5E0..+0x668` | promote a pending slot into the active state | 136 |
| overlay 101 `+0x00000A3C..+0x00000B28`, `+0x00000C1C..+0x00000D08`, `+0x00000D80..+0x00000E54`, `+0x00000EF4..+0x00000FF4`, `+0x0000110C..+0x000011E0`, `+0x000012D4..+0x000013C0`, `+0x00001558..+0x00001678` | allocate linked pair, scalar, byte, scaled, and color transitions | 1,676 |
| overlay 101 `+0x00001678..+0x00001728`, `+0x00001728..+0x00001868`, `+0x00001970..+0x00001A38` | update and schedule frame/global-pair transitions | 696 |
| overlay 101 `+0x000036E4..+0x00003814`, `+0x00003814..+0x00003998`, `+0x00003998..+0x00003A58` | draw and update presentation chains and slots | 884 |
| overlay 101 `+0xCBDC..+0xCD50`, `+0xCD50..+0xCEA8` | update presentation state and release completed work | 716 |
| overlay 68 `+0x21C..+0x2E0`, `+0x2E0..+0x484`, `+0x8E0..+0x96C` | attach an object, update its trail, and initialize object state | 756 |
| **subtotal** | **19 functions** | **4,864** |

The closure and volume lanes add 1,536 more bytes:

| Newly matched ranges | Role | Bytes |
|---|---|---:|
| overlay 85 `+0x0C0..+0x29C` | update the configured countdown/timer and emit the triggered effect | 476 |
| overlay 1 `+0x000..+0x050`, `+0x080..+0x0E4` | ring-pointer and entry-index helpers | 180 |
| overlay 1 `+0x000002D4..+0x00000378`, `+0x000010C0..+0x000010C8`, `+0x00001D58..+0x00001D78`, `+0x00003E48..+0x00003FD8`, `+0x00005ECC..+0x00005ED4`, `+0x00008008..+0x00008114` | linked-record filters, generic no-op, call wrappers, sampled/relative-angle and distance helpers, and zero-return leaf | 880 |
| **subtotal** | **18 functions** | **1,536** |

The next overlay 101 control-flow wave adds 560 bytes:

| Newly matched ranges | Role | Bytes |
|---|---|---:|
| `+0x000..+0x0B4` | initialize configuration, selection, and resource state | 180 |
| `+0x1A38..+0x1BB4` | dispatch 40 active entries across thirteen exact update routines | 380 |
| **subtotal** | **2 functions** | **560** |

Overlay 16 contributes a further 340 bytes:

| Newly matched ranges | Role | Bytes |
|---|---|---:|
| `+0x08C..+0x1A8` | allocate and populate four 64-step RGB gradient bands | 284 |
| `+0x1A8..+0x1E0` | release the gradient buffer and clear its owner | 56 |
| **subtotal** | **2 functions** | **340** |

The compact-function follow-up contributes 3,524 bytes:

| Newly matched ranges | Role | Bytes |
|---|---|---:|
| overlays 45 and 66 | state reset, pair accessor, and selection update | 124 |
| overlays 8, 14, and 79 | indexed selection, resource release, and timer update | 208 |
| overlays 2, 13, 83, and 99 | mode/color wrappers, two-stage submit, and entry cleanup | 236 |
| overlays 17, 55, 58, 62, and 87 | exact resource-lifecycle wrappers | 296 |
| overlays 50–55 | six instruction-identical index-to-pointer patch loops | 480 |
| overlays 54 and 55 | copy linked records with table-selected offsets and return the selected offset pair | 800 |
| overlay 91 | object/state initializer | 76 |
| overlays 11, 46, and 51 | handle cleanup and state lifecycle wrappers | 252 |
| overlay 11 grouped-release family | six exact fixed-span release/finalize loops | 600 |
| overlay 29 | select parallel table entries and rotate the four active values in both directions | 452 |
| overlays 3, 28, 35, 36, 48, 59, and 100 | queue/vector/state helpers, fixed-list lifecycle, buffer reset, and compact record selectors | 1,724 |
| **subtotal** | **56 functions** | **5,248** |
| **Epoch 10 total** | **117 functions** | **16,708** |

The repeated functions were not credited by resemblance. Each TU was compiled
at the overlay default `-O2 -mips2 -32`, trimmed to its proven boundary, and
compared word-for-word. The complete linked ROM exposed one integration error:
local calls in the stored image retain a zero J target for the runtime overlay
relocator, so assigning their eventual local offsets changed ten bytes. Giving
the C call aliases the correct stored zero target restored the exact US SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`.

The refreshed pinned scan remains `1 strong / 1 ambiguous / 104 none / 1
empty` for both DKR v77 and v80 and `5 / 5 / 96 / 1` for JFG. Overlay 101 is
`none` in all three ledgers; linked-list traversal, fixed-pool allocation, and
timed interpolation are semantic patterns only, so no donor name was adopted.
Overlay 85 is now fully owned by exact C except its final four-byte alignment
pad. That supplies **1 / 4** of Epoch 10's closure exit and moves the cumulative
cohort ledger to **3 / 6**. The hard Epoch 10 byte exit has **20,651 bytes
remaining**, and three more cohort modules are still required.

Later linked-exact waves raise the same active Epoch 10 checkpoint to **22,788
new non-padding bytes**, **37,772 / 469,264 overlay C bytes (8.05%)**, and
**80,424 / 949,944 whole-program resolved bytes (8.47%)**. The newest ranges
are overlay 13 `+0x124..+0x188` (100 bytes), which releases three optional
resources and clears the active word, and overlay 59 `+0x168..+0x1D4` (108
bytes), which resets the fixed entry table. Both were compared word-for-word
after relocation resolution. The hard Epoch 10 byte exit now has **14,571
bytes remaining**; the module-closure exit remains **1 / 4**.

A further lifecycle pass adds **496 exact non-padding bytes**: overlay 20's
release-handle and release-entry functions (116 bytes), overlay 44's four-slot
release loop (112), overlay 18's initializer (88), overlay 33's global release
wrapper (56), and overlay 86's current-entry processor (124). The linked
checkpoint reaches **24,284 new Epoch 10 bytes**, **39,268 / 469,264 overlay C
bytes (8.37%)**, and **81,920 / 949,944 whole-program resolved bytes (8.62%)**.

Overlay 27's `+0xB68..+0xBC0` activation transition adds **88 exact bytes**.
All 22 instruction words were independently compared before atlas adoption;
the linked ROM remains byte-identical. The live checkpoint is therefore
**24,372 new Epoch 10 bytes**, **39,356 / 469,264 overlay C bytes (8.39%)**,
and **82,008 / 949,944 whole-program resolved bytes (8.63%)**. The hard 10.00%
exit has **12,987 bytes remaining**.

Overlay 65's `+0x000..+0x080` fixed-pool initializer adds another **128 exact
bytes**. Its complete record-clear loop, both allocation calls, the local reset
call, and all linked data addends compare word-for-word; pinned DKR v77/v80 and
JFG scans remain donor-negative for the module. The live checkpoint is now
**24,500 new Epoch 10 bytes**, **39,484 / 469,264 overlay C bytes (8.41%)**,
and **82,136 / 949,944 whole-program resolved bytes (8.65%)**. The hard 10.00%
exit has **12,859 bytes remaining**.

Overlay 20's `+0x000..+0x07C` recursive three-child release adds **124 exact
bytes**. Its null path, branch-likely child traversal, recursive local calls,
field clears, and final release call all compile exactly on the first recovered
source shape. The checkpoint advances to **24,624 new Epoch 10 bytes**,
**39,608 / 469,264 overlay C bytes (8.44%)**, and **82,260 / 949,944
whole-program resolved bytes (8.66%)**. The hard 10.00% exit has **12,735
bytes remaining**.

The next compact cluster adds **1,012 exact bytes**. Overlay 13's
`+0x508..+0x580` active-record walker contributes 120 bytes. Overlay 84 adds
the adjacent `+0xA54..+0xAFC` input-gated resource update (168) and
`+0xAFC..+0xB7C` current-resource reset (128). Overlay 11 adds its
`+0xA18..+0xAF4` four-handle creator (220), `+0x1058..+0x1130` enable
transition (216), and `+0x1130..+0x11D0` disable transition (160). Every range
was linked at its real overlay offset and the complete ROM compared
byte-for-byte. DKR v77/v80 source searches found only generic fixed-pool,
resource, and handle-array idioms, with no exact donor or naming evidence.
The checkpoint is now **25,636 new Epoch 10 bytes**, **40,620 / 469,264
overlay C bytes (8.66%)**, and **83,272 / 949,944 whole-program resolved bytes
(8.77%)**. The hard 10.00% exit has **11,723 bytes remaining**.

The following compact pass adds another **504 exact bytes**. Overlay 11's
`+0x2BF4..+0x2CB4` six-way release dispatcher contributes 192 bytes; its IDO
jump table is redirected to Mickey's runtime-relocated table with the same
post-link ELF-word patching already established for overlay 101. Overlay 7's
`+0x298..+0x324` entry-appending helper adds 140 bytes, and overlay 19's
`+0x000..+0x0AC` selected-or-all item dispatcher adds 172. All three ranges
match after relocation resolution, and the rebuilt ROM retains SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. DKR v77/v80 and JFG scans are
donor-negative for the three clusters. The checkpoint is now **26,140 new
Epoch 10 bytes**, **41,124 / 469,264 overlay C bytes (8.76%)**, and **83,776 /
949,944 whole-program resolved bytes (8.82%)**. The hard 10.00% exit has
**11,219 bytes remaining**.

### 5.14 Epoch 10 final exit

Epoch 10 closes at **37,360 new exact non-padding overlay bytes**. Overlay C
ownership is **52,344 / 469,264 (11.15%)**, and total resolved text is
**94,996 / 949,944 (10.00%)**. This is one exact byte beyond the integer
94,995-byte hard floor.

The final 488-byte step is independently linked and compared at each real
overlay offset:

| Overlay range | Role | Bytes |
|---|---|---:|
| 1 `+0x1CA4..+0x1D58` | release the record array, secondary allocation, and final handle | 180 |
| 1 `+0x7B64..+0x7BDC` | select the best matching fixed record | 120 |
| 61 `+0x19B0..+0x1A6C` | choose the first unused controller-pak filename extension | 188 |
| **final step** | | **488** |

The controller-pak selector requires its measured per-object
`-Wab,-r4300_mul` schedule. The fixed-record selector reproduces the original
control flow, loads, stores, and branch schedule; a fail-loud twelve-word
normalization restores IDO's interchangeable `a1`/`a3` register coloring.
The release loop is natural exact C after relocation resolution.

Overlays 74, 77, 85, and 97 are now entirely exact C apart from proven
alignment padding. Together with the earlier overlay 39 and 95 closures, this
satisfies the Epoch 5 cohort exit at **6 / 6**. The regenerated donor ledger
covers 107 overlays against pinned DKR v77/v80 and JFG and remains negative
for the final three clusters. A bounded `gmake -j2` rebuild is byte-identical
to the US baserom with SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`.

### 5.15 Epoch 11 execution checkpoint

Epoch 11 opens with three exact bodies. Overlay 21's remaining plane-side
priority routine at `+0x10C..+0x2D4` contributes **456 bytes** and selects
lower or upper priority for a candidate list according to which side of a
registered plane each position occupies. The following `+0x2D4..+0x2E0` is
12 bytes of alignment padding and receives no C credit. Together with the
existing registration routine, every non-padding text word in overlay 21 is
now exact C, supplying **1 / 8** Epoch 11 closures.

Pinned DKR v77/v80 and JFG scans are exact-negative for the body. The measured
`-Wab,-r4300_mul` flag reproduces its FP schedule. Natural IDO output agrees at
104 of 114 words; ten fail-loud word assertions change only the caller-saved
register holding the overlay-local object count from `v1` to the shipped `a0`
across three reload/branch groups. The CFG, relocations, FP registers, memory
effects, and function length are otherwise identical.

Overlay 30's initializer at `+0x000..+0x2B4` contributes **692 bytes**. Its
natural source reproduces all 173 words and all 61 resident/local relocation
sites; the following 388-byte transposition tail remains assembly. Overlay
41's transition updater at `+0x1B00..+0x1C84` contributes **388 bytes**. A
natural five-record loop reproduces all 97 words and all nine address pairs
without calls, floating point, or code-word correction. Pinned DKR v77/v80
and JFG scans remain exact-negative for both bodies. Overlay 31's palette
builder at `+0x4F8..+0x6B0` adds **440 bytes**. Its bounded source-shape search
settles the compiler's 0x70-byte stack layout and reproduces all 110 words,
three calls, and three address pairs without post-compilation correction; the
same pinned donor scans are exact-negative.

Overlay 37's object updater at `+0x088..+0x19C` contributes **276 bytes**. Its
exact-size natural object agrees at 53 of 69 words; 16 fail-loud assertions
normalize one parameter-home/resource-register web and exchange the operands
of a finite sine-result multiplication. The CFG, calls, relocation sites,
effective addresses, memory effects, and function length are unchanged. The
pinned donor scans are exact-negative for this body as well.

Overlay 40's duplicate-removing table scan at `+0x084..+0x0E8` contributes
**100 bytes**. The source reproduces all 25 words and both relocated global
address pairs naturally. Its second formal is overwritten from the meaningful
identifier before use, an ABI-neutral source shape that selects the shipped
saved-identifier register web; the function remains a frame-free, call-free
leaf and continues scanning after a hit by design.

Overlay 40's add-entry scan at `+0x000..+0x084` contributes another **132
bytes**. Its exact-size object agrees at 29 of 33 words; four fail-loud changes
move the eight-iteration counter between equal `v1` and `a0` copies. The
success path returns before its only `v1` overwrite can reach a backedge, and
the CFG, four relocations, and all memory effects are unchanged.

Overlay 36's final-effect callback at `+0x1688..+0x1748` contributes **192
bytes**. Its natural object agrees at 46 of 48 words; two guarded frame
constants shrink an otherwise unused private stack tail by eight bytes. Every
request field, save slot, call, relocation, branch, and externally visible
effect is already exact. The following `+0x1748..+0x1750` is padding and
receives no C credit.

Overlay 46's release/synchronization leaf at `+0x614..+0x69C` contributes
**136 bytes**. Its exact-size object agrees at 26 of 34 words. Eight guarded
changes load and test two nullable resource pointers through `v0`, then copy
them to `a0` in the corresponding call delay slots; the natural object uses
the same pointers directly in `a0`. The null predicates, seven call sites,
five relocated data addends, call arguments, and all effects are unchanged.

Overlay 14's packed-rectangle builder at `+0x12D8..+0x13F4` contributes
**284 bytes**. Its exact-size object agrees naturally at 69 of 71 words. Two
guarded immediates move the private 24-entry rectangle array from IDO's
`sp+0x28` placement to the shipped `sp+0x24` placement; the stack frame,
bounded loop, all packed and expanded field accesses, multiplication, one
relocation, call arguments, and externally visible effects are unchanged.

Overlay 46's state initializer at `+0x000..+0x120` contributes **288 bytes**.
All 72 words are natural after the eleven fixed overlay-local addends and the
genuine `D_184` relocation resolve. The configure helper's third formal is
`f32`; passing `0.0f` selects the shipped `addiu` zero materialization. The
64-byte frame, eight calls, descriptor construction, nullable result update,
address pairs, branches, and effects are exact without code-word correction.

Overlay 91's render wrapper at `+0x4BC..+0x574` contributes **184 bytes**.
The first source basin reproduces all 46 words naturally, including six
resident relocation calls, temporary buffer and width outputs, the centered
render band, alpha initialization, and the `0x400` flag clear/restore around
the final object draw. The trailing `+0x574..+0x580` is padding and receives no
C credit.

Overlay 46's particle initializer at `+0x69C..+0x874` contributes **472
bytes**. It walks nineteen 60-byte records and their four-byte configurations
from index 18 through zero, assigning randomized angles and signed offsets,
fixed scale, copied target coordinates, table-selected resources, variants,
and the final state/timer values. After authoritative local addends resolve,
116 of 118 words are natural. Two guarded words reverse adjacent,
unconditional initialization of the independent `s4` resource-table base and
`s3` loop counter; neither is consumed until well inside the loop, so the loop
entry state, calls, relocations, FP schedule, and memory effects are unchanged.

Overlay 94's controller initializer at `+0x000..+0x110` contributes **272
bytes**. It copies kind/selector/scale into the object state, establishes the
controller, preserves one entity value across installation, attaches the
selected record, queries its three-vector output, and stores the resulting
angle. The natural object agrees at 63 of 68 words. Four guarded frame words
restore the shipped but otherwise unused eight-byte tail and its dependent
private spill location; one more changes an `or zero` to the equivalent
`addiu zero`. All six call sites, three output-local slots, field accesses,
branches, and externally visible effects are already exact.

Overlay 80's contact initializer at `+0x000..+0x11C` contributes **284
bytes**. It derives the object's scale from a signed initialization halfword,
the overlay-local `0.05f` scalar, and the object's scale pointer; scans a
resident-provided nearby-candidate array for the first threshold not exceeding
the object limit; resolves that record into two halfword outputs; and updates
the cached state and optional notice. The measured `-Wab,-r4300_mul` assembler
mode naturally inserts the shipped FP hazard no-op, grows the function from
280 to 284 bytes, and places both call relocations at their exact offsets.
Seventy of 71 words are then natural; one guarded word exchanges the two finite
sources of the commutative final multiply without changing its destination,
exceptions under the established input contract, or any later effect.

Overlay 80's contact updater at `+0x11C..+0x3EC` contributes **720 bytes**. It
queries a nine-pointer resident contact buffer, computes signed plane distance
and crossing state, derives Euclidean separation through the relocated square
root helper, classifies the scaled impact into four overlay-local bands, emits
a one-shot event, advances or clears the latch, and updates the optional notice.
The measured `-Wab,-r4300_mul` mode reproduces its exact 720-byte instruction
and relocation schedule. After eight fixed local data addends resolve, 152 of
180 words are natural; 28 fail-loud assertions translate only the immediate
fields of one complete, non-overlapping private-frame storage web from IDO's
compact 0x78-byte frame to the shipped 0x80-byte layout. Every opcode,
register, CFG edge, call and relocation position, FP operation, effective
storage pairing, and externally visible effect is already exact. Together the
two C bodies cover all **1,004 executable bytes** in overlay 80, supplying
**2 / 8** Epoch 11 closures; the final `+0x3EC..+0x3F0` remains four bytes of
explicit padding and receives no C credit.

Overlay 10's sole initializer at `+0x000..+0x2B0` contributes **688 bytes** and
closes the module with no padding. It initializes eight viewport records and
thirty-two descriptors; allocates the state table, resource table, data blocks,
and four lookup buffers; expands four 256-entry resources; clears three final
flags; and invokes the now-exact overlay 45 reset dependency. The frozen source
reproduces 135 of 172 words naturally, including all eleven call relocations
and fifteen resident-data address pairs. Thirty-seven guarded assertions cover
five complete webs only: the private frame/output homes, width/height register
permutation, two adjacent base/end completion swaps, the full call-free entry
loop register cycle, and two pairs of independent outer-loop initializations.
The normalization changes no CFG edge, call, relocation site, address, value,
iteration count, memory effect, or externally visible behavior, and supplies
**3 / 8** Epoch 11 closures.

Overlay 23's `+0x468..+0x568` render helper adds **256 exact C bytes** while the
final `+0x568..+0x570` remains eight bytes of explicit padding. It builds the
state-derived 36-byte render packet, emits pipeline-sync and environment-color
display commands, and invokes two distinct resident render helpers at their
exact shipped positions. The source is naturally exact at 31 of 64 words.
Thirty-three fail-loud assertions select three complete private webs: the
state-pointer register permutation, the call-free packet/display-list schedule,
and final-call argument registers. They change no frame, CFG edge, call,
relocation, ordered memory/FPU effect, display command, or submitted value.

Overlay 31's pool allocator at `+0xE7C..+0xF44` contributes **200 bytes**; the
following `+0xF44..+0xF50` remains 12 bytes of explicit padding. It allocates
and clears 192-byte records, creates the secondary local table with fifteen
slots per record, stores that table at local data `+0x10`, and returns the
primary allocation. The exact 200-byte source reproduces 34 of 50 words before
the local-call and data-pair relocations resolve naturally. Fourteen guarded
assertions cover the unused eight-byte private-frame tail and the complete
call-free record-pointer/outer-count register-and-initialization web. All
stores, iterations, addresses, values, calls, relocations, and return data are
unchanged.

Overlay 50's `+0x1C54..+0x1E68` formatter now contributes **532 exact C
bytes**, preserving the separate eight-byte padding tail. Overlay 19's
`+0xC1C..+0xD78` adjacency search contributes **348 exact C bytes** and one
exact local `R_MIPS_26` call to the `+0xD78` classifier. Its configured object
is 87/87 words; the standalone compiler's following alignment word is trimmed
because it belongs to the next function. Both splits survive atlas regeneration,
the donor scan, full link, and direct ROM comparison.

Overlay 96's `+0x5C8..+0x6D4` object renderer contributes **268 exact C
bytes**. All 67 configured instruction words and the call plus local-data
HI/LO relocation tuples are exact. The compiler alignment word is trimmed at
the `0x10C` symbol boundary, while the authoritative three-NOP tail remains a
separate 12-byte assembly padding owner.

Overlay 44's `+0x000..+0x224` animation-state constructor contributes **548
exact C bytes** and crosses Epoch 11's 8,000-byte Milestone A gate. All 137
configured instruction words are exact. Its nine runtime SYMBOL relocation
sites comprise two data HI/LO pairs and five calls, with the two uploader calls
sharing one identity. The assembler proxy's placeholder relocation model was
reconciled against the retained runtime census; the linked fields and complete
ROM then match. Three compiler alignment words are trimmed at the exact
function boundary before the existing `+0x224` C owner.

Overlay 8 now owns seven additional exact C islands. The motion starter at
`+0x0E88..+0x0F1C` contributes **148 bytes / 37 words** and has its one call
relocation exact. The activation path at `+0x0F1C..+0x1000` contributes **228
bytes / 57 words** with seven retail runtime-relocation sites reconciled. The
color applicator at `+0x3278..+0x3368` contributes **240 bytes / 60 words** and
has both call relocations exact. The authoritative atlas splits surrounding
assembly at every boundary; configured objects and the cumulative linked ROM
remain exact.

The `+0x2EC0..+0x3018` child updater adds **344 bytes / 86 words**. Its 28
FP-touching instructions, ordered branches, signed post-decrement loop, six
LOCAL data relocations, and terminal resident-call identity are reconciled and
exact. All three LOCAL pairs resolve to module-local `+0x73B0` while preserving
their distinct shipped low addends.

The channel updater beginning at `+0x3018` adds **608 bytes / 152 words** with
natural instruction, register, FP, CFG, and frame identity. Its five compiler
literals exactly reproduce the retained 32-byte pool at overlay-local
`+0x1BC`; a fail-loud externalizer asserts that payload, moves the paired MIPS
LO16 addends to the existing anchor, and emits no duplicate data. All 16 retail
relocation identities are preserved through the exact linked image.

The scale-output body beginning at `+0x3368` adds **312 bytes / 78 words**. Four
asserted words select the shipped allocation of two complete independent
threshold FPR webs; all GPRs, opcodes, branches, delay slots, frame words, and
address pairs are otherwise natural. The two threshold identities resolve to
overlay-local `+0x1D0/+0x1D4`. Direct slice comparisons and the cumulative ROM
are exact for both new islands.

The motion-output body at decimal overlay offsets `+18,920..+19,696` adds **776 bytes / 194 words**.
The measured `-Wab,-r4300_mul` object naturally reproduces its complete text,
including its R4300 scheduler no-op. The private `0.04f` literal is asserted
and externalized at decimal overlay-local offset `+652`, so the retained data payload remains
the sole byte owner. Three call sites remain distinct semantic proxies and the
retail runtime relocation stream remains intact. The direct ROM slice and
cumulative linked ROM are exact.

Overlay 24 is now fully C-owned through its 1,044 executable bytes. The updater
at `+0x001C..+0x0284` adds **616 bytes / 154 words**, and the renderer at
`+0x0284..+0x0414` adds **400 bytes / 100 words** beside the already matched
28-byte initializer. The updater's five calls, unloaded-overlay input field,
and local `0.3f` literal retain their exact retail words and relocations. A
narrow fail-loud ELF normalization externalizes the compiler's duplicate
literal section because the retained data payload already owns those bytes.
The three-word tail at `+0x0414..+0x0420` remains separate assembly padding,
so it contributes no executable credit. This completes the overlay-24 closure.

Overlay 43's child submitter at `+0x1264..+0x1378` adds **276 bytes / 69
words**. Its natural source is exact in size, frame, opcode/CFG sequence, and
both local call relocations, with one isolated four-word `s2`/`s1` owner-web
difference. A fail-loud postprocess asserts and normalizes that complete web;
its lifetime ends before the later `s1` child-offset reuse, leaving behavior,
memory effects, and relocation identity unchanged. The configured object,
linked slice, and cumulative ROM are exact; the separate module padding remains
assembly-owned.

Overlay 62 is now fully C-owned across its 1,456 executable bytes. The newly
matched decimal overlay range `+212..+1,388` adds **1,176 bytes / 294 words** between
the existing initializer and release routine. Its natural object reproduces
the full control-flow, stack, FP, call, and delay-slot schedule; four guarded
words select the retail assignment of two complete overlapping GPR webs. The
retained runtime relocation census, direct slice, cumulative link, and full
ROM are exact, awarding the fifth Epoch 11 closure.

Overlay 84's current-record activation body at decimal overlay offsets
`+4,192..+4,596` adds **404 bytes / 101 words**. The configured source retains
the exact root-object address pair, three distinct call identities, selector
branch-likely behavior, and FP/copy schedule. Nine asserted whole-word
normalizations select the retail private-frame and equal selected-value homes.
The direct ROM slice at file offset 26,023,232 has SHA256
`87ac5da55e8b23ea2a9f42a737c478716c2b762b1f792a3423dc6b255198ed24` in
both images, and the cumulative full ROM is exact.

Overlay 68's kind classifier at decimal overlay offsets `+5,228..+5,548`
adds **320 bytes / 80 words**. The configured object retains two distinct
local map-address pairs, five call identities, both branch-likely sites, and
the original two-loop control flow. Eighteen guarded words normalize only
complete private frame, stack-slot, and equal register-allocation webs. The
following four-byte alignment word remains assembly-owned. Direct ROM slices
from file offset `0x18C85CC` share SHA256
`8a7b237e640d76a01166f3d9b862b59a4ccf03fa576eb6be6b3be9306e90a8c7`,
and the cumulative full ROM is exact.

Overlay 1's type-five keyed search at decimal overlay offsets `+888..+1,044`
adds **156 bytes / 39 words**. The natural object has the exact frame, opcode
and delay-slot schedule, sole call relocation, and memory-access sequence.
Seventeen guarded words select one complete bijective temporary-register web.
The direct ROM slices at file offset `0x184C758` share SHA256
`daeb9395211c01871e6c40bafdf49a8187ac111a96855d1ed62d05ca5e80271d`,
and the cumulative full ROM is exact.

Overlay 1's entry lookup at decimal offsets `+80..+128` adds **48 bytes / 12
words**. Three guarded words select the duplicate retail return site after the
natural object reproduces the address pair, 0x1C stride, null branch, and first
return. Its direct ROM slices at file offset `0x184C430` share SHA256
`698f9adbf65e2128e034071353e438b9fa2c3a346ee401752148a970cc652875`.

Overlay 1's backward usable-record search at decimal offsets `+1,044..+1,204`
adds **160 bytes / 40 words**. Twelve guarded words select the retail preheader
and complete temporary-register webs while leaving its bounded reverse search,
wrap behavior, flag tests, and output stores unchanged. Its direct ROM slices
at file offset `0x184C7F4` share SHA256
`837c7fcf2d1ee011313f664b42f663b8bca7658473c801b697e24a81031461ff`.

Overlay 82 is now fully C-owned across its 1,228 executable bytes. The newly
matched updater at `+0x0040..+0x0498` adds **1,112 bytes / 278 words** with an
exact normalized natural object, 26 retained relocation sites, and separate
four-byte tail padding. Two call proxies and three input-data proxies preserve
the runtime relocation placeholders. Direct ROM slices from file offset
`0x18CF1C0` share SHA256
`38fbefda01c642125d3f1badef620054a90760e461f494741c370f660c65aad2`,
and the cumulative ROM is exact, awarding the sixth Epoch closure.

Overlay 45's layout configurator at decimal offsets `+788..+1,600` adds **812
bytes / 203 words**. Its natural object reproduces the complete 0x80-frame
code body, five calls, fourteen data-address relocations, twelve FP
instructions, stream loop, likely branches, and glyph update order with no
word correction. Three call proxies and five data proxies retain the shipped
runtime relocation identities. Direct ROM slices from file offset `0x188C76C`
share SHA256
`0eb13f9257a0d760179622fb1929659d758b435121ba7c3f3722dc9a015766b2`,
and the cumulative ROM is exact.

Overlay 23's `+0x000..+0x208` attachment spawner adds **520 naturally exact C
bytes / 130 words** with its `0xA0` frame and two call relocations exact. With
the existing C islands and separate eight-byte tail padding, all executable
text is now C-owned, awarding the seventh Epoch 11 closure.

Overlay 19's `+0xA30..+0xC1C` adjacency builder adds **492 exact C bytes / 123
words**. The natural object is topology-, frame-, and relocation-exact; 41
guarded register-only sites select the shipped allocation without changing an
opcode, memory effect, branch, delay slot, or call. Its configured object and
direct/cumulative ROM comparisons are exact.

Overlay 19's `+0xD78..+0xF58` edge classifier adds **480 exact C bytes / 120
words**. The natural object is size-, opcode-, CFG-, memory-, return-, and
likely-branch-exact at 106/120 words. Guarded normalization covers only four
independent two-load schedules and one complete six-use temporary web. Its
configured object and direct/cumulative ROM comparisons are exact.

Overlay 42's `+0x0F4..+0x6A4` captured-buffer renderer adds **1,456 naturally
exact C bytes / 364 words**. Its historical display-list macro spelling emits
the exact `0xD0` frame, seven calls, nine address pairs, every register and
schedule choice, and the complete command stream without word normalization.
With the four existing exact functions, overlay 42's full `+0x000..+0x700`
executable text is C-owned and exact, awarding the eighth Epoch 11 closure.

Overlay 19's `+0xF58..+0x12E4` spatial-mask builder adds **908 exact C bytes /
227 words**, with twelve following padding bytes retained separately in
assembly. Its minimized source naturally matches 164/227 words and the exact
frame, CFG, memory effects, and zero-relocation contract. SHA-anchored semantic
normalization selects two complete register webs and four independent
schedule pairs. The configured body, padding, and cumulative ROM are exact.

Overlay 58's rank-set refresh adds **608 exact C bytes /
152 words**. The natural object is 141/152 with exact topology, opcodes,
memory effects, and local/call relocation roles. SHA-anchored decoded-field
normalization selects two complete interchangeable private temporary webs.
The configured body, direct linked slice, and cumulative ROM are exact; this
is volume credit and does not close overlay 58.

The next volume tranche adds overlay 57's draw body (**832 bytes**), overlay
58's segment-strip renderer (**804 bytes**), overlay 101's clock renderer
(**952 bytes**), and two overlay 57 mode bodies (**1,132** and **868 bytes**).
Each configured object, owned linked slice, and cumulative ROM is exact; no
compiler-only alignment bytes are counted.

The byte-identical shared renderers in overlays 69 and 88 add **1,436 bytes
each**. Their objects and six call relocations were validated independently.
Overlay 69's following four-byte padding remains assembly-owned, while overlay
88 ends directly at its relocation payload. Both complete linked overlay spans
are exact.

Overlay 58's two adjacent point-quad renderers add **416 bytes each**. They
share one reviewed source and normalization shape but keep independent symbol
and payload bindings. Both configured objects, direct slices, and the full ROM
are exact.

Overlay 40's state updater adds **184 bytes / 46 words** at
`+0x0E8..+0x1A0`, creating a 416-byte contiguous exact-C prefix. Its complete
guarded permutation moves a dead incoming-argument precolor into rejected
alignment, fixes one independent address schedule, and preserves the exact four
runtime relocation records with separate entry-table and object-table symbol
identities. The configured object, direct ROM slice, atlas, and full ROM are
exact; the two later assembly regions remain unresolved.

Overlay 30's `+0x2B4..+0x438` byte-plane transposer adds **388 exact C bytes /
97 words**. Its guarded schedule and complete decoded field ledger preserve
the exact opcode census, counted loop, memory behavior, and zero-relocation
surface. The separate `+0x438..+0x440` padding remains assembly-owned. With
the existing initializer, all executable text is now exact C, awarding the
ninth Epoch 11 overlay closure.

Overlay 1's `+0x7B0..+0xBD4` point-record builder adds **1,060 naturally exact
C bytes / 265 words**. The configured four-way-unrolled R4300-scheduled object
matches SHA256
`f1c43d4bed886287e2cdade26036351d04985a00eb7ae3f0d2456f431aaa1d85`.
Its two calls retain distinct runtime relocation identities at `+0x1C` and
`+0x2C`; neighboring assembly resumes exactly at `+0xBD4`.

Overlay 84's `+0xC9C..+0xDBC` current-resource loader adds **288 exact C bytes
/ 72 words**. Natural source supplies the complete frame, CFG, opcode/call/FP
schedule, memory effects, and five relocation sites. A complete decoded
private allocator/spill ledger selects the retail GPR webs and unused spill
slot. The configured body and direct ROM slice share SHA256
`b7de8811d38658d16be513ab475b25409988a0f32c2e2220a34637bfc8e100f7`.

Overlay 59's `+0x070..+0x168` entry preparer adds **248 exact C bytes / 62
words**. Its natural object supplies the exact boundary, frame, CFG, memory
effects, four calls, and six relocation records. A complete nine-word decoded
ledger selects retail's equivalent descriptor-value/call-argument web. The
direct linked slice has SHA256
`9869b09a05313e3fe3ba522a2a596c6a635f6bab7dbbb5d672a2a0956bf9a41d`.

Overlay 48's `+0x144..+0x40C` state updater adds **712 exact C bytes / 178
words**. Natural source supplies the exact boundary, `0x38` frame, every
opcode and register lane, and all 22 runtime relocation records. A complete
schedule permutation moves one side-effect-free argument materialization to
the retail slot and updates five mechanically induced branch displacements.
The direct linked slice has SHA256
`dea26b8cb47cf81cae262079d27fff2beb1de86732434d53fbfa52529a7f6004`.
The independent `+0x060..+0x144` body remains assembly-owned, so this is volume
credit rather than a closure; `+0x46C..+0x470` remains explicit padding.

Overlay 101's transformed-object renderer adds **664 exact C bytes / 166
words**. Its natural object supplies the exact `0x90` frame,
ABI, seven-call layout, CFG, memory/stack effects, and FP topology. Two
complete command-schedule permutations and decoded private temporary webs
select retail's equivalent allocation. The configured body has SHA256
`e6114aff1a4f4a44186b450d9e131e32c22a44f327c329692e644bb998576515`,
with all five semantic call roles retained at the seven relocation sites.

Overlay 59's `+0x36C..+0x784` six-state advancer adds **1,048 exact C bytes /
262 words**. Natural source supplies the exact `0x58` frame, CFG, loops,
branch-likely forms, every integer/FP register web, and all calls and memory
effects. A complete prologue permutation plus asserted retained-data addends
selects the retail schedule before the duplicate compiler jump table and
relocations are discarded. The configured body has SHA256
`7068b6b1cf81620f6aeec1b9b7b0695705ede22aad9f166e80d70b6baa83939b`.
All executable overlay 59 text is now exact C; the separate
`+0xA1C..+0xA20` padding word remains assembly-owned, awarding closure ten.

Overlay 68's secondary-entry promoter adds **308 exact C bytes / 77 words**.
Natural source supplies the exact boundary, call order, copy loop, memory
effects, and nine runtime relocation sites. A complete fail-loud private frame,
call-survival, primary-pointer, and null-exit ledger selects retail's
equivalent allocation and branch-likely form. The configured body and linked
slice are byte-identical.

Overlay 68's following interpolation body adds **656 exact C bytes / 164
words**. Its natural object supplies the complete semantic body and three zero
alignment words. Fail-loud symbol metadata, schedule, and decoded private
register ledgers prove and select the full retail ownership unit. The
configured object, linked slice, and rebuilt ROM are byte-identical.

Overlay 68's secondary-resource rebuild adds **488 exact C bytes / 122
words**. Natural source supplies the exact frame, opcode schedule, map walk,
probe selection, nine calls, and all 19 relocation offsets. A complete
fail-loud private stack/register ledger selects retail's equivalent homes; the
configured relocation form, linked slice, and rebuilt ROM are exact.

Overlay 66's RGB5551 smoothing-and-draw body at `+0x040..+0x4E0` adds
**1,184 exact C bytes / 296 words**. Natural source supplies the exact frame,
boundary, opcode inventory, CFG, unrolled four-pixel filter, and all 28 runtime
relocation records. A complete fail-loud instruction-schedule and private
register ledger selects retail's equivalent allocation and three proved local
symbol addends. Its raw configured body and proxy-linked slice are exact; the
independent `+0x4E0..+0x810` helper remains assembly-owned.

Overlay 68's keyframe-animation updater at `+0x96C..+0xEFC` adds **1,424
exact C bytes / 356 words**. Natural source supplies the exact frame, boundary,
eleven calls, keyframe loop, interpolation/direction CFG, and runtime
relocation sites. A complete fail-loud decoded-field ledger selects retail's
equivalent private allocation. Five role-specific source calls fold to the
retail proxy, while an asserted filter removes only four duplicate compiler
records whose shipped runtime-table owner independently resolves both global
loads to the independently proved resident BSS flag. The configured object,
linked slice, and rebuilt ROM are exact; `+0xEFC..+0x1250` remains separately
assembly-owned.

Overlay 69's anchor updater at `+0x04C..+0x170` adds **292 exact C bytes / 73
words** and removes its final executable assembly gap. Natural source supplies
the exact `0x50` frame, branch-likely gate, store order, four call sites, and
all instruction words. The shipped overlay relocation table proves the four
distinct resident call identities; the configured object folds their
source-role names to the split target's established offset-zero proxy while
retaining the runtime table unchanged. The configured object, direct linked
slice, and rebuilt ROM are byte-identical. The independent
`+0x70C..+0x710` alignment word remains assembly-owned, awarding closure
eleven.

Overlay 18's state reconfiguration body at `+0x24C..+0x4F4` adds **680 exact
C bytes / 170 words**. Natural source supplies the exact `0x68` frame, state
transitions, equality-only rate loops, old-velocity integration order, five
semantic calls, and all 64 relocation records. A complete fail-loud ledger
removes one redundant compiler copy and selects three independent schedules
plus equivalent private stack and GPR homes. The configured object, direct
linked slice, adjacent `+0x4F4` boundary, and rebuilt ROM are byte-identical.
The independent `+0x000..+0x1F4` and `+0x4F4..+0x650` bodies remain
assembly-owned, so this is volume credit rather than a closure.

Overlay 98's unique-height collector at `+0x000..+0x144` adds **324 exact C
bytes / 81 words**. Natural source supplies the complete nested traversal,
flag gate, duplicate scan, signed/unsigned field behavior, fifteen-entry cap,
and exact instruction count. A complete fail-loud schedule and decoded-field
ledger selects retail's equivalent three-base allocation and independent
address materializations. A text-hash-gated metadata step adds only the proved
second list-address relocation pair after asserting its zero-addend
instructions and existing symbol. The configured object, all eight BSS
relocations, direct linked slice, and rebuilt ROM are byte-identical. The
successor begins exactly at `+0x144` and remains assembly-owned.

Overlay 89's effect updater at `+0x000..+0x138` adds **312 exact C bytes / 78
words**. Natural source supplies the exact `0x58` frame, opcode and branch
topology, six calls, FP operations, and eight runtime relocation sites. A
complete fail-loud ledger selects the retail private stack/register web and
four-instruction create-call schedule. The configured object folds the six
static call-role names to the target's offset-zero proxy and removes only the
two local-data ELF records whose runtime identity remains in the shipped
overlay table. The configured object, direct ROM slice, exact `+0x138`
successor boundary, and rebuilt ROM are byte-identical. The unrelated tail at
`+0x270` remains assembly-owned, so this is volume credit rather than a
closure.

Overlay 18's buffer initializer at `+0x4F4..+0x650` adds **348 exact C bytes /
87 words**. Natural source supplies the exact `0x20` frame, straight-line CFG,
opcode inventory, two allocation calls, memory effects, and all 50 runtime
relocation sites and types. Two complete relocation-aware schedule
permutations plus a fail-loud local-addend/private-spill/GPR ledger select the
retail layout. The configured object, direct linked slice, and rebuilt ROM are
byte-identical. Data begins immediately at `+0x650`; the compiler's following
alignment word is trimmed and not credited. Only `+0x000..+0x1F4` remains
assembly-owned, so this is volume credit rather than a closure.

Overlay 18's startup loader at `+0x000..+0x1F4` adds **500 exact C bytes /
125 words** and removes the module's final executable assembly gap. Natural
source supplies the exact `0x18` frame, straight-line control flow, 46 calls,
display-list command stream, and all 60 runtime relocation sites and types. A
complete fail-loud ledger selects one five-instruction result-publication
schedule, the retail-equivalent display-list register web, and two equal zero
materializations. The configured object, direct linked slice, exact `+0x1F4`
successor boundary, and rebuilt ROM are byte-identical. Together with the two
already exact successors, all **1,616 executable bytes** of overlay 18 are now
C-owned, awarding closure thirteen.

Overlay 88's anchor updater at `+0x04C..+0x1A4` adds **344 exact C bytes / 86
words** and removes its final executable assembly gap. Natural source supplies
the exact `0x58` frame, branch-likely delay-slot load, FP and store schedule,
and seven call sites. The shipped overlay relocation table proves the seven
distinct resident roles; the configured object folds their source-role names
to the split target's established offset-zero proxy without changing the
runtime table. The configured object, direct ROM slice, exact neighboring
`+0x04C` and `+0x1A4` boundaries, and rebuilt ROM are byte-identical. Overlay
88 has no executable padding, awarding closure twelve.

Overlay 68's sorted-entry renderer adds **852 exact C bytes / 213 words** and
removes its final executable assembly gap. Natural
source reproduces the exact boundary, CFG, three calls, gathered-entry and
descriptor effects, and stack-array offsets. A relocation-aware permutation
moves the prepare call to its retail site, a second reviewed permutation
selects the equivalent peeled adjacent-swap schedule, and a complete fail-loud
decoded-field ledger selects retail's private frame/register/spill allocation.
The configured object, all three runtime call relocation sites, direct ROM
slice, and neighboring function boundaries are byte-identical. The only
remaining assembly interval is the explicit four-byte tail padding,
awarding closure fourteen.

Overlay 89's state/particle updater adds **544 exact C bytes / 136 words**.
Natural source reproduces the exact instruction count, FP/integer inventories,
constants, structure accesses, and fourteen runtime relocation roles. A single
complete fail-loud bijection selects retail's argument spill, private frame,
saved-register allocation, and equivalent instruction schedule. The configured
object, twelve static call sites, linked slice, and successor padding boundary
are exact.

Overlay 89's initializer at `+0x270..+0x5A4` adds **820 exact C bytes / 205
words** and removes the module's final executable assembly gap. Natural source
supplies the exact `0x58` frame, sole saved register, CFG, three calls, local
scale relocation pair, descriptor construction, state initialization, and
nested color propagation loop. A complete fail-loud schedule and decoded-field
ledger selects retail's equivalent state-pointer spill/register web while
pinning all five relocation-bearing instructions. The configured object has
the exact three static local-call relocations after the scale pair is handed to
the shipped runtime table. Its raw body SHA256 is
`91a336e39261e09b3690e088760c6bdb0bf39854393b7d119442b9db64239a70`;
the direct linked slice SHA256 is
`22640530500357251ecb92d3d6e719294dfe59b969c8539fa7d5919d6465a024`.
Only the explicit twelve-byte tail padding remains assembly-owned, awarding
closure fifteen.

Overlay 48's state initializer at `+0x060..+0x144` adds **228 exact C bytes /
57 words** and removes that module's final executable assembly gap. Natural
source supplies the exact `0x18` frame, stores, two calls, and branch-free
integer topology. A complete fail-loud schedule/field ledger selects retail's
equivalent private register web and proved constant boolean. Sixteen
compiler-created local-address records are removed with exact offset, type,
and symbol assertions, leaving the exact six local-address and two call sites.
The raw configured body SHA256 is
`312385b63b2beaee48fb7cb069e6737ad4f7a622031d1e50220c157016092ce0`;
the direct linked slice SHA256 is
`927bc336aca97507f1ed258868c5d24a53d8db1eacb9cdcb2a07f53f37d380a5`.
Only the explicit four-byte tail padding remains assembly-owned, awarding
closure sixteen.

Overlay 16's gradient applicator at `+0x1E0..+0x424` adds **580 exact C bytes /
145 words** and removes the module's final executable assembly gap. The typed
natural source has the exact loops, color arithmetic, memory effects, and six
local relocation sites but emits two CFG-proven unreachable duplicate ternary
stores. Two guarded deletion steps remove only those dead stores; a complete
fail-loud schedule/field ledger then selects retail's equivalent frame,
saved-register allocation, and branch spelling. Configured-link validation
also corrected the module's local storage names: the accumulator is at `+8`
and the mode selector at `+4`, consistently in both its initializer and this
consumer. The raw configured body SHA256 is
`e51a18791518c07f21b505a4105a51c8d4ef354bba38f2444af0ec2562aa78e6`;
the retail-linked slice SHA256 is
`237587594c7077add06692d8498a6b5c71f67130f275dd4719b0d81f0a91a5e2`.
Only the explicit twelve-byte tail padding remains assembly-owned, awarding
closure seventeen.

Overlay 49's initializer and updater at `+0x000..+0x354` add **852 exact C
bytes / 213 words** and remove its final executable assembly. The 500-byte
initializer preserves thirty call/address relocation sites; its guarded
ledger selects complete private allocation and scheduling webs and removes one
proved compiler-alignment copy. The 352-byte updater preserves twenty-three
relocations and the shipped post-decrement input loop, including index zero;
three guarded field/schedule operations select that equivalent loop web. The
two retail body SHA256 values are
`75dbf550918557a9ade8802c46458d379d324e23d1eca9b477ed9c902e564a96`
and
`122d09b0ff67c971b223ce8cee57ea647fa1e8646077794a30be6095569f5bee`.
The exact `refractOutput` tail stays C and only twelve bytes of explicit
padding remain assembly-owned, awarding closure eighteen.

Overlay 37's renderer at `+0x19C..+0x4F4` adds **856 exact C bytes / 214
words** and removes that module's final executable assembly. Natural IDO owns
852 bytes followed by one proved zero alignment word; the integration extends
that word into the function before a complete fail-loud bijection selects the
retail frame, private GPR/FPR allocation, branch distances, and legal schedule.
All twelve call/local-data relocation sites remain exact. The configured raw
body SHA256 is
`812e309eb30b88554676a6b7a11b8cd823a7426d7666506654bef7db8b6119e5`;
the retail ROM slice SHA256 is
`4b26c018a45c6d4ccd8285ec8570fa279d399e0253b008485be69f74456ce975`.
Only eight bytes of explicit padding remain, awarding closure nineteen.

Overlay 31's configuration allocator at `+0xA84..+0xDC4` adds **832 exact C
bytes / 208 words**. Natural output is already exact in opcode inventory, CFG,
delay slots, `0x50` frame, and its sole overlay-local call relocation, agreeing
at 200 of 208 words. Eight fail-loud decoded-field assertions select four
complete private representation webs: commutative operand order, one spill
home, one comparison destination, and one producer/use/store temporary. The
retail body SHA256 is
`43fe93dec2619d929e2a047471d108014dc9916045bcbbcfab2ea9a323779782`.

This checkpoint contributes **48,908 / 47,496 (102.97%)** Epoch 11 bytes,
records **nineteen overlay closures against the eight-closure target**, raises
overlay C ownership to **101,252 / 469,264 (21.58%)**, and raises whole-program
resolved text to **144,292 / 950,332 (15.18%)**. The hard byte exit is exceeded
by **1,412 bytes**. The linked binary remains byte-identical to the US baserom at SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`.

### 5.16 Epoch 12 execution checkpoint

Epoch 12 opens with four consecutive presentation builders in overlay 101.
`+0x000099C4..+0x00009D04`, `+0x00009D04..+0x0000A044`, and
`+0x0000A044..+0x0000A384` each add
**832 exact C bytes / 208 words**; `+0xA384..+0xA6BC` adds **824 exact C
bytes / 206 words**. Each object preserves 24 complete HI/LO address pairs and
four calls, for 52 exact relocations. The first three natural objects contain
one asserted redundant `addiu` constant rematerialization at `+0x324`; the
shared fail-loud normalizer deletes only that instruction before selecting the
complete private register, FP, and schedule web. The fourth body is naturally
the exact `0x338` size and needs no deletion.

The direct retail slice SHA256 values are, in order,
`a6088c4fd52b6e11ac01d04fb1d7e3db24e158d34043a6116cf5778c2814475e`,
`370193c2805e36438e3f5c5f6f71294625652a36b9c3f4f6841b379b2d5e5237`,
`fe201f237e570e9a8557cb21a4bae8d84754354262bc198c31eeeb9c3d00cd9a`,
and `41f00063ea694612c8108688b870e6cea1b64f967146d17266a0026513b1c514`.
This family also exposed why a proxy link is not an acceptance gate: sibling B
matched a zero-valued proxy at
`49280062adf6f1357dcb9e5fae8f128f988f30814605484eaf3f22d71cfd751c`
but differed at 28 bytes from the real ROM link. The mandatory full-ROM
comparison caught the false positive; binding the actual root, pool, count,
asset, text, input, and local-call addends then produced the retail hash above.
Every subsequent GO packet must include the direct canonical ROM-slice proof.

Overlay 33's present-and-swap helper at `+0x0000066C..+0x00000708` adds **156 exact C
bytes / 39 words**. Declaring the double-buffer index volatile reproduces the
target's otherwise missing address materialization and exact `0x18` frame.
The natural object is then exact in size, opcode schedule, two calls, and all
18 relocation sites. A fail-loud decoded-field ledger selects only five
complete private temporary-register webs. Its direct retail slice SHA256 is
recorded in the retained workbench packet and compares byte-for-byte; the
larger renderer and initializer remain assembly-owned, so this is not a closure.

Overlay 58's final executable body at
`+0x00005554..+0x00005A14` adds **1,216 exact C bytes / 304 words**. The typed
source retains the complete packed-status state machine, all 24 calls, and all
48 source relocations. Its guarded private register/schedule ledger reproduces
the direct retail slice exactly; the following
`+0x00005A14..+0x00005A20` nop island remains assembly-owned padding.

Overlay 101's next presentation body at
`+0x0000A6BC..+0x0000AB4C` adds **1,168 exact C bytes / 292 words**. The natural
source preserves the three-word ABI, six-call semantic graph, and all 44
compile relocations. A fail-loud preparation recovers three retained address
rematerializations from one unused argument home and two compiler-alignment
words; the complete bijective ledger then selects the retail private
frame/register/schedule web. Both its direct baserom slice and full-ROM link are
byte-identical.

Overlay 57's mode-state dispatcher at
`+0x00003A4C..+0x00003FD4` adds **1,416 exact C bytes / 354 words**. Its natural
object preserves the exact `0x30` frame, complete CFG, 32 calls, 45 address
pairs, and every immediate and schedule choice. A fail-loud ledger changes
only one complete 87-site private GPR-allocation web, while the canonical link
retains all 122 source relocation records and directly matches the retail ROM
slice.

Overlay 33's `overlay33InitializeBuffers` adds **324 exact C bytes / 81 words**
and retains all 25 runtime relocation roles. Overlay 40's
`overlay40BuildFrame` eight-record builder adds another **324 bytes**;
its typed straight-line record construction normalizes to all 81 retail
instructions. `overlay40DrawTintRectangle` adds **348 bytes / 87 words** after
one guarded redundant pointer-copy deletion and a
complete relocation-aware schedule/register permutation. All three direct
retail slices and the cumulative ROM are exact.

Overlay 91's `overlay91UpdateTimeline` adds **1,136 bytes / 284 words** for
the complete eight-state elapsed-carrying timeline and flagged graph-record
walk. IDO emits every instruction word naturally with the measured R4300
multiply scheduler flag; the duplicate private switch table is removed while
the shipped overlay data and runtime relocation assets remain authoritative.
Overlay 40's `overlay40FadeRecords` then adds its final **404 executable
bytes / 101 words** through a guarded saved-register/countdown preparation and
complete bijective schedule ledger. Its three trailing nop words remain
explicit assembly padding, closing every executable overlay 40 range in C.

The live checkpoint contributes **9,812 / 45,775 (21.44%)** Epoch 12 bytes,
leaves **35,963 bytes** to the hard byte exit, raises overlay C ownership to
**111,064 / 469,264 (23.67%)**, and raises whole-program resolved text to
**154,104 / 950,332 (16.22%)**. Overlay 40 raises the campaign closure gate to
**1 / 8**. The linked binary remains byte-identical to the US baserom at SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`.

Overlay 96's `overlay96Register` and `overlay96Unregister` add **248 bytes / 62
words**. The first scans backwards for duplicates and appends within the
bounded 16-entry registry; the second scans backwards for a match and compacts
all following entries. Guarded copy/address preparations and decoded-field
ledgers recover the retained private schedules while preserving each body's
six runtime address relocations.

Its `overlay96FindVolume` adds **192 bytes / 48 words**. It walks the registry
backwards and accepts the first volume whose six plane equations are all
nonnegative. The natural object owns the exact ABI, frame, CFG, countdown
loops, and four address sites; one guarded bijection swaps two independent FP
producers into the shipped schedule.

Overlay 94's `overlay94UpdateController` adds **1,100 bytes / 275 words** and
closes the module. Natural source supplies the exact frame, CFG, twelve calls,
twelve private address pairs, FP behavior, and boundary. A complete fail-loud
120-site decoded-field ledger selects the shipped private allocation/schedule
web; the existing runtime relocation assets remain authoritative. Direct
retail slices and the cumulative 32 MiB ROM are exact. Epoch 12 reaches
**11,352 / 45,775 bytes (24.80%)**, leaving **34,423 bytes**; overlay C reaches
**112,604 / 469,264 (24.00%)**, whole-program resolved text reaches **155,644 /
950,332 (16.38%)**, and the closure gate reaches **2 / 8**.

Overlay 96's `overlay96BuildVolume` adds **964 bytes / 241 words**. It expands
an oriented-volume definition into eight transformed vertices and six
normalized plane equations, then registers the finished volume. Natural IDO
output retains the exact frame, three calls, loops, arithmetic, and complete
241-word boundary under the measured R4300 scheduler option; a guarded
52-field ledger selects only private stack-home and center-coordinate webs.
Together with the existing registry, query, bit-test, and draw routines this
closes every executable non-padding range in overlay 96.

Overlay 75's `overlay75UpdateMovingObject` adds **1,216 bytes / 304 words** and
closes that module. Its typed five-phase motion state machine naturally owns
the exact boundary, `0x58` frame, 16 calls, branch topology, and FP work. The
guarded schedule/home ledger preserves every call site and external effect;
its sole opcode site replaces a redundant unescaped-stack initialization with
the retail stable `active` reload at the same control-flow join. Direct slice
and complete-ROM comparisons are exact. Epoch 12 reaches **13,532 / 45,775
bytes (29.56%)**, overlay C reaches **114,784 / 469,264 (24.46%)**, resolved
text reaches **157,824 / 950,332 (16.61%)**, and the closure gate reaches **4 /
8**.

Overlay 25's `overlay25InitializeEffect` adds **380 bytes / 95 words** starting
at `+0x0000`, with its exclusive end boundary at `+0x017C`. It initializes
lifetime, owner-derived motion, palette
color, and an optional output vector with the exact frame, five calls, CFG,
and opcode/FP inventories. A proved IDO alignment nop becomes the retained
pipeline nop; the guarded ledger then selects only the complete private
schedule/register/stack web. Its direct slice and cumulative ROM are exact.
Epoch 12 reaches **13,912 / 45,775 bytes (30.40%)**, overlay C reaches **115,164
/ 469,264 (24.54%)**, and resolved text reaches **158,204 / 950,332 (16.65%)**.
Overlay 25 remains open from `+0x017C`, with its next function boundary at
`+0x0588`.

Overlay 3's `overlay3SelectTarget` adds **336 bytes / 84 words** starting at
`+0x0588`, with its exclusive end boundary at `+0x06D8`. It switches between
ordinary and weighted target search,
expires a repeatedly selected object, updates the group selection, and returns
the selected or anchor coordinates. Its source compiles naturally to the exact
frame, instruction stream, calls, and nine runtime relocation sites. The
direct slice and cumulative ROM are exact. Epoch 12 reaches **14,248 / 45,775
bytes (31.13%)**, overlay C reaches **115,500 / 469,264 (24.61%)**, and
resolved text reaches **158,540 / 950,332 (16.68%)**.

Overlay 25's `overlay25UpdateEffect` adds the final **1,036 bytes / 259 words**
starting at `+0x017C`, with its exclusive end boundary at `+0x0588`, completing
that module. It implements ballistic
movement, collision response, entity-hit accounting, fade timing, and output
vector scaling with the exact operation multiset, eleven calls, 25
relocations, and six retained literals. The guarded schedule/register/home
web and externalized identical pool preserve the original runtime relocation
assets; its direct slice and the complete ROM are exact.

Overlay 3's `overlay3TouchObject` adds **136 bytes / 34 words** starting at
`+0x06D8`, with its exclusive end boundary at `+0x0760`. Its two fixed
32-entry descending searches have the exact
natural CFG and operation inventory; a thirteen-site register-only ledger
selects the shipped caller-saved allocation. The direct slice is exact at
SHA-256 `8ab0631883aaacab6543e21bee6e963d113281cd23b0c11195a6c94ad9696ba1`.
Epoch 12 reaches **15,420 / 45,775 bytes (33.69%)**, overlay C reaches **116,672
/ 469,264 (24.86%)**, resolved text reaches **159,712 / 950,332 (16.81%)**,
and the closure gate reaches **5 / 8**.

Overlay 44's `overlay44UpdateFrameCache` adds **748 bytes / 187 words**
starting at offset `0x0294` and ending at the exclusive `0x0580` boundary. It
walks the frame cache and refreshes stale entries with the exact nested CFG,
calls, frame, and complete guarded allocation webs.

Overlay 83's `overlay83BuildBatch` adds **672 bytes / 168 words** starting at
offset `0x053C` and ending at the exclusive `0x07DC` boundary. It allocates a
batch, scales source records, transforms world coordinates, and conditionally
creates linked children while preserving the retained runtime relocation
authority.

Overlay 3's `overlay3FindClosestObject` adds **308 bytes / 77 words** starting
at offset `0x027C` and ending at the exclusive `0x03B0` boundary. Its typed
scan rejects ineligible and already-related objects, orders candidates by the
resident six-float distance helper, and returns the closest eligible object.
One complete four-use temporary allocation web is selected by field-only
guards; the operation stream, CFG, frame, and five relocation sites are
otherwise natural.

The three direct slices and cumulative ROM are exact. Epoch 12 reaches
**17,148 / 45,775 bytes (37.46%)**, overlay C reaches **118,400 / 469,264
(25.23%)**, resolved text reaches **161,440 / 950,332 (16.99%)**, and the
closure gate remains **5 / 8**.

Overlay 83's `overlay83Update` adds **628 bytes / 157 words** at offset
`0x02A0`, ending at the exclusive `0x0514` boundary. It ages an eight-entry
circular record queue, creates new trail records, integrates motion, transforms
world coordinates, and publishes them to an optional linked object with its
exact two runtime call sites.

Overlay 83's `overlay83DrawStrip` adds the final **308 bytes / 77 words** at
offset `0x0850`, ending at the exclusive `0x0984` boundary. It emits primitive
color, environment color, vertex-load, and polygon commands while retaining
the local vertex-template HI/LO identity. Together these bodies make every
overlay 83 executable interval exact C and close the module.

Overlay 3's `overlay3SelectScoredObject` adds **472 bytes / 118 words** at
offset `0x03B0`, ending at the exclusive `0x0588` boundary. Its cached-result
path and scored descending search preserve the exact five calls, FP topology,
and frame. The measured R4300 multiply-hazard flag supplies the target's one
spacing nop; the guarded carrier and stack-owner webs preserve all effects.

The direct slices and cumulative ROM remain exact. Epoch 12 reaches **18,556 /
45,775 bytes (40.54%)**, overlay C reaches **119,808 / 469,264 (25.53%)**,
resolved text reaches **162,848 / 950,332 (17.14%)**, and overlay 83 advances
the closure gate to **6 / 8**.

Overlay 3's `overlay3RunCachedModeAction` adds the final **452 bytes / 113
words** at offset `0x00B8`, ending at the exclusive `0x027C` boundary. It
performs cached-target angle and path gating, then dispatches four mode actions
through two local chance-table reads with the exact eleven calls and fifteen
runtime relocation operations. The relocation-aware carrier cycle and
complete owner webs preserve the natural semantic operation inventory. With
only the separately classified twelve-byte padding left as assembly, every
executable overlay 3 interval is exact C and the module closes.

Epoch 12 reaches **19,008 / 45,775 bytes (41.53%)**, overlay C reaches
**120,260 / 469,264 (25.63%)**, resolved text reaches **163,300 / 950,332
(17.18%)**, and the closure gate advances to **7 / 8**.

Overlay 4's `overlay4FindCategory2Object` adds **448 bytes / 112 words** at
offset `0x0734`, ending at the exclusive `0x08F4` boundary. It walks the
resident-provided object range and returns the first type-`0x30` payload in
category 2 with the requested byte identifier. The natural source has the
exact four-way unrolled control flow, frame, call placement, and full
positional operation inventory; its complete guarded register web restores
the retail allocation. The configured object, direct retail slice, and full
ROM are exact. Epoch 12 reaches **19,456 / 45,775 bytes (42.50%)**, overlay C
reaches **120,708 / 469,264 (25.72%)**, and resolved text reaches **163,748 /
950,332 (17.23%)**.

Overlay 7's `overlay7EntryPool` adds **552 bytes / 138 words** from
offset `0x0000` through the exclusive `0x0228` boundary. Its two functions
unlink released entries into a free list and acquire entries after duplicate,
owner/type, and priority checks. The guarded joint object preserves the local
call and address-pair identities while selecting the shipped return-tail
schedule. The acquire body has a distinct static symbol so the later matched
callers retain their runtime-relocated zero-address placeholder. Direct linked
text and the complete ROM are exact. The module still has executable assembly
at `0x0324..0x0EDC` and `0x0F08..0x0FC0`, so this is not a module closure and
the closure gate remains **7 / 8**. Epoch 12 reaches **20,008 / 45,775 bytes
(43.71%)**, overlay C reaches **121,260 / 469,264 (25.84%)**, and resolved text
reaches **164,300 / 950,332 (17.29%)**.

Overlay 4's `overlay4FindSearchPosition` adds **952 bytes / 238 words** at
offset `0x08F4`, ending at the exclusive `0x0CAC` padding boundary. Mode one
performs a four-way unrolled nearest type-`0x21` search in the X/Z plane; the
other mode calls the preceding category-2 identifier search. The measured
multiply-hazard flag produces all target FP spacing, and a complete six-use
stack-home web selects the retained ABI representation. The linked local
maximum-distance and preceding-function addends reproduce the direct retail
slice and full ROM exactly. Epoch 12 reaches **20,960 / 45,775 bytes (45.79%)**,
overlay C reaches **122,212 / 469,264 (26.04%)**, and resolved text reaches
**165,252 / 950,332 (17.39%)**.

Overlay 4's final three unresolved bodies add **1,552 executable bytes**:
`overlay4InitializeObjectMotion` at `0x0000..0x0138`,
`overlay4UpdateObjectMotion` at `0x0138..0x04D0`, and
`overlay4UpdateGroupSpacing` at `0x05D0..0x0710`. Their configured objects and
direct linked ranges are exact, closing every non-padding executable interval
in the module and advancing the Epoch 12 closure gate to **8 / 8**.

Overlay 99's `overlay99BuildHeightGrid` at `0x0638..0x0800` adds **456 bytes**,
and `overlay99RenderSortedEntries` at `0x0800..0x0BA4` adds **932 bytes**. The
grid body preserves its mixed ABI, three loops, and private runtime-relocated
state; the renderer preserves its ordered float sort, temporary transform
mutation, record layout, six-call carrier ledger, and exact `0x148` frame.

Overlay 5's `overlay5InitializeAudio` at `0x031C..0x06C0` adds **932 bytes**
and closes that module's executable text. Its 29 call sites and 21 data-address
pairs remain exact after relocation-aware guarded scheduling. Overlay 7's
natural `overlay7InitPool` at `0x0F08..0x0FB8` adds **176 bytes** and owns the
local BSS free-list layout through `+0x2A0`; the following eight zero bytes and
final BSS tail remain separately owned.

All new direct slices and the cumulative ROM match exactly at SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **25,008 /
45,775 bytes (54.63%)**, overlay C reaches **126,260 / 469,264 (26.91%)**,
resolved text reaches **169,300 / 950,332 (17.81%)**, and the closure gate is
**9 / 8**.

Overlay 7's `overlay7DispatchSelection` at `0x0CCC..0x0DBC` adds **240 bytes**.
The source retains its thirteen-call/data relocation topology, local
`overlay7CreateEntry` binding, selection sentinels, and exact branch-likely
effects. Overlay 99's `overlay99RenderSegments` at `0x0BA4..0x0DDC` adds
**568 bytes**. Its first-live-segment setup gate, command emission, signed
heading correction, object mutation/restoration, and adjacent five-argument
renderer call retain all seven shipped call relocations.

Both configured objects, direct linked slices, and the cumulative ROM are
exact at SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches
**25,816 / 45,775 bytes (56.40%)**, overlay C reaches **127,068 / 469,264
(27.08%)**, and resolved text reaches **170,108 / 950,332 (17.90%)**.

Overlay 7's adjacent `overlay7CommitSelection` at `0x0DBC..0x0EDC` adds
**288 bytes**. Its twelve-entry post-decrement mapping search, branch-likely
latch, and seventeen-entry relocation contract are exact; the call back into
the preceding dispatch body deliberately retains the shipped runtime proxy
rather than being statically resolved. The full ROM remains exact. Epoch 12
reaches **26,104 / 45,775 bytes (57.03%)**, overlay C reaches **127,356 /
469,264 (27.14%)**, and resolved text reaches **170,396 / 950,332 (17.93%)**.

Overlay 7's adjacent middle pair adds **1,080 bytes / 270 words**.
`overlay7DispatchModes` at `0x0894..0x0AA0` selects one of two ten-column mode
matrices, raises the second owner's timer and height, and dispatches seven
paired create/append cases. The compiler's seven-entry switch table is
independently identical to the existing initialized-data table at
`0x18F4..0x1910`; the configured object redirects its proved `+4` base addend
there and leaves the asset and runtime relocation records authoritative.
`overlay7UpdateOwnerMode` at `0x0AA0..0x0CCC` records staged checks, validates
three thresholds for the special third pass, and chooses the next owner mode
from the previous state and failure result. The two bodies retain 21 and 23
configured text relocations respectively, and their complete guarded private
register/stack webs do not change calls, branches, immediates, or memory
effects. Their direct ranges and the cumulative ROM match retail exactly.

Epoch 12 reaches **27,184 / 45,775 bytes (59.39%)**, overlay C reaches
**128,436 / 469,264 (27.37%)**, and resolved text reaches **171,476 / 950,332
(18.04%)**.

The following exact integration batch owns another **1,252 bytes**:
overlay 61 `+0x000..+0x1C0` (input), overlay 100 `+0x38C..+0x50C` (motion),
and overlay 61 `+0x7C4..+0x968` (list rendering). The first is natural exact.
The list renderer uses one guarded three-instruction schedule permutation with
relocations moved atomically; the motion updater's guarded normalization is
confined to complete private compiler webs, while its runtime relocation still
targets the distinct module `+0x278` release helper. Configured objects, direct
linked retail slices, and the full ROM all compare byte-for-byte, with full-ROM
SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **28,436 /
45,775 bytes (62.12%)**, overlay C reaches **129,688 / 469,264 (27.64%)**, and
resolved text reaches **172,728 / 950,332 (18.18%)**.

Overlay 61 `+0x1DC..+0x3C0` adds a further **484 exact bytes**. Its natural C
object has the target frame, CFG, size, calls, delay slots, and ten relocation
records; two guarded complete schedule permutations account for the five
independent instruction-order residuals without changing effects. The
configured object, direct linked slice, and full ROM are byte-identical at
SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **28,920 /
45,775 bytes (63.18%)**, overlay C reaches **130,172 / 469,264 (27.74%)**, and
resolved text reaches **173,212 / 950,332 (18.23%)**.

Overlay 61 `+0x3C0..+0x7C4` adds **1,028 natural-exact bytes**, closing the
last assembly gap in its `+0x000..+0xB84` prefix. The configured object
reproduces the frame, 128-byte formatting buffer, switch CFG, 49 relocations,
calls, and delay slots with no instruction normalization; only compiler
section alignment beyond the function boundary is trimmed. The direct linked
slice and full ROM remain byte-identical at SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **29,948 /
45,775 bytes (65.42%)**, overlay C reaches **131,200 / 469,264 (27.96%)**, and
resolved text reaches **174,240 / 950,332 (18.33%)**.

The next exact batch adds Overlay 61 `+0x1578..+0x1648` and Overlay 100
`+0x000..+0x214`, **740 executable bytes** in total. The Overlay 61 release
body is natural exact with sixteen calls and all 44 relocations. Overlay 100's
motion initializer preserves the target `0x78` frame, branch-likely loop, and
five runtime call identities; its guarded private normalization owns complete
stack-home, allocation, scheduling, and loop-induction webs. The configured
objects, direct linked slices, and complete ROM compare byte-for-byte at SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`.

Overlay 61 `+0x17B8..+0x18A0` adds **232 exact bytes** for the character-record
writer. Its source reproduces the rounded allocation, word-copy loop, calls,
delay slots, and six relocations. The sole guarded normalization removes an
unused `0x10` compiler stack gap through a complete prologue/epilogue and
incoming-argument-home web. Its configured object and direct retail slice are
exact, and the cumulative ROM retains SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 now reaches **30,920 /
45,775 bytes (67.55%)**, overlay C reaches **132,172 / 469,264 (28.17%)**, and
resolved text reaches **175,212 / 950,332 (18.44%)**.

Overlay 61 `+0x18A0..+0x19B0` adds **272 exact bytes**, joining the write,
read, extension-choice, and size helpers into contiguous exact C through
`+0x1A84`. The natural object already has the target 68-word boundary, CFG,
calls, nine relocations, branches, and delay slots. Its guarded complete stack
web changes only the frame and all associated local/argument slot immediates.
The configured object, direct retail slice, and cumulative ROM are exact at
SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **31,192 /
45,775 bytes (68.14%)**, overlay C reaches **132,444 / 469,264 (28.22%)**, and
resolved text reaches **175,484 / 950,332 (18.47%)**.

Overlay 20 `+0x07C4..+0x09DC` contributes **536 exact bytes** for the
tile-command builder. Its natural object has the target 134-word boundary,
control flow, three command construction sequences, nested row/chunk loops,
delay slots, and one runtime-call relocation. A complete four-site frame and
array-base web plus a complete three-site polygon-count register web account
for every remaining word difference; the guards permit no partial rewrite.
The runtime call uses a semantic proxy because overlay 20's raw relocation
carrier is shared by multiple runtime targets, while the linked call word and
relocation record remain exact. Diddy Kong Racing's published graphics macros
are cited only as a source-shape crosswalk. The configured object, direct ROM
slice, and cumulative ROM are byte-identical at SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **31,728 /
45,775 bytes (69.31%)**, overlay C reaches **132,980 / 469,264 (28.34%)**, and
resolved text reaches **176,020 / 950,332 (18.52%)**.

Overlay 20 `+0x1018..+0x10EC` adds **212 exact bytes** for entry removal and
pointer-array compaction. Its C reconstructs the full 53-word semantic body,
including all early exits, the active-count decrement, overlapping forward
copy, the pool-address walk, active-bit clear, and ten data relocations. IDO's
natural object uses a four-instruction search-backedge web where retail uses
three; the guarded `drop-branch` operation verifies the redundant unconditional
branch, repairs every crossing branch displacement and later relocation
offset, and shrinks the sole owned function by exactly one word. Complete
count/base/copy and mask-temporary register webs account for the remaining
private allocation differences. The direct linked slice and cumulative ROM
are byte-identical at SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **31,940 /
45,775 bytes (69.78%)**, overlay C reaches **133,192 / 469,264 (28.38%)**, and
resolved text reaches **176,232 / 950,332 (18.54%)**.

Overlay 20 `+0x0E28..+0x0F78` contributes **336 exact bytes** for the
32-slot entry allocator and field configurer. Its first typed C object has the
exact `0x10` frame, nullable reuse path, bitmap/pool search, table/count/bitmap
updates, floating-point arithmetic and conversions, ten semantic data
relocations, and complete memory topology. IDO emits 83 semantic words plus
one zero alignment word. A complete guarded permutation moves that existing
nop to retail's allocation delay slot, carries every relocated instruction
with it, and closes the allocation-prefix schedule/register web; the generic
metadata resizer verifies the final 84-word digest and sole-function ownership
before admitting the nop. The linked slice and cumulative ROM are exact at
SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **32,276 /
45,775 bytes (70.51%)**, overlay C reaches **133,528 / 469,264 (28.45%)**, and
resolved text reaches **176,568 / 950,332 (18.58%)**.

Overlay 29 `+0x14C8..+0x16CC` contributes **516 exact bytes / 129 words** for
the grouped-command renderer. Its typed nested traversal and three calls are
complete; a guarded private schedule/register ledger preserves the three
`R_MIPS_26` sites, after which the two named semantic callees are rebound to
the same retained runtime carrier already emitted by the first call. The
direct linked slice SHA256 is
`1c0437b30e748ac5ed69b7e03ec81f9f81030e0f45e979df35ebf43bfbfe3fa5`,
and the cumulative ROM remains exact. Epoch 12 reaches **32,792 / 45,775 bytes
(71.64%)**, overlay C reaches **134,044 / 469,264 (28.56%)**, and resolved
text reaches **177,084 / 950,332 (18.63%)**.

Overlay 2 `+0x02C4..+0x0400` contributes **316 exact bytes / 79 words** for
the boundary classifier between the already matched append and intersection
helpers. Its C preserves both axis modes and the equality-side copy semantics.
The complete guarded scheduler/register ledger removes only one dead trailing
section nop while retaining all six overlay-data relocations exactly. The
configured unlinked text SHA256 is
`2df25f34d8fb3c8f9c9120c0cc14fa170d63d896721be9184b3cc3986ffb8fb5`;
the real-offset linked slice SHA256 is
`350e93d46482948c90825512da9d331df66abf9d378b894312c55d64a62ffdcf`,
and the complete ROM remains byte-identical at SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **33,108 /
45,775 bytes (72.33%)**, overlay C reaches **134,360 / 469,264 (28.63%)**,
and resolved text reaches **177,400 / 950,332 (18.67%)**.

Overlay 17 `+0x0668..+0x08B4` contributes **588 exact bytes / 147 words** for
the double-buffer chain advance between its release and strip-render helpers.
The source naturally reproduces the frame, backward halfword copy, point/color
publication, fade loop, and sole call relocation; its 82-word suffix is exact.
A complete guarded 65-word prefix permutation selects the shipped pre-call
schedule/register/spill web, and a near-neighbor source artifact fails the
first changed assertion before mutation. Configured text and the real linked
retail slice share SHA256
`9fd1ba4bc365994bede02dbd38aafbca1d3af31bf6fbb73efec1c76ee92e5bf5`;
the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **33,696 /
45,775 bytes (73.61%)**, overlay C reaches **134,948 / 469,264 (28.76%)**,
and resolved text reaches **177,988 / 950,332 (18.73%)**.

Overlay 2 `+0x049C..+0x06E0` contributes **580 exact bytes / 145 words** for
the line clipper immediately following the boundary helpers. Its typed source
owns the complete 20-byte-record traversal, accept/split cases, six calls,
range publication, and all eight local data address records. The guarded
normalization selects the complete frame/home, fallback-classification, and
suffix-schedule web while retaining all 14 relocations exactly. Configured
text SHA256 is
`05945c5c8e0d7cf4e5162c0cc09a29a384971983ea728dd8dff640324ffd3721`;
the linked retail slice SHA256 is
`efad8df5374c9fcc6255c86bbaa763d8e2facccbb5423eefc7e716050577eb74`,
and the full ROM remains exact. Epoch 12 reaches **34,276 / 45,775 bytes
(74.88%)**, overlay C reaches **135,528 / 469,264 (28.88%)**, and resolved
text reaches **178,568 / 950,332 (18.79%)**.

Overlay 17 `+0x0318..+0x0628` contributes **784 exact bytes / 196 words** for
the chain constructor immediately before the three already exact chain
operations. The typed source owns the physical twelve-argument ABI, allocation
mode, optional material resource, double-buffer layout, scaled 16-entry
template copy, endpoint query, vertex conversion, RGB publication, and alpha
clear. Its natural boundary, CFG, calls, and memory/FP effects are exact; a
fail-loud decoded-field normalization selects the complete private frame,
schedule, and register web, then filters only the asserted runtime-table-owned
template HI/LO pair. The configured object retains exactly three `R_MIPS_26`
calls at `+0x48`, `+0x6C`, and `+0x204`. Configured text and the real linked
retail slice share SHA256
`46372736806a2433adde61c1a5ba86787af17f3014c58b1df88aab7c6838e118`,
and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **35,060 /
45,775 bytes (76.59%)**, overlay C reaches **136,312 / 469,264 (29.05%)**,
and resolved text reaches **179,352 / 950,332 (18.87%)**.

Overlay 17 `+0x0000..+0x0318` adds the module's final **792 exact bytes / 198
words**, closing all `0xA90` executable bytes as C. The endpoint builder
preserves the physical seven-argument ABI, transformed and untransformed
geometry paths, runtime square-root normalization, cached-position update,
null-chain clear loop, and six output stores. Its natural function boundary
and semantic FP graph are exact; a complete guarded frame/schedule/register
web moves the two late calls into their exact positions and binds all three
`R_MIPS_26` records at `+0x68`, `+0x108`, and `+0x1D4` to the offset-zero
runtime identity. Configured text and the real linked retail slice share
SHA256
`dd9d1eee3f8e2d2cef8ad3543c188f0ca2e6115dce5a01f6f023b2cf107509aa`,
and the full ROM remains exact. Epoch 12 reaches **35,852 / 45,775 bytes
(78.32%)**, overlay C reaches **137,104 / 469,264 (29.22%)**, and resolved
text reaches **180,144 / 950,332 (18.96%)**.

Overlay 2 `+0x06E0..+0x0C90` adds **1,456 exact bytes / 364 words** across
the boundary chooser and recursive region splitter. The two typed functions
establish the shared 20-byte line/range and 16-byte candidate layouts, preserve
the exhaustive scoring and clipping behavior, and reproduce all **73** owned
relocations at their exact sites and identities. Their direct linked SHA256
values are `fbf47133352ec46078f2d64933cf559112250a54420ebb98ae2572bfb1969dd4`
and `ce32035117e1091aca20ffdd4cfa2593004c1a4fd893bae2e085f280cfff9173`.
Overlay 20 `+0x0A68..+0x0DC4` adds **860 exact bytes / 215 words** with the
complete overlap scan, grid displacement/color update, FP clamp behavior, and
runtime relocation surface; its configured and direct linked SHA256 is
`6f03934f0a2bdd75e0fb6dd0817476de322ac6d1b4b056d851bbbf68e692ef0f`.
The cumulative ROM remains exact at SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **38,168 /
45,775 bytes (83.38%)**, overlay C reaches **139,420 / 469,264 (29.71%)**,
and resolved text reaches **182,460 / 950,332 (19.20%)**.

Overlay 20 `+0x00A8..+0x038C` contributes **740 exact bytes / 185 words**
across resource configuration and object-resource update. Independent audit
replaced the initial opcode-mutation ledgers with true schedule permutations
and bounded private FP/frame/home/register fields; the configured hashes remain
`a1f6c87daeef1c5ae09c92c11aa98cde8b5be8e597a5ce6f7e674f26fc0f0725`
and `a627ff31fd721ca0b53ad4f40af40eb92c7a1f3b57a73ed41658736b3deb840b`.
All nine call relocations, both direct retail slices, and the cumulative ROM
are exact. The neighboring `+0x038C..+0x07C4` body remains assembly-owned and
uncredited until a policy-compliant compiler basin is proved. Epoch 12 reaches
**38,908 / 45,775 bytes (85.00%)**, overlay C reaches **140,160 / 469,264
(29.87%)**, and resolved text reaches **183,200 / 950,332 (19.28%)**.

Overlay 1 `+0x6424..+0x64F8` contributes **212 naturally exact bytes / 53
words** for the selection-vector reader. Its typed body preserves the signed
selection index, direct object-position path, inclusive descriptor bound,
null-vector hazard, repeated vector-base loads, and fallback output triple.
No relocations or normalization are present. The configured object and true
linked retail slice share SHA256
`6751426a6d253f250c158beb650a41abc445cf644103434e92dce1f5e162dc70`;
the complete linked Overlay 1 range shares SHA256
`66a12f59a250f9fdac39214b48d0a6663e8e9d28beb5f3ee15790f355a66793d`
with retail, and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. A separate Overlay 20 tail
experiment is explicitly uncredited because its whole-function field ledger
failed the bounded-normalization policy. Epoch 12 reaches **39,120 / 45,775
bytes (85.46%)**, overlay C reaches **140,372 / 469,264 (29.91%)**, and
resolved text reaches **183,412 / 950,332 (19.30%)**.

Overlay 2 `+0x0000..+0x01BC` contributes **444 naturally exact bytes / 111
words** for the region validator. Direct use of `region->count` in both the
outer condition and countdown initialization naturally retains retail's two
loop-value moves; there is no instruction normalization. Configured text and
the direct retail slice share SHA256
`77e66299eec8cec6e4ba9b3c4f5c6f4915eebc1e409f3f8372a86b2f028819b7`.
Its eight runtime relocations agree with the shipped tables: one local HI/LO
pair, three angle calls, and three signed-angle-difference calls. The complete
linked Overlay 2 image shares SHA256
`e4e2f43b9e1b986d793fa2784dc7595cd2991b138e64495afbac2bffd673a3bb`
with retail, and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **39,564 /
45,775 bytes (86.43%)**, overlay C reaches **140,816 / 469,264 (30.01%)**,
and resolved text reaches **183,856 / 950,332 (19.35%)**.

Overlay 2 `+0x123C..+0x1364` contributes **296 naturally exact bytes / 74
words** for BSP point containment. The configured `-O2 -mips2 -32` object has
the retail 56-byte frame and requires no instruction normalization. Configured
text, the direct retail slice, and the linked owned range share SHA256
`9270e0b824f7d1e477465b75b54557e19925c12f2ba190eea22df1069e762891`.
Its three `R_MIPS_26` relocations at function offsets `+0xAC`, `+0xCC`, and
`+0xE0` agree exactly with runtime-table rows 26--28 and the two resident angle
helper identities. The complete linked Overlay 2 image remains exact at SHA256
`e4e2f43b9e1b986d793fa2784dc7595cd2991b138e64495afbac2bffd673a3bb`,
and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **39,860 /
45,775 bytes (87.08%)**, overlay C reaches **141,112 / 469,264 (30.07%)**,
and resolved text reaches **184,152 / 950,332 (19.38%)**.

Overlay 14 `+0x1B7C..+0x1C40` contributes **196 naturally exact bytes / 49
words** for active-handle finalization. The nested-call source naturally emits
the retail 48-byte frame, spill slot, call order, and complete instruction
schedule with no normalization. The linked owned range and retail share SHA256
`901381c8386f7123b8bba33436d27383449a2821df896c92abd06e608a970b55`.
Five distinct `R_MIPS_26` identities and three local HI/LO pairs agree exactly
with the runtime tables. The complete linked Overlay 14 image is exact at
SHA256 `7b1d6309d510089646b5c261816e9989ad036375d2baf55f5f972f1374d07edd`,
and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **40,056 /
45,775 bytes (87.51%)**, overlay C reaches **141,308 / 469,264 (30.11%)**,
and resolved text reaches **184,348 / 950,332 (19.40%)**.

Overlay 1 `+0x6B6C..+0x6CE8` contributes **380 exact bytes / 95 words** for
nearby-object search and activation. Natural source supplies the exact 72-byte
frame, scan and unsigned-threshold behavior, three calls, delay slots, opcode
stream, and complete register/FP allocation. A two-entry fail-loud ledger moves
one complete spill lifetime from `sp+0x28` to retail's equivalent `sp+0x2C`
home without changing behavior or schedule. The linked owned range and retail
share SHA256
`6db7aa6f757439c4795cf0cde43fb1f9943895cca1c9e1260cdbeea810ca2a1b`.
The three `R_MIPS_26` sites agree with the shipped runtime table's two resident
targets and Overlay 4 target. The complete linked Overlay 1 image is exact at
SHA256 `66a12f59a250f9fdac39214b48d0a6663e8e9d28beb5f3ee15790f355a66793d`,
and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **40,436 /
45,775 bytes (88.34%)**, overlay C reaches **141,688 / 469,264 (30.19%)**,
and resolved text reaches **184,728 / 950,332 (19.44%)**.

Overlay 1 `+0x72A4..+0x7344` contributes **160 exact bytes / 40 words** for
pool-record allocation. Natural source supplies the exact wraparound,
exhaustion, record-filtering and status-bit semantics, instruction count, and
schedule. A fail-loud decoded-field ledger selects retail's complete private
temporary suffix plus one commutative `or` order. The configured object retains
all ten runtime HI/LO records for the cursor, group, pool bounds, and exhausted
flag. Its linked range and retail share SHA256
`3f4fb29255fa8096b1f968249e7b5f0ac9353e8177b87ca22b54d7441fcc1e35`;
the complete linked Overlay 1 image remains exact at SHA256
`66a12f59a250f9fdac39214b48d0a6663e8e9d28beb5f3ee15790f355a66793d`,
and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **40,596 /
45,775 bytes (88.69%)**, overlay C reaches **141,848 / 469,264 (30.23%)**,
and resolved text reaches **184,888 / 950,332 (19.46%)**.

Overlay 87 `+0x000..+0x128` contributes **296 exact bytes / 74 words** for
object initialization. The target-proven `-Wab,-r4300_mul` flag supplies the
exact multiply hazards; natural source otherwise recovers the 32-byte frame,
complete opcode schedule, stores, and two resident calls. A nine-site
fail-loud ledger selects retail's equivalent complete private FP temporary
web. The configured object retains the runtime local HI/LO pair and preserves
the separately decoded `mathRnd` and `func_8005AD64` identities before folding
them to the pre-loader carrier. The linked range and retail share SHA256
`94a0f9e03cfdbc4b59cdc47c58e10e5ffafa2ff27a77903d361825635a79149b`;
the complete linked Overlay 87 image is exact at SHA256
`35af73d01bddebb793a4a4301a486d16ef5514a628c1d59d54ae7981e51adfaa`,
and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **40,892 /
45,775 bytes (89.33%)**, overlay C reaches **142,144 / 469,264 (30.29%)**,
and resolved text reaches **185,184 / 950,332 (19.49%)**.

Overlay 1 `+0x7130..+0x72A4` and `+0x73A0..+0x7580` contribute **852 exact
bytes / 213 words** for transient-object state and a 64-entry value cache.
Natural source recovers both complete CFGs, conversions, memory effects, and
boundaries. Fail-loud ledgers preserve the updater's one relocation-aware
schedule permutation and private temporary web, and select the cache manager's
equivalent two-register coloring. The configured objects retain all 25 updater
relocations and the cache's local HI/LO pair. Their linked ranges and retail
share SHA256 values
`c76f3f56e45fd0a291f29f48dce092cf512efc1cb732bbd3717fc29f38be592e` and
`ec9d5a416499a7177d95a298fd46b814355ae13f8704223093e17589830304ca`;
the complete linked Overlay 1 image remains exact at SHA256
`66a12f59a250f9fdac39214b48d0a6663e8e9d28beb5f3ee15790f355a66793d`,
and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **41,744 /
45,775 bytes (91.19%)**, overlay C reaches **142,996 / 469,264 (30.47%)**,
and resolved text reaches **186,036 / 950,332 (19.58%)**.

Overlay 34 `+0x000..+0x0C8` contributes **200 exact bytes / 50 words** for
storage allocation and zero-initialization. The runtime relocation table fixes
both calls as the two-argument resident allocator `func_8002B280`; with that
interface corrected, natural source supplies retail's full opcode schedule,
register allocation, frame, and two clear loops. A four-site fail-loud ledger
selects retail's equivalent call-surviving size home. The configured object
retains the exact two call records and three local HI/LO pairs. Its linked
range and retail share SHA256
`c36c8500dfdbc0a68753d221ca14b5b601a2af054c07ff221e709b735d285022`;
the complete linked Overlay 34 image is exact at SHA256
`31ba43afb9bfe16cc3d5ae93acacad2920e2a60dfec6dae7b4db6ffa0340f6ab`,
and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **41,944 /
45,775 bytes (91.63%)**, overlay C reaches **143,196 / 469,264 (30.52%)**,
and resolved text reaches **186,236 / 950,332 (19.60%)**.

Overlay 1 `+0x7580..+0x7730` contributes **432 exact bytes / 108 words** for
path-point append, segment-length accumulation, cache update, and anchor
distance. Natural source supplies the complete behavior and boundary; a
fail-loud ledger selects retail's equivalent private compiler web. The
configured object retains the exact two call records and three anchor HI/LO
pairs. Its linked range and retail share SHA256
`6f1bdf89d901e5aaaa6e0f560610f415690b08218cc623c6c97523215d68dba3`;
the complete linked Overlay 1 image remains exact at SHA256
`66a12f59a250f9fdac39214b48d0a6663e8e9d28beb5f3ee15790f355a66793d`,
and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **42,376 /
45,775 bytes (92.58%)**, overlay C reaches **143,628 / 469,264 (30.61%)**,
and resolved text reaches **186,668 / 950,332 (19.64%)**.

Overlay 1 `+0x7BDC..+0x7D6C` contributes **400 exact bytes / 100 words** for
record allocation, first-point initialization, squared-distance root, anchor
publication, and validation/fallback selection. With the target-proven R4300
multiply-hazard flag, natural source emits retail's exact frame, six-call CFG,
conversion sequence, delay slots, and private register web. The configured
object retains all six call records and both local anchor HI/LO pairs. Its
linked range and retail share SHA256
`9cc91ff1062ab212f4a07c88bff17d0d546ff072860801d148aff021d5254bd7`;
the complete linked Overlay 1 image remains exact at SHA256
`66a12f59a250f9fdac39214b48d0a6663e8e9d28beb5f3ee15790f355a66793d`,
and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **42,776 /
45,775 bytes (93.45%)**, overlay C reaches **144,028 / 469,264 (30.69%)**,
and resolved text reaches **187,068 / 950,332 (19.68%)**.

Overlay 1 `+0x78DC..+0x7B64` contributes **648 exact bytes / 162 words** for
bounded path tracing, endpoint append, and branch-record cloning. Natural
source emits the complete opcode schedule, 88-byte frame, stack layout,
eight-call CFG, FP conversions, delay slots, and all observable effects. Its
fail-loud ledger selects only an equivalent complete private integer temporary
web. The configured object retains all eight call records and seven local
HI/LO pairs. Its linked range and retail share SHA256
`2d45569efa8c98d2fd85575993fe2dd0904e21bc24f66a81ecaa9bebf5973251`;
the complete linked Overlay 1 image remains exact at SHA256
`66a12f59a250f9fdac39214b48d0a6663e8e9d28beb5f3ee15790f355a66793d`,
and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **43,424 /
45,775 bytes (94.86%)**, overlay C reaches **144,676 / 469,264 (30.83%)**,
and resolved text reaches **187,716 / 950,332 (19.75%)**.

Overlay 34 `+0x2C8..+0x378` contributes **176 exact bytes / 44 words** for
active-record removal, list compaction, resident resource release, and count
update. Natural source owns the exact boundary, frame, CFG, call ABI, and
effects. A bijective fail-loud schedule and complete private count/pointer web
select retail's equivalent compiler representation; exact guards remove only
the two literal pointer-address relocation records absent from retail. The
configured object retains the loader call and both active-count HI/LO pairs.
Its linked range and retail share SHA256
`b27d66e83577bef19ecfb3b19288475828f6fcc0785048ea407dfc9746fc1bfb`;
the complete linked Overlay 34 image remains exact at SHA256
`31ba43afb9bfe16cc3d5ae93acacad2920e2a60dfec6dae7b4db6ffa0340f6ab`,
and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **43,600 /
45,775 bytes (95.25%)**, overlay C reaches **144,852 / 469,264 (30.87%)**,
and resolved text reaches **187,892 / 950,332 (19.77%)**.

Overlay 34's record constructor at `+0x0D4..+0x2C8` adds **500 exact
executable bytes / 125 words**. Natural source supplies the exact functional
boundary, CFG, field initialization, allocation/load path, direction setup,
and two-call ABI. A complete bijective fail-loud schedule restores retail's
equivalent private compiler representation while preserving all 12 relocation
sites: five local-data HI/LO pairs and the two independently decoded resident
calls. The linked owned range and retail share SHA256
`4323ae1bfbdac7e585c53bac0260ca73784f4cd01a65f579fe14eeceeb1b8cc5`,
and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **44,100 /
45,775 bytes (96.34%)**, overlay C reaches **145,352 / 469,264 (30.97%)**,
and resolved text reaches **188,392 / 950,332 (19.82%)**.

Overlay 34 `+0x378..+0x540` contributes **456 exact bytes / 114 words** for
storage reset and active-record updates. Reset is naturally exact and retains
15 shipped runtime relocations. Update's natural source owns the full behavior,
FP lanes, loop topology, call ABI, and seven relocation roles; a fail-loud
preparation removes exactly three asserted IDO-only invariant-home words and a
complete guarded ledger selects retail's equivalent schedule, frame, and
private register web. The configured zero-addend update SHA256 is
`1ef3cff290da4b1b623f4aa86de717e4cafaf4dcd3df71b8eb3be07d43c4005e`;
the combined linked range and retail share SHA256
`a04dc2ca90461deb111cd790aaa7c9a3ca2c1ca8301df765fc68f7933bd34386`,
all 22 runtime records remain exact, and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **44,556 /
45,775 bytes (97.34%)**, overlay C reaches **145,808 / 469,264 (31.07%)**,
and resolved text reaches **188,848 / 950,332 (19.87%)**.

Overlay 1 `+0x5CD4..+0x5ECC` contributes **504 exact bytes / 126 words** for
directional object selection. Target-local R4300 multiply hazards recover the
natural exact frame, opcode stream, CFG, FP schedule, and all 11 relocation
sites. A complete guarded ledger selects only the equivalent count stack home
and one two-instruction private temporary web. The configured zero-addend
SHA256 is
`d9cb02c73e426f6b2a4c36a71a7ab3c4d38b5d74812dbe63e39b77e187489e66`;
the linked range and retail share SHA256
`0e67b8a417c54916cf9d5deb51836032586d1b13dd694e6b7eb21a5f28980fe2`,
and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **45,060 /
45,775 bytes (98.44%)**, overlay C reaches **146,312 / 469,264 (31.18%)**,
and resolved text reaches **189,352 / 950,332 (19.92%)**.

Overlay 1 `+0x7D6C..+0x7FCC` contributes **608 exact bytes / 152 words** for
path-point resolution. Natural source recovers the exact 120-byte frame,
collision-output stack home, CFG, call topology, delay-slot effects, and full
boundary. A fail-loud target-local ledger applies two independent scheduling
permutations and selects four asserted private register fields without
changing an opcode, immediate, branch target, complete instruction word, or
externally visible behavior. The configured zero-addend SHA256 is
`33306da7b622053e30f3405bab944f30333b1efd4dd763f2fb118a75127ad3bb`;
all 22 shipped runtime relocation records remain exact, the linked range and
retail share SHA256
`4ec5bd5bdf61dfce75beaf785c243b265959db077cd367e55787922cc924ed1d`,
and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 reaches **45,668 /
45,775 bytes (99.77%)**, overlay C reaches **146,920 / 469,264 (31.31%)**,
and resolved text reaches **189,960 / 950,332 (19.99%)**.

Overlay 1 `+0x5BF4..+0x5CD4` contributes **224 exact bytes / 56 words** for
timer and mode callback dispatch. Natural source recovers the exact boundary,
frame, direct setup call, two indirect callbacks, loop semantics, and all 13
runtime relocation records. A complete fail-loud ledger reschedules only the
shared loop tail and selects the shipped equivalent private register and
branch-lowering representation. The configured zero-addend SHA256 is
`f3c91e2fc503b18a3c613732fe20e10e9ef6fa33ebedfe31c2851df98c651c3a`;
the linked range and retail share SHA256
`ce2a00b8edefbfbe433a6219edcbb8169d56d19f2367cf3e6d959465def3509f`,
and the full ROM retains canonical SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Epoch 12 closes at **45,892 /
45,775 bytes (100.26%)**, Overlay C reaches **147,144 / 469,264 (31.36%)**,
resolved text reaches **190,184 / 950,332 (20.01%)**, and the closure gate
remains **9 / 8**.

Overlay 34 `+0x608..+0x900` contributes its final **760 exact bytes / 190
words**. Typed source owns the depth census, parallel bubble sort, color
interpolation, and ten-argument render dispatch. A complete guarded
schedule/register/frame ledger produces configured zero-addend SHA256
`d477cc8d3a6fb0208dea06a124ec8041fba9e6864ad4b04129cf57127ab5f652`.
Canonical integration rejected a zero-local proxy false positive and retained
the shipped semantic local addends for all five HI16/LO16 pairs while using
pre-loader carriers only for the six calls. The corrected direct linked range
and retail share SHA256
`466c7b30566a2a656904fa9193af248d78adbf0c1efa0460d46066795656b8a8`;
all 16 runtime records, the complete Overlay 34 image at SHA256
`31ba43afb9bfe16cc3d5ae93acacad2920e2a60dfec6dae7b4db6ffa0340f6ab`,
and the full ROM are exact. Every executable interval is now C, closing the
module and raising the closure result to **10 / 8**. The stretch checkpoint is
**46,652 / 45,775 campaign bytes (101.92%)**, **147,904 / 469,264 Overlay C
(31.52%)**, and **190,944 / 950,332 resolved text (20.09%)**.

Overlay 1 `+0x7730..+0x78DC` contributes **428 exact bytes / 107 words** for
path-point bending. Natural source recovers the exact 48-byte frame, explicit
byte-truncation home, six-call CFG, FP allocation, and full owned boundary. A
fail-loud ledger performs one independent five-instruction prologue
permutation, one commutative operand swap, and a complete private register/home
selection without changing an opcode, branch displacement, jump target, or
complete word. The configured and direct linked retail ranges share SHA256
`e0ccf37ee706bd9abf7931693b2821ddaac0fc83a3410000e8b9dbd4072d1704`;
all six runtime calls and the full ROM are exact. The following alignment word
remains separately owned. The stretch checkpoint becomes **47,080 / 45,775
campaign bytes (102.85%)**, **148,332 / 469,264 Overlay C (31.61%)**, and
**191,372 / 950,332 resolved text (20.14%)**.

Overlay 1 `+0x6A14..+0x6B28` contributes **276 exact bytes / 69 words** for
nearby-pending consumption. A measured `-g3` TU naturally recovers the exact
72-byte frame, reverse branch-likely traversal, FP expression, field effects,
and sole call. The target-local fail-loud ledger contains two true scheduling
permutations and one complete private register representation web without
changing an opcode, immediate, branch displacement, jump target, or whole
word. The configured and direct linked retail ranges share SHA256
`dce8a10514f20586e8504e0a2c8a29e8ec29d4699b10eb9f86e4f6022d2d0ac0`;
the one runtime relocation and the full ROM are exact. The following 12 bytes
remain separately owned alignment. The stretch checkpoint becomes **47,356 /
45,775 campaign bytes (103.45%)**, **148,608 / 469,264 Overlay C (31.67%)**,
and **191,648 / 950,332 resolved text (20.17%)**.

Overlay 1 `+0x5ED4..+0x61F0` contributes **796 exact bytes / 199 words** for
the eight-way mode dispatcher. Typed source recovers the exact 40-byte frame,
CFG, calls, FP behavior, state effects, and private switch table. A fail-loud
ledger chooses complete private allocator webs and asserts the shipped
pre-loader addends. The duplicate compiler table/relocations are removed while
Mickey's retained initialized table and **61 runtime text records** remain
authoritative. Configured and direct linked text share SHA256
`e8faa26be0625819767e67a7c916ee69be0d3e1b637a0f67da46c2da973afb8d`;
the residual owner beginning at `+0x61F0` and the full ROM are exact. The
stretch checkpoint becomes **48,152 / 45,775 campaign bytes (105.19%)**,
**149,404 / 469,264 Overlay C (31.84%)**, and **192,444 / 950,332 resolved
text (20.25%)**.

Overlay 1 `+0x6270..+0x63CC` contributes **348 exact bytes / 87 words** for
mode-object selection. Typed source recovers the backward walk, five-entry
candidate array, signed state lookup, 600.0f threshold, random choice, and
world-state updates. The fail-loud ledger changes only three complete private
stack-home webs and resolves the proved pre-loader `D_1D9C` addend. The
compiler's duplicate `D_1D9C` pair is removed while Mickey's retained pair and
all **13 runtime text records** stay authoritative. Configured text has SHA256
`a850d7a4391045f9fbf46c38ea7d4d9fcf89543ba703f95e568c337b1019cb1f`;
direct linked retail text has SHA256
`42328484600c2bcb7223456faf7e4f0883f9dfb3855c7e89d36ae23ace1a2a51`.
Both adjacent owners and the full ROM are exact. The stretch checkpoint becomes
**48,500 / 45,775 campaign bytes (105.95%)**, **149,752 / 469,264 Overlay C
(31.91%)**, and **192,792 / 950,332 resolved text (20.29%)**.

Overlay 1 `+0x64F8..+0x6724` contributes **556 exact bytes / 139 words** for
the angle-candidate solver. Typed source recovers the 0xA8 frame, complete FP
web, discriminant and two-root walk, signed-angle selection, sentinel, and four
calls under measured R4300 scheduling. A fail-loud permutation moves the
compiler-emitted two-instruction loop-control carrier into the proved
preheader and selects the retail shared-tail ordering; the carrier and proved
zero tail are then removed. Configured, linked, and direct retail text share
SHA256 `34cb7aea1efe5006a94ce32ba12d1d1e7aeefeb15c4e7cedcfade43a083f9a5e`.
All **four runtime call records**, the following matched owner, and the full ROM
are exact. The stretch checkpoint becomes **49,056 / 45,775 campaign bytes
(107.17%)**, **150,308 / 469,264 Overlay C (32.03%)**, and **193,348 /
950,332 resolved text (20.35%)**.

Overlay 1 `+0x6D4C..+0x7130` contributes **996 exact bytes / 249 words** for
the aimed-transient updater. Typed source recovers object creation, four-pass
source prediction, distance/flight-factor recomputation, angle solving,
resident trig velocity construction, the default path, flag effect, and
timer/mode maintenance. A fail-loud permutation selects equivalent compiler
schedules and two private stack homes; the filter removes only one asserted
duplicate compiler `D_1DA0` pair. Configured zero-addend text has SHA256
`67f2432b565ef811c7ea7f65006672eb77e7436f338d8580128be842abee8b2e`;
direct linked retail text has SHA256
`67e4bdf8b704752ed9060f1c95ae75bcecba59cddb8db3de9b3a4b4f0c5aec84`.
All **43 runtime text records**, both matched neighbors, and the full ROM are
exact. The stretch checkpoint becomes **50,052 / 45,775 campaign bytes
(109.34%)**, **151,304 / 469,264 Overlay C (32.24%)**, and **194,344 /
950,332 resolved text (20.45%)**.

Overlay 1 `+0x67C0..+0x69A0` contributes **480 exact bytes / 120 words** for
range-flag updates. Typed source recovers the reverse object-list walk, signed
angle classification, horizontal and vertical range tests, status-bit switch
effects, and four calls under measured R4300 scheduling. A fail-loud ledger
changes only six complete private compiler representation webs, leaving
opcodes, CFG, calls, relocs, and instruction order intact. Configured
zero-addend text has SHA256
`20f4c618c398f99e547483c22187cf1589128af4e447dde6f152f4248ea99302`;
direct linked retail text has SHA256
`2c7f8bf2e4727b60780e4a0ef22d87928783d40672ee21aeb3c3e38c68c8c98a`.
All **four runtime call records**, the following matched owner, and the full ROM
are exact. The stretch checkpoint becomes **50,532 / 45,775 campaign bytes
(110.39%)**, **151,784 / 469,264 Overlay C (32.35%)**, and **194,824 /
950,332 resolved text (20.50%)**.

Overlay 1 `+0x04B4..+0x0758` contributes **676 exact bytes / 169 words** for
the activation and closest-sample pair. Both typed functions recover exact
size, frame, ABI, CFG, scheduling, delay slots, and calls; the second is
naturally instruction-field exact. A fail-loud ledger covers only three
complete allocator webs and loader-local addends in the first. Duplicate
compiler ELF carriers are removed while all **44 runtime text records** remain
authoritative. Configured raw text has SHA256
`dc87390d75490d0fae0b8a3237ac4abf767154cd218060a2e16fddc2bed6906e`;
direct linked retail text has SHA256
`212356ece7cbf207b79d183db33bb5664d64020845ad2b8f2a618aa3a32c5bdc`.
The following matched owner and full ROM are exact. The stretch checkpoint
becomes **51,208 / 45,775 campaign bytes (111.87%)**, **152,460 / 469,264
Overlay C (32.49%)**, and **195,500 / 950,332 resolved text (20.57%)**.

Overlay 1 `+0x01AC..+0x02D4` contributes **296 exact bytes / 74 words** for
the type-47 angle scan. Typed source recovers the bounded object walk,
unsigned phase conversion, signed wrap call, strict-positive minimum, frame,
FP diamond, and branch-likely loop. A fail-loud relocation-aware permutation
selects an equivalent independent schedule and private register/home webs,
moving the scale relocation with its instruction; two alignment words are
trimmed. Configured zero-addend text has SHA256
`2c0f17cd3245e50f18c359971324f98ec59e52a6c2af814409cdf4cc6f135f80`;
direct linked retail text has SHA256
`12cd29c76828be5bfdedf7a17ab8ef1ffcfb57a4c1135ea9d129a245810cc969`.
All **six runtime text records**, both matched neighbors, and the full ROM are
exact. The stretch checkpoint becomes **51,504 / 45,775 campaign bytes
(112.52%)**, **152,756 / 469,264 Overlay C (32.55%)**, and **195,796 /
950,332 resolved text (20.60%)**.

Overlay 1 `+0x0BD4..+0x10C0` contributes **1,260 exact bytes / 315 words**
across four functions for motion scaling, path interpolation, motion-point
resolution, and sampled curve length. Typed source recovers all four complete
behaviors and boundaries. Target-local fail-loud ledgers select only
independent schedules, private allocation webs, stack homes, and proved local
literal addends; asserted compiler-local relocation duplicates are removed
while Mickey's runtime assets remain authoritative. The four configured text
SHA256 values in address order are `1c8e29b2...b5a4a`, `007e7280...b001`,
`97fafcab...a3066`, and `305159f6...6a146`; the combined direct linked retail
range has SHA256
`2641cea2b0de10ed4cbe7d63baf818ba5b4c19316fc8049919490730f1a62b6a`.
All **44 runtime text records**, neighboring owners, and the full ROM are
exact. The stretch checkpoint becomes **52,764 / 45,775 campaign bytes
(115.27%)**, **154,016 / 469,264 Overlay C (32.82%)**, and **197,056 /
950,332 resolved text (20.74%)**.

Overlay 1 `+0x19B8..+0x1CA4` contributes **748 exact bytes / 187 words** for
mode-state initialization and object-mapping construction. The first typed
function is naturally exact apart from an external symbol carrier and one
trimmed alignment word. The second recovers the complete two-level reverse
traversal, object flags, calls, state fields, byte-matrix lookup, widths, and
unchecked semantics; a natural four-word compiler carrier and complete
fail-loud representation ledger select the retail schedule without changing
the proved executable set. Configured text SHA256 values are
`e9829b22...c003a` and `3abde2cc...cec34`; direct linked retail SHA256
values are `51c78a58...df3a2` and `17c8a516...4812`. All **28 runtime text
records**, neighboring owners, and the full ROM are exact. The stretch
checkpoint becomes **53,512 / 45,775 campaign bytes (116.90%)**, **154,764 /
469,264 Overlay C (32.98%)**, and **197,804 / 950,332 resolved text
(20.81%)**.

Overlay 1 `+0x296C..+0x2B4C` and `+0x3578..+0x3750` contribute **952 exact
bytes / 238 words** across four gauge and variable-record functions. The
typed bodies preserve both reverse object walks, saturation thresholds,
two-pass initialization, random-range fill, packed record traversal, and
unchecked owner semantics. Measured no-unroll compilation and bounded
fail-loud ledgers complete private representation webs; two identity copies
are removed under whole-function guards, while the reviewed helper-result
alias and conditional-store delay encodings retain the same live values and
effects. Configured SHA256 values are `2ee06458...b6d40`,
`63acb6fa...e0c72`, `be06be13...2c284`, and `9f87498d...d9fd`; direct linked
span SHA256 values are `ef4bdef8...f0819` and `5e9846a5...029a`. All **17
runtime records**, adjoining assembly, and the full ROM are exact. The
checkpoint becomes **54,464 / 45,775 campaign bytes (118.98%)**, **155,716 /
469,264 Overlay C (33.18%)**, and **198,756 / 950,332 resolved text
(20.91%)**.

Overlay 1 `+0x61F0..+0x6270` contributes **128 exact bytes / 32 words** for
cached-mode handling. The typed body preserves both state-clear exits, the
Overlay 27 eligibility call, mode-three Overlay 3 action, fallback dispatch,
and return behavior. Four guarded stages remove only three identity copies and
one redundant unconditional branch before the final fail-loud private
register/schedule ledger. Configured text SHA256 is `e29e178d...15ed`; direct
linked retail SHA256 is `4227d0d2...0f26`. The **13 runtime records** comprise
three calls, five HI16, and five LO16 sites. Zero-valued local carrier symbols
retain the addends already encoded in Mickey's instructions; the double-addend
link is a preserved negative. Both neighbors and the full ROM are exact. The
checkpoint becomes **54,592 / 45,775 campaign bytes (119.26%)**, **155,844 /
469,264 Overlay C (33.21%)**, and **198,884 / 950,332 resolved text
(20.93%)**.

Overlay 60 `+0x3488..+0x355C` contributes **212 exact bytes / 53 words** for
choice-slot reassignment. The typed body preserves the ten-entry availability
map, the low-nibble reservation pass, and the original unchecked first-free
scan for active slots at least six. Its complete guarded ledger selects only
private stack/address/cursor representations. Configured and direct linked
text both have SHA256 `2a3fabec...bf64`; the **8 runtime records** are four
HI16 and four LO16 SYMBOL sites targeting resident offset `0x4D618`. The
four-byte padding neighbor and full ROM are exact. The checkpoint becomes
**54,804 / 45,775 campaign bytes (119.72%)**, **156,056 / 469,264 Overlay C
(33.26%)**, and **199,096 / 950,332 resolved text (20.95%)**.

Overlay 15 `+0xB94..+0xC6C` contributes **216 exact bytes / 54 words** for
rain rendering. The typed body retains the enable/positive-count gates,
intensity conversion, active-camera angle adjustment, three translation
values, unchecked retail hazards, and full `rainFastDraw` ABI. Its guarded
ledger removes one redundant address producer and selects only private count
homes and shipped loader-local addends. Configured and direct linked text both
have SHA256 `22794a3e...a715f`; the **17 runtime records** comprise two
resident calls plus seven HI16 and eight LO16 local sites. Its four-byte zero
padding neighbor and the full ROM are exact. The checkpoint becomes **55,020
/ 45,775 campaign bytes (120.20%)**, **156,272 / 469,264 Overlay C (33.30%)**,
and **199,312 / 950,332 resolved text (20.97%)**.

Overlay 36 `+0x09B8..+0x0A60` contributes **168 exact bytes / 42 words** for
the position-effect callback. The typed body preserves variant-kind selection,
three float-to-s16 coordinate conversions, request ownership, the resident
spawn ABI, nullable return, and spawned-state clear. The configured compiler
emits all owned instructions naturally; the rule trims only eight bytes of
tail alignment. Configured and direct linked text both have SHA256
`58f53464...fb15`; the exact **3 runtime records** are one resident call and
one HI16/LO16 pair for the Overlay 36 variant byte. Its next-function assembly
neighbor and the full ROM are exact. The checkpoint becomes **55,188 / 45,775
campaign bytes (120.56%)**, **156,440 / 469,264 Overlay C (33.34%)**, and
**199,480 / 950,332 resolved text (20.99%)**.

Overlay 15 `+0x0428..+0x0500` contributes **216 exact bytes / 54 words** for
star motion. Typed source publishes the raw movement vector, preserves the
nullable star-array guard and in-place rate scaling, and reproduces the full
fourteen-argument `starfieldFastMove` ABI. Four guarded drops remove only
redundant odd-bound address anchors; complete addend and bijective schedule
ledgers select retail's equivalent local/compiler representation. Configured
and direct linked text both have SHA256 `a51912f1...fa90d`; the sealed semantic
object proves all **21 runtime roles**, comprising one resident call and twenty
local data/BSS records. Both function neighbors and the full ROM are exact. The
checkpoint becomes **55,404 / 45,775 campaign bytes (121.04%)**, **156,656 /
469,264 Overlay C (33.38%)**, and **199,696 / 950,332 resolved text (21.01%)**.

Overlay 53 `+0x0000..+0x011C` contributes **284 exact bytes / 71 words** for
initialization. Typed source preserves ten ordered calls, table setup, the
two-entry configuration loop, height/handle initialization, and final published
handle. A guarded ledger selects only two independent address finalizers and
two complete local LO16 addends. Configured text has SHA256
`cd11eded...60fe`; direct linked text has SHA256 `2e7fe595...06b1`. The exact
**30 runtime records** comprise eight symbol and 22 local/jump sites. The next
matched function and full ROM remain exact. The checkpoint becomes **55,688 /
45,775 campaign bytes (121.66%)**, **156,940 / 469,264 Overlay C (33.44%)**,
and **199,980 / 950,332 resolved text (21.04%)**.

Overlay 53 `+0x016C..+0x0240` contributes **212 exact bytes / 53 words** for
offset-entry copying. Typed source retains the four-argument ABI, mode-selected
signed offset pairs, sentinel list, branch-likely copy loop, coordinate
adjustments, terminal null, and delay-slot effects. IDO emits it naturally with
no normalization. Configured text has SHA256 `c08dfd2d...5708`; direct linked
text has SHA256 `b8c637c7...5f27`. Its exact **5 runtime records** are one
resident call and two local HI16/LO16 table pairs. Both boundaries and full ROM
are exact. The checkpoint becomes **55,900 / 45,775 campaign bytes (122.12%)**,
**157,152 / 469,264 Overlay C (33.49%)**, and **200,192 / 950,332 resolved text
(21.07%)**.

Overlay 1 `+0x438C..+0x5BA4` contributes **6,168 exact bytes / 1,542 words**
for the central object-physics/state updater. Typed source preserves the
collision-record scan, input/mode gates, angle and velocity calculations,
gravity and position integration, recovery path, callbacks, unchecked hazards,
and the exact 48-call order. Object-local `-Wo,-loopunroll,0` reproduces the
single-body loop. A natural `0x17E0` object is extended by a fail-loud
fourteen-word representation pool; a full-digest-guarded bijective decoded
schedule/register/frame web consumes the exact `0x1818` ownership with no
target word array. Configured text has SHA256 `fc05dbdd...b5839`; direct linked
text has SHA256 `392f07c8...6f1c`. All **184 runtime relocations** remain
present and the true-address linked range plus full ROM are exact. The
checkpoint becomes **62,068 / 45,775 campaign bytes (135.59%)**, **163,320 /
469,264 Overlay C (34.80%)**, and **206,360 / 950,332 resolved text (21.71%)**.

Overlay 15 `+0x09E0..+0x0B7C` contributes **412 exact bytes / 103 words** for
the moving-star camera updater. Typed source preserves the camera ABI,
rate-gated coordinate deltas, snapshots, published position, data pointer and
count, all nine bounds, and the fourteen-argument fast-move call. Seven guarded
drops remove only redundant adjacent-field HI anchors; decoded local addends
and a bijective schedule select the shipped form. Configured and linked text
share SHA256 `0b0c1d2c...f36fe`. The loader-authority ledger proves all **39
runtime records**—two calls, 33 BSS records, and four data records—and the
true-address range plus full ROM are exact. The checkpoint becomes **62,480 /
45,775 campaign bytes (136.49%)**, **163,732 / 469,264 Overlay C (34.89%)**,
and **206,772 / 950,332 resolved text (21.76%)**.

Overlay 1 `+0x3FD8..+0x438C` contributes **948 exact bytes / 237 words** for a
two-path, five-phase transition-state machine and makes the adjoining
`+0x3FD8..+0x5BA4` run exact. Typed source retains fades, spawned-position and
path-record selection, rotations, flags, cleanup, hazards, and call order. A
fail-loud three-word representation pool and whole-digest-guarded bijective
schedule/register web select retail's form. Configured and linked text share
SHA256 `2e8eb279...6fb40`; all **13 runtime relocations** remain exact. The
three intra-overlay loader calls keep raw zero fields in the static link while
the runtime ledger proves their separate identities. The true-address range
and full ROM are exact. The checkpoint becomes **63,428 / 45,775 campaign
bytes (138.56%)**, **164,680 / 469,264 Overlay C (35.09%)**, and **207,720 /
950,332 resolved text (21.86%)**.

Overlay 15 `+0x0500..+0x06A4` contributes **420 exact bytes / 105 words** for
the starfield renderer. Typed source retains dimension lookup, display-list
setup, depth clipping, projection, grayscale fade, fill rectangles, pointer
publication, and finish/reset order. Target-local `-Wab,-r4300_mul` supplies
the shipped FP hazard; guarded local addends, a same-target branch displacement,
and a bijective schedule select retail's representation. Configured and linked
text share SHA256 `459d35d0...1eb0`; the loader ledger proves all **10 runtime
records**—two resident calls, six initialized-data sites, and two constant
sites. Both boundaries, the true-address range, and full ROM are exact. The
checkpoint becomes **63,848 / 45,775 campaign bytes (139.48%)**, **165,100 /
469,264 Overlay C (35.18%)**, and **208,140 / 950,332 resolved text (21.90%)**.

Overlay 1 `+0x3750..+0x3E48` contributes **1,784 exact bytes / 446 words** for
the path-selection and interpolation state machine. Typed source preserves the
selection loops, path/record scans, interpolation effects, unchecked hazards,
and all nine calls. One guarded redundant zero rematerialization is removed;
a whole-digest-guarded relocation-aware bijective web selects the shipped
private schedule and register representation. The configured object has the
exact **31-record** linker surface, while the sealed loader proof covers all
**63 runtime relocation roles** by identity and addend. Configured text SHA256
is `c3819bf6...f8`, and the true-address linked range SHA256 is
`fbc2b6be...7b0`. Both boundaries and the full ROM are exact. The checkpoint
becomes **65,632 / 45,775 campaign bytes (143.38%)**, **166,884 / 469,264
Overlay C (35.56%)**, and **209,924 / 950,332 resolved text (22.09%)**.

Overlay 36 `+0x0818..+0x0914` contributes **252 exact bytes / 63 words** for
the nearby-height filter. Typed source retains the inactive-state gate,
six-argument nearby query, reverse result walk, inclusive vertical-band
filter, survivor activation, flag clear, world-state change, and unchecked
hazards. One guarded redundant-zero removal establishes the exact ownership
boundary, and a private whole-digest-guarded bijective schedule/register web
selects retail's equivalent representation without changing an opcode.
Configured and linked text share SHA256 `c457a77e...418f`; all **three runtime
relocations** are exact at their sites, types, operations, identities, and
addends. The true-address range, both boundaries, and full ROM are exact. The
checkpoint becomes **65,884 / 45,775 campaign bytes (143.93%)**, **167,136 /
469,264 Overlay C (35.62%)**, and **210,176 / 950,332 resolved text (22.12%)**.

Overlay 1 `+0x10C8..+0x19B8` contributes **2,288 exact bytes / 572 words**
for packed-record loading and group/metric construction. Typed source retains
both record paths, allocation failure, cycle handling, descending eight-entry
score/rank loops, status sequence, all sixteen calls, and unchecked hazards.
A fail-loud representation pool plus a complete relocation-aware bijective
schedule/register/frame web selects the shipped private form. Configured text
SHA256 is `bd367c6e...40bd`; the sealed loader proof covers all **114 runtime
roles**, while the canonical static object retains the exact **32-record** raw
link surface. The true-address range has SHA256 `f39a4e3c...4ee5`, and both
boundaries plus full ROM are exact. The checkpoint becomes **68,172 / 45,775
campaign bytes (148.93%)**, **169,424 / 469,264 Overlay C (36.10%)**, and
**212,464 / 950,332 resolved text (22.36%)**.

Overlay 13 `+0x0000..+0x0124` and `+0x0188..+0x0284` contribute **544 exact
bytes / 136 words** for module initialization and record allocation. Natural
typed source reproduces every instruction with no normalization while
preserving the distinct initialized-data/BSS bases, 32-record loops,
branch-likely free-record scan, eight-float O32 call shape, and shipped
unchecked paths. The configured objects retain exact **17-record** and
**7-record** relocation surfaces. True-address linked SHA256 values are
`8afb302f...8a8` and `432052e9...e2fb`; both boundaries and the full ROM are
exact. The checkpoint becomes **68,716 / 45,775 campaign bytes (150.12%)**,
**169,968 / 469,264 Overlay C (36.22%)**, and **213,008 / 950,332 resolved
text (22.41%)**.

Overlay 14 `+0x1040..+0x1164` contributes **292 exact bytes / 73 words** for
the module's signed command-index dispatcher. Typed source retains the five
action modes, state updates, success/failure halfword follow-ups, call order,
branch-likely behavior, and unchecked indexing. The natural object owns the
exact opcode census and **15 runtime relocation roles**; a complete guarded
bijective schedule/register web changes no opcode class and leaves the exact
seven-call static link surface. Configured SHA256 is `95798886...34c0`, linked
SHA256 is `5f028b85...f525`, and both owner boundaries plus full ROM are exact.
The checkpoint becomes **69,008 / 45,775 campaign bytes (150.75%)**,
**170,260 / 469,264 Overlay C (36.28%)**, and **213,300 / 950,332 resolved
text (22.44%)**.

Overlay 55 `+0x0000..+0x013C` contributes **316 exact bytes / 79 words** for
module startup. Typed source retains the setup calls, two distinct raw-zero
local bases, state/resource stores, paired four-entry record loop, status
writes, `-80.0f` state, Overlay 56 initialization, and signed result carrier.
Natural codegen owns the exact opcode census and **33 runtime relocation
roles**; a guarded two-word schedule plus two local addends changes no opcode
class and preserves the exact **21-record** raw link surface. Configured
SHA256 is `4cb07c78...bba3`, linked SHA256 is `d8c71b8d...c1d0`, and both
boundaries plus full ROM are exact. The checkpoint becomes **69,324 / 45,775
campaign bytes (151.45%)**, **170,576 / 469,264 Overlay C (36.35%)**, and
**213,616 / 950,332 resolved text (22.48%)**.

Overlay 84 `+0x0DD0..+0x0F18` contributes **328 exact bytes / 82 words** for
`overlay84AdvanceCurrent`. Typed `-O2 -mips2 -32` source preserves signed-byte
forward/backward indexing and wraparound, null and disabled-node skipping,
the branch-likely reload, heading calculation, tilt negation, direction-bit
replacement, and height publication. Natural codegen has the exact CFG,
opcode/register census, memory effects, and all **five runtime relocation
roles**. A complete guarded frame/spill and twelve-instruction schedule web
changes no opcode class or semantic effect. Configured and true-address linked
SHA256 are both `b8992c05...7060`; both boundaries plus full ROM are exact.
The checkpoint becomes **69,652 / 45,775 campaign bytes (152.16%)**, **170,904
/ 469,264 Overlay C (36.42%)**, and **213,944 / 950,332 resolved text
(22.51%)**.

Overlay 36 `+0x150C..+0x1688` contributes **380 exact bytes / 95 words** for
`overlay36UpdatePeers`. Typed `-O2 -mips2 -32` source preserves indexed peer
traversal, rank/timer eligibility, peer-resource activation and spawning,
current-resource replacement, and the shared countdown/state/action teardown.
Natural codegen has the exact six-call CFG, branch-likely sites, FP conversion
schedule, and relocation offsets. A complete guarded schedule, frame, `s0/s1`,
and scratch-register web changes no opcode or semantic effect. Configured and
true-address linked SHA256 are both `cecde584...0620`; all six runtime
identities, both boundaries, and full ROM are exact. The checkpoint becomes
**70,032 / 45,775 campaign bytes (152.99%)**, **171,284 / 469,264 Overlay C
(36.50%)**, and **214,324 / 950,332 resolved text (22.55%)**.

Overlay 86 `+0x007C..+0x0158` contributes **220 exact bytes / 55 words** for
`overlay86ScaledVectorPosition`. Typed `-O2 -mips2 -32` source preserves
signed-byte vector selection and fallback, scale multiplication, in-place
resident transform, the fifth o32 stack argument, and final object-position
addition. Natural codegen has the exact frame, CFG, opcode/register and FP
schedules, branch-likely behavior, and all **three runtime relocation roles**.
A complete guarded seven-immediate private-local-home web changes no opcode or
semantic effect. Configured and true-address linked SHA256 are both
`fa210ac0...e92fa`; both boundaries and full ROM are exact. The checkpoint
becomes **70,252 / 45,775 campaign bytes (153.47%)**, **171,504 / 469,264
Overlay C (36.55%)**, and **214,544 / 950,332 resolved text (22.58%)**.

Overlay 36 `+0x1378..+0x1470` contributes **248 exact bytes / 62 words** for
`overlay36SpawnAndUpdate`. Typed `-O2 -mips2 -32` source preserves
object-position conversion, effect `0x82` spawning, result initialization,
active-resource replacement, and countdown/state/action teardown. Recovering
the second physical argument as `s32` yields natural **62/62** codegen with
the exact three-call relocation surface; no instruction normalization is used.
Configured and true-address linked SHA256 are both `bde47695...52c3`; all
runtime identities, both boundaries, and full ROM are exact. The checkpoint
becomes **70,500 / 45,775 campaign bytes (154.01%)**, **171,752 / 469,264
Overlay C (36.60%)**, and **214,792 / 950,332 resolved text (22.60%)**.

Overlay 86 `+0x02E4..+0x0444` contributes **352 exact bytes / 88 words** for
`overlay86SelectPosition`. Typed `-O2 -mips2 -32` source preserves prior-record
selection, index advance, output clearing, record publication and flagging,
node-list fallback, branch-likely iteration, Y offset, and signed angle return.
Natural codegen has the exact frame, CFG, FP operations, calls, effects, and
all **three runtime relocation roles**. A complete guarded five-home and
two-load private representation changes no opcode or semantic address.
Configured and true-address linked SHA256 are both `9dcc96e3...d111`; both
boundaries and full ROM are exact. The checkpoint becomes **70,852 / 45,775
campaign bytes (154.78%)**, **172,104 / 469,264 Overlay C (36.68%)**, and
**215,144 / 950,332 resolved text (22.64%)**.

Overlay 86 `+0x0158..+0x02E4` contributes **396 exact bytes / 99 words** for
`overlay86BuildTransform` and closes the entire **968-byte
`+0x007C..+0x0444` island** across three independently proved owners. Typed
`-O2 -mips2 -32` source preserves vector-index clamping, active-current
selection, volatile position snapshot, angle setup, scaled table transform,
position subtraction, and the `-1` direction sentinel. Natural codegen has
the exact frame, CFG, opcode/register and FP schedules, calls, effects, and all
**five runtime relocation roles**. A complete guarded two-home swap changes no
opcode or semantic effect. Configured and linked SHA256 are both
`22aabebb...8339`; both boundaries and full ROM are exact. The checkpoint
becomes **71,248 / 45,775 campaign bytes (155.65%)**, **172,500 / 469,264
Overlay C (36.76%)**, and **215,540 / 950,332 resolved text (22.68%)**.

Overlay 36 `+0x1214..+0x1378` contributes **356 exact bytes / 89 words** for
`overlay36SpawnOffsetA9`. Typed `-O2 -mips2 -32` source preserves the local
three-float offset, position conversion, `0xA9` spawn request, owner and ID
publication, spawned-state initialization, conditional kind override,
resource replacement, and countdown/action teardown. Natural codegen has the
exact four-call CFG, opcode census, FP schedule, memory effects, and all
**four runtime relocation roles**. A complete guarded `$f4/$f6` private
constant-carrier web changes no opcode or semantic effect. Configured and
true-address linked SHA256 are both `09ca017f...0be7d2`; both boundaries and
the full ROM are exact. The checkpoint becomes **71,604 / 45,775 campaign
bytes (156.43%)**, **172,856 / 469,264 Overlay C (36.84%)**, and **215,896 /
950,332 resolved text (22.72%)**.

Overlay 36 `+0x0F20..+0x1084` contributes **356 exact bytes / 89 words** for
`overlay36SpawnDirectional`. Typed `-O2 -mips2 -32` source preserves the
directional offset, `0x9F` spawn request, count/link/value fields, owner and
position publication, spawned-state initialization, kind selection, resource
replacement, and countdown/action teardown. Natural codegen has the exact
four-call CFG, opcode census, FP schedule, effects, and all **four runtime
relocation roles**. A complete guarded `$f4/$f6` private constant-carrier web
changes no opcode or semantic effect. Configured and true-address linked
SHA256 are both `2fa63c29...502f7c`; both boundaries and full ROM are exact.
The checkpoint becomes **71,960 / 45,775 campaign bytes (157.20%)**, **173,212
/ 469,264 Overlay C (36.91%)**, and **216,252 / 950,332 resolved text
(22.76%)**.

Overlay 36 `+0x1084..+0x1214` contributes **400 naturally exact bytes / 100
words** for `overlay36SpawnConditional`. Typed `-O2 -mips2 -32` source
preserves the signed and NaN-sensitive direction test, conditional Z offset
and `+45/-45` request value, branch-likely effect, `0x9F` spawn request,
spawn-status override, resource replacement, and countdown/action teardown.
The compiler emits the exact owner with all **four runtime relocation roles**;
no normalization or section trim is used. Configured and true-address linked
SHA256 are both `a1736880...5e38`; both boundaries and full ROM are exact. The
checkpoint becomes **72,360 / 45,775 campaign bytes (158.08%)**, **173,612 /
469,264 Overlay C (37.00%)**, and **216,652 / 950,332 resolved text (22.80%)**.

Overlay 36 `+0x0D8C..+0x0F20` contributes **404 exact bytes / 101 words** for
`overlay36SpawnLinked7F` and closes the full **1,764-byte
`+0x0D8C..+0x1470` island** across five independently proved owners. Typed
source preserves transformed position publication, the `0x7F` request and
three IDs, directional lookup using Overlay 36's finite `FLT_MAX`, spawn
status, resource replacement, and countdown teardown. Natural codegen has all
opcodes and the exact **seven-record** relocation topology. A complete guarded
initial FP-carrier and uniform private-local-home web changes no opcode or
semantic effect. Configured and linked SHA256 are both
`dc2c921f...1a980a`; both boundaries and full ROM are exact. The checkpoint
becomes **72,764 / 45,775 campaign bytes (158.96%)**, **174,016 / 469,264
Overlay C (37.08%)**, and **217,056 / 950,332 resolved text (22.84%)**.

Overlay 36 `+0x0694..+0x07B0` contributes **284 exact bytes / 71 words** for
`overlay36SpawnTransient`. Typed `-O2 -mips2 -32` source preserves the
ten-byte descriptor, float-to-s16 coordinate conversion with `+30.0f` Y,
spawn initialization, mode-dependent state value, type-dependent timer
decrement, and active-object teardown. Natural codegen is exact from `+0x38`
onward with the exact leading opcode multiset. A guarded bijective setup
schedule plus its complete three-temporary rotation changes no instruction or
relocation. Configured and linked SHA256 are both `d6cf5d7e...2a2b43`; the
exact **three-record** relocation surface, both boundaries, and full ROM are
exact. The checkpoint becomes **73,048 / 45,775 campaign bytes (159.58%)**,
**174,300 / 469,264 Overlay C (37.14%)**, and **217,340 / 950,332 resolved
text (22.87%)**.

Overlay 46 `+0x0FD0..+0x112C` contributes **348 naturally exact bytes / 87
words** for `overlay46UpdateTransition`. Typed `-O2 -mips2 -32` source
preserves the mode/timer dispatch, signed halfword fade advance, display-state
publication, resident notification order, cross-overlay unit-scale predicate,
and unconditional local update tail. The exact owner needs no instruction
normalization; its independent compiler alignment word is trimmed. All **28
runtime relocation roles**, both boundaries, the true-address link, and the
full ROM are exact. Configured unlinked SHA256 is `d6c3284f...c4f01` and
linked/direct-retail SHA256 is `2ae1a267...f9e4`. The checkpoint becomes
**73,396 / 45,775 campaign bytes (160.34%)**, **174,648 / 469,264 Overlay C
(37.22%)**, and **217,688 / 950,332 resolved text (22.91%)**.

Overlay 14 `+0x1184..+0x12D8` contributes **340 exact bytes / 85 words** for
`overlay14UpdateTransition`. Natural typed `-O2 -mips2 -32` source already
has the exact CFG, instruction count, opcode inventory, call sites, and all
**28 runtime relocation roles**. A complete guarded private field web selects
the shipped register/home allocation without moving, adding, removing, or
changing the opcode of any instruction. Configured unlinked SHA256 is
`663751d0...e3ba`, linked/direct-retail SHA256 is `c6b6d69c...776e`, both
boundaries and the full ROM are exact. The checkpoint becomes **73,736 /
45,775 campaign bytes (161.08%)**, **174,988 / 469,264 Overlay C (37.29%)**,
and **218,028 / 950,332 resolved text (22.94%)**.

Overlay 57 `+0x4C18..+0x4D90` contributes **376 exact bytes / 94 words** for
`overlay57UpdateModeTrigger`. Natural typed source preserves the exact-size
CFG, all seven calls and delay slots, the deliberately uninitialized trigger,
and all memory effects. A complete guarded setup/index/register web plus three
asserted compiler-elided local-base records reproduces the shipped code. The
configured object retains the exact **38 runtime relocation roles**: 31 local
HI16/LO16 records, five external calls, and two overlay-local jumps.
Configured SHA256 is `5a0c1281...2ff66`; linked/direct-retail SHA256 is
`48dfba27...ae586`. Both boundaries and the full ROM are exact. The checkpoint
becomes **74,112 / 45,775 campaign bytes (161.90%)**, **175,364 / 469,264
Overlay C (37.37%)**, and **218,404 / 950,332 resolved text (22.98%)**.

Overlay 36 `+0x01D0..+0x0694` contributes **1,220 exact bytes / 305 words**
for `overlay36UpdateInteractiveEntity`. Typed source preserves the complete
countdown, record lookup, alpha convergence, animation/position update,
nearby-object query, callback/effect dispatch, ownership, and release paths.
Two guarded countdown-CFG corrections, twelve complete relocation-aware
schedule permutations, and a decoded private frame/register web select the
shipped compiler representation without changing semantic effects. The
configured object retains all **24 runtime relocation roles**. Configured
SHA256 is `b83f6f43...d3758`; linked/direct-retail SHA256 is
`a3befe2e...3af8`. Both boundaries and the full ROM are exact. The checkpoint
becomes **75,332 / 45,775 campaign bytes (164.57%)**, **176,584 / 469,264
Overlay C (37.63%)**, and **219,624 / 950,332 resolved text (23.11%)**.

Overlay 41 `+0x1C84..+0x1DE0` contributes **348 naturally exact bytes / 87
words** for `overlay41DrawItem`. Typed source preserves the indexed resource
lookup, centered background dimensions, intensity-controlled Overlay 67 pass,
and offset/foreground resource draws with exact signed arithmetic. Ordinary
`-O2 -mips2 -32` codegen is exact; only the independent trailing alignment
word is trimmed. The configured object retains all **14 runtime relocation
roles**. Configured and linked/direct-retail SHA256 are
`0d2487d4...7419e`; both boundaries and the full ROM are exact. The checkpoint
becomes **75,680 / 45,775 campaign bytes (165.33%)**, **176,932 / 469,264
Overlay C (37.70%)**, and **219,972 / 950,332 resolved text (23.15%)**.

Overlay 99 `+0x0064..+0x021C` contributes **440 exact bytes / 110 words** for
`overlay99InitializeEntries`. Typed source preserves the bounded three-entry
copy, sentinel-controlled spawn path, deliberately partial descriptor setup,
created-object accounting, storage count, and conditional commit. A complete
guarded address-taken descriptor stack web changes only nine equivalent stack
offsets while preserving every opcode, register, branch, call, relocation,
and memory effect. The configured object retains all **7 runtime relocation
roles**, including both local storage pairs and the exact `+0x90` addend.
Configured and linked/direct-retail SHA256 are `e38eb2e7...684`; both
boundaries and the full ROM are exact. The checkpoint becomes **76,120 /
45,775 campaign bytes (166.29%)**, **177,372 / 469,264 Overlay C (37.80%)**,
and **220,412 / 950,332 resolved text (23.19%)**.

Overlay 11 `+0x0AF4..+0x0C88` contributes **404 naturally exact bytes / 101
words** for `overlay11InitializeFour`. Typed source preserves the
status-dependent formatting, fixed first two handles, mode/flag/string choice
for the third, optional fourth handle, and every unchecked resource hazard.
Ordinary `-O2 -mips2 -32` codegen is exact; only independent section alignment
is trimmed, while the real link resolves three proved local addends. The
configured object retains all **22 runtime relocation roles**. Configured
relocatable SHA256 is `95f9d5d0...4497`; linked/direct-retail SHA256 is
`f6aab93e...170b`. Both boundaries and the full ROM are exact. The checkpoint
becomes **76,524 / 45,775 campaign bytes (167.17%)**, **177,776 / 469,264
Overlay C (37.88%)**, and **220,816 / 950,332 resolved text (23.24%)**.

Overlay 11 `+0x0000..+0x0150` contributes **336 naturally exact bytes / 84
words** for `overlay11Initialize`. Typed source preserves resident and Overlay
66 setup, timer/counter/selection initialization, first-entry gating, the
nonsequential six-mode handle-group dispatch, final common creation, and the
initialized flag. Its compiler-emitted six-entry switch table exactly matches
the existing runtime-relocated table at module `+0x2ED8`; the duplicate private
section is discarded only after rebinding the exact local `+8` text pair. The
configured object retains all **31 runtime text relocation roles**, and the
retained table preserves its six `R_MIPS_32` roles. Configured relocatable text
SHA256 is `4fd1f85f...681a7`, linked/direct-retail text SHA256 is
`cacc9938...f7f3`, and table SHA256 is `38e28147...37b`. Both ranges and the
full ROM are exact. The checkpoint becomes **76,860 / 45,775 campaign bytes
(167.91%)**, **178,112 / 469,264 Overlay C (37.96%)**, and **221,152 / 950,332
resolved text (23.27%)**.

Overlay 11 `+0x11D0..+0x1398` contributes **456 naturally exact bytes / 114
words** for `overlay11UpdateSelection`. Typed source preserves signed direction
thresholds, the asymmetric mode transition sounds, handle emphasis,
controller-bit priority, and the exact `-1/0/1` result encodings. Ordinary
`-O2 -mips2 -32` codegen is exact; only independent section alignment is
trimmed. The configured object retains all **58 runtime relocation roles**:
ten calls and 24 HI16/LO16 pairs. Configured relocatable SHA256 is
`eb9c7d07...3c4c8`; linked/direct-retail SHA256 is `7f1bb1a7...6824`. Both
boundaries and the full ROM are exact. The checkpoint becomes **77,316 /
45,775 campaign bytes (168.90%)**, **178,568 / 469,264 Overlay C (38.05%)**,
and **221,608 / 950,332 resolved text (23.32%)**.

Overlay 19 `+0x00AC..+0x01E0` contributes **308 naturally exact bytes / 77
words** for `overlay19BuildOutput`. Typed source preserves allocation and
failure gates, the three variable output regions, strict capacity rejection,
and allocator-state copy ordering. Ordinary `-O2 -mips2 -32` codegen is exact;
only independent trailing section alignment is trimmed. The configured object
retains all **10 `R_MIPS_26` runtime roles**: three local calls and seven
resident calls across five semantic imports. Configured owned SHA256 is
`a9243893...099`; the true-address linked range and direct retail slice share
SHA256 `534ff130...d3bc`, and the cumulative ROM retains SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. The checkpoint becomes **77,624 /
45,775 campaign bytes (169.58%)**, **178,876 / 469,264 Overlay C (38.12%)**,
and **221,916 / 950,332 resolved text (23.35%)**.

Overlay 99 `+0x02A0..+0x0638` contributes **920 exact bytes / 230 words** for
`overlay99ApplySegment`. Typed source preserves the interpolated line
equations, strict grid-window predicates, NaN behavior, wave identities,
signed-height update, and traversal order. A complete fail-loud bijective web
selects the shipped private compiler representation while preserving the exact
**13 static and 27 runtime relocation roles**. Canonical linkage additionally
removes a proved 16-byte compiler-private `.rodata` duplicate whose text
records are already owned by the retained runtime table. Configured SHA256 is
`a61fff61...53c1`; the true-address linked range and direct retail slice share
SHA256 `098eeddb...ce1b`, and the full ROM retains SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. The checkpoint becomes **78,544 /
45,775 campaign bytes (171.59%)**, **179,796 / 469,264 Overlay C (38.31%)**,
and **222,836 / 950,332 resolved text (23.45%)**.

Overlay 11 `+0x1398..+0x184C` contributes **1,204 exact executable bytes / 301
words** for `overlay11UpdateMenu`. Typed source preserves the complete menu
input, handle, sound, mode, timer, and transition dispatcher, including its
asymmetric thresholds and delay-slot effects. A bounded fail-loud private
frame/spill normalization retains the exact 301-instruction topology and all
**102 runtime relocation roles**. Configured SHA256 is `18c39fe9...6204`;
linked/direct-retail SHA256 is `0f58f7c4...0268`. Both boundaries and the full
ROM are exact. The checkpoint becomes **79,748 / 45,775 campaign bytes
(174.22%)**, **181,000 / 469,264 Overlay C (38.57%)**, and **224,040 / 950,332
resolved text (23.57%)**.

Overlay 63 `+0x077C..+0x0928` contributes **428 exact executable bytes / 107
words** for `overlay63UpdateSequence`. The separate `+0x0928..+0x0930`
eight-byte zero padding remains assembly and earns no C credit. Typed source
preserves token-sequence recognition, toggling, timer/transition behavior, and
both draw passes. A complete immutable-input normalization restores one folded
identity copy and rotates only its private v0/v1 web; the configured object
retains all **39 runtime relocation roles**. Configured executable SHA256 is
`28681480...17ee`; linked/direct-retail executable SHA256 is
`907f4459...fbae`, while the owner including padding is `dd34fe8c...6a00`.
Both boundaries and the full ROM are exact. The checkpoint becomes **80,176 /
45,775 campaign bytes (175.15%)**, **181,428 / 469,264 Overlay C (38.66%)**,
and **224,468 / 950,332 resolved text (23.62%)**.

Overlay 11 `+0x184C..+0x1A7C` contributes **560 exact executable bytes / 140
words** for `overlay11UpdateTwoOptionMenu`. Typed source preserves direction
thresholds, sound choices, the two-handle emphasis loop, input override, both
selection paths, and early-return delay-slot effects. Its only private
compiler residual is a two-use loop-index spill home, selected by a complete
fail-loud normalization. The configured object retains all **46 runtime
relocation roles**. Configured SHA256 is `46895030...361a` and
linked/direct-retail SHA256 is `3660df72...a50f`; both boundaries and the full
ROM are exact. The checkpoint becomes **80,736 / 45,775 campaign bytes
(176.38%)**, **181,988 / 469,264 Overlay C (38.78%)**, and **225,028 / 950,332
resolved text (23.68%)**.

Overlay 63 `+0x0000..+0x01D4` contributes **468 exact executable bytes / 117
words** for `overlay63Initialize`. Typed source preserves subsystem setup,
resource creation, chain/config construction, random ranges and sign choice,
state initialization, four-slot reset, and external flag clears. A complete
fail-loud normalization restores the compiler-folded chain address
materialization and rotates only its bounded private register web. The
configured object retains all **46 runtime relocation roles**. Configured
SHA256 is `f2058a99...9ba6`; linked/direct-retail SHA256 is
`e805a5fa...a0c4`. Both boundaries and the full ROM are exact. The checkpoint
becomes **81,204 / 45,775 campaign bytes (177.40%)**, **182,456 / 469,264
Overlay C (38.88%)**, and **225,496 / 950,332 resolved text (23.73%)**.

Overlay 100 `+0x0580..+0x094C` contributes **972 exact executable bytes /
243 words** for `overlay100DrawMotion`; its final four-byte zero word remains
separately owned padding. The guarded private representation retains the exact
five-call relocation contract. Configured SHA256 is `92be8e4f...73b6`; the
linked 976-byte owner including padding is `75edd755...a5d`. Overlay 100 is
now executable-complete. The checkpoint becomes **82,176 / 45,775 campaign
bytes (179.52%)**, **183,428 / 469,264 Overlay C (39.09%)**, and **226,468 /
950,332 resolved text (23.83%)**.

Overlay 11 `+0x1A7C..+0x1E4C` contributes **976 exact executable bytes / 244
words** for `overlay11UpdateFiveOptionMenu`. Its configured object retains 78
static relocation records, and its separately retained 20-byte switch table is
also exact. Configured SHA256 is `a828dc0f...9189`, linked/direct-retail
SHA256 is `bc8cc120...2bcc`, and the switch table is `31e62780...de1d`.
The checkpoint becomes **83,152 / 45,775 campaign bytes (181.65%)**, **184,404
/ 469,264 Overlay C (39.30%)**, and **227,444 / 950,332 resolved text
(23.93%)**.

Overlay 63 `+0x01D4..+0x074C` contributes **1,400 exact executable bytes /
350 words** for `overlay63UpdateEffects`. The configured object retains all
71 static relocation records: 23 calls and 24 HI16/LO16 pairs. Configured
SHA256 is `b4899f79...1494`; linked/direct-retail SHA256 is
`c335598f...9d1b`. The checkpoint becomes **84,552 / 45,775 campaign bytes
(184.71%)**, **185,804 / 469,264 Overlay C (39.59%)**, and **228,844 / 950,332
resolved text (24.08%)**.

Overlay 19 `+0x01E0..+0x0A30` contributes **2,128 exact executable bytes /
532 words** for `overlay19BuildPlanes`. Typed source preserves the two-pass
plane construction, reciprocal-edge bookkeeping, sentinels, FP behavior, and
unchecked allocation contract. A complete fail-loud private representation
retains the exact four-call relocation surface. Configured and
linked/direct-retail SHA256 are
`4ca7cde8eac58b704c340b7f803d43533f98b78cbafc2242f47f6a1f8920b2a6`.
Every non-padding executable byte in Overlay 19 is now exact C. The checkpoint
becomes **86,680 / 45,775 campaign bytes (189.36%)**, **187,932 / 469,264
Overlay C (40.05%)**, and **230,972 / 950,332 resolved text (24.30%)**.

Overlay 98's edge owners add **684 exact executable bytes / 171 words**:
`overlay98CollectAccepted` at `+0x0144..+0x0234` and
`overlay98CheckObject` at `+0x0848..+0x0A04`. The latter's final 12 zero
bytes remain separately owned assembly padding. Each configured object retains
all six relocation roles. True-address SHA256 values are `90dd69f8...0b23`
and `2d1cbc7a...5a83`; the second complete owner including padding is
`1ec0cbf7...30cc`. The checkpoint becomes **87,364 / 45,775 campaign bytes
(190.86%)**, **188,616 / 469,264 Overlay C (40.19%)**, and **231,656 / 950,332
resolved text (24.38%)**.

Overlay 65 `+0x0080..+0x0BC0` contributes **2,880 exact executable bytes /
720 words** for `overlay65UpdateParticles`. Typed source preserves the
150-record particle update, four-spawn bound, randomized motion and color,
ground-height scan, four-vertex emission, six-quad batching, partial flush,
cursor returns, and finalizer ordering. Its fail-loud complete private
representation retains all 36 static relocation records and all 64 runtime
roles. Configured SHA256 is `54a8f06a...8ed`; linked/direct-retail SHA256 is
`aa0a58c5...1663`. The linked owner and full ROM are byte-exact, and Overlay
65 reaches executable closure. The checkpoint becomes **90,244 / 45,775
campaign bytes (197.15%)**, **191,496 / 469,264 Overlay C (40.81%)**, and
**234,536 / 950,332 resolved text (24.68%)**.

Overlay 98 `+0x0234..+0x0848` contributes **1,556 exact executable bytes /
389 words** for `overlay98RenderReflections`. Typed source preserves the
visible-object traversal, mirrored transform construction, special geometry,
display-list emission, state restoration, and temporary object reflection.
Its complete private representation accounts for all 36 runtime semantic
relocations before conversion to the exact 16-record static proxy surface.
The configured object is 389/389 words exact; linked/direct SHA256 is
`ad443728...f033`, and the complete overlay is `bd4cb91c...2e21`. The linked
owner, whole Overlay 98, and full ROM are byte-exact. All non-padding Overlay
98 executable bytes are now C. The checkpoint becomes **91,800 / 45,775
campaign bytes (200.55%)**, **193,052 / 469,264 Overlay C (41.14%)**, and
**236,092 / 950,332 resolved text (24.84%)**.

Overlay 13's three remaining owners at `+0x0284..+0x0508`,
`+0x0580..+0x0874`, and `+0x0874..+0x0B0C` contribute **2,064 exact executable
bytes / 516 words** and close every non-padding executable byte in the module.
Their configured objects preserve exact 3-, 10-, and 2-record static surfaces
while the runtime ledgers retain all 5, 18, and 10 semantic roles. Every owner,
the complete `0xDC0`-byte overlay, and the full ROM compare byte-for-byte. The
checkpoint becomes **93,864 / 45,775 campaign bytes (205.06%)**, **195,116 /
469,264 Overlay C (41.58%)**, and **238,156 / 950,332 resolved text (25.06%)**.

Overlay 11 `+0x2714..+0x2948` contributes **564 exact executable bytes / 141
words** for `overlay11UpdateModeSix`. Natural codegen is exact in length,
frame, CFG, delay slots, calls, and all 47 relocation sites; the guarded
normalization selects only one complete two-use private spill home. Its owner,
complete overlay, and full ROM compare byte-for-byte. The checkpoint becomes
**94,428 / 45,775 campaign bytes (206.29%)**, **195,680 / 469,264 Overlay C
(41.70%)**, and **238,720 / 950,332 resolved text (25.12%)**.

Overlay 15 `+0x06E8..+0x09E0` contributes **760 exact executable bytes / 190
words** for `overlay15InitStars`. The guarded complete address-lowering web
preserves every store and semantic operation while recovering retail's three
explicit global-address carriers; all 15 runtime roles reduce to the exact
13-record split surface. The owner, complete overlay, and full ROM compare
byte-for-byte. The checkpoint becomes **95,188 / 45,775 campaign bytes
(207.95%)**, **196,440 / 469,264 Overlay C (41.86%)**, and **239,480 / 950,332
resolved text (25.20%)**.

Overlay 15 `+0x004C..+0x0428` contributes **988 exact executable bytes / 247
words** for `overlay15InitStarsAndPalette` and closes every non-padding
executable byte in the module. Its complete relocation-anchored carrier web
drops or zeros no natural instruction, preserves all 14 shipped runtime roles,
and reduces to the exact six-record split surface. The owner, complete overlay,
and full ROM compare byte-for-byte. The checkpoint becomes **96,176 / 45,775
campaign bytes (210.11%)**, **197,428 / 469,264 Overlay C (42.07%)**, and
**240,468 / 950,332 resolved text (25.30%)**.

Overlay 61 `+0x1648..+0x17B8` contributes **368 exact executable bytes / 92
words** for `func_overlay_061_F0001648_18C0A10`. The configured object retains
all nine call relocations and the local path-address pair, while the guarded
normalization owns the complete private copy-size/loop carrier web. The linked
owner, complete overlay, and full ROM compare byte-for-byte. The checkpoint
becomes **96,544 / 45,775 campaign bytes (210.91%)**, **197,796 / 469,264
Overlay C (42.15%)**, and **240,836 / 950,332 resolved text (25.34%)**.

Overlay 11 `+0x1E4C..+0x22E8` contributes **1,180 exact executable bytes / 295
words** for `func_overlay_011_F0001E4C_186A694`. Its guarded complete carrier
schedule drops no natural instruction, retains the exact 41-record split text
surface, and leaves the five-entry runtime switch table and all five table
relocations in their original data/rodata owner. The linked owner, complete
overlay, and full ROM compare byte-for-byte. The checkpoint becomes **97,724 /
45,775 campaign bytes (213.49%)**, **198,976 / 469,264 Overlay C (42.40%)**,
and **242,016 / 950,332 resolved text (25.47%)**.

Overlay 11 `+0x22E8..+0x2714` contributes **1,068 exact executable bytes / 267
words** and eliminates the former `+0x1E4C..+0x2714` middle deficit. Its
complete carrier schedule drops no natural instruction, the pre-link object
retains the exact 39-record split text surface, and an asserted link-only
rebind maps already-encoded local addends to their zero/base carriers. The
linked owner, complete overlay, and full ROM compare byte-for-byte. The
checkpoint becomes **98,792 / 45,775 campaign bytes (215.82%)**, **200,044 /
469,264 Overlay C (42.63%)**, and **243,084 / 950,332 resolved text (25.58%)**.

## 6. Compiler flags

### 6.1 What is measured

| Scope | Flags | How established |
|---|---|---|
| Project default | `-O2 -mips1 -32` | splat/IDO preset; not measured |
| `src/main/` (game code) | `-O2 -mips2 -32` | **Measured.** `ResolveRelocAddress` at `-mips1` emits five load-delay `nop`s the ROM does not have |
| overlay game code, except overlay 5 | `-O2 -mips2 -32` | **Measured** across tranche-A leaves and structural pilots |
| selected overlay TUs | `-O2 -mips2 -32 -Wab,-r4300_mul` | **Measured per object** where the R4300 hazard schedule changes emitted words; kept as narrow Makefile overrides |
| overlay 5 audio-bank TUs | `-O3 -mips2 -32` | **Measured.** DKR supplies the source/flag crosswalk; Mickey's own text fixes six separate boundaries |
| `src/libultra/string.c` | `-O2 -mips2 -32` | **Measured.** Branch-likely instructions |
| 49 libultra io/os TUs | `-O1 -mips2 -32` | **Measured**, one variant at a time. At `-O2` IDO folds away a stack frame the ROM has. Locals need `register` or `-O1` spills them |
| 23 libultra PI/EPI/PFS TUs | `-O2 -g3 -mips2 -32` | **Measured**, one TU at a time. See below |
| 1 libultra TU (`setglobalintmask`) | `-O1 -mips2 -32`, `uopt` forced on at `-O1` | **Measured.** The only group the `cc` driver cannot express; see below |
| 2 libultra libc TUs (`ldiv`, `xlitob`) | all four IDO phases at `-O3 -mips2 -32` | **Measured.** The phase wrapper bypasses the driver's failing `uld` handoff while preserving the phase options that reproduce the ROM |
| `ll` | `-O1 -mips3 -32` | **Measured.** MIPS III compiler-runtime helpers; the generated ELF flags are normalized to O32 after compilation |
| `n_cspsetvol`, `cents2ratio` | `-g -mips2 -32` | **Measured.** Old debug-codegen objects |
| `sinf` | `-O2 -mips2 -32 -Wab,-r4300_mul` | **Measured.** The R4300 multiply-hazard assembler option is required |

The `-mips2` finding now holds for both measured resident game code and the
adopted overlay tranche. It remains expressed as directory-scoped rules, not a
project default: libultra has independently measured per-file flag groups and
future overlays still require at least one discriminating function before a
new module-wide optimisation assumption is made.

`-g3` is IDO's "optimise, but keep full debug information" level, and it
changes code generation. All twelve combinations of
`{-O0,-O1,-O2}` × `{with, without -g3}` × `{-mips1,-mips2}` were compiled and
compared against ROM `0x730A0`/`0x730F0`:

| Flags | Function size | vs the ROM's `0x48` |
|---|---|---|
| `-O0`, any of the four | `0x60` | no |
| `-O1` / `-O2`, `-mips1` or `-mips2`, no `-g3` | `0x44` | no |
| `-O1 -g3 -mips1` | `0x50` | no |
| `-O2 -g3 -mips1` | `0x4C` | no |
| **`-O1 -g3 -mips2`** | `0x48` | **byte-identical** |
| **`-O2 -g3 -mips2`** | `0x48` | **byte-identical** |

The `cc` driver still dies in `uld` on an ordinary `-O3` invocation. The
phase wrapper makes `ldiv` and `xlitob` reproducible because neither needs the
interprocedural convention rewrite performed by `uld`: it asks the driver for
an `-O2` four-stage pipeline, then promotes `cfe`, `uopt`, `ugen`, and `as1`
individually to `-O3`. `xprintf` and `xldtob` do need `uld`'s rewrite and stay
assembly; compiling their published C without it changes function boundaries
and calling convention, not merely scheduling.

Three consequences:

- **`-O2` was not established by `epiread`/`epiwrite`, and is now.** On those
  two files `-O1 -g3 -mips2` and `-O2 -g3 -mips2` are byte-identical, so their
  bytes do not discriminate the optimisation level at all, and `-O2` was
  borrowed from JFG's published Makefile. The nine TUs added to the group since
  do discriminate it: `pidma` (ROM `0x6FB90`) is 48 instructions at
  `-O2 -g3 -mips2` and matches word for word, against 68 at `-O1 -g3 -mips2`
  and 78 at `-O0`. So `-O2 -g3 -mips2` is now measured on this group rather
  than assumed for it. The *structural* claim epiread supported still holds:
  `-g3` stops IDO hoisting the third argument's spill into the first `jal`'s
  delay slot and reverses the epilogue's `lw ra` / `lw v0` order, and the ROM
  agrees with `-g3` on both.
- **The ten `-O1 -mips2` TUs above should be re-examined.** They were measured
  before `-g3` was known to be in play. They match byte-for-byte, so nothing is
  wrong; but "the flags that reproduce these bytes" and "the flags this file
  was built with" are different claims, and only the first is established, as
  the `-O1`/`-O2` tie above demonstrates for the new group.
- **`-g3` came from a reference build's configuration, not from deduction.**
  Reading a permitted decompilation's build configuration is the same
  permission as reading its source (§1.3); what makes `-g3 -mips2` a fact about
  Mickey is that it was then measured against Mickey's bytes. `-O2` did not
  survive that test.

A practical side effect: `-g3` makes IDO write a `.u` sidecar into the working
directory for every such compile. `.gitignore` covers it.

`-Wab,-r4300_mul` is now settled for `sinf`: without it the object does not
match, and with it the text is byte-identical. It remains per-file evidence,
not a project default.

**The fourth group: `uopt` at `-O1`, which the driver cannot produce.**
`tools/ido/cc` runs `uopt` only at `-O2` and above — `cc -v` lists no `uopt`
stage at `-O1` × `{-g0,-g1,-g2,-g3,none}` — so "optimised by `uopt`, at `-O1`"
is unreachable through it, and `-Wo,-O1` does not fake it because
`-W<pass>,-O<n>` is inserted *before* the driver's own `-O` and the last `-O`
wins. `tools/ido-phases.py` drives `cfe`/`uopt`/`ugen`/`as1` directly,
taking each phase's command line from `cc -v` rather than reimplementing the
driver, and accepts `-Xphase,<phase>,<option>` overrides (`-Xphase,uopt,+`
forces the stage to exist). With no override it is byte-identical to `cc`,
verified on `string.c`, `epidma.c`, `si.c`, `pfsreadwritefile.c` and on a
`GLOBAL_ASM` TU under asm-processor.

What it buys: `__osSetGlobalIntMask` (ROM `0x75080`) is byte-identical with
`uopt` at `-O1` — 19 of 19 words, only the six the linker fills in differing.
Through the driver the same source gives 20 instructions with the first `jal`'s
delay slot empty, and none of twenty-two `{-O0,-O1,-O2}` × `{-g,-g1,-g2,-g3,
none}` × `{-mips1,-mips2}` combinations closes it. `-g3` is irrelevant here;
`-mips2` is required; `-O2` destroys the frame size.

This is also the strongest evidence in the tree that the vendored IDO *is*
Mickey's compiler for libultra: a whole ROM function, delay slots included,
reproduced exactly once the driver stops hiding a phase. `uopt`'s `-O` is the
only per-phase `-O` with teeth — on `string.c` at `-O2 -mips2`,
`-Xphase,uopt,-O1` changes `.text` from `0xA0` to `0xE0` bytes while
`-Xphase,ugen,-O1/-O3` and `-Xphase,as1,-O1/-O3` are bit-identical to the
baseline.

### 6.2 Odd floating-point registers: settled — those files stay assembly

ROM-wide there are 1727 odd FP register operands across 9 static-segment asm
files, and GNU as warns once per occurrence. **No SGI IDO build can produce
them.** That is a mechanism, not an exhausted sweep, and the question is closed.

`ugen` does implement SGI's `-fp32regs`, which frees the odd single-precision
registers, and the `cc` driver can route it — `-Wc,` is the code generator,
`-Wo,` is `uopt`, which prints "unrecognized option" and drops it, which is why
earlier sweeps found nothing. `cc … -mips2 -32 -Wc,-mips3 -Wc,-fp32regs` is
byte-identical to driving the phases by hand, and it does emit odd registers.
It still cannot produce Mickey's:

- **The mechanism.** In the function-matched decompilation of real IDO 7.1's
  `ugen`, `-fp32regs` is one unconditional loop that adds all 16 odd registers
  `$f1,$f3,…,$f31` to the free list, with no reference to which are argument,
  return or callee-saved halves. The reservation logic that protects live
  registers walks the register file by even steps only — it predates
  `-fp32regs` and was never taught about odd halves. The two paths are
  structurally disconnected, so `-fp32regs` can never confine itself to
  caller-saved temps and can never save what it does clobber, in any
  invocation. The `-ufsm`/`-ufsa` callee-save tracking is a separate feature
  the `-fp32regs` free-list path does not consult.
- **Every version agrees.** Six builds — IDO 4.1, 5.2, 5.3, 6.0, 7.1 and
  MIPSpro 7.4.4 — choose the identical all-32 register set on the same test
  function; 5.2, 5.3, 6.0 and 7.4.4 are byte-identical for it. 4.1 differs only
  in schedule length, not in allocation policy. There is no version boundary to
  find.
- **What our build actually gets.** With `-Wc,-mips3 -Wc,-fp32regs`,
  `MatrixMultiplyVec4` comes out at the ROM's exact 53 instructions, the exact
  instruction kinds, and the odd-register phenomenon — but allocated across all
  32 registers including `$f0/$f2` (return), `$f12/$f14` (argument) and the odd
  halves of callee-saved `$f20`–`$f30`, written in a leaf function with no
  save/restore. The ROM confines itself to `$f4`–`$f11` and `$f16`–`$f18`: the
  caller-saved even temps plus their odd partners, and nothing else.

**Consequence for this project: the nine files stay as assembly. Do not
re-sweep compiler flags or IDO versions for them** — the mechanism above says
in advance that every such sweep fails. Hand-written assembly is the standing
explanation, consistent with the density spread (61.3% of FP operands in
`asm/59DB0.s` down to 2.7% in `asm/16140.s`, the matrix TU at 40.1%) and with
sibling decomps, where genuinely hand-written float assembly (JFG's
`asm/hasm/math_matrix.s`, PD's `src/lib/mtxasm.s`) sits in the same odd-density
range and is equally ABI-indifferent, while real IDO-compiled float code in
those trees contains no single-precision odd-register arithmetic at all.

Two things would reopen it, and only these two: a code generator whose
odd-register free list is gated by the same reservation logic the even path
already uses (no such build is known — it would have to be a hand patch), or a
matched, non-assembly function in some other decomp that uses odd single
registers, with a working build recipe attached.

No TU is built with `-Wc,-mips3 -Wc,-fp32regs` and none is expected to be. If
one ever is, it must be a per-file group and never a default: all seven
already-matching TUs tested change bytes under it, and the damage is entirely
`-mips3` reaching `ugen` (which `-fp32regs` requires) — on FP-free code
`ugen -mips3` and `ugen -mips3 -fp32regs` are bit-identical, so `-fp32regs`
itself is inert there, while `ugen -mips3` stops folding small negative
immediates and grows `libultra/string.c`'s `.text` by 30%. `as1` also asserts
outright on some `-fp32regs` ucode, so any member must be built and checked,
never assumed to compile.

It is not a blocker for the naming work: nothing named so far is float-heavy,
and the tier-A matches that *are* in float-adjacent code (`mtxf_*` in
`math_util.s`) are against DKR's **hand-written assembly** objects, which
sidesteps the question rather than answering it.

Relevant to any future resolution: `mtxf_transform_point`, `mtxf_mul`,
`mtxf_to_mtx`, `vec3s_reflect` and `mtxf_from_inverse_transform` at ROM
`0x2A250` are byte-identical to DKR's `.s` sources. If Mickey's own
`main/matrix` TU at `0x2B650` turns out to be hand-written too, that is the
same story one file over.

**Names are already sitting there, waiting on the C.** A masked-skeleton scan
against Jet Force Gemini's built `asm/hasm/math_matrix.s.o`, independently
re-verified byte-for-byte with `tools/find_known_objects.py` (real
relocation-masking, not the coarse scan), identifies all four of
`src/main/matrix.c`'s parked functions:

| VRAM | ROM | JFG name | masked/words | romocc |
|---|---|---|---|---|
| `0x8002AB78` | `0x2B778` | `matrix_RPY_XYZ` | 6/67 | 1 |
| `0x8002AC84` | `0x2B884` | `matrix_XYZ_YPR_SCL` | 6/99 | 1 |
| `0x8002AE10` | `0x2BA10` | `matrix_XYZ_YPR` | 6/87 | 1 |
| `0x8002AF6C` | `0x2BB6C` | `matrixTransposeVectorMultiply` | 0/53 | 1 |

None of these are adopted into `symbol_addrs.us.txt`: section 1.5 forbids a
name for a function whose C is parked non-matching, since that would put a
second evidence tier into the symbol file. They are recorded here, in prose,
against the day `main/matrix.c`'s odd-FP-register blocker (above) is resolved
and the four bodies match — at which point these names, JFG's own for the
identical bytes, are what tier A says to adopt, and `src/main/matrix.c`'s own
comments (out of this lane's ownership) are where a `PROVENANCE` line for them
belongs.

### 6.3 Data ownership: SDK tails carved without guessing the bulk boundary

ROM `0x76E60`–`0x81590` is currently treated as `data` and
`0x81590`–`0x86640` as `rodata`. Most remains anonymous. This epoch assigned
only sections whose size and ordering are fixed by a matching reference object
and whose first symbol is reached by Mickey's own relocation: initialized data
for `aisetnextbuf`, `vimgr`, `thread`, `siacs`, `vi`, `timerintr`, and `xlitob`;
rodata for `cents2ratio` and `sinf`; and BSS slices for `seteventmesg`, `vimgr`,
`sptask`, `siacs`, and `timerintr`. Anonymous gaps remain raw and explicit.

**`gmake verify` is not the gate for a rodata boundary.** Re-slicing never
creates, drops or reorders a byte, so a wrong boundary rebuilds the ROM
byte-identically and every gate stays green. Cutting four bytes into
`jtbl_80080D2C` was tried deliberately: the build passes, and the half of the
table on the far side of the cut comes out as a fresh `dlabel D_80080D30`
holding a plain `.word 0x80007774` where the whole table holds
`.word .L80007774`. Same bytes, silently stripped of their typing. What checks
a boundary is reading the generated `.s` on both sides of it.

Where the boundary comes from:

- **`0x81510` is where read-only content demonstrably starts, although the
  current conservative split is `0x81590`.** A 32-space pad
  string, a 32-zero pad string and `"0123456789ABCDEF"` -- libultra `xprintf`'s
  field-padding tables -- then the audio subsystem's `printf` format strings,
  unbroken to `0x81920`. The symbol before it ends exactly at `0x81510`.
- **It is an upper bound, not the boundary.** Everything below it in
  `0x76E60`–`0x81510` is undifferentiated; some of it is certainly rodata too.
- **`0x80080D24` (ROM `0x81924`)** is a weaker bound of a different kind: the
  `FLT_MAX` constant loaded from ROM `0x50B0`, two words below the first jump
  table, and the lowest address any resident instruction reads as read-only
  data. The strings above it are read by nothing resident at all -- the same
  pattern as the model/sprite strings in §7 -- so a reference-derived bound
  cannot see them.
- **rodata order follows text order exactly.** 35 functions, 44 jump tables,
  monotonic in both columns, **zero inversions**. So `.rodata` can be carved TU
  by TU in text order, which is what makes the per-TU split tractable.

Two toolchain facts govern the per-TU split:

- Typing the tail as rodata labels every jump *target* in the text it lands in.
  Those labels are emitted with `asm_jtbl_label_macro`, and inside a
  `#pragma GLOBAL_ASM` file a `jlabel` line makes asm-processor miscount the
  section. The yaml uses `glabel`, which is the same global label and parses.
- Attaching rodata to a `c` subsegment migrates it into the function's own
  `.s` as `.late_rodata`. The vendored asm-processor now recognizes splat's
  zero-size `enddlabel` directive, so the symbol-size metadata is preserved
  and this is no longer a tooling blocker. The remaining blocker is evidence:
  do not assign another slice until its object size and ROM references agree.

---

## 7. What this map does not cover

- **The original source ownership inside overlay modules.** All 107 modules
  are represented and all 106 non-empty text ranges are buildable code
  segments (§5.4), but the generated assembly does not establish original TU
  boundaries. Exact donor matches establish a few functions; most overlay
  functions still carry generated `(overlay, offset)` identities.
- **`0x16B0000`–`0x1848B70`** (1.60 MiB, the `unclassified` segment). It is not
  overlay data and its entropy is not the asset band's. Nothing in the resident
  segment or the overlay tables points into it.
- **The asset region `0x87000`–`0x16B0000`** is one `bin`. Entropy suggests at
  least two classes (a near-random band to ~`0xAC0000` and a more structured
  one beyond), but nothing is decoded. `gzip_inflate_*` at `0x4EA60` is the
  decompressor that reads it, which is the obvious way in.
- **`0x86640`–`0x87000`** looks like a table of ROM offsets and is unidentified.
- **The model/sprite code**, on the same evidence as the build stamp (§2.1).
  `"CREATE LOD MODEL :: null model pointer!"` (`0x80081904`) and `"Cam do 2D
  sprite called with NULL pointer!"` (`0x800819F1`) are in the resident
  segment's rodata, but **no resident instruction builds either address**: the
  nearest references anywhere in `asm/` are `D_80081898` below and
  `D_80081A1C` above, and the two strings sit in the gap between them. So the
  asserts that print them are in an overlay, and the resident segment holds
  only their text. In particular `0x1FC9C` is *not* a model/sprite anchor:
  `func_8001F09C` is a float routine that steps a value at `+0x50` toward a
  bound at `+0x54` and clamps it, with no connection to either string.
- **9.4% of the libultra corridor** (§4.1): `0x70AF0`–`0x70E20` and
  `0x74090`–`0x748B0`, `0xB50` between them, matching nothing in any of the
  five reference builds. **It may not be libultra at all**: the label is
  inherited from a map made when the corridor was 78% identified against one
  build, and five builds' worth of silence is now the more informative fact.
  Disassemble it rather than mine it further; §4.1.
- **What is still `asm` that need not be.** 174 subsegments are named; 172 of
  them have a measured whole-`.text` boundary (the exceptions are `main/matrix`
  and `main/runlink`, which are decompiled rather than matched-as-a-unit).
  **92** are now `c`: 81 fully C and the other 11 carrying only
  `#pragma GLOBAL_ASM`. A scaffolded subsegment is worth having on its own:
  it proves the file boundary against the link before any C is written, and it
  turns every function in the unit into a separate work item. The remaining
  82 named subsegments include verified original assembly as well as compiler
  output; `verified_asm.us.txt` is the explicit evidence ledger that keeps
  those two completion states separate.
- **`libultra/contramread` and `libultra/contramwrite` are blocked on one
  scheduling decision each.** Both reproduce every word of the ROM but five,
  inside one four-times-unrolled byte fill: the ROM puts `addiu s0, s0, 4` in
  the loop branch's delay slot, this IDO bumps the pointer early and puts
  `sb zero, -4(s0)` there. The instruction multisets agree. `setglobalintmask`
  was in this list and is no longer: its empty delay slot was the `cc` driver
  refusing to run `uopt` at all below `-O2`, and driving the phases directly
  closes it (§6.1's fourth flag group). That removes the "the scheduler is not
  the one that built Mickey" reading of the third case; what remains for these
  two is a real disagreement, not a phase the driver skipped.
- **The `0xF0` bytes at `0x505E0`–`0x506D0`**, between the end of
  `os/exceptasm.s` and the next subsegment. Unidentified.
- **`0x6B860`–`0x6BDF0`** (`0x590`), between Perfect Dark's three Transfer Pak
  routines. Almost certainly more of the same driver; PD's build does not
  contain whatever it is.
- **The object system.** The `"setting up"` / `"freeing"` / `"processing"` /
  `"exploding"` phase names sit in a 5-entry pointer table at `0x8007A220`, whose first
  entry is `"null"`, immediately followed by four function pointers
  (`0x8002B280`, `0x8002B314`, `0x8002B768`, `0x8002B524`) which are exactly
  the memory routines the linker calls. Something at `0x8007A214` is a
  descriptor combining phase names with allocator entry points. The only
  resident reader of the phase table is the crash reporter at `0x80046548`, so
  the phases are what a fault is reported *during*. That is as far as the
  evidence goes; no struct is asserted for it.
