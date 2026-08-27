# Module map

This document records the main code regions and the evidence used for adopted
names. ROM offsets refer to the US image. For the resident segment,
`VRAM = ROM + 0x7FFFF400`.

## 1. Evidence for names

Every adopted name has one of four evidence levels. State the level in this
document and in `symbol_addrs.us.txt`.

| Level | Evidence | Allowed name |
|---|---|---|
| A | Relocation-aware byte identity with a named object from a permitted reference build | The reference name |
| B | Callers, arguments, return use, and data flow establish the role | A descriptive name, or a reference name when the role is exact |
| C | A distinctive string and matching use identify the same routine in a permitted reference | The reference name |
| D | Structure visible only in Mickey's code establishes a limited role | A descriptive project name |

Level A is a measurement. Levels B through D are interpretations and can be
revised when stronger evidence appears.

### 1.1 Byte-identity threshold

Adopt a level-A name only when:

- at least six instruction words remain after masking linker relocations;
- the fixed words occur once in the complete ROM; and
- the reference bytes identify one function name.

Shorter functions need a separate argument based on unique behavior or exact
local context. The few accepted exceptions are documented beside their symbol
entries. A whole-translation-unit match is stronger than an isolated function
match and defines a measured file boundary.

Generated `func_` names from another project are not adopted. Similar size,
similar control flow, or a shared string without matching use is not level A.

### 1.2 Provenance

The permitted reference projects are Diddy Kong Racing, Jet Force Gemini,
Perfect Dark, Banjo-Kazooie, and Conker's Bad Fur Day. Their exact commits,
build results, and checksums are in [Reference builds](references.md).

Most newly adopted translation units came from Jet Force Gemini objects.
Perfect Dark supplied the three Transfer Pak names. BK and Conker contributed **none** of the adopted names; the pass recorded corroboration of **2** and **8** of the adopted translation units, and re-confirmation of **73** and **65** subsegments. Corroboration and re-confirmation are different counts.

Borrowed bodies and symbol blocks carry a point-of-use `PROVENANCE` note.
Mickey's bytes and call graph remain authoritative. See the
[clean-room policy](CLEANROOM.md).

### 1.3 Naming style

Keep established names in their original spelling so they remain searchable.

| Code | Style |
|---|---|
| libultra | Official SDK spelling, such as `osRecvMesg` |
| Shared Rare engine code | Existing DKR or JFG spelling |
| Runtime linker | Existing `PascalCase` and `runlinkCamelCase` names |
| Mickey-specific descriptive names | `PascalCase` functions and `lowerCamel` data |
| Types | `PascalCase` |
| Macros | prefixed `SCREAMING_SNAKE_CASE` |

Do not rename a borrowed symbol only to make the repository more uniform.

## 2. Top-level ROM map

| Range | Size | Contents | Evidence |
|---|---:|---|---|
| `0x000000`–`0x000040` | `0x40` | ROM header | Standard Nintendo 64 layout |
| `0x000040`–`0x001000` | `0xFC0` | IPL3 boot code | Standard Nintendo 64 layout |
| `0x001000`–`0x086640` | `0x85640` | Resident code and data | Splat segment and BSS initialization |
| `0x086640`–`0x087000` | `0x9C0` | Unidentified offset table | Current binary segment |
| `0x087000`–`0x16B0000` | 22.16 MiB | Compressed assets | Entropy and loader references |
| `0x16B0000`–`0x1848B70` | 1.60 MiB | Unclassified data | No resident reference found |
| `0x1848B70`–`0x184C3E0` | `0x3870` | Runtime-linker tables | Parsed table sizes and uses |
| `0x184C3E0`–`0x18F1FE0` | 0.65 MiB | 107 overlay images | Runtime-linker headers |
| `0x18F1FE0`–`0x2000000` | 7.06 MiB | Fill | Direct byte check |

The resident ROM range `0x001000`–`0x086640` maps to VRAM
`0x80000400`–`0x80085A40`. The entrypoint clears BSS beginning at the latter
address, which fixes the end of the resident image.

The build stamp near the resident data tail contains version `1.1153`, date
`18/08/00 13:08`, and tag `pmountain`. It is descriptive build data and does
not establish a source boundary.

## 3. Resident code

The resident segment is always loaded. It contains boot and main-loop code,
input, video, scheduling, save-device support, the runtime linker, debugging
helpers, game systems, and libultra.

[Resident code](resident.md) gives the current high-level translation-unit map.
The README contains the generated current progress figures.

## 4. libultra

### 4.1 Contiguous corridor

The main libultra corridor is ROM `0x6F420`–`0x76D10`, VRAM
`0x8006E820`–`0x80076110`, and `0x78F0` bytes long. It contains **95 named subsegments, all with measured file boundaries, and 123 named functions**.

The unnamed remainder is `0xB50`, **9.4% of the corridor**, in two ranges:

| ROM range | Size | Position |
|---|---:|---|
| `0x70AF0`–`0x70E20` | `0x330` | Between `dpsetstat` and `pfsdeletefile` |
| `0x74090`–`0x748B0` | `0x820` | Between `timerintr` and `vigetcurrcontext` |

Neither range matches a complete object in the five locked reference builds.
They need direct reconstruction, not more exact-object scanning.

### 4.2 Other libultra regions

libultra also appears below the corridor:

| ROM range | Contents | Evidence |
|---|---|---|
| `0x4FC30`–`0x505E0` | Exception handler and thread dispatcher | JFG whole-object match |
| `0x5E6B0`–`0x6AF90` | `n_audio` synthesis library with two interleaved maths units | JFG object matches |
| `0x6B3D0`–`0x6F3E0` | Transfer Pak, Rumble Pak, and Controller Pak code | JFG and Perfect Dark object matches |

Not every SDK-shaped candidate is adopted. Boundaries and names must pass the
same uniqueness and relocation checks as game code.

## 5. Overlays

The resident runtime linker loads 107 overlay records; overlay 32 has an empty
text image. All non-empty images are represented in the build. The canonical
layout is `config/overlays.us.json`, generated from the shipped linker tables.

[Overlays](overlays.md) describes the file format, runtime behavior, totals,
and identity rules. [Overlay graph](overlay-graph.md) describes the import
graph.

## 6. Compiler configuration

The common flags are `-O2 -mips1 -32`. Resident game code and most overlays use
`-mips2 -32`. Measured exceptions include:

- libultra groups using `-O1 -mips2`, `-O2 -g3 -mips2`, bare `-g -mips2`, or
  phase-specific optimization;
- `libultra/ll.c` using `-O1 -mips3 -32` with an ELF flag correction;
- selected game units using `-Wab,-r4300_mul` or disabled loop unrolling; and
- the overlay 5 audio-bank code using `-O3 -mips2`.

The Makefile is authoritative for each file. A source header comment explains
every non-default flag group. Change a shared flag only after checking all
objects that use it.

Metadata-only post-processing is listed in
`config/postprocess-audit.us.json`. Instruction changes are prohibited by
[ADR 0002](adr/0002-no-post-compile-instruction-editing.md).

## 7. Unmapped areas

The current map does not claim:

- original source-file boundaries inside most overlays;
- the contents of ROM `0x16B0000`–`0x1848B70`;
- internal formats for most assets in `0x087000`–`0x16B0000`;
- the exact purpose of the table at `0x086640`–`0x087000`; or
- source ownership for the remaining resident assembly blocks.

Record new claims only with an evidence level and a reproducible address or
object comparison.
