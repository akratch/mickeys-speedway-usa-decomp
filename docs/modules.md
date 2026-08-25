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
| `0x25C20`-`0x263F0` | `0x80025020` | `main/joy` | B | Controller setup, polling, mapping, accessors and CIC helper; §3.4 |
| `0x263F0`-`0x27760` | `0x800257F0` | `main/level` | B | Level lifecycle and metadata accessors; §3.4 |
| `0x27760`-`0x2A250` | `0x80026B60` | `main/main` | B + C | Main state/frame control, identified by call graph and six file-string references; §3.4 |
| `0x27BB4`, `0x28BB8` | — | `"main/main.c"` asserts | — | **`main` code is resident** |
| `0x29FD0` | `0x800293D0` | `"x = %5d"` … `"a = %3.1f"` | — | On-screen coordinate readout |
| `0x2A250`–`0x2AE44` | `0x80029650` | 11 named `math_util.s` routines | A | Matrix / vector / RNG library. 13 routines matched: 11 named here, `rand_range` already carried as `mathRnd`, and `func_80070058` left unnamed as a placeholder |
| `0x2B650`–`0x2BCD0` | `0x8002AA50` | `main/matrix` | — | The parked float TU (§6.2) |
| `0x2C860` | `0x8002BC60` | `align16`/`align8`/`align4` | A | The allocator |
| `0x2C8C0`–`0x2ECA0` | `0x8002BCC0` | `main/saves` | A/B/D | Rumble, EEPROM/save bitstreams, and Controller Pak files (§3.15) |
| `0x2ECA0`–`0x2F0D0` | `0x8002E0A0` | `main/pi` | B | Asset lookup and cartridge DMA (§3.15) |
| `0x2F0D0`–`0x2F400` | `0x8002E4D0` | `main/screen` | B | Compressed screen loading and drawing (§3.15) |
| `0x2F400`–`0x30CD0` | `0x8002E800` | `main/rcpFast3d` | A/B | Fast3D/RCP task and clear helpers (§3.15) |
| `0x30CD0`–`0x323A0` | `0x800300D0` | `main/sched` | A/B/C | The 21-function game scheduler (§3.15) |
| `0x316E8` | `0x80030AE8` | `"SP CRASHED"`, `"Version %s"` | — | The frame loop / RCP watchdog |
| `0x323A0`–`0x323E0` | `0x800317A0` | `main/rsp_segment` | A | Measured file boundary (whole `.text`) |
| `0x323E0`–`0x33FA0` | `0x800317E0` | `main/runlink` | A/B/C | **The runtime overlay linker** (§5) |
| `0x33FA0`–`0x34180` | `0x800333A0` | `main/trapDanglingJump` | A | The overlay call trampoline. **Measured file boundary**: JFG's whole 0x1E0 `hasm/ido/trapDanglingJump.s`. Was named at tier B from Mickey's call graph alone; the bytes agree |
| `0x34180`–`0x34E60` | `0x80033580` | `main/gameVi` | B + A landmarks | **Video and framebuffer management** (§3.8). The complete 23-function order and call/global surface establish the TU boundary; four functions inside are independently tier-A JFG skeleton hits |
| `0x342A8` | `0x800336A8` | `"Ntsc LowRes"` … | — | Video-mode table (15 entries) |
| `0x39A1C` | `0x80038E1C` | `"front/front.c"` asserts | — | **`front` code is partly resident** |
| `0x3B1A0` | `0x8003A5A0` | `"UNKNOWN TRACK"` | — | Track selection |
| `0x3B57C` | `0x8003A97C` | `weather_clip_planes` | A | |
| `0x3D5F0`–`0x43470` | `0x8003C9F0` | `main/particles` | A + B + D | 44-function resident particle TU; §3.16 |
| `0x43470`–`0x45760` | `0x80042870` | `main/diprint` | A + B + C | 19-function formatting/debug-text TU; §3.16 |
| `0x459C0`–`0x467BC` | `0x80044DC0` | `diRcpPrintDL`, `diRcpMoveWd`, `diRcpStrName`, `diRcpOtherMode`, `diRcpGeometryMode` | C | **The display-list disassembler**, a full GBI pretty-printer left in the retail build |
| `0x467BC`–`0x47A60` | `0x80045BBC` | `diCpuReportWatchpoint`, plus the memory/module debug pages and the register-dump crash reporter | C | **The debug monitor**, also left in |
| `0x47A60`–`0x47A70` | `0x80046E60` | `main/get_stack_pointer` | A | Measured file boundary |
| `0x4BC40`–`0x4E1E0` | `0x8004B040` | `main/font` | A/D | JFG's `font.c`: six exact function anchors plus source-order and adjacent-function evidence establish the provisional C split; §3.4 |
| `0x4E378` | `0x8004D778` | `byteswap32` | A | |
| `0x4EA60`–`0x4F4D4` | `0x8004DE60` | `main/gzip_asm` | A | **Measured file boundary**: DKR's whole 0xA74 inflate core, in one piece |
| `0x4FC30`–`0x505E0` | `0x8004F030` | `libultra/exceptasm` | A | **Measured file boundary**, 9 routines including `__osException` and `__osDispatchThread`; §4.2. `0x4FC20` before it is the **rejected** `io/leointerrupt` match, and `0x505E0`–`0x506D0` after it is a separate unknown |
| `0x50820`–`0x50C00` | `0x8004FC20` | `main/refractOutputAssembler` | A | Measured file boundary (JFG) |
| `0x58E50`–`0x59B90` | `0x80058250` | `main/vehicle_sounds` | B + D | Four-function positional racer-sound block. Calls the resident XYZ sound API to maintain engine handles and derives pitch/volume from racer speed and listener distance. No exact JFG skeleton hit; the name is descriptive and the existing splat boundary is not claimed as measured |
| `0x59B90`–`0x59BF0` | `0x80058F90` | `main/osBootRamTest` | A | Measured file boundary (JFG): the IPL3 6105 RAM test |
| `0x5B300`–`0x5C310` | `0x8005A700` | `main/models` | B + D; one A island | Animation-table loading, reference-counted animation storage, frame selection and model-matrix construction establish the descriptive TU name. `camConvertMatrixList` at ROM `0x5B778` alone is byte-identical to JFG `camera.c`; no whole-object identity is claimed |
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
| `src/gameVi.c.o` | 4 | `0x34B68`–`0x34E60` | Inside the now-split `main/gameVi` TU (§3.8). The four exact skeleton hits are landmarks; the boundary is separately established at tier B from the complete ordered function/call surface, not claimed as a whole-`.text` byte match |
| `src/menu.c.o` | 6 | `0x3A184`–`0x3B008` | The automated pass found only interior anchors. A later function-order and call-graph census established the narrower `0x39350`–`0x3B1A0` ownership (§3.11); no whole-`.text` match is claimed |
| `src/gameVi.c.o` | 4 | `0x34B68`–`0x34E60` | Inside yaml's unnamed `0x34180`–`0x37D50` block. No whole-`.text` match; no boundary claimed |
| `src/anim.c.o` | 3 | `0x50D7C`–`0x51D28` | Inside yaml's `main/anim` source-owning block at `0x50C00`–`0x58570`. No whole-`.text` match; the individual hits do not establish an internal boundary |
| `src/models.c.o` | 3 | `0x20020`–`0x21710` | Inside yaml's unnamed `0x20020`–`0x21DA0` block, starting exactly at its boundary. No whole-`.text` match; no boundary claimed |
| `src/font.c.o` | 2 | `0x4BC70`–`0x4C884` | The original >=10-word scan found two anchors. The later complete census in §3.4 found four more exact short functions and split `main/font` provisionally; no whole-`.text` match is claimed |
| `src/audio_manager_4C50.c.o` | 2 | `0x45F0`–`0x4F3C` | Starts exactly at yaml's `0x45F0` boundary; ends inside the unnamed `0x4F40`–`0xC950` block. No whole-`.text` match; no boundary claimed |
| `src/audio_manager_1050.c.o` | 3 | `0x12BC`–`0x22C8` | Inside yaml's unnamed `0x1050`–`0x45F0` block. Wide span for 3 hits -- other code plainly sits between them; no boundary claimed |
| `src/charControl.c.o` | 2 | `0x1CED4`–`0x1FFAC` | Inside yaml's former unnamed `0x1C790`–`0x20020` block. This scan alone claimed no boundary; §3.4 records the later TU split and its additional evidence |
| `src/camera.c.o` | 2 | `0x23360`, `0x5B778` | 230KB apart -- evidently not one placed TU here; treat as two independent identifications, not a span |
| `src/memory.c.o` | 2 | `0x2BCD0`–`0x2C3AC` | Starts exactly at yaml's `0x2BCD0` boundary (end of `main/matrix`); the already-named `align16`/`align8`/`align4` (tier A, `memory.c.o`) sit at `0x2C860`, past this span. Consistent with one TU, no boundary claimed |
| `src/shadows_214A0.c.o` | 2 | `0x18FF0`–`0x19144` | Inside yaml's unnamed `0x18FF0`–`0x1AE60` block, starting exactly at its boundary. No boundary claimed |
| `src/saves.c.o`, `src/rcpFast3d.c.o` | 1 each | single points | These were the Tier A seeds expanded by the later call-graph census in §3.15 |
| `src/track.c.o`, `src/textures.c.o`, `src/diCpu.c.o`, `src/objects.c.o`, `libultra/src/flash/flashreadid.c.o`, `us.v10/src/core1/code_1D00.c.o` (BK) | 1 each | single points | Isolated identifications, no span to claim |

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
| `0x4BCA4` | `0x14` | `func_8004B0A4` | `fontUseFont` | D, matched C | leaf; text-setup callers |
| `0x4BCB8` | `0x24` | `fontColour` | same | A, matched C | leaf; text-setup callers |
| `0x4BCDC` | `0x1C` | `func_8004B0DC` | `fontBackground` | D, matched C | leaf; text-setup callers |
| `0x4BCF8` | `0x44` | `func_8004B0F8` | `fontPrintXY` | B, matched C | calls `0x4BD3C` |
| `0x4BD3C` | `0xA0` | `func_8004B13C` | `fontPrintWindowXY` | B, matched C | calls `0x4BDDC` |
| `0x4BDDC` | `0x8B0` | `func_8004B1DC` | JFG `func_80070518` | D | calls `0x4DF9C`, `0x4C68C`, `0x4D290`, ext |
| `0x4C68C` | `0xB8` | `func_8004BA8C` | `fontStringWidth` | B | calls `0x4DF9C`; ext callers |
| `0x4BCDC` | `0x1C` | `func_8004B0DC` | `fontBackground` | B/D, matched C | leaf; text-setup callers |
| `0x4BCF8` | `0x44` | `func_8004B0F8` | `fontPrintXY` | B/D, matched C | calls `0x4BD3C` |
| `0x4BD3C` | `0xA0` | `func_8004B13C` | `fontPrintWindowXY` | B/D, matched C | calls `0x4BDDC` |
| `0x4BDDC` | `0x8B0` | `func_8004B1DC` | JFG `func_80070518` | D, plateau | calls `0x4DF9C`, `0x4C68C`, `0x4D290`, ext |
| `0x4C68C` | `0xB8` | `func_8004BA8C` | `fontStringWidth` | B/D, plateau | calls `0x4DF9C`; ext callers |
| `0x4C744` | `0x9C` | `func_8004BB44` | `fontWindowSize` | D, matched C | leaf; ext callers |
| `0x4C7E0` | `0x1C` | `func_8004BBE0` | `fontWindowUseFont` | D, matched C | leaf; ext callers |
| `0x4C7FC` | `0x40` | `fontWindowColour` | same | A, matched C | leaf; ext callers |
| `0x4C83C` | `0x48` | `fontWindowFontColour` | same | A, matched C | leaf; ext callers |
| `0x4C884` | `0x40` | `fontWindowFontBackground` | same | A, matched C | leaf; ext callers |
| `0x4C8C4` | `0x2A0` | `func_8004BCC4` | `fontWindowAddStringXY` | B, plateau | calls `0x4D1A4`, `0x4C68C`; ext callers |
| `0x4CB64` | `0x4C` | `func_8004BF64` | `fontWindowFlushStrings` | D, matched C | leaf; ext callers |
| `0x4CBB0` | `0x28` | `func_8004BFB0` | `fontWindowEnable` | D, matched C | leaf; ext callers |
| `0x4CBD8` | `0x28` | `func_8004BFD8` | `fontWindowDisable` | D, matched C | leaf; ext callers |
| `0x4CC00` | `0xC4` | `func_8004C000` | `fontStringAddNumber` | D, matched C | leaf; called by `0x4D1A4` |
| `0x4CCC4` | `0x7C` | `func_8004C0C4` | `fontWindowsDraw` | B | calls `0x4CE00`; ext caller |
| `0x4CD40` | `0xC0` | `func_8004C140` | JFG `func_80071564` | D | ext callee; called by `0x4CE00` |
| `0x4CE00` | `0x3A4` | `func_8004C200` | `fontWindowDraw` | B | calls `0x4CD40`, `0x4D1A4`, `0x4BDDC` |
| `0x4D1A4` | `0xEC` | `func_8004C5A4` | JFG `func_80071A0C` | D, matched C | calls `0x4CC00`; in-range callers |
| `0x4D290` | `0x248` | `func_8004C690` | JFG `func_80071B08` | D | ext callee; called by `0x4BDDC` |
| `0x4D4D8` | `0xA54` | `func_8004C8D8` | `fontCreateDisplayList` | D | ext callee |
| `0x4DF2C` | `0x70` | `func_8004D32C` | no JFG counterpart | D | leaf; ext caller |
| `0x4DF9C` | `0x70` | `func_8004D39C` | `fontConvertString` | D, plateau | leaf; in-range callers |
| `0x4E00C` | `0x1B4` | `func_8004D40C` | `fontGetLine` | D | leaf |
| `0x4CCC4` | `0x7C` | `func_8004C0C4` | `fontWindowsDraw` | B/D, matched C | calls `0x4CE00`; ext caller |
| `0x4CD40` | `0xC0` | `func_8004C140` | DKR `render_fill_rectangle` | B/D, matched C | ext callee; called by `0x4CE00` |
| `0x4CE00` | `0x3A4` | `func_8004C200` | `fontWindowDraw` | B/D, matched C | calls `0x4CD40`, `0x4D1A4`, `0x4BDDC` |
| `0x4D1A4` | `0xEC` | `func_8004C5A4` | JFG `func_80071A0C` | D, matched C | calls `0x4CC00`; in-range callers |
| `0x4D290` | `0x248` | `func_8004C690` | JFG `func_80071B08` | D, plateau | ext callee; called by `0x4BDDC` |
| `0x4D4D8` | `0xA54` | `func_8004C8D8` | `fontCreateDisplayList` | B/D, matched C | ext callee |
| `0x4DF2C` | `0x70` | `func_8004D32C` | no JFG counterpart | D, matched C | leaf; ext caller |
| `0x4DF9C` | `0x70` | `func_8004D39C` | `fontConvertString` | B/D, matched C | leaf; in-range callers |
| `0x4E00C` | `0x1B4` | `func_8004D40C` | `fontGetLine` | D, plateau | leaf |
| `0x4E1C0` | `0x20` | `func_8004D5C0` | `fontYSpacing` | D, matched C | leaf |
| `0x4E1E0` | `0x170` | `func_8004D5E0` | `osCreatePiManager` | D | SDK calls; ext callers |
| `0x4E350` | `0x28` | `func_8004D750` | `rzipInit` | D | allocator call; ext caller |
| `0x4E378` | `0x30` | `byteswap32` | JFG `rzipUncompressSize` | A name collision | leaf; ext callers |
| `0x4E3A8` | `0x38` | `func_8004D7A8` | `rzipUncompressSizeROM` | B | calls `byteswap32`, ext |
| `0x4E3E0` | `0x60` | `func_8004D7E0` | `rzipUncompress` | B | calls `gzip_inflate_block`; ext callers |
| `0x4E440` | `0x620` | `func_8004D840` | `huft_build` | B | calls `_bzero`; called by `main/gzip_asm` |

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

