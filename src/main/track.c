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

typedef struct TrackLightColourEntry {
    s8 red;
    s8 green;
    s8 blue;
} TrackLightColourEntry;

typedef struct TrackLight {
    f32 x;
    f32 y;
    f32 z;
    f32 radius;
    f32 secondaryRadius;
    f32 radiusSquared;
    f32 secondaryRadiusSquared;
    f32 falloff;
    TrackLightColourEntry colours[32];
} TrackLight;

typedef struct TrackLightAllocation {
    u32 pad00;
    void *data;
} TrackLightAllocation;

typedef struct TrackTextureHeader {
    u8 pad00[6];
    u16 width;
    u16 height;
    u8 pad0A[6];
    u16 numOfTextures;
    u16 frameAdvanceDelay;
    Gfx *displayList;
    u8 pad18[3];
    u8 unk1B;
} TrackTextureHeader;

typedef struct TrackTextureEntry {
    TrackTextureHeader *texture;
    u32 pad04;
} TrackTextureEntry;

typedef struct TrackTextureLoadLocals {
    s32 pad20;
    void *activeTextureAddress;
    void *textureAddress;
    s32 useOriginalTexture;
    s32 pad30[4];
    s32 activeMaskS;
    s32 pad44;
    s32 maskT;
    s32 maskS;
} TrackTextureLoadLocals;

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

/*
 * PROVENANCE: field order comes from Diddy Kong Racing's public
 * `include/structs.h`, type `LevelModelSegmentBoundingBox`. Mickey's
 * 12-byte accessor stride independently confirms the layout size.
 */
typedef struct TrackBoundingBox {
    s16 x1;
    s16 y1;
    s16 z1;
    s16 x2;
    s16 y2;
    s16 z2;
} TrackBoundingBox;

typedef union TrackSegmentIndex {
    s32 value;
    TrackBoundingBox *bounds;
} TrackSegmentIndex;

typedef struct TrackData {
    TrackTextureEntry *textures;
    TrackSegment *segments;
    TrackBoundingBox *segmentBounds;
    u8 pad0C[0x14 - 0x0C];
    void *bspTree;
    u8 pad18[0x1A - 0x18];
    s16 segmentCount;
} TrackData;

typedef struct TrackLevelData {
    u8 pad00[0x52];
    s8 skyMode;
    u8 skyRotationSpeed;
    u8 pad54[0xB2 - 0x54];
    u8 skyScaleS;
    u8 skyScaleT;
    u8 padB4[4];
    TrackTextureHeader *skyTexture;
    s16 skyOffsetS;
    s16 skyOffsetT;
    u8 padC0[0xD2 - 0xC0];
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

typedef struct TrackTriangle {
    u8 flags;
    u8 vertex0;
    u8 vertex1;
    u8 vertex2;
    s16 u0;
    s16 v0;
    s16 u1;
    s16 v1;
    s16 u2;
    s16 v2;
} TrackTriangle;

typedef struct TrackCamera {
    s16 rotationX;
    s16 rotationY;
    s16 rotationZ;
    u8 pad06[0xC - 6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x30 - 0x18];
    f32 offsetX;
    f32 offsetY;
    f32 offsetZ;
} TrackCamera;

typedef struct TrackSkyMaterial {
    u8 pad00[0xA2];
    u8 textureIndex;
} TrackSkyMaterial;

typedef struct TrackSkyObject {
    s16 rotationY;
    u8 pad02[0xC - 2];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x40 - 0x18];
    TrackSkyMaterial *material;
} TrackSkyObject;

typedef struct TrackVec3f {
    f32 f[3];
} TrackVec3f;

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

extern TrackCamera *D_800C9530;
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
extern TrackTriangle *D_800C952C;
extern u8 D_79330[];
extern TrackTextureHeader *D_800792F0;
extern s32 D_800792F4;
extern Gfx D_80079358[];
extern Gfx D_80079380[];
extern Gfx D_800793A8[];
extern Gfx D_800793D8[];
extern u8 D_80079318[];
extern s32 D_800C9560;
extern s32 D_800C954C;
extern s32 D_800C9554;
extern s32 D_800C955C;
extern s32 D_800C9564;
extern s32 D_800C956C;
extern void *D_800C9574;
extern TrackLight *D_80079300;
extern TrackLightAllocation *D_80079308;
extern s32 D_800792F8;
extern s32 D_80079350;
extern s32 D_80079354;
extern f32 D_80081690;

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
void *func_800348D4(TrackTextureHeader *texture, s32 frame);
TrackCamera *func_8002462C(void);
f32 func_8002A8BC(s32 angle);
f32 func_8002A8C0(s32 angle);
void func_8000F82C(s32 start, s32 count, s32 end);
void func_8000D768(TrackLight *light, s32 red, s32 green, s32 blue,
                   s32 intensity);
