# The overlay system (docs/modules.md section 5)

Split out of `docs/modules.md` on 2026-08-24; evidence tiers and naming rules are defined in that file, section 1. Section numbers below keep their original 5.x identity so existing references resolve.

## 5. The overlay system

### 5.1 What runs it

The resident segment carries a complete Rare/DKR-lineage runtime linker at ROM
`0x323E0`–`0x33FA0`, plus its trampoline at `0x33FA0`. Fourteen of its functions are
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
image. Its C now reproduces all four instructions and its `D_80082410`
relocation exactly under the resident `-O2 -mips2 -32` flags.

`runlinkResumeAll` (`0x80032FE0`, `0x60` bytes) scans the sixteen pending-load
slots and resumes every entry not marked `0xFFB`. Its name and role follow
JFG's ordered `runlinkResumeAll` peer (tier B); the adapted C compiles to all
24 Mickey instruction words with the exact data/call relocation surface under
the resident `-O2 -mips2 -32` flags, and its linked ROM range is byte-identical.

`runlinkFlushModules` (`0x80032820`, `0xAC` bytes) unloads resident overlays in
descending order, then frees and clears all sixteen pending-load slots. Its
name, position, and control flow follow JFG's `runlinkFlushModules` peer (tier
B); the adapted C compiles to all 43 Mickey instruction words with the exact
global and call relocation surface under the resident `-O2 -mips2 -32` flags,
and its linked ROM range is byte-identical.

`runlinkTick` (`0x80033090`, `0xBC` bytes) walks the packed link-slot table
backwards when the resident enable flag is set. It ages both counters and calls
`runlinkFreeCode` when the upper ten-bit counter reaches zero. Its name, role, and
control flow follow JFG's 0xC0-byte `runlinkTick` peer (tier B); Mickey's mips2
build omits JFG's load-delay nops. The C compiles to all 47 instruction words
with the exact three-global/one-call relocation surface, and its linked ROM
range is byte-identical under `-O2 -mips2 -32`.

`runlinkSuspendCode` (`0x80032B14`, `0xE4` bytes) claims a free pending-load
slot, records the overlay's resident address, unloads it, and reserves its old
address range for a later resume. Its name, position, and control flow follow
JFG's 0xE8-byte peer (tier B); Mickey uses allocation tag `0x83` and its mips2
build omits JFG's load-delay no-op. The adapted C compiles to all 57
instruction words with the exact relocation surface, and the linked ROM range
is byte-identical under `-O2 -mips2 -32`.

`runlinkUnloadOverlay` (`0x80032618`, `0x208` bytes) frees a resident overlay
or cancels its pending allocation, clears the packed link-slot state, and
rewrites resident references to the dangling-jump trap. Its name, role, and
body follow JFG's published peer (tier B), adapted to Mickey's relocation and
section layouts. The C compiles to all 130 instruction words with the exact
relocation surface, and its linked ROM range is byte-identical under
`-O2 -mips2 -32`.

`ProcessRelocationEntry` (`0x80031A30`, `0x248` bytes) remains
`NON_MATCHING`. A fresh 119-combination flag sweep and an explicit
`-O2 -g0 -mips2 -32` schedule probe both retain the stock 147-instruction
candidate against the target's 146, with 126 positional word differences from
`+0x0` and a `0x48` frame against `0x40`; `-O2 -g3` regresses to 128 words.
The candidate continues to promote `patchLocation` to `s1`, adding its
save/restore pair, while the target caller-saves that value around
`ResolveRelocAddress` and reserves `s0` only for `relocEntry`. The remaining
untried family is the documented pool-position levers 8-13, not another flag
or branch-shape permutation.

`runlinkFreeCode` remains `NON_MATCHING`: 117 words differ, 183/184 instructions, frame -104 versus -88, first `+0x0`.
Levers covered cached base, declaration/register, relocation lifetime, flags, and bounded permutation variants.
Remaining: frame/home excess, relocation bindings, and patch-loop schedule.

`tier-B runlinkResumeCode`: 6 stack operands remain, first `+0x0`; 250 instructions, opcode schedule, registers, and relocations are exact.
Workbench frame-layout; stack-home levers 26/32, frame-local variants, flag lattice, and bounded permutation did not alter the frame.
The target reserves `0x50` with pendingLoad at `sp+0x44`; the candidate reserves `0x48` with the home at `sp+0x40`.

`runlinkInit` remains `NON_MATCHING` after seven coherent source variants, the
119-combination flag lattice, and a bounded permuter pass. The best adapted
JFG candidate is 142 instructions against 146, with a `0x40` frame against
`0x38`, 64 masked positional differences, and its first mismatch at `+0x8`.
The three allocation-and-copy sequences agree apart from frame-relative local
homes; the four-instruction deficit comes later, where IDO reuses two resident
section-anchor addresses that Mickey rematerializes. The permuter found lower
numeric scores only by introducing the wrong relocation identities, so those
variants were rejected rather than promoted.

| Function | ROM | Bytes | Flags | Donor and verdict |
|---|---:|---:|---|---|
| `runlinkDownloadCode` | `0x32878` | `0x478` | `-O2 -mips2 -32` | JFG `src/runLink.c`; 286/286 instruction words and all relocations exact, linked ROM byte-identical |

`runlinkEnsureJumpIsValid` (`0x800320F0`, `0x194` bytes) also remains
`NON_MATCHING` after a bounded ten-attempt pass. JFG's 0x1A8-byte peer is the
nearest skeleton at 0.504 similarity; Mickey's mips2 target omits its five
load-delay no-ops. The best coherent C has the exact 101-word boundary and
selector CFG under `-O2 -mips2 -32`, but 35 masked words differ, beginning at
`+0x20`, where IDO keeps the jump address in `a3` instead of the target's
`s0`; the resulting long-lived-register allocation differs through the loop.
The complete 119-combination flag lattice found no better flag group. A
bounded ten-minute, two-thread permuter batch improved its internal score from
230 to 125 but did not reach zero; its best result added a constant-true block
and was retained only as an ignored diagnostic artifact, not canonical source.

`runlinkGetAddressInfo` (`0x800331E4`) is an exact 108-word match under
`-O2 -mips2 -32`; its three following nop words are alignment padding before
the separately split trap TU. The body donor is JFG's public runlink assembly.

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
| 106 | `0x000`–`0x008` | 8 | tier-A `osRamTest3_6105`; complete text matches JFG o144, with eight padding bytes excluded |
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
  remain raw. Exact C now owns the contiguous `+0x000..+0xB84` prefix and
  `+0x1578..+0x1A84`; the
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

#### Retracted postcompile overlay promotions

Two additional public exact claims fail canonical reproof. Overlay 34
`+0x40C..+0x540` compiles to 80 instructions / `0x140` with frame `0x38`,
versus the 77-instruction / `0x134` retail owner with frame `0x30`; the former
wrapper deleted three instructions, reordered another, and edited fields.
Overlay 41 `+0x1740..+0x195C` is exact-sized at 135 instructions and matches
133/135, but its former postprocess changed retained-rodata LO16 fields at
`+0x80` and `+0x12C`. Both now retain their natural C under `NON_MATCHING` and
use byte-exact assembly fallbacks.

Overlay 98's `+0x144..+0x234` collector and `+0x848..+0xA04` checker were
reproved the same way. The collector is exact-sized but 48/60 linked words
with frame `0x58` versus `0x50`; the checker is exact-sized but 78/111 linked
words with frame `0x80` versus `0xA8`. Their former normalizers rewrote frames,
control flow, registers, and instruction sequences. Both claims are withdrawn,
their corrected pointer ABI remains in candidate C, and assembly is canonical.

The `+0x764..+0x1158` renderer remains `NON_MATCHING`. Its identity-correct
candidate is exact-sized at 637 words, matches 634/637 positional words, keeps
the retail `0x88` frame, and emits all 24 runtime relocation offsets and types.
The three residuals are retained-constant LO16 sites at `+0x160`, `+0x168`,
and `+0x174`; direct, cached, and split-object source forms regress. An earlier
public promotion reached equality by externalizing those instruction fields
and collapsing distinct call identities, so it has been withdrawn. The
assembly fallback is canonical until ordinary source reproduces the bytes and
all local identities/addends can be proved.

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
| 40 | `+0x2E4..+0x690` | 940 | timed interpolation is semantically related to DKR weather shifting; the tint rectangle matched through SDK display-list macro scoping, with no exact donor; `+0x0E8` remains assembly after a two-instruction startup scheduling mismatch |
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
**94,996 / 949,944 (10.00%)**, one exact byte beyond the integer 94,995-byte
hard floor.

The final 488-byte step is independently linked and compared at each real
overlay offset:

| Overlay range | Role | Bytes |
|---|---|---:|
| 1 `+0x1CA4..+0x1D58` | release the record array, secondary allocation, and final handle | 180 |
| 1 `+0x7B64..+0x7BDC` | select the best matching fixed record | 120 |
| 61 `+0x19B0..+0x1A6C` | choose the first unused controller-pak filename extension | 188 |
| **final step** | | **488** |

Overlay 61's controller-pak selector (`+0x19B0..+0x1A6C`, 188 bytes) requires
its measured per-object `-Wab,-r4300_mul` schedule and stays exact C, as does
overlay 1's release loop (`+0x1CA4..+0x1D58`, 180 bytes), natural exact after
relocation resolution. Overlay 1's fixed-record selector (`+0x7B64..+0x7BDC`)
— 120 bytes. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match
via a twelve-word register-coloring reassignment, IDO's interchangeable
`a1`/`a3` choice restored to retail's); source kept as decomp-permuter input.

