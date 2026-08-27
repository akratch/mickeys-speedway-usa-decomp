# Resident code

## 3. Resident segment

The resident image occupies ROM `0x001000`–`0x086640` and VRAM
`0x80000400`–`0x80085A40`. Its BSS follows the loaded image. This code remains
available while overlays are loaded and unloaded.

The README reports current function and byte progress. This document records
the stable source map and the evidence behind its main boundaries.

### 3.1 Major translation units

| ROM range | Source area | Main role | Boundary evidence |
|---|---|---|---|
| `0x001000` onward | boot and early engine code | Startup, audio, track, lighting, and object support | Entry flow, strings, and reference objects |
| `0x021DA0`–`0x02A250` | `main/main`, `main/joy`, `main/level` | Main loop, input, and level state | Call graph, file strings, and ordered functions |
| `0x02A250`–`0x02AE44` | `main/math_util` | Matrix, vector, and random-number helpers | Reference object matches |
| `0x02BCD0`–`0x02C8C0` | `main/memory` | Heap allocation | Call graph and exact leaf matches |
| `0x02C8C0`–`0x0323A0` | `main/saves`, `main/pi`, `main/screen`, `main/rcpFast3d`, `main/sched` | Storage, DMA, display tasks, and scheduling | Call graph, strings, and reference functions |
| `0x0323E0`–`0x033FA0` | `main/runlink` | Overlay loading and relocation | JFG correspondence and Mickey's table use |
| `0x033FA0`–`0x034180` | `main/trapDanglingJump` | Overlay call trampoline | Whole-object identity |
| `0x034180`–`0x034E60` | `main/gameVi` | Video modes and framebuffers | Complete ordered call and data use |
| `0x039350`–`0x03B1A0` | `main/menu` | Resident front-end code | Strings, callers, and reference function order |
| `0x03B480`–`0x03D5F0` | `main/weather` | Weather and environmental effects | Call graph and global use |
| `0x03D5F0`–`0x043470` | `main/particles` | Particle systems | Call graph and exact internal functions |
| `0x043470`–`0x047A60` | `main/diprint`, `main/diRcp`, `main/diCpu` | Text formatting, display-list inspection, and crash reporting | Distinct strings and call graph |
| `0x04BC40`–`0x04E1E0` | `main/font` | Font state and text drawing | Ordered JFG functions and exact leaves |
| `0x04EA60`–`0x04F4D4` | `main/gzip_asm` | Asset decompression | Whole-object identity |
| `0x04FC30`–`0x0505E0` | `libultra/exceptasm` | Exception and thread dispatch | Whole-object identity |
| `0x050C00`–`0x058570` | animation, camera, model, and object code | Shared game systems | Call graph, strings, and reference functions |
| `0x058E50`–`0x059B90` | `main/vehicle_sounds` | Positional vehicle audio | Call graph and data flow |
| `0x05B300`–`0x05C310` | `main/models_5B300` | Model and animation storage | Call graph and one exact camera function |
| `0x05C310`–`0x05E6B0` | `main/gsSnd` | Sound player | Whole-object identity |
| `0x05E6B0`–`0x06AF90` | `libultra/n_*` | `n_audio` synthesis | Consecutive reference object matches |
| `0x06B3D0`–`0x06F3E0` | `libultra` device code | Transfer Pak, Rumble Pak, and Controller Pak | JFG and Perfect Dark objects |
| `0x06F420`–`0x076D10` | `libultra` corridor | Core SDK routines | Whole-object matches from several references |
| `0x076E60`–`0x086640` | resident data and read-only data | Globals, strings, constants, and jump tables | Relocations and section use |

These ranges describe current source ownership. Only rows explicitly described
as whole-object identity are measured original file boundaries. Other rows are
working translation-unit boundaries supported by the listed evidence.

### 3.2 Resident modules

Strings embedded by assertions show four source-module names:

| Name | Resident evidence | Interpretation |
|---|---|---|
| `main` | Several resident references | Fully resident core code |
| `track` | Resident assertion references | Some track code is resident |
| `front` | Resident assertion references | Some front-end code is resident |
| `clone` | Strings occur in overlay 43 data and have no resident reference | Overlay-only task code |

These names describe original source groups, not overlay boundaries.

### 3.3 Runtime linker

`src/main/runlink.c` and `src/main/trapDanglingJump.c` implement the loader that
connects resident calls to overlay functions. The call trampoline identifies a
call site in the resident relocation table, loads the required overlay, applies
relocations, restores the saved arguments, and transfers control to the loaded
function.

The implementation follows JFG's published runtime linker where the behavior
agrees and carries point-of-use provenance. Mickey differs in relocation-entry
layout, table sizes, allocation tags, and some control flow. Those differences
are represented from Mickey's own bytes. See [Overlays](overlays.md).

### 3.4 Data ownership

The resident data tail is still only partly assigned to translation units.
Move data or read-only data into a C object only when all of these agree:

- the target object's section size and order;
- every relocation into and out of the range;
- alignment and padding;
- references from the owning functions; and
- the linked ROM bytes.

Keep function order in a partly decompiled translation unit equal to ROM order.
Reordering `GLOBAL_ASM` includes changes link placement even when each function
body is unchanged.

BSS symbols may be named inside an existing anonymous BSS range when their
addresses and sizes are established by relocations and adjacent symbols. BSS
has no ROM bytes, so a successful ROM comparison alone does not prove a BSS
boundary.

### 3.5 Compiler groups

Resident game code normally uses `-O2 -mips2 -32`. libultra contains several
measured compiler groups, including `-O1 -mips2`, `-O2 -g3 -mips2`, bare
`-g -mips2`, and one `-mips3` file. The Makefile lists the files in each group.

Do not copy a flag from a nearby function without checking the target object.
Run the documented flag sweep when a natural candidate has the wrong frame,
instruction scheduling, or floating-point form.

### 3.6 Remaining work

The main resident tasks are:

- split the remaining unnamed assembly into defensible translation units;
- reconstruct the two unmatched ranges in the libultra corridor;
- complete C bodies already kept under `NON_MATCHING`;
- assign data and read-only data using object and relocation evidence; and
- replace generated function names only when a documented evidence level
  supports a real name.

Do not infer an original file boundary from one matching function. Preserve the
current assembly fallback until the compiled object and full ROM both match.
