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
the segment, carrying 194 function names.

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
| `0x34180`–`0x34E60` | `0x80033580` | `main/gameVi` | B + A landmarks | **Video and framebuffer management** (§3.4). The complete 23-function order and call/global surface establish the TU boundary; four functions inside are independently tier-A JFG skeleton hits |
| `0x342A8` | `0x800336A8` | `"Ntsc LowRes"` … | — | Video-mode table (15 entries) |
| `0x39A1C` | `0x80038E1C` | `"front/front.c"` asserts | — | **`front` code is partly resident** |
| `0x3B1A0` | `0x8003A5A0` | `"UNKNOWN TRACK"` | — | Track selection |
| `0x3B57C` | `0x8003A97C` | `weather_clip_planes` | A | |
| `0x3D5F0` | `0x8003C9F0` | `reset_particles` | A | |
| `0x43470` | `0x80042870` | `strcpy`, `memset`, `sprintf`, `_itoa`, `vsprintf` | A + C | The C-library / formatting layer |
| `0x459C0`–`0x467BC` | `0x80044DC0` | `diRcpPrintDL`, `diRcpMoveWd`, `diRcpStrName`, `diRcpOtherMode`, `diRcpGeometryMode` | C | **The display-list disassembler**, a full GBI pretty-printer left in the retail build |
| `0x467BC`–`0x47A60` | `0x80045BBC` | `diCpuReportWatchpoint`, plus the memory/module debug pages and the register-dump crash reporter | C | **The debug monitor**, also left in |
| `0x47A60`–`0x47A70` | `0x80046E60` | `main/get_stack_pointer` | A | Measured file boundary |
| `0x4BC40`–`0x4E1E0` | `0x8004B040` | `main/font` | A/D | JFG's `font.c`: six exact function anchors plus source-order and adjacent-function evidence establish the provisional C split; §3.4 |
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
| `src/gameVi.c.o` | 4 | `0x34B68`–`0x34E60` | Inside the now-split `main/gameVi` TU (§3.4). The four exact skeleton hits are landmarks; the boundary is separately established at tier B from the complete ordered function/call surface, not claimed as a whole-`.text` byte match |
| `src/menu.c.o` | 6 | `0x3A184`–`0x3B008` | The automated pass found only interior anchors. A later function-order and call-graph census established the narrower `0x39350`–`0x3B1A0` ownership (§3.4); no whole-`.text` match is claimed |
| `src/gameVi.c.o` | 4 | `0x34B68`–`0x34E60` | Inside yaml's unnamed `0x34180`–`0x37D50` block. No whole-`.text` match; no boundary claimed |
| `src/anim.c.o` | 3 | `0x50D7C`–`0x51D28` | Inside yaml's unnamed `0x50C00`–`0x58570` block. No whole-`.text` match; no boundary claimed |
| `src/models.c.o` | 3 | `0x20020`–`0x21710` | Inside yaml's unnamed `0x20020`–`0x21DA0` block, starting exactly at its boundary. No whole-`.text` match; no boundary claimed |
| `src/font.c.o` | 2 | `0x4BC70`–`0x4C884` | The original >=10-word scan found two anchors. The later complete census in §3.4 found four more exact short functions and split `main/font` provisionally; no whole-`.text` match is claimed |
| `src/audio_manager_4C50.c.o` | 2 | `0x45F0`–`0x4F3C` | Starts exactly at yaml's `0x45F0` boundary; ends inside the unnamed `0x4F40`–`0xC950` block. No whole-`.text` match; no boundary claimed |
| `src/audio_manager_1050.c.o` | 3 | `0x12BC`–`0x22C8` | Inside yaml's unnamed `0x1050`–`0x45F0` block. Wide span for 3 hits -- other code plainly sits between them; no boundary claimed |
| `src/charControl.c.o` | 2 | `0x1CED4`–`0x1FFAC` | Inside yaml's unnamed `0x1C790`–`0x20020` block. No boundary claimed |
| `src/camera.c.o` | 2 | `0x23360`, `0x5B778` | 230KB apart -- evidently not one placed TU here; treat as two independent identifications, not a span |
| `src/memory.c.o` | 2 | `0x2BCD0`–`0x2C3AC` | Starts exactly at yaml's `0x2BCD0` boundary (end of `main/matrix`); the already-named `align16`/`align8`/`align4` (tier A, `memory.c.o`) sit at `0x2C860`, past this span. Consistent with one TU, no boundary claimed |
| `src/shadows_214A0.c.o` | 2 | `0x18FF0`–`0x19144` | Inside yaml's unnamed `0x18FF0`–`0x1AE60` block, starting exactly at its boundary. No boundary claimed |
| `src/saves.c.o`, `src/rcpFast3d.c.o`, `src/track.c.o`, `src/textures.c.o`, `src/diCpu.c.o`, `src/objects.c.o`, `libultra/src/flash/flashreadid.c.o`, `us.v10/src/core1/code_1D00.c.o` (BK) | 1 each | single points | Isolated identifications, no span to claim |

**Why this table originally added no `mickey.us.yaml` splits.** §1's
"measured file boundary" tier requires a whole-`.text` match; this pass only
matched individual functions (`tools/find_known_objects.py --sections` found
no whole-object match for any of the not-yet-named TUs above). The later
`main/font` split is explicitly provisional, not a tier-A measured-file claim:
§3.4 records its additional endpoint and ordering evidence. The already
measured TUs above (`n_csplayer`, `gsSnd`, `n_drvrNew`, `n_env`, `n_load`,
`math_util`) needed no new split; they already have one.

### 3.4 `main/font` census

ROM `0x4BC40`–`0x4E1E0`, VRAM `0x8004B040`–`0x8004D5E0`, is split as
`main/font`. The split is provisional (tier D at the file-boundary level), not
a whole-object match. It begins with JFG's byte-identical `fontSetWindow0`,
contains JFG `font.c` functions in source order, and ends after the
`fontYSpacing`-shaped leaf; the next function is the independently identified
`osCreatePiManager`. A supplemental all-size object scan found six exact JFG
anchors in the range: `fontSetWindow0`, `fontSetWindowNoise`, `fontColour`,
`fontWindowColour`, `fontWindowFontColour`, and
`fontWindowFontBackground`. Each has one ROM occurrence; the four colour
setters have at least 7 unmasked words and the two already adopted functions
have at least 10. JFG's complete `font.c.o` does not match Mickey's complete
range.

PROVENANCE: the TU identity, candidate names, declarations, and struct-layout
starting point come from Jet Force Gemini's public decompilation
(`src/font.c`, `src/font.h`, and its built object), a permitted published
retail-derived decomp under `docs/CLEANROOM.md`. Mickey's instructions,
relocations, call graph, and ROM comparison remain authoritative. A
PROVENANCE note is carried at the point of use in `src/main/font.c`.

The table is the complete original `0x4BC40`–`0x4EA60` block census. "A" is
an exact object/ROM skeleton identity; "B" is a call-graph role; "D" is only
source order and structure. D-only JFG placeholders remain Mickey
`func_<VRAM>` names. Calls list in-range callees; `ext` means only resident or
overlay callers/callees outside the range were observed.

