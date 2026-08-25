# The resident segment (docs/modules.md section 3)

Split out of `docs/modules.md` on 2026-08-25; evidence tiers and naming rules are defined in that file, section 1. Section numbers below keep their original 3.x identity so existing references resolve.

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
recover the retail spill layout. A focused retry after the independently
observed IDO source-line scheduling effect in `runlinkFlushModules` explored
formatting-only permutations for three minutes without improving the canonical
475 permuter score; the frame and spill layout are insensitive to that lever.
A fresh full 119-configuration flag sweep again left stock `-O2 -mips2 -32`
best at 46 instructions, 28 positional differences, and first mismatch
`+0x18`. Explicit padded spill-carrier variants retained the 40-byte frame
(29 differences), while making the carrier volatile regressed to 31
differences without producing the target stack layout.
An additional retry checked the lattice's only plausible missing flag family,
optimized code with bare debug metadata, but IDO's driver rejects `-O2` and
`-g` together in either order. Marking the spacing and font-data pointers
`register`, singly and together, produced objects byte-identical to the
46-word baseline. The frame/spill plateau is therefore unchanged.
Changing the input pointer from `char *` to `u8 *`, as suggested by the
target's byte loads, also produced an object byte-identical to that baseline.
A fresh 119-configuration sweep still left stock flags at 46 words, 28
positional differences, and first mismatch `+0x18`; signedness is not the
lever behind the frame and spill layout.

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
| `0x8001879C` | `0x130` | `setupLights` | tier-B comparison; `NON_MATCHING` plateau at 75/76 linked words, first mismatch +`0x98` from commutative `addu` operand order |
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
| `0x80019934` | `0xF0` | `lightDistanceCalc` | tier-B comparison; `NON_MATCHING` plateau: canonical flag sweep gives 60/60 text words and exact call relocations, but the shared resident rodata segment retains `jtbl_800817B4`, so promotion duplicates the compiler's anonymous table and leaves the extracted table's five local labels unresolved |
| `0x80019A24` | `0x94` | `lightDirectionCalc` | unique nearest skeleton (0.432) and exact JFG size; comparison only |
| `0x80019AB8` | `0x2E0` | `lightObject` | tier-B comparison: calls all three `lights2` pipelines |
| `0x80019D98` | `0x50` | `lightDefaultObjectLight` | tier-B comparison: delegates to the following setter |
| `0x80019DE8` | `0xFC` | `lightSetObjectLight` | tier-D boundary; `NON_MATCHING` plateau after the flag lattice, 10 source/type hypotheses, and a 10-minute permuter batch: exact `0x38` frame, 64 instructions versus 63, 46 positional words differ, first `+0x48` from byte-store/delta scheduling; JFG body is also assembly-only, so retain `func_` |
| `0x80019EE4` | `0x98` | `lightSetupLightSources` | tier-B comparison: loop calls the adopted `addObjectLight` comparison |
| `0x80019F7C` | `0x8C` | `lightSetupFlareSources` | tier-B comparison: adjacent setup loop and flare helper |
| `0x8001A008` | `0x14C` | `lightInitObjectLighting` | tier-B comparison; `NON_MATCHING` plateau after the flag lattice and nine source/declaration forms: exact 83-word frame/opcode/register/FP/relocation shape, but 4 positional words differ, first `+0x70`, because the call-live result spills at `0x28(sp)` instead of `0x2C(sp)`; the permuter importer scores the isolated function zero, but the required full-TU build retains this mismatch |
| `0x8001A154` | `0xE8` | `lightAdjustGlowingLight` | tier-B comparison; `NON_MATCHING` plateau after flag sweep, 10 source/lifetime hypotheses, and a 10-minute permuter batch: exact 58-opcode/frame shape and call relocation, 45/58 words exact, first `+0x1C` from integer temp-FIFO phase |
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
| `0x2BCD0` | `mmInit` | `mmInit` | A: unique 30-word skeleton with 14 relocated words; linked C exact |
| `0x2BD48` | `mmExtended` | `mmExtended` | B: returns the expansion-memory flag consumed by `mmInit`; matched C exact |
| `0x2BD54` | `func_8002B154` | `mmAllocRegion` | B: allocates slot storage, then calls the pool initializer with it; linked C exact |
| `0x2BDA0` | `func_8002B1A0` | `mempool_init` | B: shared callee of `mmInit` and the region allocator; initializes the 0x10-byte pool and 0x14-byte slot records; linked C exact |
| `0x2BE80` | `func_8002B280` | `mmAlloc` | B: main-pool wrapper that derives a caller colour tag and calls the slot finder; linked C exact |
| `0x2BF14` | `func_8002B314` | `mmAlloc2` | B: second wrapper with the same calls and result role; linked C exact |
| `0x2BFA8` | `func_8002B3A8` | `mempool_slot_find` | B: common worker used by all three allocation wrappers and the fixed-address allocator; linked C exact |
| `0x2C0C0` | `func_8002B4C0` | `mmAllocR` | B: selects a pool by its slot-array pointer, then calls the common worker; linked C exact |
| `0x2C124` | `func_8002B524` | `mmAllocAtAddr` | B: fixed-address allocation through up to three slot assignments; plateau, exact size, 14/116 words differ, first `+0xE0` |
| `0x2C2F4` | `mmSetDelay` | `mmSetDelay` | B: writes the deferred-free delay used by `mmFree`; matched C exact |
| `0x2C300` | `func_8002B700` | `mmFlushFreeStack` | B: drains queued addresses through the address-free worker; linked C exact |
| `0x2C368` | `mmFree` | `mmFree` | A: unique 17-word skeleton with four relocated words masked; linked C exact |
| `0x2C3AC` | `func_8002B7AC` | `mmFreeTick` | B: services the delayed-free queue; Mickey additionally calls `ReleaseUnusedLinkSlots`; plateau, canonical C is one word short, first `+0x4` |
| `0x2C4A8` | `func_8002B8A8` | `mempool_free_addr` | B: finds an address's pool and clears its matching live slot; linked C exact |
| `0x2C53C` | `func_8002B93C` | `mempool_free_queue` | B: appends an address and delay to the deferred-free arrays; linked C exact |
| `0x2C578` | `func_8002B978` | `mempool_get_pool` | B: reverse-searches the pool table for the containing address range; linked C exact |
| `0x2C5D0` | `func_8002B9D0` | `mempool_slot_clear` | B: frees a slot and coalesces adjacent free records; linked C exact |
| `0x2C720` | `mmGetSlotPtr` | `mmGetSlotPtr` | B: returns one pool's slot-array pointer; matched C exact |
| `0x2C734` | `mmGetDelay` | `mmGetDelay` | B: returns the deferred-free delay; matched C exact |
| `0x2C740` | `func_8002BB40` | `mempool_slot_assign` | B: assigns a slot and, where needed, creates and links its remainder; plateau, exact size, 57/72 words differ, first `+0x4` |
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
`mmFree` is exact for all `0x44` bytes with canonical flags. Its branch and
two call relocations reproduce the target's immediate-free/deferred-free
selection, using the body adapted from JFG `src/memory.c`.
`func_8002B93C` is exact for all `0x3C` bytes with canonical flags; its queue
address, delay, and count accesses reproduce the JFG `mempool_free_queue`
role without the donor's diagnostic overflow branch.
`func_8002B978` is exact for all `0x58` bytes with canonical flags. Its reverse
pool-table scan is adapted from JFG `mempool_get_pool` and preserves Mickey's
pool count and 16-byte record layout.
`func_8002B700` is exact for all `0x68` bytes with canonical flags. The JFG
`mmFlushFreeStack` loop reproduces Mickey's LIFO queue drain and its call
relocation to the immediate-free worker.
`func_8002B8A8` is exact for all `0x94` bytes with canonical flags. The JFG
`mempool_free_addr` search matches after expressing Mickey's 20-byte slot
stride explicitly and retaining the linked list index at its 16-bit width.
`func_8002B9D0` is exact for all `0x150` bytes with canonical flags. Its JFG
coalescing body matches Mickey after preserving direct pool-table expressions
and natural 20-byte indexing for the allocator's recycled-slot tail.
`func_8002B4C0` is exact for all `0x64` bytes with canonical flags. The JFG
`mmAllocR` reverse pool search and zero colour tag reproduce Mickey's target
and its call relocation to the shared slot finder.
`func_8002B3A8` is exact for all `0x118` bytes with canonical flags. Its JFG
best-fit search matches with Mickey's 16-bit traversal index, retained stack
pad, and natural 20-byte slot indexing at the selected-address return.
`func_8002B1A0` is exact for all `0xE0` bytes with canonical flags. JFG's
pool initializer reproduces the pool/slot setup after applying Mickey's
byte-sized slot flags and colour index and retaining the repeated pool-table
expressions that determine IDO's schedule.
`func_8002B280` is exact for all `0x94` bytes with canonical flags. Its JFG
allocation wrapper matches after retaining Mickey's caller-colour global and
expressing the address/module scratch area as a padded stack record.
`func_8002B314` is exact for all `0x94` bytes with canonical flags. It is the
instruction-identical duplicate of the preceding JFG allocation wrapper and
uses the same padded stack-record spelling.
`func_8002B154` is exact for all `0x4C` bytes with canonical flags. JFG's
region-allocation size calculation and allocator/initializer call sequence
reproduce Mickey's target and both call relocations.
`mmInit` is exact for all `0x78` bytes with canonical flags. The JFG donor's
extended-RAM choice, main-pool construction, deferred-free delay, and queue
reset reproduce all 30 words and the linked global/call relocations.

Three JFG-derived bodies remain assembly-backed `NON_MATCHING` plateaus after
bounded source trials and the full 119-combination flag lattice. The best
`func_8002B524` candidate has the target's 116-word size with 14 positional
differences, first at `+0xE0`; the remaining mismatch is the slot/data pointer
allocation, branch-likely schedule, and one stack home (`0x38` versus target
`0x3C`). Reusing the exact wrappers' padded stack record was a new hypothesis,
but regressed to 20 differences from `+0x34`, so the prior body is retained.
The bounded permuter could not run because `tools/permuter/import.py` is absent
from this lane.

Canonical `func_8002B7AC` C emits 62 words against 63 in the target and first
diverges at `+0x4`: IDO folds the initial `D_800D21B0` address/load while the
target retains the address in a saved register, shifting the otherwise-close
queue loop. `-O2 -g3` reaches the target size with 15 differences but is not a
valid TU-wide replacement for the canonical flags. `func_8002BB40` reaches the
target's 72-word size but differs in 57 words from `+0x4`; pool/slot pointer
allocation and split-record scheduling remain structurally different.

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
| `0x20850` | `func_8001FC50`, `0x534` | JFG placeholder helper | D: function order and model-instance construction; non-matching C plateau |
| `0x20D84` | `modFreeModel`, `0xF4` | `modFreeModel` | B: instance free followed by model-reference/resource release; linked C match |
| `0x20E78` | `func_80020278`, `0x168` | JFG placeholder resource-free helper | B: texture free plus the same family of owned allocations; linked C match |
| `0x20FE0` | `func_800203E0`, `0xD8` | no adoptable name | D: model helper calls only; linked C match, placeholder retained |
| `0x210B8` | `func_800204B8`, `0xAC` | no adoptable name | D: texture/allocation release structure only; linked C exact |
| `0x21164` | `modelSetModelFlags`, `0xC` | `modelSetModelFlags` | B: paired global setter and observed callers; linked C match |
| `0x21170` | `modelGetModelFlags`, `0xC` | `modelGetModelFlags` | B: paired global getter; linked C match |
| `0x2117C` | `func_8002057C`, `0x558` | `makeModelGfx` | B: texture/display-list construction call graph and TU order; non-matching C plateau |
| `0x216D4` | `func_80020AD4`, `0x3C` | JFG placeholder in `models.c.o` | A: exact 15-word skeleton and linked C match; placeholder retained |
| `0x21710` | `func_80020B10`, `0x27C` | JFG placeholder helper | D: adjacent table-builder structure; non-matching C plateau |
| `0x2198C` | `func_80020D8C`, `0xC0` | `modSetTextureFrame` | B: model texture-frame traversal and matching TU position |
| `0x21A4C` | `func_80020E4C`, `0x1C4` | `modSuspendModelTextures` | B: allocate/save/free texture ownership sequence |
| `0x21C10` | `modResumeModelTextures`, `0x8C` | `modResumeModelTextures` | B: reload/free saved texture ownership sequence; linked C match |
| `0x21C9C` | `func_8002109C`, `0xF8` + `0xC` alignment | no adoptable name | D: model point/matrix traversal; linked C exact, JFG candidates diverge |

**PROVENANCE.** JFG's public `src/models.c`, its built `src/models.c.o`, its
`asm/nonmatchings/models/` filenames, and its published symbol map supplied
the correspondence vocabulary above. The three tier-A rows are measurements
against Mickey's ROM; every other row is explicitly an argument. No JFG body
is present in the initial all-`GLOBAL_ASM` split.
`func_800204B8` is a Mickey-only exact reconstruction for all `0xAC` bytes
under canonical `-O2 -mips2 -32`. Directly reloading the model's byte-sized
texture count reproduces the target register allocation; its texture releases,
two allocation frees, call relocations, and nulling stores are linked exact.
`func_8002109C` is exact for all `0xF8` executable bytes under the same flags;
the following `0xC` bytes are TU alignment, not function text. Its typed loop
uses a four-byte point-index record to select ten-byte signed-coordinate
records, transforms each point into a three-float output, and preserves both
call relocation identities. JFG's neighboring model helpers remain assembly,
so the body and tier-D role are reconstructed from Mickey alone.
`func_80020B10` plateaus after the complete 119-combination flag lattice,
pointer- and index-induction spellings, the measured `gSPMatrix` command macro,
and a bounded ten-minute permutation. The best coherent candidate is 160
instructions against 159 and differs first at `+0x0`: it uses a `0x20` frame
and six saved registers where the target uses `0x10` and three. The permuter's
lower-scoring mutations depended on invented guards or a potentially
uninitialized read and were rejected.
`func_8001FC50` plateaus after ten coherent allocation-layout, stack-home,
zeroing-loop, and copy-loop spellings plus a bounded ten-minute permutation.
The best canonical candidate is 330 instructions against 333, has 300
differing positional words, and first differs at `+0x0`: its frame is `0x88`
instead of `0x78`. A function-local `-Wo,-loopunroll,2` diagnostic was also
non-exact and cannot establish a TU-wide override for the already-proven
canonical consumers.
`func_8002057C` plateaus after the complete 119-combination flag lattice,
ten coherent command-emission, measured-type, copy-loop, and lifetime
spellings, and a bounded permutation. Its best canonical candidate has the
target's exact 342-instruction size but 257 positional words differ from
`+0x0`; its frame is `0xC8` rather than `0xD0`, with a different saved-register
and stack-home allocation. The permuter's lower-scoring candidate reused the
last texture parameter as a command-word temporary and would corrupt the next
part's cache comparison, so it was rejected.
`func_80020D8C` plateaus after the 119-combination flag lattice and ten
source/type/lifetime spellings. Its best `NON_MATCHING` candidate has the
target's exact 48-instruction opcode schedule, frame, and relocation surface,
but 17 register operands differ from first mismatch `+0x38`. IDO assigns the
texture-table address temporaries later in the temporary FIFO and tests the
copied loop count in `t1`, while the target uses the preceding registers and
tests the original count in `t0`; target assembly remains canonical.
`func_80020E4C` plateaus after the full flag lattice and ten coherent
source/type/control-flow spellings. Its best Mickey-only `NON_MATCHING`
candidate has the target's `0x40` frame and emits 112 instructions versus
113, with 83 differing positional words from first mismatch `+0x48`. The
candidate collapses the exception scan's zero-index scale-and-add pointer
setup into one move where the target retains both instructions; the following
temporary allocation and schedule remain shifted. Target assembly remains
canonical and no exact bytes are claimed.
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
remaining schedule. A fresh incomplete-array extern experiment likewise added
one instruction under the resident `-mips2` flags and produced 43 positional
differences; a full 119-configuration flag sweep found no exact alternative.
Driving the same extern through a direct indexed loop also emitted 51 words,
with 44 masked positional differences from `+0x1C` and 12 relocation records
against the target's 13. An alignment-qualified extern, intended to expose the
framebuffer's zero low half without an added address instruction, is not valid
IDO C syntax and was rejected by cfe. The original literal-pointer body remains
the instruction-exact candidate.
Section 1.5 therefore keeps the address label and the original asm canonical.

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
| `func_8000BD50` | `0xC950` | 0x64 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG `trackUpdateFX` three-module structure at the established tier-D TU position, with Mickey's module IDs and unresolved calls; public name deliberately not adopted; 25/25 instruction words and all six call relocations exact, linked ROM exact |
| `func_8000C400` | `0xD000` | 0x140 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG texture-animation loop at the established tier-D TU position, revised to Mickey's segment, batch, texture, and flag layout; donor placeholder deliberately not adopted; 80/80 instruction words and all five relocation records exact, linked ROM exact |
| `func_8000C540` | `0xD140` | 0xA8 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG `initSky` body structure at the established tier-D TU position, with Mickey's player-count guard and object layout; public name deliberately not adopted; 42/42 instruction words and all 11 relocation records exact, linked ROM exact |
| `trackSkySet` | `0xD1E8` | 0xC | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG `src/track.c` body; tier B role and tier D TU position; 3/3 instruction words and relocation layout exact, linked ROM exact |
| `func_8000C5F4` | `0xD1F4` | 0x684 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG flashy-sky builder at the established tier-D TU position, with DKR's published workbench documenting the donor family's load-bearing expression forms and local padding; Mickey's level-data offsets, display-list bindings, and geometry layouts are authoritative; donor placeholder deliberately not adopted; 417/417 instruction words and all 28 relocation records exact, linked ROM exact |
| `func_8000CC78` | `0xD878` | 0x258 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG background-gradient builder and display-list command forms at the established tier-D TU position, revised to Mickey's ten-byte vertex layout and resident bindings; donor placeholder deliberately not adopted; 150/150 instruction words and all 24 relocation records exact, linked ROM exact |
| `func_8000CED0` | `0xDAD0` | 0x13C | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG's assembly-only `func_80013478` supplies tier-D sky-object update structure; Mickey proves the revised mode test, fields, calls, and final draw condition, so the donor placeholder is deliberately not adopted; 79/79 instruction words and all 19 relocation records exact, linked ROM exact |
| `func_8000D00C` | `0xDC0C` | 0xC | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction; JFG's corresponding `trackGetSky` is only tier D and is deliberately not adopted; 3/3 instruction words and relocation layout exact, linked ROM exact |
| `func_8000D16C` | `0xDD6C` | 0x4C | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction; JFG's corresponding `trackAddTextureScroll` is tier D only and its public name is deliberately not adopted; 19/19 instruction words and both HI16/LO16 relocation pairs exact, linked ROM exact |
| `func_8000D570` | `0xE170` | 0xBC | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG's assembly-only `trackLightFreeMem` supplies tier-D role/TU and control-flow context; Mickey reconstruction retains the placeholder; 47/47 instruction words and all 17 relocation records exact, linked ROM exact |
| `func_8000D62C` | `0xE22C` | 0xFC | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG's assembly-only `trackLightAdd` supplies tier-D role/TU and the 0x80-byte pool stride; Mickey's stores establish the typed light record and body, so the public name is deliberately not adopted; 63/63 instruction words and all nine relocation records exact, linked ROM exact |
| `func_8000D728` | `0xE328` | 0x40 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction; JFG's corresponding `trackLightDelete` is tier D only and its public name is deliberately not adopted; 16/16 instruction words and the D_800792FC HI16/LO16 pair exact, linked ROM exact |
| `func_8000D768` | `0xE368` | 0x90 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction of the colour-ramp loop; JFG's assembly-only `trackLightColour` supplies tier-D role/TU context and its public name is deliberately not adopted; 36/36 instruction words, no relocation records, linked ROM exact |
| `func_8000D7F8` | `0xE3F8` | 0x28 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction; JFG's corresponding `trackLightMove` is tier D only and its public name is deliberately not adopted; 10/10 instruction words, no relocation records, linked ROM exact |
| `func_8000D978` | `0xE578` | 0x1BC | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG's assembly-only `trackUpdateLighting` supplies tier-D role/TU and the alternating segment-lighting structure; Mickey proves the revised module path, fields, and calls, so the public name is deliberately not adopted; 111/111 instruction words and all 28 relocation records exact, linked ROM exact |
| `func_8000F57C` | `0x1017C` | 0x2B0 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction of the bounded visible-segment distance list and adjacent-swap ordering; JFG's assembly-only `trackGetBlockList` supplies tier-D role/TU context and its public name is deliberately not adopted; 172/172 instruction words and all eight relocation records exact, linked ROM exact |
| `func_8000F82C` | `0x1042C` | 0x200 | `-O2 -mips2 -32 -Wab,-r4300_mul` | DKR `traverse_segments_bsp_tree` body adapted to Mickey's global camera/result state; JFG independently supplies tier-D TU-position context, but neither donor name is adopted; 128/128 instruction words and all 18 relocation records exact, linked ROM exact |
| `func_8000FA2C` | `0x1062C` | 0xB4 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction of the camera/BSP range setup wrapper; the reference scan found no credible donor and the placeholder is retained; 45/45 instruction words and all 19 relocation records exact, linked ROM exact |
| `func_8000FBD8` | `0x107D8` | 0xCC | `-O2 -mips2 -32 -Wab,-r4300_mul` | DKR `check_if_inside_segment` bounding-box containment structure adapted to Mickey's direct coordinates and inclusive bounds; donor name deliberately not adopted; 51/51 instruction words and the `D_800792E8` HI16/LO16 relocation pair exact, linked ROM exact |
| `func_8000FCA4` | `0x108A4` | 0xC4 | `-O2 -mips2 -32 -Wab,-r4300_mul` | DKR `get_inside_segment_count_xz` body adapted to Mickey's 16-bit output indices and resident bindings; donor name deliberately not adopted; 49/49 instruction words and both `D_800792E8` HI16/LO16 relocation pairs exact, linked ROM exact |
| `func_8000FD68` | `0x10968` | 0x14C | `-O2 -mips2 -32 -Wab,-r4300_mul` | DKR `get_inside_segment_count_xyz` body adapted to Mickey's resident track and bounding-box types; JFG independently supplies tier-D `trackGetCubeBlockList` context, but the public name is deliberately not adopted; 83/83 instruction words and both `D_800792E8` HI16/LO16 relocation pairs exact, linked ROM exact |
| `func_8000FEB4` | `0x10AB4` | 0x38 | `-O2 -mips2 -32 -Wab,-r4300_mul` | DKR `block_get` accessor structure with Mickey's stricter upper bound and 0x40-byte segment layout; donor name deliberately not adopted; 14/14 instruction words and the `D_800792E8` HI16/LO16 relocation pair exact, linked ROM exact |
| `func_8000FEEC` | `0x10AEC` | 0x40 | `-O2 -mips2 -32 -Wab,-r4300_mul` | DKR `block_boundbox` body and 12-byte bounding-box layout; donor name deliberately not adopted; 16/16 instruction words and the `D_800792E8` HI16/LO16 relocation pair exact, linked ROM exact |
| `func_8000FF2C` | `0x10B2C` | 0x24C | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction of three transformed plane equations; JFG's same-position assembly-only placeholder supplies tier-D structure/TU context and is deliberately not adopted; 147/147 instruction words and all nine relocation records exact, linked ROM exact |
| `func_80010178` | `0x10D78` | 0x25C | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction of the visibility gate and three-plane AABB test; the reference scan found no credible donor and the placeholder is retained; 151/151 instruction words and all 11 text relocation records exact, linked ROM exact |
| `func_800131AC` | `0x13DAC` | 0x178 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG's assembly-only `trackClip3D` supplies the tier-D six-plane clipping structure and paired helper context; Mickey proves the shorter boundary and exact body, so the public name is deliberately not adopted; 94/94 instruction words and all ten relocation records exact, linked ROM exact |
| `func_80013324` | `0x13F24` | 0xD8 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction of the interval-clipping helper; the reference scan found no credible donor and the placeholder is retained; 54/54 instruction words and all four relocation records exact, linked ROM exact |
| `trackGetTrack` | `0x14AB4` | 0xC | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction with JFG name (tier B callers); 3/3 instruction words and relocation layout exact, linked ROM exact |
| `func_80013EC0` | `0x14AC0` | 0x20C | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG's assembly-only `trackFreeAll` supplies tier-D teardown structure/TU context; Mickey proves the resident calls, fields, and source spelling, so the public name is deliberately not adopted; 131/131 instruction words and all 56 relocation records exact, linked ROM exact |
| `trackSetFog` | `0x15030` | 0xF8 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG `src/track.c` body with tier B callers and tier D TU order; 62/62 instruction words and relocation layout exact, linked ROM exact |
| `trackGetFog` | `0x15128` | 0x78 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG direct-path body with tier B caller and tier D TU order; 30/30 instruction words and relocation layout exact, linked ROM exact |
| `trackSetFogOff` | `0x151A0` | 0x74 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG `src/track.c`; 29/29 instruction words and relocation layout exact, linked ROM exact |
| `func_80014614` | `0x15214` | 0x190 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction of the fog-state updater; JFG same-position skeleton is the 0.733 top hit but its placeholder is not imported; 100/100 instruction words and relocation layout exact, linked ROM exact |
| `func_800147A4` | `0x153A4` | 0x13C | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction using the SDK fog-colour/position macros; JFG same-size top skeleton supplies structural context but its placeholder is not imported; 79/79 instruction words and relocation layout exact, linked ROM exact |
| `func_800148E0` | `0x154E0` | 0x2CC | `-O2 -mips2 -32 -Wab,-r4300_mul` | DKR `obj_loop_fogchanger` body and declaration order adapted to Mickey's direct player-list call, 0x54 fallback stride, object offsets, and 0x40 fog records; JFG independently supplies tier-D `trackChangeFog` TU context, but the public name is deliberately not adopted; 179/179 instruction words and all three text relocation records exact, linked ROM exact |
| `func_80014BAC` | `0x157AC` | 0x238 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG `trackFadeFog` body at the established tier-D TU position; its public name is deliberately not adopted; 142/142 instruction words and both HI16/LO16 relocation pairs exact, linked ROM exact |
| `func_80014DE4` | `0x159E4` | 0xC8 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction; JFG supplies only tier-D transform-role context and no public name is adopted; 50/50 instruction words and relocation layout exact, linked ROM exact |
| `func_80014EAC` | `0x15AAC` | 0x20 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG `func_8001C550` is a tier-A 8/8-word TU donor, unique in the ROM; JFG placeholder not imported; linked ROM exact |
| `func_80014ECC` | `0x15ACC` | 0x668 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction using the SDK GBI display-list macros; JFG's assembly-only final `track.c.o` helper supplies tier-D TU-position and structural context, but its placeholder is deliberately not adopted; 410/410 instruction words and all 22 relocation records exact, linked ROM exact |