`func_8004B1DC` has a readable DKR-JP-derived candidate under
`NON_MATCHING`. Its best stock-flag build has the target's 128-byte frame and
matches through function offset `+0x2C`, but is 28 instructions short with
broad control-flow divergence after the initial null check. The flag lattice
kept `-O2 -mips2` best; the unresolved issue is source organization and live
ranges across the scissor and glyph loops, not a compiler-flag mismatch.

`func_8004C690` has a readable JFG-derived cache-allocation candidate under
`NON_MATCHING`. Its best stock-flag build has the target's 112-byte frame and
is one instruction longer (147 versus 146); the first mismatch is at function
offset `+0x0`. The candidate keeps the character in saved register `$s0`,
adding a save slot and broadly changing allocation. Struct-copy and explicit
pointer-loop variants did not reproduce the target's four-word header-copy
loop, so the remaining blocker is source shape and live ranges rather than
semantics or constants.

`func_8004D40C` has an exact-size, exact-frame, exact-relocation candidate
under `NON_MATCHING`: 109 instructions with five differing words. The first
mismatch is at function offset `+0x5C`, where the target preserves the loaded
character with `move $t2,$a3` in a branch delay slot; IDO coalesces that copy
out of the candidate and reverses four dependent comparison operands. The
stock flag lattice found no exact result. The requested bounded permuter could
not start because `tools/permuter` is absent from this lane.

`func_8004BA8C` has an exact-size JFG/DKR-derived width candidate under
`NON_MATCHING`. The best declaration ordering compiles to 46 instructions
with the first six exact; its first mismatch is the frame allocation at
function offset `+0x18` (40 bytes versus the target's 48). The font pointer
spill is at the exact `sp+0x18` slot, but the spacing pointer lands at
`sp+0x24` instead of `sp+0x20`, causing IDO to restore the stack before the
loop and cascading into 28 positional word differences. The stock flag
lattice remained best; ten source-shape and local-order attempts did not
recover the retail spill layout.

The font subsegment's FP-register census contains only even-numbered single-
precision registers (`$f0`, `$f4`, `$f6`, `$f8`, `$f10`, `$f16`, and `$f18`),
so no function in this TU was excluded by the odd-register rule in section
6.2.

There are no direct string-literal references in this block. Its data
relocations address font/window state, a font-cache jump table, and rzip
state; consequently no tier-C names are available. ROM `0x4E1E0`–`0x4EA60`
is deliberately outside `main/font`: it is the PI-manager/rzip prefix of the
inflate subsystem, immediately followed by `main/gzip_asm` at `0x4EA60`.

### 3.5 The resident shadows and lights TUs

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
### 3.6 The resident allocator (`main/memory`)

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

### 3.7 `main/models` working split

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
**Why the original skeleton table did not itself produce a split.** §1's "measured
| `src/saves.c.o`, `src/rcpFast3d.c.o`, `src/track.c.o`, `src/textures.c.o`, `src/diCpu.c.o`, `src/objects.c.o`, `libultra/src/flash/flashreadid.c.o`, `us.v10/src/core1/code_1D00.c.o` (BK) | 1 each | single points | Isolated identifications at the time of this scan; §3.4 subsequently measures the complete `diCpu` span |

**Why no new `mickey.us.yaml` split accompanies this table.** §1's "measured
file boundary" tier requires a whole-`.text` match; this pass only matched
individual functions (`tools/find_known_objects.py --sections` found no
whole-object match for any of the not-yet-named TUs above). Asserting a yaml
`asm`/`c` split from function-level hits alone would claim more than was
measured, exactly the mistake 1.2's uniqueness clause exists to prevent one
level up. `gameVi` is the exception added later: §3.8 supplies independent
tier-B boundary evidence from its complete ordered function and call surface.
The already-measured TUs above (`n_csplayer`, `gsSnd`, `n_drvrNew`, `n_env`,
`n_load`, `math_util`) needed no new split; they already have one.

### 3.8 `gameVi`: ROM `0x34180`–`0x34E60`

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

`viInit` is adopted at tier B. Resident startup passes its scheduler at the
same point where JFG initializes video, immediately before the PI/RCP sequence.
The canonical body is exact at 74 words and all 52 relocation sites.

`func_800339B4` has JFG's `viReset` role: the shutdown caller invokes it after
stopping RSP/RDP work and before rumble teardown. Its best 50-word
`NON_MATCHING` body is linked-byte-exact, but the fixed framebuffer literal
omits the target object's three `D_80380000`/`D_80380004` relocation sites,
first at function offset `0x1C`. Spelling the address as an extern restores the
first symbol identity but adds an address-formation instruction and shifts the
remaining schedule. Section 1.5 therefore keeps the address label and the
original asm canonical.

`viAllocateZBuffer` and `viFreeZBuffer` are adopted at tier B as the paired
allocation lifecycle around mode changes. Their canonical bodies are exact at
22/20 words and 7/9 relocation sites respectively.

`viGetCurrentSize` is adopted at tier B: its callers pass two output pointers
and consume the active display dimensions written through them. Its 18 words
and four relocation sites are exact. `viConvertXY` is likewise pinned by
callers that pass coordinate pairs and immediately consume the scaled values;
all 21 words and four relocation sites are exact.

`viSetTiming` is adopted at tier B. Both the mode changer and the tier-A
wide-adjust setter call this same-position JFG role after changing video state.
The adapted body is canonical C at all 102 words and all 28 relocation sites;
its linked range and full-ROM hash are exact under `-O2 -mips2 -32`.

`viFrameSync` is adopted at tier B. The resident game loop passes its
buffer-swap message, stores the returned update rate, then bounds that rate
before the next update; this pins JFG's same-position role. The adapted body is
canonical C at all 106 words and all 26 relocation sites, with exact symbol
identities, linked bytes and full-ROM hash under the resident flags.

`fb_swap` is adopted at tier B. `viFrameSync` calls it for each non-skip frame,
and the mode changer calls the same routine after rebuilding its buffers. The
canonical body is exact at 56 words and all 18 relocation sites.

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

### 3.9 `main/track`: ROM `0xC950`-`0x16140`

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
| `func_8000D16C` | `0xDD6C` | 0x4C | `-O2 -mips2 -32` | Mickey reconstruction; JFG's corresponding `trackAddTextureScroll` is tier D only and its public name is deliberately not adopted; 19/19 instruction words and both HI16/LO16 relocation pairs exact, linked ROM exact |
| `func_8000D728` | `0xE328` | 0x40 | `-O2 -mips2 -32` | Mickey reconstruction; JFG's corresponding `trackLightDelete` is tier D only and its public name is deliberately not adopted; 16/16 instruction words and the D_800792FC HI16/LO16 pair exact, linked ROM exact |
| `func_8000D7F8` | `0xE3F8` | 0x28 | `-O2 -mips2 -32` | Mickey reconstruction; JFG's corresponding `trackLightMove` is tier D only and its public name is deliberately not adopted; 10/10 instruction words, no relocation records, linked ROM exact |
| `trackGetTrack` | `0x14AB4` | 0xC | `-O2 -mips2 -32` | Mickey reconstruction with JFG name (tier B callers); 3/3 instruction words and relocation layout exact, linked ROM exact |
| `trackSetFog` | `0x15030` | 0xF8 | `-O2 -mips2 -32` | JFG `src/track.c` body with tier B callers and tier D TU order; 62/62 instruction words and relocation layout exact, linked ROM exact |
| `trackGetFog` | `0x15128` | 0x78 | `-O2 -mips2 -32` | JFG direct-path body with tier B caller and tier D TU order; 30/30 instruction words and relocation layout exact, linked ROM exact |
| `trackSetFogOff` | `0x151A0` | 0x74 | `-O2 -mips2 -32` | JFG `src/track.c`; 29/29 instruction words and relocation layout exact, linked ROM exact |
| `func_80014614` | `0x15214` | 0x190 | `-O2 -mips2 -32` | Mickey reconstruction of the fog-state updater; JFG same-position skeleton is the 0.733 top hit but its placeholder is not imported; 100/100 instruction words and relocation layout exact, linked ROM exact |
| `func_800147A4` | `0x153A4` | 0x13C | `-O2 -mips2 -32` | Mickey reconstruction using the SDK fog-colour/position macros; JFG same-size top skeleton supplies structural context but its placeholder is not imported; 79/79 instruction words and relocation layout exact, linked ROM exact |
| `func_80014DE4` | `0x159E4` | 0xC8 | `-O2 -mips2 -32` | Mickey reconstruction; JFG supplies only tier-D transform-role context and no public name is adopted; 50/50 instruction words and relocation layout exact, linked ROM exact |
| `func_80014EAC` | `0x15AAC` | 0x20 | `-O2 -mips2 -32` | JFG `func_8001C550` is a tier-A 8/8-word TU donor, unique in the ROM; JFG placeholder not imported; linked ROM exact |

### 3.10 Resident camera: ROM `0x21EE0`–`0x25C20`

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

| Matched C function | ROM | Tier | Exact executable bytes | Proof |
|---|---:|---|---:|---|
| `camUseShake` | `0x22084` | B — role/order | 16 | Configured object, relocation pair, linked range and full ROM exact. |
| `camOverrideProjScales` | `0x220E4` | B — role/order (named above) | 32 | Configured object, six relocations, linked range and full ROM exact. |
| `camSetWaterLine` | `0x225B0` | A — byte identity (named above) | 32 | Configured object, relocation pair, linked range and full ROM exact. |
| `camGetProjOrgMtx` | `0x25270` | B — role/order (named above) | 28 | Configured object, two relocation pairs, linked range and full ROM exact. |
| `camSetZoom` | `0x258C8` | B — role/order (named above) | 56 | Configured object, two relocation pairs, linked range and full ROM exact. |
| `camGetPlayerProjMtx` | `0x23360` | A — byte identity (named above) | 52 | Configured object, five relocations, linked range and full ROM exact. |
| `camStopShakes` | `0x25754` | B — role/order (named above) | 76 | Configured object, three relocation pairs, linked range and full ROM exact. |
| `camIgnoreShake` | `0x22094` | B — role/order | 12 | Configured object, relocation pair, linked range and full ROM exact. |
| `camGetFOV` | `0x220A0` | B — role/order | 12 | Configured object, relocation pair, linked range and full ROM exact. |
| `camGetWaterLine` | `0x225A0` | D — TU order only, no per-symbol callgraph argument recorded | 16 | Configured object, relocation pair, linked range and full ROM exact. |
| `camGetMode` | `0x22518` | D — TU order only, no per-symbol callgraph argument recorded | 12 | Configured object, relocation pair, linked range and full ROM exact. |
| `camSetMode` | `0x22524` | D — TU order only, no per-symbol callgraph argument recorded | 64 | Configured object, two relocation pairs, linked range and full ROM exact. |
| `camGetNo` | `0x22564` | D — TU order only, no per-symbol callgraph argument recorded | 12 | Configured object, relocation pair, linked range and full ROM exact. |
| `camSetNo` | `0x22594` | D — TU order only, no per-symbol callgraph argument recorded | 12 | Configured object, relocation pair, linked range and full ROM exact; Mickey omits JFG's bounds guard. |
level up. The already-measured TUs above (`n_csplayer`, `gsSnd`, `n_drvrNew`,
`n_env`, `n_load`, `math_util`) needed no new split; they already have one.
The later menu census below adds independent boundary evidence rather than
retroactively treating the six hits as a whole-object match.

### 3.11 Resident front-end menu: ROM `0x39350`–`0x3B1A0`

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

### 3.12 Track assembly and shadows (`0x16140`–`0x18FF0`)

This block contains two JFG-lineage translation units. The boundary claims are
explicitly **not tier A whole-object matches**:

- `main/trackasm`, ROM `0x16140`–`0x16A90`: **tier B** from the track callers
  and helper call graph, plus **tier D** from JFG's exact four-function order
  (`trackMakePolylist`, `getXZCompareMask`, `getYCompareMask`,
  `trackLightAsm`). JFG carries the same run in `asm/hasm/trackasm.s`.
- `main/shadows`, ROM `0x16A90`–`0x18FF0`: **tier B** from allocation/free and
  track-render call relationships, plus **tier D** from JFG `src/shadows.c`'s
  order and the per-function masked-skeleton results. Its upper boundary is
  independently corroborated at **tier A**: `shadowBoxPolyOverlap` begins at
  `0x18FF0`, the first function of JFG's next TU, `shadows_214A0.c`.

**PROVENANCE:** the TU names and descriptive function names are borrowed from
Jet Force Gemini's public decomp (`asm/hasm/trackasm.s`, `src/shadows.c`, and
their `asm/nonmatchings/` file names), a permitted retail-derived source under
`docs/CLEANROOM.md`. Mickey's own bytes determine the bodies. JFG's
address-placeholder helper names are not imported.

| ROM | Size | Symbol | Evidence / disposition |
|---|---:|---|---|
| `0x16140` | `0x49C` | `trackMakePolylist` | B; extractor-marked handwritten, stays `asm` |
| `0x165DC` | `0x11C` | `getXZCompareMask` | B; extractor-marked handwritten, stays `asm` |
| `0x166F8` | `0x98` | `getYCompareMask` | B; extractor-marked handwritten, stays `asm` |
| `0x16790` | `0x300` | `trackLightAsm` | B; uses odd single-precision FP registers, stays `asm` |
| `0x16A90` | `0x12C` | `shadowInitBuffers` | B |
| `0x16BBC` | `0x78` | `shadowFreeBuffers` | B |
| `0x16C34` | `0x18` | `shadowChangeBuffer` | B name; exact C, 6 words, 2 relocs |
| `0x16C4C` | `0x4C` | `shadowGetBuffers` | B name; exact C, 19 words, 8 relocs |
| `0x16C98` | `0x7F8` | `shadowGenerate` | B |
| `0x17490` | `0x8B0` | `func_80016890` | unresolved |
| `0x17D40` | `0x520` | `func_80017140` | unresolved |
| `0x18260` | `0x56C` | `func_80017660` | unresolved |
| `0x187CC` | `0x4E8` | `func_80017BCC` | unresolved |
| `0x18CB4` | `0x33C` | `func_800180B4` | unresolved |

There are no string references in either TU. The only resident-tail anchors
are `D_800817A0` and `D_800817A4`, both floating-point constants. Of the four
extractor-marked handwritten track routines, only `trackLightAsm` uses odd FP
registers; the other three contain non-compiler instruction shapes and remain
assembly with it. No function in `main/shadows` uses an odd FP register.

### 3.13 Camera lights and sprite animation (`0x1BE50`–`0x1C790`)

