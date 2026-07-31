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
  scanned — a distinction that has already cost one wrong claim. `occ` in
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
| `__osGetCause` | 2 real of 4 | Same argument as `stack_pointer`: the body is `mfc0 v0, Cause` / `jr ra`, and **those two words occur exactly once in the 32MB image** — unlike the rejected `leointerrupt`, whose body alone occurs 39 times, so the padding is doing none of the work. Three reference builds name those bytes, all identically |
| `__osExceptionPreamble` | 2 unmasked of 4 | Shape: `lui k0,0x8005` / `addiu k0,-4032` / `jr k0` jumps to the very next instruction |
| `__osPopThread` | 4 unmasked | Island context — three other matches already establish that this is `exceptasm.s` |

Six rows, four arguments: `osScGetCmdQ` and `osScGetInterruptQ` share one, and
`__osGetCause` reuses `stack_pointer`'s. A pass that adds a fifth argument
should be treated as suspect — exceptions are meant to stay rare enough to
enumerate.

Passing the threshold is not the same as the name being *right about Mickey*.
Tier A establishes that the routine is the same routine; the reference build's
name may still describe that project's use of it. `weather_clip_planes` and
`audspat_jingle_off` are carried because renaming shared code would hide the
sharing, and are flagged where they appear.

**A whole-`.text` match outranks a standalone function match.** When both are
available for one address, the whole-object match wins and the standalone one
is recorded as noise — see the `__osPfsGetInitData` and `__osPiGetAccess`
collisions in `symbol_addrs.us.txt`.

### 1.3 When a public decomp's name is adopted, and how it is disclosed

`docs/CLEANROOM.md` permits public retail-derived decompilations (DKR, JFG, PD,
BK, Conker) as sources. This project reads **all five**, and
`docs/references.md` records each one's repo URL, pinned commit, verified
baserom checksum, build outcome and match yield. The two that supply most of
the tree are:

- **Diddy Kong Racing** — for tier A, its *built objects only*: compiled bytes
  plus symbol and relocation tables. That restriction is what makes tier A
  evidence rather than transcription, and it is a claim about the method, not
  about what a human was allowed to read: both projects' sources are permitted
  reading under `docs/CLEANROOM.md`, and DKR's was grepped for string literals
  during the tier-C work. It produced **no adopted name** — every DKR name in
  the tree came out of a built object.
- **Jet Force Gemini** — two uses, not to be conflated. Its *published source
  text* was read to answer "what did that project call the function that does
  this", and — in `src/main/runlink.c` — for adapted function bodies, disclosed
  at the point of use. Separately, its *built objects* are the largest single
  source of tier-A names in the tree: **84 of the 87 translation units that
  pass adopted are JFG's, carrying 187 of its 190 names**, on the same
  built-objects-only basis as DKR's.
- **Perfect Dark, Banjo-Kazooie and Conker** — built objects only. PD
  contributed three names (the Transfer Pak driver, which no other reference
  build contains). BK and Conker contributed **none**, and that documented zero
  is in `docs/references.md` alongside what they did contribute:
  independent corroboration of **2** and **8** of the adopted translation units,
  and re-confirmation of **73** and **65** subsegments Phase 1 had already
  named. Those are two different measurements and they are not summed — see
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
- No name for a function whose C is parked non-matching — it would put a second
  evidence tier into the symbol file. Such names live in the source file's
  comments only.
