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
 * Flags: -O2 -mips2 -32 -Wab,-r4300_mul.
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

typedef struct TrackPlanePoints {
    f32 x0;
    f32 y0;
    f32 z0;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 x2;
    f32 y2;
    f32 z2;
} TrackPlanePoints;

typedef struct TrackPlane {
    f32 x;
    f32 y;
    f32 z;
    f32 distance;
} TrackPlane;

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
    void *lightData;
    u8 pad04[0xC - 0x04];
    TrackBatch *batches;
    u8 pad10[0x24 - 0x10];
    s16 batchCount;
    u8 pad26[0x2C - 0x26];
    s16 unk2C;
    s8 lightingMode;
    u8 pad2F[0x30 - 0x2F];
    void *unk30;
    u8 pad34[0x38 - 0x34];
    void *unk38;
    u8 pad3C[0x40 - 0x3C];
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

typedef struct TrackBspNode {
    s16 left;
    s16 right;
    u8 axis;
    u8 segmentIndex;
    s16 splitValue;
} TrackBspNode;

typedef struct TrackData {
    TrackTextureEntry *textures;
    TrackSegment *segments;
    TrackBoundingBox *segmentBounds;
    u8 pad0C[0x10 - 0x0C];
    s32 *visibility;
    void *bspTree;
    s16 textureCount;
    s16 segmentCount;
} TrackData;

