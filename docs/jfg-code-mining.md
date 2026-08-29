# Jet Force Gemini code mining guide

Mickey's Speedway USA and Jet Force Gemini share parts of the same engine.
This page lists JFG functions that may be able to use C source from this
repository.

The comparison originally used JFG `master` at `2f49ab3f`. It was rechecked
against
[`fd910f6b`](https://github.com/Ryan-Myers/Jet-Force-Gemini/commit/fd910f6bd61e4033e0d0208d763addd32fbb6118)
on 29 August 2026. At the newer revision, all 33 JFG functions in the exact
list below still use `GLOBAL_ASM`.

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
[`src/gsSnd.c`](https://github.com/Ryan-Myers/Jet-Force-Gemini/blob/fd910f6bd61e4033e0d0208d763addd32fbb6118/src/gsSnd.c).

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
`mainUpdateZBCheck`, and `frontSetMode`. Their source is
in [`sched.c`](../src/main/sched.c),
[`audio_manager_1050.c`](../src/main/audio_manager_1050.c),
[`spranim.c`](../src/main/spranim.c), [`main.c`](../src/main/main.c),
[`runlink.c`](../src/main/runlink.c), and [`menu.c`](../src/main/menu.c).
Treat these as source references, not matching implementations.

Eight more Mickey functions have become exact C since this guide was first
published, while their JFG counterparts still use `GLOBAL_ASM` at `fd910f6b`.
They implement the same broad jobs, but their sizes or instruction streams
differ, so they do not change the 33-function / 9,648-byte exact-cross-title
tally.

The “JFG insertion point” links go directly to the unresolved pragma that the
Mickey source can help replace. These eight are source references rather than
drop-in matches: copy the algorithm into that JFG location, translate the
listed engine types and globals, and prove it against JFG's own function bytes
and relocations.

| JFG insertion point | Exact Mickey source | Where the source is useful |
|---|---|---|
| [`src/font.c`: `fontGetLine`](https://github.com/Ryan-Myers/Jet-Force-Gemini/blob/fd910f6bd61e4033e0d0208d763addd32fbb6118/src/font.c#L547) | [`func_8004D40C`](../src/main/font.c#L1110) | Use the line-scanning, glyph-width, wrapping, and output-cursor control flow. JFG's four-argument declaration differs from Mickey's five-argument ABI, so reconcile its width/result handling rather than copying the signature. |
| [`src/audio_manager_36D0.c`: `func_80003B14_4714`](https://github.com/Ryan-Myers/Jet-Force-Gemini/blob/fd910f6bd61e4033e0d0208d763addd32fbb6118/src/audio_manager_36D0.c#L123) | [`func_80003760`](../src/main/audio_manager_36D0.c#L667) | Use the audio-point queue unlink and free-list update. Map JFG's queue head, entry links, and point ownership fields. |
| [`src/particles.c`: `func_8005FE4C_60A4C`](https://github.com/Ryan-Myers/Jet-Force-Gemini/blob/fd910f6bd61e4033e0d0208d763addd32fbb6118/src/particles.c#L61) (`func_800608EC` in kiosk) | [`func_8003F5F8`](../src/main/particles.c#L937) | Use the particle-emitter initialization sequence and flag-dependent field setup. Translate the particle, emitter, trigger-slot, and config layouts before matching. |
| [`src/fx.c`: `func_8006DF90_6EB90`](https://github.com/Ryan-Myers/Jet-Force-Gemini/blob/fd910f6bd61e4033e0d0208d763addd32fbb6118/src/fx.c#L126) | [`func_8004A380`](../src/main/fx.c#L1965) | Use the integer-to-digits conversion, minimum-width handling, and digital-glyph render loop. Replace Mickey's screen, texture, and display-list globals with JFG's equivalents. |
| [`src/hit.c`: `hitInitObjectHit`](https://github.com/Ryan-Myers/Jet-Force-Gemini/blob/fd910f6bd61e4033e0d0208d763addd32fbb6118/src/hit.c#L13) | [`func_80053550`](../src/main/anim.c#L1001) | Use the object-hit allocation and complete field-initialization order. Reconcile JFG's hit-owner, rotation, radius/height, collision-type, and flag fields. |
| [`src/charControl.c`: `func_8002ED94_2F994`](https://github.com/Ryan-Myers/Jet-Force-Gemini/blob/fd910f6bd61e4033e0d0208d763addd32fbb6118/src/charControl.c#L45) | [`func_8001BE0C`](../src/main/charControl.c#L190) | Use the character/player and camera-state initialization block. Confirm every JFG actor, player, and camera offset before adopting the stores. |
| [`src/runLink.c`: `runlinkEnsureJumpIsValid`](https://github.com/Ryan-Myers/Jet-Force-Gemini/blob/fd910f6bd61e4033e0d0208d763addd32fbb6118/src/runLink.c#L545) | [`func_800320F0`](../src/main/runlink.c#L565) | Use the relocation scan, jump validation, and lazy-overlay-load flow. Map JFG's overlay table, module identifiers, relocation records, and jump-address type; this is the most directly relevant overlay-system reference. |
| [`src/overlays/o26/overlay_26.c`: `partSetupLib`](https://github.com/Ryan-Myers/Jet-Force-Gemini/blob/fd910f6bd61e4033e0d0208d763addd32fbb6118/src/overlays/o26/overlay_26.c#L9) and the [kiosk counterpart](https://github.com/Ryan-Myers/Jet-Force-Gemini/blob/fd910f6bd61e4033e0d0208d763addd32fbb6118/src/overlays_kiosk/o26/overlay_26.c#L9) | [`overlay31InitializeBuffers`](../src/overlays/o031/overlay31InitializeBuffers.c#L44) | Use this in JFG's particle-library setup path: capacity defaults, vertex-buffer sizing, four renderer configurations, point/line pools, typed dummy-asset loading, and effect-record initialization. **Evidence: strong structural lead.** Mickey's 245-word C is compiler- and ROM-exact; JFG's 259-word `partSetupLib` has the same `0x48` frame and is the uniquely close JFG function by masked four-instruction shape (0.415 similarity; the next candidate is 0.061). **Limits:** this is not cross-title exact. JFG has a 56-byte-longer body, different tail initialization, and project-specific globals, callees, and record layouts. Port it phase by phase into overlay 26 and verify JFG's own relocations and ROM. |

### Latest-pass exclusions

The same 29 August audit did not find another release-grade JFG source lead:

- `overlay43FilterImage` has a strong structural counterpart in JFG overlay 4
  (`func_overlay_4_000015A8_1EF7898`, 0.574 masked-shape similarity), and
  Mickey's `func_8003A2C8` has the expected front-end role beside JFG's
  `frontSetScreenMode`. Both Mickey bodies still remain guarded
  `NON_MATCHING` plateaus, however, so this guide does not recommend either as
  contribution source yet.
- `overlay97InitScale`, `overlay34InitStorage`, and `overlay40FadeRecords` have
  no qualified JFG counterpart. Their best masked-shape scores are 0.090,
  0.065, and 0.054 respectively, with no corroborating subsystem or call-role
  evidence.
- The new exact C for `overlay41AdvanceStepRecords`, `overlay80UpdateContact`,
  `overlay21ApplyPriorities`, and
  `func_overlay_061_F0001648_18C0A10` also has no concrete JFG integration
  point. Their best JFG scores range from 0.052 to 0.113 and the pinned exact
  donor scan is negative. Those values are treated as generic instruction
  overlap, not reusable code evidence.

JFG has since implemented `runlinkFlushModules`, so it is no longer included
among the open source-reference targets above.

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