Current matching plateau:

| Function | Target | Best attempt | First mismatch and blocker |
|---|---:|---|---|
| `func_80010B4C` | 0xA98 / 678 words | Four serious source forms reconstructed the full JFG `trackGetPlayerIntersect` role from Mickey's m2c draft. The closest was 0xA60 / 664 words under `-O2 -mips2 -32`; the required flag sweep found no exact compiler mode. | `+0x0`: target frame is `0x148`, versus `0x128`; the target also retains two additional FP live ranges and uses a different pointer-variable ordering for its unrolled copy/initialization loops. JFG's public body is assembly-only, so another attempt needs new source/declaration evidence. The bounded permuter could not run because this lane has no `tools/permuter/import.py`. |
| `func_80012574` | 0xE4 / 57 words | JFG's assembly-only `trackSphereIntersect` is the 0.316 nearest skeleton and confirms the role and 0x48 frame. Mickey's reconstructed ray/sphere body under `-O2 -mips2 -32 -Wab,-r4300_mul` reached the exact 57-word opcode schedule, exact frame, and exact `sqrtf` relocation, with six differing words. | `+0x54`: the projection-square and distance-square webs are exchanged between `f16` and `f18`; the saved boolean also occupies `sp+0x18` instead of `sp+0x1C`. Further source-local additions overshot the frame to 0x50, so another attempt needs new declaration-order or original-source evidence rather than more register guessing. |
| `func_8000FAE0` | 0xF8 / 62 words | DKR `get_level_segment_index_from_position` and JFG `trackGetBlock` establish the segment-scan role. Mickey's inclusive-bound/nearest-edge reconstruction reached the exact 62-word size and exact two-relocation surface under `-O2 -mips2 -32`; the clean bounded-permuter improvement caches the upper Y bound and leaves 43 positional word differences. | `+0x1C`: IDO colors the cached segment count into `a0` instead of Mickey's `t0`, cascading into the converted-coordinate register and short-circuit branch forms. The lower numerical permuter candidate illegally hoisted a changing bound across `bounds++`; another attempt needs original local/loop evidence, not unsafe score-only transformations. |
| `func_8000D018` | 0x154 / 85 words | JFG's assembly-only `func_800135E0` establishes the camera/update skeleton. Mickey's reconstruction reaches all 85 instruction words and all 32 target relocations under `-O2 -mips2 -32` when `TrapDanglingJump` has the required three-float prototype. | Relocation `+0xF4`: the same TU's already-exact `func_8000BD50` calls `TrapDanglingJump` with an incompatible one-integer ABI. A function-pointer cast changes those calls to `jalr`; an alias keeps both instruction streams exact but binds this relocation to the alias rather than `TrapDanglingJump`. Another attempt needs an IDO declaration mechanism that preserves two direct-call ABIs and the canonical relocation identity. |
| `func_8000D820` | 0x158 / 86 words | JFG's assembly-only `func_80013DCC` establishes the track-light colour-update structure. Mickey's reconstruction under `-O2 -mips2 -32` reaches the exact 86-word opcode schedule and all six target relocations; a 119-mode flag sweep found no better mode, and the bounded permuter reduced the residual to 33 register-only word differences. | `+0x3C`: IDO assigns the active-segment load to `a2` instead of the target's `t7`, then colors the positive-copy temporaries and dirty-mask loop differently. The candidate has no opcode, schedule, frame, size, or relocation mismatch; another attempt needs new original declaration/lifetime evidence rather than further register-order guessing. |
| `func_800133FC` | 0x180 / 96 words | Mickey's reconstructed three-point plane helper under `-O2 -mips2 -32 -Wab,-r4300_mul` reaches the exact 96-word length, 0xA0 frame, and `sqrtf` relocation. The required 119-mode flag sweep uniquely selected the R4300 multiply schedule, and a bounded ten-minute permuter plus ten source/lifetime hypotheses reduced the residual to 58 positional words. | `+0x18`: IDO loads the point coordinates in a different order and gives the retained integer coordinates and pre-normalized components different stack homes, cascading through the GPR and FP webs. The reference scan found no credible source donor; another attempt needs original declaration/lifetime evidence rather than more register-order guessing. |
| `func_8000D3B8` | 0x1B8 / 110 words | JFG's assembly-only `trackLightAllocate` is the 0.460 nearest skeleton and establishes the full allocation/copy algorithm. Mickey's typed reconstruction under `-O2 -mips2 -32` is one instruction short; the bounded permuter's best semantics-preserving integer-widening variant reaches the exact 110-word length, 0x38 frame, and all 21 target relocations, with 50 positional words and two opcodes differing. | `+0x4`: IDO exchanges the long-lived count-global pointer and copy-mode argument between `s0` and `s1`, then colors the segment index, allocation record, and segment pointer into different saved registers. The score improvement depends on an unproved widening identity in the allocation-size expression, so it is retained only as an ignored workbench artifact; another attempt needs original declaration/expression evidence. |
| `func_8000DDE4` | 0x1D8 / 118 words | Mickey's reconstructed key-filter and pointer-sort body under `-O2 -mips2 -32` reaches the exact 118-word opcode schedule, 0x28 frame, and both call relocations. The required 119-mode flag sweep found no better mode; ten coherent source/lifetime variants and the bounded ten-minute permuter reduced the residual to 15 register-only words. | `+0x24`: IDO keeps the unrolled scan induction web in `v1` instead of the target's UGEN-only `t1`, rotating the remainder value through `a3`/`a0` instead of `a0`/`v1`. The reference scan found no credible donor, and hundreds of permuter spellings collapsed above zero; another attempt needs original loop/local evidence rather than more register-order guessing. |
| `func_8000D1B8` | 0x200 / 128 words | JFG's assembly-only `trackUpdateTextureScroll` is the 0.440 nearest skeleton and establishes the packed-scroll and nested segment/batch/vertex loops. Mickey's typed layout adaptation under `-O2 -mips2 -32` reaches the exact 128-word opcode schedule, 0x28 frame, and all eight target relocations; the 119-mode flag sweep and bounded ten-minute permuter leave 24 register-only words. | `+0x38`: IDO exchanges the scroll count and command pointer between `s0` and `s1`; at `+0x110`, the vertex-range setup enters a different temporary-FIFO phase, rotating `t6`, `t8`, and `t9`. The permuter's lower numerical candidates overwrote live scroll state and were rejected; another attempt needs original declaration/coalescing evidence rather than unsafe score-only assignments. |
| `func_80010900` | 0x24C / 147 words | Mickey's reconstructed repeated segment-intersection wrapper under `-O2 -mips2 -32 -Wab,-r4300_mul` identifies the 0x20-byte callback record and reaches the exact 147-word opcode schedule, 0xB8 frame, every stack offset, FP allocation, and all five call relocations. The 119-mode flag sweep found no better mode; a bounded ten-minute permuter and ten type, declaration, lifetime, and call-schedule hypotheses leave 17 register-only words. | `+0x14`: one clean saved-register bijection assigns the direction pointer, intersection pointer, and secondary result to `s4`, `s5`, and `s6` instead of the target's `s5`, `s6`, and `s4`. The reference scan found no credible donor, and explicit pointer/return-category variants reproduced the same allocator basin; another attempt needs original declaration or forced-color evidence rather than more register-order guessing. |
| `func_800103D4` | 0x280 / 160 words | JFG's assembly-only `func_80015D54` and DKR's `check_if_in_draw_range` establish the object-alpha, camera-distance fade, and three-plane visibility structure. Mickey's complete reconstruction reaches the exact 160-word length and branch/call schedule under `-O2 -mips2 -32 -Wab,-r4300_mul`; the required 119-mode flag sweep found no exact alternative. | `+0x0`: retaining the four plane temporaries gives IDO the target-style saved FP web but also saves `f26` and expands the frame to `0x58` instead of `0x38`; removing those temporaries drops the candidate to 153 words and loses the target's `f20`/`f22`/`f24` allocation. Another attempt needs original declaration/storage-class evidence that produces the three-register web without six scalar stack homes. |
| `func_80010654` | 0x2AC / 171 words | DKR's public `resolve_collisions` and Mickey's m2c draft establish the candidate-stream, base-plane, signed three-edge, and nearest-intersection algorithm. The best semantics-preserving reconstruction under `-O2 -mips2 -32 -Wab,-r4300_mul` reaches the exact 171-word length and 0x98 frame; the 119-mode flag sweep found no exact mode, and the bounded permuter's clean product-temporary variant leaves 129 positional words, 48 opcodes, and ten relocation positions differing. | `+0x4`: the candidate saves and hoists the `D_80081774` address in `s7`, while the target saves only `s0`–`s6` and loads that value through `at` inside the loop; the output pointer and three inner-loop state values consequently occupy a different register web. A direct `volatile` storage-class test produced identical code, so another attempt needs original declaration/scope evidence that prevents the hoist and colors the long-lived values without an extra saved register. |
| `func_8000DB34` | 0x2B0 / 172 words | Mickey's m2c draft establishes the inverse segment-order table, descending object scan, integer AABB test, and eight-byte result records; the reference skeleton scan found no credible donor. The best semantics-preserving reconstruction under `-O2 -mips2 -32 -Wab,-r4300_mul` reaches 170 words with a 0x188 frame; its 168 positional words, 127 opcodes, and 14 relocation-metadata sites differ. The 119-mode flag sweep found no exact mode, and the bounded permuter's exact-length forms did not improve the full-TU structure. | `+0x0`: the candidate allocates a 0x188 frame and places the 256-byte inverse table at `sp+0x84`, while the target uses a 0x190 frame, places the table at `sp+0x5C`, and retains a separate 0x20-byte local region before the object-count slot. This changes all three argument colors and the later loop register web. Another attempt needs original local-structure and declaration-order evidence; expression permutations do not recover that layout. |
| `func_80012658` | 0x2C4 / 177 words | JFG's public assembly-only collision-edge builder supplies the role and loop structure. Mickey's typed reconstruction reaches the exact 177-word length; 109 positional words and two relocation-metadata sites remain after the 119-mode flag sweep and corrected ten-minute `-mips2` permuter pass (score 3065, no improvement). | `+0x0`: the candidate frame is 0x38 versus the target's 0x40, which changes the retained segment-pointer spill and saved-register/global-address web. Another attempt needs original local-object or declaration evidence rather than further expression permutations. |
| `func_8000E5EC` | 0x334 / 205 words | Mickey's reconstruction and DKR's `render_level_geometry_and_objects` establish the 128-entry visible-segment list, visibility-map setup, lighting/particle passes, and per-segment dispatch. The best corrected `-mips2` permuter candidate is 206 words with the exact 0xD8 frame; score 1525, 192 positional words and 73 relocation-metadata sites differ after the 119-mode sweep. | `+0x4`: the target reuses the track-global address in `s2` and places the segment list/count at `sp+0x44`/`sp+0xCC`, while the candidate enters a different saved-register/local-placement web. Another attempt needs original declaration/storage evidence rather than register-order guessing. |
| `func_80012234` | 0x340 / 208 words | JFG's assembly-only `trackCylinderIntersect` confirms Mickey's line/cylinder intersection structure. The best source-plausible reconstruction has the exact 208-word length, 0x60 frame, three `sqrtf` relocations, and every target local-vector stack home; 153 positional words remain after the 119-mode sweep. The corrected ten-minute permuter reached score 1235 and 107 words only through a rejected code-free comma expression. | `+0x8`: the source-plausible candidate starts the temporary-FP ring at `f6` instead of the target's `f8`, producing a one-step `f4`/`f6`/`f8`/`f10` rotation across 149 register-only sites plus two schedule sites. Another attempt needs original scalar declaration/lifetime evidence; aggregate, scalar, split-length, and extra-temporary forms did not recover it. |

### 3.10 Resident camera: ROM `0x21EE0`–`0x25C20`