| ROM | Size | Mickey symbol | JFG correspondence | Evidence | Calls |
|---|---:|---|---|---|---|
| `0x4BC40` | `0x24` | `fontSetWindow0` | same | A, matched C | leaf; ext callers |
| `0x4BC64` | `0x0C` | `func_8004B064` | `fontSetButtonMode` | D, matched C | leaf; overlay caller |
| `0x4BC70` | `0x34` | `fontSetWindowNoise` | same | A, matched C | leaf |
| `0x4BCA4` | `0x14` | `func_8004B0A4` | `fontUseFont` | B/D, matched C | leaf; text-setup callers |
| `0x4BCB8` | `0x24` | `fontColour` | same | A, matched C | leaf; text-setup callers |
| `0x4BCDC` | `0x1C` | `func_8004B0DC` | `fontBackground` | B/D, matched C | leaf; text-setup callers |
| `0x4BCF8` | `0x44` | `func_8004B0F8` | `fontPrintXY` | B/D, matched C | calls `0x4BD3C` |
| `0x4BD3C` | `0xA0` | `func_8004B13C` | `fontPrintWindowXY` | B/D, matched C | calls `0x4BDDC` |
| `0x4BDDC` | `0x8B0` | `func_8004B1DC` | JFG `func_80070518` | D | calls `0x4DF9C`, `0x4C68C`, `0x4D290`, ext |
| `0x4C68C` | `0xB8` | `func_8004BA8C` | `fontStringWidth` | B/D | calls `0x4DF9C`; ext callers |
| `0x4C744` | `0x9C` | `func_8004BB44` | `fontWindowSize` | D, matched C | leaf; ext callers |
| `0x4C7E0` | `0x1C` | `func_8004BBE0` | `fontWindowUseFont` | B/D, matched C | leaf; ext callers |
| `0x4C7FC` | `0x40` | `fontWindowColour` | same | A, matched C | leaf; ext callers |
| `0x4C83C` | `0x48` | `fontWindowFontColour` | same | A, matched C | leaf; ext callers |
| `0x4C884` | `0x40` | `fontWindowFontBackground` | same | A, matched C | leaf; ext callers |
| `0x4C8C4` | `0x2A0` | `func_8004BCC4` | `fontWindowAddStringXY` | B/D, plateau | calls `0x4D1A4`, `0x4C68C`; ext callers |
| `0x4CB64` | `0x4C` | `func_8004BF64` | `fontWindowFlushStrings` | B/D, matched C | leaf; ext callers |
| `0x4CBB0` | `0x28` | `func_8004BFB0` | `fontWindowEnable` | B/D, matched C | leaf; ext callers |
| `0x4CBD8` | `0x28` | `func_8004BFD8` | `fontWindowDisable` | B/D, matched C | leaf; ext callers |
| `0x4CC00` | `0xC4` | `func_8004C000` | `fontStringAddNumber` | D, matched C | leaf; called by `0x4D1A4` |
| `0x4CCC4` | `0x7C` | `func_8004C0C4` | `fontWindowsDraw` | B/D | calls `0x4CE00`; ext caller |
| `0x4CD40` | `0xC0` | `func_8004C140` | JFG `func_80071564` | D | ext callee; called by `0x4CE00` |
| `0x4CE00` | `0x3A4` | `func_8004C200` | `fontWindowDraw` | B/D | calls `0x4CD40`, `0x4D1A4`, `0x4BDDC` |
| `0x4D1A4` | `0xEC` | `func_8004C5A4` | JFG `func_80071A0C` | D, matched C | calls `0x4CC00`; in-range callers |
| `0x4D290` | `0x248` | `func_8004C690` | JFG `func_80071B08` | D | ext callee; called by `0x4BDDC` |
| `0x4D4D8` | `0xA54` | `func_8004C8D8` | `fontCreateDisplayList` | B/D | ext callee |
| `0x4DF2C` | `0x70` | `func_8004D32C` | no JFG counterpart | D | leaf; ext caller |
| `0x4DF9C` | `0x70` | `func_8004D39C` | `fontConvertString` | B/D, plateau | leaf; in-range callers |
| `0x4E00C` | `0x1B4` | `func_8004D40C` | `fontGetLine` | D | leaf |
| `0x4E1C0` | `0x20` | `func_8004D5C0` | `fontYSpacing` | D, matched C | leaf |
| `0x4E1E0` | `0x170` | `func_8004D5E0` | `osCreatePiManager` | B/D | SDK calls; ext callers |
| `0x4E350` | `0x28` | `func_8004D750` | `rzipInit` | B/D | allocator call; ext caller |
| `0x4E378` | `0x30` | `byteswap32` | JFG `rzipUncompressSize` | A name collision | leaf; ext callers |
| `0x4E3A8` | `0x38` | `func_8004D7A8` | `rzipUncompressSizeROM` | B/D | calls `byteswap32`, ext |
| `0x4E3E0` | `0x60` | `func_8004D7E0` | `rzipUncompress` | B/D | calls `gzip_inflate_block`; ext callers |
| `0x4E440` | `0x620` | `func_8004D840` | `huft_build` | B/D | calls `_bzero`; called by `main/gzip_asm` |

`func_8004BCC4` compiles to the exact 168-instruction shape and exact masked
words at the stock flags, but is not an object match: four relocation sites
(two HI16/LO16 pairs) retain different symbol identities. The first is at
function offset `+0x34`, where the pool-end address is spelled as
`D_800D60E8 + 0x400` instead of `D_800D64E8`; the second starts at `+0x98`,
where window zero's width field is based on `D_800D64E8` instead of its
`D_800D64F4` field alias. The 119-combination flag lattice kept the stock
`-O2 -mips2` result best. An explicit-alias source variant fixed the names but
changed the frame and added three instructions, so the readable JFG-derived
candidate remains under `NON_MATCHING` and the extracted assembly is
canonical.

`func_8004D39C` plateaued after the stock JFG body and six source-allocation
variants. Its best candidate has the exact 28-instruction shape and 27 exact
words; the first and only mismatch is at function offset `+0x60`, where the
loop-back branch consumes the copied character rather than the original load.
The 119-combination flag lattice found no exact result and kept the same
one-word residue throughout the `-O2 -mips2` family, identifying an allocator
coalescing choice rather than a flag mismatch. The candidate remains guarded
by `NON_MATCHING`; the extracted assembly stays canonical.

The font subsegment's FP-register census contains only even-numbered single-
precision registers (`$f0`, `$f4`, `$f6`, `$f8`, `$f10`, `$f16`, and `$f18`),
so no function in this TU was excluded by the odd-register rule in section
6.2.

There are no direct string-literal references in this block. Its data
relocations address font/window state, a font-cache jump table, and rzip
state; consequently no tier-C names are available. ROM `0x4E1E0`–`0x4EA60`
is deliberately outside `main/font`: it is the PI-manager/rzip prefix of the
inflate subsystem, immediately followed by `main/gzip_asm` at `0x4EA60`.

### 3.4 The resident shadows and lights TUs