Overlays 74, 77, 85, and 97 are now entirely exact C apart from proven
alignment padding. Together with the earlier overlay 39 and 95 closures, this
satisfies the Epoch 5 cohort exit at **6 / 6**. The regenerated donor ledger
covers 107 overlays against pinned DKR v77/v80 and JFG and remains negative
for the final three clusters. A bounded `gmake -j2` rebuild is byte-identical
to the US baserom with SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`.

### 5.15 Epoch 11 execution checkpoint

Epoch 11 opens with three exact bodies. Overlay 21's remaining plane-side
priority routine at `+0x10C..+0x2D4` contributes **456 bytes / 114 words**.
The measured `-Wab,-r4300_mul` flag reproduces its FP schedule, while a
redundant comparison temporary preserves IDO's shipped caller-saved allocation.
The resulting object has the exact 0x28-byte frame, all nine relocation records,
the linked overlay bytes, and the whole-ROM hash. Pinned DKR v77/v80 and JFG
scans remain exact-negative for the body. The following `+0x2D4..+0x2E0` is 12
bytes of alignment padding and receives no C credit.

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

Overlay 31's buffer initializer at `+0x6B0..+0xA84` adds **980 bytes / 245
words**. Keeping `assetBuffer` after the three integer locals preserves IDO's
0x48-byte frame without changing the DKR-derived control flow. The compiled C
reproduces all 16 calls and 19 HI16/LO16 pairs (54 records total), the linked
overlay range, and the whole-ROM hash. The owned range has no padding.

Overlay 37's object updater at `+0x088..+0x19C` — 276 bytes / 69 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
sixteen-word parameter-home/resource-register web normalization and an
operand commute on a finite sine-result multiply); source kept as
decomp-permuter input. Pinned donor scans are exact-negative for this body.

Overlay 40's duplicate-removing table scan at `+0x084..+0x0E8` contributes
**100 bytes**. The source reproduces all 25 words and both relocated global
address pairs naturally. Its second formal is overwritten from the meaningful
identifier before use, an ABI-neutral source shape that selects the shipped
saved-identifier register web; the function remains a frame-free, call-free
leaf and continues scanning after a hit by design.

Overlay 40's add-entry scan at `+0x000..+0x084` — 132 bytes / 33 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
four-word loop-counter register reassignment between equal `v1`/`a0` copies);
source kept as decomp-permuter input.

Overlay 40's fade-record owner at `+0x690..+0x824` remains `NON_MATCHING` at
404 bytes / 101 words; `+0x824..+0x830` is separate padding. Its bounded best
C object is 98/101 words with the exact `0x8` frame and all ten runtime BSS
records. Only `+0xC/+0x10/+0x24` differ, as one `v0`/`v1` globalcolor outcome.
An allocator trace and all 119 flag combinations found no exact object;
canonical `-O2 -mips2` ties for best. Linked range/module/full-ROM equality
continues to prove the assembly fallback only.

Overlay 36's final-effect callback at `+0x1688..+0x1748` — 192 bytes / 48
words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
two-word private stack-tail shrink, eight bytes); source kept as
decomp-permuter input. The following `+0x1748..+0x1750` is padding and
receives no C credit.

Overlay 46's release/synchronization leaf at `+0x614..+0x69C` — 136 bytes / 34
words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via an
eight-word pointer-register reassignment, two nullable resource pointers moved
from `v0` to `a0` at their call delay slots); source kept as decomp-permuter
input.

Overlay 14's packed-rectangle builder at `+0x12D8..+0x13F4` — 284 bytes / 71
words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
two-word private stack-offset shift, the 24-entry rectangle array moved from
IDO's `sp+0x28` placement to the shipped `sp+0x24`); source kept as
decomp-permuter input.

Overlay 46's state initializer at `+0x000..+0x120` contributes **288 bytes**.
All 72 words are natural after the eleven fixed overlay-local addends and the
genuine `D_184` relocation resolve. The configure helper's third formal is
`f32`; passing `0.0f` selects the shipped zero materialization. The 64-byte
frame, eight calls, descriptor construction, nullable result update, address
pairs, branches, and effects are exact without code-word correction.

Overlay 91's render wrapper at `+0x4BC..+0x574` contributes **184 bytes**.
The first source basin reproduces all 46 words naturally, including six
resident relocation calls, temporary buffer and width outputs, the centered
render band, alpha initialization, and the flag clear/restore around the final
object draw. The trailing `+0x574..+0x580` is padding and receives no C
credit.

Overlay 46's particle initializer at `+0x69C..+0x874` — 472 bytes / 118 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
two-word swap of adjacent, unconditional initialization order between the
independent resource-table-base and loop-counter registers); source kept as
decomp-permuter input. Walks nineteen 60-byte records and their four-byte
configurations, assigning randomized angles/offsets, fixed scale, copied
target coordinates, table-selected resources, and final state/timer values.

Overlay 94's controller initializer at `+0x000..+0x110` — 272 bytes / 68
words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
four-word private frame-tail restoration, eight bytes, plus a one-word
or-zero/addiu-zero substitution); source kept as decomp-permuter input.

Overlay 80's contact initializer at `+0x000..+0x11C` — 284 bytes / 71 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
one-word operand commute on the finite sources of the final multiply); source
kept as decomp-permuter input. The measured `-Wab,-r4300_mul` assembler mode
naturally inserts the shipped FP hazard no-op and places both call relocations
at their exact offsets.

Overlay 80's contact updater at `+0x11C..+0x3EC` — 720 bytes / 180 words.
Exact C: a bounded annotated-target permutation found three redundant pointer
and float aliases that naturally give IDO the shipped `0x80` frame and register
allocation. Under `-Wab,-r4300_mul`, the final source reproduces all 180 words,
all 20 relocation records, the linked module, and the full ROM. The aliases are
disclosed in source for a future readability pass. Together the two contact
bodies cover all **1,004 executable bytes** in overlay 80; the final
`+0x3EC..+0x3F0` remains four bytes of explicit padding and receives no C
credit.

Overlay 10's sole initializer at `+0x000..+0x2B0` — 688 bytes / 172 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via
thirty-seven word assertions covering five register/frame webs: private
frame/output homes, a width/height permutation, two base/end completion
swaps, the entry-loop register cycle, and two outer-loop initializations);
source kept as decomp-permuter input. Closes the module with no padding; 135
of 172 words were natural, including all eleven call relocations and fifteen
resident-data address pairs. Previously credited as **3 / 8** Epoch 11
closures.

Overlay 23's `+0x468..+0x568` render helper — 256 bytes / 64 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via
thirty-three word assertions covering three register webs: state-pointer
permutation, the call-free packet/display-list schedule, and final-call
argument registers); source kept as decomp-permuter input. The final
`+0x568..+0x570` remains eight bytes of explicit padding.

Overlay 31's pool allocator at `+0xE7C..+0xF44` — 200 bytes / 50 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via fourteen
word assertions covering the unused eight-byte private-frame tail and a
call-free record-pointer/outer-count register web); source kept as
decomp-permuter input. The following `+0xF44..+0xF50` remains 12 bytes of
explicit padding.

Overlay 50's `+0x1C54..+0x1E68` formatter contributes **532 exact C bytes**,
preserving the separate eight-byte padding tail. Overlay 19's `+0xC1C..+0xD78`
adjacency search contributes **348 exact C bytes** and one exact local
`R_MIPS_26` call to the `+0xD78` classifier; its configured object is 87/87
words. Both splits survive atlas regeneration, the donor scan, full link, and
direct ROM comparison.

Overlay 96's `+0x5C8..+0x6D4` object renderer contributes **268 exact C
bytes**. All 67 configured instruction words and the call plus local-data
HI/LO relocation tuples are exact; the compiler alignment word is trimmed at
the `0x10C` symbol boundary.

Overlay 44's `+0x000..+0x224` animation-state constructor contributes **548
exact C bytes** and crosses Epoch 11's 8,000-byte Milestone A gate. All 137
configured instruction words are exact, with nine runtime relocation sites
(two data HI/LO pairs and five calls) reconciled against the retained runtime
census.

Overlay 8 gained seven exact C islands. The motion starter at
`+0x0E88..+0x0F1C` contributes **148 bytes / 37 words**; the activation path
at `+0x0F1C..+0x1000` contributes **228 bytes / 57 words**; the color
applicator at `+0x3278..+0x3368` contributes **240 bytes / 60 words** — all
naturally exact with their call relocations reconciled against retained atlas
splits.

The `+0x2EC0..+0x3018` child updater adds **344 bytes / 86 words**, naturally
exact: 28 FP-touching instructions, ordered branches, a signed post-decrement
loop, six LOCAL data relocations (all resolving to module-local `+0x73B0`),
and a terminal resident-call identity.

Overlay 8's channel updater beginning at `+0x3018` — 608 bytes / 152 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
fail-loud data-literal externalization moving paired MIPS LO16 addends to an
existing anchor to avoid a duplicate 32-byte literal pool at overlay-local
`+0x1BC`); source kept as decomp-permuter input. All 16 retail relocation
identities were otherwise preserved through the exact linked image.

Overlay 8's scale-output body at `+0x3368..+0x34A0` contributes **312 exact C
bytes / 78 words**. Loading the lower threshold through the upper local before
overwriting it preserves IDO's retail copy-coalescing order. All four local
relocation tuples, the linked owned range, the complete overlay, and the US ROM
are byte-identical.

Overlay 8's `overlay8UpdateChannels` at `+0x3018..+0x3278` remains guarded
`NON_MATCHING` and contributes no C credit. Its stale public-only exact claim
depended on rewriting five LO16 instruction fields after compilation, which is
prohibited by ADR 0002; the canonical build therefore uses the assembly
fallback until untouched compiler output proves all 608 bytes.

The motion-output body at decimal overlay offsets `+18,920..+19,696` adds
**776 bytes / 194 words**. The measured `-Wab,-r4300_mul` object naturally
reproduces its complete text, including its R4300 scheduler no-op; the
private `0.04f` literal is externalized to decimal overlay-local offset
`+652` so the retained data payload remains the sole byte owner. The direct
ROM slice and cumulative linked ROM are exact.

Overlay 24's updater at `+0x001C..+0x0284` — 616 bytes / 154 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
fail-loud ELF normalization externalizing a duplicate compiler literal
section already owned by the retained data payload); source kept as
decomp-permuter input. Overlay 24's renderer at `+0x0284..+0x0414` adds **400
bytes / 100 words**, naturally exact, beside the already-matched 28-byte
initializer. The three-word tail at `+0x0414..+0x0420` remains separate
assembly padding. This had completed overlay 24's closure under the retired
scheme.

Overlay 43's child submitter at `+0x1264..+0x1378` — 276 bytes / 69 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
fail-loud four-word `s2`/`s1` owner-web normalization); source kept as
decomp-permuter input. Natural source was otherwise exact in size, frame, and
opcode/CFG sequence, with both local call relocations exact.

Overlay 62's decimal overlay range `+212..+1,388`, between the existing
initializer and release routine — 1,176 bytes / 294 words. NON_MATCHING: the
configured C baseline preserves the target's 294-instruction shape and 0x88-byte
frame but differs in four register/opcode words; its relocation surface has 71
records versus the target's 29. Seven bounded source-faithful probes did not
change that result, and no instrumented globalcolor/UGEN trace is available.
The assembly fallback remains canonical pending a source spelling with the
target allocation and relocation surface.

Overlay 84's current-record activation body at decimal overlay offsets
`+4,192..+4,596` — 404 bytes / 101 words. NON_MATCHING: retired 2026-08-24 per
ADR 0002 (was made to match via nine whole-word assertions selecting the
retail private-frame and equal selected-value homes); source kept as
decomp-permuter input. The configured source otherwise retained the exact
root-object address pair, three distinct call identities, selector
branch-likely behavior, and FP/copy schedule. The direct ROM slice at file
offset 26,023,232 has SHA256
`87ac5da55e8b23ea2a9f42a737c478716c2b762b1f792a3423dc6b255198ed24`
in both images, and the cumulative full ROM is exact.

Overlay 68's kind classifier at decimal overlay offsets `+5,228..+5,548` —
320 bytes / 80 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made
to match via eighteen guarded words normalizing private frame, stack-slot,
and register-allocation webs); source kept as decomp-permuter input. Direct
ROM slices from file offset `0x18C85CC` share SHA256
`8a7b237e640d76a01166f3d9b862b59a4ccf03fa576eb6be6b3be9306e90a8c7`,
and the cumulative full ROM is exact.

Overlay 1's type-five keyed search at decimal overlay offsets `+888..+1,044`
— 156 bytes / 39 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was
made to match via seventeen guarded words selecting one bijective
temporary-register web); source kept as decomp-permuter input. Direct ROM
slices at file offset `0x184C758` share SHA256
`daeb9395211c01871e6c40bafdf49a8187ac111a96855d1ed62d05ca5e80271d`,
and the cumulative full ROM is exact.

| Overlay | Range | Function | Bytes | Exactness | Donor |
|---:|---|---|---:|---|---|
| 1 | `+0x0050..+0x0080` | `overlay1GetEntry` | 48 | canonical object and linked ROM exact | pinned DKR/JFG scans negative |
| 14 | `+0x0328..+0x0498` | `overlay14ApplyValues` | 368 | canonical object and linked ROM exact | Mickey-only |
| 14 | `+0x0D68..+0x0F64` | `overlay14AdvanceCommand` | 508 | canonical object and linked ROM exact | Mickey-only |
| 14 | `+0x0F64..+0x1028` | `overlay14StepCommand` | 196 | canonical object and linked ROM exact | Mickey-only |
| 14 | `+0x13F4..+0x1540` | `func_overlay_014_F00013F4_1870CCC` | 332 | canonical object and linked ROM exact | Mickey-only |
| 43 | `+0x0194..+0x0280` | `func_overlay_043_F0000194_188A164` | 236 | canonical object and linked ROM exact | Mickey-only |
| 70 | `+0x0000..+0x00D8` | `func_overlay_070_F0000000_18C91C8` | 216 | canonical object and linked ROM exact | Mickey-only |

Overlay 1's backward usable-record search at decimal offsets `+1,044..+1,204`
— 160 bytes / 40 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was
made to match via twelve guarded words selecting the retail preheader and
temporary-register webs); source kept as decomp-permuter input. Direct ROM
slices at file offset `0x184C7F4` share SHA256
`837c7fcf2d1ee011313f664b42f663b8bca7658473c801b697e24a81031461ff`.

Overlay 82's newly matched updater at `+0x0040..+0x0498` contributes **1,112
bytes / 278 words** with an exact normalized natural object, 26 retained
relocation sites, and separate four-byte tail padding; two call proxies and
three input-data proxies preserve the runtime relocation placeholders. This
made overlay 82 fully C-owned across its 1,228 executable bytes. Direct ROM
slices from file offset `0x18CF1C0` share SHA256
`38fbefda01c642125d3f1badef620054a90760e461f494741c370f660c65aad2`,
and the cumulative ROM is exact, credited as the sixth Epoch closure.

Overlay 45's layout configurator at decimal offsets `+788..+1,600` adds **812
bytes / 203 words**. Its natural object reproduces the complete 0x80-frame
code body, five calls, fourteen data-address relocations, twelve FP
instructions, stream loop, likely branches, and glyph update order with no
word correction; three call proxies and five data proxies retain the shipped
runtime relocation identities. Direct ROM slices from file offset `0x188C76C`
share SHA256
`0eb13f9257a0d760179622fb1929659d758b435121ba7c3f3722dc9a015766b2`,
and the cumulative ROM is exact.

Overlay 23's `+0x000..+0x208` attachment spawner adds **520 naturally exact C
bytes / 130 words** with its `0xA0` frame and two call relocations exact.
With the existing C islands and separate eight-byte tail padding, this had
made all executable text C-owned, credited as the seventh Epoch 11 closure.

Overlay 19's `+0xA30..+0xC1C` adjacency builder — 492 bytes / 123 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via 41
guarded register-only word selections); source kept as decomp-permuter
input. The natural object was otherwise topology-, frame-, and
relocation-exact.

Overlay 19's `+0xD78..+0xF58` edge classifier — 480 bytes / 120 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via guarded
normalization of four independent two-load schedules and one six-use
temporary web); source kept as decomp-permuter input. The natural object was
size-, opcode-, CFG-, memory-, return-, and likely-branch-exact at 106/120
words.

Overlay 42's `+0x0F4..+0x6A4` captured-buffer renderer adds **1,456 naturally
exact C bytes / 364 words**. Its historical display-list macro spelling
emits the exact `0xD0` frame, seven calls, nine address pairs, every
register and schedule choice, and the complete command stream without word
normalization. With the four existing exact functions, this had made
overlay 42's full `+0x000..+0x700` executable text C-owned, credited as the
eighth Epoch 11 closure.

Overlay 19's `+0xF58..+0x12E4` spatial-mask builder — 908 bytes / 227 words,
with twelve following padding bytes retained separately in assembly.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
SHA-anchored normalization selecting two register webs and four independent
schedule pairs); source kept as decomp-permuter input. Its minimized source
naturally matched 164/227 words and the exact frame, CFG, memory effects, and
zero-relocation contract.

Overlay 58's rank-set refresh — 608 bytes / 152 words. NON_MATCHING: retired
2026-08-24 per ADR 0002 (was made to match via a SHA-anchored decoded-field
normalization selecting two interchangeable private temporary webs); source
kept as decomp-permuter input. The natural object was 141/152 with exact
topology, opcodes, memory effects, and local/call relocation roles; this was
volume credit and did not close overlay 58.

The next volume tranche added overlay 57's draw body (**832 bytes**), overlay
58's segment-strip renderer (**804 bytes**), overlay 101's clock renderer
(**952 bytes**), and two overlay 57 mode bodies (**1,132** and **868 bytes**),
each naturally exact with no compiler-only alignment bytes counted.

The byte-identical shared renderers in overlays 69 and 88 add **1,436 bytes
each**. Their objects and six call relocations were validated independently.
Overlay 69's following four-byte padding remains assembly-owned, while
overlay 88 ends directly at its relocation payload. Both complete linked
overlay spans are exact.

Overlay 58's two adjacent point-quad renderers add **416 bytes each**. They
share one reviewed source and normalization shape but keep independent
symbol and payload bindings. Both configured objects, direct slices, and the
full ROM are exact.

Overlay 40's state updater at `+0x0E8..+0x1A0` — 184 bytes / 46 words,
creating a 416-byte contiguous prefix. NON_MATCHING: retired 2026-08-24 per
ADR 0002 (was made to match via a guarded permutation moving a dead
incoming-argument precolor into rejected alignment plus one address schedule
fix); source kept as decomp-permuter input. The exact four runtime relocation
records with separate entry-table and object-table symbol identities were
otherwise preserved; the two later assembly regions remain unresolved.

Overlay 30's `+0x2B4..+0x438` byte-plane transposer — 388 bytes / 97 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
guarded schedule and decoded field ledger preserving the opcode census,
counted loop, memory behavior, and zero-relocation surface); source kept as
decomp-permuter input. The separate `+0x438..+0x440` padding remains
assembly-owned; with the existing initializer this had made all overlay 30
text exact C, credited as the ninth Epoch 11 overlay closure.

Overlay 1's `+0x7B0..+0xBD4` point-record builder adds **1,060 naturally
exact C bytes / 265 words**. The configured four-way-unrolled R4300-scheduled
object matches SHA256
`f1c43d4bed886287e2cdade26036351d04985a00eb7ae3f0d2456f431aaa1d85`.
Its two calls retain distinct runtime relocation identities at `+0x1C` and
`+0x2C`; neighboring assembly resumes exactly at `+0xBD4`.

Overlay 84's `+0xC9C..+0xDBC` current-resource loader — 288 bytes / 72
words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
decoded private allocator/spill ledger selecting the retail GPR webs and
unused spill slot); source kept as decomp-permuter input. Natural source
otherwise supplied the complete frame, CFG, opcode/call/FP schedule, memory
effects, and five relocation sites.

Overlay 59's `+0x070..+0x168` entry preparer — 248 bytes / 62 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
nine-word decoded ledger selecting retail's equivalent descriptor-value/
call-argument web); source kept as decomp-permuter input. Natural source
otherwise supplied the exact boundary, frame, CFG, memory effects, four
calls, and six relocation records.

Overlay 48's `+0x144..+0x40C` state updater — 712 bytes / 178 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
schedule permutation moving one argument materialization to the retail slot
and five branch-displacement updates); source kept as decomp-permuter input.
Natural source otherwise supplied the exact boundary, `0x38` frame, and all
22 runtime relocation records. The independent `+0x060..+0x144` body remains
assembly-owned, so this was volume credit rather than a closure;
`+0x46C..+0x470` remains explicit padding.

Overlay 101's transformed-object renderer — 664 bytes / 166 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via two
command-schedule permutations and decoded private temporary webs selecting
retail's equivalent allocation); source kept as decomp-permuter input.
Natural source otherwise supplied the exact `0x90` frame, ABI, seven-call
layout, CFG, memory/stack effects, and FP topology.

Overlay 59's `+0x36C..+0x784` six-state advancer — 1,048 bytes / 262 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
prologue permutation plus asserted retained-data addends selecting the
retail schedule, discarding a duplicate compiler jump table and
relocations); source kept as decomp-permuter input. Natural source otherwise
supplied the exact `0x58` frame, CFG, loops, branch-likely forms, every
integer/FP register web, and all calls and memory effects. This had made all
overlay 59 text exact C, credited as closure ten.

Overlay 68's secondary-entry promoter — 308 bytes / 77 words. NON_MATCHING:
retired 2026-08-24 per ADR 0002 (was made to match via a fail-loud private
frame, call-survival, primary-pointer, and null-exit ledger selecting
retail's equivalent allocation and branch-likely form); source kept as
decomp-permuter input. Natural source otherwise supplied the exact boundary,
call order, copy loop, memory effects, and nine runtime relocation sites.

Overlay 68's following interpolation body — 656 bytes / 164 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via
fail-loud symbol metadata, schedule, and decoded private register ledgers
proving and selecting the full retail ownership unit); source kept as
decomp-permuter input. Natural source otherwise supplied the complete
semantic body and three zero alignment words.

Overlay 68's secondary-resource rebuild — 488 bytes / 122 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
fail-loud private stack/register ledger selecting retail's equivalent
homes); source kept as decomp-permuter input. Natural source otherwise
supplied the exact frame, opcode schedule, map walk, probe selection, nine
calls, and all 19 relocation offsets.

Overlay 66's RGB5551 smoothing-and-draw body at `+0x040..+0x4E0` — 1,184
bytes / 296 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to
match via a fail-loud instruction-schedule and private register ledger
selecting retail's equivalent allocation and three proved local symbol
addends); source kept as decomp-permuter input. Natural source otherwise
supplied the exact frame, boundary, opcode inventory, CFG, unrolled
four-pixel filter, and all 28 runtime relocation records. The independent
`+0x4E0..+0x810` helper remains assembly-owned.

Overlay 68's keyframe-animation updater at `+0x96C..+0xEFC` — 1,424 bytes /
356 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match
via a fail-loud decoded-field ledger selecting retail's equivalent private
allocation, plus removal of four duplicate compiler records resolving to the
resident BSS flag); source kept as decomp-permuter input. Natural source
otherwise supplied the exact frame, boundary, eleven calls, keyframe loop,
interpolation/direction CFG, and runtime relocation sites.
`+0xEFC..+0x1250` remains separately assembly-owned.

Overlay 69's anchor updater at `+0x04C..+0x170` adds **292 exact C bytes / 73
words** and removes its final executable assembly gap. Natural source
supplies the exact `0x50` frame, branch-likely gate, store order, four call
sites, and all instruction words; the shipped overlay relocation table
proves the four distinct resident call identities. The independent
`+0x70C..+0x710` alignment word remains assembly-owned, credited as closure
eleven.

Overlay 18's state reconfiguration body at `+0x24C..+0x4F4` — 680 bytes / 170
words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
fail-loud ledger removing one redundant compiler copy and selecting three
independent schedules plus private stack and GPR homes); source kept as
decomp-permuter input. Natural source otherwise supplied the exact `0x68`
frame, state transitions, equality-only rate loops, old-velocity integration
order, five semantic calls, and all 64 relocation records. The independent
`+0x000..+0x1F4` and `+0x4F4..+0x650` bodies remain assembly-owned, so this
was volume credit rather than a closure.

Overlay 98's unique-height collector at `+0x000..+0x144` — 324 bytes / 81
words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
fail-loud schedule and decoded-field ledger selecting retail's equivalent
three-base allocation and address materializations); source kept as
decomp-permuter input. Natural source otherwise supplied the complete nested
traversal, flag gate, duplicate scan, signed/unsigned field behavior,
fifteen-entry cap, and exact instruction count. The successor begins exactly
at `+0x144` and remains assembly-owned.

Overlay 89's effect updater at `+0x000..+0x138` — 312 bytes / 78 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
fail-loud ledger selecting the retail private stack/register web and a
four-instruction create-call schedule); source kept as decomp-permuter
input. Natural source otherwise supplied the exact `0x58` frame, opcode and
branch topology, six calls, FP operations, and eight runtime relocation
sites. The unrelated tail at `+0x270` remains assembly-owned, so this was
volume credit rather than a closure.

Overlay 18's buffer initializer at `+0x4F4..+0x650` — 348 bytes / 87 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via two
relocation-aware schedule permutations plus a fail-loud local-addend/
private-spill/GPR ledger selecting the retail layout); source kept as
decomp-permuter input. Natural source otherwise supplied the exact `0x20`
frame, straight-line CFG, opcode inventory, two allocation calls, memory
effects, and all 50 runtime relocation sites and types. Only
`+0x000..+0x1F4` remains assembly-owned, so this was volume credit rather
than a closure.

Overlay 18's startup loader at `+0x000..+0x1F4` — 500 bytes / 125 words, and
removed the module's final executable assembly gap. NON_MATCHING: retired
2026-08-24 per ADR 0002 (was made to match via a fail-loud ledger selecting a
five-instruction result-publication schedule, the retail display-list
register web, and two equal zero materializations); source kept as
decomp-permuter input. Natural source otherwise supplied the exact `0x18`
frame, straight-line control flow, 46 calls, display-list command stream, and
all 60 runtime relocation sites and types. Together with the two already
exact successors, this had made all **1,616 executable bytes** of overlay 18
C-owned, credited as closure thirteen.

Overlay 88's anchor updater at `+0x04C..+0x1A4` adds **344 exact C bytes / 86
words** and removes its final executable assembly gap. Natural source
supplies the exact `0x58` frame, branch-likely delay-slot load, FP and store
schedule, and seven call sites; the shipped overlay relocation table proves
the seven distinct resident roles. Overlay 88 has no executable padding,
credited as closure twelve.

Overlay 68's sorted-entry renderer — 852 bytes / 213 words, and removed its
final executable assembly gap. NON_MATCHING: retired 2026-08-24 per ADR 0002
(was made to match via a relocation-aware permutation moving the prepare
call to its retail site, a peeled adjacent-swap schedule choice, and a
fail-loud decoded-field ledger selecting private frame/register/spill
allocation); source kept as decomp-permuter input. Natural source otherwise
reproduced the exact boundary, CFG, three calls, gathered-entry and
descriptor effects, and stack-array offsets. The only remaining assembly
interval is the explicit four-byte tail padding, credited as closure
fourteen.

Overlay 89's state/particle updater — 544 bytes / 136 words. NON_MATCHING:
retired 2026-08-24 per ADR 0002 (was made to match via a single fail-loud
bijection selecting retail's argument spill, private frame, saved-register
allocation, and instruction schedule); source kept as decomp-permuter input.
Natural source otherwise reproduced the exact instruction count, FP/integer
inventories, constants, structure accesses, and fourteen runtime relocation
roles.

Overlay 89's initializer at `+0x270..+0x5A4` — 820 bytes / 205 words, and
removed the module's final executable assembly gap. NON_MATCHING: retired
2026-08-24 per ADR 0002 (was made to match via a fail-loud schedule and
decoded-field ledger selecting retail's equivalent state-pointer
spill/register web); source kept as decomp-permuter input. Natural source
otherwise supplied the exact `0x58` frame, sole saved register, CFG, three
calls, local scale relocation pair, descriptor construction, state
initialization, and nested color propagation loop. Its raw body SHA256 is
`91a336e39261e09b3690e088760c6bdb0bf39854393b7d119442b9db64239a70`;
the direct linked slice SHA256 is
`22640530500357251ecb92d3d6e719294dfe59b969c8539fa7d5919d6465a024`.
Only the explicit twelve-byte tail padding remains assembly-owned, credited
as closure fifteen.

Overlay 48's state initializer at `+0x060..+0x144` — 228 bytes / 57 words,
and removed that module's final executable assembly gap. NON_MATCHING:
retired 2026-08-24 per ADR 0002 (was made to match via a fail-loud
schedule/field ledger selecting retail's equivalent private register web and
a proved constant boolean); source kept as decomp-permuter input. Natural
source otherwise supplied the exact `0x18` frame, stores, two calls, and
branch-free integer topology. The raw configured body SHA256 is
`312385b63b2beaee48fb7cb069e6737ad4f7a622031d1e50220c157016092ce0`;
the direct linked slice SHA256 is
`927bc336aca97507f1ed258868c5d24a53d8db1eacb9cdcb2a07f53f37d380a5`.
Only the explicit four-byte tail padding remains assembly-owned, credited as
closure sixteen.

Overlay 16's gradient applicator at `+0x1E0..+0x424` — 580 bytes / 145 words,
and removed the module's final executable assembly gap. NON_MATCHING:
retired 2026-08-24 per ADR 0002 (was made to match via two dead-store
deletions plus a fail-loud schedule/field ledger selecting retail's
equivalent frame, saved-register allocation, and branch spelling); source
kept as decomp-permuter input. The typed natural source otherwise had the
exact loops, color arithmetic, memory effects, and six local relocation
sites. Configured-link validation also corrected the module's local storage
names: the accumulator is at `+8` and the mode selector at `+4`, consistently
in both its initializer and this consumer. The raw configured body SHA256 is
`e51a18791518c07f21b505a4105a51c8d4ef354bba38f2444af0ec2562aa78e6`;
the retail-linked slice SHA256 is
`237587594c7077add06692d8498a6b5c71f67130f275dd4719b0d81f0a91a5e2`.
Only the explicit twelve-byte tail padding remains assembly-owned, credited
as closure seventeen.

Overlay 49's initializer and updater at `+0x000..+0x354` — 852 bytes / 213
words, and removed its final executable assembly. NON_MATCHING: retired
2026-08-24 per ADR 0002 (was made to match via a guarded ledger selecting
private allocation/scheduling webs plus removal of one compiler-alignment
copy, and three guarded field/schedule operations on the post-decrement
input loop); source kept as decomp-permuter input. The 500-byte initializer
preserved thirty call/address relocation sites; the 352-byte updater
preserved twenty-three relocations and the shipped post-decrement input loop
including index zero. The two retail body SHA256 values are
`75dbf550918557a9ade8802c46458d379d324e23d1eca9b477ed9c902e564a96`
and
`122d09b0ff67c971b223ce8cee57ea647fa1e8646077794a30be6095569f5bee`.
The exact `refractOutput` tail stays C and only twelve bytes of explicit
padding remain assembly-owned, credited as closure eighteen.

Overlay 37's renderer at `+0x19C..+0x4F4` — 856 bytes / 214 words, and
removed that module's final executable assembly. NON_MATCHING: retired
2026-08-24 per ADR 0002 (was made to match via a fail-loud bijection
selecting the retail frame, private GPR/FPR allocation, branch distances,
and legal schedule); source kept as decomp-permuter input. Natural IDO owned
852 bytes followed by one proved zero alignment word, extended into the
function. All twelve call/local-data relocation sites remain exact. The
configured raw body SHA256 is
`812e309eb30b88554676a6b7a11b8cd823a7426d7666506654bef7db8b6119e5`;
the retail ROM slice SHA256 is
`4b26c018a45c6d4ccd8285ec8570fa279d399e0253b008485be69f74456ce975`.
Only eight bytes of explicit padding remain, credited as closure nineteen.

Overlay 31's configuration allocator at `+0xA84..+0xDC4` — 832 bytes / 208
words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via
eight fail-loud decoded-field assertions selecting four private
representation webs: commutative operand order, one spill home, one
comparison destination, and one producer/use/store temporary); source kept
as decomp-permuter input. Natural output was otherwise already exact in
opcode inventory, CFG, delay slots, `0x50` frame, and its sole overlay-local
call relocation, agreeing at 200 of 208 words. The retail body SHA256 is
`43fe93dec2619d929e2a047471d108014dc9916045bcbbcfab2ea9a323779782`.

This checkpoint had contributed **48,908 / 47,496 (102.97%)** Epoch 11 bytes
and recorded nineteen overlay closures against the eight-closure target under
the retired object-editing scheme; under ADR 0002 the functions listed above
as NON_MATCHING are reclassified pending decomp-permuter matches, and the
closure/byte totals above are historical record rather than current status.
The linked binary from that checkpoint remained byte-identical to the US
baserom at SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548`.

