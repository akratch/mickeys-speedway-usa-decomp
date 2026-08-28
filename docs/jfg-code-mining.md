# Jet Force Gemini code mining guide

Mickey's Speedway USA and Jet Force Gemini share parts of the same engine.
This page lists JFG functions that may be able to use C source from this
repository.

The comparison used JFG `master` at
[`2f49ab3f`](https://github.com/Ryan-Myers/Jet-Force-Gemini/commit/2f49ab3f7437f0b7ea8ed8da83e9f765b95fe4ff).
At that revision, the JFG functions below still use `GLOBAL_ASM`.

## What the exact list means

For 33 functions, covering 9,648 bytes of JFG code, the compiled Mickey C has:

- the same function size;
- the same instruction bits outside linker-controlled fields; and
- the same relocation count, types, and offsets inside the function.

This is strong evidence that the source is useful to JFG. It is not a finished
JFG match. Names, types, globals, and relocation targets must be changed to the
JFG versions. The result must then be compiled and verified in JFG.

## Best place to start: `gsSnd.c`

Nine JFG sound-player functions have exact candidates in
[`src/main/gsSnd.c`](../src/main/gsSnd.c). Together they cover 7,800 bytes.
JFG's missing functions are at the top of its
[`src/gsSnd.c`](https://github.com/Ryan-Myers/Jet-Force-Gemini/blob/2f49ab3f7437f0b7ea8ed8da83e9f765b95fe4ff/src/gsSnd.c).

| JFG function | Mickey function | Bytes |
|---|---|---:|
| `gsSndpNew` | `gsSndpNew` | 616 |
| `func_80084468_85068` | `func_8005B978` | 200 |
| `func_80084530_85130` | `func_8005BA40` | 4,860 |
| `func_8008582C_8642C` | `func_8005CD3C` | 112 |
| `func_8008589C_8649C` | `func_8005CDAC` | 124 |
| `func_80085918_86518` | `func_8005CE28` | 260 |
| `func_80085B20_86720` | `func_8005D030` | 560 |
| `func_80085D50_86950` | `func_8005D260` | 324 |
| `ad_sndp_play` | `ad_sndp_play` | 744 |

Both projects compile this file with `-g -mips2`, which removes one common
source of code-generation differences. The 4,860-byte event handler also owns
a switch table, so it is best handled after the smaller functions.

## Other exact candidates

The links below point to the C in this repository. Search for the listed
function name in each file.

- Audio manager, 412 bytes:
  [`audio_manager_1050.c`](../src/main/audio_manager_1050.c) contains
  `amTuneSetFadeScaled`, `amTuneSetChlVolume`, `amSndSetPan`, and `forcelink`.
  [`audio_manager_4C50.c`](../src/main/audio_manager_4C50.c) contains
  `amVibratoInit`.
- Display setup, 248 bytes:
  [`rcpFast3d.c`](../src/main/rcpFast3d.c) contains `rcpInit` and
  `bgdraw_fillcolour`. The second function corresponds to JFG's
  `rcpSetBorderColour`.
- Front end, 368 bytes:
  [`menu.c`](../src/main/menu.c) contains `frontDrawRectangle`,
  `setupFrontEndObject`, `frontGetScreenMode`, and
  `frontGetLevelScreenMode`.
- Animation, 208 bytes:
  [`anim.c`](../src/main/anim.c) contains `animseqInitGroup`, `animseqPlay`,
  `func_8005017C`, and `func_8005027C`. The two unnamed functions correspond
  to JFG's `func_80076198_76D98` and `func_800762A0_76EA0`.
- Player control, 140 bytes:
  [`charControl.c`](../src/main/charControl.c) contains
  `controlSetPlayerSetup` and `func_8001C2D4`. The latter corresponds to
  JFG's `func_80031F9C_32B9C`.
- Models, 164 bytes:
  [`models.c`](../src/main/models.c) contains `func_8001FB64` and
  `func_80020AD4`. They correspond to JFG's `func_8003C0C4_3CCC4` and
  `func_8003E35C_3EF5C`.
- Particles, 48 bytes:
  [`particles.c`](../src/main/particles.c) contains `reset_particles`, which
  corresponds to JFG's `partFreeLib`.
- Track, 32 bytes:
  [`track.c`](../src/main/track.c) contains `func_80014EAC`, which corresponds
  to JFG's `func_8001C758_1D358`.
- Overlays, 228 bytes:
  [`overlay_049.c`](../src/overlays/o049/overlay_049.c) contains
  `refractOutput` for JFG overlay 2;
  [`overlay_005.c`](../src/overlays/o005/overlay_005.c) contains
  `overlay5InitSequence` for JFG overlay 25 function
  `func_overlay_25_000002D0_1F43BA8`; and
  [`overlay_016.c`](../src/overlays/o016/overlay_016.c) contains
  `overlay16BuildGradient` for JFG overlay 27 function
  `func_overlay_27_00000000_1F45710`.

`osRamTest4_6105` in
[`overlay_107.c`](../src/overlays/o107/overlay_107.c) is one more useful
candidate. Its instruction shape matches JFG's 40-byte function, but the
Mickey source uses the address directly. JFG needs a reference to its
`D_A00002E8` symbol so the expected `HI16` and `LO16` relocations are kept.

JFG already has disabled C bodies for `_depth2Cents` and `diCpuTraceInit`.
The matching Mickey versions are in
[`audio_manager_4C50.c`](../src/main/audio_manager_4C50.c) and
[`diCpu.c`](../src/main/diCpu.c), but JFG should start with its own existing C
for those functions.

## Related source that is not exact

Several same-name functions are close between the two games but are not byte
matches. Useful starting points include `osScGetTaskType`,
`amWaitForMidiSync`, `texscrollControl`, `spranimControl`, `mainCPUeffects`,
`mainUpdateZBCheck`, `runlinkFlushModules`, and `frontSetMode`. Their source is
in [`sched.c`](../src/main/sched.c),
[`audio_manager_1050.c`](../src/main/audio_manager_1050.c),
[`spranim.c`](../src/main/spranim.c), [`main.c`](../src/main/main.c),
[`runlink.c`](../src/main/runlink.c), and [`menu.c`](../src/main/menu.c).
Treat these as source references, not matching implementations.

## Suggested porting order

1. Start with a small function and replace Mickey globals and types with JFG
   names.
2. Compile the complete JFG translation unit with its normal flags.
3. Compare the function bytes and every relocation with the JFG target.
4. Check the linked function range and rebuild the full JFG ROM.
5. Move to the larger `gsSnd` functions after the shared structures and
   globals are correct.

Point-of-use comments in the Mickey source record where a body or idea came
from. Keep appropriate provenance when moving code between projects. The
repository is published under [CC0](../LICENSE).