ROM `0x18FF0`–`0x1AE60` contains two source units followed by the already
measured `main/lights2` hand-written assembly object. The boundary is ROM
`0x19310` (VRAM `0x80018710`), a 16-byte-aligned function boundary. JFG's US
layout puts `shadows_214A0.c` immediately before `lights.c`; Mickey reproduces
the same function order, with the exact `shadowMakeYs` body ending at this
boundary and the allocation/free call graph of `lights.c` beginning there.
The far end is fixed independently by the whole-object tier-A `main/lights2`
match at `0x1AE60`. This is tier B boundary evidence, not a claim that either
C object is whole-object-identical to JFG's.

PROVENANCE DISCLOSURE. The comparison names below come from JFG's public
decomp, `src/shadows_214A0.c`, `src/lights.c`, `src/lights.h`, and its built
objects. JFG is a permitted published retail-derived decomp under
`docs/CLEANROOM.md`. A name marked "comparison" is navigation evidence only:
it is not adopted into `symbol_addrs.us.txt` until it earns one of §1's name
tiers and its C is no longer parked behind `GLOBAL_ASM` (§1.5).

The four shadows functions contain all 88 odd-FP operands in this range and
remain `main/shadows` assembly under §6.2. The 28-function `main/lights` TU
contains none and is split to C with assembly fallbacks. The complete census:

| Mickey VRAM | Size | JFG namesake | Evidence / disposition |
|---:|---:|---|---|
| `0x800183F0` | `0xC4` | `shadowBoxPolyOverlap` | Tier A: 49/49 unmasked words, ROM-wide unique; already adopted |
| `0x800184B4` | `0x90` | `shadowBoundingBox` | Tier A: 36/36 unmasked words, ROM-wide unique; already adopted |
| `0x80018544` | `0x110` | `shadowYHeight` | comparison only: unique nearest 4-gram skeleton, 0.919; remains `func_80018544` |
| `0x80018654` | `0xBC` | `shadowMakeYs` | Tier-A candidate: 47/47 unmasked words, ROM-wide unique; assembly pending a function-sized naming commit |
| `0x80018710` | `0x8C` | `freeLights` | tier-B comparison: three frees and the JFG TU position; C still `func_80018710` |
| `0x8001879C` | `0x130` | `setupLights` | tier-B comparison: calls the preceding free, three allocators and `lightCreateLightTable`; C still `func_8001879C` |
| `0x800188CC` | `0xB0` | JFG placeholder `func_80020D94` | placeholder names are prohibited by §1.5; remains `func_800188CC` |
| `0x8001897C` | `0x238` | `addRomdefLight` | tier-B comparison from TU order and light-update callees; C still `func_8001897C` |
| `0x80018BB4` | `0x200` | `addObjectLight` | tier-B comparison from TU order and light-update callees; C still `func_80018BB4` |
| `0x80018DB4` | `0x10` | `turnLightOff` | Tier A: adapted JFG body is compiler-exact under canonical flags and linked byte-identically |
| `0x80018DC4` | `0x10` | `turnLightOn` | Tier A: adapted JFG body is compiler-exact under canonical flags and linked byte-identically |
| `0x80018DD4` | `0x10` | `toggleLight` | Tier A: adapted JFG body is compiler-exact under canonical flags and linked byte-identically |
| `0x80018DE4` | `0x2C` | `changeLightColour` | Tier A: adapted JFG body is compiler-exact under canonical flags and linked byte-identically |
| `0x80018E10` | `0x20` | `changeLightColourCycle` | Tier A: 7 unmasked of 8 words, ROM-wide unique; adapted C is linked byte-identically and adopted |
| `0x80018E30` | `0x4C` | `changeLightIntensity` | Tier A: adapted JFG body is compiler-exact under canonical flags and linked byte-identically |
| `0x80018E7C` | `0x8C` | `lightUpdateLights` | tier-B comparison: loop calls the following per-light updater |
| `0x80018F08` | `0x334` | JFG placeholder `func_80021444` | placeholder prohibited; remains `func_80018F08` |
| `0x8001923C` | `0x104` | `killLight` | tier-B comparison from free/update call graph and TU order |
| `0x80019340` | `0x18` | `lightGetLights` | Tier A: adapted JFG body and both global relocations are linked byte-identically |
| `0x80019358` | `0x13C` | `lightGetStrongestEffect` | tier-B comparison: square-root distance calculation and TU order |
| `0x80019494` | `0xA8` | `lightUpdateObjects` | tier-B comparison: calls the following object-light helper |
| `0x8001953C` | `0x3F8` | JFG placeholder `func_80021B9C` | placeholder prohibited; remains `func_8001953C` |
| `0x80019934` | `0xF0` | `lightDistanceCalc` | tier-B comparison: same distance-mode call surface |
| `0x80019A24` | `0x94` | `lightDirectionCalc` | unique nearest skeleton (0.432) and exact JFG size; comparison only |
| `0x80019AB8` | `0x2E0` | `lightObject` | tier-B comparison: calls all three `lights2` pipelines |
| `0x80019D98` | `0x50` | `lightDefaultObjectLight` | tier-B comparison: delegates to the following setter |
| `0x80019DE8` | `0xFC` | `lightSetObjectLight` | unique nearest skeleton (0.704) and transform call; comparison only |
| `0x80019EE4` | `0x98` | `lightSetupLightSources` | tier-B comparison: loop calls the adopted `addObjectLight` comparison |
| `0x80019F7C` | `0x8C` | `lightSetupFlareSources` | tier-B comparison: adjacent setup loop and flare helper |
| `0x8001A008` | `0x14C` | `lightInitObjectLighting` | tier-B comparison: calls the object-light setter twice |
| `0x8001A154` | `0xE8` | `lightAdjustGlowingLight` | tier-B comparison: paired flare helper and TU order |
| `0x8001A23C` | `0x24` | `lightKillGlowingLight` | tier-B comparison: calls the paired delete helper and returns success |
### 3.4 The resident allocator (`main/memory`)

ROM `0x2BCD0`–`0x2C8C0`, VRAM `0x8002B0D0`–`0x8002BCC0`, `0xBF0` bytes.
This boundary is measured from the linked pre-split ELF: `2BCD0.s.o` owns one
`0xBF0`-byte text section, its last function ends at `0x2C8B4`, and its final
12 bytes are alignment before `2C8C0.s.o` begins. This corrects the earlier
provisional task range ending at `0x2C950`, which crosses into the next object.
The TU has no floating-point instructions and no string references.

**PROVENANCE:** the correspondence names below were read from Jet Force
Gemini's published `src/memory.c`, `src/memory.h`, built `memory.c.o`, and
public symbol map. The two exact skeleton hits are tier A; the remaining JFG
correspondences are tier B call-graph arguments and stay beside their Mickey
`func_` symbols until matched C justifies adoption under §1.5. JFG lacks
Mickey's 8-byte alignment helper and has a trailing `mmSlotPrint` routine that
Mickey lacks. No distinctive string is referenced, so there is no tier C row.

