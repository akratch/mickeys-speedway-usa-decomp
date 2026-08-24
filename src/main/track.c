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
#include "game/math.h"
#include "n_audio/mbi.h"

typedef struct TrackRotation {
    s16 x;
    s16 y;
    s16 z;
} TrackRotation;

typedef struct TrackLocalTransform {
    s16 xRotation;
    s16 yRotation;
    s16 zRotation;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
} TrackLocalTransform;

typedef struct TrackCachedPoint {
    s32 x;
    s32 y;
    s32 z;
} TrackCachedPoint;

typedef struct TrackFloatRecord {
    f32 x;
    f32 y;
    f32 z;
    f32 unkC;
} TrackFloatRecord;

extern TrackRotation *D_800C9530;
extern TrackCachedPoint D_800C9B40;
extern Gfx *D_800C9520;
extern s32 D_80079314;
extern u32 D_800C9B50[16];
extern s32 D_800792FC;

void func_8002AB78(TrackLocalTransform *transform, MtxF matrix);
void mtxf_transform_point(MtxF matrix, f32 x, f32 y, f32 z,
                          f32 *outX, f32 *outY, f32 *outZ);

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
void func_8000D16C(s32 arg0, s32 arg1, s32 arg2) {
    if (D_80079314 < 16) {
        D_800C9B50[D_80079314] =
            (arg0 << 24) | ((arg1 & 0xFFF) << 12) | (arg2 & 0xFFF);
        D_80079314++;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D1B8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D3B8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D570.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D62C.s")
void func_8000D728(TrackFloatRecord *arg0) {
    if ((arg0 != NULL) && (arg0->unkC != 0.0f)) {
        arg0->unkC = 0.0f;
        D_800792FC--;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D768.s")
void func_8000D7F8(TrackFloatRecord *arg0, f32 arg1, f32 arg2, f32 arg3) {
    if (arg0 != NULL) {
        arg0->x = arg1;
        arg0->y = arg2;
        arg0->z = arg3;
    }
}
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
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function
 * `trackSetFog`. Mickey's function boundary and fog-data accesses are
 * authoritative where the revisions differ.
 */
void trackSetFog(s32 fogIndex, s16 near, s16 far, s16 targetNear,
                 u8 red, u8 green, u8 blue, s8 state) {
    s32 tempNear;
    TrackFog *fogData;

    fogData = &D_800C99C0[fogIndex];

    if (far < near) {
        tempNear = near;
        near = far;
        far = tempNear;
    }

    if (far > 1023) {
        far = 1023;
    }
    if (near >= far - 5) {
        near = far - 5;
    }

    fogData->addFog.near = 0;
    fogData->addFog.far = 0;
    fogData->addFog.r = 0;
    fogData->addFog.g = 0;
    fogData->addFog.b = 0;
    fogData->fog.r = red << 16;
    fogData->fog.g = green << 16;
    fogData->fog.b = blue << 16;
    fogData->fog.near = near << 16;
    fogData->fog.far = far << 16;
    fogData->initialNear = near << 16;
    fogData->targetNear = targetNear << 16;
    fogData->intendedFog.state = state;
    fogData->intendedFog.r = red;
    fogData->intendedFog.g = green;
    fogData->intendedFog.near = near;
    fogData->intendedFog.far = far;
    fogData->switchTimer = 0;
    fogData->fogChanger = NULL;
    fogData->intendedFog.b = blue;
}
/*
 * PROVENANCE: adapted from the direct fog-data path in Jet Force Gemini's
 * public `src/track.c`, function `trackGetFog`. Mickey omits JFG's overlay
 * special cases; Mickey's function boundary and accesses are authoritative.
 */
void trackGetFog(s32 playerID, s16 *near, s16 *far, s16 *targetNear,
                 u8 *red, u8 *green, u8 *blue, s8 *state) {
    TrackFog *fogData;

    fogData = &D_800C99C0[playerID];
    *near = fogData->fog.near >> 16;
    *far = fogData->fog.far >> 16;
    *targetNear = fogData->targetNear >> 16;
    *red = fogData->fog.r >> 16;
    *green = fogData->fog.g >> 16;
    *blue = fogData->fog.b >> 16;
    *state = fogData->intendedFog.state & 0x7F;
}
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
/*
 * PROVENANCE: JFG's corresponding track.c placeholder supplies strong
 * skeleton and TU-position context. The body is reconstructed from Mickey's
 * TrackFog accesses and control flow; no JFG placeholder name is imported.
 */
void func_80014614(s32 fogCount, s32 updateRate) {
    TrackFog *fogData;
    s32 fogIndex;
    s8 state;

    fogIndex = 0;
    if (fogCount > 0) {
        fogData = D_800C99C0;
        do {
            state = fogData->intendedFog.state;
            fogIndex++;
            if (state > 0) {
                fogData->fog.near +=
                    (updateRate * (state & 0x7F)) << 11;
                if (fogData->targetNear < fogData->fog.near) {
                    fogData->fog.near =
                        (fogData->targetNear - fogData->fog.near) +
                        fogData->targetNear;
                    fogData->intendedFog.state = state | 0x80;
                }
            } else if (state < 0) {
                fogData->fog.near -=
                    (updateRate * (state & 0x7F)) << 11;
                if (fogData->fog.near < fogData->initialNear) {
                    fogData->fog.near =
                        (fogData->initialNear - fogData->fog.near) +
                        fogData->initialNear;
                    fogData->intendedFog.state = state ^ 0x80;
                }
            } else {
                if (fogData->switchTimer > 0) {
                    if (updateRate < fogData->switchTimer) {
                        fogData->fog.r += fogData->addFog.r * updateRate;
                        fogData->fog.g += fogData->addFog.g * updateRate;
                        fogData->fog.b += fogData->addFog.b * updateRate;
                        fogData->fog.near += fogData->addFog.near * updateRate;
                        fogData->fog.far += fogData->addFog.far * updateRate;
                        /* The volatile cast forces IDO to re-load switchTimer
                         * from memory immediately before the subtraction,
                         * instead of reusing the value it already has in a
                         * register from the `switchTimer > 0` and
                         * `updateRate < switchTimer` comparisons above. That
                         * reload is what the target's instruction schedule
                         * requires; without it the match breaks. */
                        fogData->switchTimer =
                            *(volatile s32 *)&fogData->switchTimer - updateRate;
                    } else {
                        fogData->fog.r = fogData->intendedFog.r << 16;
                        fogData->fog.g = fogData->intendedFog.g << 16;
                        fogData->fog.b = fogData->intendedFog.b << 16;
                        fogData->fog.near = fogData->intendedFog.near << 16;
                        fogData->fog.far = fogData->intendedFog.far << 16;
                        fogData->switchTimer = 0;
                    }
                }
            }
            fogData++;
        } while (fogIndex != fogCount);
    }
}
/*
 * PROVENANCE: JFG's corresponding track.c function supplies tier-D position
 * and structural context. The body is reconstructed from Mickey's display-
 * list writes and its call to trackGetFog; JFG's placeholder is not imported.
 */
void func_800147A4(s32 playerID) {
    s16 near;
    s16 far;
    s16 targetNear;
    u8 red;
    u8 green;
    u8 blue;
    s8 state;

    trackGetFog(playerID, &near, &far, &targetNear, &red, &green, &blue,
                &state);
    gDPSetFogColor(D_800C9520++, red, green, blue, 0xFF);
    gSPFogPosition(D_800C9520++, near, far);
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800148E0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80014BAC.s")
/*
 * PROVENANCE: JFG's corresponding track.c position identifies the transform
 * role. This body and its local layout are reconstructed from Mickey's own
 * function bytes and call signatures.
 */
void func_80014DE4(void) {
    MtxF matrix;
    TrackLocalTransform transform;
    f32 x;
    f32 y;
    f32 z;

    x = 0.0f;
    y = 0.0f;
    z = -65536.0f;
    transform.zRotation = D_800C9530->z;
    transform.yRotation = D_800C9530->y;
    transform.xRotation = D_800C9530->x;
    transform.x = 0.0f;
    transform.y = 0.0f;
    transform.z = 0.0f;
    func_8002AB78(&transform, matrix);
    mtxf_transform_point(matrix, x, y, z, &x, &y, &z);
    D_800C9B40.x = (s32)x;
    D_800C9B40.y = (s32)y;
    D_800C9B40.z = (s32)z;
}
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