This whole `0x3D40`-byte block is the resident camera TU: **69 functions,
`0x3D3C` executable bytes and four bytes of terminal alignment**. Its ordered
systems are camera/FOV state, user viewports, projection setup, sprite and
model matrices, projection helpers, then screen shake. The split is
`main/camera`; flags are `-O2 -mips2 -32 -Wab,-r4300_mul`, with the multiply
scheduler mode fixed by the exact `camGetProjZ` projection-depth dot product.

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
| `camInit` | `0x21EE0` | B — JFG role/call graph and nearest camera skeleton | 344 | JFG body adapted to Mickey's six-camera array, reset routine and projection globals; configured object, 25 text relocations, linked range and full ROM exact. |
| `func_80021438` | `0x22038` | D — retained Mickey auto-name; camera TU position only | 12 | Mickey-only global read; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `camUseShake` | `0x22084` | B — role/order | 16 | Configured object, relocation pair, linked range and full ROM exact. |
| `camOverrideProjScales` | `0x220E4` | B — role/order (named above) | 32 | Configured object, six relocations, linked range and full ROM exact. |
| `func_800217AC` | `0x223AC` | D — retained Mickey auto-name; camera TU position only | 12 | Mickey-only matrix pointer getter; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `camDistance` | `0x223B8` | D — JFG TU role/order and camera-position dataflow | 128 | JFG body adapted to Mickey's active-camera array; configured object, five relocations, linked range and full ROM exact. |
| `camSetWaterLine` | `0x225B0` | A — byte identity (named above) | 32 | Configured object, relocation pair, linked range and full ROM exact. |
| `camGetProjOrgMtx` | `0x25270` | B — role/order (named above) | 28 | Configured object, two relocation pairs, linked range and full ROM exact. |
| `camSetZoom` | `0x258C8` | B — role/order (named above) | 56 | Configured object, two relocation pairs, linked range and full ROM exact. |
| `camGetPlayerProjMtx` | `0x23360` | A — byte identity (named above) | 52 | Configured object, five relocations, linked range and full ROM exact. |
| `camStopShakes` | `0x25754` | B — role/order (named above) | 76 | Configured object, three relocation pairs, linked range and full ROM exact. |
| `camStartShake` | `0x256C4` | D — JFG TU role/order and shake-record dataflow | 144 | JFG body adapted to Mickey's six-camera bound; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `func_80024ED8` | `0x25AD8` | D — retained Mickey auto-name; fixed-distance camera-transform dataflow | 324 | Mickey-only body; 81 executable instructions and nine text relocations are exact. The following four-byte TU alignment NOP is reproduced by compiler section padding; linked range and full ROM exact. |
| `camIgnoreShake` | `0x22094` | B — role/order | 12 | Configured object, relocation pair, linked range and full ROM exact. |
| `camGetFOV` | `0x220A0` | B — role/order | 12 | Configured object, relocation pair, linked range and full ROM exact. |
| `func_80021444` | `0x22044` | D — retained Mickey auto-name; paired camera-state effect only | 64 | Mickey-only bounded state setter; configured object, two HI16/LO16 relocation pairs, linked range and full ROM exact. |
| `func_800214AC` | `0x220AC` | D — retained Mickey auto-name; active-camera state effect only | 56 | Mickey-only active-camera byte toggle; configured object, two HI16/LO16 relocation pairs, linked range and full ROM exact. |
| `func_80021838` | `0x22438` | D — retained Mickey auto-name; DKR reset role and JFG camera TU position only | 224 | DKR reset body adapted to Mickey's extended camera fields and store order; configured object, six relocations, linked range and full ROM exact. |
| `camGetWaterLine` | `0x225A0` | D — TU order only, no per-symbol callgraph argument recorded | 16 | Configured object, relocation pair, linked range and full ROM exact. |
| `camGetMode` | `0x22518` | D — TU order only, no per-symbol callgraph argument recorded | 12 | Configured object, relocation pair, linked range and full ROM exact. |
| `camSetMode` | `0x22524` | D — TU order only, no per-symbol callgraph argument recorded | 64 | Configured object, two relocation pairs, linked range and full ROM exact. |
| `camGetNo` | `0x22564` | D — TU order only, no per-symbol callgraph argument recorded | 12 | Configured object, relocation pair, linked range and full ROM exact. |
| `func_80021970` | `0x22570` | D — retained Mickey auto-name; indexed camera-array role only | 36 | Mickey-only indexed camera-array getter; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `camSetNo` | `0x22594` | D — TU order only, no per-symbol callgraph argument recorded | 12 | Configured object, relocation pair, linked range and full ROM exact; Mickey omits JFG's bounds guard. |
| `camEnableUserView` | `0x22770` | D — JFG TU role/order and viewport-flag dataflow | 116 | JFG body adapted to Mickey's viewport array; configured object, two HI16/LO16 relocation pairs, linked range and full ROM exact. |
| `camDisableUserView` | `0x227E4` | D — JFG TU role/order and viewport-flag dataflow | 120 | JFG body adapted to Mickey's viewport array; configured object, two HI16/LO16 relocation pairs, linked range and full ROM exact. |
| `camIsUserView` | `0x2285C` | D — JFG TU role/order | 44 | JFG body adapted to Mickey's viewport-flags symbol; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `func_80021C88` | `0x22888` | D — retained Mickey auto-name; JFG `camSetUserView` role/order | 364 | DKR `viewport_menu_set` body adapted to Mickey's video-size call and viewport layout; configured object, seven text relocations, linked range and full ROM exact. |
| `camSetUserViewSpecial` | `0x229F4` | D — JFG TU role/order and viewport-field dataflow | 252 | JFG body adapted to Mickey's viewport array; configured object, four relocations, linked range and full ROM exact. |
| `camGetVisibleUserView` | `0x22AF0` | D — JFG TU role/order and viewport-scissor dataflow | 120 | JFG body adapted to Mickey's viewport array; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `camGetUserView` | `0x22B68` | D — JFG TU role/order and viewport-field dataflow | 72 | JFG body adapted to Mickey's viewport array; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `func_80021FB0` | `0x22BB0` | D — retained Mickey auto-name; JFG `camGetWindowLimits` role/order | 568 | JFG body adapted for Mickey's inset margins and split-orientation state; configured object, nine text relocations, linked range and full ROM exact. |
| `func_800221E8` | `0x22DE8` | D — retained Mickey auto-name; JFG `camSetView` role/order | 1,052 | JFG body adapted for Mickey's region flag, half-resolution and zoom state; configured object, 19 text relocations, linked range and full ROM exact. |
| `func_80022604` | `0x23204` | D — retained Mickey auto-name; camera TU position only | 12 | Mickey-only global setter; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `camSetScissor` | `0x23210` | D — JFG TU role/order and scissor-command dataflow | 336 | JFG role adapted to Mickey's window-limit helper and scissor encoding; configured object, five text relocations, linked range and full ROM exact. |
| `func_80022794` | `0x23394` | D — retained Mickey auto-name; JFG `camSetProjMtx` role/order | 676 | JFG body adapted for Mickey's extra camera-state FOV check; configured object, 40 text relocations, linked range and full ROM exact. |
| `camOrthoYAspect` | `0x23638` | D — JFG TU role/order | 12 | JFG body and masked skeleton exact; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `func_80022A44` | `0x23644` | D — retained Mickey auto-name; camera TU position only | 12 | Mickey-only float-state setter; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `camStandardOrtho` | `0x23650` | D — JFG TU role/order and orthographic viewport dataflow | 324 | JFG body adapted for Mickey's half-resolution alternate viewport bank; configured object, 18 text relocations, linked range and full ROM exact. |
| `camStandardPersp` | `0x23794` | D — JFG TU role/order and perspective-matrix dataflow | 196 | JFG body adapted to Mickey's camera transform and matrix globals; configured object, 17 relocations, linked range and full ROM exact. |
| `camSetViewport` | `0x23858` | D — JFG TU role/order and viewport dataflow | 200 | JFG body adapted for Mickey's alternate viewport bank and horizontal region flip; configured object, 10 relocations, linked range and full ROM exact. |
| `func_80022D20` | `0x23920` | D — retained Mickey auto-name; JFG `camResetView` role/order | 352 | JFG body adapted to Mickey's viewport flags and region-flip argument; configured object, 12 text relocations, linked range and full ROM exact. |
| `func_80022E80` | `0x23A80` | D — retained Mickey auto-name; camera-relative billboard-offset dataflow | 340 | Mickey-only body; configured object, 23 text relocations, linked range and full ROM exact. |
| `func_80023A08` | `0x24608` | D — retained Mickey auto-name; JFG `camDoSprite` role/order | 708 | JFG body adapted for Mickey's one-shot projection flip and display-list encoding; configured object, 24 text relocations, linked range and full ROM exact. |
| `func_80023CCC` | `0x248CC` | D — retained Mickey auto-name; JFG `camDoSpriteDirect` role/order | 696 | JFG body adapted for Mickey's secondary matrix scale, one-shot projection flip and display-list encoding; configured object, 21 text relocations, linked range and full ROM exact. |
| `func_80023F84` | `0x24B84` | D — retained Mickey auto-name; JFG `camDo2DSprite` role/order | 640 | JFG body adapted to Mickey's 10-byte vertex layout, resident transforms and display-list encoding; configured object, 23 text relocations, linked range and full ROM exact. |
| `camPushFloatModelMtx` | `0x24E04` | D — JFG TU role/order and float-model matrix dataflow | 220 | JFG body adapted to Mickey's matrix globals and display-list encoding; configured object, 14 relocations, linked range and full ROM exact. |
| `camPushMuzzleMtx` | `0x24EE0` | D — JFG TU role/order and muzzle-matrix dataflow | 332 | JFG body adapted to Mickey's matrix globals and display-list encoding; configured object, 16 text relocations, linked range and full ROM exact. |
| `camScaleModelMtx` | `0x2502C` | D — JFG TU role/order and model-scale matrix dataflow | 192 | JFG body adapted to Mickey's matrix globals and display-list encoding; configured object, 15 relocations, linked range and full ROM exact. |
| `camPushModelMtx` | `0x250EC` | D — JFG TU role/order and model-matrix dataflow | 256 | JFG body adapted to Mickey's transform, matrix globals and display-list encoding; configured object, 21 relocations, linked range and full ROM exact. |
| `camRestoreModelMtx` | `0x251EC` | D — JFG TU role/order | 32 | JFG display-list body adapted to Mickey's Gfx layout; configured object, relocation-free linked range and full ROM exact. |
| `camPopModelMtx` | `0x2520C` | D — JFG TU role/order | 32 | JFG display-list body adapted to Mickey's Gfx layout; configured object, relocation-free linked range and full ROM exact. |
| `camGetPtr` | `0x2522C` | B — Mickey/JFG weather call-graph correspondence and active-camera dataflow | 44 | JFG body adapted to Mickey's 0x54-byte Camera stride; configured object, two HI16/LO16 relocation pairs, linked range and full ROM exact. |
| `camGetListPtr` | `0x25258` | D — JFG TU role/order | 12 | JFG body and masked skeleton exact; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `camGetInvProjMtx` | `0x25264` | D — JFG TU role/order and Mickey matrix dataflow | 12 | JFG body and masked skeleton exact; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `func_8002468C` | `0x2528C` | D — retained Mickey auto-name; camera matrix dataflow only | 12 | Mickey standalone perspective-matrix getter; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `camGetRotationMtx` | `0x25298` | B — Mickey/JFG weather call-graph correspondence and camera matrix dataflow | 12 | JFG body and masked skeleton exact; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `camGetProjectionMtx` | `0x252A4` | D — JFG role and Mickey final projection-matrix dataflow | 12 | JFG body and masked skeleton exact; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `func_800246B0` | `0x252B0` | D — retained Mickey auto-name; JFG `camProjectPoint` role/order | 388 | Mickey matrix/viewport reconstruction; configured object, 13 text relocations, linked range and full ROM exact. |
| `camGetProjZ` | `0x25538` | D — JFG TU role/order and projection-depth dataflow | 64 | JFG body adapted to Mickey's rotation matrix; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `func_80024D00` | `0x25900` | D — retained Mickey auto-name; JFG `camTick` role/order | 472 | Mickey shake-envelope tick reconstruction for six cameras; configured object, 11 text relocations, linked range and full ROM exact. |

Bounded plateau:

| Function | ROM | Evidence and retained result |
|---|---:|---|
| `func_80021504` | `0x22104` | D — retained Mickey auto-name; JFG `camSetFOV` supplies the role and starting projection body. After the full flag lattice, nine coherent source/type/lifetime variants and a bounded two-worker permuter batch, the best configured candidate has the exact 532-byte size and 133 instructions but differs in 11 positional words from first mismatch `+0x1D4`. The first 117 instructions are exact; only temporary registers in the final projection-matrix ring update differ. The permuter's lower score moved the mandatory perspective rebuild inside the state-mirror branch and was rejected as semantically invalid; assembly remains canonical. |
| `func_800219D0` | `0x225D0` | D — retained Mickey auto-name; DKR `copy_viewports_to_stack` supplies the body and JFG supplies the `camUserViewTick` role/order. After the full flag lattice, eight coherent expression/lifetime variants and a bounded two-worker permuter batch, the best configured candidate has the exact 416-byte size and 104 instructions but differs in 17 positional words from first mismatch `+0x98`: the output index uses the opposite commutative `addu` operand order, then IDO makes different scheduling/register choices in the viewport extent expressions. The permuter's only lower score masked a signed 32-bit coordinate to 16 bits and was rejected as semantically invalid; assembly remains canonical. |
| `func_80021718` | `0x22318` | D — retained Mickey auto-name; DKR `cam_reset_fov` is the 0.439 nearest skeleton and supplies the projection-reset body shape. After the full flag lattice, six semantics-preserving source/type/address variants, and a bounded two-worker permuter batch, the best candidate has the exact 148-byte size, 37 instructions and relocation surface but differs in 11 positional words from first mismatch `+0x4C`, all in temporary-register allocation for the rotating matrix-slot update. The permuter's score-195 candidate removed the required ring mask and invented a dead guard, so it was rejected; assembly remains canonical. |
| `func_80024834` | `0x25434` | D — retained Mickey auto-name; JFG `camReversePoint` is the nearest camera-TU role/skeleton. After the full flag lattice and a bounded two-worker permuter batch, the best configured candidate emits 66 instructions against 65 and differs in 59 positional words from first mismatch `+0x0`: `-Wab,-r4300_mul` reduces the frame from target `0x38` to `0x28` and removes the target's dead float spill. The same semantic body is byte-exact without that required TU override; assembly remains canonical. |
| `func_80024978` | `0x25578` | D — retained Mickey auto-name; JFG `camCopyOrthoMatrix` supplies the role and loop body, with Mickey adding its projection scale. After the full flag lattice, eight coherent source/type/indexing variants, and a bounded two-worker permuter batch, the best configured candidate emits 84 instructions against 83 and differs in 59 positional words from first mismatch `+0x5C`: IDO emits one extra address materialization for the third peeled coefficient, likely because the reconstruction sees an extern array rather than the original same-TU data definition. The permuter's score-135 base received no improvement; assembly remains canonical. |
| `func_80024BA0` | `0x257A0` | D — retained Mickey auto-name; JFG `camScreenShake` supplies only the camera-TU role/order while Mickey establishes the distance-based shake body. After the full flag lattice, ten coherent source/lifetime spellings and a bounded two-worker permuter batch, the best configured candidate has the exact 296-byte size and differs in 15 positional words from first mismatch `+0x60`: IDO assigns the long-lived `$f20` register to the Z delta instead of the target's X delta, cascading through the arithmetic temporaries. The permuter's best score is 125, not zero; assembly remains canonical. |
| `func_80022FD4` | `0x23BD4` | D — JFG supplies only the `camDoSprite` role/order; the Mickey-only `NON_MATCHING` reconstruction plateaued after the flag lattice and ten source/lifetime variants. The best `-Wab,-r4300_mul` build has the exact `0xB0` frame and emits 366 instructions against 369, with 297 positional words differing from first mismatch `+0x2C` because IDO places three coordinate stack homes twelve bytes above the target before a later three-instruction schedule deficit. The assembly remains canonical. |
| `func_80023598` | `0x24198` | D — retained Mickey auto-name; camera-TU placement and call to the matched sprite-direct helper. The Mickey-only `NON_MATCHING` reconstruction plateaued after the full flag lattice and ten coherent control-flow, type, lifetime and parameter-homing variants. The best configured candidate emits 286 instructions against 284 and differs in 275 positional words from first mismatch `+0x0`: IDO retains the display-list argument in `$s1` and allocates a `0xA0` frame, while the target spills that argument and uses a `0x90` frame. The assembly remains canonical. |

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

`func_80038750` remains a **tier-D NON_MATCHING plateau** over **0x128 bytes /
74 words** at ROM `0x39350`. Its Mickey-derived candidate has the exact
`0x18`-byte frame and reconstructs the language-specific resource selection,
section load, and pointer-relocation loop. The best stock-flag object differs
in **6/74 positional words**, first `+0xDC`. Word-table state typing and
source load order recover the cached-header and destination schedules; the
remaining executable residual is one swapped `a0`/`a1` pool web, including a
commutative operand-order site. Its compiler-generated switch still binds two
relocation sites, first `+0x4C`, to anonymous `.rodata` rather than the
target's named jump-table symbol. The full flag lattice kept the resident
defaults best. JFG's corresponding `setLanguage` routine remains assembly, so
the Mickey placeholder name and canonical assembly are retained.

`func_80038878` remains a **tier-D NON_MATCHING plateau** over **0x154 bytes /
85 words** at ROM `0x39478`. JFG's same-position `initFront` routine supplies
the role and structural comparison but remains assembly, so the candidate is
Mickey-derived and retains its placeholder name. Under
`-Wo,-loopunroll,0`, the best candidate has the exact `0x18`-byte frame and
instruction count, but differs in **66/85 positional words**, first `+0x14`.
IDO reuses the address formed for the initial `D_800D3150` allocation store,
where the target rematerializes it before walking the pointer table; that
early lifetime difference cascades into the later clear and sentinel-loop
address schedule. The target also forms one shared high address for four
descending control-mode stores, while scalar declarations emit four address
pairs and an array declaration loses three required constant loads. The stock
flags expand the reset loops; the full flag lattice makes the no-unroll form
the only exact-size result. Ten coherent type, expression-tree, pointer-reuse,
scalar/array, and volatile variants did not satisfy the word and relocation
gates. A later chained-assignment retry made the function three instructions
longer and was rejected. Canonical code remains assembly and no TU flag
override is adopted.

`func_800389CC` reaches an **exact-text tier-D NON_MATCHING plateau** over
**0x1F8 bytes / 126 words** at ROM `0x395CC`. JFG's C
`src/menu.c::frontFreeMode` supplies the role, high-level lifetime, and switch
ordering; Mickey supplies the smaller 19-mode switch and resident state. The
resident default flags emit every executable word and the exact `0x18`-byte
frame. Promotion is blocked by section ownership: IDO emits the switch's
`0x4C` bytes as anonymous TU-local `.rodata`, while the named `jtbl_80082748`
copy remains inside the shared `0x81590` yaml slice. Compiling both would
duplicate the table. The first metadata mismatch is the target assembly's
external-label PC16 at function `+0x2C`; the named-versus-anonymous table
HI16/LO16 pair follows at `+0x34`/`+0x3C`. The 119-combination flag lattice
confirms the stock resident flags and does not change the ownership surface.
Canonical code and shared rodata remain assembly pending a measured yaml
handoff outside this lane's ownership.

`func_80038BC4` reaches an **exact-text tier-D NON_MATCHING plateau** over
**0x1E8 bytes / 122 words** at ROM `0x397C4`. JFG's same-position
`frontInitMode` supplies the role and switch-order comparison but retains an
assembly body; the candidate is Mickey-derived and keeps its placeholder
name. Explicit no-op cases zero and one recover the target's 19-entry range
check, after which the resident default flags emit every executable word and
the exact `0x18`-byte frame. Promotion is blocked by section ownership: IDO
emits the switch's `0x4C` bytes as anonymous TU-local `.rodata`, while the
named `jtbl_80082794` copy remains inside the shared `0x81590` yaml slice.
Compiling both would duplicate the table. The first metadata mismatch is the
target assembly's external-label PC16 at function `+0x2C`; the named-versus-
anonymous table HI16/LO16 pair follows at `+0x34`/`+0x3C`. The flag lattice
does not change this ownership surface. Canonical code and shared rodata
remain assembly pending a measured yaml handoff outside this lane's ownership.

The tier-B `frontSetMode` adds **0x64 bytes / 25 words** at ROM `0x399AC`.
Its exact free/init/reset call sequence, mode-state store, and ordered pairing
with `frontGetMode` establish the JFG role. The name, role, and shared control
flow carry point-of-use `PROVENANCE`; Mickey supplies the exact state surface.
The default flags, three calls plus five data-relocation pairs, object words, and
linked ROM range are exact without post-processing.

The tier-B `frontGetMode` adds **0xC bytes / 3 words** at ROM `0x39A10`.
Its exact byte getter, ordered position between the front-end mode setter and
update routine, and the update's dispatch on the same state establish the JFG
name. The adapted body carries point-of-use `PROVENANCE`; the default flags,
HI16/LO16 data relocations, object words, and linked ROM range are exact
without post-processing.

`func_80038E1C` retains a Mickey-derived `NON_MATCHING` candidate with the
exact **0x45C-byte / 279-word** size, `0x28`-byte frame, case count, and
high-level control flow. It plateaus at **248/279 differing words**, first
`+0x24`: IDO assigns the persistent fade-state address to `a0` rather than the
target's `v1`, then cascades into a different register and switch schedule.
The full 119-combination flag lattice keeps the resident defaults best; JFG
has no C donor body for the nearest front-end routine. Canonical code remains
assembly.

The tier-B `frontDemoMessage` adds **0x108 bytes / 66 words** at ROM
`0x39E78`. Its exact size, 16-tick blink gate, localized-language setup,
shadow/main text pair, and position immediately after `frontUpdate` establish
JFG's same-name role. JFG supplies the name and semantic comparison but keeps
its body in assembly; Mickey supplies the C body and message-record field, as
the point-of-use `PROVENANCE` note records. Spelling the byte timer as a direct
compound assignment recovers the target's `v0` address lifetime and temporary
register ring. The default flags, nine call relocations, three data-relocation
pairs, object words, and linked ROM range are exact without post-processing.

The tier-B `frontDrawRectangles` adds **0x204 bytes / 129 words** at ROM
`0x39F80`. Its ordered rectangle-batcher role, screen clipping, colour-change
batching, display-list state setup, and pairing with `frontDrawRectangle`
establish JFG's same-name role. JFG supplies the name and assembly-level
comparison; Mickey supplies the C body and exact render commands, as the
point-of-use `PROVENANCE` note records. Unsigned screen dimensions prevent
IDO from caching the clamped values, and their declaration order recovers the
target's `0x58`/`0x54` stack homes. The resident default flags, all 129 words,
three call relocations, the display-list data relocation pair, configured
object, linked ROM range, and full ROM are exact without post-processing.

The tier-A `frontDrawRectangle` adds **0x50 bytes / 20 words** at ROM
`0x3A184`. Its 19 unmasked words are byte-identical to JFG's uniquely
identified same-named skeleton; the only masked word is the call to the paired
rectangle-list renderer. The name/order carry point-of-use `PROVENANCE`, while
the body and 12-byte rectangle record are Mickey-derived. Full-width coordinate
parameters narrowed into that record recover the exact target schedule. The
default flags, call relocation, object words, and linked ROM range are exact
without post-processing.