The eight entry points at `0x1BE50`–`0x1BEA0` are Mickey's disabled
`main/camlight` implementation: each is a return-only or argument-spilling
stub, but their exact order and signatures follow JFG's `src/camlight.c`.
That ordering, the object-system call sites, and the clean handoff to
`spranimInit` make the boundary **tier B/D**, not a whole-object tier-A hit.
`main/spranim` then occupies the remainder of this assigned block. Its first
five functions follow JFG's `src/spranim.c` order; `texscrollControl` and
`rangetriggerControl` are additionally identified by their masked skeletons
and texture-scroll/volume-trigger callees. Helpers without that evidence keep
their Mickey address names.

**PROVENANCE:** the TU and descriptive function names are borrowed from Jet
Force Gemini's public retail-derived `src/camlight.c`, `src/spranim.c`, and
their `asm/nonmatchings/` names, as permitted by `docs/CLEANROOM.md`. No JFG
body is copied by this split; Mickey's own bytes remain authoritative.

| ROM | Size | Symbol | Evidence / disposition |
|---|---:|---|---|
| `0x1BE50` | `0x8` | `camlightInit` | D name; exact C, 2 words, 0 relocs |
| `0x1BE58` | `0x8` | `camlightFlush` | D name; exact C, 2 words, 0 relocs |
| `0x1BE60` | `0x10` | `camlightAdd` | D name; exact C, 4 words, 0 relocs |
| `0x1BE70` | `0x8` | `camlightDelete` | D name; exact C, 2 words, 0 relocs |
| `0x1BE78` | `0x8` | `camlightUpdateAll` | D name; exact C, 2 words, 0 relocs |
| `0x1BE80` | `0x8` | `camlightUpdate` | D name; exact C, 2 words, 0 relocs |
| `0x1BE88` | `0x8` | `camlightVisibilityCheck` | D name; exact C, 2 words, 0 relocs |
| `0x1BE90` | `0x10` | `camlightDraw` | D name; exact C, 4 words, 0 relocs |
| `0x1BEA0` | `0x74` | `spranimInit` | D |
| `0x1BF14` | `0x4C` | `spranimControl` | D |
| `0x1BF60` | `0x48` | `sprasjiInit` | D |
| `0x1BFA8` | `0x78` | `spranimOnceControl` | D |
| `0x1C020` | `0x304` | `effectboxControl` | D |
| `0x1C324` | `0x74` | `texscrollControl` | B; identified additionally by masked skeleton and texture-scroll callees |
| `0x1C398` | `0x2BC` | `func_8001B798` | unresolved |
| `0x1C654` | `0x90` | `rangetriggerControl` | B; identified additionally by masked skeleton and volume-trigger callees |
| `0x1C6E4` | `0x14` | `func_8001BAE4` | exact C, 5 words, 0 relocs; role unresolved |
| `0x1C6F8` | `0xC` | `func_8001BAF8` | exact C, 3 words, 0 relocs; role unresolved |
| `0x1C704` | `0xC` | `func_8001BB04` | exact C, 3 words, 0 relocs; role unresolved |
| `0x1C710` | `0x78` | `func_8001BB10` | plateau: 8/30 words differ; first `+0x3C`, load scheduling |

No function in this range uses an odd single-precision FP register, and there
are no string references. All twenty functions are compiler-generated. ROM
`0x1C788`-`0x1C790` is alignment padding and receives no function credit.

### 3.14 Weather (`0x3B480`–`0x3D5F0`)

This run is `main/weather`. The second function, `weather_clip_planes`, was
already a unique tier-A DKR byte match. The preceding `initWeather` and the
remaining snow/rain call graph agree with JFG and DKR at **tier B/D**; the
masked-skeleton scan independently selected their weather counterparts for
all public entry points and most helpers. The upper boundary is fixed
independently at **tier A** by `reset_particles`, the first function of the
following `particles.c` run at `0x3D5F0`. The lower boundary is structural,
not a claimed whole-object match.

The logical TU is represented by three physical splat fragments:
`main/weather` (`0x3B480`–`0x3D030`), the hand-written
`main/weather_snow_asm` island (`0x3D030`–`0x3D370`), and
`main/weather_tail` (`0x3D370`–`0x3D5F0`). This keeps the hand-written pair
out of asm-processor while retaining C ownership around it.

**PROVENANCE:** the TU and descriptive function names are borrowed from Jet
Force Gemini's and Diddy Kong Racing's public retail-derived `src/weather.c`
files and JFG's `asm/nonmatchings/weather/` names, as permitted by
`docs/CLEANROOM.md`. The matched `initWeather`, `weather_clip_planes`,
`freeWeather`, `setupWeather`, `snow_init`, `changeWeather`, and `rainDensity`
bodies, plus `rain_set`, `rainSetFog`, `rain_update`, `rain_lightning`, and
`rain_sound`, are adapted from those disclosed sources and carry point-of-use
notes; Mickey's own bytes remain authoritative.

The tier-B/D `initWeather` adds **0xFC bytes / 63 words** at ROM `0x3B480`.
JFG's initialization and asset-table walk reproduce Mickey's instruction
stream at the canonical `-O2 -mips2 -32` flags, with all 25 relocations and
the linked ROM range agreeing.

The tier-B/D `freeWeather` adds **0x120 bytes / 72 words** at ROM `0x3B5D0`.
JFG's release sequence maps directly onto Mickey's global layout; the
canonical `-O2 -mips2 -32` object is instruction-exact with all 34 relocations
and the linked ROM range agreeing.

The tier-B/D `setupWeather` adds **0x420 bytes / 264 words** at ROM `0x3B6F0`.
JFG's declaration order and control spelling reproduce Mickey's 0x60-byte
frame, while Mickey's own rain-init arguments, random bounds, texture layout,
and buffer-end sentinel settle the revision differences. The canonical
`-O2 -mips2 -32` object is instruction-exact with all 41 relocations agreeing,
and the linked ROM range is exact without post-processing.

The tier-B/D `changeWeather` adds **0x1EC bytes / 123 words** at ROM
`0x3BC30`. JFG supplies the state transition; Mickey's combined condition and
assignment order compile instruction-exact at the canonical `-O2 -mips2 -32`
flags, with all 5 relocations and the linked ROM range agreeing.

The tier-B/D `snow_init` adds **0x120 bytes / 72 words** at ROM `0x3BB10`.
DKR supplies the circular position loop; Mickey's scale constants and texture
loader compile instruction-exact at canonical `-O2 -mips2 -32`, with all 8
relocations and the linked ROM range agreeing.

The tier-B/D `rain_lightning` adds **0x128 bytes / 74 words** at ROM
`0x3CE48`. The DKR/JFG timer structure plus Mickey's transition arguments and
thresholds compile instruction-exact at canonical `-O2 -mips2 -32`, with all
17 relocations and the linked ROM range agreeing.

The tier-B/D `rain_update` adds **0x144 bytes / 81 words** at ROM `0x3C6B4`.
JFG's transition and dispatch structure, including Mickey's unresolved
rain-movement binding, compiles instruction-exact at canonical
`-O2 -mips2 -32`, with all 27 relocations and the linked ROM range agreeing.

The tier-B/D `rain_set` adds **0x104 bytes / 65 words** at ROM `0x3C468`.
JFG's TV-rate-dependent transition setup compiles instruction-exact at
canonical `-O2 -mips2 -32`, with all 18 relocations and the linked ROM range
agreeing.

The tier-B/D `rainSetFog` adds **0xD0 bytes / 52 words** at ROM `0x3C56C`.
JFG's level-flag guard and fog calculation compile instruction-exact at
canonical `-O2 -mips2 -32`, with all 7 relocations and the linked ROM range
agreeing.

The tier-B/D `rain_sound` adds **0xC0 bytes / 48 words** at ROM `0x3CF70`.
JFG's camera-relative sound positioning compiles instruction-exact at canonical
`-O2 -mips2 -32`, with all 13 relocations and the linked ROM range agreeing.

`doWeather` plateaued after the JFG body, the 119-combination flag lattice,
and seven source-order, typing, and allocation hypotheses. The best canonical
candidate, preserved behind `NON_MATCHING`, differs in 54 of 169 positional
words (115 exact), with the first mismatch at `+0xB4`; relocation identities
agree, but update-block scheduling shifts the remaining register allocation.
The bounded permuter was unavailable because this lane has no local
decomp-permuter checkout.

`rain_init` and `free_rain_memory` share a synthetic static
`TrapDanglingJump` binding with `rain_update`, but require incompatible integer,
void, and float call signatures inside the consolidated TU. Their JFG bodies
otherwise reproduce all 59 and 33 instruction words and every relocation kind;
the best candidates retain one relocation-identity mismatch each, at `+0xA0`
and `+0x68` respectively. Direct calls, typed function-pointer casts, weak
aliases, and three- versus four-parameter `rain_init` declarations were tested;
the candidates remain preserved behind `NON_MATCHING`.

| ROM | Size | Symbol | Evidence / disposition |
|---|---:|---|---|
| `0x3B480` | `0xFC` | `initWeather` | D |
| `0x3B57C` | `0x54` | `weather_clip_planes` | A donor; exact C, 21 words, 2 relocs |
| `0x3B5D0` | `0x120` | `freeWeather` | D |
| `0x3B6F0` | `0x420` | `setupWeather` | D |
| `0x3BB10` | `0x120` | `snow_init` | D |
| `0x3BC30` | `0x1EC` | `changeWeather` | D |
| `0x3BE1C` | `0x2A4` | `doWeather` | D |
| `0x3C0C0` | `0x238` | `snow_render` | D |
| `0x3C2F8` | `0xEC` | `rain_init` | D |
| `0x3C3E4` | `0x84` | `free_rain_memory` | D |
| `0x3C468` | `0x104` | `rain_set` | D |
| `0x3C56C` | `0xD0` | `rainSetFog` | D |
| `0x3C63C` | `0x78` | `rainDensity` | D name; exact C, 30 words, 4 relocs |
| `0x3C6B4` | `0x144` | `rain_update` | D |
| `0x3C7F8` | `0x650` | `rain_render_splashes` | D |
| `0x3CE48` | `0x128` | `rain_lightning` | D |
| `0x3CF70` | `0xC0` | `rain_sound` | D |
| `0x3D030` | `0x144` | `snow_update` | D; handwritten asm |
| `0x3D174` | `0x1FC` | `snow_vertices` | D; odd-FP handwritten asm |
| `0x3B480` | `0xFC` | `initWeather` | B/D name; exact C, 63 words, 25 relocs |
| `0x3B57C` | `0x54` | `weather_clip_planes` | A donor; exact C, 21 words, 2 relocs |
| `0x3B5D0` | `0x120` | `freeWeather` | B/D name; exact C, 72 words, 34 relocs |
| `0x3B6F0` | `0x420` | `setupWeather` | B/D name; exact C, 264 words, 41 relocs |
| `0x3BB10` | `0x120` | `snow_init` | B/D name; exact C, 72 words, 8 relocs |
| `0x3BC30` | `0x1EC` | `changeWeather` | B/D name; exact C, 123 words, 5 relocs |
| `0x3BE1C` | `0x2A4` | `doWeather` | B/D; plateau, 54/169 words differ, first `+0xB4` |
| `0x3C0C0` | `0x238` | `snow_render` | B/D |
| `0x3C2F8` | `0xEC` | `rain_init` | B/D; plateau, 59 words exact, one reloc identity at `+0xA0` |
| `0x3C3E4` | `0x84` | `free_rain_memory` | B/D; plateau, 33 words exact, one reloc identity at `+0x68` |
| `0x3C468` | `0x104` | `rain_set` | B/D name; exact C, 65 words, 18 relocs |
| `0x3C56C` | `0xD0` | `rainSetFog` | B/D name; exact C, 52 words, 7 relocs |
| `0x3C63C` | `0x78` | `rainDensity` | B/D name; exact C, 30 words, 4 relocs |
| `0x3C6B4` | `0x144` | `rain_update` | B/D name; exact C, 81 words, 27 relocs |
| `0x3C7F8` | `0x650` | `rain_render_splashes` | B/D |
| `0x3CE48` | `0x128` | `rain_lightning` | B/D name; exact C, 74 words, 17 relocs |
| `0x3CF70` | `0xC0` | `rain_sound` | B/D name; exact C, 48 words, 13 relocs |
| `0x3D030` | `0x144` | `snow_update` | B/D; handwritten asm |
| `0x3D174` | `0x1FC` | `snow_vertices` | B/D; odd-FP handwritten asm |
| `0x3D370` | `0x9C` | `func_8003C770` | unresolved |
| `0x3D40C` | `0x1E4` | `func_8003C80C` | unresolved |

There are no string references. `snow_vertices` is the range's only function
using odd single-precision FP registers; it and extractor-marked
`snow_update` remain source assembly permanently rather than matching targets.
The independent boundary evidence used later for `saves`, `pi`, `screen`,
`rcpFast3d` and `sched` is recorded separately below rather than retroactively
attributed to this scan.

### 3.15 Save, PI, screen, RCP and scheduler census

ROM `0x2C8C0`–`0x323A0` contains **86 functions and 23,264 bytes** in five
consecutive JFG-lineage translation units. This is a function-order and call-
graph census, not a claim that any whole JFG object is byte-identical. The
source files carry the required `PROVENANCE` disclosure; Mickey's own bytes
remain authoritative for every body.

| ROM range | Size | Functions | TU | Evidence |
|---|---:|---:|---|---|
| `0x2C8C0`–`0x2ECA0` | 9,184 | 42 | `main/saves` | **A:** `rumbleKill` is byte-identical to JFG. **B:** ordered rumble calls, save checksums, and the contiguous Controller Pak API (`osPfs*`). **D:** uncertain Mickey-only helpers retain `func_<VRAM>` names. |
| `0x2ECA0`–`0x2F0D0` | 1,072 | 7 | `main/pi` | **B:** the exact JFG function order `piInit`, four asset lookups/loaders, two accessors, `romCopy`; the last routine owns the `osPiStartDma` loop. |
| `0x2F0D0`–`0x2F400` | 816 | 2 | `main/screen` | **B:** load/decompress followed by draw/VI calls, matching JFG's two-function `screen.c` order. |
| `0x2F400`–`0x30CD0` | 6,352 | 14 | `main/rcpFast3d` | **A:** `rcpInit` and the existing border-colour routine are byte-identity anchors; masked skeletons also reproduce JFG's `rcpFast3d`/screen-colour shapes. **B:** queue/RCP calls and the ordered init helpers. |
| `0x30CD0`–`0x323A0` | 5,840 | 21 | `main/sched` | **A:** the two queue accessors. **B:** the complete JFG scheduler call graph from `osCreateScheduler` through `__scSchedule`. **C:** `osScGetTaskType`'s seven task-name strings and `__scHandleRetrace`'s `"SP CRASHED"`/`"Version %s"`. |

The function boundaries are the extracted labels cross-checked against the
linked ELF's symbol sizes. All 86 functions were queried with
`tools/skeleton_scan.py similar --target <vram> --top 5`; the useful exact
anchors are the ones stated above, while the remaining results are near-match
context rather than naming evidence. No function in either original block
uses an odd single-precision FP register, so §6.2's hand-written-assembly
criterion identifies **zero** forced-ASM functions here.