### 5.16 Epoch 12 execution checkpoint

Epoch 12 opens with four consecutive presentation builders in overlay 101.

`overlay 101 +0x99C4..+0x9D04`, `+0x9D04..+0xA044`, `+0xA044..+0xA384` — 832 bytes / 208 words each. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via redundant-instruction deletion at +0x324); source kept as decomp-permuter input. Each preserves 24 HI/LO pairs and four calls (52 relocations).

`overlay 101 +0xA384..+0xA6BC` — 824 bytes / 206 words. Naturally exact at the `0x338` size; no correction needed.

Overlay 33's present-and-swap helper at `+0x066C..+0x0708` — 156 bytes / 39 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via private-register web reassignment); source kept as decomp-permuter input. Declaring the double-buffer index volatile naturally reproduces the target's `0x18` frame, size, opcode schedule, two calls, and all 18 relocation sites. The larger renderer and initializer remain assembly-owned, so this is not a module closure.

Overlay 58's final executable body at `+0x00005554..+0x00005A14` — 1,216 bytes / 304 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via private register/schedule web reassignment); source kept as decomp-permuter input. Retains the complete packed-status state machine, all 24 calls, and all 48 source relocations. The following `+0x00005A14..+0x00005A20` nop island remains assembly-owned padding.

Overlay 101's next presentation body at `+0xA6BC..+0xAB4C` — 1,168 bytes / 292 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via address-rematerialization and register/frame/schedule web reassignment); source kept as decomp-permuter input. Preserves the three-word ABI, six-call semantic graph, and all 44 compile relocations.

Overlay 57's mode-state dispatcher at `+0x00003A4C..+0x00003FD4` — 1,416 bytes / 354 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via an 87-site GPR-allocation web reassignment); source kept as decomp-permuter input. Preserves the exact `0x30` frame, complete CFG, 32 calls, 45 address pairs, and all 122 source relocation records.