The tier-B `frontPlayerScreenLimits` adds **0xB8 bytes / 46 words** at ROM
`0x3A1D4`. Its player-indexed screen-limit table, two-player split adjustment,
two coordinate-pair conversions, and front-end ordering establish the JFG
role. JFG supplies the name and semantic comparison but keeps its body in
assembly; Mickey supplies this C body and table indexing, as the point-of-use
`PROVENANCE` note records. Explicit shifts preserve `(cameraCount - 1)` against
algebraic reassociation. The default flags, three call relocations, data pair,
object words, and linked ROM range are exact without post-processing.

`func_8003968C` remains a **tier-D NON_MATCHING plateau** over **0x94 bytes /
37 words** at ROM `0x3A28C`. A Mickey-derived four-iteration initialization
loop unrolls to all 37 target instruction words and the same linked bytes, but
18 relocation sites differ, first at `+0x24`: IDO binds the later elements to
the three array-base symbols, while the target names each individual BSS
element. Scalar, volatile, cast, block, and loop variants, the full flag
lattice, and a bounded ten-minute `-mips2` permuter run did not satisfy both
the code and relocation-identity gates. The all-word result remains diagnostic
only and the original assembly is canonical. A later scalar-label retry
confirmed that the nine interior BSS labels already exist: naming them fixes
the relocation identities, but IDO then pools the repeated constants and
shrinks the body to 29 instructions. Switch, ternary-selection, scoped-local,
volatile-access, dead-reference, induction-expression, and comma-expression
variants plus a fresh 119-combination flag sweep did not preserve the target's
37-instruction temp sequence with those scalar relocations.

The tier-D `func_80039720` adds **0x320 bytes / 200 words** at ROM `0x3A320`.
Its Mickey-derived body updates four controllers' held, pressed, stick, and
repeat state, then aggregates the enabled controllers. Direct array indexing
recovers IDO's seven induction pointers and exact `0x40`-byte frame; JFG's
nearest menu routine remains assembly, so no donor body or descriptive name is
used. The resident defaults are exact in the full flag lattice. All 53 text
relocations match the target's offsets, kinds, and symbol identities, and the
configured object and linked ROM range are byte-identical without
post-processing.

The tier-B `freeFrontEndList` adds **0x5C bytes / 23 words** at ROM `0x3A640`.
Its exact sentinel-list loop calls the immediately following per-item routine,
matching JFG's ordered `freeFrontEndList`/`freeFrontEndItem` pair. The body is
adapted from DKR's public `menu_assetgroup_free` with point-of-use
`PROVENANCE`; JFG supplies the role and name. The default flags, call
relocation, object words, and linked ROM range are exact without
post-processing. A zero-byte weak alias preserves the anonymous name used by
the overlay caller.

The tier-B `freeFrontEndItem` adds **0xEC bytes / 59 words** at ROM
`0x3A69C`. Its four-way dispatch by the resource table's `0xC000` type bits,
loaded-slot clearing, live-resource decrement, and position immediately after
`freeFrontEndList` establish the JFG role. JFG supplies the name and semantic
comparison but keeps its body in assembly; Mickey supplies the C body, as the
point-of-use `PROVENANCE` note records. Treating the polymorphic resource table
as raw 32-bit handles until each typed release call preserves the target's
`v0` lifetime. The default flags, five call relocations, four data-relocation
pairs, object words, and linked ROM range are exact without post-processing.
A zero-byte weak alias preserves the anonymous name used by the list wrapper.

The parallel tier-B `loadFrontEndList` adds **0x5C bytes / 23 words** at ROM
`0x3A788`. Its exact sentinel-list loop calls the immediately following
per-item load routine, matching JFG's ordered `loadFrontEndList`/
`loadFrontEndItem` pair. The body is adapted from DKR's public
`menu_assetgroup_load` with point-of-use `PROVENANCE`; JFG supplies the role
and name. The default flags, call relocation, object words, and linked ROM
range are exact without post-processing.

The tier-B `loadFrontEndItem` adds **0x16C bytes / 91 words** at ROM
`0x3A7E4`. Its four-way dispatch by the resource table's high bits, texture,
sprite, object, and model call surface, loaded-slot bookkeeping, exact size,
and position after `loadFrontEndList` establish JFG's same-name role. The body
is adapted from DKR's public `src/menu.c::menu_asset_load`; Mickey supplies
the 13-byte spawn packet and the spawned-object fields at `0x40`, `0x22`,
`0x68`, and `0x08`, as the point-of-use `PROVENANCE` note records. A named
inner pointer recovers the target's pooled `v1`, while direct active-array
indexing recovers IDO's `0x1C` compiler spill. The resident default flags, all
91 words, four call relocations, seven data-relocation pairs, configured object,
and linked ROM range are exact without post-processing. A zero-byte weak alias
preserves the anonymous name used by the list wrapper.

The third tier-B sentinel wrapper, `setupFrontEndList`, adds **0x5C bytes /
23 words** at ROM `0x3A950`. Its call to the immediately following
`setupFrontEndObject` and JFG's same ordered pair establish the role. The body
is adapted from DKR's public `menu_imagegroup_load` with point-of-use
`PROVENANCE`; JFG supplies the name/order. The default flags, call relocation,
object words, and linked ROM range are exact without post-processing.

The tier-A `setupFrontEndObject` adds **0x88 bytes / 34 words** at ROM
`0x3A9AC`. Its complete masked instruction skeleton is identical to JFG's
same-name function, while the explicit typed record copy and signed trailing
bytes are derived from Mickey. The point-of-use `PROVENANCE` note records that
split. Computing the destination pointer before the source pointer recovers
the target's `v1`/`a1` allocation. The default `-O2 -mips2 -32` flags, both
data-relocation pairs, object words, and linked ROM range are exact without
post-processing.

`func_80039E34` retains a Mickey-derived `NON_MATCHING` draw candidate with
the target's exact **0xB8-byte frame** and local homes from `0x7C` through
`0xAC`. Its best object is one word longer than the target's **0x418 bytes /
262 words** and plateaus at **242/262 differing words**, first `+0x14`: IDO
assigns the persistent `D_800D31C8` base and selected object to `t2`/`a3`
instead of `t5`/`t0`, cascading through the command-building paths. Pointer,
volatile-access, and stack-layout variants did not recover the allocation;
the full flag lattice keeps the resident defaults best. JFG's corresponding
front-end draw routine also has no C donor body. Canonical code remains
assembly.

The tier-B `frontGetLanguage` adds **0x14 bytes / 5 words** at ROM
`0x3AE4C`. Its packed-field getter, position immediately before the paired
setter and screen-mode accessors, and the resident caller's use of its result
establish the JFG role. The JFG-derived name carries point-of-use
`PROVENANCE`, while the body is Mickey-derived. An unsigned expression feeding
the signed API recovers IDO's temporary return register; the default flags,
HI16/LO16 data relocations, object words, and linked ROM range are exact
without post-processing. A zero-byte weak alias preserves the anonymous name
used by resident assembly.

The paired tier-B `frontSetLanguage` adds **0x38 bytes / 14 words** at ROM
`0x3AE60`. Its byte-wide read/modify/write of the same six-bit field, ordered
position after `frontGetLanguage`, and call to the front-end language refresh
routine establish the JFG role. The name carries point-of-use `PROVENANCE`,
while the field layout and body are Mickey-derived. Default flags, the refresh
call and data relocations, object words, and linked ROM range are exact without
post-processing.

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

The tier-B `frontGetStereoMode` adds **0x14 bytes / 5 words** at ROM
`0x3B008`. Its two-bit packed getter and ordered position between the
wide-adjust and SFX-volume accessors establish the JFG role. The name carries
point-of-use `PROVENANCE`; Mickey's paired getter/setter encoding supplies the
bitfield and `u32` ABI. The canonical flags, HI16/LO16 data relocations, object
words, and linked ROM range are exact without post-processing.

The paired tier-B `frontSetStereoMode` adds **0x60 bytes / 24 words** at ROM
`0x3B01C`. JFG supplies the name, clamp, output-type table lookup, audio call,
and ordered position; Mickey's paired accessors supply the packed two-bit
storage. The adapted logic carries point-of-use `PROVENANCE`. The default
flags, two data pairs plus call relocation, object words, and linked ROM range
are exact without post-processing.

The tier-A-named `frontGetScreenMode` adds **0x30 bytes / 12 words** at ROM
`0x3AE98`. Mickey's draft established the two tests; JFG's published
`Resbitfield` declaration supplied the original source shape needed to recover
the compiler's temporary-register order. Mickey has two adjacent mode bits,
confirmed by the paired writes in the following setter. The adapted type has a
point-of-use `PROVENANCE` note, and the default flags, object words, and linked
ROM range are exact without post-processing.

`func_8003A2C8` remains a **tier-D NON_MATCHING plateau** over **0x80 bytes /
32 words** at ROM `0x3AEC8`. Mickey-derived packed-field writes correspond to
JFG's `frontSetScreenMode`, with the comparison recorded in a point-of-use
`PROVENANCE` note but no public name adopted. The best size-exact candidate
differs in 19 register operands, first at `+0xC`: the target retains the state
address and normalized mode in `a1`/`v0`/`v1`, while IDO assigns a different
temporary chain. The 119-combination flag lattice and a bounded ten-minute
permuter run did not reach exactness, so the original assembly remains
canonical. A later raw-byte/type-role retry kept the exact 32-instruction
opcode and relocation schedule in its best basin but did not improve the 19
register-operand residual; declaration-order and code-free dead-read probes
reached the same allocator boundary.

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

The tier-B `frontGet2PlayerSplit` adds **0x14 bytes / 5 words** at ROM
`0x3B10C`. Its extracted flag bit, its paired byte-preserving setter, and its
exact JFG menu position establish the role; the body itself was derived from
Mickey because JFG's remains `GLOBAL_ASM`. The extended bitfield declaration
carries point-of-use `PROVENANCE`. A local result recovers IDO's target `v1`
live range; the default flags, both data relocations, object words, and linked
ROM range are exact without post-processing.

`func_8003A520` plateaus size-exact at **3/9 differing words**, first `+0x8`.
Its operation and ordered position correspond to JFG's
`frontSet2PlayerSplit`, but the full 119-combination flag lattice and a bounded
permuter batch could not reproduce the target's old-flag register chain. The
best natural candidate uses a narrowing result cast; IDO still assigns that
chain two temporary registers earlier than the target. It remains assembly
and keeps its anonymous name. A later ten-variant byte-view/bitfield and
temp-FIFO retry preserved the exact nine-instruction structure but reached no
better than four register differences, so the earlier three-word candidate
remains the retained plateau. A native aggregate/bitfield and explicit-DAG
retry likewise preserved the operation but did not improve that result.

The tier-D `func_8003A544` adds **0xC bytes / 3 words** at ROM `0x3B144`.
Mickey's code is the single-word setter paired with the following getter; no
published donor body or descriptive-name evidence is used. The default flags,
HI16/LO16 data relocations, object words, and linked ROM range are exact
without post-processing.

The paired tier-D `func_8003A550` adds **0xC bytes / 3 words** at ROM
`0x3B150`. Mickey's code returns the same word written by `func_8003A544`;
no donor body or descriptive-name evidence is used. The default flags, both
data-relocation words, object words, and linked ROM range are exact without
post-processing.

The tier-D `func_8003A55C` adds **0x34 bytes / 13 words** at ROM `0x3B15C`.
Its Mickey-derived body stops the active tune, stores the caller's byte, and
sets the paired halfword timer to `0x78`. The adjacent JFG menu names do not
pin that Mickey-specific behavior, so the address label remains. The default
flags, call relocation, two data-relocation pairs, object words, and linked ROM
range are exact without post-processing.

The tier-D `func_8003A590` adds **0x10 bytes / 4 words** at ROM `0x3B190`.
Mickey's body writes `-1` to a resident halfword. A short-function skeleton
collision with an unrelated published routine is rejected as naming evidence,
so the address label remains. The default flags, HI16/LO16 relocation pair,
object words, and linked ROM range are exact without post-processing.

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
| `0x16A90` | `0x12C` | `shadowInitBuffers` | B; `NON_MATCHING` plateau after the flag lattice, eight source/loop forms, and a 10-minute permuter batch: exact 75-word frame/opcode/register shape, but 6 positional words differ and the two sentinel relocations at `+0xC4`/`+0xD4` bind `D_80079434 + 0xC` instead of `D_80079440` |
| `0x16BBC` | `0x78` | `shadowFreeBuffers` | B name; JFG-adapted exact C, 30 words, 15 relocs under O2/mips2 |
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
| `0x1BEA0` | `0x74` | `spranimInit` | D name; JFG-adapted exact C, 29 words, 0 relocs under O2/mips2 |
| `0x1BF14` | `0x4C` | `spranimControl` | D name; JFG-adapted exact C, 19 words, 1 call reloc under O2/mips2 |
| `0x1BF60` | `0x48` | `sprasjiInit` | D name; exact C, 18 words, 0 relocs under O2/mips2 |
| `0x1BFA8` | `0x78` | `spranimOnceControl` | D name; JFG-adapted exact C, 30 words, 2 call relocs under O2/mips2 |
| `0x1C020` | `0x304` | `effectboxControl` | D |
| `0x1C324` | `0x74` | `texscrollControl` | B name; JFG-adapted exact C, 29 words, 1 call reloc under O2/mips2 |
| `0x1C398` | `0x2BC` | `func_8001B798` | unresolved |
| `0x1C654` | `0x90` | `rangetriggerControl` | B; `NON_MATCHING` plateau after flag sweep and 10 stack-layout hypotheses: 34/36 words exact with both call relocs exact, first mismatch `+0x50`; IDO homes the entry pointer at `sp+0x44` instead of target `sp+0x40` |
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
| `0x3D370` | `0x9C` | `func_8003C770` | D; Mickey-only reconstruction, exact C, 39 words, 15 relocs |
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

Exact C reconstructions in this census currently include the still-unnamed
`func_8002BCC0` (ROM `0x2C8C0`–`0x2C8FC`, 60 bytes), `rumbleRumbles`
(ROM `0x2C8FC`–`0x2C908`, 12 bytes), `rumbleProcessing` (ROM `0x2C908`–
`0x2C958`, 80 bytes), `rumbleStart` (ROM `0x2C958`–`0x2CA74`, 284 bytes),
`rumbleStop` (ROM `0x2CA74`–`0x2CB00`, 140 bytes),
`rumbleKill` (ROM `0x2CB00`–`0x2CB44`, 68 bytes), `rumbleUpdate` (ROM
`0x2CB44`–`0x2CB54`, 16 bytes), the still-unnamed rumble reinitializer
`func_8002BF54` (ROM `0x2CB54`–`0x2CC98`, 324 bytes),
`packCalculateGameChecksum` (ROM `0x2D3BC`–
`0x2D3EC`, 48 bytes), `packCalculateGlobalFlagsChecksum` (ROM `0x2DA2C`–
`0x2DA54`, 40 bytes), `packClose` (ROM `0x2DED4`–`0x2DF00`, 44 bytes),
`packOpen` (ROM `0x2DCCC`–`0x2DED4`, 520 bytes),
the still-unnamed chunked save-device transfer `func_8002C7EC` (ROM
`0x2D3EC`–`0x2D4B4`, 200 bytes),
`packIsPresent` (ROM `0x2E0CC`–`0x2E128`, 92 bytes),
`packDirectory` (ROM `0x2E128`–`0x2E424`, 764 bytes),
`packDirectoryFree` (ROM `0x2E424`–`0x2E458`, 52 bytes),
`packFreeSpace` (ROM `0x2E458`–`0x2E56C`, 276 bytes),
`packDeleteFile` (ROM `0x2E56C`–`0x2E620`, 180 bytes),
`packOpenFile` (ROM `0x2E620`–`0x2E74C`, 300 bytes),
`packReadFile` (ROM `0x2E74C`–`0x2E810`, 196 bytes),
`packWriteFile` (ROM `0x2E810`–`0x2EA50`, 576 bytes),
`packFileSize` (ROM `0x2EA50`–`0x2EAB4`, 100 bytes),
the still-unnamed `func_8002C5F4` (ROM `0x2D1F4`–`0x2D20C`, 24 bytes),
the still-unnamed bitstream allocator `func_8002C60C` (ROM `0x2D20C`–
`0x2D29C`, 144 bytes),
the still-unnamed `func_8002C788` (ROM `0x2D388`–`0x2D390`, 8 bytes),
the still-unnamed `func_8002C790` (ROM `0x2D390`–`0x2D39C`, 12 bytes),
the still-unnamed `func_8002C79C` (ROM `0x2D39C`–`0x2D3BC`, 32 bytes),
the still-unnamed `func_8002C8B4` (ROM `0x2D4B4`–`0x2D54C`, 152 bytes),
the still-unnamed `func_8002CCE4` (ROM `0x2D8E4`–`0x2D96C`, 136 bytes),
the still-unnamed game-state writer `func_8002CD6C` (ROM `0x2D96C`–
`0x2DA2C`, 192 bytes),
the still-unnamed global-flags loader `func_8002CE54` (ROM `0x2DA54`–
`0x2DB0C`, 184 bytes),
the still-unnamed `func_8002CF0C` (ROM `0x2DB0C`–`0x2DB6C`, 96 bytes),
the still-unnamed `func_8002E020` (ROM `0x2EC20`–`0x2ECA0`, 128 bytes),
`piInit` (ROM `0x2ECA0`–`0x2ED48`, 168 bytes),
`piRomLoad` (ROM `0x2ED48`–`0x2EDE4`, 156 bytes),
`piRomLoadCompressed` (ROM `0x2EDE4`–`0x2EEE0`, 252 bytes),
`piRomLoadSection` (ROM `0x2EEE0`–`0x2EF5C`, 124 bytes),
`piRomGetSectionPtr` (ROM `0x2EF5C`–`0x2EFA4`, 72 bytes),
`piRomGetFileSize` (ROM `0x2EFA4`–`0x2EFE0`, 60 bytes),
`romCopy` (ROM `0x2EFE0`–`0x2F0D0`, 240 bytes),
`screenLoad` (ROM `0x2F0D0`–`0x2F1D4`, 260 bytes),
`screenDraw` (ROM `0x2F1D4`–`0x2F3FC`, 552 bytes),
`rcpWaitDP` (ROM `0x2F6A0`–`0x2F76C`, 204 bytes),
`rcpSetScreenColour` (ROM `0x2F76C`–`0x2F794`, 40 bytes),
`bgdraw_fillcolour` (ROM `0x2F794`–`0x2F7D4`, 64 bytes), and the still-
unnamed global setter `func_8002EBD4` (ROM `0x2F7D4`–`0x2F7E0`, 12 bytes),
plus `rcpClearScreen` (ROM `0x2FD88`–`0x30068`, 736 bytes),
`rcpInitDp` (ROM `0x30068`–`0x30118`, 176 bytes),
`rcpInitDpNoSize` (ROM `0x30118`–`0x3013C`, 36 bytes), and
`rcpInitSp` (ROM `0x3013C`–`0x30160`, 36 bytes), and
`rcpInit` (ROM `0x30160`–`0x30218`, 184 bytes), and
`osCreateScheduler` (ROM `0x30CD0`–`0x30E2C`, 348 bytes),
`osScGetAudioSPStats` (ROM `0x30F20`–
`0x30F38`, 24 bytes), `osScGetCmdQ` (ROM `0x30F10`–`0x30F18`, 8 bytes),
`osScGetInterruptQ` (ROM `0x30F18`–`0x30F20`, 8 bytes), `__scMain` (ROM
`0x30F38`–`0x310E0`, 424 bytes), and the still-unnamed
no-op `func_80030608` (ROM `0x31208`–`0x31210`, 8 bytes), plus
the still-unnamed scheduler helper `func_800304E0` (ROM `0x310E0`–
`0x31180`, 160 bytes),
`osScAddClient` (ROM `0x30E2C`–`0x30E88`, 92 bytes), `osScRemoveClient`
(ROM `0x30E88`–`0x30F10`, 136 bytes), `__scHandleRSP` (ROM `0x31D4C`–
`0x31E74`, 296 bytes), `__scHandleRDP` (ROM `0x31E74`–`0x31EFC`, 136
bytes), `__scTaskReady` (ROM `0x31EFC`–`0x31F4C`, 80 bytes),
`__scTaskComplete` (ROM `0x31F4C`–`0x3204C`, 256 bytes),
`__scAppendList` (ROM `0x3204C`–`0x320AC`, 96 bytes), and `__scExec` (ROM
`0x320AC`–`0x3216C`, 192 bytes). All were compiled
with the resident `-O2 -mips2 -32` flags. The saves TU additionally disables
loop unrolling: the full flag
lattice selects the target's scalar 24-record reset loop, and the full ROM
comparison confirms the setting leaves its other exact functions unchanged.
The named bodies are adapted from
JFG's `src/saves.c`, `src/pi.c`, `src/rcpFast3d.c`, and `src/sched.c`; the
Mickey `rumbleStart` body specifically corresponds to JFG's `rumbleMax`; the
still-unnamed leading rumble gate also adapts its JFG body while retaining
Mickey's placeholder name, and the still-unnamed global-flags loader adapts
JFG's load-and-default-copy flow to Mickey's 24-byte resident record and I/O
helpers. `bgdraw_fillcolour` adapts Diddy Kong Racing's
public `src/rcp_dkr.c` body. The anonymous setter, dual-global reset,
record-field accessors, allocation wrapper, and no-op are reconstructed from
Mickey's own bodies. `screenLoad` is likewise reconstructed from Mickey's
display-list command writes; JFG supplies its existing TU/name association,
not its C body. `rcpInitDp` is likewise reconstructed from Mickey's own
display-list command flow; JFG supplies its name and ordered TU position, not
its C body. `rcpClearScreen` adapts DKR's public `bgdraw_render` display-list
macro spelling to Mickey's guards, helpers, and coordinates; JFG supplies its
name and ordered TU position while retaining assembly. `rcpInit` reconstructs
Mickey's six message queues while JFG's public source supplies its name and
prototype and its object supplies the exact skeleton anchor, not a C body. All
configured object ranges and the final linked ROM are byte-exact.