The table above states each TU's evidence *categories*, not a per-function
verdict; §1 requires the latter. Per §1's rule, only the functions named in
the A/B/C cells above carry that individual argument (`rumbleKill`: tier A;
`piInit` and the rest of `main/pi`'s seven-function order: tier B; both
`main/screen` functions: tier B; `rcpInit` and the border-colour routine:
tier A, the remaining `main/rcpFast3d` functions: tier B; the two queue
accessors and `osScGetTaskType`/`__scHandleRetrace`: tier A/A/C respectively,
the rest of `main/sched`'s scheduler call graph: tier B). Every other
function in these five TUs, without an individual argument beyond TU
membership and order, is tier D. `symbol_addrs.us.txt` carries the resulting
per-symbol tier token for each of the 86 functions; this table is the
TU-level summary, not a substitute for it.

Exact C reconstructions in this census currently include `rumbleRumbles`
(ROM `0x2C8FC`–`0x2C908`, 12 bytes), `rumbleKill` (ROM `0x2CB00`–
`0x2CB44`, 68 bytes), `rumbleUpdate` (ROM `0x2CB44`–`0x2CB54`, 16 bytes),
`packCalculateGameChecksum` (ROM `0x2D3BC`–
`0x2D3EC`, 48 bytes), `packCalculateGlobalFlagsChecksum` (ROM `0x2DA2C`–
`0x2DA54`, 40 bytes), `packClose` (ROM `0x2DED4`–`0x2DF00`, 44 bytes),
`packDirectoryFree` (ROM `0x2E424`–`0x2E458`, 52 bytes),
`piRomLoadSection` (ROM `0x2EEE0`–`0x2EF5C`, 124 bytes),
`piRomGetSectionPtr` (ROM `0x2EF5C`–`0x2EFA4`, 72 bytes),
`piRomGetFileSize` (ROM `0x2EFA4`–`0x2EFE0`, 60 bytes),
`rcpSetScreenColour` (ROM `0x2F76C`–`0x2F794`, 40 bytes), and the still-
unnamed global setter `func_8002EBD4` (ROM `0x2F7D4`–`0x2F7E0`, 12 bytes),
plus `rcpInitDpNoSize` (ROM `0x30118`–`0x3013C`, 36 bytes) and
`rcpInitSp` (ROM `0x3013C`–`0x30160`, 36 bytes), and
`osScGetAudioSPStats` (ROM `0x30F20`–
`0x30F38`, 24 bytes), `osScGetCmdQ` (ROM `0x30F10`–`0x30F18`, 8 bytes),
`osScGetInterruptQ` (ROM `0x30F18`–`0x30F20`, 8 bytes), and the still-unnamed
no-op `func_80030608` (ROM `0x31208`–`0x31210`, 8 bytes). All were compiled
with the resident `-O2 -mips2 -32` flags. The named bodies are adapted from
JFG's `src/saves.c`, `src/pi.c`, `src/rcpFast3d.c`, and `src/sched.c`; the anonymous
setter and no-op are reconstructed from Mickey's own bodies. All configured
object ranges and the final linked ROM are byte-exact.

`__scHandleRetrace` has a preserved `NON_MATCHING` JFG-derived body after the
119-combination flag lattice and ten source-shape hypotheses. The best
candidate has the exact 232-byte frame and 408 instructions versus the
target's 409; 84 positional words differ after relocation masking. Its first
mismatch is at function `+0x3B4`, where the diagnostic Y-coordinate and two
stack-byte writes schedule in a different order. The remaining tail also
materialises the 64-bit retrace counter through one combined object while the
target uses separate high/low symbol references. The assembly fallback
remains canonical.

### 3.16 Particle and debug-print translation units

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

The Evidence column above states categories per TU, not a per-function
verdict. Per-symbol: `reset_particles`, `strcpy`, `memset`, and `sprintf` are
tier A (named donor-object byte matches); the internal/external
particle-caller and `diPrintf`/`diPrintfAll`/`diRcp*` call-graph functions
named above are tier B; `_itoa` and `vsprintf` are tier C (string
correspondence); every other function in `main/particles` and `main/diprint`,
identified only by JFG/DKR order and masked-skeleton shape and not named
individually above, is tier D. `symbol_addrs.us.txt` carries the resulting
per-symbol tier token for each of the 63 functions in these two TUs.

Exact C matches banked in these TUs: `partAdjustScaling` (ROM `0x3F9C8`,
`0xC` bytes, default resident flags, JFG body donor) and `func_8003EDD4`
(ROM `0x3F9D4`, `0xC` bytes, default resident flags, Mickey-only
reconstruction) and `func_8003EDE0` (ROM `0x3F9E0`, `0xC` bytes, default
resident flags, Mickey-only reconstruction); `strcpy` (ROM `0x43470`, `0x34`
bytes, default resident flags, DKR body donor) and `memset` (ROM `0x434A4`,
`0x34` bytes, default resident flags, DKR body donor); `reset_particles` (ROM
`0x3D5F0`, `0x30` bytes, default resident flags, DKR body donor); `sprintf`
(ROM `0x435A4`, `0x2C` bytes, default resident flags, DKR body donor);
`debug_text_origin` (ROM `0x45710`, `0x24` bytes, default resident flags, JFG
body donor); `sprintfSetSpacingCodes` (ROM `0x43598`, `0xC` bytes, default
resident flags, JFG body donor); `debug_text_newline` (ROM `0x45734`, `0x28`
owned bytes, default resident flags, JFG body donor; the following 4-byte TU
alignment pad is excluded from match credit); `debug_text_bounds` (ROM
`0x45680`, `0x90` bytes, default resident flags, JFG body donor); `diPrintfInit`
(ROM `0x448E0`, `0x54` bytes, default resident flags, JFG body donor);
`diPrintfSetXY` (ROM `0x44D48`, `0x8C` bytes, default resident flags, JFG body
donor); `diPrintfSetCol` (ROM `0x44C10`, `0x9C` bytes, default resident flags,
JFG body donor); `diPrintfSetBG` (ROM `0x44CAC`, `0x9C` bytes, default resident
flags, JFG body donor); `diPrintf` (ROM `0x44934`, `0x9C` bytes, default
resident flags, JFG body donor with its stubbed diagnostic call omitted);
`_itoa` (ROM `0x434D8`, `0xC0` bytes, default resident flags, identical JFG and
DKR glibc-derived body donor); `func_8003CCE4` (ROM `0x3D8E4`, `0x44` bytes,
default resident flags, Mickey-only reconstruction); `partInitTriggerSPPos`
(ROM `0x3F224`, `0x4C` bytes, default resident flags, JFG-named Mickey
reconstruction); `partInitTrigger` (ROM `0x3F1AC`, `0x78` bytes, default
resident flags, JFG-named Mickey reconstruction); `debug_text_background`
(ROM `0x452F8`, `0xA0` bytes, resident flags plus `-Wab,-r4300_mul`, JFG body
donor).

`vsprintf` reached a bounded size-exact plateau under `-Wab,-r4300_mul`: its
1,220-word candidate differs in two adjacent words, first at function offset
`0xB08`, where IDO loads the final exponent digit constants in the reverse
order. The flag lattice found no exact alternative, and the bounded permuter
could not parse the formatter's `va_arg` macros. Promotion is additionally
blocked because the C body emits formatter jump tables and static strings
still owned by the resident asm-data split.

`debug_text_parse` reached an instruction-exact 263-word plateau. Strict
object comparison still finds four relocation-identity differences: two
accesses use the separately named `D_800D4A62` instead of `D_800D4A60+2`, and
two switch-table references use compiler `.rodata` instead of
`jtbl_80082CD8`. The generated switch table would also duplicate the resident
asm-data owner, so the original asm body remains canonical.

No function in either range uses an odd single-precision floating-point
register. None is therefore classified as handwritten assembly by §6.2's
criterion.
`func_800336A8` reached a bounded `NON_MATCHING` plateau from JFG's
`viChangeMode` body. The best 195-word candidate has the exact target length
but differs at 79 relocation-masked positions, first at function offset zero;
its local requested-buffer value expands the non-save frame from 40 to 48
bytes. Removing that home restores the target frame but adds one instruction
and leaves 110 differing positions. The blocker is register allocation around
the conditional third-framebuffer allocation. The flag lattice found no exact
variant, and the configured permuter checkout is absent in this lane.

`func_80034094` has an instruction- and relocation-exact 47-word
`NON_MATCHING` switch body adapted from JFG's `viGetOsViMode`. It cannot be
promoted within this TU's ownership because the separately extracted
`jtbl_8008249C` still owns the 12 case-label references; replacing the asm body
therefore leaves those labels undefined and also emits a duplicate 48-byte
table. The canonical path remains the original asm pending coordinated rodata
ownership.
| `src/saves.c.o`, `src/rcpFast3d.c.o`, `src/track.c.o`, `src/textures.c.o`, `src/diCpu.c.o`, `src/objects.c.o`, `libultra/src/flash/flashreadid.c.o`, `us.v10/src/core1/code_1D00.c.o` (BK) | 1 each | single points | Isolated identifications, no span to claim |

**Why the rows do not establish new internal boundaries.** §1's "measured file
boundary" tier requires a whole-`.text` match; this pass only matched individual
functions (`tools/find_known_objects.py --sections` found no whole-object match
for any of the not-yet-named TUs above). The later `main/anim` split (§3.4)
therefore preserves the pre-existing, 16-byte-aligned `0x50C00`–`0x58570`
block in one piece. It is source ownership, not a claim that the whole range is
JFG's `anim.c`. The already-measured TUs above (`n_csplayer`, `gsSnd`,
`n_drvrNew`, `n_env`, `n_load`, `math_util`) needed no new split; they already
have one.

### 3.4 The `main/anim` source-owning block

ROM `0x50C00`–`0x58570`, VRAM `0x80050000`–`0x80057970`, is now one C
subsegment with 55 function starts. This is deliberately a source-ownership
boundary, not a donor-TU identity claim. The first 34 functions follow JFG's
`anim.c` family; the following code has JFG `hit.c` shapes; the final function
has the exact masked skeleton of JFG's `fmvInit`. There is no proved,
16-byte-aligned internal object boundary, so the old yaml block remains intact.

PROVENANCE: the comparison names below come from Jet Force Gemini's public
decompilation (`src/anim.c`, `src/hit.c`, `src/fmv.c`, their built objects, and
their public declarations), permitted by `docs/CLEANROOM.md`. They are
comparison labels, not silently adopted Mickey symbols. Tier A rows were
rechecked with relocation-aware byte comparison and `romocc=1`; tier B rows
are pinned by the within-block call graph; tier D rows are structural
similarity or source-order evidence only. Section 1.5 keeps the Mickey
`func_<VRAM>` name for every still-assembly function, and JFG address
placeholders are never imported.

| Mickey VRAM | Size | JFG comparison | Evidence / status |
|---|---:|---|---|
| `0x80050000` | `0x24` | `func_800767A0` | D naming; placeholder retained. Matched C: exact 36 B and relocation surface at `-O2 -mips2 -32` |
| `0x80050024` | `0x80` | `func_800767C4` | D naming; placeholder retained. Matched C: exact 128 B and relocation surface at `-O2 -mips2 -32` |
| `0x800500A4` | `0x98` | `func_80076840` | D naming; placeholder retained. Matched C: exact 152 B and relocation surface at `-O2 -mips2 -32` |
| `0x8005013C` | `0x40` | `func_800768D4` | D naming; placeholder retained. Matched C: exact 64 B and relocation surface at `-O2 -mips2 -32` |
| `0x8005017C` | `0x30` | `func_80076918` | A; exact 48 B, masked `6/12`, placeholder retained. Matched C: exact 48 B and relocation surface at `-O2 -mips2 -32` |
| `0x800501AC` | `0x1C` | `func_80076948` | D naming; placeholder retained. Matched C: exact 28 B and relocation surface at `-O2 -mips2 -32` |
| `0x800501C8` | `0xB4` | `func_80076968` | D; 0.653 skeleton similarity, placeholder retained. Plateau after 10 variants: exact size, opcode schedule, and relocations; 7 register-only words remain from one `$s2`/`$s3` allocation swap, first mismatch `+0x5C` |
| `0x8005027C` | `0x50` | `func_80076A20` | A; exact 80 B, masked `9/20`, placeholder retained. Matched C: exact 80 B and relocation surface at `-O2 -mips2 -32` |
| `0x800502CC` | `0x7C` | `func_80076A70` | B; same cleanup callees and position, placeholder retained. Matched C: exact 124 B and relocation surface at `-O2 -mips2 -32` |
| `0x80050348` | `0x214` | `animseqInitPath` | B; exact `animseqInitGroup` calls this function |
| `0x8005055C` | `0x12C` | `animseqResetPath` | B; reset/process callers and trap/audio call shape |
| `0x80050688` | `0x7C` | `animseqStartPath` | B; process-command call position, adopted name. Matched C: exact 124 B and relocation surface at `-O2 -mips2 -32` |
| `0x80050704` | `0x78` | `animseqStopPath` | B; process-command call position, adopted name. Matched C: exact 120 B and relocation surface at `-O2 -mips2 -32` |
| `0x8005077C` | `0x40` | no unique candidate | D; placeholder retained. Matched C: exact 64 B and relocation surface at `-O2 -mips2 -32` |
| `0x800507BC` | `0x88` | `animseqHoldPath` | B; process-command call position, adopted name. Matched C: exact 136 B and relocation surface at `-O2 -mips2 -32` |
| `0x80050844` | `0x38` | `animseqLockPath` | B; paired process-command calls, adopted name. Matched C: exact 56 B and relocation surface at `-O2 -mips2 -32` |
| `0x8005087C` | `0x38` | `animseqUnLockPath` | B; paired process-command calls, adopted name. Matched C: exact 56 B and relocation surface at `-O2 -mips2 -32` |
| `0x800508B4` | `0x20` | no unique candidate | D; placeholder retained. Matched C: exact 32 B and relocation surface at `-O2 -mips2 -32` |
| `0x800508D4` | `0x200` | `func_800772C4` | B; bit-reader call sequence, placeholder retained |
| `0x80050AD4` | `0x120` | `animseqLinkNodes` | D; nearest ordered `anim.c` function |
| `0x80050BF4` | `0x15C` | `animseqInit` | D; 0.753 skeleton similarity |
| `0x80050D50` | `0x58` | `func_80077784` | D; nearest `anim.c` skeleton, placeholder retained. Matched C: exact 88 B and relocation surface at `-O2 -mips2 -32` |
| `0x80050DA8` | `0x48` | `animseqFreeLevelData` | B; frees storage then the group, adopted name. Matched C: exact 72 B and relocation surface at `-O2 -mips2 -32` |
| `0x80050DF0` | `0xAC` | `animseqLoadLevelData` | D; nearest ordered `anim.c` function, placeholder retained. Plateau after 10 variants: exact size, opcode schedule, and relocations; 7 operand/register words remain from a three-temporary FIFO rotation and the source stack home at candidate `+0x18` versus target `+0x1C`, first mismatch `+0x28` |
| `0x80050E9C` | `0x168` | `animseqFreeGroup` | B; same member-cleanup call graph. Plateau after 10 variants: best candidate has the exact `0x20` frame and first 25 instructions, then differs at `+0x64` on the `slti` destination and is one instruction short because IDO reuses the preceding `D_800D6BF8` address where the target rematerializes it; `-Wo,-loopunroll,0` is required to avoid a 25-instruction unroll expansion |
| `0x80051004` | `0xE4` | `animseqSetupGroup` | B; calls free/init/reset group family. Plateau after 10 source variants: the best candidate has the exact 57-instruction size and relocation identities but 41 positional words differ, first at `+0x2C`, because removing the extra call-argument rematerialization changes the loop's argument-register allocation |
| `0x800510E8` | `0x40` | `animseqInitGroup` | A; exact 64 B, masked `1/16`, adopted name. Matched C: exact 64 B and relocation surface at `-O2 -mips2 -32` |
| `0x80051128` | `0x9C` | `animseqResetGroup` | B; calls reset-path family, adopted name. Matched C: exact 156 B and relocation surface at `-O2 -mips2 -32` |
| `0x800511C4` | `0x1A0` | `func_80077BE8` | D; 0.321 skeleton similarity, placeholder retained |
| `0x80051364` | `0x47C` | `animseqUpdate` | D; nearest ordered `anim.c` function |
| `0x800517E0` | `0x1C40` | `animseqProcessCommandList` | B; command dispatcher calls the path family in JFG order |
| `0x80053420` | `0x90` | `animseqCamera` | D; ordered tail and nearest same-family shape. Matched C: exact 144 B and relocation surface at `-O2 -mips2 -32` |
| `0x800534B0` | `0x10` | `animseqPlay` | D adoption; ordered JFG tail and the `playing = 1` store. Matched C: exact 16 B and relocation surface at `-O2 -mips2 -32`; skeleton remains too short for tier A |
| `0x800534C0` | `0x2C` | `animseqPause` | D; ordered `anim.c` tail only, so the placeholder remains. Matched C: exact 44 B and relocation surface at `-O2 -mips2 -32`; the overwritten formal counter is required for IDO's target `$a0` allocation and has no static Mickey caller |
| `0x800534EC` | `0x64` | no unique `hit.c` candidate | D; placeholder retained at the start of collision-shaped code. Matched C: exact 100 B and relocation surface at `-O2 -mips2 -32` |
| `0x80053550` | `0x318` | `hitInitObjectHit` | B; same two matrix-builder calls |
| `0x80053868` | `0x12D4` | `hitUpdate` | B; collision dispatcher over the following helpers |
| `0x80054B3C` | `0x5C8` | no unique `hit.c` candidate | D; collision/vector shape |
| `0x80055104` | `0x6F4` | no unique `hit.c` candidate | D; collision/vector shape |
| `0x800557F8` | `0x178` | no unique `hit.c` candidate | D; collision handler family |
| `0x80055970` | `0x1B4` | no unique `hit.c` candidate | D; collision handler family |
| `0x80055B24` | `0x1E4` | no unique `hit.c` candidate | D; collision handler family |
| `0x80055D08` | `0x148` | no unique `hit.c` candidate | D; collision handler family |
| `0x80055E50` | `0x114` | no unique `hit.c` candidate | D; collision handler family |
| `0x80055F64` | `0x16C` | no unique `hit.c` candidate | D; collision handler family |
| `0x800560D0` | `0x1A4` | no unique `hit.c` candidate | D; collision handler family |
| `0x80056274` | `0x140` | no unique `hit.c` candidate | D; collision handler family |
| `0x800563B4` | `0xA24` | `hitVectorCheck` | B; vector/cylinder/sphere-style callee pattern |
| `0x80056DD8` | `0x394` | no unique `hit.c` candidate | D; collision/vector shape |
| `0x8005716C` | `0x140` | `hitGetInelasticVelocity` | D; nearest named leaf shape |
| `0x800572AC` | `0xA4` | no unique `hit.c` candidate | D; collision handler, placeholder retained. Matched C: exact 164 B and relocation surface at `-O2 -mips2 -32` |
| `0x80057350` | `0x78` | no unique `hit.c` candidate | D; collision handler, placeholder retained. Matched C: exact 120 B and relocation surface at `-O2 -mips2 -32` |
| `0x800573C8` | `0x3A4` | no unique `hit.c` candidate | D; collision/vector shape |
| `0x8005776C` | `0x1A4` | `hitPlayer` | B; same player-list/square-root call shape |
| `0x80057910` | `0x5C` + `0x4` pad | `fmvInit` | A; exact masked JFG skeleton and C donor, adopted name. Matched C: exact 92 executable B and relocation surface at `-O2 -mips2 -32`; trailing 4 B is compiler alignment padding and earns no function credit |