typedef struct TrackLevelData {
    u8 pad00[0x22];
    s8 unk22;
    u8 pad23[0x52 - 0x23];
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

typedef struct TrackFogChangerData {
    u8 pad00[0x0B];
    u8 red;
    u8 green;
    u8 blue;
    s16 near;
    s16 far;
    s16 duration;
} TrackFogChangerData;

typedef struct TrackFogChanger {
    u8 pad00[0x0C];
    f32 x;
    u8 pad10[4];
    f32 z;
    u8 pad18[0x3C - 0x18];
    TrackFogChangerData *data;
    u8 pad40[0x84 - 0x40];
    f32 radiusSquared;
} TrackFogChanger;

typedef struct TrackFogPlayerState {
    s8 fogIndex;
} TrackFogPlayerState;

typedef struct TrackFogPlayer {
    u8 pad00[0x0C];
    f32 x;
    u8 pad10[4];
    f32 z;
    u8 pad18[0x64 - 0x18];
    TrackFogPlayerState *state;
} TrackFogPlayer;

typedef struct TrackFallbackPlayer {
    u8 pad00[0x0C];
    f32 x;
    u8 pad10[4];
    f32 z;
    u8 pad18[0x54 - 0x18];
} TrackFallbackPlayer;

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
    u8 pad3C[0x3E - 0x3C];
    s16 segmentIndex;
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
extern s32 D_80078F84;
extern f32 D_800C99B0;
extern f32 D_800C99B4;
extern f32 D_800C99B8;
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
extern s32 D_80079304;
extern TrackLightAllocation *D_80079308;
extern s32 D_800792F8;
extern s32 D_80079350;
extern s32 D_80079354;
extern f32 D_80081690;
extern u8 D_800C95B4;
extern s16 D_800D6C4C;
extern s16 D_800D6C54;
extern s8 D_80079274;
extern s32 D_80079278;
extern s32 D_8007930C;
extern void *D_80079310;
extern void *D_800C9548;
extern void *D_800C95A8;
extern void *D_800C9D20;
extern void *D_800C9D2C;
extern void *D_800C9D30;
extern void *D_800C9D34;
extern void *D_8007926C;
extern s32 D_800C953C;
extern TrackPlanePoints D_8007927C[3];
extern TrackPlane D_800C9578[3];

void func_8002AB78(TrackLocalTransform *transform, MtxF matrix);
void mtxf_transform_point(MtxF matrix, f32 x, f32 y, f32 z,
                          f32 *outX, f32 *outY, f32 *outZ);
ControlSpawned *func_8000590C(ControlSpawnPacket *packet, s32 mode);
TrackFogPlayer **func_80005750(s32 *count);
void func_800367E8(TrackTextureHeader *texture, u32 *flags, s32 *frame,
                   s32 updateRate);
s32 runlinkIsModuleLoaded(s32 module);
s32 TrapDanglingJump();
void func_8000A62C(f32 x, f32 y, f32 z);
void func_8000E5EC(s32 arg0, s32 arg1);
void func_8000E920(s32 arg0, s32 arg1);
void func_80014DE4(void);
void camStandardOrtho(Gfx **displayList, Mtx **matrix);
void func_80034920(Gfx **displayList);
void func_800349A4(Gfx **displayList, void *texture, s32 mode, s32 flags);
void func_800221E8(Gfx **displayList, Mtx **matrix);
s32 camGetMode(void);
s32 camGetNo(void);
void func_80021FB0(s32 mode, s32 camera, s32 *left, s32 *bottom,
                   u32 *right, u32 *top);
void viGetCurrentSize(s32 *width, s32 *height);
void *func_800348D4(TrackTextureHeader *texture, s32 frame);
TrackCamera *camGetPtr(void);
TrackLight *trackLightAsm(TrackData *track, TrackLight *light, void *state);
s32 mainGetNumberOfCameras(void);
f32 func_8002A8BC(s32 angle);
s32 func_80013324(f32 coefficient, f32 numerator,
                  f32 *minimum, f32 *maximum);
f32 func_8002A8C0(s32 angle);
void func_8000F82C(s32 start, s32 count, s32 end);
s32 func_80010178(u32 segmentIndex);
void func_8000D768(TrackLight *light, s32 red, s32 green, s32 blue,
                   s32 intensity);
void func_8000D820(void);
void func_8000439C(void);
void func_80006EA0(void *handle);
void func_80006FA0(void);
void func_8001F364(void);
void func_800347A0(void *texture);
void mmFree(void *data);
void shadowFreeBuffers(void);
void animseqFreeLevelData(void);
f32 (*camGetInvProjMtx(void))[4];
f32 sqrtf(f32 value);
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
    camera = camGetPtr();
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
    camStandardOrtho(&D_800C9520, &D_800C9524);
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
        camera = camGetPtr();
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
#ifdef NON_MATCHING
/* PROVENANCE: the camera/update structure is adapted from Jet Force Gemini's
 * public src/track.c TU position (`func_800135E0`); Mickey's fields, globals,
 * call graph, and instruction boundary remain authoritative. */
void func_8000D018(s32 arg0, s32 arg1) {
    TrackData *track;
    s16 segmentIndex;

    D_800C9530 = camGetPtr();
    func_80014DE4();
    func_8000A62C((f32) D_800C9B40.x / 65536.0f,
                  (f32) D_800C9B40.y / 65536.0f,
                  (f32) D_800C9B40.z / 65536.0f);
    segmentIndex = D_800C9530->segmentIndex;
    if ((segmentIndex >= 0) &&
        ((track = D_800792E8), segmentIndex < track->segmentCount)) {
        D_800C953C = track->segments[segmentIndex].unk2C;
    } else {
        D_800C953C = -1;
    }
    D_800C99B0 = D_800C9530->x;
    D_800C99B4 = D_800C9530->y;
    D_800C99B8 = D_800C9530->z;
    if (D_80078F84 > 0) {
        D_8007926C = TrapDanglingJump(D_800C9530->x, D_800C9530->y,
                                      D_800C9530->z);
    } else {
        D_8007926C = 0;
    }
    if (D_800792EC->unk22 != 0) {
        func_8000E5EC(arg0, arg1);
        return;
    }
    func_8000E920(arg0, arg1);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D018.s")
#endif
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
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c` and
 * assembly-only `trackUpdateLighting`. Mickey's module path, segment layout,
 * globals, and bytes are authoritative; the public name is not adopted.
 */
void func_8000D978(s32 copySegmentData, s32 updateRate) {
    s32 segmentCount;
    s8 mode;
    TrackSegment *segment;
    TrackLightAllocation *allocation;
    TrackLight *light;

    if ((D_800792E8 != NULL) && (mainGetNumberOfCameras() < 2) &&
        ((copySegmentData == 0) || (D_80079308 == NULL)) &&
        ((copySegmentData != 0) || (D_80079308 != NULL))) {
        allocation = D_80079308;
        if (allocation != NULL) {
            D_80079304 ^= 1;
            segmentCount = D_800792E8->segmentCount;
            segment = D_800792E8->segments;
            while (segmentCount--) {
                mode = segment->lightingMode;
                segment->lightData =
                    ((void **) allocation)[D_80079304];
                segment->lightingMode =
                    ((mode << 1) & 2) | ((mode >> 1) & 1);
                segment++;
                allocation++;
            }
        }
        if (runlinkIsModuleLoaded(16) != 0) {
            TrapDanglingJump(&D_800C95B4, D_800792E8, updateRate);
        } else if ((D_800D6C54 != 0xFF) || (D_800D6C4C != 0)) {
            TrapDanglingJump(&D_800C95B4, D_800792E8, updateRate);
        } else {
            func_8000D820();
        }
        segmentCount = D_800792F8;
        light = D_80079300;
        while (segmentCount--) {
            if (light->radius != 0.0f) {
                trackLightAsm(D_800792E8, light, &D_800C95B4);
            }
            light++;
        }
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000DB34.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000DDE4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000DFBC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000E5EC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000E920.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000F198.s")
/*
 * PROVENANCE: Jet Force Gemini's public assembly-only `trackGetBlockList` in
 * `src/track.c` supplies tier-D TU-position and role context. The body and
 * resident layouts below are reconstructed from Mickey-only evidence; the
 * public name is not adopted.
 */
void func_8000F57C(s32 *resultCount, u8 *resultSegments) {
    s32 distanceX;
    s32 distanceY;
    s32 distanceZ;
    s32 resultIndex;
    s32 lastIndex;
    s32 cameraX;
    s32 cameraY;
    s32 cameraZ;
    s32 index;
    s32 tempDistance;
    s32 segmentIndex;
    s32 distances[100];
    TrackBoundingBox *bounds;

    cameraX = D_800C9530->x;
    resultIndex = 0;
    cameraY = D_800C9530->y;
    segmentIndex = 0;
    bounds = D_800792E8->segmentBounds;
    cameraZ = D_800C9530->z;

    if (D_800792E8->segmentCount > 0) {
        do {
            if (func_80010178(segmentIndex) != 0) {
                if (cameraX < bounds->x1) {
                    distanceX = bounds->x1 - cameraX;
                } else if (bounds->x2 < cameraX) {
                    distanceX = cameraX - bounds->x2;
                } else {
                    distanceX = 0;
                }

                if (cameraY < bounds->y1) {
                    distanceY = bounds->y1 - cameraY;
                } else if (bounds->y2 < cameraY) {
                    distanceY = cameraY - bounds->y2;
                } else {
                    distanceY = 0;
                }

                if (cameraZ < bounds->z1) {
                    distanceZ = bounds->z1 - cameraZ;
                } else if (bounds->z2 < cameraZ) {
                    distanceZ = cameraZ - bounds->z2;
                } else {
                    distanceZ = 0;
                }

                distances[resultIndex] =
                    (distanceX * distanceX) + (distanceY * distanceY) +
                    (distanceZ * distanceZ);
                resultSegments[resultIndex] = segmentIndex;
                resultIndex++;
                if (resultIndex >= 100) {
                    segmentIndex = D_800792E8->segmentCount;
                }
            }
            segmentIndex++;
            bounds++;
        } while (segmentIndex < D_800792E8->segmentCount);
    }

    lastIndex = resultIndex - 1;
    while (lastIndex > 0) {
        index = 0;
        while (index < lastIndex) {
            if (distances[index + 1] < distances[index]) {
                tempDistance = *(resultSegments + index);
                *(resultSegments + index) = *(resultSegments + index + 1);
                *(resultSegments + index + 1) = tempDistance;
                tempDistance = distances[index];
                distances[index] = distances[index + 1];
                distances[index + 1] = tempDistance;
            }
            index++;
        }
        lastIndex--;
    }
    *resultCount = resultIndex;
}
/*
 * PROVENANCE: adapted from Diddy Kong Racing's public `src/tracks.c`,
 * `traverse_segments_bsp_tree`; JFG's assembly-only `func_800150A4` confirms
 * the same TU role. Mickey's global result state and integer camera values are
 * authoritative, and the donor names are not imported.
 */
void func_8000F82C(s32 nodeIndex, s32 segmentIndex, s32 segmentIndex2) {
    TrackBspNode *node;
    s32 cameraValue;

    node = (TrackBspNode *)
        ((nodeIndex * sizeof(TrackBspNode)) + (u8 *) D_800C9574);
    if (node->axis == 0) {
        cameraValue = D_800C954C;
    } else if (node->axis == 1) {
        cameraValue = D_800C9554;
    } else {
        cameraValue = D_800C955C;
    }

    if (cameraValue < node->splitValue) {
        if (node->left != -1) {
            func_8000F82C(node->left, segmentIndex,
                          node->segmentIndex - 1);
        } else if (func_80010178(segmentIndex) != 0) {
            ((u8 *) D_800C956C)[D_800C9564++] = segmentIndex;
        }

        if (node->right != -1) {
            func_8000F82C(node->right, node->segmentIndex,
                          segmentIndex2);
        } else if (func_80010178(segmentIndex2) != 0) {
            ((u8 *) D_800C956C)[D_800C9564++] = segmentIndex2;
        }
    } else {
        if (node->right != -1) {
            func_8000F82C(node->right, node->segmentIndex,
                          segmentIndex2);
        } else if (func_80010178(segmentIndex2) != 0) {
            ((u8 *) D_800C956C)[D_800C9564++] = segmentIndex2;
        }

        if (node->left != -1) {
            func_8000F82C(node->left, segmentIndex,
                          node->segmentIndex - 1);
        } else if (func_80010178(segmentIndex) != 0) {
            ((u8 *) D_800C956C)[D_800C9564++] = segmentIndex;
        }
    }
}
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
#ifdef NON_MATCHING
/*
 * PROVENANCE: Diddy Kong Racing's public `src/tracks.c`,
 * `get_level_segment_index_from_position`, supplies the segment scan and
 * nearest-height selection structure. Mickey's bounds are inclusive and its
 * TrackData layout, function boundary, and bytes remain authoritative.
 */
s32 func_8000FAE0(f32 x, f32 y, f32 z) {
    s16 segmentCount;
    s16 yLower;
    s16 yUpper;
    s32 xInt;
    s32 zInt;
    s32 yInt;
    s32 minVal;
    s32 i;
    s32 heightDiff;
    s32 result;
    TrackBoundingBox *bounds;

    result = -1;
    if (D_800792E8 != NULL) {
        segmentCount = D_800792E8->segmentCount;
        minVal = 0x7FFF;
        bounds = D_800792E8->segmentBounds;
        i = 0;
        if (segmentCount > 0) {
            xInt = x;
loop_3:
            if (bounds->x2 < xInt) {
                goto block_14;
            }
            if (xInt < bounds->x1) {
                goto block_14;
            }
            zInt = z;
            if (bounds->z2 < zInt) {
                goto block_14;
            }
            if (zInt < bounds->z1) {
                goto block_14;
            }
            yInt = y;
            yLower = bounds->y1;
            yUpper = bounds->y2;
            if ((yInt >= yLower) && (yUpper >= yInt)) {
                result = i;
                goto done;
            }
            heightDiff = yInt - yUpper;
            if (yInt < yLower) {
                heightDiff = yLower - yInt;
            }
            if (heightDiff < minVal) {
                minVal = heightDiff;
                result = i;
            }
block_14:
            i++;
            bounds++;
            if (i < segmentCount) {
                goto loop_3;
            }
        }
    }
done:
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FAE0.s")
#endif
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
/*
 * PROVENANCE: JFG's public `src/track.c` supplies a same-position,
 * assembly-only placeholder with the same three-plane skeleton. Mickey's
 * matrix, inputs, arithmetic, and output layout are authoritative.
 */
void func_8000FF2C(void) {
    f32 x0;
    f32 y0;
    f32 z0;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 x2;
    f32 y2;
    f32 z2;
    f32 pad0;
    f32 distance;
    TrackPlanePoints *points;
    TrackPlane *plane;
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 inverseLength;
    f32 (*matrix)[4];
    s32 index;

    points = D_8007927C;
    plane = D_800C9578;
    matrix = camGetInvProjMtx();
    index = 0;
    do {
        mtxf_transform_point(matrix, points->x0, points->y0, points->z0,
                             &x0, &y0, &z0);
        mtxf_transform_point(matrix, points->x1, points->y1, points->z1,
                             &x1, &y1, &z1);
        mtxf_transform_point(matrix, points->x2, points->y2, points->z2,
                             &x2, &y2, &z2);

        normalX = ((z1 - z2) * y0) + (y1 * (z2 - z0)) +
                  (y2 * (z0 - z1));
        normalY = ((x1 - x2) * z0) + (z1 * (x2 - x0)) +
                  (z2 * (x0 - x1));
        normalZ = ((y1 - y2) * x0) + (x1 * (y2 - y0)) +
                  (x2 * (y0 - y1));
        inverseLength = 1.0f /
            sqrtf((normalX * normalX) + (normalY * normalY) +
                  (normalZ * normalZ));
        if (inverseLength > 0.0f) {
            normalX *= inverseLength;
            normalY *= inverseLength;
            normalZ *= inverseLength;
        }

        distance = -((x0 * normalX) + (y0 * normalY) +
                     (z0 * normalZ));
        index++;
        plane->x = normalX;
        plane->y = normalY;
        plane->z = normalZ;
        points++;
        plane++;
        plane[-1].distance = distance;
    } while (index != 3);
}
s32 func_80010178(u32 segmentIndex) {
    f32 pad0;
    f32 pad1;
    f32 pad2;
    f32 pad3;
    f32 pad4;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 pad5;
    f32 y2;
    f32 z2;
    f32 x2;
    f32 pad6;
    f32 planeX;
    f32 planeY;
    f32 planeZ;
    TrackPlane *plane;
    TrackBoundingBox *bounds;
    s32 planeCount;

    if (D_8007926C != NULL) {
        if (TrapDanglingJump(D_8007926C, segmentIndex) == 0) {
            return FALSE;
        }
    } else {
        if ((segmentIndex >= (u32) D_800792E8->segmentCount) ||
            (D_800C953C == -1) ||
            (D_800792E8->visibility[D_800C953C + segmentIndex] == 0)) {
            return FALSE;
        }
    }

    bounds = &D_800792E8->segmentBounds[segmentIndex];
    plane = D_800C9578;
    planeCount = 2;
    x2 = bounds->x2;
    y2 = bounds->y2;
    z2 = bounds->z2;
    x1 = bounds->x1;
    y1 = bounds->y1;
    z1 = bounds->z1;
    do {
        planeX = plane->x;
        planeY = plane->y;
        planeZ = plane->z;
        if ((-plane->distance <
             (((x2 * planeX) + (y2 * planeY)) + (z2 * planeZ))) ||
            (-plane->distance <
             (((x1 * planeX) + (y2 * planeY)) + (z2 * planeZ))) ||
            (-plane->distance <
             (((x2 * planeX) + (y1 * planeY)) + (z2 * planeZ))) ||
            (-plane->distance <
             (((x1 * planeX) + (y1 * planeY)) + (z2 * planeZ))) ||
            (-plane->distance <
             (((x2 * planeX) + (y2 * planeY)) + (z1 * planeZ))) ||
            (-plane->distance <
             (((x1 * planeX) + (y2 * planeY)) + (z1 * planeZ))) ||
            (-plane->distance <
             (((x2 * planeX) + (y1 * planeY)) + (z1 * planeZ))) ||
            (-plane->distance <
             (((x1 * planeX) + (y1 * planeY)) + (z1 * planeZ)))) {
            goto next_plane;
        }
        return FALSE;
next_plane:
        plane++;
    } while (planeCount--);
    return TRUE;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800103D4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80010654.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80010900.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80010B4C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800115E4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80011980.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80011CDC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80012234.s")
#ifdef NON_MATCHING
/*
 * PROVENANCE: Jet Force Gemini's public `src/track.c` retains
 * `trackSphereIntersect` as assembly; its signature and collision role
 * supply structural context only. Mickey's vector layout, arithmetic, and
 * output pointers are reconstructed from the resident call sites and bytes.
 */
/* Workbench plateau: register-permutation, 57 instructions, first +0x50.
 * The exact frame, schedule, and sqrtf relocation remain; IDO swaps the f14/f18
 * projection webs. Declaration, lifetime, volatile, and ABI probes did not move it. */
s32 func_80012574(TrackVec3f *origin, TrackVec3f *direction,
                  TrackVec3f *center, f32 radius, f32 *minimum,
                  f32 *maximum) {
    f32 sp38;
    s32 sp1C;
    f32 temp_f0;
    f32 temp_f0_2;
    f32 temp_f12;
    f32 temp_f14;
    f32 temp_f16;
    f32 temp_f18;
    f32 temp_f2;
    f32 temp_f2_2;
    s32 var_v1;

    temp_f0 = origin->f[0] - center->f[0];
    temp_f2 = origin->f[1] - center->f[1];
    var_v1 = FALSE;
    temp_f12 = origin->f[2] - center->f[2];
    temp_f14 = ((temp_f0 * direction->f[0]) +
                (temp_f2 * direction->f[1])) +
               (temp_f12 * direction->f[2]);
    temp_f18 = temp_f14 * temp_f14;
    temp_f16 = (((temp_f0 * temp_f0) + (temp_f2 * temp_f2)) +
                (temp_f12 * temp_f12)) -
               (radius * radius);
    if (temp_f16 <= temp_f18) {
        var_v1 = TRUE;
    }
    if (var_v1 != FALSE) {
        sp1C = var_v1;
        sp38 = temp_f14;
        temp_f0_2 = sqrtf(temp_f18 - temp_f16);
        temp_f2_2 = -temp_f14;
        *minimum = temp_f2_2 - temp_f0_2;
        *maximum = temp_f2_2 + temp_f0_2;
    }
    return var_v1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80012574.s")
#endif
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80012658.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8001291C.s")
/*
 * PROVENANCE: Jet Force Gemini's public assembly-only `trackClip3D` in
 * `src/track.c` supplies the six-plane clipping structure and paired helper
 * context. Mickey's shorter function boundary, fields, globals, and body are
 * reconstructed from Mickey-only evidence.
 */
s32 func_800131AC(TrackVec3f *origin, TrackVec3f *direction,
                  TrackVec3f *minimum, TrackVec3f *maximum,
                  f32 *nearClip, f32 *farClip) {
    f32 near;
    f32 far;
    s32 result;

    D_80079350 = 0;
    result = FALSE;
    near = -32000.0f;
    far = 32000.0f;
    if ((func_80013324(direction->f[0],
                       minimum->f[0] - origin->f[0], &near, &far) != 0) &&
        (func_80013324(-direction->f[0],
                       origin->f[0] - maximum->f[0], &near, &far) != 0) &&
        (func_80013324(direction->f[1],
                       minimum->f[1] - origin->f[1], &near, &far) != 0) &&
        (func_80013324(-direction->f[1],
                       origin->f[1] - maximum->f[1], &near, &far) != 0) &&
        (func_80013324(direction->f[2],
                       minimum->f[2] - origin->f[2], &near, &far) != 0) &&
        (func_80013324(-direction->f[2],
                       origin->f[2] - maximum->f[2], &near, &far) != 0)) {
        result = D_80079354;
        *nearClip = near;
        *farClip = far;
    }
    return result;
}
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
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function
 * `trackFreeAll`, whose assembly-only body supplies the teardown order and
 * loop structure. Mickey's pointers, calls, and object layouts are
 * authoritative; the donor name is not adopted.
 */
void func_80013EC0(void) {
    TrackSegment *segment;
    TrackData *track;
    TrackData **trackSlot;
    s32 index;
    s32 offset;

    trackSlot = &D_800792E8;
    if (D_80079278 > 0) {
        TrapDanglingJump();
        D_80079278 = 0;
    }
    func_8000D570();
    if (D_80079310 != NULL) {
        mmFree(D_80079310);
        D_80079310 = NULL;
        D_8007930C = 0;
    }
    func_8001F364();
    if (D_800792F0 != NULL) {
        func_800347A0(D_800792F0);
        D_800792F0 = NULL;
    }

    track = *trackSlot;
    index = 0;
    if (track->segmentCount > 0) {
        offset = 0;
        do {
            segment = (TrackSegment *) ((u8 *) track->segments + offset);
            if (segment->unk30 != NULL) {
                mmFree(segment->unk30);
                track = *trackSlot;
                segment = (TrackSegment *)
                    ((u8 *) track->segments + offset);
            }
            if (segment->unk38 != NULL) {
                TrapDanglingJump(segment->unk38);
                track = *trackSlot;
            }
            index++;
            offset += sizeof(TrackSegment);
        } while (index < track->segmentCount);
        index = 0;
    }

    if (track->textureCount > 0) {
        offset = 0;
        do {
            func_800347A0(((TrackTextureEntry *)
                ((u8 *) track->textures + offset))->texture);
            track = *trackSlot;
            index++;
            offset += sizeof(TrackTextureEntry);
        } while (index < track->textureCount);
    }

    mmFree(D_800C95A8);
    mmFree(D_800C9D2C);
    mmFree(D_800C9D30);
    mmFree(D_800C9D34);
    if (TrapDanglingJump(osRomBase) != 0) {
        mmFree(D_800C9D20);
    }
    shadowFreeBuffers();
    if (D_800C9550 != NULL) {
        func_80006EA0(D_800C9550);
        func_80006FA0();
    }
    animseqFreeLevelData();
    func_8000439C();
    D_800792E8 = NULL;
    if (D_800C9548 != NULL) {
        mmFree(D_800C9548);
        D_800C9548 = NULL;
    }
    D_80079274 = 0;
}
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
/*
 * PROVENANCE: adapted from Diddy Kong Racing's public `src/tracks.c`,
 * `obj_loop_fogchanger`; JFG's assembly-only `trackChangeFog` independently
 * supplies the TU position. Mickey's object fields, direct player-list call,
 * fallback stride, radius offset, and fog layout are authoritative.
 */
void func_800148E0(TrackFogChanger *changer) {
    s32 nearTemp;
    s32 fogNear;
    s32 views;
    s32 playerIndex;
    s32 index;
    s32 pad;
    s32 fogFar;
    s32 i;
    s32 fogR;
    s32 fogG;
    s32 fogB;
    f32 x;
    f32 z;
    s32 switchTimer;
    TrackFogChangerData *fogChanger;
    TrackFogPlayer **racers;
    TrackFogPlayerState *racer;
    s32 pad2;
    TrackFog *fogData;
    TrackFallbackPlayer *camera;

    racers = NULL;
    fogChanger = changer->data;
    camera = NULL;
    racers = func_80005750(&views);

    i = 0;
    if (views > 0) {
        do {
            index = -1;
            if (racers != NULL) {
                racer = racers[i]->state;
                playerIndex = racer->fogIndex;
                if ((playerIndex >= 0) && (playerIndex < 4) &&
                    (changer != D_800C99C0[playerIndex].fogChanger)) {
                    index = playerIndex;
                    x = racers[i]->x;
                    z = racers[i]->z;
                }
            } else if ((i < 4) &&
                       (changer != D_800C99C0[i].fogChanger)) {
                index = i;
                x = camera[i].x;
                z = camera[i].z;
            }

            if (index != -1) {
                x -= changer->x;
                z -= changer->z;
                if (1) {
                }
                if (((x * x) + (z * z)) <
                    changer->radiusSquared) {
                    fogNear = fogChanger->near;
                    fogFar = fogChanger->far;
                    fogR = fogChanger->red;
                    fogG = fogChanger->green;
                    fogB = fogChanger->blue;
                    switchTimer = fogChanger->duration;
                    if (fogFar < fogNear) {
                        nearTemp = fogNear;
                        fogNear = fogFar;
                        fogFar = nearTemp;
                    }
                    if (fogFar > 1023) {
                        fogFar = 1023;
                    }
                    if (fogNear >= fogFar - 5) {
                        fogNear = fogFar - 5;
                    }

                    fogData = &D_800C99C0[index];
                    fogData->intendedFog.r = fogR;
                    fogData->intendedFog.g = fogG;
                    fogData->intendedFog.b = fogB;
                    fogData->intendedFog.near = fogNear;
                    fogData->intendedFog.far = fogFar;
                    fogData->addFog.r =
                        ((fogR << 16) - fogData->fog.r) / switchTimer;
                    fogData->addFog.g =
                        ((fogG << 16) - fogData->fog.g) / switchTimer;
                    fogData->addFog.b =
                        ((fogB << 16) - fogData->fog.b) / switchTimer;
                    fogData->addFog.near =
                        ((fogNear << 16) - fogData->fog.near) / switchTimer;
                    fogData->addFog.far =
                        ((fogFar << 16) - fogData->fog.far) / switchTimer;
                    fogData->switchTimer = switchTimer;
                    fogData->fogChanger = changer;
                }
            }
            i++;
        } while (i != views);
    }
}
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