`rcpFast3d` retains a `NON_MATCHING` Mickey-derived task-construction body
after the 119-combination flag lattice and ten source/type/scheduling
hypotheses. A candidate using the SDK's distinct `rspbootTextEnd` identity is
exact across all 168 instruction words, but strict object comparison rejects
the HI/LO pair at function `+0x204`: the target relocations name
`D_80077AD0`, while the candidate names the weak alias. Using
`D_80077AD0` for both the boot-end and Fast3D-start roles gives exact
relocation identities but lets IDO coalesce the address, emitting 166
instructions and first diverging at the same offset. The exact-word candidate
remains guarded for the distinct-symbol scheduling evidence; the assembly
fallback remains canonical.

`rcpClearZBuffer` retains a `NON_MATCHING` DKR-shaped command-stream body after
the 119-combination flag lattice and nine source-shape hypotheses. The best
no-frame candidate has 96 instructions versus the target's 107 and first
diverges at function `+0x10`, where the shortened body changes the first guard's
branch span. IDO folds the candidate's derived command pointers into fixed
base-relative stores; formulations that preserve the target's successive
cursor advances instead introduce a 24- or 32-byte spill frame absent from the
target. The assembly fallback remains canonical pending a source spelling that
homes the three register arguments without spilling the cursor.

`func_8002EBE0` retains a Mickey-derived `NON_MATCHING` eight-band gradient
renderer after the 119-combination flag lattice and ten source-shape
hypotheses. The nearest skeleton is Diddy Kong Racing's `bgdraw_render` at only
0.055 similarity; JFG has no function in the corresponding ordered gap. The
best faithful MIPS II candidate has 249 instructions versus the target's 255
and a 0x50-byte frame versus 0x88, with 242 positional word mismatches and the
first at function `+0x0`. SDK scissor, fill-colour, fill-rectangle, and pipe
macros close the body-size gap, but IDO retains 56 fewer bytes of non-save stack
and assigns the display-list cursor and colour-step webs differently from the
prologue onward. A bounded permuter import selected MIPS I and was rejected as
non-canonical; its pack-expression lead also failed when recompiled with the
resident MIPS II flags. The assembly fallback remains canonical.

`__scHandleRetrace` has a preserved `NON_MATCHING` JFG-derived body after the
119-combination flag lattice and ten source-shape hypotheses. The best
candidate has the exact 232-byte frame and 408 instructions versus the
target's 409; 84 positional words differ after relocation masking. Its first
mismatch is at function `+0x3B4`, where the diagnostic Y-coordinate and two
stack-byte writes schedule in a different order. The remaining tail also
materialises the 64-bit retrace counter through one combined object while the
target uses separate high/low symbol references. The assembly fallback
remains canonical.

The still-unnamed scheduler diagnostic `func_80030610` retains a Mickey-
derived `NON_MATCHING` display-list bisection body after the 119-combination
flag lattice, nine serious source/layout hypotheses, and a bounded two-worker
permuter batch. The best candidate has 194 instructions versus the target's
192 and first diverges at function `+0x5C`; 148 positional words differ. Its
0x90-byte frame is eight bytes shorter than the target's 0x98-byte frame: IDO
homes the received message at stack `+0x78` instead of `+0x70` and the saved
second-command pointer at `+0x48` instead of `+0x4C`. Scalar, `s64` backup,
macro, and explicit-pointer formulations either retain those homes or add
more instructions. JFG supplies the exact assembly skeleton and scheduler
position but no C body. The assembly fallback remains canonical pending a
source spelling that reproduces both stack homes without synthetic padding.

`__scSchedule` retains a JFG-derived `NON_MATCHING` body whose 122 instruction
words and 0x28-byte frame are exact under the resident flags and the full
119-combination lattice. Perfect Dark's compiled scheduler is the closest
independent skeleton at 0.857 similarity, while Mickey's two independent RCP
state checks select JFG's source spelling. Promotion is blocked by rodata
ownership rather than C: both switch relocations bind the compiler's anonymous
late-rodata section, while Mickey's existing seven-entry `jtbl_800823F4`
remains in the shared `0x81590` rodata object and names assembly-local case
labels. Removing the fallback therefore leaves those seven entries undefined.
Moving the table requires a measured YAML/shared-rodata boundary handoff
outside this lane's assigned files, so the assembly fallback remains canonical.

`__scYield` also retains a `NON_MATCHING` JFG-derived body. The resident flag
lattice and five storage/source shapes leave the faithful external-`u64`
candidate at 20 instructions versus the target's 19, with the first positional
mismatch at function `+0x14` after the extra base reload changes the branch
span. Defining the timestamp in this TU is diagnostic only: it reproduces all
19 instruction words, but its final low-half store relocates against
`D_800D2D48` plus four rather than `D_800D2D4C` at `+0x38`, and it introduces
new scheduler BSS that this split does not own. The assembly fallback remains
canonical until BSS ownership can be reconstructed without changing the
shared data layout.

A fresh structural retry kept the external timestamp split without claiming
new BSS ownership. Declaring the two extracted high/low words separately made
IDO emit 28 instructions, with all 19 target positions differing from `+0x0`;
writing through one adjacent `u64` lvalue returned to the existing 20-word,
eight-difference basin at `+0x14`. The repeated full flag lattice therefore
leaves the external-`u64` candidate as the best faithful form and confirms the
remaining blocker is the combined return-value store versus distinct BSS
relocation identities.

`osScGetTaskType` retains its JFG-derived switch under `NON_MATCHING`. The
canonical candidate emits all 34 target text instructions and all eight
string relocation identities exactly, but its two jump-table relocations at
function `+0x14` and `+0x1C` name the compiler's anonymous 0x20-byte
`.rodata` section rather than Mickey's existing `jtbl_800823D8`. The flag
lattice cannot change section ownership, and promoting the table requires a
measured `mickey.us.yaml` rodata-boundary handoff outside this lane's assigned
files. The assembly fallback therefore remains canonical.

The still-unnamed bit writer `func_8002C69C` retains a Mickey-derived
`NON_MATCHING` body after the 119-combination flag lattice and seven
source/type/loop shapes. Its best faithful candidate is 29 instructions versus
the target's 28; the first mismatch is at function `+0x0` because the longer
branch span is followed by an extra value-preserving move at `+0x4`. That move
changes the loop's register allocation even though the control flow and bit
stream operations agree. The lane's `tools/permute.sh` could not run because
the vendored `tools/permuter/import.py` is absent. The assembly fallback
remains canonical.

A fresh structural retry removed the explicit bit-test temporary and tested
the source value directly. That form regressed to 30 instructions versus 28,
with 30 positional differences from `+0x0`; changing only the temporary's
signedness stayed in the existing 29-instruction, first-`+0x0` basin. The
original candidate remains the best faithful form.

Its paired bit reader `func_8002C70C` likewise retains a Mickey-derived
`NON_MATCHING` body. The 119-combination flag lattice and five local-width and
expression shapes reach the target's exact 31-instruction control-flow shape,
but 18 words differ from the first mismatch at `+0x14`: the loop-invariant
`0x80` mask and subsequent temporaries allocate to different registers, with
one remaining opcode difference and no relocation differences. Automated
declaration permutation is blocked by the same absent permuter import. The
assembly fallback remains canonical.

A fresh local-width retry used byte-sized mask temporaries, matching the
values' stored width rather than their promoted arithmetic width. It regressed
to 32 instructions, 24 positional differences, and first mismatch `+0x0`.
The promoted-width candidate remains best at the target's 31 instructions,
18 differences, and first mismatch `+0x14`.

The full-save-image builder `func_8002CF6C` also retains a Mickey-derived
`NON_MATCHING` body after the 119-combination flag lattice and ten
stack-layout, call-arity, and lifetime hypotheses. Its best candidate has the
target's exact 88-instruction opcode schedule, 72-byte frame, and relocation
surface; nine register-only words remain from the first mismatch at `+0xCC`.
The saved global-flags bit reload crosses from the target's temporary FIFO to
the candidate's colored fourth-argument web, rotating the following integer
temporaries through the second copy loop. The assembly fallback remains
canonical.

The 0x94-byte save-window serializer `func_8002C94C` retains a Mickey-derived
`NON_MATCHING` body after the 119-combination flag lattice and ten source,
stack-layout, and lifetime hypotheses. Its best candidate has the target's
exact 115-instruction opcode schedule, 112-byte frame, and relocation surface;
12 words differ, first at function `+0x30`. Six differences place the message
queue and two-word footer at candidate `sp+0x6C` and `sp+0x64` instead of the
target's `sp+0x54` and `sp+0x40`; the other six swap the save-slot base and
outer counter between `$s5` and `$s6`. Mirroring the recovered local order
instead drops the footer-magic stores, and aggregate layouts grow the frame.
The unavailable permuter import prevents a bounded automated declaration
search; the assembly fallback remains canonical.

The paired save-window loader `func_8002CB18` retains a Mickey-derived
`NON_MATCHING` body after the 119-combination flag lattice and ten local-order,
scope, and aggregate-layout hypotheses. Its best candidate has the exact 115
instructions, 112-byte frame, and relocations; ten words differ, first at
`+0x30`. Nine are stack homes: the footer is exact, but the queue lands at
candidate `sp+0x64` versus target `sp+0x5C`, and the decoded value at `sp+0x5C`
versus `sp+0x60`. The last reuses `$s5` where the target rematerialises four.
Aggregates grow the frame and narrower scopes move both homes together. With
the permuter import unavailable, the assembly fallback remains canonical.

`packInit` retains a `NON_MATCHING` body adapted from Diddy Kong Racing's
public `src/save_data.c:init_controller_paks` after the 119-combination flag
lattice and ten loop, local-order, pointer, and controller-limit hypotheses.
Its best candidate has the exact 115 instructions, 96-byte frame, reset loop,
calls, and relocations; 34 words differ, first at `+0xA0`. IDO delays the PFS
base's low half into the loop, rotating address temporaries and serialising the
rumble-success stores. Explicit pointers revert to a multiply, grow the frame,
and score worse. The assembly fallback remains canonical.

The scheduler display-list trace helper `func_80030910` retains a
`NON_MATCHING` body adapted from Jet Force Gemini's public
`src/sched.c:func_8004FF64_50B64` after the 119-combination flag lattice and
ten declaration-order, aggregate-layout, pointer, and address-expression
hypotheses. Its best candidate has the exact 152-byte frame, local addresses,
control flow, calls, and relocations, but 117 instructions versus 118; 20 words
differ, first at `+0x11C`. IDO keeps `0x80000000` live once in `$v1`, while the
target rematerialises it twice through `$at`; the missing word changes branch
spans. Other signed, unsigned, additive, and bitwise spellings retain that CSE
or emit OR. The assembly fallback remains canonical.

`font_codes_to_string` retains a JFG-derived `NON_MATCHING` body after the
flag lattice and ten source/type/coalescing shapes. With ordinary resident
flags the donor loop is instruction-exact, but this TU's required
`-Wo,-loopunroll,0` removes the target's four-byte padding expansion. Spelling
that expansion explicitly restores the exact 44-instruction opcode and
relocation shape, leaving five register-only words from one `$a0`/`$v0` web
swap, first at function `+0x64`. The unavailable permuter import prevents a
bounded automated declaration search; the assembly fallback remains
canonical.

`string_to_font_codes` retains its paired JFG-derived `NON_MATCHING` body
after the flag lattice and ten source, CFG, and type shapes. The donor loop is
instruction-exact with ordinary resident flags, while this TU's required
`-Wo,-loopunroll,0` removes the target's twelve-instruction four-byte padding
expansion. Spelling the peel and four stores explicitly restores the exact
47-instruction opcode and relocation shape, leaving five register-only words
from one `$a0`/`$v0` web swap, first at function `+0x70`. The unavailable
permuter import prevents a bounded automated declaration search; the assembly
fallback remains canonical.

### 3.16 Particle and debug-print translation units

ROM `0x3D5F0`–`0x45760` contains two aligned resident C subsegments.
`symbol_addrs.us.txt` records every function's exact size and evidence tier.
Unresolved functions remain `GLOBAL_ASM`, so the split claims no matched bytes.

| Mickey TU | ROM / VRAM | Functions | Evidence |
|---|---|---:|---|
| `main/particles` | `0x3D5F0`–`0x43470` / `0x8003C9F0` | 44 | **A:** DKR's built `particles.c.o` identifies `reset_particles` byte-for-byte. **B:** the internal call graph and external particle callers. **D:** the full function order and masked-skeleton sequence track JFG's 42-function `particles.c.o` from `partFreeLib` through `partNullifyCircularParticleParents`; Mickey inserts two extra 12-byte state setters before `partUpdateTriggers`, after which the sequences reconverge. |
| `main/diprint` | `0x43470`–`0x45760` / `0x80042870` | 19 | **A:** DKR objects identify `strcpy`, `memset`, and `sprintf` exactly. **B:** `diPrintf` brackets `vsprintf` with `sprintfSetSpacingCodes`, `diPrintfAll` drives the parse/background/character/bounds/origin helpers, and later `diRcp*` routines call `sprintf`. **C:** `_itoa` owns both digit alphabets and `vsprintf` owns `(null)` and `(nil)`. The order matches JFG's `diprint.c.o`, with DKR's `debug_text_width` inserted between `diPrintfSetXY` and `debug_text_parse`. |

**PROVENANCE.** Names/TU attribution use JFG's public `src/particles.c`,
`src/diprint.c`, and objects; DKR's `src/printf.c` supplies
`debug_text_width`, and its `unused_string.c.o`, `printf.c.o`, and
`particles.c.o` supply the stated tier-A rows. Donor placeholders stay
excluded; Mickey's bytes/call graph decide disagreements.

The table tiers are TU-level. Per symbol, donor-object matches are tier A;
named call-graph functions are tier B; `_itoa`/`vsprintf` string evidence is
tier C; remaining JFG/DKR order/skeleton attributions are tier D. Each of the
63 `symbol_addrs.us.txt` rows carries its tier token.

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
default resident flags, Mickey-only reconstruction); `func_8003E730` (ROM
`0x3F330`, `0x88` bytes, default resident flags, Mickey-only reconstruction);
`func_80041C50` (ROM `0x42850`, `0x94` bytes, default resident flags,
Mickey-only reconstruction);
`func_80041F48` (ROM `0x42B48`, `0xA4` bytes, default resident flags,
Mickey-only reconstruction);
`func_8004233C` (ROM `0x42F3C`, `0xB0` bytes, default resident flags, DKR
`move_particle_basic` body donor);
`partInitTriggerPos` (ROM `0x3F270`, `0xC0` bytes, default resident flags,
Mickey reconstruction with the JFG assembly sibling as a structural oracle);
`func_8003CD28` (ROM `0x3D928`, `0xE8` bytes, default resident flags,
Mickey reconstruction with the JFG assembly sibling as a structural oracle);
`func_80041FEC` (ROM `0x42BEC`, `0xF4` bytes, default resident flags, DKR
`move_particle_basic_parent` body donor);
`func_800423EC` (ROM `0x42FEC`, `0x108` bytes, default resident flags, DKR
`move_particle_forward` body donor);
`func_800420E0` (ROM `0x42CE0`, `0x114` bytes, default resident flags, DKR
`move_particle_attached_to_parent` body donor);
`func_8003CA20` (ROM `0x3D620`, `0x11C` bytes, default resident flags,
Mickey reconstruction with the JFG assembly sibling as a structural oracle);
`func_8003E7B8` (ROM `0x3F3B8`, `0x120` bytes, default resident flags,
Mickey reconstruction with the JFG assembly sibling as a structural oracle);
`func_8003CB3C` (ROM `0x3D73C`, `0x1A8` bytes, default resident flags,
Mickey reconstruction with the JFG assembly sibling as a structural oracle);
`func_80041388` (ROM `0x41F88`, `0x1A8` bytes, default resident flags,
Mickey reconstruction with the JFG assembly sibling as a structural oracle);
`func_8003EF80` (ROM `0x3FB80`, `0x1D4` bytes, default resident flags,
Mickey reconstruction with the JFG assembly sibling as a structural oracle);
`func_8003EB08` (ROM `0x3F708`, `0x184` bytes, default resident flags,
Mickey reconstruction with the JFG assembly sibling as a structural oracle);
`func_800421F4` (ROM `0x42DF4`, `0x148` bytes, default resident flags, DKR
`move_particle_with_acceleration` body donor);
`partDraw` (ROM `0x43264`, `0x160` bytes, default resident flags, Mickey
reconstruction with the JFG assembly sibling as a structural oracle);
`partUpdateParticles` (ROM `0x430F4`, `0x170` B, default flags, Mickey/JFG
assembly reconstruction); `func_80040878` (ROM `0x41478`, `0x310` B,
`-O2 -mips2 -32`, DKR body/JFG assembly oracle); `func_80041040` (ROM
`0x41C40`, `0x348` B, default flags, Mickey body/JFG assembly oracle);
`partInitTriggerSPPos`
(ROM `0x3F224`, `0x4C` bytes, default resident flags, JFG-named Mickey
reconstruction); `partInitTrigger` (ROM `0x3F1AC`, `0x78` bytes, default
resident flags, JFG-named Mickey reconstruction); `debug_text_background`
(ROM `0x452F8`, `0xA0` bytes, resident flags plus `-Wab,-r4300_mul`, JFG body
donor).

`func_8003CE10` plateaued after the flag lattice, ten hypotheses, and a
canonical-`mips2` permuter: its 275-instruction C has exact opcodes/relocations,
but 154 words differ from `+0x0` because IDO emits a `0x98` frame versus
`0x90`. JFG `func_8005DD88` is the assembly oracle; asm stays canonical.

`func_8003F5F8` plateaued one instruction short after the flag lattice, ten
hypotheses, and a canonical-`mips2` permuter. Its `0x48` frame and all register
lanes match; 262/276 aligned rows match. The first mismatch is the target's
redundant branch at `+0xA4`; 11 later words swap the flags spill (`sp+0x44`)
with the rotation pair (`sp+0x30`). JFG `func_800608EC` is the assembly oracle;
asm stays canonical.

`func_8003F154` plateaued frame-exact at 294 instructions versus 297 after the
flag lattice, ten structural hypotheses, and a canonical-`mips2` permuter.
252 aligned rows match; the first raw mismatch is the end-branch displacement
at `+0x204`, followed at `+0x20C` by the zero-vector `f0`/`f6` choice. The
remaining cluster is header-copy/branch and FP normalization scheduling. JFG
`func_80060400` is the assembly oracle; asm stays canonical.

`func_8003EC8C` plateaued size-exact at 47 words, with 24 residuals from
`+0x30`: the target hoists `D_8007C894`'s HI16 load before the count decrement.
A branch-local pointer fixes that schedule but disrupts both branches' register
allocation. The flag lattice and bounded permuter found no compliant exact
form; asm stays canonical.

`func_8004054C` plateaued one word short (124/125), with 43 aligned residuals
from `+0x4C`: IDO folds the free-bit scan into a pointer move, then colors the
scan/index scratch registers differently. The flag lattice and bounded
permuter (score 1065 to 705) found no exact form; asm stays canonical.

`func_8003E8D8` plateaued size-exact at 140 words with the target opcode
schedule, but the whole TU emits a `0x30` frame versus `0x38`: 22 residuals
(19 stack/frame, two registers, one branch) begin at `+0x8`. The flag lattice
did not improve it; a standalone permuter zero failed in the canonical TU and
was rejected. Asm stays canonical.

`func_80040740` has a 78-word C candidate with the target text instruction
schedule under the default resident flags, reconstructed from Mickey evidence
with the JFG assembly sibling as a structural oracle. It cannot be promoted
within this TU's ownership: the first relocation mismatch is at function
offset `0x38`, where IDO binds the generated switch to anonymous compiler
rodata rather than the separately extracted `jtbl_80082A58`. That external
table still owns five case-label references, so replacing the asm body also
leaves those labels undefined and emits a duplicate 20-byte table. The flag
lattice found no alternative relocation binding. The candidate remains under
`NON_MATCHING` and the original asm body remains canonical pending coordinated
resident-rodata ownership.