No function in this block directly references a distinctive string. Its
references into `0x80083FA8`–`0x80084218` are floating-point constants, so no
tier-C name is available. A scan of every function found no odd
single-precision FP register operand; §6.2 therefore parks none of this block
as hand-written assembly on that criterion.

### 3.4 The resident shadows and lights TUs

ROM `0x18FF0`–`0x1AE60` holds `main/shadows` and `main/lights`. JFG's exact
`shadowMakeYs` ends at ROM `0x19310` (VRAM `0x80018710`), where its `lights.c`
call graph begins; Mickey shares the order and alignment (tier B). Shadows has
all 88 odd-FP operands and stays assembly under §6.2; lights has none.

PROVENANCE DISCLOSURE. Comparisons use JFG's permitted public
`src/{shadows_214A0,lights}.c` and `src/lights.h`.

| Mickey VRAM | Size | JFG namesake | Evidence / disposition |
|---:|---:|---|---|
| `0x800183F0` | `0xC4` | `shadowBoxPolyOverlap` | Tier A: 49/49 unmasked words, ROM-wide unique; already adopted |
| `0x800184B4` | `0x90` | `shadowBoundingBox` | Tier A: 36/36 unmasked words, ROM-wide unique; already adopted |
| `0x80018544` | `0x110` | `shadowYHeight` | comparison only: unique nearest 4-gram skeleton, 0.919; remains `func_80018544` |
| `0x80018654` | `0xBC` | `shadowMakeYs` | Tier-A candidate: 47/47 unmasked words, ROM-wide unique; assembly pending a function-sized naming commit |
| `0x80018710` | `0x8C` | `freeLights` | Tier A: JFG-adapted C is compiler/link exact |
| `0x8001879C` | `0x130` | `setupLights` | tier-B comparison: calls the preceding free, three allocators and `lightCreateLightTable`; C still `func_8001879C` |
| `0x800188CC` | `0xB0` | JFG placeholder `func_80020D94` | Tier A: Mickey/DKR-adapted C is compiler/link exact; placeholder remains prohibited by §1.5 |
| `0x8001897C` | `0x238` | `addRomdefLight` | Tier A: Mickey/JFG-adapted C is compiler/link exact |
| `0x80018BB4` | `0x200` | `addObjectLight` | Tier A: Mickey/JFG-adapted C is compiler/link exact |
| `0x80018DB4` | `0x10` | `turnLightOff` | Tier A: JFG-adapted C is compiler/link exact |
| `0x80018DC4` | `0x10` | `turnLightOn` | Tier A: JFG-adapted C is compiler/link exact |
| `0x80018DD4` | `0x10` | `toggleLight` | Tier A: JFG-adapted C is compiler/link exact |
| `0x80018DE4` | `0x2C` | `changeLightColour` | Tier A: JFG-adapted C is compiler/link exact |
| `0x80018E10` | `0x20` | `changeLightColourCycle` | Tier A: 7/8 unmasked words, ROM-unique; linked C is byte-exact and adopted |
| `0x80018E30` | `0x4C` | `changeLightIntensity` | Tier A: JFG-adapted C is compiler/link exact |
| `0x80018E7C` | `0x8C` | `lightUpdateLights` | Tier A: JFG-adapted C is compiler/link exact |
| `0x80018F08` | `0x334` | JFG placeholder `func_80021444` | placeholder prohibited; remains `func_80018F08` |
| `0x8001923C` | `0x104` | `killLight` | Tier A: Mickey/DKR-adapted C is compiler/link exact |
| `0x80019340` | `0x18` | `lightGetLights` | Tier A: JFG C and both global relocations are link exact |
| `0x80019358` | `0x13C` | `lightGetStrongestEffect` | Tier A: Mickey/JFG-adapted C is compiler/link exact |
| `0x80019494` | `0xA8` | `lightUpdateObjects` | Tier A: Mickey/JFG-adapted C is compiler/link exact |
| `0x8001953C` | `0x3F8` | JFG placeholder `func_80021B9C` | placeholder prohibited; remains `func_8001953C` |
| `0x80019934` | `0xF0` | `lightDistanceCalc` | tier-B comparison: same distance-mode call surface |
| `0x80019A24` | `0x94` | `lightDirectionCalc` | Tier A: JFG C is compiler/link exact |
| `0x80019AB8` | `0x2E0` | `lightObject` | tier-B comparison: calls all three `lights2` pipelines |
| `0x80019D98` | `0x50` | `lightDefaultObjectLight` | Tier A: Mickey/JFG-adapted C is compiler/link exact |
| `0x80019DE8` | `0xFC` | `lightSetObjectLight` | unique nearest skeleton (0.704) and transform call; comparison only |
| `0x80019EE4` | `0x98` | `lightSetupLightSources` | Tier A: Mickey/JFG-adapted C is compiler/link exact |
| `0x80019F7C` | `0x8C` | `lightSetupFlareSources` | Tier A: Mickey/JFG-adapted C is compiler/link exact |
| `0x8001A008` | `0x14C` | `lightInitObjectLighting` | tier-B comparison: calls the object-light setter twice |
| `0x8001A154` | `0xE8` | `lightAdjustGlowingLight` | tier-B comparison: paired flare helper and TU order |
| `0x8001A23C` | `0x24` | `lightKillGlowingLight` | Tier A: Mickey/JFG-adapted C is compiler/link exact; Mickey uses a no-argument delete wrapper |

### 3.4 Resident controller, level and main TUs

ROM `0x25C20`-`0x2A250` is 17,968 bytes (`0x4630`) containing 108
functions. A per-function census recorded every boundary, direct caller and
callee, string reference, and the top five masked n-gram neighbours from
`tools/skeleton_scan.py`. No function in the range uses an odd-numbered
single-precision floating-point register, so §6.2 does not force any of these
functions to remain hand-written assembly.

| Canonical TU | ROM / VRAM | Bytes | Functions | Evidence |
|---|---|---:|---:|---|
| `main/joy` | `0x25C20`-`0x263F0` / `0x80025020`-`0x800257F0` | 2,000 | 19 | **Tier B:** exact ordered correspondence to JFG's controller setup/read, map accessors, stick clamp and CIC helper; Mickey's callers agree |
| `main/level` | `0x263F0`-`0x27760` / `0x800257F0`-`0x80026B60` | 4,976 | 21 | **Tier B:** exact ordered correspondence to JFG `level.c`; Mickey omits `levelGetWorldRegions` and four donor tail accessors |
| `main/main` | `0x27760`-`0x2A250` / `0x80026B60`-`0x80029650` | 10,992 | 68 | **Tiers B + C:** ordered main-state call graph plus six references to `main/main.c`; the last routine references the `x/y/z/a` coordinate readout strings |

The boundaries are all 16-byte aligned and are evidence-backed TU splits, but
they are not tier-A whole-object matches: no complete JFG object was
byte-identical. The strongest masked-skeleton anchors include
`levelUpdateColourCycling` (0.622), `levelGetNextOfWorld` (0.615),
`mainCPUeffects` (0.671), and the unnamed `func_80027EC0` (0.837). Three tiny
controller routines compare byte-identically with JFG under relocation masks,
but each has fewer than six unmasked words and therefore remains tier B under
§1.2 rather than being promoted to tier A.

The direct-call census supplies independent anchors. `joyResetMap` is called
by `joyInit`; the stick accessors converge on `joyClamp`; `levelInit` owns the
subsystem initialization/free fanout and `levelFreeAll` is reached from the
main-state loop; `mainThread` reaches `mainInitGame`, `joyRead`,
`mainChangeLevel` and `mainPreNMI`. The two large routines at `0x80026FB4` and
`0x80027FB8` build the six `main/main.c` string addresses. Placeholder-named
JFG functions were not imported: unresolved routines retain Mickey's own
`func_<VRAM>` symbol.

One initial combined symbol was corrected during reconstruction: the
108-byte routine at `0x80028E2C` has JFG `mainFrontInit`'s exact size,
top-ranked skeleton and call role; the independent return stub at
`0x80028E98` occupies JFG's following `mainStartGame` slot. Both names are
tier B because the complete donor bodies are not byte-identical.

The original 24-byte `func_80028F3C` range was likewise split at tier-D
structural boundaries: it consists of three consecutive independent
return/delay-slot islands at `0x80028F3C`, `0x80028F44`, and `0x80028F4C`.
Their placeholder names remain because JFG role attribution is not unique.

**Matching progress.** Eighty-six functions / 4,836 bytes compile exactly
under the resident `-O2 -mips2 -32` flags. Owned bytes, relocation identity,
linked ranges and the full ROM are exact.

- `main/joy` (16 / 996 bytes): `joyMessageQ`, `joyDisable`, `joyEnable`,
  `joyCreateMap`, `joyGetController`, `joyGetButtons`, `joyGetPressed`,
  `joyGetReleased`, `joyGetStickX`, `joyGetAbsX`, `joyGetStickY`, `joyGetAbsY`,
  `joyClamp`, `joySetSecurity`, `arithmeticFunction`, and `joyCharVal`.
- `main/level` (18 / 1,324 bytes): `levelNGetType`, `levelGetTune`,
  `levelGetWorld`, `levelGetRegionNo`, `levelGetScreenMode`,
  `levelGetBlurEffect`, `levelGetGfxIndex`, `levelGetColourCycling`,
  `levelGetNumber`, `levelGetLevel`, `levelGetType`, `levelGetCamera`,
  `levelTunePlay`, `levelUpdateColourCycling`, `levelGetName`,
  `levelGetNextOfWorld`, `levelGetPrevOfWorld`, and `levelInitRegionFlags`.
- `main/main` (52 / 2,516 bytes): `mainGetZBCheck`, `mainGameWindowChanging`,
  `mainGameWindowSize`, `mainSetGameWindow`, `mainSetAnimGroup`,
  `mainGetAnimGroup`,
  `mainChangeCameras`, `mainGetNextCharacter`, `mainGetNextLevel`, `mainAddZBCheck`,
  `func_80027EC0`, `func_800282C8`,
  `mainResetPressed`, `mainPreNMI`, `mainSyncNextLevel`, `mainGetMode`, `mainSetMode`,
  `mainTitlePageInit`,
  `mainFrontInit`, `mainStartGame`,
  `mainGetNumberOfCameras`, `func_80028DE4`, `func_80028EA0`, `func_80028F3C`,
  `func_80028F44`, `func_80028F4C`, `func_80028F54`,
  `func_80028F60`, `func_80028F98`,
  `func_80028FA8`,
  `func_80028FB8`,
  `func_80029038`, `func_8002904C`, `func_8002905C`, `func_80029084`,
  `func_800290A0`,
  `func_80029090`, `func_800290EC`, `func_800290F8`, `func_80029104`,
  `func_80029120`, `func_80029144`, `func_80029160`, `func_8002917C`,
  `func_80029198`,
  `func_800291B4`,
  `func_800291C4`,
  `func_800291D0`, `func_800291D8`, `func_800291E4`, `func_800291FC`, and
  `func_80029240`.

The exact source preserves Mickey's six-byte level-summary and controller-pad
layouts, packed flag extractions, bounded/wraparound searches, and guarded
input calls. `arithmeticFunction` binds its three unavailable CIC-overlay
calls to Mickey's existing `TrapDanglingJump` relocations.

