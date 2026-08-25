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
#include "game/charControl.h"
#include "game/math.h"
#include "n_audio/mbi.h"
#include "PR/os_internal.h"

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

typedef struct TrackTextureHeader {
    u8 pad00[0x10];
    u16 numOfTextures;
    u16 frameAdvanceDelay;
} TrackTextureHeader;

typedef struct TrackTextureEntry {
    TrackTextureHeader *texture;
    u32 pad04;
} TrackTextureEntry;

typedef struct TrackBatch {
    u8 textureIndex;
    u8 pad01[9];
    u16 frame;
    u32 flags;
} TrackBatch;

typedef struct TrackSegment {
    u8 pad00[0xC];
    TrackBatch *batches;
    u8 pad10[0x24 - 0x10];
    s16 batchCount;
    u8 pad26[0x40 - 0x26];
} TrackSegment;

typedef struct TrackData {
    TrackTextureEntry *textures;
    TrackSegment *segments;
    u8 pad08[0x1A - 0x08];
    s16 segmentCount;
} TrackData;

typedef struct TrackLevelData {
    u8 pad00[0xD2];
    u8 bottomR;
    u8 bottomG;
    u8 bottomB;
    u8 topR;
    u8 topG;
    u8 topB;
} TrackLevelData;

typedef struct TrackVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} TrackVertex;

#define TRACK_SP_VERTEX(packet, vertex, count, first)                      \
    gDma1p(packet, G_VTX, vertex,                                         \
           (((count) << 3) + ((count) << 1)) + 8,                         \
           ((count) << 3) | (((u32) (vertex)) & 6) | (first))

#define TRACK_SP_POLYGON(packet, address, count, textured)                 \
    {                                                                      \
        Gfx *_g = (Gfx *) (packet);                                        \
        _g->words.w0 = _SHIFTL((((count) - 1) << 4) | (textured), 16, 8) | \
                       _SHIFTL(5, 24, 8) | _SHIFTL((count) << 4, 0, 16);  \
        _g->words.w1 = (u32) (address);                                    \
    }

extern TrackRotation *D_800C9530;
extern TrackCachedPoint D_800C9B40;
extern Gfx *D_800C9520;
extern s32 D_80079314;
extern u32 D_800C9B50[16];
extern s32 D_800792FC;
extern u8 D_8007BEF4;
extern s16 D_800C9570;
extern TrackData *D_800792E8;
extern TrackLevelData *D_800792EC;
extern Mtx *D_800C9524;
extern TrackVertex *D_800C9528;
extern u8 D_79330[];

void func_8002AB78(TrackLocalTransform *transform, MtxF matrix);
void mtxf_transform_point(MtxF matrix, f32 x, f32 y, f32 z,
                          f32 *outX, f32 *outY, f32 *outZ);
ControlSpawned *func_8000590C(ControlSpawnPacket *packet, s32 mode);
void func_800367E8(TrackTextureHeader *texture, u32 *flags, s32 *frame,
                   s32 updateRate);
s32 runlinkIsModuleLoaded(s32 module);
void TrapDanglingJump(s32 updateRate);
void func_80022A50(Gfx **displayList, Mtx **matrix);
void func_80034920(Gfx **displayList);
void func_800349A4(Gfx **displayList, void *texture, s32 mode, s32 flags);
void func_800221E8(Gfx **displayList, Mtx **matrix);
s32 camGetMode(void);
s32 camGetNo(void);
void func_80021FB0(s32 mode, s32 camera, s32 *left, s32 *bottom,
                   u32 *right, u32 *top);
void viGetCurrentSize(s32 *width, s32 *height);

/*
 * PROVENANCE: Jet Force Gemini's public `src/track.c`, function
 * `trackUpdateFX`, supplies the three-module update structure. Mickey proves
 * its own module indices and unresolved call sites, so the name is not adopted.
 */