| ROM | Mickey symbol | JFG correspondence | Tier and evidence |
|---|---|---|---|
| `0x2BCD0` | `mmInit` | `mmInit` | A: unique 30-word skeleton; 14 relocated words masked |
| `0x2BD48` | `mmExtended` | `mmExtended` | B: returns the expansion-memory flag consumed by `mmInit`; matched C exact |
| `0x2BD54` | `func_8002B154` | `mmAllocRegion` | B: allocates slot storage, then calls the pool initializer with it |
| `0x2BDA0` | `func_8002B1A0` | `mempool_init` | B: shared callee of `mmInit` and the region allocator; initializes the 0x10-byte pool and 0x14-byte slot records |
| `0x2BE80` | `func_8002B280` | `mmAlloc` | B: main-pool wrapper that derives a caller colour tag and calls the slot finder |
| `0x2BF14` | `func_8002B314` | `mmAlloc2` | B: second wrapper with the same calls and result role |
| `0x2BFA8` | `func_8002B3A8` | `mempool_slot_find` | B: common worker used by all three allocation wrappers and the fixed-address allocator |
| `0x2C0C0` | `func_8002B4C0` | `mmAllocR` | B: selects a pool by its slot-array pointer, then calls the common worker |
| `0x2C124` | `func_8002B524` | `mmAllocAtAddr` | B: fixed-address allocation through up to three slot assignments |
| `0x2C2F4` | `mmSetDelay` | `mmSetDelay` | B: writes the deferred-free delay used by `mmFree`; matched C exact |
| `0x2C300` | `func_8002B700` | `mmFlushFreeStack` | B: drains queued addresses through the address-free worker |
| `0x2C368` | `mmFree` | `mmFree` | A: unique 17-word skeleton; four relocated words masked |
| `0x2C3AC` | `func_8002B7AC` | `mmFreeTick` | B: services the delayed-free queue; Mickey additionally calls `ReleaseUnusedLinkSlots` |
| `0x2C4A8` | `func_8002B8A8` | `mempool_free_addr` | B: finds an address's pool and clears its matching live slot |
| `0x2C53C` | `func_8002B93C` | `mempool_free_queue` | B: appends an address and delay to the deferred-free arrays |
| `0x2C578` | `func_8002B978` | `mempool_get_pool` | B: reverse-searches the pool table for the containing address range |
| `0x2C5D0` | `func_8002B9D0` | `mempool_slot_clear` | B: frees a slot and coalesces adjacent free records |
| `0x2C720` | `mmGetSlotPtr` | `mmGetSlotPtr` | B: returns one pool's slot-array pointer; matched C exact |
| `0x2C734` | `mmGetDelay` | `mmGetDelay` | B: returns the deferred-free delay; matched C exact |
| `0x2C740` | `func_8002BB40` | `mempool_slot_assign` | B: assigns a slot and, where needed, creates and links its remainder |
| `0x2C860` | `align16` | `mmAlign16` | A: existing exact 7-word `memory.c.o` match; JFG corroborates the role |
| `0x2C87C` | `align8` | — | A: existing exact 7-word `memory.c.o` match; no JFG counterpart |
| `0x2C898` | `align4` | `mmAlign4` | A: existing exact 7-word `memory.c.o` match; JFG corroborates the role |

Matched C: `align16` is exact for all `0x1C` bytes and has no relocations.
The canonical `-O2 -mips2 -32` flags reproduce the target; JFG's
`mmAlign16` body is the adapted donor.
`align8` is likewise exact for `0x1C` relocation-free bytes with canonical
flags; it is the Mickey-only member derived from the same alignment family.
`align4` completes the family with seven exact instruction words and no
relocations. Its compiled body is `0x1C` bytes; the flag sweep's only reported
delta is the separate 12-byte TU alignment tail already excluded above.
`mmExtended` is exact for `0xC` bytes with the canonical flags. Its two data
relocations retain the target HI16/LO16 offsets and bind `D_8007A274`; the JFG
body and `mmInit` flag role support the tier B name.
`mmSetDelay` is exact for `0xC` bytes under the same flags. Its target-matching
HI16/LO16 pair binds the deferred-free state at `D_800D21AC`.
`mmGetDelay` is the exact `0xC`-byte getter for that same state, with the
target HI16/LO16 relocation pair and canonical flags.
`mmGetSlotPtr` is exact for `0x14` bytes; its HI16/LO16 pair binds the pool
slot-pointer anchor at `D_800D1C64`. The 0x10-byte stride and neighboring
allocator accesses establish Mickey's 16-bit counts at `+0/+2`, slot pointer
at `+4`, size at `+8`, and free-size field at `+0xC`; these differ from JFG's
starting declaration and are reflected in `include/game/memory.h`.

The `models` block is now the deliberate exception to that earlier scheduling
rule: it has been split as a **working decompilation TU**, not promoted to a
tier-A original-file-boundary claim. The evidence and the distinction are
recorded below.

### 3.4 `main/models` working split

ROM `0x20020`-`0x21DA0`, VRAM `0x8001F420`-`0x800211A0`, 19 functions.
The start is an existing 16-byte-aligned yaml boundary and the first function
is an exact, ROM-wide-unique JFG `src/models.c.o` skeleton match. Two later
functions in the same block are exact matches to that object too. The
intervening call graph stays within model allocation, texture ownership and
matrix generation, and the next boundary is the independently tier-A
`mainproc`/`thread1_main` anchor. That supports a practical source split, but
not the stronger statement that every byte came from one original object.

JFG names below are **correspondences, not adopted Mickey symbols** unless a
later matched-C row says otherwise. Tier A means exact masked-skeleton
identity; tier B means the external calls and role agree; tier D means only
function order and local structure agree. There are no distinctive string
references in this block, so it has no tier-C rows. Reference placeholders are
never imported as names, and uncertain rows retain Mickey's `func_` spelling.

| ROM | Mickey symbol / size | JFG correspondence | Tier and evidence |
|---|---|---|---|
| `0x20020` | `func_8001F420`, `0x3C` | JFG placeholder in `models.c.o` | A: exact 15-word skeleton and linked C match; placeholder retained |
| `0x2005C` | `modInitModels`, `0xC4` | `modInitModels` | B: same allocation/table-initialisation calls and TU position; linked C match |
| `0x20120` | `func_8001F520`, `0x644` | `modLoadModel` | B: same cache, decompression, texture and instance-helper call graph |
| `0x20764` | `func_8001FB64`, `0x68` | JFG placeholder in `models.c.o` | A: exact 26-word skeleton and linked C match; placeholder retained |
| `0x207CC` | `func_8001FBCC`, `0x84` | JFG placeholder helper | D: function order and allocation/copy structure; linked C match |
| `0x20850` | `func_8001FC50`, `0x534` | JFG placeholder helper | D: function order and model-instance construction only |
| `0x20D84` | `modFreeModel`, `0xF4` | `modFreeModel` | B: instance free followed by model-reference/resource release; linked C match |
| `0x20E78` | `func_80020278`, `0x168` | JFG placeholder resource-free helper | B: texture free plus the same family of owned allocations; linked C match |
| `0x20FE0` | `func_800203E0`, `0xD8` | no adoptable name | D: model helper calls only; linked C match, placeholder retained |
| `0x210B8` | `func_800204B8`, `0xAC` | no adoptable name | D: texture/allocation release structure only |
| `0x21164` | `modelSetModelFlags`, `0xC` | `modelSetModelFlags` | B: paired global setter and observed callers; linked C match |
| `0x21170` | `modelGetModelFlags`, `0xC` | `modelGetModelFlags` | B: paired global getter; linked C match |
| `0x2117C` | `func_8002057C`, `0x558` | `makeModelGfx` | B: texture/display-list construction call graph and TU order |
| `0x216D4` | `func_80020AD4`, `0x3C` | JFG placeholder in `models.c.o` | A: exact 15-word skeleton and linked C match; placeholder retained |
| `0x21710` | `func_80020B10`, `0x27C` | JFG placeholder helper | D: adjacent table-builder structure only |
| `0x2198C` | `func_80020D8C`, `0xC0` | `modSetTextureFrame` | B: model texture-frame traversal and matching TU position |
| `0x21A4C` | `func_80020E4C`, `0x1C4` | `modSuspendModelTextures` | B: allocate/save/free texture ownership sequence |
| `0x21C10` | `modResumeModelTextures`, `0x8C` | `modResumeModelTextures` | B: reload/free saved texture ownership sequence; linked C match |
| `0x21C9C` | `func_8002109C`, `0x104` | no adoptable name | D: model limb/matrix traversal; JFG candidates diverge |