Two ABI/name exceptions remain explicit. Mickey's `mainGameWindowChanging`
returns a 32-bit word, not JFG's declared `s16`; the JFG signature changed the
load and was rejected. `mainGetMode` is a tier-D paired-getter name correcting
an earlier positional setter attribution. `mainGetNumberOfCameras` is tier B
from JFG tail order plus the `levelGetGfxIndex` caller. `func_800291C4` is
consistent with `mainGetGameArrayPtr`, but not uniquely; `func_80028F54` has
the tier-B `mainGetGame` role but retains its placeholder because renaming it
would require out-of-scope overlay edits. The inherited `levelInitRegionFlags`
name is suspect: Mickey's exact 56-byte body is a boolean query over the level
type byte and `D_8007BF08`, not JFG's region-table initializer.

**Bounded plateaus (all remain assembly):**

- `mainThread`, five source/address hypotheses plus the full flag lattice,
  first object mismatch at relocation `+0x18`: the JFG-shaped candidate has
  the exact 200-byte linked instruction stream, frame and control flow, but
  its literal RAM-end address omits the target assembly's `D_803FFFFC`
  HI16/LO16 pair at `+0x18`/`+0x28`. Every symbolic spelling adds an address
  instruction and moves the aligned epilogue, growing the function by eight
  words.
- `mainUpdateZBCheck`, five loop/type hypotheses, the full flag lattice and a
  bounded two-worker permuter batch, first mismatch `+0x24`: the best
  Mickey-derived candidate has the exact `-0x48` frame and screen-size stack
  slots but compiles to 60 rather than 63 instructions. IDO schedules the
  outer counter before the target's `D_8007A24C`/`D_800D2FAC` LO16 pair and
  removes three dead-looking countdown-loop register copies retained by the
  target.
- `RevealReturnAddresses`, nine source/expression hypotheses, the full flag
  lattice and a bounded canonical-MIPS-II permuter batch, first mismatch
  `+0x24`: the best candidate preserves all 66 target opcodes, the 264-byte
  boundary, `-0x30` frame and exact relocations but has 20 register-operand
  differences. The target assigns its comparison constants and byte-patch
  temporaries in a different allocator order; the permuter improved its score
  from 225 to 120 without reaching identity.
- `levelGetCounts`, ten source/type/loop hypotheses, first mismatch `+0x13c`:
  the best candidate has the target's 1,036-byte size, 259-instruction opcode
  schedule and `-0x58` frame, but three register operands use `$v0` where the
  target uses `$a0`. Its initial count-table loop also relocates against
  `D_800CF3E0`, while the target object's HI16/LO16 pair names `D_800CF420`.
  The resident flag lattice was unchanged; a bounded two-worker MIPS II
  permuter batch improved its internal score from 45 to 25 but did not change
  these object-level residuals.
- `joyResetMap`, first mismatch `+0x0`: external storage emits 48 rather than
  36 bytes; TU-local storage is instruction-exact but wrongly claims 16 B of
  BSS and shifts the real symbol.
- `func_800290AC`, six spellings, first mismatch `+0x0`: exact 64-byte size and
  11-word tail, but five entry words differ because IDO frames before loading
  the global into `$t6`; the target loads it into `$v0` first.
- `func_80028FCC`, ten spellings, first mismatch `+0x1c`: its 108-byte skeleton
  identifies the tier-B `mainAnyoneHas` role (JFG: 108 B, similarity 0.357),
  but Mickey passes zero as every middle argument. The exact-sized candidate
  differs in ten words: raw-return branches versus target normalization into
  `$t6`/`$t7`/`$t8` and a shared epilogue.
- `levelFreeAll`, ten spellings, first mismatch `+0x13c`: exact 468-byte size
  and 113/117 words; only the masked resource index/table-base registers swap.
- `func_80028EFC`, ten spellings, first mismatch `+0x1c`: exact 64-byte size
  and 14/16 words; the correct loop predicate is allocated to `$t6`, while the
  target uses `$at`.

The full flag lattice did not change any of these allocation plateaus.

**PROVENANCE.** TU identities and adopted function names are adapted from Jet
Force Gemini's published `src/{joy,level,main}.c` and built
`src/{controller,level,main}.c.o`, a permitted public retail-derived decomp
under `docs/CLEANROOM.md`. The tier-B/C evidence above comes independently
from Mickey's own function order, callers/callees and strings. Any C body
adapted during matching carries the same disclosure at its point of use.

### 3.4 The resident debug and effects run

The four assigned ROM runs in `0x45760`–`0x4BC40` total 25,808 bytes. Including
the already-measured 16-byte `main/get_stack_pointer` island, their continuous
span is 25,824 bytes in five source units. The four new C splits below own 75
functions; none uses an odd single-precision register,
so the hand-written-assembly test in §6.2 excludes none of them. The `fx` range
has 16,840 executable bytes and eight bytes of compiler alignment padding.

| ROM | Source unit | Functions | Tier | Evidence |
|---|---|---:|---|---|
| `0x45760`–`0x459C0` | `main/diRcpTrace` | 4 | B | JFG's `src/diRcpTrace.c` has the same four-function order and near-identical sizes. Mickey's scheduler/track callers and the trace-buffer consumer establish the roles. |
| `0x459C0`–`0x465B0` | `main/diRcp` | 18 | B/C | The complete GBI opcode/mode string set identifies the disassembler (C); `diRcpPrintDL` calls the same ordered helper family as JFG's `src/diRcp.c` (B). |
| `0x465B0`–`0x47A60` | `main/diCpu` | 14 | A/B/C | `diCpuTraceInit` is a 21-word Tier-A skeleton/object hit, the exception/watchpoint strings identify the monitor (C), and the OS-thread/debug call graph follows JFG's `src/diCpu.c` (B). The end is pinned by the measured `get_stack_pointer` TU at `0x47A60`. |
| `0x47A70`–`0x4BC40` | `main/fx` | 39 | B/D | Mickey begins where JFG's `src/fx.c` reaches `fxFreeCone`: the cone and wake routines have the same allocator, texture, trigonometry and draw call graph in the same order (B). The later unresolved effects retain Mickey `func_` names (D). The next block contains JFG `font.c` hits, independently fixing the far end. |

The strongest `fx` call-graph pairs are structural rather than merely
positional: `fxAllocateCone` calls the allocator, texture loader and the same
three cone builders; `wakeSetupRipple` calls the alignment helper, texture
loader and `wakeAllocate`; `wakeUpdateRipple` calls `Arctanf` and
`wakeUpdate`; and `wakeDrawRipple` calls the texture setup/draw pair and
`wakeDraw`. The earlier JFG level-effect functions are absent, which is why
Mickey's TU begins at `fxFreeCone` instead of JFG's first `fx.c` symbol.

Pre-existing assembly callers still spell 18 of these targets as
`func_<VRAM>`. Those exported labels are retained in `symbol_addrs.us.txt`,
with the JFG identity and tier on the same row, until each function or its
caller becomes C-owned; this avoids pretending that a source-level rename is
already available to the stale generated caller assembly.

Exact C closures in these splits begin with 68 bytes across two `diCpu`
functions: the 8-byte `func_80046504` (`diCpuTraceGetFault` in JFG) and the
60-byte `func_8004650C` (`diCpuTraceTick`). Their natural return-zero and
60-tick counter bodies are identical under the resident `-O2 -mips2 -32`
rule; the getter has no relocations and the tick routine retains both exact
HI16/LO16 data pairs. Five JFG `diRcp` return-eight leaves are also exact at
the resident defaults with no relocations: 16-byte `diRcpReserved0`, 20-byte
`diRcpStrNameMacro`, 12-byte `diRcpPrimColor`, 20-byte `diRcpColor`, and
12-byte `diRcpDmaOffsets`. Six 52-byte JFG unpack-and-return bodies,
`diRcpVertex`, `diRcpReserved1`, `diRcpMatrix`, `diRcpReserved2`,
`diRcpMoveMem`, and `diRcpDisplayList`, are exact at the same defaults,
including their helper-call relocations and source-specific stack frames. The
52-byte `diRcpStrName` formatter is exact as well, including its format-string
and `sprintf` relocations. The 44-byte `func_80044B9C` (`diRcpTraceReset`) is
exact too, including both data-symbol relocation pairs.
The 60-byte `diRcpTraceInit` is likewise exact, preserving both allocator
calls and their call/data relocations. The 60-byte JFG-identified `wakeFree`
is exact after resolving `func_800347A0` as a one-argument call; its two call
relocations and the wake-linked field access match without normalization. The
same ABI resolves the adjacent 72-byte `func_80048980` (`wakeFreeRipple`),
which is exact with both its linked-release and nested-wake call relocations.
The 84-byte `func_80046E70` (`fxFreeCone`) is exact too: two distinct texture
handle locals reproduce the target's direct second argument register and
branch-delay schedule, with both texture-free calls and the allocator call
retaining their exact relocations under the resident defaults.
The adjacent 52-byte `func_8004707C` is exact without relocations: its six
full-width value parameters are stored into byte fields only after the null
check, preserving the target's leaf schedule under the same default flags.
The 108-byte JFG-identified `fxQueueScreenEffect` is also exact: expressing
the four-entry queue selection as an array subscript with a post-incremented
global count reproduces the target's 20-byte offset schedule and both data
relocation pairs under the resident defaults.
Its 172-byte dequeue sibling `func_8004A9CC` (`fxUnQueueScreenEffect`) is exact
on the natural pointer/count loop, including the 64-byte frame, all nine
arguments to `fxScreenEffect`, the call relocation, and both queue-global
relocation pairs.
The 60-byte Mickey-named `func_80049828` bounds-checks one of five effect
records and tests a caller-supplied flag mask; its natural 32-byte-stride
record access is exact at the resident defaults, including the data-symbol
relocation pair. Its adjacent 56-byte `func_80049864` sibling tests a byte
status field with the same bounds and stride and is exact under the same flags,
also with the target's data-symbol relocation pair. The following 96-byte
`func_8004989C` packs the record's RGB bytes into a duplicated 16-bit color;
the typed record body, expression schedule, and data relocation are exact at
the resident defaults. The 28-byte `func_8004A0F0` clears two adjacent effect
queue words and their index; its three stores and both data relocation pairs
are exact under the same defaults.
The 76-byte JFG-identified `fxInit` is exact as well: its post-decrement loop
clears all five 32-byte records, resets the global state, and preserves the
callee plus two data relocation pairs without normalization.
The 136-byte Mickey-named `func_80049A8C` resets either one record or all five,
clearing state/status and two flag bits. Its selection branches, stack home,
countdown loop, and data relocation pair are exact at the resident defaults.
The 180-byte `func_8004AD34` (`fxGenerateTextures` in JFG) is exact too. Its
four-entry descending callback loop, flag test, callback-table refresh, and
indirect call retain all target instruction words and relocation identities at
the resident defaults; spelling the constant-count loop as `while (index--)`
reproduces IDO's rotated `3`-through-`0` schedule without normalization.

**PROVENANCE.** The TU identities and descriptive names in this subsection,
`symbol_addrs.us.txt`, and the four `src/main/*.c` files are adapted from Jet
Force Gemini's public decompilation (`src/diRcpTrace.c`, `src/diRcp.c`,
`src/diCpu.c`, and `src/fx.c`). JFG is a permitted published decomp under
`docs/CLEANROOM.md`; Mickey's own bytes, strings and linked call graph decide
every mapping. JFG address placeholders are not imported.

### 3.4 `main/charControl`

ROM `0x1C790`–`0x20020`, VRAM `0x8001BB90`–`0x8001F420`, is split as
`src/main/charControl.c`. The endpoints retain splat's original aligned
file-boundary candidates, but the assignment no longer rests on that heuristic
alone. At the start, the first six functions follow JFG's `charControl.c`
camera-control cluster by masked-skeleton similarity and call graph. Inside the
block, `func_8001C2D4` and `controlSetPlayerSetup` are tier-A skeleton anchors
from JFG's built `src/charControl.c.o`. At the tail, the latter is followed by
the setup getter and clearer behavior in JFG's order. The next yaml block
begins at `0x20020` with a separately tier-A function from JFG's `models.c.o`.
Together these are **B/D TU-boundary evidence**, not a whole-`.text` tier-A
match; the distinction is why §3.3's original two-hit row did not itself draw
the split.

**PROVENANCE:** JFG's public `src/charControl.c`, `src/charControl.h`, built
object, public symbol map, and `asm/nonmatchings/charControl` filenames supplied
the names in the comparison column below. Only the two tier-A rows were already
adopted in `symbol_addrs.us.txt`. Tier-B/D names remain comparison leads while
their functions use `GLOBAL_ASM`; §1.5 therefore keeps Mickey's `func_` names
until matching C independently establishes a role strongly enough to adopt
one. A dash means that neither the JFG order, masked similarity, nor the current
call graph isolates one namesake.