Overlay 33's `overlay33InitializeBuffers` adds **324 exact C bytes / 81 words** and retains all 25 runtime relocation roles. Overlay 40's `overlay40BuildFrame` eight-record builder adds another **324 bytes**; its typed straight-line record construction naturally normalizes to all 81 retail instructions.

`overlay 40 DrawTintRectangle` — 348 bytes / 87 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via redundant pointer-copy deletion plus register/schedule permutation); source kept as decomp-permuter input.

Overlay 91's `overlay91UpdateTimeline` adds **1,136 bytes / 284 words** for the complete eight-state elapsed-carrying timeline and flagged graph-record walk. IDO emits every instruction word naturally under the measured R4300 multiply scheduler flag, and the duplicate private switch table is naturally removed.

`overlay 40 FadeRecords` — 404 bytes / 101 words. NON_MATCHING: canonical C is exact-size with three register operands differing at `+0xC`, `+0x10`, and `+0x24`; the target assembly remains canonical.

The live checkpoint contributes **9,812 / 45,775 (21.44%)** Epoch 12 bytes, leaves **35,963 bytes** to the hard byte exit, raises overlay C ownership to **111,064 / 469,264 (23.67%)**, and raises whole-program resolved text to **154,104 / 950,332 (16.22%)**. Overlay 40 raises the campaign closure gate to **1 / 8**. The linked binary remains byte-identical to the US baserom at SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548`.

`overlay 96 Register`/`Unregister` — 248 bytes / 62 words combined. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via copy/address preparation plus schedule web reassignment); source kept as decomp-permuter input. Register scans backward for duplicates and appends within the bounded 16-entry registry; Unregister scans backward for a match and compacts following entries; each preserves six runtime address relocations.

`overlay 96 FindVolume` — 192 bytes / 48 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via an FP-producer swap); source kept as decomp-permuter input. Natural object owns the exact ABI, frame, CFG, countdown loops, and four address sites; walks the registry backward and accepts the first volume whose six plane equations are all nonnegative.

Overlay 94's `overlay94UpdateController` — 1,100 bytes / 275 words, closes the module. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a 120-site allocation/schedule web reassignment); source kept as decomp-permuter input. Natural source supplies the exact frame, CFG, twelve calls, twelve private address pairs, FP behavior, and boundary. Epoch 12 reaches **11,352 / 45,775 bytes (24.80%)**, leaving **34,423 bytes**; overlay C reaches **112,604 / 469,264 (24.00%)**, resolved text reaches **155,644 / 950,332 (16.38%)**, and the closure gate reaches **2 / 8**.

`overlay 96 BuildVolume` — 964 bytes / 241 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via stack-home/center-coordinate web reassignment); source kept as decomp-permuter input. Expands an oriented-volume definition into eight transformed vertices and six normalized plane equations, then registers the finished volume; natural IDO output otherwise retains the exact frame, three calls, loops, arithmetic, and complete 241-word boundary under the measured R4300 scheduler option. Together with the existing registry, query, bit-test, and draw routines this closes every executable non-padding range in overlay 96.

Overlay 75's `overlay75UpdateMovingObject` — 1,216 bytes / 304 words, closes that module. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a redundant-init replacement plus schedule/home web reassignment); source kept as decomp-permuter input. Its typed five-phase motion state machine naturally owns the exact boundary, `0x58` frame, 16 calls, branch topology, and FP work. Epoch 12 reaches **13,532 / 45,775 bytes (29.56%)**, overlay C reaches **114,784 / 469,264 (24.46%)**, resolved text reaches **157,824 / 950,332 (16.61%)**, and the closure gate reaches **4 / 8**.

Overlay 25's `overlay25InitializeEffect` — 380 bytes / 95 words, `+0x0000..+0x017C`. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via schedule/register/stack web reassignment); source kept as decomp-permuter input. Initializes lifetime, owner-derived motion, palette color, and an optional output vector with the exact frame, five calls, CFG, and opcode/FP inventories. A proved IDO alignment nop is naturally the retained pipeline nop. Epoch 12 reaches **13,912 / 45,775 bytes (30.40%)**, overlay C reaches **115,164 / 469,264 (24.54%)**, and resolved text reaches **158,204 / 950,332 (16.65%)**. Overlay 25 remains open from `+0x017C`, with its next function boundary at `+0x0588`.

Overlay 3's `overlay3SelectTarget` adds **336 exact C bytes / 84 words** starting at `+0x0588`, ending at `+0x06D8`. It switches between ordinary and weighted target search, expires a repeatedly selected object, updates the group selection, and returns the selected or anchor coordinates. Its source compiles naturally to the exact frame, instruction stream, calls, and nine runtime relocation sites. Epoch 12 reaches **14,248 / 45,775 bytes (31.13%)**, overlay C reaches **115,500 / 469,264 (24.61%)**, and resolved text reaches **158,540 / 950,332 (16.68%)**.

Overlay 25's `overlay25UpdateEffect` — final 1,036 bytes / 259 words, `+0x017C..+0x0588`, completing that module. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via schedule/register/home web reassignment); source kept as decomp-permuter input. Implements ballistic movement, collision response, entity-hit accounting, fade timing, and output vector scaling with the exact operation multiset, eleven calls, 25 relocations, and six retained literals.

Overlay 3's `overlay3TouchObject` — 136 bytes / 34 words, `+0x06D8..+0x0760`. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via caller-saved register reassignment, thirteen sites); source kept as decomp-permuter input. Its two fixed 32-entry descending searches have the exact natural CFG and operation inventory. Epoch 12 reaches **15,420 / 45,775 bytes (33.69%)**, overlay C reaches **116,672 / 469,264 (24.86%)**, resolved text reaches **159,712 / 950,332 (16.81%)**, and the closure gate reaches **5 / 8**.

Overlay 44's `overlay44UpdateFrameCache` — 748 bytes / 187 words, offset `0x0294..0x0580`. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via allocation web reassignment); source kept as decomp-permuter input. Walks the frame cache and refreshes stale entries with the exact nested CFG, calls, and frame.

Overlay 83's `overlay83BuildBatch` adds **672 exact C bytes / 168 words** starting at offset `0x053C`, ending at `0x07DC`. It allocates a batch, scales source records, transforms world coordinates, and conditionally creates linked children, with the natural runtime relocation set intact.

Overlay 3's `overlay3FindClosestObject` contributes **308 exact C bytes / 77 words** at `+0x027C..+0x03B0`. A bounded permutation reproduces the exact `0x78` frame, instruction stream, relocation surface, linked owned range, and full ROM. The retained inert allocation blocks are documented in the source for later readability cleanup without weakening the exact claim.

Epoch 12 reaches **17,148 / 45,775 bytes (37.46%)**, overlay C reaches **118,400 / 469,264 (25.23%)**, resolved text reaches **161,440 / 950,332 (16.99%)**, and the closure gate remains **5 / 8**.

Overlay 83's `overlay83Update` adds **628 exact C bytes / 157 words** at offset `0x02A0`, ending at `0x0514`. It ages an eight-entry circular record queue, creates new trail records, integrates motion, transforms world coordinates, and publishes them to an optional linked object, with its exact two runtime call sites natural throughout.

Overlay 83's `overlay83DrawStrip` adds the final **308 exact C bytes / 77 words** at offset `0x0850`, ending at `0x0984`. It emits primitive color, environment color, vertex-load, and polygon commands while retaining the local vertex-template HI/LO identity. Together these bodies make every overlay 83 executable interval exact C and close the module.

Overlay 3's `overlay3SelectScoredObject` — 472 bytes / 118 words, offset `0x03B0..0x0588`. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via carrier/stack-owner web reassignment); source kept as decomp-permuter input. Its cached-result path and scored descending search preserve the exact five calls, FP topology, and frame; the measured R4300 multiply-hazard flag naturally supplies the target's one spacing nop.

Epoch 12 reaches **18,556 / 45,775 bytes (40.54%)**, overlay C reaches **119,808 / 469,264 (25.53%)**, resolved text reaches **162,848 / 950,332 (17.14%)**, and overlay 83 advances the closure gate to **6 / 8**.

Overlay 3's `overlay3RunCachedModeAction` — final 452 bytes / 113 words, offset `0x00B8..0x027C`. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via carrier-cycle/owner-web reassignment); source kept as decomp-permuter input. Performs cached-target angle and path gating, then dispatches four mode actions through two local chance-table reads with the exact eleven calls and fifteen runtime relocation operations. With only the separately classified twelve-byte padding left as assembly, every executable overlay 3 interval is exact C and the module closes.

Epoch 12 reaches **19,008 / 45,775 bytes (41.53%)**, overlay C reaches **120,260 / 469,264 (25.63%)**, resolved text reaches **163,300 / 950,332 (17.18%)**, and the closure gate advances to **7 / 8**.

Overlay 4's `overlay4FindCategory2Object` at `+0x0734..+0x08F4` contributes
**448 exact C bytes / 112 words**. It walks the resident-provided object range
and returns the first type-`0x30` payload in category 2 with the requested byte
identifier. The configured mixed-TU object, relocations, linked range, complete
overlay, and US ROM are byte-identical.

Overlay 7's `overlay7EntryPool` — 552 bytes / 138 words, offset `0x0000..0x0228`. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via return-tail schedule reassignment); source kept as decomp-permuter input. Its two functions unlink released entries into a free list and acquire entries after duplicate, owner/type, and priority checks; the acquire body has a distinct static symbol so later matched callers retain their runtime-relocated zero-address placeholder. The module still has executable assembly at `0x0324..0x0EDC` and `0x0F08..0x0FC0`, so this is not a module closure and the closure gate remains **7 / 8**. Epoch 12 reaches **20,008 / 45,775 bytes (43.71%)**, overlay C reaches **121,260 / 469,264 (25.84%)**, and resolved text reaches **164,300 / 950,332 (17.29%)**.

Overlay 4's `overlay4FindSearchPosition` at `+0x08F4..+0x0CAC` contributes
**952 exact C bytes / 238 words**. Mode one performs a four-way unrolled nearest
type-`0x21` search in the X/Z plane; the other mode calls the preceding
category-2 identifier search. The measured multiply-hazard flag supplies the
retail FP spacing, and the configured object through full ROM is byte-identical.

Overlay 4's `overlay4UpdateGroupSpacing` at `+0x05D0..+0x0710` contributes
**320 exact C bytes / 80 words**. Together these three newly reconciled Overlay
4 owners add 1,720 exact bytes. The separate `+0x0138..+0x04D0` body remains
outside this exact set, so this reconciliation does not claim module closure.

Overlay 99's `overlay99BuildHeightGrid` at `0x0638..0x0800` remains guarded
`NON_MATCHING`: clean configured C emits 115 words for a 114-word target, with
104 differing positions and only 8/29 runtime offset/type pairs aligned. All
119 flags and one trace-led magnitude carrier are exhausted. The adjacent
`overlay99RenderSortedEntries` is also guarded; neither range contributes exact
C credit, and linked equality comes from their assembly fallbacks.

Overlay 5's `overlay5InitializeAudio` at `0x031C..0x06C0` — 932 bytes, closes that module's executable text.

`overlay 5 InitializeAudio` — NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via relocation-aware schedule reassignment); source kept as decomp-permuter input. Its 29 call sites and 21 data-address pairs remain exact.

Overlay 7's natural `overlay7InitPool` at `0x0F08..0x0FB8` adds **176 exact C bytes** and owns the local BSS free-list layout through `+0x2A0`; the following eight zero bytes and final BSS tail remain separately owned.

Epoch 12 reaches **25,008 / 45,775 bytes (54.63%)**, overlay C reaches **126,260 / 469,264 (26.91%)**, resolved text reaches **169,300 / 950,332 (17.81%)**, and the closure gate is **9 / 8**.

The historical Epoch 12 claim that Overlay 7's `overlay7DispatchSelection` at `0x0CCC..0x0DBC` added 240 exact C bytes is retracted. Current configured C is 58/60 relocation-normalized words, with a two-site UGEN temp-coalescing residual; all 13 runtime identities are exact, but the assembly fallback remains canonical. Overlay 99's `overlay99RenderSegments` at `0x0BA4..0x0DDC` adds **568 exact C bytes**, naturally retaining all seven shipped call relocations across its first-live-segment setup gate, command emission, signed heading correction, object mutation/restoration, and adjacent five-argument renderer call.

Epoch 12 reaches **25,816 / 45,775 bytes (56.40%)**, overlay C reaches **127,068 / 469,264 (27.08%)**, and resolved text reaches **170,108 / 950,332 (17.90%)**.

The historical adjacent claim for `overlay7CommitSelection` at `0x0DBC..0x0EDC` is likewise retracted: current configured C is 69/72 relocation-normalized words, with one post-`mathRnd` conversion carrier differing. All 17 runtime identities are proven, but the function is not exact and retains its assembly fallback. The following Epoch 12 percentages are retained only as historical campaign snapshots, not current scoreboard values: **26,104 / 45,775**, **127,356 / 469,264**, and **170,396 / 950,332**.

Overlay 7's adjacent middle pair — 1,080 bytes / 270 words — remains `NON_MATCHING`; source is retained as decomp input. `overlay7DispatchModes` at `0x0894..0x0AA0` is 129/131 runtime-normalized words with exact 131-word size and frame `0x20`; all 30 runtime identities are proven, including seven local switch-table records, while one two-site flags-carrier temp web remains. `overlay7UpdateOwnerMode` at `0x0AA0..0x0CCC` records staged checks, validates three thresholds for the special third pass, and chooses the next owner mode from the previous state and failure result.

Epoch 12 reaches **27,184 / 45,775 bytes (59.39%)**, overlay C reaches **128,436 / 469,264 (27.37%)**, and resolved text reaches **171,476 / 950,332 (18.04%)**.

An integration batch owns another **1,252 bytes**: overlay 61 `+0x000..+0x1C0` (input, natural exact), overlay 100 `+0x38C..+0x50C` (motion), and overlay 61 `+0x7C4..+0x968` (list rendering).

`overlay 61 +0x7C4..+0x968` and `overlay 100 +0x38C..+0x50C` — NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via schedule permutation / private compiler web reassignment); source kept as decomp-permuter input. The motion updater's runtime relocation targets the distinct module `+0x278` release helper.

Epoch 12 reaches **28,436 / 45,775 bytes (62.12%)**, overlay C reaches **129,688 / 469,264 (27.64%)**, and resolved text reaches **172,728 / 950,332 (18.18%)**.

`overlay 61 +0x1DC..+0x3C0` — 484 bytes. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via two schedule permutations covering five instruction-order residuals); source kept as decomp-permuter input. Natural C object otherwise has the target frame, CFG, size, calls, delay slots, and ten relocation records. Epoch 12 reaches **28,920 / 45,775 bytes (63.18%)**, overlay C reaches **130,172 / 469,264 (27.74%)**, and resolved text reaches **173,212 / 950,332 (18.23%)**.

Overlay 61 `+0x3C0..+0x7C4` adds **1,028 natural-exact bytes**, closing the last assembly gap in its `+0x000..+0xB84` prefix, with no instruction normalization — only compiler section alignment beyond the function boundary was trimmed. The configured object reproduces the frame, 128-byte formatting buffer, switch CFG, 49 relocations, calls, and delay slots. Epoch 12 reaches **29,948 / 45,775 bytes (65.42%)**, overlay C reaches **131,200 / 469,264 (27.96%)**, and resolved text reaches **174,240 / 950,332 (18.33%)**.

Overlay 61 `+0x1578..+0x1648` (release body, natural exact, sixteen calls, all 44 relocations) and Overlay 100 `+0x000..+0x214` (motion initializer) together add **740 executable bytes**.

`overlay 100 +0x000..+0x214` — NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via stack-home/allocation/scheduling/loop-induction web reassignment); source kept as decomp-permuter input. Preserves the target `0x78` frame, branch-likely loop, and five runtime call identities.

Overlay 61 `+0x17B8..+0x18A0` — 232 bytes, character-record writer. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via stack-gap removal / prologue-epilogue home reassignment); source kept as decomp-permuter input. Its source reproduces the rounded allocation, word-copy loop, calls, delay slots, and six relocations. Epoch 12 now reaches **30,920 / 45,775 bytes (67.55%)**, overlay C reaches **132,172 / 469,264 (28.17%)**, and resolved text reaches **175,212 / 950,332 (18.44%)**.

Overlay 61 `+0x18A0..+0x19B0` — 272 bytes, joining the write, read, extension-choice, and size helpers into contiguous exact C through `+0x1A84`. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via stack/frame web reassignment); source kept as decomp-permuter input. The natural object already has the target 68-word boundary, CFG, calls, nine relocations, branches, and delay slots. Epoch 12 reaches **31,192 / 45,775 bytes (68.14%)**, overlay C reaches **132,444 / 469,264 (28.22%)**, and resolved text reaches **175,484 / 950,332 (18.47%)**.

Overlay 20 `+0x07C4..+0x09DC` — 536 bytes, tile-command builder. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via frame/array-base plus polygon-count register web reassignment); source kept as decomp-permuter input. Natural object has the target 134-word boundary, control flow, three command construction sequences, nested row/chunk loops, delay slots, and one runtime-call relocation. The runtime call uses a semantic proxy because overlay 20's raw relocation carrier is shared by multiple runtime targets, while the linked call word and relocation record remain exact; Diddy Kong Racing's published graphics macros are cited only as a source-shape crosswalk. Epoch 12 reaches **31,728 / 45,775 bytes (69.31%)**, overlay C reaches **132,980 / 469,264 (28.34%)**, and resolved text reaches **176,020 / 950,332 (18.52%)**.

Overlay 20 `+0x1018..+0x10EC` — 212 bytes, entry removal and pointer-array compaction. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a redundant-branch removal plus count/base/copy/mask register web reassignment); source kept as decomp-permuter input. Reconstructs the full 53-word semantic body, including all early exits, the active-count decrement, overlapping forward copy, the pool-address walk, active-bit clear, and ten data relocations; IDO's natural object uses a four-instruction search-backedge sequence where retail uses three. Epoch 12 reaches **31,940 / 45,775 bytes (69.78%)**, overlay C reaches **133,192 / 469,264 (28.38%)**, and resolved text reaches **176,232 / 950,332 (18.54%)**.

Overlay 20 `+0x0E28..+0x0F78` — 336 bytes, 32-slot entry allocator and field configurer. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via nop-relocation plus schedule/register web reassignment); source kept as decomp-permuter input. Has the exact `0x10` frame, nullable reuse path, bitmap/pool search, table/count/bitmap updates, floating-point arithmetic and conversions, ten semantic data relocations, and complete memory topology; IDO naturally emits 83 semantic words plus one zero alignment word. Epoch 12 reaches **32,276 / 45,775 bytes (70.51%)**, overlay C reaches **133,528 / 469,264 (28.45%)**, and resolved text reaches **176,568 / 950,332 (18.58%)**.

Overlay 29 `+0x14C8..+0x16CC` — 516 bytes / 129 words, grouped-command renderer. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via schedule/register web reassignment plus callee rebinding); source kept as decomp-permuter input. Its typed nested traversal and three calls are otherwise complete, preserving the three `R_MIPS_26` sites. Epoch 12 reaches **32,792 / 45,775 bytes (71.64%)**, overlay C reaches **134,044 / 469,264 (28.56%)**, and resolved text reaches **177,084 / 950,332 (18.63%)**.

Overlay 2 `+0x02C4..+0x0400` — 316 bytes / 79 words, boundary classifier between the already matched append and intersection helpers. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via dead-nop removal plus scheduler/register reassignment); source kept as decomp-permuter input. Its C preserves both axis modes and the equality-side copy semantics, and all six overlay-data relocations. Epoch 12 reaches **33,108 / 45,775 bytes (72.33%)**, overlay C reaches **134,360 / 469,264 (28.63%)**, and resolved text reaches **177,400 / 950,332 (18.67%)**.

Overlay 17 `+0x0668..+0x08B4` — 588 bytes / 147 words, double-buffer chain advance between its release and strip-render helpers. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a 65-word prefix schedule/register/spill web reassignment); source kept as decomp-permuter input. The source naturally reproduces the frame, backward halfword copy, point/color publication, fade loop, sole call relocation, and an exact 82-word suffix. Epoch 12 reaches **33,696 / 45,775 bytes (73.61%)**, overlay C reaches **134,948 / 469,264 (28.76%)**, and resolved text reaches **177,988 / 950,332 (18.73%)**.

Overlay 2 `+0x049C..+0x06E0` — 580 bytes / 145 words, line clipper immediately following the boundary helpers. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via frame/home, fallback-classification, and suffix-schedule web reassignment); source kept as decomp-permuter input. Its typed source owns the complete 20-byte-record traversal, accept/split cases, six calls, range publication, and all eight local data address records, and retains all 14 relocations. Epoch 12 reaches **34,276 / 45,775 bytes (74.88%)**, overlay C reaches **135,528 / 469,264 (28.88%)**, and resolved text reaches **178,568 / 950,332 (18.79%)**.

Overlay 17 `+0x0318..+0x0628` — 784 bytes / 196 words, chain constructor immediately before the three already exact chain operations. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via frame/schedule/register web reassignment); source kept as decomp-permuter input. The typed source owns the physical twelve-argument ABI, allocation mode, optional material resource, double-buffer layout, scaled 16-entry template copy, endpoint query, vertex conversion, RGB publication, and alpha clear; the configured object retains exactly three `R_MIPS_26` calls at `+0x48`, `+0x6C`, and `+0x204`. Epoch 12 reaches **35,060 / 45,775 bytes (76.59%)**, overlay C reaches **136,312 / 469,264 (29.05%)**, and resolved text reaches **179,352 / 950,332 (18.87%)**.

Overlay 17 `+0x0000..+0x0318` — final 792 bytes / 198 words, closing all `0xA90` executable bytes as C. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via frame/schedule/register web reassignment repositioning two calls); source kept as decomp-permuter input. The endpoint builder preserves the physical seven-argument ABI, transformed and untransformed geometry paths, runtime square-root normalization, cached-position update, null-chain clear loop, and six output stores; it binds three `R_MIPS_26` records at `+0x68`, `+0x108`, and `+0x1D4` to the offset-zero runtime identity. Epoch 12 reaches **35,852 / 45,775 bytes (78.32%)**, overlay C reaches **137,104 / 469,264 (29.22%)**, and resolved text reaches **180,144 / 950,332 (18.96%)**.

Overlay 2 `+0x06E0..+0x0C90` adds **1,456 exact C bytes / 364 words** across the boundary chooser and recursive region splitter, both natural. The two typed functions establish the shared 20-byte line/range and 16-byte candidate layouts, preserve the exhaustive scoring and clipping behavior, and reproduce all **73** owned relocations at their exact sites and identities. Overlay 20 `+0x0A68..+0x0DC4` adds **860 exact C bytes / 215 words**, also natural, with the complete overlap scan, grid displacement/color update, FP clamp behavior, and runtime relocation surface intact. Epoch 12 reaches **38,168 / 45,775 bytes (83.38%)**, overlay C reaches **139,420 / 469,264 (29.71%)**, and resolved text reaches **182,460 / 950,332 (19.20%)**.

Overlay 20 `+0x00A8..+0x038C` — 740 bytes / 185 words across resource configuration and object-resource update. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via schedule permutation plus FP/frame/home/register web reassignment); source kept as decomp-permuter input. All nine call relocations are retained. The neighboring `+0x038C..+0x07C4` body remains assembly-owned and uncredited until a policy-compliant compiler basin is proved. Epoch 12 reaches **38,908 / 45,775 bytes (85.00%)**, overlay C reaches **140,160 / 469,264 (29.87%)**, and resolved text reaches **183,200 / 950,332 (19.28%)**.

Overlay 1 `+0x6424..+0x64F8` adds **212 naturally exact bytes / 53 words** for the selection-vector reader. Its typed body preserves the signed selection index, direct object-position path, inclusive descriptor bound, null-vector hazard, repeated vector-base loads, and fallback output triple, with no relocations or normalization present. A separate Overlay 20 tail experiment is explicitly uncredited because its whole-function field ledger failed the bounded-normalization policy. Epoch 12 reaches **39,120 / 45,775 bytes (85.46%)**, overlay C reaches **140,372 / 469,264 (29.91%)**, and resolved text reaches **183,412 / 950,332 (19.30%)**.

Overlay 2 `+0x0000..+0x01BC` adds **444 naturally exact bytes / 111 words** for the region validator. Direct use of `region->count` in both the outer condition and countdown initialization naturally retains retail's two loop-value moves; there is no instruction normalization. Its eight runtime relocations agree with the shipped tables: one local HI/LO pair, three angle calls, and three signed-angle-difference calls. Epoch 12 reaches **39,564 / 45,775 bytes (86.43%)**, overlay C reaches **140,816 / 469,264 (30.01%)**, and resolved text reaches **183,856 / 950,332 (19.35%)**.

Overlay 2 `+0x123C..+0x1364` adds **296 naturally exact bytes / 74 words** for BSP point containment. The configured `-O2 -mips2 -32` object has the retail 56-byte frame and requires no instruction normalization. Its three `R_MIPS_26` relocations at function offsets `+0xAC`, `+0xCC`, and `+0xE0` agree exactly with runtime-table rows 26–28 and the two resident angle helper identities. Epoch 12 reaches **39,860 / 45,775 bytes (87.08%)**, overlay C reaches **141,112 / 469,264 (30.07%)**, and resolved text reaches **184,152 / 950,332 (19.38%)**.

Overlay 14 `+0x1B7C..+0x1C40` adds **196 naturally exact bytes / 49 words** for active-handle finalization. The nested-call source naturally emits the retail 48-byte frame, spill slot, call order, and complete instruction schedule with no normalization. Five distinct `R_MIPS_26` identities and three local HI/LO pairs agree exactly with the runtime tables. Epoch 12 reaches **40,056 / 45,775 bytes (87.51%)**, overlay C reaches **141,308 / 469,264 (30.11%)**, and resolved text reaches **184,348 / 950,332 (19.40%)**.

Overlay 1 `+0x6B6C..+0x6CE8` — 380 bytes / 95 words, nearby-object search and activation. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a private-stack/frame spill offset shift from `sp+0x28` to `sp+0x2C`); source kept as decomp-permuter input. Natural source otherwise supplies the exact 72-byte frame, scan and unsigned-threshold behavior, three calls, delay slots, opcode stream, and register/FP allocation; the three `R_MIPS_26` sites agree with the shipped runtime table's two resident targets and overlay 4 target. Epoch 12 reaches **40,436 / 45,775 bytes (88.34%)**, overlay C reaches **141,688 / 469,264 (30.19%)**, and resolved text reaches **184,728 / 950,332 (19.44%)**.

Overlay 1 `+0x72A4..+0x7344` — 160 bytes / 40 words, pool-record allocation. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a temporary-suffix reassignment plus an operand-commute on an `or`); source kept as decomp-permuter input. Natural source supplies the exact wraparound, exhaustion, record-filtering and status-bit semantics, instruction count, and schedule; the configured object retains all ten runtime HI/LO records for the cursor, group, pool bounds, and exhausted flag. Epoch 12 reaches **40,596 / 45,775 bytes (88.69%)**, overlay C reaches **141,848 / 469,264 (30.23%)**, and resolved text reaches **184,888 / 950,332 (19.46%)**.

Overlay 87 `+0x000..+0x128` — 296 bytes / 74 words, object initialization. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via an FP-temporary web reassignment, nine sites); source kept as decomp-permuter input. The target-proven `-Wab,-r4300_mul` flag naturally supplies the exact multiply hazards; natural source otherwise recovers the 32-byte frame, complete opcode schedule, stores, and two resident calls. The configured object retains the runtime local HI/LO pair and preserves the separately decoded `mathRnd` and `func_8005AD64` identities before folding them to the pre-loader carrier. Epoch 12 reaches **40,892 / 45,775 bytes (89.33%)**, overlay C reaches **142,144 / 469,264 (30.29%)**, and resolved text reaches **185,184 / 950,332 (19.49%)**.

Overlay 1's `overlay1UpdateValueCache` at `+0x73A0..+0x7580` contributes **480
exact C bytes / 120 words**. A bounded source permutation found an algebraically
zero counter expression that naturally preserves the retail caller-saved web;
the configured mixed-TU object, both local relocations, linked range, complete
overlay, and US ROM are byte-identical. The adjacent `+0x7130..+0x72A4`
updater remains outside this exact claim.

Overlay 34 `+0x000..+0x0C8` — 200 bytes / 50 words, storage allocation and zero-initialization. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a call-surviving size-home reassignment, four sites); source kept as decomp-permuter input. The runtime relocation table fixes both calls as the two-argument resident allocator `func_8002B280`; with that interface corrected, natural source supplies retail's full opcode schedule, register allocation, frame, and two clear loops. The configured object retains the exact two call records and three local HI/LO pairs. Epoch 12 reaches **41,944 / 45,775 bytes (91.63%)**, overlay C reaches **143,196 / 469,264 (30.52%)**, and resolved text reaches **186,236 / 950,332 (19.60%)**.

Overlay 1 `+0x7580..+0x7730` — 432 bytes / 108 words, path-point append, segment-length accumulation, cache update, and anchor distance. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a private-compiler-web reassignment); source kept as decomp-permuter input. Natural source otherwise supplies the complete behavior and boundary; the configured object retains the exact two call records and three anchor HI/LO pairs. Epoch 12 reaches **42,376 / 45,775 bytes (92.58%)**, overlay C reaches **143,628 / 469,264 (30.61%)**, and resolved text reaches **186,668 / 950,332 (19.64%)**.

Overlay 1 `+0x7BDC..+0x7D6C` adds **400 naturally exact bytes / 100 words** for record allocation, first-point initialization, squared-distance root, anchor publication, and validation/fallback selection. With the target-proven R4300 multiply-hazard flag, natural source emits retail's exact frame, six-call CFG, conversion sequence, delay slots, and register web. The configured object retains all six call records and both local anchor HI/LO pairs. Epoch 12 reaches **42,776 / 45,775 bytes (93.45%)**, overlay C reaches **144,028 / 469,264 (30.69%)**, and resolved text reaches **187,068 / 950,332 (19.68%)**.

Overlay 1 `+0x78DC..+0x7B64` — 648 bytes / 162 words, bounded path tracing, endpoint append, and branch-record cloning. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via an integer-temporary web reassignment); source kept as decomp-permuter input. Natural source otherwise emits the complete opcode schedule, 88-byte frame, stack layout, eight-call CFG, FP conversions, delay slots, and all observable effects; the configured object retains all eight call records and seven local HI/LO pairs. Epoch 12 reaches **43,424 / 45,775 bytes (94.86%)**, overlay C reaches **144,676 / 469,264 (30.83%)**, and resolved text reaches **187,716 / 950,332 (19.75%)**.

Overlay 34 `+0x2C8..+0x378` — 176 bytes / 44 words, active-record removal, list compaction, resident resource release, and count update. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via count/pointer web reassignment plus removal of two literal pointer-address relocation records absent from retail); source kept as decomp-permuter input. Natural source otherwise owns the exact boundary, frame, CFG, call ABI, and effects; the configured object retains the loader call and both active-count HI/LO pairs. Epoch 12 reaches **43,600 / 45,775 bytes (95.25%)**, overlay C reaches **144,852 / 469,264 (30.87%)**, and resolved text reaches **187,892 / 950,332 (19.77%)**.

Overlay 34's record constructor at `+0x0D4..+0x2C8` — 500 bytes / 125 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a compiler-representation/schedule web reassignment); source kept as decomp-permuter input. Natural source otherwise supplies the exact functional boundary, CFG, field initialization, allocation/load path, direction setup, and two-call ABI, preserving all 12 relocation sites: five local-data HI/LO pairs and two independently decoded resident calls. Epoch 12 reaches **44,100 / 45,775 bytes (96.34%)**, overlay C reaches **145,352 / 469,264 (30.97%)**, and resolved text reaches **188,392 / 950,332 (19.82%)**.

Overlay 34 `+0x378..+0x540` — 456 bytes / 114 words, storage reset and active-record updates. Reset is naturally exact and retains 15 shipped runtime relocations.

`overlay 34` update half — NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via removal of three IDO-only invariant-home words plus schedule/frame/register web reassignment); source kept as decomp-permuter input. Update's natural source owns the full behavior, FP lanes, loop topology, call ABI, and seven relocation roles; all 22 combined runtime records remain exact. Epoch 12 reaches **44,556 / 45,775 bytes (97.34%)**, overlay C reaches **145,808 / 469,264 (31.07%)**, and resolved text reaches **188,848 / 950,332 (19.87%)**.

Overlay 1 `+0x5CD4..+0x5ECC` — 504 bytes / 126 words, directional object selection. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via stack-home plus a two-instruction temporary-web reassignment); source kept as decomp-permuter input. Target-local R4300 multiply hazards naturally recover the exact frame, opcode stream, CFG, FP schedule, and all 11 relocation sites. Epoch 12 reaches **45,060 / 45,775 bytes (98.44%)**, overlay C reaches **146,312 / 469,264 (31.18%)**, and resolved text reaches **189,352 / 950,332 (19.92%)**.

Overlay 1 `+0x7D6C..+0x7FCC` — 608 bytes / 152 words, path-point resolution. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via two scheduling permutations plus a four-field register reassignment); source kept as decomp-permuter input. Natural source otherwise recovers the exact 120-byte frame, collision-output stack home, CFG, call topology, delay-slot effects, and full boundary; all 22 shipped runtime relocation records remain exact. Epoch 12 reaches **45,668 / 45,775 bytes (99.77%)**, overlay C reaches **146,920 / 469,264 (31.31%)**, and resolved text reaches **189,960 / 950,332 (19.99%)**.

Overlay 1 `+0x5BF4..+0x5CD4` — 224 bytes / 56 words, timer and mode callback dispatch. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a loop-tail reschedule plus register/branch-lowering reassignment); source kept as decomp-permuter input. Natural source otherwise recovers the exact boundary, frame, direct setup call, two indirect callbacks, loop semantics, and all 13 runtime relocation records. Epoch 12 closes at **45,892 / 45,775 bytes (100.26%)**, Overlay C reaches **147,144 / 469,264 (31.36%)**, resolved text reaches **190,184 / 950,332 (20.01%)**, and the closure gate remains **9 / 8**.

Overlay 34 `+0x608..+0x900` — final 760 bytes / 190 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via schedule/register/frame web reassignment); source kept as decomp-permuter input. Typed source owns the depth census, parallel bubble sort, color interpolation, and ten-argument render dispatch; canonical integration rejected a zero-local proxy false positive and retained the shipped semantic local addends for all five HI16/LO16 pairs while using pre-loader carriers only for the six calls, and all 16 runtime records remain exact. Every executable interval is now C, closing the module and raising the closure result to **10 / 8**. The stretch checkpoint is **46,652 / 45,775 campaign bytes (101.92%)**, **147,904 / 469,264 Overlay C (31.52%)**, and **190,944 / 950,332 resolved text (20.09%)**.
`overlay 1 +0x7730..+0x78DC` — 428 bytes / 107 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via prologue-instruction reordering plus a commutative-operand swap); source kept as decomp-permuter input. All six runtime calls were confirmed exact. The stretch checkpoint becomes **47,080 / 45,775 campaign bytes (102.85%)**, **148,332 / 469,264 Overlay C (31.61%)**, and **191,372 / 950,332 resolved text (20.14%)**.

`overlay 1 +0x6A14..+0x6B28` — 276 bytes / 69 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via instruction-scheduling reorder plus a register-allocation rewrite); source kept as decomp-permuter input. One runtime relocation was confirmed exact. The stretch checkpoint becomes **47,356 / 45,775 campaign bytes (103.45%)**, **148,608 / 469,264 Overlay C (31.67%)**, and **191,648 / 950,332 resolved text (20.17%)**.

`overlay 1 +0x5ED4..+0x61F0` — 796 bytes / 199 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via allocator-web reassignment plus a pre-loader addend rewrite); source kept as decomp-permuter input. 61 runtime text records were confirmed authoritative. The stretch checkpoint becomes **48,152 / 45,775 campaign bytes (105.19%)**, **149,404 / 469,264 Overlay C (31.84%)**, and **192,444 / 950,332 resolved text (20.25%)**.

`overlay 1 +0x6270..+0x63CC` (`overlay1ChooseModeObject`) is now **348 exact C bytes / 87 words**. The ordinary configured IDO build reaches the target by declaring the existing locals in reverse stack-home order; no synthetic local, compiler-output edit, or pre-loader addend rewrite is used. The `0x50` frame and all 13 runtime text records are exact by offset, type, and identity: calls resolve to resident `func_80005750` once and `mathRnd` twice, while the five local HI16/LO16 pairs retain their overlay-local identities. The function has no owned padding, is referenced by the overlay-local `R_MIPS_32` table entry at `+0x8218`, and is exact in the linked overlay and full ROM. The donor scan found no useful DKR, JFG, Conker, or same-overlay sibling candidate.

`overlay 1 +0x64F8..+0x6724` — 556 bytes / 139 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via loop-control carrier relocation and a shared-tail reorder); source kept as decomp-permuter input. Four runtime call records were confirmed exact. The stretch checkpoint becomes **49,056 / 45,775 campaign bytes (107.17%)**, **150,308 / 469,264 Overlay C (32.03%)**, and **193,348 / 950,332 resolved text (20.35%)**.

`overlay 1 +0x6D4C..+0x7130` — 996 bytes / 249 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via schedule reordering plus a two-slot stack-home rewrite); source kept as decomp-permuter input. All 43 runtime text records were confirmed exact. The stretch checkpoint becomes **50,052 / 45,775 campaign bytes (109.34%)**, **151,304 / 469,264 Overlay C (32.24%)**, and **194,344 / 950,332 resolved text (20.45%)**.

`overlay 1 +0x67C0..+0x69A0` — 480 bytes / 120 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via register-allocation reassignment across six sites); source kept as decomp-permuter input. Four runtime call records were confirmed exact. The stretch checkpoint becomes **50,532 / 45,775 campaign bytes (110.39%)**, **151,784 / 469,264 Overlay C (32.35%)**, and **194,824 / 950,332 resolved text (20.50%)**.

`overlay 1 +0x04B4..+0x0758` — 676 bytes / 169 words, covering the activation function and its closest-sample pair. The second function is naturally exact. NON_MATCHING (first function): retired 2026-08-24 per ADR 0002 (was made to match via allocator-web reassignment plus loader-local addend rewrites); source kept as decomp-permuter input. All 44 runtime text records were confirmed authoritative. The stretch checkpoint becomes **51,208 / 45,775 campaign bytes (111.87%)**, **152,460 / 469,264 Overlay C (32.49%)**, and **195,500 / 950,332 resolved text (20.57%)**.

`overlay 1 +0x01AC..+0x02D4` — 296 bytes / 74 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a schedule/register-home rewrite that also relocated a scale-relocation instruction); source kept as decomp-permuter input. All six runtime text records were confirmed exact. The stretch checkpoint becomes **51,504 / 45,775 campaign bytes (112.52%)**, **152,756 / 469,264 Overlay C (32.55%)**, and **195,796 / 950,332 resolved text (20.60%)**.

`overlay 1 +0x0BD4..+0x10C0` — 1,260 bytes / 315 words across four functions (motion scaling, path interpolation, motion-point resolution, sampled curve length). NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via schedule/allocation/stack-home rewrites and local literal-addend edits); source kept as decomp-permuter input. All 44 runtime text records were confirmed authoritative. The stretch checkpoint becomes **52,764 / 45,775 campaign bytes (115.27%)**, **154,016 / 469,264 Overlay C (32.82%)**, and **197,056 / 950,332 resolved text (20.74%)**.

`overlay1MeasureCurves` (`overlay 1 +0x0F84..+0x10C0`) plateau: exact 79-word size,
frame `0x70`, and five call-relocation sites; 27 masked words differ from `+0xC`.
The full flag lattice and 40-minute permuter produced no valid exact source.

`overlay 1 +0x19B8..+0x1CA4` — 748 bytes / 187 words, covering mode-state initialization and object-mapping construction. The first function is naturally exact apart from a trimmed alignment word. NON_MATCHING (second function): retired 2026-08-24 per ADR 0002 (was made to match via a representation-web rewrite selecting the retail schedule); source kept as decomp-permuter input. All 28 runtime text records were confirmed exact. The stretch checkpoint becomes **53,512 / 45,775 campaign bytes (116.90%)**, **154,764 / 469,264 Overlay C (32.98%)**, and **197,804 / 950,332 resolved text (20.81%)**.

| Overlay | Range | Function | Bytes | Exactness | Donor |
|---:|---|---|---:|---|---|
| 1 | `+0x296C..+0x2AA4` | `overlay1AdvanceObjectGauges` | 312 | canonical object and linked ROM exact | Mickey-only |

`overlay 1 +0x2AA4..+0x2B4C` and `+0x3578..+0x3750` — 640 bytes / 160 words across three gauge and variable-record functions. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via representation-web rewrites, with two identity copies removed under whole-function edits); source kept as decomp-permuter input. All remaining runtime records were confirmed exact. The checkpoint becomes **54,464 / 45,775 campaign bytes (118.98%)**, **155,716 / 469,264 Overlay C (33.18%)**, and **198,756 / 950,332 resolved text (20.91%)**.

`overlay 1 +0x61F0..+0x6270` — 128 bytes / 32 words, cached-mode handling. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via redundant-branch/identity-copy removal plus a register/schedule rewrite); source kept as decomp-permuter input. The 13 runtime records comprise three calls, five HI16, and five LO16 sites. The checkpoint becomes **54,592 / 45,775 campaign bytes (119.26%)**, **155,844 / 469,264 Overlay C (33.21%)**, and **198,884 / 950,332 resolved text (20.93%)**.

`overlay 60 +0x3488..+0x355C` — 212 bytes / 53 words, choice-slot reassignment. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a stack/address/cursor representation rewrite); source kept as decomp-permuter input. The 8 runtime records are four HI16 and four LO16 sites targeting resident offset `0x4D618`. The checkpoint becomes **54,804 / 45,775 campaign bytes (119.72%)**, **156,056 / 469,264 Overlay C (33.26%)**, and **199,096 / 950,332 resolved text (20.95%)**.

`overlay 15 +0xB94..+0xC6C` — 216 bytes / 54 words, rain rendering. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via redundant address-producer removal plus a count-home/addend rewrite); source kept as decomp-permuter input. The 17 runtime records comprise two resident calls plus seven HI16 and eight LO16 local sites. The checkpoint becomes **55,020 / 45,775 campaign bytes (120.20%)**, **156,272 / 469,264 Overlay C (33.30%)**, and **199,312 / 950,332 resolved text (20.97%)**.

`overlay 36 +0x09B8..+0x0A60` (`overlay36` position-effect callback) contributes **168 exact bytes / 42 words**. Natural codegen owns every instruction; only eight bytes of tail alignment are trimmed. The exact 3 runtime records are one resident call and one HI16/LO16 pair for the Overlay 36 variant byte. The checkpoint becomes **55,188 / 45,775 campaign bytes (120.56%)**, **156,440 / 469,264 Overlay C (33.34%)**, and **199,480 / 950,332 resolved text (20.99%)**.

`overlay 15 +0x0428..+0x0500` — 216 bytes / 54 words, star motion. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via redundant address-anchor removal plus an addend/schedule rewrite); source kept as decomp-permuter input. All 21 runtime roles (one resident call, twenty local data/BSS records) were confirmed. The checkpoint becomes **55,404 / 45,775 campaign bytes (121.04%)**, **156,656 / 469,264 Overlay C (33.38%)**, and **199,696 / 950,332 resolved text (21.01%)**.

`overlay 53 +0x0000..+0x011C` — 284 bytes / 71 words, initialization. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via an address-finalizer plus local LO16-addend rewrite); source kept as decomp-permuter input. The exact 30 runtime records comprise eight symbol and 22 local/jump sites. The checkpoint becomes **55,688 / 45,775 campaign bytes (121.66%)**, **156,940 / 469,264 Overlay C (33.44%)**, and **199,980 / 950,332 resolved text (21.04%)**.

`overlay 53 +0x016C..+0x0240` (offset-entry copying) contributes **212 exact bytes / 53 words**. IDO emits it naturally with no normalization. Its exact 5 runtime records are one resident call and two local HI16/LO16 table pairs. The checkpoint becomes **55,900 / 45,775 campaign bytes (122.12%)**, **157,152 / 469,264 Overlay C (33.49%)**, and **200,192 / 950,332 resolved text (21.07%)**.

`overlay 1 +0x438C..+0x5BA4` — 6,168 bytes / 1,542 words, the central object-physics/state updater. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a large decoded schedule/register/frame rewrite over most of the function); source kept as decomp-permuter input. All 184 runtime relocations were confirmed present. The checkpoint becomes **62,068 / 45,775 campaign bytes (135.59%)**, **163,320 / 469,264 Overlay C (34.80%)**, and **206,360 / 950,332 resolved text (21.71%)**.

`overlay 15 +0x09E0..+0x0B7C` — 412 bytes / 103 words, moving-star camera updater. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via redundant HI-anchor removal plus a decoded addend/schedule rewrite); source kept as decomp-permuter input. All 39 runtime records (two calls, 33 BSS records, four data records) were confirmed. The checkpoint becomes **62,480 / 45,775 campaign bytes (136.49%)**, **163,732 / 469,264 Overlay C (34.89%)**, and **206,772 / 950,332 resolved text (21.76%)**.

`overlay 1 +0x3FD8..+0x438C` — 948 bytes / 237 words, a two-path five-phase transition-state machine. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a representation-pool plus schedule/register-web rewrite); source kept as decomp-permuter input. All 13 runtime relocations were confirmed exact; three intra-overlay loader calls keep raw zero fields in the static link while runtime resolves their separate identities. The checkpoint becomes **63,428 / 45,775 campaign bytes (138.56%)**, **164,680 / 469,264 Overlay C (35.09%)**, and **207,720 / 950,332 resolved text (21.86%)**.

`overlay 15 +0x0500..+0x06A4` — 420 bytes / 105 words, the starfield renderer. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a local-addend and branch-displacement schedule rewrite); source kept as decomp-permuter input. All 10 runtime records (two resident calls, six initialized-data sites, two constant sites) were confirmed. The checkpoint becomes **63,848 / 45,775 campaign bytes (139.48%)**, **165,100 / 469,264 Overlay C (35.18%)**, and **208,140 / 950,332 resolved text (21.90%)**.

`overlay 1 +0x3750..+0x3E48` — 1,784 bytes / 446 words, the path-selection and interpolation state machine. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a redundant-zero removal plus a relocation-aware schedule/register rewrite); source kept as decomp-permuter input. The static linker surface has 31 records; the runtime evidence covers all 63 relocation roles. The checkpoint becomes **65,632 / 45,775 campaign bytes (143.38%)**, **166,884 / 469,264 Overlay C (35.56%)**, and **209,924 / 950,332 resolved text (22.09%)**.

`overlay 36 +0x0818..+0x0914` — 252 bytes / 63 words, the nearby-height filter. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a redundant-zero removal plus a schedule/register rewrite); source kept as decomp-permuter input. All three runtime relocations were confirmed exact. The checkpoint becomes **65,884 / 45,775 campaign bytes (143.93%)**, **167,136 / 469,264 Overlay C (35.62%)**, and **210,176 / 950,332 resolved text (22.12%)**.

`overlay 1 +0x10C8..+0x19B8` — 2,288 bytes / 572 words, packed-record loading and group/metric construction. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a representation-pool plus relocation-aware schedule/register/frame rewrite); source kept as decomp-permuter input. Runtime evidence covers all 114 relocation roles; the static object's raw link surface is 32 records. The checkpoint becomes **68,172 / 45,775 campaign bytes (148.93%)**, **169,424 / 469,264 Overlay C (36.10%)**, and **212,464 / 950,332 resolved text (22.36%)**.

`overlay 13 +0x0000..+0x0124` and `+0x0188..+0x0284` (module initialization and record allocation) contribute **544 exact bytes / 136 words**. Natural typed source reproduces every instruction with no normalization. The configured objects retain exact 17-record and 7-record relocation surfaces. The checkpoint becomes **68,716 / 45,775 campaign bytes (150.12%)**, **169,968 / 469,264 Overlay C (36.22%)**, and **213,008 / 950,332 resolved text (22.41%)**.

`overlay 14 +0x1040..+0x1164` — 292 bytes / 73 words, the module's signed command-index dispatcher. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a register/home rewrite over an otherwise-natural object); source kept as decomp-permuter input. The natural object owns 15 runtime relocation roles and the exact seven-call static link surface. The checkpoint becomes **69,008 / 45,775 campaign bytes (150.75%)**, **170,260 / 469,264 Overlay C (36.28%)**, and **213,300 / 950,332 resolved text (22.44%)**.

`overlay 55 +0x0000..+0x013C` — 316 bytes / 79 words, module startup. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a two-word schedule rewrite plus two local addend edits); source kept as decomp-permuter input. The natural object owns 33 runtime relocation roles and a 21-record raw link surface. The checkpoint becomes **69,324 / 45,775 campaign bytes (151.45%)**, **170,576 / 469,264 Overlay C (36.35%)**, and **213,616 / 950,332 resolved text (22.48%)**.

`overlay 84 +0x0DD0..+0x0F18` (`overlay84AdvanceCurrent`) — 328 bytes / 82 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a frame/spill plus twelve-instruction schedule rewrite); source kept as decomp-permuter input. Natural codegen owns the exact CFG, opcode/register census, and all five runtime relocation roles. The checkpoint becomes **69,652 / 45,775 campaign bytes (152.16%)**, **170,904 / 469,264 Overlay C (36.42%)**, and **213,944 / 950,332 resolved text (22.51%)**.

`overlay 36 +0x150C..+0x1688` (`overlay36UpdatePeers`) — 380 bytes / 95 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a schedule/frame/scratch-register rewrite); source kept as decomp-permuter input. Natural codegen owns the exact six-call CFG and all six runtime identities. The checkpoint becomes **70,032 / 45,775 campaign bytes (152.99%)**, **171,284 / 469,264 Overlay C (36.50%)**, and **214,324 / 950,332 resolved text (22.55%)**.

`overlay 86 +0x007C..+0x0158` (`overlay86ScaledVectorPosition`) — 220 bytes / 55 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a seven-immediate private-local-home rewrite); source kept as decomp-permuter input. Natural codegen owns the exact frame, CFG, and all three runtime relocation roles. The checkpoint becomes **70,252 / 45,775 campaign bytes (153.47%)**, **171,504 / 469,264 Overlay C (36.55%)**, and **214,544 / 950,332 resolved text (22.58%)**.

`overlay 36 +0x1378..+0x1470` (`overlay36SpawnAndUpdate`) — 248 exact bytes / 62 words. Recovering the second physical argument as `s32` yields natural 62/62 codegen with the exact three-call relocation surface; no instruction normalization is used. The checkpoint becomes **70,500 / 45,775 campaign bytes (154.01%)**, **171,752 / 469,264 Overlay C (36.60%)**, and **214,792 / 950,332 resolved text (22.60%)**.

`overlay 86 +0x02E4..+0x0444` (`overlay86SelectPosition`) — 352 bytes / 88 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a five-home plus two-load representation rewrite); source kept as decomp-permuter input. Natural codegen owns the exact frame, CFG, and all three runtime relocation roles. The checkpoint becomes **70,852 / 45,775 campaign bytes (154.78%)**, **172,104 / 469,264 Overlay C (36.68%)**, and **215,144 / 950,332 resolved text (22.64%)**.

`overlay 86 +0x0158..+0x02E4` (`overlay86BuildTransform`) — 396 bytes / 99 words. Natural local-declaration order gives the exact frame, CFG, all five runtime relocation roles, linked owned range, and full-US-ROM hash without instruction normalization or compiler forcing.

`overlay 36 +0x1214..+0x1378` (`overlay36SpawnOffsetA9`) — 356 bytes / 89 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via an `$f4`/`$f6` constant-carrier rewrite); source kept as decomp-permuter input. Natural codegen owns the exact four-call CFG and all four runtime relocation roles. The checkpoint becomes **71,604 / 45,775 campaign bytes (156.43%)**, **172,856 / 469,264 Overlay C (36.84%)**, and **215,896 / 950,332 resolved text (22.72%)**.

`overlay 36 +0x0F20..+0x1084` (`overlay36SpawnDirectional`) — 356 bytes / 89 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via an `$f4`/`$f6` constant-carrier rewrite); source kept as decomp-permuter input. Natural codegen owns the exact four-call CFG and all four runtime relocation roles. The checkpoint becomes **71,960 / 45,775 campaign bytes (157.20%)**, **173,212 / 469,264 Overlay C (36.91%)**, and **216,252 / 950,332 resolved text (22.76%)**.

`overlay 36 +0x1084..+0x1214` (`overlay36SpawnConditional`) contributes **400 naturally exact bytes / 100 words**. The compiler emits the exact owner with all four runtime relocation roles; no normalization or section trim is used. The checkpoint becomes **72,360 / 45,775 campaign bytes (158.08%)**, **173,612 / 469,264 Overlay C (37.00%)**, and **216,652 / 950,332 resolved text (22.80%)**.

`overlay 36 +0x0D8C..+0x0F20` (`overlay36SpawnLinked7F`) — 404 bytes / 101 words, closing the 1,764-byte `+0x0D8C..+0x1470` island across five independently proved owners. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via an initial FP-carrier plus uniform private-local-home rewrite); source kept as decomp-permuter input. Natural codegen owns all opcodes and the exact seven-record relocation topology. The checkpoint becomes **72,764 / 45,775 campaign bytes (158.96%)**, **174,016 / 469,264 Overlay C (37.08%)**, and **217,056 / 950,332 resolved text (22.84%)**.

`overlay 36 +0x0694..+0x07B0` (`overlay36SpawnTransient`) — 284 bytes / 71 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a bijective setup-schedule plus three-temporary rotation rewrite); source kept as decomp-permuter input. Natural codegen is exact from `+0x38` onward; the exact relocation surface is three records. The checkpoint becomes **73,048 / 45,775 campaign bytes (159.58%)**, **174,300 / 469,264 Overlay C (37.14%)**, and **217,340 / 950,332 resolved text (22.87%)**.

`overlay 46 +0x0FD0..+0x112C` (`overlay46UpdateTransition`) contributes **348 naturally exact bytes / 87 words**. The exact owner needs no instruction normalization; only an independent compiler alignment word is trimmed. All 28 runtime relocation roles were confirmed. The checkpoint becomes **73,396 / 45,775 campaign bytes (160.34%)**, **174,648 / 469,264 Overlay C (37.22%)**, and **217,688 / 950,332 resolved text (22.91%)**.

`overlay 14 +0x1184..+0x12D8` (`overlay14UpdateTransition`) — 340 bytes / 85 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a register/home allocation rewrite over an otherwise-natural object); source kept as decomp-permuter input. Natural codegen owns the exact CFG, instruction count, opcode inventory, and all 28 runtime relocation roles. The checkpoint becomes **73,736 / 45,775 campaign bytes (161.08%)**, **174,988 / 469,264 Overlay C (37.29%)**, and **218,028 / 950,332 resolved text (22.94%)**.

`overlay 57 +0x4C18..+0x4D90` (`overlay57UpdateModeTrigger`) — 376 bytes / 94 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a setup/index/register rewrite plus three asserted compiler-elided local-base records); source kept as decomp-permuter input. The natural object owns 38 runtime relocation roles: 31 local HI16/LO16 records, five external calls, and two overlay-local jumps. The checkpoint becomes **74,112 / 45,775 campaign bytes (161.90%)**, **175,364 / 469,264 Overlay C (37.37%)**, and **218,404 / 950,332 resolved text (22.98%)**.

`overlay 36 +0x01D0..+0x0694` (`overlay36UpdateInteractiveEntity`) — 1,220 bytes / 305 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via two countdown-CFG corrections plus twelve schedule permutations and a decoded frame/register rewrite); source kept as decomp-permuter input. The object retains all 24 runtime relocation roles. The checkpoint becomes **75,332 / 45,775 campaign bytes (164.57%)**, **176,584 / 469,264 Overlay C (37.63%)**, and **219,624 / 950,332 resolved text (23.11%)**.

`overlay 41 +0x1C84..+0x1DE0` (`overlay41DrawItem`) contributes **348 naturally exact bytes / 87 words**. Ordinary `-O2 -mips2 -32` codegen is exact; only the independent trailing alignment word is trimmed. The object retains all 14 runtime relocation roles. The checkpoint becomes **75,680 / 45,775 campaign bytes (165.33%)**, **176,932 / 469,264 Overlay C (37.70%)**, and **219,972 / 950,332 resolved text (23.15%)**.

`overlay 99 +0x0064..+0x021C` (`overlay99InitializeEntries`) contributes **440 exact C bytes / 110 words**. Declaring the address-taken descriptor after the entry cursor reproduces the retail `sp+0x54` home and exact `0x70` frame. The configured object, all seven runtime relocation roles, linked owned range, overlay, and full ROM are exact.

`overlay 11 +0x0AF4..+0x0C88` (`overlay11InitializeFour`) contributes **404 naturally exact bytes / 101 words**. Ordinary `-O2 -mips2 -32` codegen is exact; only independent section alignment is trimmed, and the link resolves three proved local addends. The object retains all 22 runtime relocation roles. The checkpoint becomes **76,524 / 45,775 campaign bytes (167.17%)**, **177,776 / 469,264 Overlay C (37.88%)**, and **220,816 / 950,332 resolved text (23.24%)**.

`overlay 11 +0x0000..+0x0150` (`overlay11Initialize`) contributes **336 naturally exact bytes / 84 words**. Typed source's compiler-emitted six-entry switch table exactly matches the existing runtime-relocated table at module `+0x2ED8`; the duplicate private section is discarded after rebinding a local `+8` text pair. The object retains all 31 runtime text relocation roles, and the retained table preserves its six `R_MIPS_32` roles. The checkpoint becomes **76,860 / 45,775 campaign bytes (167.91%)**, **178,112 / 469,264 Overlay C (37.96%)**, and **221,152 / 950,332 resolved text (23.27%)**.

`overlay 11 +0x11D0..+0x1398` (`overlay11UpdateSelection`) contributes **456 naturally exact bytes / 114 words**. Ordinary `-O2 -mips2 -32` codegen is exact; only independent section alignment is trimmed. The object retains all 58 runtime relocation roles: ten calls and 24 HI16/LO16 pairs. The checkpoint becomes **77,316 / 45,775 campaign bytes (168.90%)**, **178,568 / 469,264 Overlay C (38.05%)**, and **221,608 / 950,332 resolved text (23.32%)**.

`overlay 19 +0x00AC..+0x01E0` (`overlay19BuildOutput`) contributes **308 naturally exact bytes / 77 words**. Ordinary `-O2 -mips2 -32` codegen is exact; only independent trailing section alignment is trimmed. The object retains all 10 `R_MIPS_26` runtime roles: three local calls and seven resident calls across five semantic imports. The checkpoint becomes **77,624 / 45,775 campaign bytes (169.58%)**, **178,876 / 469,264 Overlay C (38.12%)**, and **221,916 / 950,332 resolved text (23.35%)**.

`overlay 99 +0x02A0..+0x0638` (`overlay99ApplySegment`) — 920 bytes / 230 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a bijective private-representation rewrite, plus removal of a 16-byte compiler-private `.rodata` duplicate already covered by the retained runtime table); source kept as decomp-permuter input. The object covers 13 static and 27 runtime relocation roles. The checkpoint becomes **78,544 / 45,775 campaign bytes (171.59%)**, **179,796 / 469,264 Overlay C (38.31%)**, and **222,836 / 950,332 resolved text (23.45%)**.

`overlay 11 +0x1398..+0x184C` (`overlay11UpdateMenu`) — 1,204 bytes / 301 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a bounded private frame/spill rewrite); source kept as decomp-permuter input. The retained topology is 301 instructions and 102 runtime relocation roles. The checkpoint becomes **79,748 / 45,775 campaign bytes (174.22%)**, **181,000 / 469,264 Overlay C (38.57%)**, and **224,040 / 950,332 resolved text (23.57%)**.

`overlay 63 +0x077C..+0x0928` (`overlay63UpdateSequence`) — 428 bytes / 107 words; the separate `+0x0928..+0x0930` eight-byte zero padding remains assembly with no C credit. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via restoring one folded identity copy and rotating a `v0`/`v1` register pair); source kept as decomp-permuter input. The object retains all 39 runtime relocation roles. The checkpoint becomes **80,176 / 45,775 campaign bytes (175.15%)**, **181,428 / 469,264 Overlay C (38.66%)**, and **224,468 / 950,332 resolved text (23.62%)**.

`overlay 11 +0x184C..+0x1A7C` (`overlay11UpdateTwoOptionMenu`) — 560 bytes / 140 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a two-use loop-index spill-home rewrite); source kept as decomp-permuter input. The object retains all 46 runtime relocation roles. The checkpoint becomes **80,736 / 45,775 campaign bytes (176.38%)**, **181,988 / 469,264 Overlay C (38.78%)**, and **225,028 / 950,332 resolved text (23.68%)**.

`overlay 63 +0x0000..+0x01D4` (`overlay63Initialize`) — 468 bytes / 117 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via restoring a folded chain-address materialization and rotating a bounded register web); source kept as decomp-permuter input. The object retains all 46 runtime relocation roles. The checkpoint becomes **81,204 / 45,775 campaign bytes (177.40%)**, **182,456 / 469,264 Overlay C (38.88%)**, and **225,496 / 950,332 resolved text (23.73%)**.

`overlay 100 +0x0580..+0x094C` (`overlay100DrawMotion`) — 972 bytes / 243 words; its final four-byte zero word remains separately owned padding. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a private representation rewrite); source kept as decomp-permuter input. The five-call relocation contract was retained. Every non-padding executable interval has C source, but `overlay100InitializeMotion` and `overlay100DrawMotion` remain guarded `NON_MATCHING`; Overlay 100 is not exact-closed.

`overlay 100 +0x038C..+0x050C` (`overlay100UpdateMotion`) contributes **384 exact C bytes / 96 words**. Replacing the expanded count/remaining loop with its natural `if (count--)` and `while (count--)` form restores IDO's otherwise-dead post-decrement carrier and closes the prior one-word structural gap. The configured object has the exact `0x18` frame and no padding. Its three runtime records agree by offset, type, identity, and addend: the call at `+0x3C` targets local `overlay100RemoveEntry` (`+0x278`), and the HI16/LO16 pair at `+0xD4/+0xD8` addresses module `+0x984` with addend 4. ORT 1930 exports the function and its sole inbound is local `overlay100ApplyValue+0x44`. The linked owned range, complete overlay, and full ROM are byte-identical.

`overlay 11 +0x1A7C..+0x1E4C` (`overlay11UpdateFiveOptionMenu`) contributes **976 exact bytes / 244 words**. The configured object retains 78 static relocation records, and its separately retained 20-byte switch table is also exact. The checkpoint becomes **83,152 / 45,775 campaign bytes (181.65%)**, **184,404 / 469,264 Overlay C (39.30%)**, and **227,444 / 950,332 resolved text (23.93%)**.

`overlay 63 +0x01D4..+0x074C` (`overlay63UpdateEffects`) contributes **1,400 exact bytes / 350 words**. The configured object retains all 71 static relocation records: 23 calls and 24 HI16/LO16 pairs. The checkpoint becomes **84,552 / 45,775 campaign bytes (184.71%)**, **185,804 / 469,264 Overlay C (39.59%)**, and **228,844 / 950,332 resolved text (24.08%)**.

`overlay 19 +0x01E0..+0x0A30` (`overlay19BuildPlanes`) — 2,128 bytes / 532 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a complete private-representation rewrite); source kept as decomp-permuter input. The retained relocation surface covers four calls. Overlay 19's non-padding executable bytes now all have C source. The checkpoint becomes **86,680 / 45,775 campaign bytes (189.36%)**, **187,932 / 469,264 Overlay C (40.05%)**, and **230,972 / 950,332 resolved text (24.30%)**.

`overlay 98`'s edge owners add **684 exact bytes / 171 words**: `overlay98CollectAccepted` at `+0x0144..+0x0234` and `overlay98CheckObject` at `+0x0848..+0x0A04`. The latter's final 12 zero bytes remain separately owned assembly padding. Each configured object retains all six relocation roles. The checkpoint becomes **87,364 / 45,775 campaign bytes (190.86%)**, **188,616 / 469,264 Overlay C (40.19%)**, and **231,656 / 950,332 resolved text (24.38%)**.

`overlay 65 +0x0080..+0x0BC0` (`overlay65UpdateParticles`) — 2,880 bytes / 720 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a complete private-representation rewrite across the function); source kept as decomp-permuter input. The retained surface covers 36 static relocation records and all 64 runtime roles; Overlay 65's non-padding executable bytes now all have C source. The checkpoint becomes **90,244 / 45,775 campaign bytes (197.15%)**, **191,496 / 469,264 Overlay C (40.81%)**, and **234,536 / 950,332 resolved text (24.68%)**.

`overlay 98 +0x0234..+0x0848` (`overlay98RenderReflections`) — 1,556 bytes / 389 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a private-representation rewrite that converted 36 runtime semantic relocations to a 16-record static proxy surface); source kept as decomp-permuter input. Overlay 98's non-padding executable bytes now all have C source. The checkpoint becomes **91,800 / 45,775 campaign bytes (200.55%)**, **193,052 / 469,264 Overlay C (41.14%)**, and **236,092 / 950,332 resolved text (24.84%)**.

`overlay 13`'s three remaining owners at `+0x0284..+0x0508`, `+0x0580..+0x0874`, and `+0x0874..+0x0B0C` contribute **2,064 exact bytes / 516 words** and close every non-padding executable byte in the module. Their configured objects preserve exact 3-, 10-, and 2-record static surfaces while the runtime ledgers retain all 5, 18, and 10 semantic roles. The checkpoint becomes **93,864 / 45,775 campaign bytes (205.06%)**, **195,116 / 469,264 Overlay C (41.58%)**, and **238,156 / 950,332 resolved text (25.06%)**.

`overlay 11 +0x2714..+0x2948` (`overlay11UpdateModeSix`) — 564 bytes / 141 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a two-use private spill-home rewrite); source kept as decomp-permuter input. Natural codegen owned the exact length, frame, CFG, and all 47 relocation sites. The checkpoint becomes **94,428 / 45,775 campaign bytes (206.29%)**, **195,680 / 469,264 Overlay C (41.70%)**, and **238,720 / 950,332 resolved text (25.12%)**.

`overlay 15 +0x06E8..+0x09E0` (`overlay15InitStars`) — 760 bytes / 190 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via an address-lowering rewrite recovering three explicit global-address carriers); source kept as decomp-permuter input. The 15 runtime roles reduce to an exact 13-record split surface. The checkpoint becomes **95,188 / 45,775 campaign bytes (207.95%)**, **196,440 / 469,264 Overlay C (41.86%)**, and **239,480 / 950,332 resolved text (25.20%)**.

`overlay 15 +0x004C..+0x0428` (`overlay15InitStarsAndPalette`) — 988 bytes / 247 words, closing every non-padding executable byte in the module. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a relocation-anchored carrier rewrite); source kept as decomp-permuter input. All 14 shipped runtime roles reduce to an exact six-record split surface. The checkpoint becomes **96,176 / 45,775 campaign bytes (210.11%)**, **197,428 / 469,264 Overlay C (42.07%)**, and **240,468 / 950,332 resolved text (25.30%)**.

`overlay 61 +0x1648..+0x17B8` (`func_overlay_061_F0001648_18C0A10`) contributes **368 exact C bytes / 92 words**. A disclosed one-iteration grouping preserves IDO's exact 0x38-byte frame and instruction allocation. The object retains all nine calls plus the local path-address pair, and the linked overlay and whole ROM are byte-identical.

`overlay 11 +0x1E4C..+0x22E8` (`func_overlay_011_F0001E4C_186A694`) — 1,180 bytes / 295 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a complete carrier-schedule rewrite); source kept as decomp-permuter input. The retained split text surface is 41 records; the five-entry runtime switch table and its five relocations remain in their original data/rodata owner. The checkpoint becomes **97,724 / 45,775 campaign bytes (213.49%)**, **198,976 / 469,264 Overlay C (42.40%)**, and **242,016 / 950,332 resolved text (25.47%)**.

`overlay 11 +0x22E8..+0x2714` — 1,068 bytes / 267 words, eliminating the former `+0x1E4C..+0x2714` middle deficit. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a complete carrier-schedule rewrite plus a link-only addend rebind); source kept as decomp-permuter input. The pre-link object's split text surface was 39 records. The checkpoint becomes **98,792 / 45,775 campaign bytes (215.82%)**, **200,044 / 469,264 Overlay C (42.63%)**, and **243,084 / 950,332 resolved text (25.58%)**.

`overlay 14 +0x0498..+0x0578` (`overlay14ResetMode`) — 224 bytes / 56 words. NON_MATCHING: bounded 2026-08-28 Mickey-only closeout retained the typed command-loop source after five source-faithful probes. The best candidate preserves the exact boundary, 56-instruction shape, and 0x30 frame; workbench alignment reports 45/56 matching rows, with seven differing words, four opcode mismatches, and two prologue alignment gaps. The first divergence is the `s3` save/setup schedule at `+0x14`; the remaining block is the four target raw-offset stores at `+0xF8`, `+0xD8`, `+0xDC`, and `+0xE0`, which do not have candidate relocation records. The target has 10 relocations versus the candidate's 18, despite rebinding both call sites and the four setup data references to the target identities. `coddog` was unavailable and `skeleton_scan` could not bound the target because the symbol-size entry is absent; no trace oracle was available. No C credit is claimed.

| Overlay | C-owned non-text | Translation unit | Proof |
|---:|---|---|---|
| 15 | data/rodata `+0x0..+0x50`; BSS `+0x0..+0xA0` | `overlay_015.c` | configured object and linked ROM exact |
| 101 | BSS `+0x0..+0xFD0` (shared builder owner) | `func_overlay_101_F000571C_18E0F3C.c` (consumer `func_overlay_101_F00078F4_18E3114.c`) | linked BSS placement exact; both C bodies remain structure-mismatch |

| Overlay | Range | Function | Bytes | Exactness | Donor |
|---:|---|---|---:|---|---|
| 35 | `+0x000..+0x1E0` | `func_overlay_035_F0000000_1881CE0` | 480 | canonical object and linked ROM exact | Mickey-only |

`overlay 20 +0x07C4..+0x09DC` (`overlay20BuildTileCommands`) contributes **536 exact C bytes / 134 words**. Mickey's `0x90` frame proves the seven-entry chunk array; its declaration order reproduces the retail stack home. The configured object, helper relocation, linked owned range, overlay, and full ROM are exact.
`overlay 34 +0x02C8..+0x0378` (`overlay34RemoveRecord`) — 176 bytes / 44 words. NON_MATCHING: bounded 2026-08-28 closeout retained the size-exact source after indexed-compaction, pointer-cursor, separated-count, and scoped-call probes. The best candidate matches 32/44 words with the first schedule/register residual at `+0x14`; it preserves one helper relocation and two repeated active-count HI16/LO16 pairs, while the target encodes its pointer load without relocation. Removing those source pointer relocations is prohibited, so no C credit is claimed.

`overlay 2 +0x0B70..+0x0C90` (`overlay2SplitRegion`) contributes **288 exact C
bytes / 72 words**. All 119 compiler configurations leave the configured
seven-word prologue schedule residual. A fidelity-clean IDO 5.3 `as1 -R`
trace identifies a physical-line tie between the function and loop headers;
placing those token-equivalent headers on one line produces the retail order.
The exact `0x30` frame and all nine runtime relocation tuples and identities
agree: local data at `+0x4/+0x24`, calls to `+0/+0x6E0/+0x49C` twice, and the
recursive `+0xB70` call. The linked owned range, complete overlay, and full ROM
are byte-identical.

`overlay 25 +0x000..+0x17C` (`overlay25InitializeEffect`) contributes **380
exact C bytes / 95 words**. Reading the retained owner through `state->owner`
reproduces the shipped `s0`/`s1` carrier allocation, while
`-Wab,-r4300_mul` supplies the required FP multiply-hazard nop. The exact
`0x40` frame and all nine runtime relocation offsets, types, and identities
agree. The linked owner, complete 1,904-byte overlay, and full ROM are
byte-identical. Pinned DKR and JFG donor rows for Overlay 25 remain empty, so
this is Mickey-derived source and does not support a JFG module mapping.

### Exact public backlog mirrors

The following previously proven owners were independently rebuilt in this
tree. Each configured object, linked owned range, complete overlay, and full
ROM is byte-identical to the US target:

| Overlay | Range | Function | Exact bytes | Object evidence |
|---:|---|---|---:|---|
| 14 | `+0x09F4..+0x0ACC` | `func_overlay_014_F00009F4_18702CC` | 216 | 54 words; exact runtime relocation surface |
| 18 | `+0x0000..+0x01F4` | `overlay18Load` | 500 | 125 words; the runtime identity resolves the 64-bit `osSetTime` argument ABI |
| 68 | `+0x051C..+0x0650` | `overlay68PromoteSecondary` | 308 | 77 words; `0x30` frame; nine relocation sites |
| 84 | `+0x0C9C..+0x0DBC` | `overlay84LoadCurrent` | 288 | 72 words; `0x28` frame; five relocation sites |
| 84 | `+0x1060..+0x11F4` | `overlay84ActivateCurrent` | 404 | 101 words; `0x30` frame; five relocation sites |