**PROVENANCE.** JFG's public `src/models.c`, its built `src/models.c.o`, its
`asm/nonmatchings/models/` filenames, and its published symbol map supplied
the correspondence vocabulary above. The three tier-A rows are measurements
against Mickey's ROM; every other row is explicitly an argument. No JFG body
is present in the initial all-`GLOBAL_ASM` split.
**Why most rows have no new `mickey.us.yaml` split.** §1's "measured file
boundary" tier requires a whole-`.text` match; this pass only matched
**Why the original scan added no `mickey.us.yaml` splits.** §1's "measured
file boundary" tier requires a whole-`.text` match; this pass only matched
individual functions (`tools/find_known_objects.py --sections` found no
whole-object match for any of the not-yet-named TUs above). Asserting a yaml
`asm`/`c` split from function-level hits alone would claim more than was
measured, exactly the mistake 1.2's uniqueness clause exists to prevent one
level up. `gameVi` is the exception added later: §3.4 supplies independent
tier-B boundary evidence from its complete ordered function and call surface.
The already-measured TUs above (`n_csplayer`, `gsSnd`, `n_drvrNew`, `n_env`,
`n_load`, `math_util`) needed no new split; they already have one.

### 3.4 `gameVi`: ROM `0x34180`–`0x34E60`

This is the resident video-interface and framebuffer translation unit,
`src/main/gameVi.c`. **PROVENANCE:** the TU and function names used as matching
candidates come from Jet Force Gemini's public decompilation,
`src/gameVi.c`/`src/gameVi.h`; Mickey's ROM decides every boundary, body and
verdict.

The boundary is tier B rather than a whole-object tier-A claim. Mickey has the
same complete ordered sequence of 23 functions as JFG, from the video
initialiser through the byte-copy leaf. Their call and global-access roles
agree: the first routine creates the video message queue and scheduler client,
the mode routine owns buffer allocation and timing, and the last routine is
`fb_memcpy`. The preceding range is the independently measured
`trapDanglingJump` TU, while ROM `0x34E60` starts the `texInitTextures`-shaped
function and the following `textures.c` sequence. Both ends are 16-byte
aligned.

Four landmarks inside the TU are independently tier A in
`symbol_addrs.us.txt`: `viSetWideAdjust`, `viDisplayingScreen0`, the
placeholder-retaining `func_80034018`, and `fb_memcpy`. Unmatched functions
retain their Mickey `func_` labels as §1.5 requires; a JFG counterpart is not
promoted merely because it occupies the same position in the sequence.

`fb_memcpy` is now canonical C, adapted from JFG with a point-of-use
PROVENANCE note. IDO 5.3 under the resident `-O2 -mips2 -32` flags emits all
12 instruction words exactly, with no relocations; the linked range and full
ROM are byte-identical.

`viDisplayingScreen0` is also canonical C under the same flags and provenance:
all 11 instruction words and the four HI16/LO16 relocation records to Mickey's
framebuffer globals are exact. Its linked range and the full ROM are
byte-identical.

`viSetWideAdjust` is canonical C as well. The adapted clamp/store/timing-call
body emits all 16 instruction words and its HI16, LO16 and call relocations
exactly under the resident flags.

The placeholder-retaining `func_80034018` is canonical C. JFG's public decomp
provides the framebuffer-fill body but not a descriptive function name, so
§1.5 keeps Mickey's address label. All 31 instruction words and its two
global-address plus cache-flush-call relocations are exact.

`viGetVideoMode` is adopted at tier B after its five-word accessor body became
canonical C. Eleven same-address Mickey callers use the returned low mode bits
to choose display dimensions or compare them with a requested mode before
calling the mode-change routine; this is the exact role of JFG's same-position
function. Its HI16/LO16 relocation pair and linked bytes are exact.

`viGetWideAdjust` is adopted at tier B with its three-word canonical accessor:
`frontSetWideAdjust` calls the already tier-A setter, immediately reads this
value back, and stores it as the front-end's current setting. The getter's
HI16/LO16 relocation pair and linked bytes are exact. The public declarations
for the matched named surface now live in `include/game/gameVi.h`.

`viSetTrippleBuffer` is adopted at tier B with JFG's original spelling. The
front-end passes a requested resolution mode, then reads the current video mode
and tests whether the buffer configuration changed before calling the mode
changer. The four-word setter and its HI16/LO16 relocation pair are exact.

`viChangeBuffers` is the seven-word predicate used by that same caller. It
compares the active and requested triple-buffer flags, and the caller invokes
the mode changer exactly when it returns true. That pins the JFG name at tier B;
both HI16/LO16 relocation pairs and the linked body are exact.

`viFrameRateReset` is adopted at tier B. The mode changer and two runtime
state-reset paths call it before frame pacing resumes, while the canonical body
resets the skip-adjust flag, delta counter, delta interval and one-frame mode.
All 11 instruction words and four HI16/LO16 relocation pairs are exact.

`func_80033D58` is canonical C: all seven words and the two scale globals'
HI16/LO16 relocation pairs are exact. JFG calls the equivalent body
`viGetScaleXY`, but only three words are unmasked and no same-address Mickey
caller pins the role, so the public name is recorded only in the source comment
and not adopted.

`func_80033FB8` is canonical C at three words with an exact HI16/LO16 pair.
JFG calls the equivalent accessor `viGetTrippleBuffer`, but no same-address
Mickey caller pins that public name and the body is below the tier-A threshold,
so it remains an address label.

`func_80033FE0` is likewise canonical C at three words with an exact
HI16/LO16 pair. JFG calls the store-only helper `viNoClear`, but no
same-address Mickey caller pins that public name and the body is below the
tier-A threshold, so the address label remains canonical.

### 3.4 `main/track`: ROM `0xC950`-`0x16140`

This 0x97F0-byte, 66-function block is one resident translation unit. The
identification is stronger than the isolated `trackSetFogOff` row in §3.3:

- **Tier A:** `trackSetFogOff` at ROM `0x151A0` is byte-identical to JFG's
  function, with 27 unmasked words, two masked words, and one ROM occurrence.
- **Tier B:** callers use the block as one track-rendering, collision-query,
  lighting, and fog API. Internal calls stay within those same clusters; the
  block's first large routine orchestrates its later helpers.
- **Tier C:** the routines at `0x8000BDB4` and `0x8000E920` reference all 14
  resident copies of `"track/track.c"` at `0x80081540`-`0x80081610`.
- **Tier D:** the complete function order follows JFG's built `src/track.c.o`:
  update/draw/sky, texture scrolling, track lights, spatial queries, and fog,
  ending with the corresponding display-list helper. Running
  `tools/skeleton_scan.py similar --target <vram> --top 5` for every Mickey
  function puts a JFG `track.c.o` member first for 39 of the 62 functions large
  enough for the default ten-word index, and in the top five for 40. Exact
  sizes drift, as expected for a different engine revision, but the order does
  not.

The 16-byte-aligned yaml boundaries agree with that sequence: ROM `0xC950`
starts at the function corresponding to JFG's first track routine, and the
last Mickey function ends at `0x16134`, leaving only 12 bytes of compiler
alignment before the next subsegment. No routine in the block uses an odd
single-precision FP register, so none is classified as hand-written assembly
under §6.2.

**PROVENANCE:** the TU name, comparison order, and reference function names
come from Jet Force Gemini's public decomp, `src/track.c` and its built
`src/track.c.o`, a permitted published retail-derived source under
`docs/CLEANROOM.md`. Mickey's ROM supplies the boundaries, call graph, string
references, and matching verdicts; JFG is a starting point, never authority
over a disagreement.

Matched C in this TU:

| Function | ROM | Bytes | Flags | Donor and verdict |
|---|---:|---:|---|---|
| `trackSkySet` | `0xD1E8` | 0xC | `-O2 -mips2 -32` | JFG `src/track.c` body; tier B role and tier D TU position; 3/3 instruction words and relocation layout exact, linked ROM exact |
| `func_8000D00C` | `0xDC0C` | 0xC | `-O2 -mips2 -32` | Mickey reconstruction; JFG's corresponding `trackGetSky` is only tier D and is deliberately not adopted; 3/3 instruction words and relocation layout exact, linked ROM exact |
| `trackGetTrack` | `0x14AB4` | 0xC | `-O2 -mips2 -32` | Mickey reconstruction with JFG name (tier B callers); 3/3 instruction words and relocation layout exact, linked ROM exact |
| `trackSetFog` | `0x15030` | 0xF8 | `-O2 -mips2 -32` | JFG `src/track.c` body with tier B callers and tier D TU order; 62/62 instruction words and relocation layout exact, linked ROM exact |
| `trackGetFog` | `0x15128` | 0x78 | `-O2 -mips2 -32` | JFG direct-path body with tier B caller and tier D TU order; 30/30 instruction words and relocation layout exact, linked ROM exact |
| `trackSetFogOff` | `0x151A0` | 0x74 | `-O2 -mips2 -32` | JFG `src/track.c`; 29/29 instruction words and relocation layout exact, linked ROM exact |
| `func_80014614` | `0x15214` | 0x190 | `-O2 -mips2 -32` | Mickey reconstruction of the fog-state updater; JFG same-position skeleton is the 0.733 top hit but its placeholder is not imported; 100/100 instruction words and relocation layout exact, linked ROM exact |
| `func_800147A4` | `0x153A4` | 0x13C | `-O2 -mips2 -32` | Mickey reconstruction using the SDK fog-colour/position macros; JFG same-size top skeleton supplies structural context but its placeholder is not imported; 79/79 instruction words and relocation layout exact, linked ROM exact |
| `func_80014DE4` | `0x159E4` | 0xC8 | `-O2 -mips2 -32` | Mickey reconstruction; JFG supplies only tier-D transform-role context and no public name is adopted; 50/50 instruction words and relocation layout exact, linked ROM exact |
| `func_80014EAC` | `0x15AAC` | 0x20 | `-O2 -mips2 -32` | JFG `func_8001C550` is a tier-A 8/8-word TU donor, unique in the ROM; JFG placeholder not imported; linked ROM exact |

### 3.4 Resident camera: ROM `0x21EE0`–`0x25C20`

This whole `0x3D40`-byte block is the resident camera TU: **69 functions,
`0x3D3C` executable bytes and four bytes of terminal alignment**. Its ordered
systems are camera/FOV state, user viewports, projection setup, sprite and
model matrices, projection helpers, then screen shake. The split is
`main/camera`; flags are the resident default `-O2 -mips2 -32`.

**PROVENANCE.** The TU identity, source order and borrowed names below come
from Jet Force Gemini's public retail-derived decomp, `src/camera.c`, permitted
by `docs/CLEANROOM.md`. JFG is a starting point only; the tiers say which parts
Mickey's own bytes establish.

| Evidence | Result |
|---|---|
| **A — byte identity** | `camSetWaterLine` at ROM `0x225B0` has 6 unmasked words of 8 and `romocc=1`; `camGetPlayerProjMtx` at `0x23360` has 8 unmasked words of 13 and `romocc=1`. Both clear §1.2. |
| **B — role/call graph** | `camInit` opens the block and ranks JFG `camInit` at 0.3125 masked 4-gram Jaccard versus 0.0851 for the runner-up. `camOverrideProjScales`, `camGetProjOrgMtx`, `camStopShakes`, and `camSetZoom` have the same state effects and ordered camera roles as JFG; their comments in `symbol_addrs.us.txt` retain why they are below tier A. |
| **C — strings** | None. The resident strings `"Camera Error: Illegal mode!"` and `"Cam do 2D sprite called with NULL pointer!"` are not addressed by resident code (§7); this identification does not use them. |
| **D — structure** | The existing endpoints are 16-byte aligned, the first function is the `camInit` nearest neighbour, and the last function ends four bytes before `0x25C20`. The 69-function call/data sequence follows one camera-state cluster throughout. |

The call census finds 15 direct internal edges. Representative chains are
the view setup calling the window-limit, projection and viewport routines;
the reset path calling scissor and viewport setup; the sprite helpers sharing
matrix conversion; and the shake updater calling the shake initializer.
External callers span resident render, track, menu and update code, while the
TU calls the matrix library, video helpers, `sqrtf`, and `Arctanf`. There are
no odd single-precision FP registers anywhere in the block, so none of these
functions is classified as handwritten assembly under §6.2.