void func_8000BD50(s32 updateRate) {
    if (runlinkIsModuleLoaded(13) != 0) {
        TrapDanglingJump(updateRate);
    }
    if (runlinkIsModuleLoaded(12) != 0) {
        TrapDanglingJump(updateRate);
    }
    if (runlinkIsModuleLoaded(34) != 0) {
        TrapDanglingJump(updateRate);
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000BDB4.s")
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function
 * `func_800129AC_135AC`. Mickey proves the revised segment and texture layouts
 * and flag bit; the donor's placeholder name is not imported.
 */
void func_8000C400(s32 updateRate) {
    s32 segmentNumber;
    TrackTextureHeader *texture;
    s32 batchNumber;
    TrackBatch *batch;
    TrackSegment *segments;
    s32 frame;

    segments = D_800792E8->segments;
    for (segmentNumber = 0; segmentNumber < D_800792E8->segmentCount; segmentNumber++) {
        batch = segments[segmentNumber].batches;
        for (batchNumber = 0; batchNumber < segments[segmentNumber].batchCount; batchNumber++) {
            if (batch[batchNumber].flags & 0x100000) {
                if (batch[batchNumber].textureIndex != 0xFF) {
                    texture = D_800792E8->textures[batch[batchNumber].textureIndex].texture;
                    if ((texture->numOfTextures != 0x100) && (texture->frameAdvanceDelay != 0)) {
                        frame = batch[batchNumber].frame;
                        func_800367E8(texture, &batch[batchNumber].flags, &frame, updateRate);
                        batch[batchNumber].frame = frame;
                    }
                }
            }
        }
    }
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function
 * `initSky`. Mickey adds the player-count guard and proves its own object-field
 * offsets; the public name is not adopted from tier-D TU position alone.
 */
void func_8000C540(s32 arg0) {
    ControlSpawnPacket packet;

    if ((arg0 == -1) || (D_8007BEF4 >= 3)) {
        D_800C9550 = NULL;
        D_800C9570 = arg0;
    } else {
        packet.x = 0;
        packet.y = 0;
        packet.z = 0;
        packet.mode = 10;
        packet.kind = arg0;
        D_800C9550 = func_8000590C(&packet, 2);
        D_800C9570 = arg0;
        if (D_800C9550 != NULL) {
            ((ControlSpawned *) D_800C9550)->unk3C = 0;
            ((ControlSpawned *) D_800C9550)->unk46 = -1;
        }
    }
}
/* PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function `trackSkySet`. */
void trackSkySet(s32 skyDome) {
    D_800C9558 = skyDome;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000C5F4.s")
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function
 * `func_80013454_14054`, including its display-list command structure. Mickey
 * proves the ten-byte vertex layout and all resident function bindings; the
 * donor's placeholder name is not imported.
 */
void func_8000CC78(void) {
    s32 width;
    s32 height;
    s32 left;
    s32 bottom;
    u32 right;
    u32 top;
    u8 topR;
    u8 topG;
    u8 topB;
    u8 bottomR;
    u8 bottomG;
    u8 bottomB;
    TrackVertex *vertices;

    vertices = D_800C9528;
    D_800C9570 = -1;
    func_80022A50(&D_800C9520, &D_800C9524);
    func_80034920(&D_800C9520);
    func_800349A4(&D_800C9520, NULL, 8, 0);

    TRACK_SP_VERTEX(D_800C9520++, (u32) vertices + 0x80000000, 4, 0);
    TRACK_SP_POLYGON(D_800C9520++, D_79330, 2, 0);

    func_800221E8(&D_800C9520, &D_800C9524);
    topR = D_800792EC->topR;
    topG = D_800792EC->topG;
    topB = D_800792EC->topB;
    bottomR = D_800792EC->bottomR;
    bottomG = D_800792EC->bottomG;
    bottomB = D_800792EC->bottomB;
    viGetCurrentSize(&width, &height);
    func_80021FB0(camGetMode(), camGetNo(), &left, &bottom, &right, &top);
    width = (u32) width >> 1;
    height = (u32) height >> 1;

    vertices->x = left - (u32) width;
    vertices->y = (u32) height - top;
    vertices->z = 0x10;
    vertices->r = topR;
    vertices->g = topG;
    vertices->b = topB;
    vertices->a = 0xFF;
    vertices++;

    vertices->x = right - (u32) width;
    vertices->y = (u32) height - top;
    vertices->z = 0x10;
    vertices->r = topR;
    vertices->g = topG;
    vertices->b = topB;
    vertices->a = 0xFF;
    vertices++;

    vertices->x = left - (u32) width;
    vertices->y = (u32) height - bottom;
    vertices->z = 0x10;
    vertices->r = bottomR;
    vertices->g = bottomG;
    vertices->b = bottomB;
    vertices->a = 0xFF;
    vertices++;

    vertices->x = right - (u32) width;
    vertices->y = (u32) height - bottom;
    vertices->z = 0x10;
    vertices->r = bottomR;
    vertices->g = bottomG;
    vertices->b = bottomB;
    vertices->a = 0xFF;
    vertices++;

    D_800C9528 = vertices;
}
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
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function
 * `trackFadeFog`. Mickey's argument width and direct fog-data path are
 * authoritative where the revisions differ; JFG's name is not adopted.
 */
void func_80014BAC(s32 fogIndex, s32 red, s32 green, s32 blue, s32 near,
                   s32 far, f32 timer) {
    s32 temp;
    s32 switchTimer;
    TrackFog *fogData;

    fogData = &D_800C99C0[fogIndex];

    if (osTvType == 0) {
        switchTimer = timer * 50.0f;
    } else {
        switchTimer = timer * 60.0f;
    }

    if (far < near) {
        temp = near;
        near = far;
        far = temp;
    }

    if (far > 1023) {
        far = 1023;
    }
    if (near >= far - 5) {
        near = far - 5;
    }

    fogData->intendedFog.r = red;
    fogData->intendedFog.g = green;
    fogData->intendedFog.b = blue;
    fogData->intendedFog.near = near;
    fogData->intendedFog.far = far;

    if (switchTimer > 0) {
        fogData->switchTimer = switchTimer;
        fogData->addFog.r = ((red << 16) - fogData->fog.r) / switchTimer;
        fogData->addFog.g = ((green << 16) - fogData->fog.g) / switchTimer;
        fogData->addFog.b = ((blue << 16) - fogData->fog.b) / switchTimer;
        fogData->addFog.near = ((near << 16) - fogData->fog.near) / switchTimer;
        fogData->addFog.far = ((far << 16) - fogData->fog.far) / switchTimer;
    } else {
        fogData->switchTimer = 0;
        fogData->fog.r = red << 16;
        fogData->fog.g = green << 16;
        fogData->fog.b = blue << 16;
        fogData->fog.near = near << 16;
        fogData->fog.far = far << 16;
    }
    fogData->fogChanger = NULL;
}
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