`partModelObjEmitModelPart` reached a bounded size-exact 84-word plateau under
the default resident flags. The best compliant Mickey reconstruction differs
in 19 words, first at function offset `0x58`, where the candidate performs the
final trigger-stride shift before the descriptor-table add/load while the
target performs it afterward. The residual continues through the trigger
initialization stores; the call and complete post-call FP control flow are
exact. The flag lattice found no exact alternative. A bounded permuter import
selected `-mips1` instead of this TU's configured `-mips2`, so its output was
inadmissible and discarded. The candidate remains under `NON_MATCHING` and the
original asm body remains canonical.

`partUpdateTriggers` reached a bounded instruction-count and opcode-exact
101-word plateau under the default resident flags. Six register-only words
remain, first at function offset `0xE4`: the target colors the late trigger
array base/current pointer pair as `$v1`/`$v0`, while IDO colors the candidate
as `$a2`/`$v1`; the final `bne` also emits the two commutative operands in the
opposite order. Pointer and integer base types plus declaration/lifetime
variants all canonicalized to this basin, and the full flag lattice found no
alternative. The bounded permuter imported the TU as `-mips1`; its only lead
made the signed trigger count unsigned and was rejected. The candidate remains
under `NON_MATCHING` and the original asm body remains canonical.

`func_80041CE4` reached a bounded 153-instruction plateau under the default
resident flags with the exact opcode schedule, `0x80` frame, and relocation
surface. Its best Mickey-derived candidate differs in 34 words, first at
function offset `0x48`: the target colors the outer entry-count web in `$a3`
rather than `$a2` and places the address-taken display-list local at
`sp+0x6C` rather than `sp+0x7C`; the same pool-register rotation continues
through the two generated command words. Declaration, lifetime, pointer-loop,
and expression-tree variants converged on that allocation basin after the full
flag lattice. The bounded permuter imported the TU as inadmissible `-mips1`
and only improved its internal score with a dummy label, so the typed candidate
remains under `NON_MATCHING` and the original asm body remains canonical.

`func_8003D25C` reached a bounded 168-instruction plateau under the default
resident flags with the exact opcode schedule, `0xB8` frame, and relocation
surface. Its best typed Mickey reconstruction differs in 70 register-only
words, first at function offset `0x50`: the target begins the command-temporary
ring with `$t0` while the candidate begins with `$t1`, and the color-component
webs occupy different pool positions. The full 119-entry flag lattice found no
exact alternative. Expression ordering, explicit and macro command forms,
component lifetimes, and nested scopes converged either on this allocation
basin or on structurally worse schedules. The candidate remains under
`NON_MATCHING` and the original asm body remains canonical.

`vsprintf` reached a bounded size-exact plateau under `-Wab,-r4300_mul`: its
1,220-word candidate differs in two adjacent words, first at function offset
`0xB08`, where IDO loads the final exponent digit constants in the reverse
order. The flag lattice found no exact alternative, and the bounded permuter
could not parse the formatter's `va_arg` macros. Promotion is additionally
blocked because the C body emits formatter jump tables and static strings
still owned by the resident asm-data split.

`diPrintfAll` reached an instruction-exact 144-word plateau under
`-Wab,-r4300_mul`. Strict object comparison finds four relocation-identity
differences, first at function offset `0x144`: both accesses to the saved text
Y coordinate name the separately declared `D_800D4A62`, while the target names
`D_800D4A60+2`. Array, pointer, volatile, integer-cast, and structure spellings
that produce the target relocation identity change IDO's address commoning,
register allocation, and schedule to 142--145 words. The full flag lattice
found the exact instruction stream only with separate globals, so the JFG
candidate remains under `NON_MATCHING` and the original asm body remains
canonical.

`debug_text_parse` reached an instruction-exact 263-word plateau. Strict
object comparison still finds four relocation-identity differences: two
accesses use the separately named `D_800D4A62` instead of `D_800D4A60+2`, and
two switch-table references use compiler `.rodata` instead of
`jtbl_80082CD8`. The generated switch table would also duplicate the resident
asm-data owner, so the original asm body remains canonical.

`debug_text_character` reached a bounded 186-instruction plateau under
`-Wab,-r4300_mul` with the exact opcode schedule, register assignment, and
relocation surface. The JFG-derived Mickey candidate differs in six stack
operands, first at function offset `0x10`: IDO gives the candidate a `0x10`
frame with the selected texture width at `sp+0`, while the target has a `0x18`
frame and places that width at `sp+8`; the selected texture address is already
at the target's `sp+4`. The full 119-entry flag lattice found no alternative,
and scalar, aggregate, reserved-field, and physical-address source layouts did
not reproduce the empty target slot without worsening the code. The bounded
permuter importer selected inadmissible `-mips1` and then failed on this TU's
expanded `va_arg` syntax. The candidate remains under `NON_MATCHING` and the
original asm body remains canonical.

`func_80040B88` reached a bounded 300-instruction plateau against the target's
302 instructions under the canonical resident flags. Its `0x70` frame differs
from the target's `0x68` frame at function offset zero; shift-tolerant alignment
needs seven insertions and nine deletions, with 141 paired residuals. The first
substantive divergence preserves the trigger pointer in `$t7` where the target
spills and reloads it, rotating the later temporary lanes. The full flag
lattice and bounded permuter found no exact form. The DKR
`update_line_particle` body and JFG assembly sibling are the structural oracles;
the candidate remains under `NON_MATCHING` and asm remains canonical.

`func_80041530` reached a bounded 386-instruction plateau against the target's
456 instructions. The candidate has a `0x160` frame instead of the target's
`0x168`, first differing at function offset zero; alignment needs ten
insertions, 21 deletions, and 64 replacements. The unresolved structural gap is
the target's software-pipelined input-vector construction. The full flag
lattice found no exact alternative, and the bounded permuter imported this
resident TU as inadmissible `-mips1`. JFG `func_80062BFC` is the assembly
oracle; the typed candidate remains under `NON_MATCHING` and asm remains
canonical.

`func_8003FB98` reached a bounded near-exact plateau at the target's full 621
instructions, `0x38` frame, and relocation surface. Ten words remain, first at
function offset `0x10C`: six stack operands place the reused scale value at
`sp+0x20` rather than the target's `sp+0x24`, while two adjacent FP instruction
pairs schedule the Z square before the X/Y partial sum. Ten coherent source,
type, and expression-shape attempts plus the full flag lattice found no exact
form. The bounded permuter used inadmissible `-mips1` and only moved its internal
score from 19985 to 19900. The body is adapted from DKR
`create_general_particle` and cross-checked against JFG `func_80060ED4`; it
remains under `NON_MATCHING` and asm remains canonical.

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

`func_80034094` has an instruction-exact 47-word `NON_MATCHING` switch body
adapted from JFG's `viGetOsViMode`; a full 119-configuration flag sweep again
found the resident flags exact. Strict object comparison, however, finds two
relocation-identity differences at function offsets `+0x10` and `+0x18`: the
target names `jtbl_8008249C`, while IDO names the candidate's anonymous
`.rodata` table. It cannot be promoted within this TU's ownership because the
separately extracted `jtbl_8008249C` still owns the 12 case-label references;
replacing the asm body therefore leaves those labels undefined and also emits
a duplicate 48-byte table. The canonical path remains the original asm pending
coordinated rodata ownership.
Retyping the mode argument and its `ResolutionSettings` field to JFG's
`VideoModes` enum produced the same 47-word object and the same anonymous-table
relocations. Strict comparison still reports only the two symbol-identity
differences at `+0x10` and `+0x18`; the donor enum type cannot perform the
required rodata ownership handoff.
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
| `0x80050348` | `0x214` | `animseqInitPath` | B; exact `animseqInitGroup` calls this function. Plateau after the flag lattice, nine type/lifetime/source variants, and a bounded canonical-flag permuter run: the best candidate is 132 instructions against the target's 133, first positional mismatch `+0x0`; the decisive missing instruction is the dead incoming path-index home at `+0x18`, whose absence swaps the entry temporaries and leaves the live path spill at `sp+0x28` instead of target `sp+0x20`. The correct `-mips2` permuter base score was 275 and the capped run found no improvement |
| `0x8005055C` | `0x12C` | `animseqResetPath` | B; reset/process callers and trap/audio call shape. Plateau after 9 variants: best candidate has the exact 75-instruction size and call layout, with 9 positional words remaining (first mismatch `+0x40`); seven are a three-temporary register cycle/tail allocation, and the typed `animResetTrap` call has the correct relocation kind but cannot carry the required `TrapDanglingJump` identity alongside this consolidated TU's incompatible integer-signature calls |
| `0x80050688` | `0x7C` | `animseqStartPath` | B; process-command call position, adopted name. Matched C: exact 124 B and relocation surface at `-O2 -mips2 -32` |
| `0x80050704` | `0x78` | `animseqStopPath` | B; process-command call position, adopted name. Matched C: exact 120 B and relocation surface at `-O2 -mips2 -32` |
| `0x8005077C` | `0x40` | no unique candidate | D; placeholder retained. Matched C: exact 64 B and relocation surface at `-O2 -mips2 -32` |
| `0x800507BC` | `0x88` | `animseqHoldPath` | B; process-command call position, adopted name. Matched C: exact 136 B and relocation surface at `-O2 -mips2 -32` |
| `0x80050844` | `0x38` | `animseqLockPath` | B; paired process-command calls, adopted name. Matched C: exact 56 B and relocation surface at `-O2 -mips2 -32` |
| `0x8005087C` | `0x38` | `animseqUnLockPath` | B; paired process-command calls, adopted name. Matched C: exact 56 B and relocation surface at `-O2 -mips2 -32` |
| `0x800508B4` | `0x20` | no unique candidate | D; placeholder retained. Matched C: exact 32 B and relocation surface at `-O2 -mips2 -32` |
| `0x800508D4` | `0x200` | `func_800772C4` | B; bit-reader call sequence, placeholder retained. Plateau after the flag lattice, nine source/lifetime variants, and a bounded permuter pass: the best candidate is exact-size at 128 instructions with the exact frame, loop, exits, and relocation surface; four preheader words remain from first mismatch `+0x40` because IDO loads `D_80083FA8` before the `0.5f`/`0.390625f` constants, while the target loads those constants first. The permuter imported with the wrong `-mips1` mode and its suggestion regressed the canonical `-mips2` comparison |
| `0x80050AD4` | `0x120` | `animseqLinkNodes` | D; nearest ordered `anim.c` function. Matched C: exact 288 B and `D_800D6B00` relocation pair at `-O2 -mips2 -32 -Wo,-loopunroll,0` |
| `0x80050BF4` | `0x15C` | `animseqInit` | D; 0.753 skeleton similarity. Plateau after 10 source/type shapes and a bounded canonical-flag permuter run: best semantic candidate is 84 instructions versus 87, first mismatch `+0x34`; IDO folds three repeated array-base HI/LO relocation pairs into carried registers. A nominal 1090-score permutation was rejected because it made the scroll-loop condition invariant |
| `0x80050D50` | `0x58` | `func_80077784` | D; nearest `anim.c` skeleton, placeholder retained. Matched C: exact 88 B and relocation surface at `-O2 -mips2 -32` |
| `0x80050DA8` | `0x48` | `animseqFreeLevelData` | B; frees storage then the group, adopted name. Matched C: exact 72 B and relocation surface at `-O2 -mips2 -32` |
| `0x80050DF0` | `0xAC` | `animseqLoadLevelData` | D; nearest ordered `anim.c` function, placeholder retained. Plateau after 10 variants: exact size, opcode schedule, and relocations; 7 operand/register words remain from a three-temporary FIFO rotation and the source stack home at candidate `+0x18` versus target `+0x1C`, first mismatch `+0x28` |
| `0x80050E9C` | `0x168` | `animseqFreeGroup` | B; same member-cleanup call graph. Plateau after 10 variants: best candidate has the exact `0x20` frame and first 25 instructions, then differs at `+0x64` on the `slti` destination and is one instruction short because IDO reuses the preceding `D_800D6BF8` address where the target rematerializes it; `-Wo,-loopunroll,0` is required to avoid a 25-instruction unroll expansion |
| `0x80051004` | `0xE4` | `animseqSetupGroup` | B; calls free/init/reset group family. Plateau after 10 source variants: the best candidate has the exact 57-instruction size and relocation identities but 41 positional words differ, first at `+0x2C`, because removing the extra call-argument rematerialization changes the loop's argument-register allocation |
| `0x800510E8` | `0x40` | `animseqInitGroup` | A; exact 64 B, masked `1/16`, adopted name. Matched C: exact 64 B and relocation surface at `-O2 -mips2 -32` |
| `0x80051128` | `0x9C` | `animseqResetGroup` | B; calls reset-path family, adopted name. Matched C: exact 156 B and relocation surface at `-O2 -mips2 -32` |
| `0x800511C4` | `0x1A0` | `func_80077BE8` | D; 0.321 skeleton similarity, placeholder retained. Plateau after 10 source/lifetime shapes and a bounded canonical-flag permuter: the best semantic candidate has the exact 104-instruction size, `0x48` frame, and five call relocations, but 58 allocation/schedule words remain from first mismatch `+0x34`; the target copies the header count through `v0`/`s0` and reuses one scaled path offset, while IDO keeps the count in `s4` and rematerializes that offset. The nominal 490-score permutation is one word long |
| `0x80051364` | `0x47C` | `animseqUpdate` | D; nearest ordered `anim.c` function. Plateau after the flag lattice, focused type/lifetime variants, and a bounded canonical-flag permuter: the corrected unsigned command clock makes the best semantic candidate exact-size at 287 instructions. Its saved-register slots match, but IDO reserves a `0x48` frame against the target's `0x40`, leaving 192 positional words from first mismatch `+0x0`. The permuter's score-3205 sound-handle lifetime is incorporated |
| `0x800517E0` | `0x1C40` | `animseqProcessCommandList` | B; command dispatcher calls the path family in JFG order |
| `0x80053420` | `0x90` | `animseqCamera` | D; ordered tail and nearest same-family shape. Matched C: exact 144 B and relocation surface at `-O2 -mips2 -32` |
| `0x800534B0` | `0x10` | `animseqPlay` | D adoption; ordered JFG tail and the `playing = 1` store. Matched C: exact 16 B and relocation surface at `-O2 -mips2 -32`; skeleton remains too short for tier A |
| `0x800534C0` | `0x2C` | `animseqPause` | D; ordered `anim.c` tail only, so the placeholder remains. Matched C: exact 44 B and relocation surface at `-O2 -mips2 -32`; the overwritten formal counter is required for IDO's target `$a0` allocation and has no static Mickey caller |
| `0x800534EC` | `0x64` | no unique `hit.c` candidate | D; placeholder retained at the start of collision-shaped code. Matched C: exact 100 B and relocation surface at `-O2 -mips2 -32` |
| `0x80053550` | `0x318` | `hitInitObjectHit` | B; same two matrix-builder calls. Plateau after the flag lattice, approximately 10 type/lifetime/workspace shapes, and a bounded canonical-flag permuter: the best candidate has the exact 198-instruction opcode/register schedule, `0x78` frame, and two call relocations, with 16 stack-offset-only words remaining from first mismatch `+0xA4`. IDO places the three-float workspace at `sp+0x54` and the call-live hit pointer at `sp+0x74`; the target places them at `sp+0x6C` and `sp+0x64`. The permuter normalizes those offsets, reports the base as score zero, and supplies no object-exact alternative |
| `0x80053868` | `0x12D4` | `hitUpdate` | B; collision dispatcher over the following helpers |
| `0x80054B3C` | `0x5C8` | no unique `hit.c` candidate | D; collision/vector shape |
| `0x80055104` | `0x6F4` | no unique `hit.c` candidate | D; collision/vector shape |
| `0x800557F8` | `0x178` | no unique `hit.c` candidate | D; collision handler family. Matched C: exact 376 B and eight-call relocation surface at `-O2 -mips2 -32 -Wo,-loopunroll,0`; reconstructed from Mickey's resident state/counter/audio ABI after no external skeleton exceeded 0.070 similarity |
| `0x80055970` | `0x1B4` | no unique `hit.c` candidate | D; collision handler family. Plateau after the flag lattice, 10 allocation/lifetime shapes, and a bounded canonical-flag permuter: the best full-TU semantic candidate has the exact 109-instruction size and `0x50` frame, but the target keeps the first state in `s2` and the second target in `s3`, while IDO homes the first state on the stack and uses only `s0`-`s2`. The missing saved register shifts all seven calls by one instruction (105 positional words differ, first `+0x8`); the best isolated permutation added an instruction in the consolidated TU and was rejected |
| `0x80055B24` | `0x1E4` | no unique `hit.c` candidate | D; collision handler family. Plateau after the flag lattice, 10 source/lifetime shapes, and a bounded canonical-flag permuter: the Mickey-local composition is exact for 113/121 instructions, all calls, the `0x50` frame, and the FP schedule. Eight words remain from first mismatch `+0x20`: five initial pointer-load/move scheduling words and the three-use `0x258` temporary in `v1` rather than target `v0`. The isolated score-15 assignment reorder does not change full-TU output; score 85 adds an observable store and was rejected |
| `0x80055D08` | `0x148` | no unique `hit.c` candidate | D; collision handler family. Matched C: exact 328 B and three-call relocation surface at `-O2 -mips2 -32 -Wo,-loopunroll,0`; composed from Mickey-local exact state-update and normalization patterns |
| `0x80055E50` | `0x114` | no unique `hit.c` candidate | D; collision handler family. Matched C: exact 276 B and three-call relocation surface at `-O2 -mips2 -32 -Wo,-loopunroll,0` |
| `0x80055F64` | `0x16C` | no unique `hit.c` candidate | D; collision handler family. Plateau after 10 source/type shapes and a bounded canonical-flag permuter: the best semantic candidate has the exact 91-instruction size, `0x48` frame, stack homes, and call relocations, but 46 FP allocation/schedule words remain from first mismatch `+0x2C`; a nominal score-10 permutation was rejected because it reads an uninitialized float |
| `0x800560D0` | `0x1A4` | no unique `hit.c` candidate | D; collision handler family. Matched C: exact 420 B and three-call relocation surface at `-O2 -mips2 -32 -Wo,-loopunroll,0`; composed from Mickey-local dual-state advance and normalization patterns |
| `0x80056274` | `0x140` | no unique `hit.c` candidate | D; collision handler family. Matched C: exact 320 B and three-call relocation surface at `-O2 -mips2 -32 -Wo,-loopunroll,0`; the Mickey-led declaration order fixes the two target-pointer spill homes without importing a donor body |
| `0x800563B4` | `0xA24` | `hitVectorCheck` | B; vector/cylinder/sphere-style callee pattern |
| `0x80056DD8` | `0x394` | no unique `hit.c` candidate | D; collision/vector shape. Plateau after the flag lattice, focused expression/lifetime variants, and an eight-minute canonical-flag permuter batch: the best semantic full-TU candidate has 226 instructions against 229 and a `0x80` frame against `0x70`; all eight call/global relocation identities agree, but 214 positional words differ from first mismatch `+0x0` because the extra local/spill space changes the FP schedule. The nominal lower-score permutation read a branch-local normal component before initialization and was rejected |
| `0x8005716C` | `0x140` | no unique `hit.c` candidate | D; the prior JFG `hitGetInelasticVelocity` suggestion is structurally unrelated. Plateau after 10 source/type shapes and a bounded canonical-flag permuter run: best candidate is exact-size at 80 instructions with the target's 0x28 frame and HI/LO relocation, 18 FP register/schedule words remain, first mismatch `+0x54` |
| `0x800572AC` | `0xA4` | no unique `hit.c` candidate | D; collision handler, placeholder retained. Matched C: exact 164 B and relocation surface at `-O2 -mips2 -32` |
| `0x80057350` | `0x78` | no unique `hit.c` candidate | D; collision handler, placeholder retained. Matched C: exact 120 B and relocation surface at `-O2 -mips2 -32` |
| `0x800573C8` | `0x3A4` | no unique `hit.c` candidate | D; collision/vector shape. Plateau after the flag lattice, focused stack/loop shapes, and a bounded canonical-flag permuter: the best semantic candidate has 231 instructions against 233, the exact `0x88` frame and exact bounds-array offsets, with 203 positional words remaining from first mismatch `+0x2C`. A shared separation label reproduces the target loop CFG; IDO still folds two target pointer-initialization instructions and changes the surrounding FP allocation. The permuter's score-1980 expression split is incorporated |
| `0x8005776C` | `0x1A4` | `hitPlayer` | B; same player-list/square-root call shape. Plateau after the flag lattice, 10 source/workspace shapes, and a bounded canonical-flag permuter: the best full-TU semantic candidate has the exact 105-instruction size and `0xC0` frame, with 53 words remaining from first mismatch `+0x24`; the target rotates the saved players/result/count registers and places the count/distance workspace at `sp+0x7C`/`sp+0xA0`. A nominal score-545 permutation was rejected because it failed to reset the distance cursor on each outer sort pass |
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
| `0x8001879C` | `0x130` | `setupLights` | tier-B comparison; `NON_MATCHING` plateau at 75/76 linked words, first mismatch +`0x98` from commutative `addu` operand order |
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
| `0x80019934` | `0xF0` | `lightDistanceCalc` | tier-B comparison; `NON_MATCHING` plateau: canonical flag sweep gives 60/60 text words and exact call relocations, but the shared resident rodata segment retains `jtbl_800817B4`, so promotion duplicates the compiler's anonymous table and leaves the extracted table's five local labels unresolved |
| `0x80019A24` | `0x94` | `lightDirectionCalc` | Tier A: JFG C is compiler/link exact |
| `0x80019AB8` | `0x2E0` | `lightObject` | tier-B comparison: calls all three `lights2` pipelines |
| `0x80019D98` | `0x50` | `lightDefaultObjectLight` | Tier A: Mickey/JFG-adapted C is compiler/link exact |
| `0x80019DE8` | `0xFC` | `lightSetObjectLight` | tier-D boundary; `NON_MATCHING` plateau after the flag lattice, 10 source/type hypotheses, and a 10-minute permuter batch: exact `0x38` frame, 64 instructions versus 63, 46 positional words differ, first `+0x48` from byte-store/delta scheduling; JFG body is also assembly-only, so retain `func_` |
| `0x80019EE4` | `0x98` | `lightSetupLightSources` | Tier A: Mickey/JFG-adapted C is compiler/link exact |
| `0x80019F7C` | `0x8C` | `lightSetupFlareSources` | Tier A: Mickey/JFG-adapted C is compiler/link exact |
| `0x8001A008` | `0x14C` | `lightInitObjectLighting` | tier-B comparison; `NON_MATCHING` plateau after the flag lattice and nine source/declaration forms: exact 83-word frame/opcode/register/FP/relocation shape, but 4 positional words differ, first `+0x70`, because the call-live result spills at `0x28(sp)` instead of `0x2C(sp)`; the permuter importer scores the isolated function zero, but the required full-TU build retains this mismatch |
| `0x8001A154` | `0xE8` | `lightAdjustGlowingLight` | tier-B comparison; `NON_MATCHING` plateau after flag sweep, 10 source/lifetime hypotheses, and a 10-minute permuter batch: exact 58-opcode/frame shape and call relocation, 45/58 words exact, first `+0x1C` from integer temp-FIFO phase |
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