| Mickey VRAM | Size | JFG comparison lead | Evidence / current disposition |
|---|---:|---|---|
| `0x8001BB90` | `0x24` | `cameraGetBlend` | D + matched C: exact 0x2C-stride float getter under O2/mips2; JFG comparison remains structural, so retain `func_` |
| `0x8001BBB4` | `0x258` | `func_8002B378` | B: camera call graph and next-function edge; JFG placeholder, retain `func_` |
| `0x8001BE0C` | `0x248` | `func_8002EDA0` | B: camera-pointer lookup then the preceding routine; JFG placeholder, retain `func_` |
| `0x8001C054` | `0x34` | `cameraAddOverrideObject` | D + matched C: exact 24-entry append under O2/mips2; JFG comparison remains structural, so retain `func_` |
| `0x8001C088` | `0x8C` | `cameraDeleteOverrideObject` | D + matched C: exact 24-entry search-and-delete under O2/mips2; JFG comparison remains structural, so retain `func_` |
| `0x8001C114` | `0x1B0` | `func_8002F0E8` | Plateau after the 119-combination flag sweep and 10 source/lifetime hypotheses: best `NON_MATCHING` candidate preserves the target CFG in 106 instructions versus 108, with first mismatch `+0x4`; the target maps `x/y/z` to `f12/f22/f20` and saves `f22`/`f20`, while IDO maps the candidate to `f20/f14/f12` and saves only `f20`. JFG has the same target allocation but only a placeholder body; retain `func_` |
| `0x8001C2C4` | `0x10` | — | Two return stubs under one measured label; retain `func_` |
| `0x8001C2D4` | `0x4C` | `func_80031F60` | A + matched C: 19/19 unmasked JFG words and independently reconstructed byte-clear C are exact; placeholder rule retains Mickey's `func_` |
| `0x8001C320` | `0x1A0` | `controlPlayerReInit` | B + matched C: exact 104-instruction save/clear/reinitialize/restore wrapper under O2/mips2 with `-Wab,-r4300_mul`; its role and call graph mirror JFG, so the name is adopted |
| `0x8001C4C0` | `0x64C` | `controlPlayerInit` | B: initialization calls and caller edge from the preceding routine; retain `func_` |
| `0x8001CB0C` | `0x78` | — | Matched C: exact one-point transform setup under O2/mips2; no unique JFG comparison, so retain `func_` |
| `0x8001CB84` | `0x71C` | `controlPlayer` | D: large per-frame controller in JFG order; retain `func_` |
| `0x8001D2A0` | `0x17C` | — | Plateau after the 119-combination flag sweep and 10 source/type/lifetime hypotheses: best `NON_MATCHING` candidate is 96 instructions versus 95, with first mismatch `+0xE0`; IDO reuses the `D_800CB300` address across `func_800291F0`, while the target stores through `at` and rematerializes the address in `a2`, leaving one extra address instruction and shifting the remaining schedule. No unique JFG comparison; retain `func_` |
| `0x8001D41C` | `0x21C` | — | Matched C: exact 135-instruction timer, effect-spawn, and action-callback body under O2/mips2 with `-Wab,-r4300_mul`; the mandatory 119-combination sweep found no alternate flag improvement and no unique JFG comparison, so retain `func_` |
| `0x8001D638` | `0x58` | `controlFrozen` | B + matched C: exact pause/input gate under O2/mips2; calls the following restart routine as JFG does; name adopted |
| `0x8001D690` | `0x194` | `controlRestartPlayer` | B + matched C: exact 101-instruction multiplayer respawn-point search and single-player restart fallback under O2/mips2 with `-Wab,-r4300_mul`; JFG has the same role and nearest charControl skeleton, but Mickey retains `func_` because IDO's allocation changes under the public name |
| `0x8001D824` | `0x5C` | `dAngle` | B + matched C: same wrapped-angle role/body as JFG, whose MIPS-I conversion sequence is longer; adapted Mickey C is ADR 0001 exact under O2/mips2 |
| `0x8001D880` | `0x90` | `controlMakeV` | Plateau after flag sweep and 10 source/flag hypotheses: best `NON_MATCHING` candidate is exact-size with `-Wab,-r4300_mul`, 29 words differ from FP/register allocation at first mismatch `+0x4`; retain `func_` |
| `0x8001D910` | `0x50` | `controlFSUvels` | B + matched C: JFG rotation-vector role/body with Mickey's output at player `+0x14`; adapted C is ADR 0001 exact under O2/mips2 |
| `0x8001D960` | `0x370` | `controlUpdateJetFlames` | D: nearest JFG charControl skeleton and same subsystem order; retain `func_` |
| `0x8001DCD0` | `0xA0` | — | Plateau after flag sweep and 10 source shapes: best `NON_MATCHING` candidate has the exact 40-opcode/register/frame schedule under O2/mips2 with `-Wab,-r4300_mul`, but 2 stack-offset words differ from first mismatch `+0x70`; an extra FP temporary home moves the transformed-value spill from `sp+0x1C` to `sp+0x24`; retain `func_` |
| `0x8001DD70` | `0x854` | `controlGroundHits` | D: collision/movement structure and JFG order; retain `func_` |
| `0x8001E5C4` | `0x680` | `controlHangOK` / `controlGrabOK` | D: ledge/collision family, not uniquely separated; retain `func_` |
| `0x8001EC44` | `0x3B8` | `controlSquashCheckPrior` | D: collision/math structure and JFG order; retain `func_` |
| `0x8001EFFC` | `0xA0` | — | Matched C: exact point-list transform and translation loop under O2/mips2; no unique JFG comparison, so retain `func_` |
| `0x8001F09C` | `0xB0` | `func_800370D8` | D + matched C: exact target-smoothing body under O2/mips2 with `-Wab,-r4300_mul`; JFG placeholder comparison remains structural, so retain `func_` |
| `0x8001F14C` | `0x110` | `controlCeiling` | D + matched C: exact offset/spawn/effect body under O2/mips2; JFG comparison remains positional, so retain `func_` |
| `0x8001F25C` | `0x8` | `controlDisableJoypad` | B + matched C: caller supplies player and boolean, next routine tests the stored state; JFG has the same role but a one-argument global implementation |
| `0x8001F264` | `0xBC` | `controlReadJoypad` | B + matched C: calls all seven stick/button readers in JFG order; adapted per-player C is ADR 0001 exact under O2/mips2 |
| `0x8001F320` | `0x44` | `controlSetRumble` | B + matched C: sole call is the rumble dispatcher under player-state guards; Mickey-derived wrapper is ADR 0001 exact under O2/mips2 |
| `0x8001F364` | `0x8` | — | Matched C: empty routine, ADR 0001 byte-identity; retain `func_` |
| `0x8001F36C` | `0x40` | `controlSetPlayerSetup` | A + matched C: 6 unmasked of 16 JFG words established the name; Mickey-derived four-halfword/valid-byte body is ADR 0001 exact |
| `0x8001F3AC` | `0x5C` | `controlGetPlayerSetup` | B + matched C: consumes and clears the exact state written by the tier-A setter; adopted with point-of-use JFG provenance and ADR 0001 byte-identity |
| `0x8001F408` | `0xC` + `0xC` padding | `controlClearPlayerSetup` | B + matched C: clears the setup-valid byte; adopted with point-of-use JFG provenance and ADR 0001 byte-identity |

No function in this TU uses an odd single-precision FP register, so §6.2 does
not classify any of them as hand-written assembly. The `0xC` bytes after
`func_8001F408` are alignment padding, not executable ownership.

#### Audio-manager census and conservative source split

Census of yaml's former `0x1050`-`0xC950` assembly surface found **152
functions**: 82 in `0x1050`, 5 in `0x45F0`, and 65 in `0x4F40`. The assigned
JFG audio-manager family covers 74; the remainder has separate lineage.

| Mickey ROM range | Functions | Attribution and evidence | Canonical treatment |
|---|---:|---|---|
| `0x1050`-`0x2340` | 49 | JFG `audio_manager_1050.c`: **tier A** at `amTuneSetFadeScaled`, `amSndSetPan`, `forcelink`; **tier B** API order/calls. The aligned end precedes JFG's separate `audiomgr` initializer | `src/main/audio_manager_1050.c` |
| `0x2340`-`0x3100` | 13 | JFG `audiomgr.c`; **tier B** allocator/queue/scheduler/DMA/frame-state calls; outside the assigned TUs | assembly; boundaries recorded |
| `0x3100`-`0x45F0` | 20 | JFG `audio_manager_36D0.c`; **tier B** start allocator, 20-function order, positional setters, and terminal volume calculation. `audspat_jingle_off`/JFG `amAmbientPause` is a title-specific naming divergence | `src/main/audio_manager_36D0.c` |
| `0x45F0`-`0x4F40` | 5 | JFG `audio_manager_4C50.c`; **tier A** endpoints (`amVibratoInit`, `_depth2Cents`), five-function order, and `0xC` terminal alignment | `src/main/audio_manager_4C50.c` |
| `0x4F40`-`0xC950` | 65 | JFG `objects.c` lineage follows the oscillator TU; **tier A** `GetRomlistInfo`, but no whole-object match or promoted boundary | assembly |

Matched C bodies in these new TUs:

All rows use IDO 5.3 `-O2 -mips2 -32` and are linked-ROM exact unless noted.
The final column records owned object words and relocation coverage.

| Mickey routine | ROM / size | Name evidence | Match evidence |
|---|---:|---|---|
| `amTuneResetFade` | `0x1330` / `0xC` | **tier B**: exact JFG routine order and the adjacent tune-fade controller role | Exact object words and linked ROM bytes |
| `amAmbientResetFade` | `0x142C` / `0xC` | **tier B**: exact JFG routine order and the adjacent ambient-fade controller role | Exact object words and linked ROM bytes |
| `amTuneMuteChl` | `0x17E8` / `0x8` | **tier B**: exact JFG routine order between the channel-mask setter and its paired unmute leaf | Exact object words and linked ROM bytes |
| `amTuneUnmuteChl` | `0x17F0` / `0x8` | **tier B**: exact JFG routine order immediately after its paired mute leaf | Exact object words and linked ROM bytes |
| `amTuneSetChlVolume` | `0x17F8` / `0x40` | **tier B**: JFG routine order and exact channel-bound/call role; its 1.000 skeleton is ambiguous with DKR's pan/volume/fade wrappers and is not tier A | Exact 16 object words and both data/call relocation identities |
| `amTuneResetChls` | `0x1838` / `0x64` | **tier B**: exact JFG routine order and the paired unmute/full-volume loop role | Exact 25 object words and all global/call relocation identities |
| `amAmbientPlay` | `0x189C` / `0x50` | **tier B**: JFG and DKR agree on the official role; the current-sequence assignment, ambient player, playing guard, and sequence-start call match exactly | Exact 20 object words and all global/call relocation identities |
| `amTuneStop` | `0x18EC` / `0x30` | **tier B**: JFG and DKR agree on the official role; the tune-change block and tune-player stop call pin the identity | Exact 12 object words and both global/call relocation identities |
| `amAmbientStop` | `0x191C` / `0x38` | **tier B**: JFG and DKR agree on the official role; the playing guard, ambient-ID reset, and ambient-player stop call pin the identity | Exact 14 object words and all global/call relocation identities |
| `amTuneGetSeqNo` | `0x1954` / `0x3C` | **tier B**: JFG and DKR agree on the official role; the current-tune guard and tune-player `AL_PLAYING` state check pin the identity | Exact 15 object words and all data relocation identities |
| `amAmbientGetSeqNo` | `0x1990` / `0x0C` | **tier B**: JFG and DKR agree on the official return role; Mickey returns the same current-ambient global used by the play/stop pair | Exact 3 object words and data relocation identity |
| `amTuneSetVolume` | `0x199C` / `0x6C` | **tier B**: JFG supplies the full body and official name; the clamp, saved base volume, scaled tune-player call, and update flag agree exactly | Exact 27 object words and all data/call relocation identities |
| `amTuneSetGlobalVolume` | `0x1A08` / `0x5C` | **tier B**: JFG supplies the full body and official name; the global-volume clamp, saved scale, and recalculated tune-player call agree exactly | Exact 23 object words and all data/call relocation identities |
| `amTuneGetVolume` | `0x1A64` / `0x0C` | **tier B**: JFG and DKR agree on the official return role; Mickey returns the base-volume global written by `amTuneSetVolume` | Exact 3 object words and data relocation identity |
| `amAmbientSetVolume` | `0x1A70` / `0x4C` | **tier B**: JFG supplies the full body and official name; the saved relative volume and sound-global-scaled ambient-player call agree exactly | Exact 19 object words and all data/call relocation identities |
| `amDittyPlay` | `0x1ABC` / `0x64` | **tier B**: JFG has the same exact boundary and sequence-table guard/current-ID/player-start role; `skeleton_scan.py` ranks it first at 0.571, not tier-A identity | Exact 25 object words and all data/call relocation identities |
| `amDittyPlaying` | `0x1B20` / `0x54` | **tier B**: JFG has the same exact boundary and DKR supplies the official role; current-ID, enabled, and ambient-player-state guards agree exactly | Exact 21 object words and all data relocation identities |
| `amSndStop` | `0x1B74` / `0x20` | **tier B**: JFG supplies the complete one-call body and official name; the target is below the skeleton oracle's 10-word confidence floor | Exact 8 object words and call relocation identity |
| `amSndPlay` | `0x1B94` / `0x104` | **tier B**: JFG has the same exact boundary and direct-player call shape; DKR supplies the official role and `SoundData` interpretation | Exact 65 object words and all data/call relocation identities |
| `amSndPlayDirect` | `0x1C98` / `0xAC` | **tier B**: JFG supplies the official name, parameter roles, range check, scaler, and direct-player call shape; Mickey's branch-likely form is four bytes shorter | Exact 43 object words and all data/call relocation identities |
| `amSndSetVol` | `0x1D44` / `0xC0` | **tier B**: JFG and DKR agree on the official role; base-volume lookup, relative scaling, resident scaler, and volume-parameter call agree exactly | Exact 48 object words and all data/call relocation identities |
| `amSndSetPitchDirect` | `0x1E2C` / `0x2C` | **tier B**: JFG and DKR agree on the official name and parameter role; the handle guard and pitch-parameter call agree exactly | Exact 11 object words and call relocation identity |
| `amGetSfxCount` | `0x1E58` / `0x18` | **tier B**: JFG supplies the complete body and official name; the bank/instrument traversal and sound-count field agree exactly | Exact 6 object words and data relocation identity |
| `amGetSfxSettings` | `0x1E70` / `0x38` | **tier B**: JFG supplies the complete body and official name; the optional table/size/count outputs and their globals agree exactly | Exact 14 object words and all data relocation identities |
| `amSoundIsLooped` | `0x1EA8` / `0x60` | **tier B**: JFG and DKR agree on the official role and body; the sound-count bound, sound-array traversal, and infinite-decay test agree exactly | Exact 24 object words and data relocation identity |
| `stop_ALSeqp` | `0x2168` / `0x88` | **tier B**: JFG name/body and Mickey's two-player stop state machine agree | Exact 34 object words and all call/data relocations |
| `amTuneSetReverbOnOff` | `0x21F0` / `0x8` | **tier B**: JFG supplies the name and no-op body | Exact 2 object words; no relocations |
| `func_800015F8` | `0x21F8` / `0x10` | **tier D**: direct write of one to the resident audio flag; no external name is asserted | Exact 4 object words and data relocation identity |
| `func_80001608` | `0x2208` / `0xC` | **tier B**: overlay 46 calls this routine at its sequence-transition exit, corroborating the direct resident audio-flag clear; no external name is asserted | Exact 3 object words and data relocation identity |
| `func_80001614` | `0x2214` / `0xC` | **tier B**: a resident caller branches on this direct audio-flag read; no external name is asserted | Exact 3 object words and data relocation identity |
| `func_80001620` | `0x2220` / `0x48` | **tier B**: a resident caller consumes the range-checked sound-table volume; no external name is asserted | Exact 18 object words and both data relocations |
| `func_80001668` | `0x2268` / `0x30` | **tier D**: guarded sound-volume parameter wrapper; no external name is asserted | Exact 12 object words and call relocation |
| `scalevol` | `0x22C8` / `0x24` | **tier B**: JFG supplies the complete body and official name | Exact 9 object words; no relocations |
| `func_800016EC` | `0x22EC` / `0x1C` | **tier B**: overlay 49 supplies mode-call context; no external name is asserted | Exact 7 object words and two data relocations |
| `func_80001708` | `0x2308` / `0x38` | **tier B**: a resident caller pins the master-volume reset role; no external name is asserted | Exact 14 object words, two calls, and data relocation |
| `amSndSetPan` | `0x1E04` / `0x28` | existing **tier A** JFG byte identity | Exact object words and relocation identity |
| `forcelink` | `0x2298` / `0x30` | existing **tier A** JFG byte identity | Exact object words and both call relocations |
| `_depth2Cents` | `0x4EE4` / `0x50` | existing **tier A** JFG byte identity, independently corroborated by BK's compiled object | IDO 5.3, `-O2 -mips2 -32 -Wab,-r4300_mul`; exact object words/relocations, with `0xC` target padding excluded |

Measured plateau:

| Mickey routine | Best result | First mismatch | Remaining hypothesis |
|---|---|---:|---|
| `amTuneSetFadeScaled` | Exact 29-word instruction/opcode schedule, frame, and relocation surface; 7 register-only differences after the flag lattice and 10 source-shape attempts | function `+0x1C` | IDO 5.3 temporary-FIFO phase: the target and candidate assign the three initial address/index temporaries from different positions in the same ring. The candidate remains under `NON_MATCHING`; canonical output is still assembly-backed |

PROVENANCE: TU labels, order, and semantic roles derive from JFG's permitted
public decomp/objects. C retains Mickey-owned stubs and point-disclosed adapted
bodies; every promotion remains byte-exact to Mickey.

---

### 3.17 Vehicle sounds, models and gsSnd census

These three existing 16-byte-aligned splat boundaries were moved from raw
`asm` subsegments to C translation units with one `GLOBAL_ASM` per function.
That changes ownership, not bytes. **PROVENANCE:** JFG's permitted
`src/audio_manager_36D0.c`, `audio.h`, `src/models.c`, `models.h`,
`src/camera.c`, `src/gsSnd.c` and `src/gsSnd.h` were read while identifying the
APIs and candidate names. The initial split adapts no body from them.