void func_80007E40(TrackSkyObject *object, s32 updateRate,
                   TrackLevelData **levelData);
void func_80009E78(Gfx **displayList, Mtx **matrix, TrackVertex **vertices,
                   TrackSkyObject *object);

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
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function
 * `func_80012BAC_137AC`, with the load-bearing local padding documented by
 * Diddy Kong Racing's public `src/tracks.c` version. Mickey proves the revised
 * level-data offsets, command bindings, and vertex/triangle layouts; the
 * donor's placeholder name is not imported.
 */
void func_8000C5F4(void) {
    TrackTriangle *triangles;
    TrackVertex *vertices;
    s32 maskT;
    s32 maskS;
    f32 scaledXSin;
    f32 scaledXCos;
    f32 var_f16;
    s16 textureS[9];
    s16 textureT[9];
    f32 xCos;
    f32 xSin;
    f32 pad_sp108;
    TrackCamera *camera;
    f32 pad_sp100;
    f32 xPositions[9];
    f32 zPositions[9];
    TrackVec3f pos;
    s32 i;
    s32 var_v0;
    s32 var_v1;
    s32 var_a1;
    s32 var_a2;
    u8 *var_v0_3;
    f32 var_f14;
    s16 vertY;
    s16 vTempCoord;
    s16 uTempCoord;
    TrackTextureHeader *texture;
    /* These donor-shaped locals determine IDO's stack homes and FP colours. */
    s32 pad[4];

    vertices = D_800C9528;
    triangles = D_800C952C;
    camera = func_8002462C();
    texture = D_800792EC->skyTexture;
    D_800C9570 = -1;

    maskS = (texture->width << 5) - 1;
    maskT = (texture->height << 5) - 1;
    xSin = func_8002A8C0(-camera->rotationX);
    xCos = func_8002A8BC(-camera->rotationX);

    scaledXSin = xSin * 1280.0f;
    scaledXCos = xCos * 1280.0f;
    pad_sp100 = 2.0f * scaledXSin;
    xPositions[0] = -scaledXCos - (xSin * 1280.0f);
    zPositions[0] = -scaledXCos + (xSin * 1280.0f);
    xPositions[1] = scaledXCos - (xSin * 1280.0f);
    zPositions[1] = -scaledXCos - (xSin * 1280.0f);
    xPositions[2] = scaledXCos + scaledXSin;
    zPositions[2] = scaledXCos - (xSin * 1280.0f);
    xPositions[3] = -scaledXCos + (xSin * 1280.0f);
    zPositions[3] = scaledXCos + (xSin * 1280.0f);
    xPositions[4] = 0.0f;
    zPositions[4] = 0.0f;

    xPositions[5] = -(xCos * 1280.0f) - (2.0f * scaledXSin);
    zPositions[5] = scaledXSin + -(2.0f * (xCos * 1280.0f));
    xPositions[6] = (xCos * 1280.0f) - (2.0f * scaledXSin);
    zPositions[6] = -(2.0f * (xCos * 1280.0f)) - scaledXSin;
    xPositions[7] = (xCos * 1280.0f) + (2.0f * scaledXSin);
    zPositions[7] = (2.0f * (xCos * 1280.0f)) - scaledXSin;
    xPositions[8] = -(xCos * 1280.0f) + (2.0f * scaledXSin);
    zPositions[8] = (2.0f * (xCos * 1280.0f)) + scaledXSin;

    scaledXCos = 1280.0f;
    var_f14 = scaledXCos * 0.25f;
    var_a1 = texture->width * 16 * D_800792EC->skyScaleS;
    var_a2 = texture->height * 16 * D_800792EC->skyScaleT;
    var_v0 = ((s32)(camera->x * ((scaledXCos * 0.25f) / var_a1)) +
              (D_800792EC->skyOffsetS >> 4)) & maskS;
    var_v1 = ((s32)(camera->z * ((scaledXCos * 0.25f) / var_a2)) +
              (D_800792EC->skyOffsetT >> 4)) & maskT;

    var_f14 = var_a1 * xCos;
    pos.f[2] = var_a1 * xCos;
    pos.f[0] = var_a1 * xCos;
    var_f16 = var_a2 * xSin;
    xCos = var_f16;
    pad_sp108 = var_f16;

    var_a2 = texture->height * 16 * D_800792EC->skyScaleT;

    textureS[0] = (s16)(-var_f14 - pad_sp108) + var_v0;
    textureT[0] = (s16)(var_f16 - var_f14) + var_v1;
    textureS[1] = (s16)(var_f14 - pad_sp108) + var_v0;
    textureT[1] = (s16)(-var_f14 - var_f16) + var_v1;
    textureS[2] = (s16)(var_f14 + var_f16) + var_v0;
    textureT[2] = (s16)(var_f14 - var_f16) + var_v1;
    textureS[3] = (s16)(var_f16 - var_f14) + var_v0;
    textureT[3] = (s16)(var_f14 + var_f16) + var_v1;
    textureS[4] = var_v0;
    textureT[4] = var_v1;
    textureS[5] = (s16)(-var_f14 - (2.0f * xCos)) + var_v0;
    textureT[5] = (s16)(var_f16 - (2.0f * var_f14)) + var_v1;
    textureS[6] = (s16)(var_f14 - (2.0f * xCos)) + var_v0;
    textureT[6] = (s16)(-(2.0f * var_f14) - var_f16) + var_v1;
    textureS[7] = (s16)(pos.f[2] + (2.0f * xCos)) + var_v0;
    textureT[7] = (s16)((2.0f * pos.f[0]) - var_f16) + var_v1;
    textureS[8] = (s16)((2.0f * xCos) - pos.f[2]) + var_v0;
    textureT[8] = (s16)((2.0f * pos.f[0]) + var_f16) + var_v1;

    func_800349A4(&D_800C9520, texture, 0x10, D_800C9560 << 8);
    gDPSetPrimColor(D_800C9520++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    gDPSetEnvColor(D_800C9520++, 0xFF, 0xFF, 0xFF, 0xFF);
    TRACK_SP_VERTEX(D_800C9520++, (u32)D_800C9528 + 0x80000000, 9, 0);
    TRACK_SP_POLYGON(D_800C9520++, (u32)D_800C952C + 0x80000000, 8, 1);
    gDPPipeSync(D_800C9520++);

    vertY = camera->y + 192.0f;
    for (i = 0; i < 9; i++) {
        vertices->x = xPositions[i] + camera->x;
        vertices->y = vertY;
        vertices->z = zPositions[i] + camera->z;
        vertices->r = 0xFF;
        vertices->g = 0xFF;
        vertices->b = 0xFF;
        vertices->a = (i <= 4) ? 0xFF : 0;
        vertices++;
    }

    var_v0_3 = D_80079318;
    for (i = 0; i < 8; i++) {
        triangles->flags = 0x40;
        triangles->vertex0 = *var_v0_3;
        triangles->u0 = textureS[*var_v0_3];
        triangles->v0 = textureT[*var_v0_3];
        var_v0_3++;
        triangles->vertex1 = *var_v0_3;
        triangles->u1 = textureS[*var_v0_3];
        triangles->v1 = textureT[*var_v0_3];
        var_v0_3++;
        triangles->vertex2 = *var_v0_3;
        triangles->u2 = textureS[*var_v0_3];
        triangles->v2 = textureT[*var_v0_3];
        var_v0_3++;
        triangles++;
    }

    D_800C9528 = vertices;
    D_800C952C = triangles;
}
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
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c` and
 * assembly-only `func_80013478`. Mickey proves the revised mode test, field
 * offsets, calls, and final draw condition; the donor's placeholder name is
 * not imported.
 */
void func_8000CED0(s32 updateRate) {
    TrackCamera *camera;

    if (D_800C9550 != NULL) {
        camera = func_8002462C();
        if ((D_800792EC->skyMode != 2) && (D_800792EC->skyMode != 5)) {
            ((TrackSkyObject *) D_800C9550)->x = camera->x + camera->offsetX;
            ((TrackSkyObject *) D_800C9550)->y = camera->y + camera->offsetY;
            ((TrackSkyObject *) D_800C9550)->z = camera->z + camera->offsetZ;
            ((TrackSkyObject *) D_800C9550)->rotationY +=
                D_800792EC->skyRotationSpeed * updateRate;
            if (((TrackSkyObject *) D_800C9550)->material->textureIndex !=
                0xFF) {
                func_80007E40(D_800C9550, updateRate, &D_800792EC);
            }
        } else {
            ((TrackSkyObject *) D_800C9550)->x = camera->offsetX;
            ((TrackSkyObject *) D_800C9550)->y = camera->offsetY;
            ((TrackSkyObject *) D_800C9550)->z = camera->offsetZ;
        }
        if (D_800C9558 != 0) {
            func_80009E78(&D_800C9520, &D_800C9524, &D_800C9528,
                          D_800C9550);
        }
    }
}
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
/*
 * PROVENANCE: Jet Force Gemini's public `src/track.c` and built
 * `trackLightFreeMem` establish this function's role and control-flow
 * skeleton. Mickey's own globals, types, and bytes determine this body.
 */
void func_8000D570(void) {
    s32 lightIndex;

    if (D_80079308 != NULL) {
        lightIndex = D_800792E8->segmentCount;
        while (lightIndex--) {
            if (D_80079308[lightIndex].data != NULL) {
                mmFree(D_80079308[lightIndex].data);
            }
        }
        mmFree(D_80079308);
        D_80079308 = NULL;
    }
    if (D_80079300 != 0) {
        mmFree(D_80079300);
        D_80079300 = NULL;
    }
    D_800792FC = 0;
    D_800792F8 = 0;
}
/*
 * PROVENANCE: Jet Force Gemini's public `src/track.c`, assembly-only
 * `trackLightAdd`, supplies the role and 0x80-byte pool stride. Mickey's own
 * stores establish the record fields and body; the public name is not adopted.
 */
TrackLight *func_8000D62C(f32 x, f32 y, f32 z, f32 radius,
                          f32 secondaryRadius, s32 red, s32 green, s32 blue) {
    s32 lightIndex;
    TrackLight *light;

    if (radius <= 0.0f) {
        return NULL;
    }
    light = D_80079300;
    lightIndex = D_800792F8;
    if (lightIndex--) {
        do {
            if (light->radius == 0.0f) {
                light->x = x;
                light->y = y;
                light->z = z;
                light->radius = radius;
                light->secondaryRadius = secondaryRadius;
                light->radiusSquared = radius * radius;
                light->secondaryRadiusSquared =
                    secondaryRadius * secondaryRadius;
                light->falloff =
                    D_80081690 / (radius - secondaryRadius);
                func_8000D768(light, red, green, blue, 0xFF);
                D_800792FC++;
                return light;
            }
            light++;
        } while (lightIndex--);
    }
    return NULL;
}
void func_8000D728(TrackFloatRecord *arg0) {
    if ((arg0 != NULL) && (arg0->unkC != 0.0f)) {
        arg0->unkC = 0.0f;
        D_800792FC--;
    }
}
/*
 * PROVENANCE: Jet Force Gemini's public `src/track.c` supplies the
 * `trackLightColour` role at this established TU position. Its body remains
 * assembly-only; this reconstruction comes from Mickey's own accesses.
 */
void func_8000D768(TrackLight *light, s32 red, s32 green, s32 blue,
                   s32 intensity) {
    TrackLightColourEntry *colour;
    s32 redStep;
    s32 greenStep;
    s32 blueStep;
    s32 colourIndex;

    if (light != NULL) {
        if (intensity < 255) {
            red = (red * intensity) >> 8;
            green = (green * intensity) >> 8;
            blue = (blue * intensity) >> 8;
        }
        colour = light->colours;
        redStep = red;
        greenStep = green;
        blueStep = blue;
        colourIndex = 31;
        do {
            colour->red = red >> 5;
            colour->green = green >> 5;
            colour->blue = blue >> 5;
            red += redStep;
            green += greenStep;
            blue += blueStep;
            colour++;
        } while (colourIndex--);
    }
}
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
void func_8000FA2C(s32 *result, s32 arg1) {
    D_800C954C = D_800C9530->x;
    D_800C9554 = D_800C9530->y;
    D_800C955C = D_800C9530->z;
    D_800C9574 = D_800792E8->bspTree;
    D_800C9564 = 0;
    D_800C956C = arg1;
    func_8000F82C(0, 0, D_800792E8->segmentCount - 1);
    *result = D_800C9564;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FAE0.s")
/*
 * PROVENANCE: Diddy Kong Racing's public `src/tracks.c`,
 * `check_if_inside_segment`, supplies the bounding-box containment structure.
 * Mickey's function takes coordinates directly and uses inclusive bounds.
 */
s32 func_8000FBD8(TrackSegmentIndex segmentIndex, f32 x, f32 y, f32 z) {
    s32 xInt;
    s32 yInt;
    s32 zInt;

    if (D_800792E8 != NULL) {
        xInt = x;
        segmentIndex.bounds =
            &D_800792E8->segmentBounds[segmentIndex.value];
        if (xInt >= segmentIndex.bounds->x1 &&
            segmentIndex.bounds->x2 >= xInt) {
            yInt = y;
            if (yInt >= segmentIndex.bounds->y1 &&
                segmentIndex.bounds->y2 >= yInt) {
                zInt = z;
                if (zInt >= segmentIndex.bounds->z1 &&
                    segmentIndex.bounds->z2 >= zInt) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}
/*
 * PROVENANCE: adapted from Diddy Kong Racing's public `src/tracks.c`,
 * `get_inside_segment_count_xz`. Mickey uses 16-bit output indices and its
 * resident track/bounding-box types and bindings.
 */
s32 func_8000FCA4(s32 x, s32 z, s16 *segments) {
    s32 segmentIndex;
    s32 count = 0;
    TrackBoundingBox *bounds;

    for (segmentIndex = 0; segmentIndex < D_800792E8->segmentCount;
         segmentIndex++) {
        bounds = D_800792E8->segmentBounds + segmentIndex;
        if (x < bounds->x2 + 4 && bounds->x1 - 4 < x &&
            z < bounds->z2 + 4 && bounds->z1 - 4 < z) {
            *segments = segmentIndex;
            count++;
            segments++;
        }
    }
    return count;
}
/*
 * PROVENANCE: adapted from Diddy Kong Racing's public `src/tracks.c`,
 * `get_inside_segment_count_xyz`. Mickey's resident types, bindings, and
 * instruction schedule are authoritative; the donor name is not adopted.
 */
s32 func_8000FD68(s32 *segments, s16 x1, s16 y1, s16 z1, s16 x2, s16 y2,
                  s16 z2) {
    s32 count;
    s32 segmentIndex;
    TrackBoundingBox *bounds;

    x1 -= 4;
    y1 -= 4;
    z1 -= 4;
    x2 += 4;
    y2 += 4;
    z2 += 4;

    segmentIndex = 0;
    count = 0;

    while (segmentIndex < D_800792E8->segmentCount) {
        bounds = &D_800792E8->segmentBounds[segmentIndex];
        if ((bounds->x2 >= x1) && (x2 >= bounds->x1) &&
            (bounds->z2 >= z1) && (z2 >= bounds->z1) &&
            (bounds->y2 >= y1) && (y2 >= bounds->y1)) {
            count++;
            *segments++ = segmentIndex;
        }
        segmentIndex++;
    }
    return count;
}
/*
 * PROVENANCE: adapted from Diddy Kong Racing's public `src/tracks.c`,
 * function `block_get`. Mickey's stricter upper bound, TrackData layout,
 * function boundary, and bytes are authoritative.
 */
TrackSegment *func_8000FEB4(s32 segmentIndex) {
    if ((segmentIndex < 0) ||
        (segmentIndex >= D_800792E8->segmentCount)) {
        return NULL;
    }
    return &D_800792E8->segments[segmentIndex];
}
/*
 * PROVENANCE: adapted from Diddy Kong Racing's public `src/tracks.c`,
 * function `block_boundbox`. Mickey's TrackData layout, function boundary,
 * and bytes are authoritative.
 */
TrackBoundingBox *func_8000FEEC(s32 segmentIndex) {
    if ((segmentIndex < 0) ||
        (D_800792E8->segmentCount < segmentIndex)) {
        return NULL;
    }
    return &D_800792E8->segmentBounds[segmentIndex];
}
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
s32 func_80013324(f32 coefficient, f32 numerator,
                  f32 *minimum, f32 *maximum) {
    f32 ratio;

    D_80079350++;
    if (coefficient > 0.0f) {
        ratio = numerator / coefficient;
        if (*maximum < ratio) {
            return FALSE;
        }
        if (*minimum < ratio) {
            *minimum = ratio;
            D_80079354 = D_80079350;
        }
    } else if (coefficient < 0.0f) {
        ratio = numerator / coefficient;
        if (ratio < *minimum) {
            return FALSE;
        }
        if (ratio < *maximum) {
            *maximum = ratio;
        }
    } else if (numerator > 0.0f) {
        return FALSE;
    }
    return TRUE;
}
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
    transform.zRotation = D_800C9530->rotationZ;
    transform.yRotation = D_800C9530->rotationY;
    transform.xRotation = D_800C9530->rotationX;
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
/*
 * PROVENANCE: Jet Force Gemini's public built `src/track.c.o` and its
 * assembly-only final source entry establish the tier-D TU position and
 * display-list-helper structure. The body is reconstructed from Mickey with
 * the SDK GBI macros; JFG's placeholder name is not imported.
 */
void func_80014ECC(TrackTextureHeader *texture, s32 frame, s32 flags) {
    TrackTextureLoadLocals locals;
    TrackTextureHeader *activeTexture;
    s32 activeFrame;
    s32 activeMaskT;
    s32 intensity;
    s32 shiftS;
    s32 shiftT;

    locals.textureAddress = func_800348D4(texture, frame);
    if (texture->unk1B >= 2) {
        D_800C9520->words.w0 = texture->displayList->words.w0;
        D_800C9520->words.w1 = (u32) locals.textureAddress;
        D_800C9520++;
        gSPDisplayList(D_800C9520++, texture->displayList + 1);
        gSPDisplayList(D_800C9520++, D_800793D8);
        return;
    }

    locals.maskS = func_80014EAC(texture->width);
    locals.maskT = func_80014EAC(texture->height);
    activeTexture = D_800792F0;
    if (activeTexture != NULL) {
        activeFrame = D_800792F4;
        locals.useOriginalTexture = FALSE;
    } else {
        activeTexture = texture;
        activeFrame = (frame >> 8) + 0x100;
        if (activeFrame >= texture->numOfTextures) {
            activeFrame -= texture->numOfTextures;
        }
        activeFrame <<= 8;
        locals.useOriginalTexture = TRUE;
    }

    locals.activeTextureAddress = func_800348D4(activeTexture, activeFrame);
    locals.activeMaskS = func_80014EAC(activeTexture->width);
    activeMaskT = func_80014EAC(activeTexture->height);
    shiftS = (locals.maskS - locals.activeMaskS) & 0xF;
    shiftT = (locals.maskT - activeMaskT) & 0xF;
    gDPLoadMultiBlockS(D_800C9520++, locals.activeTextureAddress, 0x100, 1,
                       G_IM_FMT_IA, G_IM_SIZ_8b, activeTexture->width,
                       activeTexture->height, 0,
                       G_TX_NOMIRROR | G_TX_WRAP,
                       G_TX_NOMIRROR | G_TX_WRAP, locals.activeMaskS,
                       activeMaskT,
                       shiftS, shiftT);

    if (!locals.useOriginalTexture) {
        gDPLoadMultiBlockS(D_800C9520++, locals.textureAddress, 0, 0,
                           G_IM_FMT_RGBA, G_IM_SIZ_16b, texture->width,
                           texture->height, 0,
                           G_TX_NOMIRROR | G_TX_WRAP,
                           G_TX_NOMIRROR | G_TX_WRAP, locals.maskS,
                           locals.maskT,
                           G_TX_NOLOD, G_TX_NOLOD);
        gSPDisplayList(D_800C9520++, D_80079358);
        return;
    }

    gDPLoadTextureBlockS(D_800C9520++, locals.textureAddress, G_IM_FMT_IA,
                         G_IM_SIZ_8b, texture->width, texture->height, 0,
                         G_TX_NOMIRROR | G_TX_WRAP,
                         G_TX_NOMIRROR | G_TX_WRAP, locals.maskS,
                         locals.maskT,
                         G_TX_NOLOD, G_TX_NOLOD);
    if ((flags & 0x70) == 0x10) {
        gSPDisplayList(D_800C9520++, D_800793A8);
    } else {
        gSPDisplayList(D_800C9520++, D_80079380);
    }
    intensity = (frame >> 8) & 0xFF;
    gDPSetEnvColor(D_800C9520++, intensity, intensity, intensity, intensity);
}
