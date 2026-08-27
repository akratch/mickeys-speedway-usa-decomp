# Overlays

## 5. Overlay system

The resident runtime linker loads code and data modules on demand. All overlay
images share the same runtime address space, so an overlay function is
identified by `(overlay, section, offset)`, not by virtual address alone.

The README reports current matched C bytes. This document describes the stable
format and build representation.

### 5.1 Call and load sequence

An unresolved overlay call initially targets `TrapDanglingJump`. The trampoline:

1. saves integer and floating-point argument registers;
2. finds the caller's `jal` address in `mainRelocTable`;
3. uses the entry's index to read `overlayRomTable`;
4. asks `runlinkDownloadCode` to load and relocate the overlay;
5. invalidates the instruction cache; and
6. restores the arguments and transfers control to the loaded function.

`runlinkInit` copies the three linker tables from ROM and creates table entry 0
for the resident image. `runlinkUnloadOverlay` restores affected calls to the
trampoline before freeing an overlay. `runlinkGetAddressInfo` performs the
reverse lookup used by the resident crash reporter.

The function and type names follow Jet Force Gemini where behavior matches.
Mickey's relocation layout and all numeric fields come from Mickey's tables and
code. Point-of-use comments record adapted bodies.

### 5.2 Runtime tables

The resident loader owns these tables:

| Table | Element | Use |
|---|---|---|
| `overlayTable` | `OverlayHeader` | Loaded address and section sizes for each module |
| `mainRelocTable` | `RelocTableEntry` | Resident call sites that target overlays |
| `overlayRomTable` | `RomTableEntry` | Overlay number and symbol offset |
| `linkSlotTable` | `LinkSlot` | Load state for the resident entry and 107 overlays |

Entry 0 in `overlayTable` represents resident code. The ROM contains 107
overlay headers, so the runtime table contains 108 entries.

`RomTableEntry` is a 32-bit value with a 12-bit overlay selector and a 20-bit
offset. Reserved selectors represent local sections. `RelocTableEntry` is
Mickey-specific: the first word is a ROM-table index; the second contains a
24-bit call-site offset from the resident base and an 8-bit relocation flag.

`OverlayHeader` is 32 bytes:

| Offset | Field | Meaning |
|---:|---|---|
| `0x00` | `vramBase` | Loaded address; zero in the shipped header |
| `0x04` | `romAddress` | Offset into the overlay image region |
| `0x08` | `textSize` | Executable bytes |
| `0x0C` | `dataSize` | Initialized data and read-only data |
| `0x10` | `bssSize` | Zero-initialized bytes |
| `0x14` | `relocTableSize` | First relocation table size |
| `0x16` | `relocTableSize2` | Second relocation table size |
| `0x18` | `initFunction` | Initialization entry offset |
| `0x1C` | `resumeFunction` | Resume entry offset |

For every module, the image size equals text plus initialized data plus both
relocation-table sizes. BSS contributes no ROM bytes.

### 5.3 ROM layout

The tables and images are flat and uncompressed:

| ROM start | Size | Contents |
|---|---:|---|
| `0x1848B70` | `0x4` | Main relocation count: 375 |
| `0x1848B74` | `0xBB8` | `RelocTableEntry[375]`, followed by four padding bytes |
| `0x1849730` | `0x1F50` | `RomTableEntry[2004]` |
| `0x184B680` | `0xD60` | `OverlayHeader[107]` |
| `0x184C3E0` | `0xA5C00` | Overlay images |

Each image has this order:

```text
text | data and read-only data | relocation table 1 | relocation table 2
```

The last image ends at ROM `0x18F1FE0`, which is also the start of the final
fill region. The loader copies these bytes directly; it does not call the asset
decompressor.

### 5.4 Canonical map

`config/overlays.us.json` is generated from the runtime tables and is the
canonical overlay map. It records section ranges, BSS sizes, entry points,
imports, exports, resident callers, relocation counts, dependencies, source
ownership, and matching status.

| Item | Total |
|---|---:|
| Overlay headers | 107 |
| Non-empty modules | 106 |
| Text | 469,264 bytes |
| Initialized data and read-only data | 61,312 bytes |
| BSS | 77,680 bytes |
| Module relocation records | 18,542 |
| Cross-overlay relocations | 608 |
| Directed dependency edges | 97 |

Overlay 32 is a real empty header and has no generated code segment. The other
106 overlays have buildable Splat code segments.

The build uses `0xF0000000` as a synthetic link address. This value is only a
build aid. It is shared by all overlays and must not appear as a unique runtime
identity. `subalign: 1` preserves the shipped section boundaries.

Run these checks after changing the map:

```sh
gmake overlay-atlas          # compare generated JSON and YAML projection
gmake overlay-atlas-write    # update both generated views
gmake overlay-donors         # check the recorded reference results
```

### 5.5 Source ownership

The long-term layout is one main C translation unit per overlay, with shared
headers where several functions use the same types. A temporary function file
may be used while establishing a first match. Consolidation must preserve:

- text order and alignment;
- initialized-data and read-only-data order;
- BSS size and symbol placement;
- relocation count, type, offset, and target; and
- complete linked ROM bytes.

The atlas uses half-open section-offset ranges to assign ownership. Never infer
ownership from a synthetic ELF address.

### 5.6 Reference results

`config/overlay-donors.us.json` records an exhaustive exact-object scan against
the locked DKR v77, DKR v80, and JFG builds. Most overlays have no exact donor.
Useful exact results include:

- DKR `alSeqFileNew` at the start of overlay 5;
- JFG `refractOutput` in overlay 49; and
- JFG `osRamTest4_6105` for the complete text of overlay 107.

Generated placeholder names are not adopted. A subsystem resemblance or a
string cross-reference is recorded as semantic evidence and must not be
reported as an exact object match.

### 5.7 Matching requirements

For each promoted overlay function, compare the exact owned byte range and its
relocations in both the object and linked image. Check adjacent padding and
already matched ranges separately. A function remains `NON_MATCHING` when its C
body compiles but differs in any instruction or required relocation.

Do not edit instruction words after compilation. Do not change a global flag,
shared header, or overlay data layout to improve one candidate without checking
every affected object.
