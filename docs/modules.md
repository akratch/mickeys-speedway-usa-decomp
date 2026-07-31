# The module map

What lives where in Mickey's Speedway USA (USA), and — for every claim — how it
was established. This file is the project's ontology: the naming convention
later phases follow, plus the address map those names hang on.

Two rules govern everything below.

1. **Every claim carries its evidence.** A range with no stated method is a bug
   in this document, not a fact about the ROM.
2. **Evidence tiers are never mixed.** A byte-identical match and a plausible
   reading of a disassembly are both useful and are not the same thing. They
   are labelled separately here and in `symbol_addrs.us.txt`, and the labels
   are load-bearing.

All ROM offsets are byte offsets into `baseroms/mickey.us.z64`
(SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548`). For the resident segment,
`VRAM = ROM + 0x7FFFF400`.

---

## 1. Naming convention

### 1.1 The four tiers

| Tier | What it is | What may be adopted | Where it is used |
|---|---|---|---|
| **A — byte-identity** | The ROM's bytes are identical, under relocation masking, to a named symbol in a permitted decomp's *built* objects | The reference build's name, verbatim | libultra corridor (Task B), DKR game code (Task D) |
| **B — call graph** | The function's role is pinned by who calls it, with what, and what the caller does with the result | An invented descriptive name, or a public decomp's name where the role matches exactly | `piacs`/`siacs` (Task B), the linker cluster (Tasks C, D) |
| **C — string correspondence** | The function builds the address of a distinctive string literal, and a public decomp of the same engine uses that literal in a function whose job matches | That project's name | `diRcp*`, `diCpu*`, `gsSndp*`, `osScGetTaskType` (Task D) |
| **D — structural inference** | Read off the disassembly and nothing else | An invented descriptive name | `SetLinkSlot`, `ReleaseUnusedLinkSlots` (Task C) |

Tier A is the only tier that is a *measurement*. B, C and D are arguments, and
an argument can be wrong. That asymmetry is why the tiers are kept apart: the
value of `symbol_addrs.us.txt` comes from a reader being able to tell, at the
point of use, how much a given name is worth.

### 1.2 Tier A: the adoption threshold

A byte-identical match is adopted as a name only when **all three** hold:

- **at least 6 unmasked instruction words** compared. Masked words are the ones
  the reference object's own relocation records say the linker patches; they
  prove nothing.
- **unique across the whole 32MB image.** Not unique within the window that was
  scanned — that distinction cost Task B a wrong claim, and `occ` in
  `tools/find_known_objects.py` is still window-scoped.
- **exactly one candidate name** for those bytes. Several different functions
  in one reference build routinely compile to identical instructions; when they
  do, byte comparison cannot say which one this is, and the row is noise.

**Five adopted symbols fall below the 6-word bar, resting on four distinct
arguments.** Every one is disclosed at its point of use in
`symbol_addrs.us.txt`, and each rests on something a word count does not
measure. They are enumerated here so the exception list is a list rather than a
gesture — an earlier version of this section claimed "two", which was a
miscount, and the whole value of a threshold is that its exceptions can be
named:

| Symbol | Words | What carries it instead |
|---|---|---|
| `osScGetCmdQ` | 2 | The **pair**: two *different* immediates (`addiu v0,a0,0x78`, `addiu v0,a0,0x40`), 8 bytes apart, in the reference's order, each ROM-wide unique |
| `osScGetInterruptQ` | 2 | as above |
| `stack_pointer` | 2 real | Semantically unambiguous on its own: the whole body is `jr ra` / `move v0,sp` |
| `__osExceptionPreamble` | 2 unmasked of 4 | Shape: `lui k0,0x8005` / `addiu k0,-4032` / `jr k0` jumps to the very next instruction |
| `__osPopThread` | 4 unmasked | Island context — three other matches already establish that this is `exceptasm.s` |

Five rows, four arguments: `osScGetCmdQ` and `osScGetInterruptQ` share one.
**A future task that adds a fifth argument should be suspicious of itself** —
exceptions are supposed to be rare enough to enumerate, and the moment they
stop being, the threshold has stopped doing its job.

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
BK, Conker) as sources. This project reads two:

- **Diddy Kong Racing** — for tier A, its *built objects only*: compiled bytes
  plus symbol and relocation tables, never its source. That restriction is what
  makes tier A evidence rather than transcription, and it is a claim about the
  method, not about what a human was allowed to look at. Both projects' sources
  are permitted reading under `docs/CLEANROOM.md`; DKR's was in fact grepped for
  string literals during Task D's investigation, and produced **no adopted
  name** — every DKR name in the tree came out of a built object. Stated
  explicitly because the earlier wording ("never its source", unqualified) said
  more than was true of the work.
- **Jet Force Gemini** — its *published source text*, and only to answer "what
  did that project call the function that does this". Names, not code.

Every JFG-derived artifact carries an explicit `PROVENANCE` disclosure at the
point of use. `src/main/runlink.c` opens with one, because the bodies in it are
adapted from JFG's `runLink.c`; the tier-C block in `symbol_addrs.us.txt`
carries another. The rule Task C's review established stands: **the provenance
line goes in before the body, and byte-identity against Mickey's ROM is what
makes the borrowing sound — not a reason to leave it unmentioned.**

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
- No name for a function whose C is parked non-matching (Task C's rule: it
  would put a second evidence tier into the symbol file). Such names live in
  the source file's comments only.
- No name inherited from a reference build's *address placeholder*. Importing
  one would assert an address that does not exist in this game. **Two** such
  cases arose, and they are different in kind:
  `func_80070058` (DKR's `math_util.s`) matched at ROM `0x2A90C` and is left
  unnamed with the reason recorded in `symbol_addrs.us.txt`;
  `func_800676F8` (JFG's `diCpu.c`) is what the string evidence for Mickey's
  `0x80045BBC` pointed at, so that address simply keeps its own `func_` name.
  An earlier version of this line said "three", which was a miscount.

---

## 2. Top-level ROM map

| Range | Size | What | Evidence |
|---|---|---|---|
| `0x000000`–`0x000040` | 0x40 | header | splat `header` segment |
| `0x000040`–`0x001000` | 0xFC0 | IPL3 boot | standard N64 layout |
| `0x001000`–`0x086640` | 0x85640 | **resident segment** (code + data + rodata) | §3 |
| `0x086640`–`0x087000` | 0x9C0 | table of ROM offsets (unidentified) | entropy transition; still `bin` |
| `0x087000`–`0x16B0000` | ~22.8MB | compressed assets | entropy 7.1–8.0 across the band |
| `0x16B0000`–`0x18F1FE0` | ~2.4MB | **overlay modules + their reloc data** | §5 |
| `0x18F1FE0`–`0x2000000` | ~7.1MB | `0xFF` fill | verified byte by byte |

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

Named anchors, in address order. **Tier A** rows are byte-identical to DKR's
built objects; **tier C** rows are string-correspondence with JFG; everything
else is noted inline. Ranges without a named anchor are omitted rather than
guessed at.

| ROM | VRAM | Anchor | Tier | What it establishes |
|---|---|---|---|---|
| `0x1000` | `0x80000400` | `entrypoint` | A | The reset vector's target |
| `0x31C4` | `0x800025C4` | `audspat_jingle_off` | A | Spatial audio, and the thinnest row adopted |
| `0xC9B4`, `0xF520` | — | `"track/track.c"` asserts | — | **`track` code is partly resident** |
| `0x1FC9C` | `0x8001F09C` | `"CREATE LOD MODEL"`, `"Cam do 2D sprite"` | — | Model / sprite setup |
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
| `0x33FA0` | `0x800333A0` | `TrapDanglingJump` | B | The overlay call trampoline |
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
| `0x4FC30`–`0x506D0` | `0x8004F030` | the exception island | A | §4.2. The slot at `0x4FC20` immediately before it is the **rejected** `io/leointerrupt` match, not part of the island |
| `0x5C640`, `0x5DFFC`, `0x5E2E4`, `0x5E498` | `0x8005BA40` … | sound-system warnings, `gsSndpStop`, `gsSndpSetParam` | C | The sound player |
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

This confirms and sharpens recon's hypothesis. `main` is the permanently
resident module; `front` and `track` straddle the boundary, with resident stubs
or shared helpers that carry their own assert strings; `clone` exists only
inside the overlay region. The scheduler's task taxonomy agrees independently —
`SC_TASK_CLONE` is one of its seven task types (`include/game/sched.h`), so
`clone` is a *task*, which is a thing that gets scheduled rather than a thing
that is always there.

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

VRAM `0x8006E820`–`0x80076100`, `0x78F0` bytes. **80 named subsegments — 79 of
them measured file boundaries — and 107 named functions**, all tier A against
DKR's built libultra. Established in Task B; the yaml carries the boundary
argument at both ends and `symbol_addrs.us.txt` carries the per-function
names.

`0x1AE0` of it — 22.2%, in 10 runs — is libultra-shaped but *not*
byte-identical to DKR's build, and is deliberately unnamed. A best-alignment
fuzzy scan accepting up to 35% differing words returned zero candidates for
those runs, so they are not lightly-drifted copies: Mickey's libultra is a
different point release or was built with different flags. `__osPiGetAccess` is
the sharp example — the same 17 instructions as DKR's, scheduled differently.

### 4.2 libultra outside the corridor

**The corridor is where libultra is contiguous, not where libultra exists.**
Scanning the whole 32MB image against the same 169 reference objects returns
matches well below it. Named in Task D:

| ROM | VRAM | Symbol | Note |
|---|---|---|---|
| `0x30F10` | `0x80030310` | `osScGetCmdQ` | `sc/sched.c`, inside the scheduler |
| `0x30F18` | `0x80030318` | `osScGetInterruptQ` | adjacent, same order as the reference |
| `0x4FC30` | `0x8004F030` | `__osExceptionPreamble` | adopted on shape, not bytes alone |
| `0x501C0` | `0x8004F5C0` | `send_mesg` | 39 unmasked words |
| `0x50274` | `0x8004F674` | `handle_CpU` | 13 words, 0 masked |
| `0x50408` | `0x8004F808` | `__osPopThread` | 4 words, named on island context |

**The exception island at ROM `0x4FC30`–`0x506D0` is still unexplored, and this
task did not change that.** What is known: it is `os/exceptasm.s`, it is
`0xAA0` bytes against DKR's `0x900` — i.e. **Mickey's is larger** — and
`__osException`, `__osEnqueueThread`, `__osDispatchThread` and
`__osEnqueueAndYield` match *nowhere*, even though `__osEnqueueThread` is 0x48
bytes of relocation-free assembly that would match trivially if it were DKR's.
Something in this island is not stock. It is where thread dispatch lives, and
it deserves a task of its own rather than a corner of a sweep. (The 0x10 bytes
at `0x4FC20` immediately before it are the *rejected* `io/leointerrupt` match
below — an unidentified `return 0`, not an established part of the island.)

Four candidate matches below the corridor were **rejected**, and the reasons
generalise: `io/leointerrupt` (`0x4FC20`), `alCSPGetState` (`0x61990`) and
`alCSeqGetTicks` (`0x620E0`) are one-line functions — `return 0`, `return
x->field` — whose ROM-wide uniqueness is an accident of surrounding padding
rather than evidence about identity. `__osDisableInt`/`__osRestoreInt` at
`0x2A25C`/`0x2A288` are subsumed: those bytes are the *tail* of DKR's larger
`interrupts_disable`/`interrupts_enable` in `math_util.s`, which is why they
matched at an offset of `0xC`. Task B recorded them as "a second copy of
os/interrupt.s"; that reading is superseded.

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

### 5.3 Where the tables are in ROM — still not found, and now with a filter

Recon left this open and it stays open, but the search is now much narrower,
because §5.2 gives the exact byte format rather than a guess.

Two searches were run over the whole 32MB image:

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
has 142 inversions, its decoded "call sites" are only 6% `jal`, and the bytes
immediately after it are a function prologue. It is overlay code being read as
a table.

**Conclusion: the reloc tables are not stored uncompressed anywhere in the
image.** Either they are compressed like the assets, or they are constructed at
load time. That is a negative result with a method behind it, and it is the
right starting point for Phase 4 — which owns splitting `0x16B0000`–`0x18F1FE0`
and can read the tables out of a decompressed module image instead.

> **The conclusion rests on one unproven assumption, and it is load-bearing.**
> The `j`/`jal` discriminator assumes that a call site into a not-yet-loaded
> overlay holds a real jump instruction *in the ROM image as shipped*. That is
> what §5.1 says the mechanism requires — `TrapDanglingJump` only ever runs
> because a `jal` reached it, so the site must be a call before anything is
> patched — but it is an inference from how the linker works, not something
> this task observed in the resident bytes at a known call site. **If unloaded
> call sites are stored as something else and rewritten into `jal`s at load
> time, the discriminator is wrong, every run it rejected comes back into play,
> and the negative result evaporates.**
>
> Cheapest way to settle it, and worth doing before trusting this section:
> find one call site whose target is known to be an overlay function, look at
> the ROM bytes there, and confirm they are a `jal` to `TrapDanglingJump`
> (`0x800333A0`). One address decides it in both directions.

---

## 6. Compiler flags

### 6.1 What is measured

| Scope | Flags | How established |
|---|---|---|
| Project default | `-O2 -mips1 -32` | splat/IDO preset; not measured |
| `src/main/` (game code) | `-O2 -mips2 -32` | **Measured.** `ResolveRelocAddress` at `-mips1` emits five load-delay `nop`s the ROM does not have. Task C |
| `src/libultra/string.c` | `-O2 -mips2 -32` | **Measured.** Branch-likely instructions. Task A |
| 10 libultra io/os TUs | `-O1 -mips2 -32` | **Measured**, one variant at a time. At `-O2` IDO folds away a stack frame the ROM has. Locals need `register` or `-O1` spills them. Task B |

The `-mips2` finding for `src/main/` is scoped to that directory on purpose. It
is believed to hold for all game code and has been measured on one TU; widen it
when the next module is measured, not before.

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

**Task D did not touch this**, per its brief, and it is not a blocker for the
work above: nothing named in this task is float-heavy, and the tier-A matches
that *are* in float-adjacent code (`mtxf_*` in `math_util.s`) are matches
against DKR's **hand-written assembly** objects, which sidesteps the question
entirely rather than answering it.

Relevant to any future resolution: `mtxf_transform_point`, `mtxf_mul`,
`mtxf_to_mtx`, `vec3s_reflect` and `mtxf_from_inverse_transform` at ROM
`0x2A250` are byte-identical to DKR's `.s` sources. If Mickey's own
`main/matrix` TU at `0x2B650` turns out to be hand-written too, that is the
same story one file over.

### 6.3 `.rodata` is still unsplit

Every byte from ROM `0x76E60` to `0x86640` is one `data` subsegment, so no C TU
can own rodata yet. This gates the object system, whose functions all carry
jump tables. Two things a split needs are measured:

- **rodata order follows text order exactly.** 35 functions, 41 jump tables,
  monotonic in both columns, **zero inversions**. So `.rodata` can be carved TU
  by TU in text order.
- **Lower bound on the boundary: `0x80080D24` (ROM `0x8191C`)** — a float
  constant loaded from ROM `0x50B0`, one word below the first jump table.
  splat's own heuristic guessed `0x8192C`; these agree.

Doing the split is still not done. It touches the yaml, the linker script and
every future TU at once.

---

## 7. What this map does not cover

Stated so nobody reads silence as absence of anything to find.

- **The overlay region `0x16B0000`–`0x18F1FE0`** is one `bin`. Module
  boundaries, per-module reloc tables and the three TOC tables are all
  unlocated (§5.3). Phase 4.
- **The asset region `0x87000`–`0x16B0000`** is one `bin`. Entropy suggests at
  least two classes — a near-random band to ~`0xAC0000` and a more structured
  one beyond — but nothing is decoded. `gzip_inflate_*` at `0x4EA60` is the
  decompressor that reads it, which is the obvious way in.
- **`0x86640`–`0x87000`** looks like a table of ROM offsets and is unidentified.
- **The exception island** (§4.2).
- **22% of the libultra corridor** (§4.1).
- **The object system.** The `"setting up"` / `"freeing"` / `"processing"` /
  `"exploding"` phase names sit in a 5-entry pointer table at `0x8007A220`, whose first
  entry is `"null"`, immediately followed by four function pointers —
  `0x8002B280`, `0x8002B314`, `0x8002B768`, `0x8002B524` — which are exactly
  the memory routines the linker calls. Something at `0x8007A214` is a
  descriptor combining phase names with allocator entry points. The only
  resident reader of the phase table is the crash reporter at `0x80046548`, so
  the phases are what a fault is reported *during*. That is as far as the
  evidence goes; no struct is asserted for it.