**Matching progress.** Ninety-three functions / 8,104 bytes compile exactly
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
- `main/main` (59 / 5,784 bytes): `RevealReturnAddresses`, `mainGetZBCheck`,
  `mainGameWindowChanging`,
  `mainGameWindowSize`, `mainSetGameWindow`, `mainSetAnimGroup`,
  `mainGetAnimGroup`,
  `mainChangeCameras`, `mainGetNextCharacter`, `mainGetNextLevel`,
  `func_80027628`, `mainAddZBCheck`,
  `func_80027EC0`, `func_80027FB8`, `func_800282C8`,
  `mainResetPressed`, `mainPreNMI`, `mainInitGame`, `mainChangeLevel`,
  `mainSyncNextLevel`,
  `mainGetMode`, `mainSetMode`,
  `mainTitlePageInit`,
  `mainFrontInit`, `mainStartGame`,
  `mainGetNumberOfCameras`, `func_80028DE4`, `func_80028EA0`, `func_80028F3C`,
  `func_80028F44`, `func_80028F4C`, `func_80028F54`,
  `func_80028F60`, `func_80028F98`,
  `func_80028FA8`,
  `func_80028FB8`,
  `func_80029038`, `func_8002904C`, `func_8002905C`, `func_80029084`,
  `func_800290A0`, `func_800290AC`,
  `func_80029090`, `func_800290EC`, `func_800290F8`, `func_80029104`,
  `func_80029120`, `func_80029144`, `func_80029160`, `func_8002917C`,
  `func_80029198`,
  `func_800291B4`,
  `func_800291C4`,
  `func_800291D0`, `func_800291D8`, `func_800291E4`, `func_800291FC`, and
  `func_80029240`, and `func_800293D0`.

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

- `joyInit`, eight source/storage hypotheses, the full flag lattice and a
  bounded two-worker canonical-MIPS-II permuter batch, first mismatch
  `+0x11C`: the JFG-shaped candidate is exact through the controller scan but
  compiles to 86 rather than 83 instructions. External `D_800CF3B4` storage
  makes IDO materialize four HI16/LO16 pairs for the final byte clears; the
  target shares one HI16 and names `D_800CF3B4` through `D_800CF3B7` in four
  distinct LO16 relocations. Alternative scalar and aggregate declarations
  disrupt the otherwise exact loop; exposing the named bytes in a block-scoped
  comma expression expands the function to 115 instructions. The permuter
  found no improvement from its base score of 325.
- `joyRead`, six loop/storage/type hypotheses, the full flag lattice and a
  bounded two-worker permuter batch, first mismatch `+0x18`: the JFG-shaped
  candidate has the exact 636-byte size, 159-instruction schedule and `-0x38`
  frame, but differs in 48 words. Original TU-local adjacency lets IDO name
  `D_800CF388`, `D_800CF3BC` and `D_800CF3B0` as three loop endpoints; the
  split extern layout materializes the preceding bases plus their array sizes,
  leaving six relocation-identity mismatches. The permuter's 5,795-to-5,305
  improvement required an invented do-while guard and was rejected.
- `func_80026FB4`, nine structural/display-command hypotheses, the full flag
  lattice and a bounded two-worker resident-MIPS-II permuter batch, first
  mismatch `+0x48`: the Mickey-derived main-loop candidate needs
  `-Wo,-Olimit,100` to reproduce the target's `-0x28` frame and transition
  result at `sp+0x24`, but compiles to 418 rather than 413 instructions. IDO
  assigns the first display-list pointer store through `$at` instead of the
  target's `$a0`; the remaining five-word structural excess is concentrated
  in the two end-of-frame display commands. The valid permuter score improved
  from 3,620 to 3,050 by introducing a matrix-array temporary, not identity.
- `func_80028564`, ten control-flow, storage, aggregate-layout and
  framebuffer-loop hypotheses, the full 119-combination flag lattice and a
  bounded two-worker resident-MIPS-II permuter batch, first mismatch `+0x4`:
  the best Mickey-derived transition/level-load body has the target's `-0x58`
  frame but 492 rather than 489 instructions. IDO saves an otherwise unused
  `$s0` and places `$ra` at `sp+0x2C`, while the target saves only `$ra` at
  `sp+0x24`. The target's six character writes share a `2 * 0x28` base;
  explicit array fields are three words long overall, while loop and grouped
  index forms miss by 20 and 40 words. The valid permuter score improved from
  13,270 to 11,970 only by reusing a pointer alias on paths where it is
  uninitialized, so that candidate was rejected.
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
- `mainCPUeffects`, seventeen type/expression/storage hypotheses and the full flag
  lattice, first mismatch `+0x48`: the best Mickey-derived candidate preserves
  all 85 target opcodes, the 340-byte boundary and `-0x40` frame, but ten
  temp-FIFO register operands differ in the cropped-framebuffer calculation.
  Its typed overlay-call alias also retains a different relocation identity at
  `+0xd8`; the natural unprototyped call instead promotes the float arguments,
  adding four instructions and eight frame bytes. A typed function-pointer
  cast emits an indirect call, while raw-integer framebuffer and zero-code
  FIFO/line probes normalize back to the same object.
- `func_80027D14`, eight control-flow/register-lifetime hypotheses, the full
  flag lattice and a bounded two-worker permuter batch, first mismatch `+0x0`:
  the best Mickey-derived interpolation candidate compiles to 92 rather than
  91 instructions. IDO starts its global-pointer live range in `$t1` rather
  than the target's `$t0`, leaving 88 differing words and 24 relocation-position
  mismatches; the permuter found no improvement from its base score of 4,900.
- `levelGetCounts`, ten source/type/loop hypotheses, first mismatch `+0x13c`:
  the best candidate has the target's 1,036-byte size, 259-instruction opcode
  schedule and `-0x58` frame, but three register operands use `$v0` where the
  target uses `$a0`. Its initial count-table loop also relocates against
  `D_800CF3E0`, while the target object's HI16/LO16 pair names `D_800CF420`.
  The resident flag lattice was unchanged; a bounded two-worker MIPS II
  permuter batch improved its internal score from 45 to 25 but did not change
  these object-level residuals.
- `levelInit`, ten structural, storage, type and register-lifetime hypotheses,
  the full 119-combination flag lattice and a bounded two-worker permuter
  batch, first mismatch `+0x238`: the JFG-adapted, Mickey-specific candidate
  reproduces all 516 target opcodes, the 2,064-byte boundary, `-0x80` frame,
  stack homes and relocation identities, but 122 register operands differ.
  The first residual is a temp-FIFO allocation (`$t4` rather than `$t7`) in
  the fog-load delay slot; the following resource-table address starts a pool
  allocation divergence (`$a2` rather than `$a3`). The permuter improved its
  MIPS I import from 12,975 to 12,580 only with a redundant fog-width mask;
  canonical MIPS II recompilation added two instructions, so it was rejected.
- `joyResetMap`, first mismatch `+0x0`: external storage emits 48 rather than
  36 bytes; TU-local storage is instruction-exact but wrongly claims 16 B of
  BSS and shifts the real symbol. A fresh descending scalar-extern probe kept
  the correct four relocation identities but still materialized four address
  pairs, and weak tentative definitions retained the same 16 B BSS claim; the
  repeated 119-combination flag lattice did not improve the valid candidate.
- `func_80028FCC`, thirteen structural/ABI spellings, first mismatch `+0x1c`:
  its 108-byte skeleton
  identifies the tier-B `mainAnyoneHas` role (JFG: 108 B, similarity 0.357),
  but Mickey passes zero as every middle argument. The exact-sized candidate
  differs in ten words: raw-return branches versus target normalization into
  `$t6`/`$t7`/`$t8` and a shared epilogue. A fresh old-style call declaration,
  logical-OR spelling, explicit boolean lifetimes, and the full flag lattice
  did not improve that result.
- `levelFreeAll`, ten spellings, first mismatch `+0x13c`: exact 468-byte size
  and 113/117 words; only the masked resource index/table-base registers swap.
- `func_80028EFC`, fifteen control/typing spellings, first mismatch `+0x1c`:
  exact 64-byte size
  and 14/16 words; the correct loop predicate is allocated to `$t6`, while the
  target uses `$at`. Fresh byte-pointer, `void *` cursor, explicit-goto and
  post-increment-bound probes either unrolled or entered a three-word
  strength-reduced basin; the repeated flag lattice did not improve the
  two-register residual.
- `func_80029274`, seventeen control-flow/parameter/register-lifetime
  hypotheses and the full flag lattice: the best canonical candidate has the
  exact 348-byte, 87-instruction boundary and `-0x10` frame, but differs in 39
  words, first at `+0x8`. Initializing the accumulators before copying the
  velocity correctly anchors `$f2`; IDO still hoists the first float argument,
  colors the velocity/distance webs as `$f12`/`$f16` rather than `$f14`/`$f12`,
  and reshapes the negative-velocity return path. The size-exact `-g3` probe
  reaches 38 differing words, first at `+0x14`, but is not exact and does not
  justify a TU flag override.

The full flag lattice produced no exact result for any of these plateaus; the
single one-word `-g3` improvement is recorded above and was not adopted.

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

Exact C closures in these splits begin with 680 bytes across seven `diCpu`
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
The 204-byte `func_80044BC8` (`diRcpTrace` in JFG) is exact too. Its typed
three-word trace entries and direct global-index expressions reproduce all 51
target words, including the repeated buffer/count reloads, the 100-entry
limit, and all four data relocation pairs at the resident defaults.
The 100-byte `stop_all_threads_except_main` is exact on Mickey's active-thread
walk: it filters priorities 1 through 127, passes the thread itself to
`osStopThread`, and retains the exact call relocation and 32-byte frame under
the resident defaults.
The 60-byte `func_800453C4` display-list unpacker is exact too: its four typed
word extractions reproduce all 15 target instructions at the resident defaults
and have no relocation surface.
Its adjacent 60-byte `func_80045400` unpacker is likewise exact: the alternate
24/16/8-bit field split retains all 15 target instructions and has no
relocations under the same flags.
The 1,540-byte `diRcpPrintDL` dispatcher reaches an instruction-exact source
plateau: JFG's natural nested switches reproduce all 385 target words, the
32-byte frame, helper-call order, and every named diagnostic-string relocation
under the resident defaults. Promotion is blocked by section ownership. IDO
emits three switch tables into this TU's `.rodata`, but the same tables remain
inside the shared `0x81590` rodata segment outside this lane's ownership; the
result has six HI16/LO16 table-relocation identity mismatches beginning at
function `+0x44` (plus two local PC16 assembler-metadata differences) and a
duplicate linked table surface. The exact source remains behind
`NON_MATCHING`, with the target assembly canonical until that rodata split is
handed off.
The 156-byte `diRcpMoveWd` helper reaches the same section-ownership plateau.
JFG's six-case empty switch reproduces all 39 target instructions, the
96-byte frame, both calls, and the diagnostic-string relocation at the
resident defaults. IDO also emits the correct 11-entry switch table, but binds
the function's first table relocation at `+0x34` to this TU's `.rodata` rather
than Mickey's existing shared `jtbl_80083950` symbol, creating a duplicate
linked table surface. The instruction-exact body remains behind
`NON_MATCHING` until the shared rodata split is handed off.
The 268-byte `diRcpGeometryMode` helper is exact at the resident defaults.
JFG's object-like `stubbed_printf` macro preserves the target's empty geometry-
flag switch and its otherwise-unused saved registers, reproducing all 67 owned
instructions plus the `sprintf` call and format-symbol relocations. The two
following target words are end-of-TU alignment padding outside the function;
IDO supplies them through normal section alignment, and the linked range is
byte-identical without post-compile editing.
The 1,004-byte `func_800475E8` (`fxMakeConeTextureCoords`) reaches a structural
plateau from Mickey's recovered coordinate-generation loops. JFG confirms the
identity but its peer is also assembly-only. After the full flag lattice and
four coherent loop/lifetime variants, the closest relevant candidate needs
`-Wo,-loopunroll,0`, is six instructions long (257 versus 251), and uses a
256-byte frame instead of 248 bytes; all 257 positional words differ beginning
at function `+0x0` because the extra scalar homes shift the complete GPR/FPR
allocation. The typed candidate remains behind `NON_MATCHING`; the TU-wide
unroll override is not adopted without an exact result and an impact proof for
the existing matches.
The 936-byte `func_80047CD8` (`fxDrawCone`) reaches an eight-word allocation
plateau after the full flag lattice and ten source-shape hypotheses. Recasting
its opaque words as JFG-style `gSPVertexJFG` and `gSPPolygon` macros reproduces
the target's exact 234-instruction size, 104-byte frame, saved-register set,
control flow and helper-call relocations under the resident defaults. The
first mismatch is function `+0x298`: in the variable-count path IDO assigns
the cone mode and triangle count to different argument registers than the
target, affecting eight words while leaving the command arithmetic and all
surrounding instructions exact. Explicit locals fix those eight uses only by
perturbing the frame or earlier allocation, so the typed macro reconstruction
remains behind `NON_MATCHING` and the target assembly stays canonical.
The 84-byte `diCpuTraceInit` is exact at the resident defaults. Keeping JFG's
distinct thread-control-block and stack-top declarations reproduces the target
evaluation schedule; Mickey resolves both operands to the same address, so the
linked function and its three call/data relocation pairs are exact.
The 88-byte `func_80046E00` screen-clear helper is also exact at the resident
defaults. JFG's natural framebuffer pointer/countdown loop reproduces all 22
owned instructions and the `viGetCurrentSize` plus framebuffer relocations;
the two following target words are alignment padding outside the function.
The 136-byte `func_80045CAC` active-thread scanner is exact as well. JFG's
natural thread-list loop reproduces all 34 owned instructions, including both
branch-likely paths, the 32-byte frame, the active-queue relocation, and both
crash-handler call relocations at the resident defaults.
The 140-byte `cpuXYPrintf` formatter is exact too. JFG's 255-byte local text
buffer and natural varargs setup reproduce the 288-byte frame, all 35 target
instructions, both call relocations, and the display-mode data relocation at
the resident defaults.
The 164-byte `diCpuReportWatchpoint` reporter is exact as well. JFG's natural
100-iteration clear loop, address-information query, two diagnostic prints,
and terminal wait reproduce all 41 target words, the 56-byte frame,
and every call and string relocation at the resident defaults.
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
are exact under the same defaults. The 108-byte `func_8004A4B0` appends a
compact eight-byte record to one of two four-entry effect queues. Its natural
post-incremented count subscript and field-order assignments reproduce all 27
target instructions plus the three data relocation pairs under the resident
defaults.
The 164-byte `func_8004A51C` consumes the completed half of that double-buffered
queue and clears the newly selected half. Keeping the record's final fields
unsigned and spelling the queue toggle as a global assignment followed by a
global-indexed clear reproduces all 41 target words, the 40-byte frame, the
five-argument helper call, and all data/call relocations at the resident
defaults.
The 304-byte `func_8004A380` decimal-glyph renderer reaches a local-layout and
allocation plateau after the full flag lattice, six coherent buffer/scan
forms, and one bounded canonical MIPS II permuter batch. JFG's assembly-only
`func_8006DF90` confirms the decimal formatting, minimum-width padding, glyph
selection, and draw-call loop, while Mickey fixes the five-argument ABI and
data symbols. The best coherent candidate retains the 128-byte frame and
intended control flow but emits 74 words against 76 in the target, with 67
positional differences beginning at function `+0x10`. IDO hoists the local
text-buffer base into `s2` at `sp+0x60`; the target places the buffer at
`sp+0x54`, keeps `s2` as the zero/index state, and forms scan and render
addresses through temporary bases. The batch's lower score appended an empty
buffer guard solely to perturb allocation and was discarded as semantically
unsupported. The coherent candidate remains behind `NON_MATCHING` and the
target assembly stays canonical.
The 76-byte JFG-identified `fxInit` is exact as well: its post-decrement loop
clears all five 32-byte records, resets the global state, and preserves the
callee plus two data relocation pairs without normalization.
The 136-byte Mickey-named `func_80049A8C` resets either one record or all five,
clearing state/status and two flag bits. Its selection branches, stack home,
countdown loop, and data relocation pair are exact at the resident defaults.
The preceding 156-byte `func_8004978C` reaches an exact-size frame-allocation
plateau after the full flag lattice and nine coherent local-layout hypotheses.
The Mickey-derived typed body selects one or all five records and applies the
caller-selected flag mask with the exact 39-instruction control flow, register
allocation, stack spill at `sp+0x4`, and data relocation pair. Only the leaf
frame adjustment differs: IDO chooses an 8-byte frame while the target uses 16
bytes, leaving two words different from function `+0x4`. Padding, aggregate,
iteration-pointer, and qualifier variants either leave that frame unchanged or
disturb otherwise-exact allocation, so the clean scalar candidate remains
behind `NON_MATCHING` and the target assembly stays canonical.
The 180-byte `func_8004AD34` (`fxGenerateTextures` in JFG) is exact too. Its
four-entry descending callback loop, flag test, callback-table refresh, and
indirect call retain all target instruction words and relocation identities at
the resident defaults; spelling the constant-count loop as `while (index--)`
reproduces IDO's rotated `3`-through-`0` schedule without normalization.

The 112-byte `func_8004ACC4` reaches an exact-size allocation plateau after
the full flag lattice, nine coherent lifetime/source forms, and one bounded
permuter batch. JFG's same-size `func_8006FFF8` establishes the descending
four-slot initialization skeleton, while Mickey fixes the independent store
order. The best typed candidate has the exact loop and relocation set but
differs in 18 of 28 words under the resident defaults, first at function
`+0x14`: IDO assigns the counter, trap address and callback cursor one register
group earlier than the target. The candidate remains behind `NON_MATCHING`.

The 208-byte `func_8004AF68` (`fxCpuTextureFlush` in JFG) reaches a bounded
boundary/allocation plateau after the full flag lattice, nine coherent
pointer/index forms, and one canonical MIPS II permuter batch. JFG's peer is
also assembly-only but confirms the four-slot descending free/reset skeleton.
The best typed candidate keeps separate byte-offset and loop-counter
inductions, the two conditional frees, callback reset, 56-byte frame, and all
relocation identities, but IDO hoists the secondary texture-array base into
`s5`. That displaces the callback/trap pair into `s6`/`s7`, emits 54
instructions against 52 owned words, consumes the two following target padding
words, and leaves 48 positional differences beginning at function `+0x4`.
The batch's lower score moved the offset decrement under a conditional and was
discarded as semantically invalid; the coherent candidate remains behind
`NON_MATCHING` and the target assembly stays canonical.