Every function was checked with `skeleton_scan.py similar --top 5`. ROM
`0x58E50`–`0x59B90` produced no exact JFG match. The only exact function in
ROM `0x5B300`–`0x5C310` is `camConvertMatrixList` (12 words, 4 relocation-
masked, ROM-wide unique); the loader/free pair's nearest JFG shapes are in
`models.c` but are non-exact, so their Mickey address names remain. The entire
`gsSnd` object is already a Tier-A match; its per-function scan re-confirmed
every function of at least 10 words except the ambiguous placeholder at
`0x8005CD3C` and the final `gsSndpLimitVoices`, whose standalone bound omits
the object's four padding bytes. `gsSndpGetGlobalVolume` is below the scanner
floor. The whole-object match carries all three without importing a
placeholder name or counting padding as function text.

| ROM / VRAM | Size | Function | Evidence and call-graph role |
|---|---:|---|---|
| `0x58E50` / `0x80058250` | `0x58` | `func_80058250` | D: clears four positional engine-sound slots; called from resident audio setup |
| `0x58EA8` / `0x800582A8` | `0x64` | `func_800582A8` | B: stops those four handles; called from the main state-transition path |
| `0x58F0C` / `0x8005830C` | `0xBE8` | `func_8005830C` | D: walks active racers and maintains two positional sounds from speed and listener distance; no per-symbol caller argument recorded |
| `0x59AF4` / `0x80058EF4` | `0x9C` | `func_80058EF4` | D: local logarithm-series helper used to derive Doppler pitch |
| `0x5B300` / `0x8005A700` | `0x64` | `func_8005A700` | D: allocates animation table/cache storage |
| `0x5B364` / `0x8005A764` | `0x0C` | `func_8005A764` | D: resets the pending-animation counter |
| `0x5B370` / `0x8005A770` | `0x30` | `func_8005A770` | D: flushes the pending animation table, then resets its count; no per-symbol caller argument recorded |
| `0x5B3A0` / `0x8005A7A0` | `0x1A8` | `func_8005A7A0` | D: loads a model's animation-ID table and allocates its animation pointer array; no per-symbol caller argument recorded |
| `0x5B548` / `0x8005A948` | `0x178` | `func_8005A948` | D: reference-counted single-animation loader; nearest non-exact JFG `models.c` skeleton |
| `0x5B6C0` / `0x8005AAC0` | `0xB8` | `func_8005AAC0` | D: releases one reference-counted animation; nearest non-exact JFG `models.c` skeleton |
| `0x5B778` / `0x8005AB78` | `0x30` | `camConvertMatrixList` | A: exact JFG `camera.c` helper, used by the matrix builder below |
| `0x5B7A8` / `0x8005ABA8` | `0x1BC` | `func_8005ABA8` | D: advances/clamps the current animation frame |
| `0x5B964` / `0x8005AD64` | `0x1B0` | `func_8005AD64` | D: selects an animation and establishes its frame/blend state; no per-symbol caller argument recorded |
| `0x5BB14` / `0x8005AF14` | `0x730` | `func_8005AF14` | B: builds model matrices and transformed attachment points, then calls `camConvertMatrixList` to queue matrix conversion |
| `0x5C244` / `0x8005B644` | `0xCC` | `func_8005B644` | D: constructs a parented matrix list for the builder |

The `gsSnd` function boundaries are: `gsSndpNew` `0x268`,
`func_8005B978` `0xC8`, `func_8005BA40` `0x12FC`, `func_8005CD3C` `0x70`,
`func_8005CDAC` `0x7C`, `func_8005CE28` `0x104`,
`getSoundStateCounts` `0x104`, `func_8005D030` `0x230`,
`func_8005D260` `0x144`, `gsSndpSetPriority` `0x28`, `gsSndpGetState`
`0x30`, `ad_sndp_play` `0x2E8`, `gsSndpStop` `0x80`,
`sndp_stop_with_flags` `0xBC`, the three `gsSndpStopAll*` wrappers `0x28`
each, `gsSndpSetParam` `0x7C`, `gsSndpGetMasterVolume` `0x2C`,
`gsSndpSetMasterVolume` `0xE0`, `gsSndpSetGlobalVolume` `0x28`,
`gsSndpGetGlobalVolume` `0x1C`, and `gsSndpLimitVoices` `0x48` followed by
four bytes of TU padding. All are inside the measured Tier-A `gsSnd.c` object.

The direct strings are confined to `gsSnd`: state-count diagnostics and bad
event/play-state diagnostics in `func_8005BA40`, allocation failure in
`ad_sndp_play`, and the existing null-handle warnings in `gsSndpStop` and
`gsSndpSetParam`. The other two ranges have no direct string reference. None
of the 38 functions uses an odd single-precision FP register, so §6.2's
hand-written-assembly exclusion removes no candidate from these ranges.

**Exact C promotions:** `getSoundStateCounts`, `gsSndpSetPriority`,
`gsSndpGetState`, `gsSndpStopAll`, `gsSndpStopAllRetrigger`,
`gsSndpStopAllLooped`, `gsSndpGetMasterVolume`, `gsSndpSetGlobalVolume`,
`gsSndpGetGlobalVolume`, `gsSndpLimitVoices`, `gsSndpStop` and
`gsSndpSetParam`, together with `sndp_stop_with_flags` and
`gsSndpSetMasterVolume` (`0x524` bytes total), are adapted JFG bodies compiled
with the TU's measured bare `-g -mips2 -32` flag group. Their linked owned
ranges are instruction-word-identical and the full ROM retains the expected
hash. Mickey-derived player initializer `gsSndpNew`, callback
`func_8005B978`, `func_8005CD3C`, event-queue unlinker `func_8005CE28`, and
sound-state allocator/releaser pair `func_8005D030`/`func_8005D260` add
another exact `0x818` bytes under the same flags, bringing exact C in
`main/gsSnd` to `0xD3C` bytes; JFG retains all six functions as assembly, so
their bodies are not donor adaptations.

`ad_sndp_play` sits inside this JFG-matched TU (its name, like every other
symbol in the whole-`.text` block above, comes from JFG's built `gsSnd.c.o`,
per the tier-A whole-TU byte match), but its *body* is not a JFG adaptation:
JFG keeps this function as assembly with no C source. The C written for it is
adapted from the corresponding permitted DKR/PD sound-player sequence logic
and then proved against Mickey, adding `0x2E8` exact bytes. Its nested
play/retrigger event lifetimes are required for IDO's target delay-slot
schedule. Exact C in `main/gsSnd` therefore totals `0x1024` bytes.

The permitted-PD-derived event dispatcher `func_8005BA40` reaches all 1,215
target instruction words under the measured bare `-g -mips2 -32` group, with
the target frame and register allocation, but is not promoted or credited.
Its switch and diagnostics emit a `0x150`-byte rodata section that is still
owned by the shared `0x81590` yaml slice; compiling both copies prevents an
exact canonical link. Promotion therefore requires a measured rodata-boundary
handoff in `mickey.us.yaml`, outside this lane's assigned files. The exact-text
candidate remains under `NON_MATCHING` and target assembly stays canonical.

The adjacent pitch-event helper `func_8005CDAC` plateaus after ten coherent
source and flag variants. Its best permitted BK/PD-derived body under the
measured bare `-g -mips2 -32` group emits 30 instructions with a `0x28` frame,
versus the target's 31 and `0x30`; the first mismatch is `+0x2C`. The target
copies the pitch word through an integer stack address while this IDO/header
combination scalarizes it as an FP copy. The best body remains under
`NON_MATCHING`; target assembly is canonical and contributes no exact bytes.

In `main/models`, `camConvertMatrixList`, initialization helper
`func_8005A700`, and the counter reset/flush pair `func_8005A764` and
`func_8005A770` (`0xD0` bytes total) are exact under the resident
`-O2 -mips2 -32` group. The first is adapted from JFG `camera.c`, and the
initialization helper from JFG `models.c`. Their function bytes and relocation
identities match in the linked ROM.

Mickey-derived parented matrix-list builder `func_8005B644` adds `0xCC`
exact bytes under the TU's measured `-Wo,-loopunroll,0` override, bringing
exact C in `main/models_5B300` to `0x19C` bytes. Its 51 instructions, `0x88`
frame, calls and relocation identities match; JFG retains the corresponding
model-matrix routine as assembly, so no donor body was adapted.

The `func_8005A948` flag lattice additionally establishes
`-Wo,-loopunroll,0` for `main/models`: without it IDO unrolls the cache scan
to 166 instructions, while the disabled-unroll candidate has the target's 94
instructions, frame, opcodes, CFG and relocation identities. That candidate
plateaus at 31 differing words (28 register operands and three accesses to a
spill at `0x34(sp)` rather than `0x30(sp)`), first diverging at function offset
`+0x40` when the cache-index shift is allocated to `t7` instead of `t8`. Seven
coherent source/flag attempts did not move that allocator split, so its best C
is retained under `NON_MATCHING` and the target assembly remains canonical.

`func_8005A7A0` plateaus at 105 instructions against 106, with a `0x58`
frame against `0x38` and 73 differing positional words from the prologue;
pointer/array loop variants and all 119 flag combinations retain the excess
live ranges. The closer `func_8005AAC0` release loop emits 47 instructions
against 46, while `func_8005ABA8` emits 110 against 111 and first diverges at
`+0x38` before an FP-allocation cascade. `func_8005AF14` remains a structural
plateau because neither Mickey's current types nor JFG's assembly-only peer
establish its model-node and attachment layouts.

In `main/vehicle_sounds`, the Mickey-derived handle cleanup loop
`func_800582A8` (`0x64` bytes) is exact under `-O2 -mips2 -32`; its linked
function bytes and call relocation match.

The remaining vehicle functions plateau without exact credit.
`func_80058250`'s best direct initializer emits 26 instructions against 22,
first differing at `+0x8`; the array spelling undershoots the target's unusual
address schedule. `func_80058EF4`'s best lattice result emits 36 instructions
against 39 and differs in 13 words from `+0x4`, with the two FP webs exchanged.
The 0xBE8-byte `func_8005830C` remains blocked on untyped racer, listener and
sound-state layouts for which the permitted JFG audio-manager sources provide
no corresponding body.

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

All 45 `n_audio` TUs are matched (`n_cspsetvol` and `cents2ratio` were
adopted before this pass; the other 43 use disclosed JFG bodies). This includes
every masked=0/1/2 TU (the
thin `N_ALEvent` posters and one-line accessors), `n_sl` (which places
the driver singletons `n_alGlobals`/`n_syn` — VRAM `0x80080160`/
`0x80080164`, measured directly off a built candidate diffed against the
ROM with `tools/wb_compare.sh --rom n_alInit --show-diff` and carved out
of the resident `.data` band in `mickey.us.yaml`, since almost everything
else in the library reads `n_syn`), the seven `ALParam`-update setters
that funnel through it, `n_synallocfx`, `n_alcspchan` (needs
`-DRAREDIFFS` for Rare's added MIDI control-change codes), and
`n_syngetfxref`. The final TU, `n_synfreevoice`, contributes `0xD4` executable
bytes plus `0xC` bytes of compiler section alignment. Its five relocations
retain the exact count, types, offsets, and symbol identities, and the full
`0xE0`-byte `.text` section is byte-identical to the whole-object JFG donor.
The `0x3220`-byte `n_csplayer` text and its owned
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

`alsurround` also owns a `0x10`-byte `.bss` section at Mickey VRAM
`0x800D7DC0`: the two linked functions' HI16/LO16 references place its
four donor-named globals there in declaration order, independently of the
JFG addresses embedded in their imported identifiers.

`n_event` owns the following `0x10`-byte `.bss` section at VRAM
`0x800D7DD0`; ten linked HI16/LO16 references independently place its
external-event clock at the section start.

`n_synsetfxparam` additionally owns the `0x10`-byte `.rodata` island at
ROM `0x853B0`; separating it from the formerly undifferentiated pool
places `n_alSynSetOutputLPParam`'s `0.1f` literal at its linked address.

`n_load` is the SDK's n_audio microcode branch (`-DN_MICRO`) and contributes
three exact functions, `0xB4C` executable bytes, four relocations, and the
translation unit's final four-byte alignment word. The internal decoder is
named `_decodeChunk` from the permitted donor source; its three callers retain
that exact relocation identity.

`n_alLPFilter` owns the `0x10`-byte `.rodata` island at ROM `0x85470`.
Its two functions contribute `0x41C` executable bytes and 13 relocations; the
translation unit ends with one separate four-byte alignment word.

`n_drvrNew` uses Rare's per-bus configuration layout (`-DRAREDIFFS`) and
the R4300 multiply scheduler. Its six functions contribute `0xD0C`
executable bytes and 34 relocations, followed by one alignment word. The TU
also owns `0x40` bytes of initialized effect parameters at ROM `0x80D80` and
the `0x20`-byte constant island at ROM `0x85390`; both sections compare
directly against their retail ranges.

`n_synthesizer` uses Rare's per-bus effect layout and the n_audio microcode
ABI (`-DRAREDIFFS -DN_MICRO`). Its nine functions contribute `0xAC8`
executable bytes and 173 relocations, followed by IDO's eight-byte return-stub
alignment at `0x800623CC`. The TU also owns `0x10` bytes of initialized
parameter counters at ROM `0x80D70` and the `0x10`-byte floating-point constant
island at ROM `0x85370`; both sections compare directly against retail. No
post-compile insertion is involved.

`n_env` is the SDK's n_audio microcode path (`-DN_MICRO`). Its five functions
occupy `0xFEC` executable bytes with 59 relocations, followed by one compiler
alignment word in the `0xFF0`-byte text section. The TU also owns the
`0x100`-byte equal-power table at ROM `0x80DC0` and a `0x50`-byte switch table
and constant island at ROM `0x853F0`; the initialized table compares directly
against retail and the linked rodata is covered by the full-ROM proof.

`n_resample` uses the SDK n_audio microcode branch (`-DN_MICRO`); the earlier
tail mismatch came from compiling the non-n_audio command path. Its two
functions occupy the full `0x2A0`-byte text section with eight relocations,
and its `0x10`-byte constant island at ROM `0x85490` compares directly against
retail.

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
for `aisetnextbuf`, `initialize`, `controller`, `vimgr`, `thread`, `siacs`, `vi`, `timerintr`, and `xlitob`;
rodata for `cents2ratio`, `sinf`, and `devmgr`; and BSS slices for `seteventmesg`, `vimgr`,
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
- **rodata order follows text order exactly.** The 35 jump tables still emitted
  in `asm/` are monotonic in both columns, with **zero inversions**. So
  `.rodata` can be carved TU by TU in text order, which is what makes the
  per-TU split tractable. Five more tables now belong to matched `n_csplayer`
  C, and one belongs to matched `n_reverb` C.
- **rodata order follows text order exactly.** The pre-carve inventory covered
  35 functions and forty-four tables. With `devmgr` now source-built, the 43
  jump tables still emitted in `asm/` belong to 34 functions and remain
  monotonic in both columns, with **zero inversions**. So `.rodata` can be
  carved TU by TU in text order.
- **rodata order follows text order exactly.** 35 functions, 35 jump tables,
  monotonic in both columns, **zero inversions**. So `.rodata` can be carved TU
  by TU in text order, which is what makes the per-TU split tractable.
  per-TU split tractable. Thirteen more tables now belong to matched C,
  including five in `n_csplayer`, one in `n_reverb`, and one in `main/font`.

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
