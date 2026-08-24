/*
 * Resident track renderer, collision, and fog code.
 * ROM 0xC950-0x16140 (VRAM 0x8000BD50-0x80015540).
 *
 * PROVENANCE -- TU attribution and reference names come from Jet Force
 * Gemini's public decomp, `src/track.c` and its built `src/track.c.o`. The
 * 66-function Mickey block follows that TU's distinctive order from the
 * update/draw/sky routines through texture scrolling, track lights, collision
 * queries, and fog, ending with the same display-list helper. Mickey's own
 * strings, calls, function boundaries, and bytes decide every disagreement.
 * Adapted bodies keep a PROVENANCE note at their point of use.
 *
 * Flags: -O2 -mips2 -32 (the resident game-code default).
 */

#include "game/track.h"

#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000BD50.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000BDB4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000C400.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000C540.s")
/* PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function `trackSkySet`. */
void trackSkySet(s32 skyDome) {
    D_800C9558 = skyDome;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000C5F4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000CC78.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000CED0.s")
/*
 * JFG's corresponding TU position is `trackGetSky`, but this three-word
 * Mickey function is kept unnamed because it has no adoptable naming tier.
 */
void *func_8000D00C(void) {
    return D_800C9550;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D018.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D16C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D1B8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D3B8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D570.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D62C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D728.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D768.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D7F8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D820.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D978.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000DB34.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000DDE4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000DFBC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000E5EC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000E920.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000F198.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000F57C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000F82C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FA2C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FAE0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FBD8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FCA4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FD68.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FEB4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FEEC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FF2C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80010178.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800103D4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80010654.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80010900.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80010B4C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800115E4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80011980.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80011CDC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80012234.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80012574.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80012658.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8001291C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800131AC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80013324.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800133FC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8001357C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8001398C.s")
/*
 * PROVENANCE: JFG supplies the name `trackGetTrack`; this trivial body is
 * reconstructed from Mickey.
 */
void *trackGetTrack(void) {
    return D_800792E8;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80013EC0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800140CC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80014430.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80014528.s")
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function
 * `trackSetFogOff`. Mickey's tier-A match to JFG's built function and the
 * offsets used below independently validate this body against Mickey's ROM.
 */
void trackSetFogOff(s32 fogIndex) {
    D_800C99C0[fogIndex].addFog.near = 0;
    D_800C99C0[fogIndex].addFog.far = 0;
    D_800C99C0[fogIndex].addFog.r = 0;
    D_800C99C0[fogIndex].addFog.g = 0;
    D_800C99C0[fogIndex].addFog.b = 0;
    D_800C99C0[fogIndex].fog.near = 1018 << 16;
    D_800C99C0[fogIndex].fog.far = 1023 << 16;
    D_800C99C0[fogIndex].intendedFog.r = D_800C99C0[fogIndex].fog.r >> 16;
    D_800C99C0[fogIndex].intendedFog.g = D_800C99C0[fogIndex].fog.g >> 16;
    D_800C99C0[fogIndex].intendedFog.b = D_800C99C0[fogIndex].fog.b >> 16;
    D_800C99C0[fogIndex].intendedFog.near = 1018;
    D_800C99C0[fogIndex].intendedFog.far = 1023;
    D_800C99C0[fogIndex].switchTimer = 0;
    D_800C99C0[fogIndex].fogChanger = NULL;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80014614.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800147A4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800148E0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80014BAC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80014DE4.s")
s32 func_80014EAC(u32 value) {
    s32 result;

    result = -1;
    while (value != 0) {
        result++;
        value >>= 1;
    }
    return result;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80014ECC.s")