| Matched C function | ROM | Exact executable bytes | Proof |
|---|---:|---:|---|
| `camUseShake` | `0x22084` | 16 | Configured object, relocation pair, linked range and full ROM exact. |
| `camOverrideProjScales` | `0x220E4` | 32 | Configured object, six relocations, linked range and full ROM exact. |
| `camSetWaterLine` | `0x225B0` | 32 | Configured object, relocation pair, linked range and full ROM exact. |
| `camGetProjOrgMtx` | `0x25270` | 28 | Configured object, two relocation pairs, linked range and full ROM exact. |
| `camSetZoom` | `0x258C8` | 56 | Configured object, two relocation pairs, linked range and full ROM exact. |
| `camGetPlayerProjMtx` | `0x23360` | 52 | Configured object, five relocations, linked range and full ROM exact. |
| `camStopShakes` | `0x25754` | 76 | Configured object, three relocation pairs, linked range and full ROM exact. |
| `camIgnoreShake` | `0x22094` | 12 | Configured object, relocation pair, linked range and full ROM exact. |
| `camGetFOV` | `0x220A0` | 12 | Configured object, relocation pair, linked range and full ROM exact. |
| `camGetWaterLine` | `0x225A0` | 16 | Configured object, relocation pair, linked range and full ROM exact. |
| `camGetMode` | `0x22518` | 12 | Configured object, relocation pair, linked range and full ROM exact. |
| `camSetMode` | `0x22524` | 64 | Configured object, two relocation pairs, linked range and full ROM exact. |
| `camGetNo` | `0x22564` | 12 | Configured object, relocation pair, linked range and full ROM exact. |
| `camSetNo` | `0x22594` | 12 | Configured object, relocation pair, linked range and full ROM exact; Mickey omits JFG's bounds guard. |
level up. The already-measured TUs above (`n_csplayer`, `gsSnd`, `n_drvrNew`,
`n_env`, `n_load`, `math_util`) needed no new split; they already have one.
The later menu census below adds independent boundary evidence rather than
retroactively treating the six hits as a whole-object match.

### 3.4 Resident front-end menu: ROM `0x39350`–`0x3B1A0`

This range is `main/menu`, corresponding to JFG's `src/menu.c`. The identity
uses permitted JFG material and is disclosed here: names, declarations,
function order, and starting bodies are compared against JFG's public
decompilation; Mickey's ROM remains authoritative for every match.

The six exact masked-skeleton anchors in §3.3 are **tier A** evidence for the
TU identity, but not its boundaries. The boundaries are a separate **tier B/D**
argument from the full function census. At ROM `0x39350`, the code begins a
sequence structurally corresponding to JFG's `setLanguage`, `initFront`,
`frontFreeMode`, `frontInitMode`, and `frontSetMode`; the same order continues
through the six tier-A anchors and the settings accessor family. The last menu
routine is the short setter at `0x3B190`, in JFG's
`frontCharSelectSetQuitMode` position. The next function, at aligned ROM
`0x3B1A0`, searches the table associated with the distinctive `"UNKNOWN
TRACK"` string and begins a different subsystem. The preceding aligned
function start at `0x39350` likewise follows texture/screen code whose JFG
ordering is outside `menu.c`. Thus the split claims only `0x39350`–`0x3B1A0`,
not the surrounding yaml block.

The source began as 41 `GLOBAL_ASM` functions. Six already have tier-A names
in `symbol_addrs.us.txt`; other JFG names remain a navigation crosswalk until
an exact body is promoted, so the unresolved symbols keep their `func_` names
per §1.5. Flags are the resident game-code defaults: `-O2 -mips2 -32`.

`frontSetWideAdjust` is the first exact C promotion: **0x2C bytes / 11 words**
at ROM `0x3AFDC`, with the target's four relocation-bearing words resolving
at their real linked addresses. Its body is adapted from JFG's public
`src/menu.c` and carries the required point-of-use `PROVENANCE` note. A flag
sweep confirmed that the default `-O2 -mips2 -32` spelling is exact; no
per-file override or post-compile instruction edit is involved.

`frontGetWideAdjust` adds **0xC bytes / 3 words** at ROM `0x3AFD0`. The name
is explicitly **tier B**, not tier A: the body is too short for the standalone
skeleton threshold, but its exact byte-return of the setter's stored state and
its position immediately before `frontSetWideAdjust` establish the same role
as JFG's ordered pair. The adapted body has a point-of-use `PROVENANCE` note,
and the default flags are byte-exact in the flag lattice.

The tier-A-named `frontGetScreenMode` adds **0x30 bytes / 12 words** at ROM
`0x3AE98`. Mickey's draft established the two tests; JFG's published
`Resbitfield` declaration supplied the original source shape needed to recover
the compiler's temporary-register order. Mickey has two adjacent mode bits,
confirmed by the paired writes in the following setter. The adapted type has a
point-of-use `PROVENANCE` note, and the default flags, object words, and linked
ROM range are exact without post-processing.

The tier-A-named `frontGetLevelScreenMode` adds **0x68 bytes / 26 words** at
ROM `0x3AF68`. Its JFG source body is still `GLOBAL_ASM`, so the C body was
derived from Mickey's own draft and control flow rather than borrowed. The
four cases return fixed mode 1, level mode with bit 1 set, fixed mode 3, or the
current level mode. The canonical flags, two call relocations, object words,
and linked ROM range are exact without post-processing.

The tier-B-named `frontStoreScreenMode` adds **0x14 bytes / 5 words** at ROM
`0x3AF48`. Its copied-byte store, its position in the ordered screen-mode
accessor family, and the matching JFG source body establish the role; the body
therefore carries a point-of-use `PROVENANCE` note. The default flags and both
global-data relocations are exact without post-processing.

The adjacent tier-B `frontRecallScreenMode` adds **0xC bytes / 3 words** at
ROM `0x3AF5C`. Its byte return reads the state written by
`frontStoreScreenMode`, reproducing JFG's ordered accessor pair. The adapted
body carries a point-of-use `PROVENANCE` note; the default flags and data
relocations are exact without post-processing.

The tier-B `frontGetSfxVolume` adds **0xC bytes / 3 words** at ROM `0x3B07C`.
The halfword getter's ordered JFG position and its adjacent setter's call to
`gsSndpSetGlobalVolume` identify the state as the SFX volume. The adapted JFG
body has a point-of-use `PROVENANCE` note; default flags and the linked global
relocation are exact without post-processing. A zero-byte weak alias retains
the anonymous spelling still referenced by resident assembly.

The paired tier-B `frontSetSfxVolume` adds **0x3C bytes / 15 words** at ROM
`0x3B088`. JFG's body accounts for both bounds clamps, the halfword store, and
the `gsSndpSetGlobalVolume` call, so the adapted body carries point-of-use
`PROVENANCE`. The canonical flags, call and data relocations, object words, and
linked ROM range are exact; a zero-byte weak alias preserves the anonymous
name used by the remaining assembly caller.

The tier-B `frontGetBgmVolume` adds **0xC bytes / 3 words** at ROM `0x3B0C4`.
Its halfword getter follows the completed SFX pair at the exact JFG menu
position, while the adjacent clamp-and-audio-call setter confirms the paired
BGM state. The adapted body carries point-of-use `PROVENANCE`; its linked data
relocation is exact, and a zero-byte weak alias preserves the anonymous name
used by resident assembly.

The paired tier-B `frontSetBgmVolume` adds **0x3C bytes / 15 words** at ROM
`0x3B0D0`. The JFG body exactly accounts for Mickey's two bounds clamps,
halfword store, and corresponding audio-volume call, and carries point-of-use
`PROVENANCE`. Canonical flags, call and data relocations, object words, and the
linked ROM range are exact; a zero-byte weak alias preserves its assembly
caller's anonymous spelling.

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