- No name inherited from a reference build's *address placeholder*. Importing
  one would assert an address that does not exist in this game. Two such cases,
  different in kind: `func_80070058` (DKR's `math_util.s`) matched at ROM
  `0x2A90C` and is left unnamed with the reason recorded in
  `symbol_addrs.us.txt`; `func_800676F8` (JFG's `diCpu.c`) is what the string
  evidence for Mickey's `0x80045BBC` pointed at, so that address keeps its own
  `func_` name. Beyond those, **37 placeholder addresses fall inside the
  translation units the cross-title pass matched** — mostly inside JFG's large
  `n_audio` and `gsSnd` objects, which that project has matched but not yet
  named — and none was imported. The rule is applied by the generator, not by
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
| `0x16B0000`–`0x18F1FE0` | 2.26 MiB | **overlay modules + their reloc data** | §5 |
| `0x18F1FE0`–`0x2000000` | 7.06 MiB | `0xFF` fill | verified byte by byte |

The resident segment's end is derived, not guessed: the entrypoint zeroes BSS
from VRAM `0x80085A40`, and `0x80085A40 - 0x80000400 + 0x1000 = 0x86640`, so
the ROM only needs to supply bytes up to there.

### 2.1 The build stamp

ROM `0x7AD00` holds three consecutive string pointers — to `"1.1153"`,
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
Conker's — `docs/references.md`); **tier C** rows are string-correspondence
with JFG; everything else is noted inline. Ranges without a named anchor are
omitted rather than guessed at. 171 translation units are matched whole across
the segment, carrying 190 function names.

| ROM | VRAM | Anchor | Tier | What it establishes |
|---|---|---|---|---|
| `0x1000` | `0x80000400` | `entrypoint` | A | The reset vector's target |
| `0x1AE60`–`0x1BE50` | `0x8001A260` | `main/lights2` | A | **Measured file boundary**: JFG's whole 0xFF0 `hasm/lights2.s`, 9 routines — the lighting pipeline, a starfield mover, a CPU line rasteriser, a rain draw. The first anchor anywhere in `0x16140`–`0x1C790` |
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
| `0x3D5F0` | `0x8003C9F0` | `reset_particles` | A | |
| `0x43470` | `0x80042870` | `strcpy`, `memset`, `sprintf`, `_itoa`, `vsprintf` | A + C | The C-library / formatting layer |
| `0x459C0`–`0x467BC` | `0x80044DC0` | `diRcpPrintDL`, `diRcpMoveWd`, `diRcpStrName`, `diRcpOtherMode`, `diRcpGeometryMode` | C | **The display-list disassembler** — a full GBI pretty-printer left in the retail build |
| `0x467BC`–`0x47A60` | `0x80045BBC` | `diCpuReportWatchpoint`, plus the memory/module debug pages and the register-dump crash reporter | C | **The debug monitor**, also left in |
| `0x47A60`–`0x47A70` | `0x80046E60` | `main/get_stack_pointer` | A | Measured file boundary |
| `0x4E378` | `0x8004D778` | `byteswap32` | A | |
| `0x4EA60`–`0x4F4D4` | `0x8004DE60` | `main/gzip_asm` | A | **Measured file boundary**: DKR's whole 0xA74 inflate core, in one piece |
| `0x4FC30`–`0x505E0` | `0x8004F030` | `libultra/exceptasm` | A | **Measured file boundary**, 9 routines including `__osException` and `__osDispatchThread`; §4.2. `0x4FC20` before it is the **rejected** `io/leointerrupt` match, and `0x505E0`–`0x506D0` after it is a separate unknown |
| `0x50820`–`0x50C00` | `0x8004FC20` | `main/refractOutputAssembler` | A | Measured file boundary (JFG) |
| `0x59B90`–`0x59BF0` | `0x80058F90` | `main/osBootRamTest` | A | Measured file boundary (JFG) — the IPL3 6105 RAM test |
| `0x5C310`–`0x5E6B0` | `0x8005B710` | `main/gsSnd` | A | **The sound player**, 0x23A0 in one piece, 22 named functions. Two of those names were predicted at tier C from error strings and fall inside this TU at exactly the predicted addresses |
| `0x5E6B0`–`0x6AF90` | `0x8005DAB0` | libultra's `n_audio` synthesis library | A | 45 consecutive measured file boundaries, 106 names, plus two JFG maths TUs interleaved (`math_atan`, `math_acosf`); a third, `math_arc`, begins at `0x6AF90` immediately after. §4.2 |
| `0x6B3D0`–`0x6F3E0` | `0x8006A7D0` | Transfer Pak, Rumble Pak, Controller Pak filesystem | A | 18 measured file boundaries, 34 names. The Transfer Pak three come from **Perfect Dark**, the only reference build that has them; §4.2 |
| `0x6F420`–`0x76D10` | `0x8006E820` | the libultra corridor | A | §4.1 |
| `0x76D10`–`0x76E60` | — | non-resident text | — | Indexes off `$at`, loads from address 0; relocated before it runs. Still `bin` |
| `0x76E60`–`0x86640` | `0x80076260` | `.data` + `.rodata` | — | §6.3 |

### 3.1 Which modules are resident

The ROM carries `__FILE__` path strings from `assert`-style call sites, and
where those strings are *referenced from* is direct evidence of where a
module's code lives.

| Module | Path string copies | Referenced from | Conclusion |
|---|---|---|---|
| `main` | 6, at `0x80081B0C`–`0x80081B48` | resident: `0x80026FB4`, `0x80027FB8` | Fully resident |
| `track` | 14, at `0x80081540`–`0x80081610` | resident: `0x8000BDB4`, `0x8000E920` | **Partly resident** |
| `front` | 2, at `0x800826C0`, `0x800826D0` | resident: `0x80038E1C` | **Partly resident** |
| `clone` | 2, at ROM `0x188B4D0`, `0x188B4E0` | not referenced from the resident segment at all | **Overlay-only** |

`main` is the permanently resident module; `front` and `track` straddle the
boundary, with resident stubs or shared helpers that carry their own assert
strings; `clone` exists only inside the overlay region. The scheduler's task
taxonomy agrees independently — `SC_TASK_CLONE` is one of its seven task types
(`include/game/sched.h`), so `clone` is a task, i.e. something scheduled rather
than something always present.

### 3.2 What the debug content says about this build

Two substantial debug subsystems survive into the retail ROM: a complete GBI
display-list disassembler (`0x459C0`–`0x467BC`, every `G_*` and `RM_*` name
spelled out) and a debug monitor with memory-region pages, a module list and a
full register-dump crash reporter (`0x467BC`–`0x47A60`). Together that is
roughly 8KB of code plus 4KB of strings, and it is why so much of the resident
segment can be identified from strings alone. It also means the *linker* is
observable from the outside: the crash reporter calls `runlinkGetAddressInfo`
to turn a faulting address into "Module %d at %08x".

---

## 4. libultra

### 4.1 The corridor — ROM `0x6F420`–`0x76D10`

VRAM `0x8006E820`–`0x80076110`, `0x78F0` bytes. **95 named subsegments, every
one of them a measured whole-`.text` file boundary, and 123 named functions**,
all tier A. The yaml carries the boundary argument at both ends and
`symbol_addrs.us.txt` carries the per-function names.

**Where the drift went.** The first sweep, against DKR's built libultra alone,
named 80 subsegments and 107 functions and left `0x1AE0` — 22.2% of the
corridor, in ten runs — unnamed: libultra-shaped code not byte-identical to
DKR's build, with a best-alignment fuzzy scan at 35% tolerance returning zero
candidates. Those runs are not drifted copies of DKR's libultra; they are a
*different build*. Run the finder over Jet Force Gemini's libultra and **eight
of the ten runs fall**, in fifteen whole-`.text` matches.

The remaining unnamed code is `0xB50` — **9.4% of the corridor**, in two
contiguous runs:

| ROM | Size | Note |
|---|---|---|
| `0x70AF0`–`0x70E20` | `0x330` | Between `dpsetstat` and `pfsdeletefile` |
| `0x74090`–`0x748B0` | `0x820` | Between `timerintr` and `vigetcurrcontext` |

Neither matches any object in any of the five reference builds, whole or
per-function — five negatives, two of them from byte-perfect builds.

**Read these two runs rather than mining them further.** "Unnamed code inside
the corridor is libultra-shaped" was a fair assumption at 78% identified
against a single build; at 90.6% against five it is carrying more weight than
it has earned, and the plainest reading of `0xB50` that five libultra builds do
not contain is that some of it **is not libultra**. Two more reference builds
would be a sixth and seventh negative; a disassembly would be an answer.
Disassemble `0x70AF0`–`0x70E20` and `0x74090`–`0x748B0` before running the
finder over anything else.

`__osPiGetAccess` — "the same 17 instructions as DKR's, scheduled differently"
— is named from JFG's whole `io/piacs.c` TU at `0x80071B80`. `libultra/piacs`
was the one corridor subsegment named without a measured boundary; JFG measures
it, which is why this section says 95 of 95.

### 4.2 libultra outside the corridor

**The corridor is where libultra is contiguous, not where libultra exists.** A
whole-image sweep against four more reference builds found **68 translation
units of libultra below the corridor**, in three blocks.

| ROM | VRAM | What | TUs | Names | From |
|---|---|---|---|---|---|
| `0x4FC30`–`0x505E0` | `0x8004F030` | `os/exceptasm.s` — the exception handler and thread dispatcher | 1 | 9 | JFG |
| `0x5E6B0`–`0x6AF90` | `0x8005DAB0` | the `n_audio` synthesis library (45 TUs) with two JFG maths TUs interleaved | 47 | 106 | JFG |
| `0x6B3D0`–`0x6F3E0` | `0x8006A7D0` | Transfer Pak, Rumble Pak, Controller Pak filesystem | 18 | 34 | JFG + PD |

Plus the two scheduler accessors at `0x30F10`/`0x30F18` (`osScGetCmdQ`,
`osScGetInterruptQ`, `sc/sched.c`), which remain the only libultra named below
the corridor that is *not* part of a measured TU.

**The exception island is stock libultra.** The whole `0x9B0` `.text` of JFG's
built `os/exceptasm.s.o` matches ROM `0x4FC30` in one piece, 76 of 620 words
masked, ROM-wide unique — including `__osException`, `__osEnqueueThread`,
`__osDispatchThread` and `__osEnqueueAndYield`, which match nothing in DKR's
build at all. The TU ends at `0x505E0`, not `0x506D0`; the `0xF0` bytes between
are a separate, still-unidentified run, split off in the yaml. The code stays
`asm` — it is hand-written assembly, and identifying it did not require
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

**Reversed** — the reason held against the evidence then available and does not
hold against the evidence now:

| ROM | Was | Now |
|---|---|---|
| `0x61990` | `alCSPGetState`, rejected: a one-line accessor whose ROM-wide uniqueness is an accident of padding | `n_alCSPGetState`, adopted — the address falls inside a whole-`.text` match (`0x20`, 0 masked, `romocc=1`), so what places it is the translation unit, not the accessor's own bytes. The `n_` prefix is also new information: this is the n_audio variant |
| `0x620E0` | `alCSeqGetTicks`, same objection | `n_alCSeqGetTicks`, adopted — inside `n_cseq.c`'s whole `0x9D0` TU |

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

---

## 5. The overlay system

### 5.1 What runs it

The resident segment carries a complete Rare/DKR-lineage runtime linker at ROM
`0x323E0`–`0x33FA0`, plus its trampoline at `0x33FA0`. Six of its functions are
decompiled and byte-matched; four more are named from Mickey's call graph. The
mechanism, entirely from Mickey's own disassembly:

1. A call to a function that lives in a not-yet-loaded overlay is assembled as
   a `jal` to **`TrapDanglingJump`** (`0x800333A0`).
2. `TrapDanglingJump` saves every argument register — `a0`–`a3`, `f12`–`f15`,
   `v0`, `v1` — and computes `ra - 8`, the address of the `jal` that reached it.
3. It searches **`mainRelocTable`** (8-byte entries) for the entry whose call
   site is that address, which yields an index into **`overlayRomTable`**.
4. That 4-byte entry splits into a 12-bit overlay number and a 20-bit offset
   (`RomTableEntry`).
5. **`runlinkDownloadCode`** (`0x80031C78`) loads the overlay and relocates it,
   calling `ProcessRelocationEntry` per record and finishing with
   `osInvalICache` — genuine self-modifying code.
6. The trampoline recomputes `overlayTable[n].vramBase + offset`, restores the
   arguments and `jr`s to the real function. The caller never knows.

`runlinkGetAddressInfo` (`0x800331E4`) is the inverse, and is what the debug
monitor uses to print "Module %d at %08x". Its fourth parameter is an optional
symbol-name out-pointer filled by **`GetSymbolName`** (`0x800317E0`) — which in
this retail build is four instructions that spill their argument to the stack,
never read it back, and return the constant string `"unknown"`. The ROM-side
symbol table the mechanism is built around is simply absent from the shipped
image.

### 5.2 The tables

Named from stride and use in Mickey's disassembly (`symbol_addrs.us.txt`), all
six in BSS:

| Symbol | VRAM | Element | Stride |
|---|---|---|---|
| `overlayTable` | `0x800D2D90` | `OverlayHeader` | `0x20` |
| `mainRelocTable` | `0x800D2D94` | `RelocTableEntry` | `0x8` |
| `overlayRomTable` | `0x800D2D98` | `RomTableEntry` | `0x4` |
| `overlayCount` | `0x800D2D9C` | count | — |
| `mainRelocTableCount` | `0x800D2DA0` | count | — |
| `linkSlotTable` | `0x800D2E48` | `LinkSlot` | `0x2` |

`overlayCount` bounds **both** the overlay table and the link-slot table, so
there is exactly one link slot per overlay — which is the best available
evidence for what `LinkSlot`'s two fields mean, and still not enough to promote
them out of inference.

`RelocTableEntry` (`include/game/runlink.h`) is **not JFG's layout**: Mickey
puts the ROM-table index first and packs the call site as a 24-bit offset from
`0x80000450` in the high bits of the second word. Derived from Mickey's ROM;
only the type's name is borrowed.

### 5.3 Where the tables are in ROM — not found

Open, but narrowed: §5.2 gives the exact byte format to search for. Two
searches were run over the whole 32MB image.

- **`OverlayHeader[]`** — three or more consecutive 0x20-byte records with
  `vramBase == 0`, `romAddress` inside the overlay region and plausible
  section sizes. **Zero candidates.**
- **`RelocTableEntry[]`** — runs of 8-byte records whose decoded call site
  `(word[1] >> 8) + 0x80000450` lands inside the resident segment. This
  produces plenty of long runs, and they are all false positives. The
  discriminator that kills them is cheap and worth reusing: *a real entry's
  call site must contain an actual `j` or `jal` instruction.* Re-running with
  that test gives **not one run of even 6 consecutive entries anywhere in the
  ROM.**

The best-looking candidate, ROM `0x18AAD00`, is recorded as ruled out rather
than as a lead: its first 40 entries are sorted and 4-byte aligned, which is
exactly what a lookup table looks like, but over its full 2205-entry extent it
has 142 inversions, its decoded "call sites" are only 4.4% `j`/`jal` (98 of 2205), and the bytes
immediately after it are a function prologue. It is overlay code being read as
a table.

**Conclusion: the reloc tables are not stored uncompressed anywhere in the
image.** Either they are compressed like the assets, or they are constructed at
load time. That is the starting point for Phase 4, which owns splitting
`0x16B0000`–`0x18F1FE0` and can read the tables out of a decompressed module
image instead.

> **One unproven assumption carries that conclusion.** The `j`/`jal`
> discriminator assumes a call site into a not-yet-loaded overlay holds a real
> jump instruction *in the ROM image as shipped*. That is what §5.1 says the
> mechanism requires — `TrapDanglingJump` only ever runs because a `jal`
> reached it — but it is an inference from how the linker works, not an
> observation of the resident bytes at a known call site. **If unloaded call
> sites are stored as something else and rewritten into `jal`s at load time,
> the discriminator is wrong, every run it rejected comes back into play, and
> the negative result evaporates.**
>
> To settle it: find one call site whose target is known to be an overlay
> function and confirm the ROM bytes there are a `jal` to `TrapDanglingJump`
> (`0x800333A0`). One address decides it in both directions.

---

## 6. Compiler flags

### 6.1 What is measured

| Scope | Flags | How established |
|---|---|---|
| Project default | `-O2 -mips1 -32` | splat/IDO preset; not measured |
| `src/main/` (game code) | `-O2 -mips2 -32` | **Measured.** `ResolveRelocAddress` at `-mips1` emits five load-delay `nop`s the ROM does not have |
| `src/libultra/string.c` | `-O2 -mips2 -32` | **Measured.** Branch-likely instructions |
| 10 libultra io/os TUs | `-O1 -mips2 -32` | **Measured**, one variant at a time. At `-O2` IDO folds away a stack frame the ROM has. Locals need `register` or `-O1` spills them |
| `src/libultra/epiread.c`, `epiwrite.c` | `-O2 -g3 -mips2 -32` | `-g3 -mips2` **measured**; `-O2` **not discriminated by these bytes** — taken from JFG's Makefile. See below |

The `-mips2` finding for `src/main/` is scoped to that directory on purpose. It
is believed to hold for all game code and has been measured on one TU; widen it
when the next module is measured, not before.

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

`-O3` could not be tested: this IDO recomp build dies in `uld` on any `-O3`
invocation. That is an environment failure, not a fact about the code.

Three consequences:

- **`-O2` is not established.** `-O1 -g3 -mips2` and `-O2 -g3 -mips2` are
  byte-identical on both files, so these bytes do not discriminate the
  optimisation level at all. `-O2` is in the Makefile because JFG's published
  Makefile builds its libultra `io/` TUs that way — borrowed, not measured. The
  *structural* claim is what the measurement supports: `-g3` stops IDO hoisting
  the third argument's spill into the first `jal`'s delay slot and reverses the
  epilogue's `lw ra` / `lw v0` order, and the ROM agrees with `-g3` on both.
- **The ten `-O1 -mips2` TUs above should be re-examined.** They were measured
  before `-g3` was known to be in play. They match byte-for-byte, so nothing is
  wrong; but "the flags that reproduce these bytes" and "the flags this file
  was built with" are different claims, and only the first is established — as
  the `-O1`/`-O2` tie above demonstrates for the new group.
- **`-g3` came from a reference build's configuration, not from deduction.**
  Reading a permitted decompilation's build configuration is the same
  permission as reading its source (§1.3); what makes `-g3 -mips2` a fact about
  Mickey is that it was then measured against Mickey's bytes. `-O2` did not
  survive that test.

A practical side effect: `-g3` makes IDO write a `.u` sidecar into the working
directory for every such compile. `.gitignore` covers it.

`-Wab,-r4300_mul` remains open. Nothing matched so far multiplies. DKR passes it.

### 6.2 The open question: odd floating-point registers

**Mickey's floating-point game code was not built by the IDO 5.3 this project
vendors.** The ROM uses odd single-precision FP registers; that compiler emits
zero of them at any of nine ISA/optimisation combinations tested. ROM-wide there
are 1727 odd FP register operands across 9 static-segment asm files, and GNU as
warns once per occurrence.

Three explanations remain open and are **not** mutually exclusive: a different
IDO release, hand-written assembly, or a non-IDO compiler. The per-file
distribution is what makes this genuinely undecided — it runs from 61.3% of FP
operands in `asm/59DB0.s` down to 2.7% in `asm/16140.s`, which looks more like
mixed origins than one allocator applied uniformly. The parked matrix TU sits
at 40.1%, so hand-written assembly is live for those two functions
specifically, in which case no compiler settles them.

It is not a blocker for the naming work: nothing named so far is float-heavy,
and the tier-A matches that *are* in float-adjacent code (`mtxf_*` in
`math_util.s`) are against DKR's **hand-written assembly** objects, which
sidesteps the question rather than answering it.

Relevant to any future resolution: `mtxf_transform_point`, `mtxf_mul`,
`mtxf_to_mtx`, `vec3s_reflect` and `mtxf_from_inverse_transform` at ROM
`0x2A250` are byte-identical to DKR's `.s` sources. If Mickey's own
`main/matrix` TU at `0x2B650` turns out to be hand-written too, that is the
same story one file over.

### 6.3 `.rodata` is still unsplit

Every byte from ROM `0x76E60` to `0x86640` is one `data` subsegment, so no C TU
can own rodata yet. This gates the object system, whose functions all carry
jump tables. Two things a split needs are measured:

- **rodata order follows text order exactly.** 35 functions, 44 jump tables,
  monotonic in both columns, **zero inversions**. So `.rodata` can be carved TU
  by TU in text order.
- **Lower bound on the boundary: `0x80080D24` (ROM `0x81924`)** — a float
  constant (`7f7fffff`, i.e. `FLT_MAX`) loaded from ROM `0x50B0`, two words
  below the first jump table `jtbl_80080D2C` (ROM `0x8192C`). splat's own
  heuristic guessed `0x8192C`; this tightens it by those two words.

The split itself has not been done: it touches the yaml, the linker script and
every future TU at once.

---

## 7. What this map does not cover

- **The overlay region `0x16B0000`–`0x18F1FE0`** is one `bin`. Module
  boundaries, per-module reloc tables and the three TOC tables are all
  unlocated (§5.3). Phase 4.
- **The asset region `0x87000`–`0x16B0000`** is one `bin`. Entropy suggests at
  least two classes — a near-random band to ~`0xAC0000` and a more structured
  one beyond — but nothing is decoded. `gzip_inflate_*` at `0x4EA60` is the
  decompressor that reads it, which is the obvious way in.
- **`0x86640`–`0x87000`** looks like a table of ROM offsets and is unidentified.
- **The model/sprite code**, on the same evidence as the build stamp (§2.1).
  `"CREATE LOD MODEL :: null model pointer!"` (`0x80081904`) and `"Cam do 2D
  sprite called with NULL pointer!"` (`0x800819F1`) are in the resident
  segment's rodata, but **no resident instruction builds either address** — the
  nearest references anywhere in `asm/` are `D_80081898` below and
  `D_80081A1C` above, and the two strings sit in the gap between them. So the
  asserts that print them are in an overlay, and the resident segment holds
  only their text. In particular `0x1FC9C` is *not* a model/sprite anchor:
  `func_8001F09C` is a float routine that steps a value at `+0x50` toward a
  bound at `+0x54` and clamps it, with no connection to either string.
- **9.4% of the libultra corridor** (§4.1) — `0x70AF0`–`0x70E20` and
  `0x74090`–`0x748B0`, `0xB50` between them, matching nothing in any of the
  five reference builds. **It may not be libultra at all**: the label is
  inherited from a map made when the corridor was 78% identified against one
  build, and five builds' worth of silence is now the more informative fact.
  Disassemble it rather than mine it further — §4.1.
- **What is still `asm` that need not be.** 173 subsegments are named; 171 of
  them have a measured whole-`.text` boundary (the exceptions are `main/matrix`
  and `main/runlink`, which are decompiled rather than matched-as-a-unit).
  Only **16** are `c`, and only 14 of those are matched C. The remaining 157
  are `asm` because nobody has tried, not because anything is in the way — and
  the `-g3` finding in §6.1 widens the flag space worth trying.
  `libultra/pigetcmdq`, `libultra/epilinkhandle` and
  `libultra/setglobalintmask` are the obvious next three: three to six
  instructions each, blocked only on naming one data symbol apiece.
- **The `0xF0` bytes at `0x505E0`–`0x506D0`**, between the end of
  `os/exceptasm.s` and the next subsegment. Unidentified.
- **`0x6B860`–`0x6BDF0`** (`0x590`), between Perfect Dark's three Transfer Pak
  routines. Almost certainly more of the same driver; PD's build does not
  contain whatever it is.
- **The object system.** The `"setting up"` / `"freeing"` / `"processing"` /
  `"exploding"` phase names sit in a 5-entry pointer table at `0x8007A220`, whose first
  entry is `"null"`, immediately followed by four function pointers —
  `0x8002B280`, `0x8002B314`, `0x8002B768`, `0x8002B524` — which are exactly
  the memory routines the linker calls. Something at `0x8007A214` is a
  descriptor combining phase names with allocator entry points. The only
  resident reader of the phase table is the crash reporter at `0x80046548`, so
  the phases are what a fault is reported *during*. That is as far as the
  evidence goes; no struct is asserted for it.