`diCpuThread` reached a bounded `NON_MATCHING` plateau after the full flag
lattice and ten source/lifetime hypotheses. The best candidate has the exact
85-instruction size, 88-byte frame, saved-register set, control flow and
relocation identities. Its first mismatch is function `+0x90`: IDO schedules
the invariant `D_80083DBC` load before the low half of the `999999` loop
constant, while the target emits those adjacent instructions in the opposite
order. The target assembly remains canonical.

The 240-byte `func_80045BBC` fault-state writer reaches a two-word allocation
plateau after the full flag lattice, ten coherent declaration/lifetime forms,
and one bounded canonical MIPS II permuter batch. JFG's `func_80066D28_67928`
supplies the copy-and-write structure, while Mickey's fixed diagnostic-buffer
addresses establish the dynamic aligned dump-size calculation. The best
candidate is exact in size, frame, saved registers, control flow, stack homes,
call order, and relocation identities, with 58 of 60 instruction positions
exact. At the first mismatch, function `+0xBC`, IDO loads the fifth
`packWriteFile` argument into `t4`; the target uses `t6`, affecting only that
load and its stack store. Casts, explicit call-argument locals, prototype
forms, and the bounded permutations do not change that coloring without
disturbing otherwise-exact code. The candidate remains behind `NON_MATCHING`
and the target assembly stays canonical.

The 292-byte `func_80046AA8` packed-glyph renderer reaches a loop-form plateau
after the full flag lattice, nine coherent counter/type/source forms, and one
bounded canonical MIPS II permuter batch. Mickey's caller supplies an `(x, y)`
pixel position and a five-word two-bit glyph; JFG's 288-byte
`func_800680B0_68CB0` is assembly-only but confirms the same framebuffer,
palette, row, line, and packed-pixel loops. The best coherent candidate has
the exact 72-byte frame, argument homes, `s0` glyph lifetime, framebuffer
association, palette branches, and relocation identities, but emits 70 words
against 73 in the target, with 49 positional differences beginning at
function `+0x54`. IDO folds the target's constant five-row pretest and one
narrowed-bit temporary move, then allocates the packed bits and pixel cursor
to `v0`/`v1` instead of `v1`/`a0`. Lower permuter scores inserted empty guards
only to perturb allocation and were discarded as semantically unsupported.
The coherent candidate remains behind `NON_MATCHING` and the target assembly
stays canonical.

The 300-byte `func_80044C94` trace-neighbor lookup reaches an allocation and
loop-scheduling plateau after the full flag lattice, seven coherent
type/lifetime/bound forms, and one bounded canonical MIPS II permuter batch.
JFG's assembly-only `diRcpTraceGetInfo` confirms the inactive-buffer scan and
nearest-lower/nearest-upper entry selection; Mickey's callers and three-word
trace entry fix the seven-argument ABI. The best candidate has the exact
75-word size, 8-byte frame, argument homes, unsigned comparisons, branch
structure, and all six data-relocation identities, but differs at 35
instruction positions beginning at function `+0xC`. Two positions swap the
hoisted `count * 12` bound shift with the byte-offset initialization; the
remaining differences are IDO temp-ring and pool allocation across the count,
buffer base, cursor, entry value, and selected-neighbor webs. Naming the bound
makes every opcode positional but worsens the residual to 47 register words;
the lower-score permuter result's explicit entry size and selected offset is
retained as the coherent best. The candidate remains behind `NON_MATCHING`
and the target assembly stays canonical.

The 1,836-byte `func_80045D34` crash-screen controller also remains
`NON_MATCHING`. Supplying its jump table recovered a complete Mickey-derived
draft, but JFG's closest 1,888-byte peer (`func_80067880`) is assembly-only and
offers no source body. At the resident defaults the best typed candidate is
eight instructions short (451 versus 459), uses a 176-byte frame instead of
168 bytes, and differs in 432 positional words from function `+0x0`; the full
flag lattice's smaller MIPS I result cannot be adopted for a TU containing
existing MIPS II exact matches. Source/lifetime and named-string experiments
remained structural mismatches, so the target assembly stays canonical.

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
| `0x8001C2C4` | `0x8` | — | Matched C: exact empty routine under O2/mips2; retain `func_` |
| `0x8001C2CC` | `0x8` | — | Matched C: exact empty routine under O2/mips2; retain `func_` |
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
| `func_80000450` | `0x1050` / `0xC0` | **tier B**: JFG supplies the `amSetMuteMode` body and exact audio-manager order; Mickey's segment-start placeholder is retained for existing address arithmetic and overlay declarations | Exact 48 object words and all data/call relocations |
| `func_80000510` | `0x1110` / `0x84` | **tier B**: JFG supplies the `amTunePlay` control flow and exact audio-manager order; Mickey's external placeholder is retained, and Mickey's shorter target omits JFG's later tempo/count updates | Exact 33 object words and all data/call relocations |
| `amTuneVoiceLimit` | `0x1194` / `0x38` | **tier B**: JFG supplies the complete body and official name; the block flag, tune-player call, and exact audio-manager order agree | Exact 14 object words and all data/call relocations |
| `func_800005CC` | `0x11CC` / `0xF0` | **tier B**: JFG supplies the `amTuneSetFade` body and exact audio-manager order; Mickey's externally used placeholder is retained | Exact 60 object words and all data/call relocations |
| `amTuneResetFade` | `0x1330` / `0xC` | **tier B**: exact JFG routine order and the adjacent tune-fade controller role | Exact object words and linked ROM bytes |
| `amAmbientSetFade` | `0x133C` / `0xF0` | **tier B**: JFG supplies the complete body and official name; the paired fade-state globals, TV-rate paths, and exact audio-manager order agree | Exact 60 object words and all data/call relocations |
| `amAmbientResetFade` | `0x142C` / `0xC` | **tier B**: exact JFG routine order and the adjacent ambient-fade controller role | Exact object words and linked ROM bytes |
| `amAudioTick` | `0x1438` / `0x284` | **tier B**: JFG supplies the official name, fade controllers, delayed-sound queue, and exact audio-manager order; Mickey's two sequence-init calls and master-volume fade tail remain authoritative | Exact 161 object words and all message-queue, fade, delayed-sound, sequence, and master-volume relocation identities |
| `amWaitForMidiSync` | `0x16BC` / `0x80` | **tier B**: JFG supplies the official name and exact audio-manager order; Mickey's own code pins the pending-sync flag, blocking receive loop, and pre-NMI call | Exact 32 object words and all flag/queue/call relocations |
| `amResetMidiSync` | `0x173C` / `0xC` | **tier B**: JFG supplies the official name and exact audio-manager order; Mickey's own code clears the same pending-sync flag consumed by `amWaitForMidiSync` | Exact 3 object words and data relocation identity |
| `func_80000B48` | `0x1748` / `0xA0` | **tier B**: JFG supplies the `amTuneSetChlMask` name and exact audio-manager order; Mickey's external placeholder is retained, while the body and `u8` call ABI come from Mickey-only evidence | Exact 40 object words and all player/mask/call relocations |
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
| `func_80001308` | `0x1F08` / `0x74` | **tier B**: JFG supplies the alternate sequence-initializer role and audio-manager order; Mickey's placeholder is retained, while its body and resident sequence-count field are reconstructed from Mickey-only evidence | Exact 29 object words and all sequence-count, player-state, and call relocation identities |
| `func_8000137C` | `0x1F7C` / `0x1EC` | **tier B**: JFG supplies the `music_sequence_init` role and exact audio-manager order; Mickey's placeholder is retained, while its body and resident metadata types are independently reconstructed from Mickey-only evidence | Exact 123 object words and all sequence-table, player-state, channel-mask, and call relocation identities |
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
| `func_80002500` | `0x3100` / `0xC4` | **tier B**: JFG supplies the `amInitAudioMap` role and exact audio-manager order; Mickey's externally visible placeholder is retained, while its pool sizes, point stride, handle field, and body come from Mickey-only evidence | Exact 49 object words and all sound-table, allocator, point-pool, count, and reset-call relocation identities under `-Wab,-r4300_mul` |
| `audspat_jingle_off` | `0x31C4` / `0x28` | existing **tier A** audio-spatial object identity; JFG's `amAmbientPause` supplies the complete body while Mickey's title-specific name remains authoritative | Exact 10 object words and both call/data relocations |
| `amAmbientRestart` | `0x31EC` / `0xC` | **tier B**: JFG supplies the complete body and official name; the paired ambient-pause flag and exact audio-manager order agree | Exact 3 object words and data relocation identity |
| `amResetAudioMap` | `0x31F8` / `0x170` | **tier B**: JFG supplies the official name, reset role, and exact audio-manager order; Mickey's point/free-pool layout, queue slots, module ID, and body remain authoritative | Exact 92 object words and all point/free-pool, active-count, queue-slot, sound-stop, module-query, and trap relocation identities under `-Wab,-r4300_mul` |
| `amPlayAudioMap` | `0x3368` / `0x720` | **tier B**: JFG supplies the official name and exact audio-manager order; DKR supplies the related positional-point update body, while Mickey's reduced point-only update and dynamic-module tail remain authoritative | Exact 456 object words and all camera, sound-player, point-heap, and dynamic-module relocation identities under `-Wab,-r4300_mul` |
| `amCalcSfxStereo` | `0x3A88` / `0x158` | **tier B**: JFG supplies the complete body and official name; the single-camera transform, signed angle bands, and exact audio-manager order agree | Exact 86 object words and all matrix/call relocation identities |
| `func_80002FE0` | `0x3BE0` / `0xBC` | **tier B**: JFG supplies the `amSndPlayXYZ` body, `SoundData` layout, and exact audio-manager order; the Mickey placeholder is retained | Exact 47 object words and all table/call relocations |
| `func_8000309C` | `0x3C9C` / `0x18` | **tier B**: JFG supplies the `amSndSetVolXYZ` body and exact audio-manager order; the externally visible Mickey placeholder is retained | Exact 6 object words; no relocations |
| `func_800030B4` | `0x3CB4` / `0x18` | **tier B**: JFG supplies the `amSndSetPitchXYZ` body and exact audio-manager order; the Mickey placeholder is retained | Exact 6 object words; no relocations |
| `func_800030CC` | `0x3CCC` / `0xF4` | **tier B**: JFG supplies the `amSndPlayDirectXYZ` body, prototype, and exact audio-manager order; the Mickey placeholder is retained | Exact 61 object words and call relocation identity |
| `func_800031C0` | `0x3DC0` / `0x28` | **tier B**: JFG supplies the `amSndSetXYZ` body and exact audio-manager order; Mickey's external placeholder is retained | Exact 10 object words; no relocations |
| `func_800031E8` | `0x3DE8` / `0x68` | **tier B**: JFG supplies the `amSndStopXYZ` body and exact audio-manager order; Mickey's widely used external placeholder is retained | Exact 26 object words and all heap/count/call relocations |
| `amSndUnlinkHandleXYZ` | `0x3E50` / `0x4C` | **tier B**: JFG supplies the official name and exact audio-manager order; Mickey's own body pins the heap search and handle unlink field | Exact 19 object words and all heap/count relocation identities |
| `func_8000329C` | `0x3E9C` / `0x114` | **tier B**: JFG supplies the `amCreateAudioPoint` role, prototype, and exact audio-manager order; Mickey's placeholder is retained, while its high-water mark, free/used pools, point-field layout, and body come from Mickey-only evidence | Exact 69 object words and all count, high-water, free-pool, and used-pool relocation identities under `-Wab,-r4300_mul` |
| `func_800035F8` | `0x41F8` / `0x168` | **tier B**: JFG supplies the ordered positional-update placeholder peer; Mickey's placeholder is retained, while the per-group queue, point layout, and sound-parameter update body come from Mickey-only evidence | Exact 90 object words and all group-count, update-entry, sound-start, parameter, priority, and echo relocation identities under `-Wab,-r4300_mul` |
| `func_800037C4` | `0x43C4` / `0x128` | **tier B**: Mickey callers pin the used-pool removal role and JFG supplies the ordered placeholder peer; no donor placeholder is adopted | Exact 74 object words and all sound-stop, auxiliary-cleanup, free/used-pool, and count relocation identities under `-Wab,-r4300_mul` |
| `func_800038EC` | `0x44EC` / `0xF8` | **tier B**: JFG supplies the `amSndGetXYZVolume` role and terminal audio-manager order; Mickey's placeholder is retained, while the coordinate distance, sound-setting layout, and attenuation body come from Mickey-only evidence | Exact 62 object words and all settings-table, `sqrtf`, and floating-constant relocation identities under `-Wab,-r4300_mul`; the following `0xC` bytes are TU alignment padding, not function credit |
| `amSndSetPan` | `0x1E04` / `0x28` | existing **tier A** JFG byte identity | Exact object words and relocation identity |
| `forcelink` | `0x2298` / `0x30` | existing **tier A** JFG byte identity | Exact object words and both call relocations |
| `amVibratoInit` | `0x45F0` / `0x90` | existing **tier A** JFG byte identity; BK supplies the matching free-list source shape | Exact 36 object words and linked ROM bytes. Relocation count/type/offset are exact; splat's per-element pool symbols resolve identically to the C array-base relocations plus their element addends |
| `amInitOsc` | `0x4680` / `0x2D8` | **tier B**: JFG supplies the official name and exact oscillator-TU order; Perfect Dark supplies the related oscillator-init vocabulary and source shape, while Mickey's eight-case state initialization and layout remain authoritative | Exact 182 object words and all free-list, depth-conversion, cents-ratio, and state-field relocation identities |
| `amUpdateOsc` | `0x4958` / `0x574` | **tier B**: JFG supplies the official name and exact oscillator-TU order; Perfect Dark supplies the related oscillator-update vocabulary and source shape, while Mickey's eight-case state machine and layout remain authoritative | Exact 349 object words and all sine, cents-ratio, constant, and state-field relocation identities |
| `amStopOsc` | `0x4ECC` / `0x18` | **tier B**: JFG supplies the official name and exact oscillator-TU order; Mickey's own body returns the state to the free-list head | Exact 6 object words and both free-list relocation identities |
| `_depth2Cents` | `0x4EE4` / `0x50` | existing **tier A** JFG byte identity, independently corroborated by BK's compiled object | IDO 5.3, `-O2 -mips2 -32 -Wab,-r4300_mul`; exact object words/relocations, with `0xC` target padding excluded |

Measured plateau:

| Mickey routine | Best result | First mismatch | Remaining hypothesis |
|---|---|---:|---|
| `amTuneSetFadeScaled` | Exact 29-word instruction/opcode schedule, frame, and relocation surface; 7 register-only differences after the flag lattice and 10 source-shape attempts | function `+0x1C` | IDO 5.3 temporary-FIFO phase: the target and candidate assign the three initial address/index temporaries from different positions in the same ring. The candidate remains under `NON_MATCHING`; canonical output is still assembly-backed |
| `func_800033B0` | Exact 52-word length, `0x40` frame, and relocation surface in the best Mickey-derived echo-surface candidate after the 119-combination flag lattice, 10 source/layout hypotheses, and a 10-minute permuter batch; 10 positional words differ | function `+0x20` | IDO 5.3 stack-home placement and entry scheduling: the target homes the closest positive height delta at `sp+0x2C` with two intervening words before the closest-surface pointer at `sp+0x38`, while the coherent candidate homes the delta at `sp+0x34`; the target also loads the saved values before the empty-result branch and forms the surface pointer after the FP addition. JFG's ordered `amSndSetEcho` peer is assembly-only; canonical output remains assembly-backed |
| `func_80003480` | Exact 94-word length in the best per-lane entry-update candidate after the 119-combination flag sweep and 10 typed/raw, loop, and array-layout hypotheses; 74 words still differ, with the candidate using a `0x40` frame instead of the target's `0x30` | function `+0x0` | IDO 5.3 web formation and spill placement: the target recomputes the lane base and spills the replacement index at `sp+0x24`, while the candidate retains the scaled lane and spills at `sp+0x38`. JFG's ordered peer is assembly-only and retains a placeholder name; canonical output remains assembly-backed |
| `func_80003760` | Exact 25-word opcode schedule, relocation surface, and temp-FIFO lane under `-Wo,-loopunroll,0`; 8 register-only words remain after the flag lattice and 10 source/web hypotheses | function `+0x4` | IDO 5.3 pool ordering: the target assigns the lane count/index to `a2`/`a1` and emits the comparison through `at`, while every coherent candidate basin assigns `a1`/`a2` and a final temp. The donor peer is assembly-only; canonical output remains assembly-backed and the TU's verified flags are unchanged |

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
| `0x59AF4` / `0x80058EF4` | `0x90` + `0x0C` padding | `func_80058EF4` | D: local logarithm-series helper used to derive Doppler pitch |
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

The adjacent pitch-event helper `func_8005CDAC` is adapted from the permitted
BK/PD sound-player implementations identified in its source provenance note.
A 16-byte raw event footprint and integer pitch-bit copy reproduce all 31
target instructions, the `0x30` frame, call relocations, and linked owned
range under bare `-g -mips2 -32`. It adds `0x7C` exact bytes, bringing exact C
in `main/gsSnd` to `0x10A0` bytes.

The permitted-PD-derived event dispatcher `func_8005BA40` reaches all 1,215
target instruction words under the measured bare `-g -mips2 -32` group, with
the target frame and register allocation, but is not promoted or credited.
Its switch and diagnostics emit a `0x150`-byte rodata section that is still
owned by the shared `0x81590` yaml slice; compiling both copies prevents an
exact canonical link. Promotion therefore requires a measured rodata-boundary
handoff in `mickey.us.yaml`, outside this lane's assigned files. The exact-text
candidate remains under `NON_MATCHING` and target assembly stays canonical.

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
live ranges. A new pointer-induction spelling brings `func_8005AAC0` to the
target's 46 instructions with 14 differing words, first at `+0x40`; its cache
temporaries rotate by one register and its selected-index spill lands at
`0x1C(sp)` instead of `0x18(sp)`. The flag lattice and bounded permutation do
not repair that FIFO. `func_8005ABA8` emits 110 against 111 and first diverges
at `+0x38` before an FP-allocation cascade. `func_8005AF14` remains a structural
plateau because neither Mickey's current types nor JFG's assembly-only peer
establish its model-node and attachment layouts.

In `main/vehicle_sounds`, the Mickey-derived handle cleanup loop
`func_800582A8` (`0x64` bytes) is exact under `-O2 -mips2 -32`; its linked
function bytes and call relocation match.

The Mickey-derived logarithm-series helper `func_80058EF4` is exact under
`-O2 -mips2 -32 -Wab,-r4300_mul`. A named loop-invariant square reproduces
the target FP lifetime coloring, and a direct integer-constant multiplication
reproduces its return-register coalescing. All 36 executable words and the
`D_80084318` relocation pair match; the following `0x0C` bytes are TU padding
and receive no credit. Exact executable C in `main/vehicle_sounds` now totals
`0xF4` bytes.

The remaining vehicle functions plateau without exact credit.
`func_80058250`'s best named-global initializer emits 26 instructions against
22 and differs in 19 positional words from `+0x0`; a typed four-slot aggregate
reaches the exact size but differs in 21 positions. The complete flag lattice
does not produce the target's mixed global-address schedule. A new per-slot
volatile aggregate spelling also reaches 22 words and improves that basin to
20 positional differences from `+0x0`, but still hoists four full extern
addresses and carries only 8 relocation records against the target assembly's
20. Statement reordering, pointer-relative stores, one array aggregate, and
volatile/non-volatile per-slot objects cover the coherent extern-layout
families; reproducing the target now appears to require the original data
ownership/layout context rather than another initializer ordering.
A split-tail retry modeled each handle as a scalar and its adjacent float and
object pointer as a two-field aggregate, preserving two source-level bases per
slot. IDO materialised each tail pointer explicitly: the candidate remained 26
words against 22, differed in all 22 target positions from `+0x8`, and carried
16 relocation records rather than 20. The full 119-configuration lattice kept
stock `-O2 -mips2 -32` best, so the original data-ownership blocker remains.
`func_8005830C` now has a complete typed `NON_MATCHING` reconstruction adapted
at the organization/terminology level from DKR's permitted published
`src/audio_vehicle.c`, with Mickey's own field offsets and calls deciding the
body. Ten coherent source shapes and the complete flag lattice leave the best
stock `-O2 -mips2 -32` candidate at 757 instructions against 762, a `0x110`
frame against `0x118`, 706 positional word differences and first mismatch
`+0x0`. Pointer induction improved the residual by 29 words. A later
loop-invariant form reached the exact frame and only a two-instruction deficit
but regressed to 730 words, so the retained candidate is still a broad
allocation/structure plateau rather than a permuter-ready near match.