31 of the 45 `n_audio` TUs are matched (`n_cspsetvol`, `cents2ratio`
adopted before this pass; 29 more from it): every masked=0/1/2 TU (the
thin `N_ALEvent` posters and one-line accessors), `n_sl` (which places
the driver singletons `n_alGlobals`/`n_syn` — VRAM `0x80080160`/
`0x80080164`, measured directly off a built candidate diffed against the
ROM with `tools/wb_compare.sh --rom n_alInit --show-diff` and carved out
of the resident `.data` band in `mickey.us.yaml`, since almost everything
else in the library reads `n_syn`), the seven `ALParam`-update setters
that funnel through it, `n_synallocfx`, `n_alcspchan` (needs
`-DRAREDIFFS` for Rare's added MIDI control-change codes), and
`n_syngetfxref`. The `0x3220`-byte `n_csplayer` text and its owned
`0x200`-byte `.data`, `0x300`-byte `.rodata`, and `0x40`-byte `.bss`
also match exactly from the JFG source; its multiply sequences require
`-Wab,-r4300_mul` in addition to `-DRAREDIFFS`. JFG's main compressed-sequence
source (`n_csq.c`) also supplies the exact `0x9D0`-byte `n_cseq` text with the
bare flag group and no owned data or rodata. The JFG `n_reverb.c`/`n_save.c`
pair supplies Mickey's combined `0x16B0`-byte reverb TU plus its `0x30` bytes
of rodata; it requires `-DN_MICRO -Wab,-r4300_mul`. The 15-function,
`0x1160`-byte `n_seqplayer` TU matches the JFG source under the bare flag group.
JFG's `N_MICRO` resampler path supplies the exact `0x2A0`-byte `n_resample`
text and its `0x10` bytes of rodata under the same bare flag group.
The same JFG flags reproduce all `0x100` text bytes and the six relocations of
`n_synsetfxparam`; assigning its `0x10`-byte literal section to ROM `0x853B0`
also resolves the recorded `0.1f` rodata-offset plateau.

Remaining unmatched, roughly by size: `n_synthesizer` (masked=173,
`0xAD0`), `n_env` (masked=59), `alsurround`
(masked=39), `n_event`/`n_drvrNew` (masked=34 each), `n_synaddplayer`
(masked=24), `n_mainbus`/`n_synallocvoice` (masked=22/23), `n_alLPFilter`
(masked=13), `n_auxbus` (masked=7),
`n_load` (masked=4, DSP-heavy ADPCM decoder), and `n_synsetvol`/
`n_synstartvoiceparam`/`n_synallocvoice` (masked=5) not yet attempted.

---

## 5. The overlay system

The overlay ledger (module layout, per-overlay ownership and evidence, the
donor scan, and the retired normalization notes) moved to
[`docs/overlays.md`](overlays.md) on 2026-08-24 when this file crossed the
256 KB tracked-file limit. Section numbering there continues as 5.x.

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

**The BSS recipe: name a fixed address inside an anonymous gap without
carving the gap.** `.data`/`.bss` are still one unsplit whole-program
subsegment (the anonymous `bss_gap_*` entries in `mickey.us.yaml`), so
giving a TU's global its own `.bss` in that TU is not available yet -- that
route makes the linker place the object by link order, which is not the ROM's
order, and shifts every fixed BSS symbol after it (measured on
`lane/libultra-rest`, `docs/CLEANROOM.md`'s deleted-history incident aside,
the actual failure there was two unmatched Transfer Pak functions'
register-save order and callee addresses drifting by 0x30, the signature of a
BSS symbol moving under a live reference). The working recipe, already in use
for `__osPfsPifRam`/`__osContLastCmd`/`__osMaxControllers` and the PI-access
queue (`__osPiAccessQueue`, `piAccessBuf`) since Phase 2:

1. Give the object a plain, untyped `NAME = 0xVRAM;` line in
   `symbol_addrs.us.txt`, inside the existing gap comment block, with a
   comment deriving the address from a masked %hi/%lo pair in an
   already-boundary-matched whole-`.text` function (Tier A/B evidence) --
   never guessed from struct layout alone. A typed `size:` annotation is
   rejected by splat for anything but its fixed vocabulary
   (`func`/`u8`/`u32`/... or a custom type starting with a capital letter);
   for a struct like `OSPifRam` the plain untyped form is what the existing
   entries already use, so follow that, not `type:object`.
2. Declare it `extern` in the shared header (`PRinternal/controller.h`
   already does, for every object in this gap) and never `#define` it as a
   real global in any `.c` file. The C body reads exactly like the SDK
   source; only the storage-class keyword differs, and that line carries a
   `DEVIATION FROM THE REFERENCE` comment saying so (see `piacs.c`,
   `controller.c`). Because nothing defines it, no object file's `.bss`
   claims space for it, so its final address depends on nothing but the
   linker-script assignment splat emits from `symbol_addrs.us.txt` --
   link order is irrelevant. This was verified directly: adding
   `__osContPifRam = 0x800D80F0;` alone (no source change) rebuilds the ROM
   byte-identically, and adding the two functions that reference it on top
   does too once they are.
3. The anonymous `bss` gap subsegment in `mickey.us.yaml` is untouched --
   BSS has no ROM bytes backing it, so the yaml's gap boundaries only affect
   generated disassembly labels, never the linked address of a
   `symbol_addrs`-fixed name. No yaml edit is needed to add a name inside an
   existing gap.
4. **Function order inside the `.c` file must match ROM order, not
   convenience.** A `#pragma GLOBAL_ASM` TU with multiple functions links
   its object's contents in file order; reordering `osContInit`'s
   `#pragma GLOBAL_ASM` after two newly-decompiled functions in the same
   file (tried and reverted while landing `__osContGetInitData`/
   `__osPackRequestData`) makes the linker place all three at addresses
   that no longer match `symbol_addrs.us.txt`, producing a corridor-wide
   byte mismatch that looks exactly like a BSS-layout bug (every relocation
   into the reordered range moves) but is a `.text` file-order bug instead.
   Keep declaration order == ROM address order in every partially-scaffolded
   TU.

Worked example: `__osContPifRam` (`OSPifRam`, 0x40 bytes) sits at
`0x800D80F0` inside `bss_gap_D800D80F0` (`0x800D80F0`-`0x800D8180`), reached
by `__osContGetInitData`/`__osPackRequestData`'s masked %hi/%lo pairs
(ROM `0x6F8E8`/`0x6F9B8`). `0x800D80F0 + 0x40 == 0x800D8130`, the already-fixed
address of `__osContLastCmd`, with no slack -- corroborating the size from a
second, independent direction. The remaining `0x800D8132`-`0x800D8180` stays
an anonymous, unnamed part of the same gap; nothing currently reaches it by
relocation, so nothing is claimed there.

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
- **rodata order follows text order exactly.** The 44 jump tables still emitted
  in `asm/` are monotonic in both columns, with **zero inversions**. So
  `.rodata` can be carved TU by TU in text order, which is what makes the
  per-TU split tractable. Five more tables now belong to matched `n_csplayer`
  C, and one belongs to matched `n_reverb` C.

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
