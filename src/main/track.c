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

typedef struct TrackKeyRecord {
    s16 key;
    s16 sortValue;
    u8 pad04[4];
} TrackKeyRecord;

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
    void *source;
    void *data;
} TrackLightAllocation;

typedef struct TrackTextureHeader {
    u8 pad00[4];
    u16 flags;
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
    u8 unk1;
    u8 pad02[4];
    s16 u0;
    s16 v0;
    u16 frame;
    u32 flags;
} TrackBatch;

typedef struct TrackSegment {
    void *lightData;
    void *vertexData;
    u8 pad08[0xC - 0x08];
    TrackBatch *batches;
    u32 *visibilityMasks;
    u8 pad14[0x18 - 0x14];
    u16 *surfaceIndices;
    TrackPlane *surfaces;
    s16 lightBatchCount;
    u8 pad22[0x24 - 0x22];
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

typedef struct TrackLightSource {
    u8 *source;
    u16 *dirtyMasks;
} TrackLightSource;

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

typedef struct TrackIntersection {
    f32 height;
    s32 flags;
} TrackIntersection;

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
extern s32 D_8007C854;
extern s32 D_8007C858;
extern s32 D_800C9544;
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
extern f32 D_80081770;
extern f32 D_80081774;
extern f32 D_80081790;
extern f32 D_80081778;
extern f32 D_8008177C;
extern f32 D_80081780;
extern f32 D_80081784;
extern f32 D_80081788;
extern f32 D_8008178C;
extern s8 D_80079260;
extern s8 D_80079264;
extern s8 D_80079268;
extern s32 D_8007A124;
extern s32 D_800C9544;
extern s32 D_800C95B0[];
extern s32 D_800C95B4[];
extern s16 D_800D6C4C;
extern s16 D_800D6C54;
extern s8 D_80079274;
extern s32 D_80079278;
extern s32 D_8007930C;
extern void *D_80079310;
extern void *D_800C9548;
extern void *D_800C95A8;
extern void *D_800C9D20;
extern s32 *D_800C9D2C;
extern s32 D_800C9D3C;
extern s16 *D_800C9D30;
extern s16 *D_800C9D34;
extern s32 D_800C9D24;
extern s32 D_800C9D28;
extern void *D_8007926C;
extern s32 D_800C953C;
extern TrackPlanePoints D_8007927C[3];
extern TrackPlane D_800C9578[3];
extern u8 D_800C9B90[];
extern void *D_800C9CD0[];
extern s32 D_800C9D24;

void func_8002AB78(TrackLocalTransform *transform, MtxF matrix);
void mtxf_transform_point(MtxF matrix, f32 x, f32 y, f32 z,
                          f32 *outX, f32 *outY, f32 *outZ);
ControlSpawned *func_8000590C(ControlSpawnPacket *packet, s32 mode);
TrackFogPlayer **func_80005750(s32 *count);
void func_800367E8(TrackTextureHeader *texture, u32 *flags, s32 *frame,
                   s32 updateRate);
void func_80014ECC(TrackTextureHeader *texture, s32 frame, s32 flags);
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
s32 func_800103D4(void *object);
u8 *func_80028F54(void);
f32 camDistance(f32 x, f32 y, f32 z);
u8 *levelGetLevel(void);
void partDraw(Gfx **displayList, s32 arg1, s32 mode);
void func_8000DFBC(s32 segment, s32 arg1, s32 arg2, s32 arg3);
s32 func_8000DDE4(s32 key, s32 recordCount, TrackKeyRecord *records, TrackKeyRecord **matches);
void func_8000F57C(s32 *resultCount, u8 *resultSegments);
void func_8000FA2C(s32 *result, s32 arg1);
void shadowGetBuffers(s32 mode, s32 *a, s32 *b, s32 *c);
void func_800343F0();
void texEnableModes(s32 mode);
s32 getXZCompareMask(TrackBoundingBox *bounds, s32 x0, s32 z0, s32 x1,
                     s32 z1);
void func_800133FC(TrackVertex *v0, TrackVertex *v1, TrackVertex *v2,
                   f32 *a, f32 *b, f32 *c, f32 *d);
s32 mathXZInTri(s32 x, s32 z, TrackVertex *v0, TrackVertex *v1,
                TrackVertex *v2);
void func_8000D768(TrackLight *light, s32 red, s32 green, s32 blue,
                   s32 intensity);
void *func_8002B280(s32 size, s32 tag);
void func_8000D570(void);
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
/*
 * PROVENANCE: Mickey's m2c control-flow draft and the resident display-list,
 * camera, level, and weather declarations reconstruct this update routine;
 * no external function body is adapted.
 */
#ifdef NON_MATCHING
typedef struct TrackFrameTexture {
    u8 pad00[0x10];
    u16 unk10;
    u16 unk12;
} TrackFrameTexture;

typedef struct TrackFrameLevel {
    u8 pad00[0x52];
    s8 unk52;
    u8 pad53[0x30];
    u8 unk83;
    u8 pad84[0x1E];
    s16 unkA2;
    u8 padA4[0x10];
    s8 unkB4;
    s8 unkB5;
    u8 padB6[2];
    TrackTextureHeader *unkB8;
    s16 unkBC;
    s16 unkBE;
    u8 padC0[0x11];
    s8 unkD1;
} TrackFrameLevel;

extern u8 D_80081540[];
extern u8 D_80081550[];
extern s32 D_800C9534;
extern s32 D_800C9538;
extern s32 D_800C9568;
extern u8 D_8007A128;
extern s32 D_8007D6B0;
extern s16 D_800D6C3E;
extern void func_8000C400(s32);
extern void func_8000C5F4(void);
extern void func_8000CC78(void);
extern void func_8000CED0(s32);
extern void func_8000D018(s32, s32);
extern void func_8000FF2C(void);
extern void func_80014614(s32, s32);
extern void func_800147A4(s32);
extern s32 func_800290A0(void);
extern s32 levelInitRegionFlags(void);
extern void camDisableUserView();
extern void camEnableUserView();
extern void camSetNo();
extern void doWeather();
extern void func_800219D0();
extern void func_80022D20();
extern void func_80036CAC();
extern void func_80044BC8();
extern void func_800534EC();
extern void levelUpdateColourCycling();
extern void rainSetFog();
extern void shadowChangeBuffer();
extern void shadowGenerate();
extern void weather_clip_planes();

/* Workbench verdict: structure-mismatch, 342 differing words, first mismatch +0x0. */
/* Candidate is 393/403 instructions with the target -0x38 frame; it is not shape-exact. */
/* Remaining gap: ten missing instructions plus unresolved command/global scheduling. */
void func_8000BDB4(Gfx **arg0, Mtx **arg1, TrackVertex **arg2,
                   TrackTriangle **arg3, s32 arg4) {
    TrackFrameTexture *texture;
    TrackFrameLevel *level;
    Gfx *command;
    s32 temp_a0;
    s32 temp_s2;
    s32 var_a0;
    s32 var_s4;
    s32 var_v0;
    s8 temp_v0;
    u16 temp_v1;

    temp_s2 = mainGetNumberOfCameras();
    camSetNo(0);
    if (TrapDanglingJump() != 0) {
        TrapDanglingJump(arg0);
        return;
    }
    D_800C9520 = *arg0;
    D_800C9524 = *arg1;
    D_800C9528 = *arg2;
    D_800C952C = *arg3;
    func_80044BC8(D_800C9520, (char *) D_80081540, 0x1CC);
    D_800C9558 = 1;
    D_800C9538 = 0;
    if (func_800290A0() != 0) {
        var_s4 = 0;
    } else {
        var_s4 = arg4;
    }
    if (D_800792F0 != NULL) {
        texture = (TrackFrameTexture *) D_800792F0;
        temp_v1 = texture->unk10;
        var_v0 = D_800792F4 + (texture->unk12 * var_s4);
        if (var_v0 >= (s32) temp_v1) {
            do {
                var_v0 -= temp_v1;
            } while (var_v0 >= (s32) temp_v1);
        }
        D_800792F4 = var_v0;
    }
    shadowGenerate(1, arg4);
    levelUpdateColourCycling(var_s4);
    level = (TrackFrameLevel *) D_800792EC;
    temp_a0 = *(s32 *) ((u8 *) level + 0xC0);
    if (temp_a0 != -1) {
        func_80036CAC(temp_a0, var_s4);
    }
    if (level->unk83 == 2) {
        D_80079260 = 0;
    } else {
        D_80079260 = 1;
    }
    temp_v0 = level->unk83;
    if ((temp_v0 == 1) || (temp_v0 == 2) || (level->unkD1 != 0)) {
        D_800C9544 = 1;
    }
    if (level->unk52 == 3) {
        level->unkBC = (level->unkBC + (level->unkB4 * var_s4)) &
                       ((level->unkB8->width << 9) - 1);
        *(s16 *) ((u8 *) level + 0xBE) =
            (*(s16 *) ((u8 *) level + 0xBE) +
             (*(s8 *) ((u8 *) level + 0xB5) * var_s4)) &
            ((level->unkB8->height << 9) - 1);
        func_800367E8(level->unkB8, (u32 *) &D_800C9568,
                      &D_800C9560, var_s4);
    }
    func_80034920(&D_800C9520);
    command = D_800C9520;
    D_800C9520 = command + 1;
    command->words.w1 = 0;
    command->words.w0 = 0xBC000002;
    if (levelInitRegionFlags() != 0) {
        command = D_800C9520;
        D_800C9520 = command + 1;
        command->words.w1 = 0x2000;
        command->words.w0 = 0xB6000000;
        command = D_800C9520;
        D_800C9520 = command + 1;
        command->words.w1 = 0x1000;
        command->words.w0 = 0xB7000000;
    } else {
        command = D_800C9520;
        D_800C9520 = command + 1;
        command->words.w1 = 0x1000;
        command->words.w0 = 0xB6000000;
        command = D_800C9520;
        D_800C9520 = command + 1;
        command->words.w1 = 0x2000;
        command->words.w0 = 0xB7000000;
    }
    command = D_800C9520;
    D_800C9520 = command + 1;
    command->words.w1 = 0x64;
    command->words.w0 = 0xF9000000;
    command = D_800C9520;
    D_800C9520 = command + 1;
    command->words.w1 = -1;
    command->words.w0 = 0xFA000000;
    command = D_800C9520;
    D_800C9520 = command + 1;
    command->words.w1 = -0x100;
    command->words.w0 = 0xFB000000;
    rainSetFog();
    func_80014614(temp_s2, var_s4);
    if (*(s16 *) ((u8 *) D_800792E8 + 0x1E) > 0) {
        func_8000C400(var_s4);
    }
    if (D_80079274 != 0) {
        TrapDanglingJump((void **) (s32) var_s4);
    }
    if ((D_8007A128 != 0) && (temp_s2 == 1)) {
        camEnableUserView(0, 1);
        func_800219D0();
    }
    D_800C9534 = 0;
    var_a0 = 0;
    if (temp_s2 > 0) {
        do {
            func_800147A4(var_a0);
            command = D_800C9520;
            D_800C9520 = command + 1;
            command->words.w1 = 0;
            command->words.w0 = 0xE7000000;
            camSetNo(D_800C9534);
            func_800221E8(&D_800C9520, &D_800C9524);
            func_8000FF2C();
            if (temp_s2 < 3) {
                if (level->unk52 == 3) {
                    func_8000C5F4();
                } else if (D_800C9550 != 0) {
                    func_8000CED0(arg4);
                }
                if (D_80079278 > 0) {
                    if (D_800C9534 == 0) {
                        TrapDanglingJump((void **) (s32) var_s4);
                    }
                    TrapDanglingJump(&D_800C9520);
                }
            } else {
                temp_v0 = level->unk52;
                if ((temp_v0 != 4) && (temp_v0 != 5)) {
                    func_8000CC78();
                }
            }
            func_80044BC8(D_800C9520, (char *) D_80081550, 0x26A);
            command = D_800C9520;
            D_800C9520 = command + 1;
            command->words.w1 = 0;
            command->words.w0 = 0xE7000000;
            func_8000D018(temp_s2, arg4);
            weather_clip_planes(-1, -0x200);
            if ((level->unkA2 > 0) && (temp_s2 < 2)) {
                doWeather(&D_800C9520, &D_800C9524,
                          (void *) &D_800C9528, (void *) &D_800C952C,
                          var_s4);
            }
            var_a0 = D_800C9534 + 1;
            D_800C9534 = var_a0;
        } while (var_a0 < temp_s2);
    }
    if (D_8007D6B0 > 0) {
        TrapDanglingJump(&D_800C9520);
    }
    func_800534EC((s32) &D_800C9520);
    if (D_800D6C3E != 0) {
        TrapDanglingJump(&D_800C9520);
    }
    if (levelInitRegionFlags() != 0) {
        command = D_800C9520;
        D_800C9520 = command + 1;
        command->words.w1 = 0x1000;
        command->words.w0 = 0xB6000000;
        command = D_800C9520;
        D_800C9520 = command + 1;
        command->words.w1 = 0x2000;
        command->words.w0 = 0xB7000000;
    }
    func_80022D20(&D_800C9520);
    camDisableUserView(0, 1);
    command = D_800C9520;
    D_800C9520 = command + 1;
    command->words.w1 = 0;
    command->words.w0 = 0xE7000000;
    command = D_800C9520;
    D_800C9520 = command + 1;
    command->words.w1 = 0;
    command->words.w0 = 0xBC000002;
    shadowChangeBuffer();
    *arg0 = D_800C9520;
    *arg1 = D_800C9524;
    *arg2 = D_800C9528;
    *arg3 = D_800C952C;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000BDB4.s")
#endif
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
/* Dangling overlay call taking the camera world position (three f32 args in
 * f12/f14/a2-raw) and returning a pointer. Typed weak alias so this call site
 * passes single-precision floats without the default-argument double promotion
 * that the unprototyped `s32 TrapDanglingJump()` forces; the build canonicalizes
 * the undefined symbol to the shared TrapDanglingJump target (0x800333A0)
 * without changing section contents. */
#pragma weak trackCamPosTrap = TrapDanglingJump
extern void *trackCamPosTrap(f32, f32, f32);
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
        D_8007926C = trackCamPosTrap(D_800C9530->x, D_800C9530->y,
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
void func_8000D16C(s32 arg0, s32 arg1, s32 arg2) {
    if (D_80079314 < 16) {
        D_800C9B50[D_80079314] =
            (arg0 << 24) | ((arg1 & 0xFFF) << 12) | (arg2 & 0xFFF);
        D_80079314++;
    }
}
/* PROVENANCE: Mickey's target accesses reconstruct the packed-scroll and
 * nested segment/batch/vertex loops; Jet Force Gemini's assembly-only
 * trackUpdateTextureScroll supplies TU-position and role context only. */
/* Workbench: structure-mismatch; 121 words differ, first mismatch +0x04. */
/* Candidate is not shape-exact: target/candidate 128/124 instructions, frame -40/-40. */
/* Structural gap: 98 aligned structural words, 40 register words, 1 constant; 7 relocation sites differ. */
#ifdef NON_MATCHING
void func_8000D1B8(void) {
    s16 temp_s3;
    s16 temp_s3_2;
    s16 temp_s3_3;
    s16 temp_s4;
    s16 temp_s4_2;
    s16 temp_t1;
    s16 temp_t2;
    s32 *var_s0;
    s32 temp_t4;
    s32 temp_t4_2;
    s32 temp_v1_2;
    s32 var_a3;
    s32 var_s1;
    s32 var_t0;
    s32 var_t2;
    s32 var_t5;
    s32 var_v1;
    u16 temp_a0;
    u16 temp_v0;
    TrackTextureHeader *temp_v1;
    u8 *var_t1;
    TrackSegment *var_t3;
    u8 *var_v0;

    if (D_800792E8 != NULL) {
        var_s0 = (s32 *) D_800C9B50;
        if (D_80079314 != 0) {
            var_s1 = D_80079314 - 1;
            if (D_80079314 != 0) {
                do {
                    temp_t4 = *var_s0;
                    var_s0 += 1;
                    temp_t4_2 = (temp_t4 >> 24) & 0xFF;
                    temp_v1 = D_800792E8->textures[temp_t4_2].texture;
                    temp_t1 = D_800792E8->segmentCount;
                    temp_a0 = temp_v1->width;
                    var_t3 = D_800792E8->segments;
                    if (temp_a0 < 0x41 && temp_v1->height < 0x41) {
                        var_a3 = (temp_a0 << 8) - 1;
                        temp_v0 = temp_v1->height;
                        var_t0 = (temp_v0 << 8) - 1;
                    } else {
                        var_a3 = (temp_a0 << 6) - 1;
                        temp_v0 = temp_v1->height;
                        var_t0 = (temp_v0 << 6) - 1;
                    }
                    var_t5 = temp_t1 - 1;
                    if (temp_t1 != 0) {
                        do {
                            temp_t2 = var_t3->batchCount;
                            var_t1 = var_t3->batches;
                            var_t2 = temp_t2 - 1;
                            if (temp_t2 != 0) {
                                do {
                                    if (temp_t4_2 == var_t1[0]) {
                                        temp_s3 = *(s16 *) (var_t1 + 8);
                                        temp_v1_2 =
                                            *(s16 *) (var_t1 + 0x18) - temp_s3;
                                        var_v0 =
                                            *(u8 **) ((u8 *) var_t3 + 4) +
                                            (temp_s3 * 0x10);
                                        var_v1 = temp_v1_2 - 1;
                                        if (temp_v1_2 != 0) {
                                            do {
                                                temp_s3_2 = *(s16 *) (var_v0 + 4);
                                                temp_s4 = *(s16 *) (var_v0 + 6);
                                                temp_s3_3 =
                                                    (temp_s3_2 +
                                                     ((s32) (temp_t4 << 8) >> 20)) &
                                                    var_a3;
                                                temp_s4_2 =
                                                    (temp_s4 +
                                                     ((s32) (temp_t4 << 20) >> 20)) &
                                                    var_t0;
                                                *(s16 *) (var_v0 + 4) = temp_s3_3;
                                                *(s16 *) (var_v0 + 6) = temp_s4_2;
                                                *(s16 *) (var_v0 + 8) =
                                                    (s16) (temp_s3_3 +
                                                           (*(s16 *) (var_v0 + 8) -
                                                            temp_s3_2));
                                                *(s16 *) (var_v0 + 10) =
                                                    (s16) (temp_s4_2 +
                                                           (*(s16 *) (var_v0 + 10) -
                                                            temp_s4));
                                                *(s16 *) (var_v0 + 12) =
                                                    (s16) (temp_s3_3 +
                                                           (*(s16 *) (var_v0 + 12) -
                                                            temp_s3_2));
                                                *(s16 *) (var_v0 + 14) =
                                                    (s16) (temp_s4_2 +
                                                           (*(s16 *) (var_v0 + 14) -
                                                            temp_s4));
                                                var_v0 += 0x10;
                                                var_v1 -= 1;
                                            } while (var_v1 != 0);
                                        }
                                    }
                                    var_t1 += 0x10;
                                    var_t2 -= 1;
                                } while (var_t2 != 0);
                            }
                            var_t3 = (TrackSegment *) ((u8 *) var_t3 + 0x40);
                            var_t5 -= 1;
                        } while (var_t5 != 0);
                    }
                    var_s1 -= 1;
                } while (var_s1 != 0);
            }
        }
    }
    D_80079314 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D1B8.s")
#endif
/*
 * PROVENANCE: Jet Force Gemini's public assembly-only `trackLightAllocate`
 * establishes the pool/segment allocation role. Mickey's +0x20 lighting
 * count and two-pointer allocation record are reconstructed from the target
 * accesses; the donor placeholder name is not adopted.
 */
#ifdef NON_MATCHING
void func_8000D3B8(s32 lightCount, s32 copyData) {
    s32 temp_lo;
    s32 var_s2;
    s32 var_s2_2;
    s32 var_s7;
    s32 var_v1;
    s32 var_s0;
    u8 *var_s1;
    u8 temp_t5;
    void *var_v0;
    u8 *var_s3;
    u8 *var_s4;
    u8 *var_v1_2;

    D_800792FC = 0;
    D_800792F8 = lightCount;
    var_s7 = 1;
    var_v0 = func_8002B280(lightCount << 7, 0x91);
    D_80079300 = var_v0;
    if (var_v0 != NULL) {
        var_v0 = (void *) D_800792F8;
        var_s2 = D_800792F8 - 1;
        if (D_800792F8 != 0) {
            var_v1 = var_s2 << 7;
            do {
                var_v0 = (void *) var_s2;
                *(f32 *) ((u8 *) D_80079300 + var_v1 + 0xC) = 0.0f;
                var_v1 -= 0x80;
                var_s2--;
            } while (var_s2 != 0);
        }
        var_s7 = copyData;
        if (copyData != 0) {
            var_v0 = func_8002B280(D_800792E8->segmentCount * 8, 0x91);
            var_s3 = var_v0;
            if (var_v0 != NULL) {
                var_s4 = (u8 *) D_800792E8->segments;
                D_80079304 = 1;
                D_80079308 = var_v0;
                var_s2_2 = 0;
                var_s7 = 0;
                if (D_800792E8->segmentCount > 0) {
                    do {
                        var_s1 = *(u8 **) var_s4;
                        temp_lo = (*(s16 *) (var_s4 + 0x20)) * 10;
                        *(u8 **) var_s3 = var_s1;
                        var_v0 = func_8002B280(temp_lo, 0x91);
                        *(void **) (var_s3 + 4) = var_v0;
                        var_v1_2 = var_v0;
                        if (var_v0 != NULL) {
                            var_v0 = (void *) temp_lo;
                            var_s0 = temp_lo - 1;
                            if (temp_lo != 0) {
                                do {
                                    temp_t5 = *var_s1;
                                    var_v0 = (void *) var_s0;
                                    var_v1_2++;
                                    var_s1++;
                                    var_v1_2[-1] = temp_t5;
                                    var_s0--;
                                } while (var_s0 != 0);
                            }
                        } else {
                            var_s7 = 1;
                        }
                        var_s2_2++;
                        var_s3 += 8;
                        var_s4 += 0x40;
                    } while (var_s2_2 < D_800792E8->segmentCount);
                }
            }
        }
    }
    if (var_s7 != 0) {
        func_8000D570();
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D3B8.s")
#endif
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
/*
 * PROVENANCE: Jet Force Gemini's public `src/track.c` lighting-update
 * skeleton supplies the alternating packed-colour copy. Mickey's separate
 * lighting count at `TrackSegment +0x20`, source record, and dirty-mask
 * fields are established by the resident accesses above.
 */
#ifdef NON_MATCHING
void func_8000D820(void) {
    s16 segmentCount;
    s16 lightCount;
    s32 *segmentFlags;
    s32 dirty;
    s32 copyMode;
    s32 maskCount;
    u16 *dirtyMasks;
    u16 mask;
    u32 shiftedMask;
    u8 colour;
    TrackLightSource *sourceRecord;
    u8 *sourceBase;
    u8 *lightData;
    u8 *source;
    u8 *destination;
    TrackSegment *segment;

    segmentFlags = D_800C95B4;
    segment = D_800792E8->segments;
    segmentCount = D_800792E8->segmentCount;
    copyMode = (D_80079308 != NULL) ? 1 : -1;
    if (segmentCount != 0) {
        do {
            dirty = *segmentFlags++;
            if ((dirty != 0) && ((segment->lightingMode & copyMode) != 0)) {
                sourceRecord = (TrackLightSource *) segment->unk30;
                if (sourceRecord != NULL) {
                    sourceBase = sourceRecord->source;
                    lightData = (u8 *) segment->lightData;
                    lightCount = segment->lightBatchCount;
                    if (copyMode > 0) {
                        source = sourceBase;
                        destination = lightData;
                        if (lightCount != 0) {
                            do {
                                destination += 10;
                                destination[-4] = source[0];
                                colour = source[1];
                                source += 3;
                                destination[-3] = colour;
                                destination[-2] = source[-1];
                                lightCount--;
                            } while (lightCount != 0);
                        }
                        segment->lightingMode ^= 1;
                    } else {
                        maskCount = (lightCount + 15) >> 4;
                        dirtyMasks = sourceRecord->dirtyMasks;
                        if (maskCount != 0) {
                            do {
                                mask = *dirtyMasks;
                                destination = lightData;
                                source = sourceBase;
                                *dirtyMasks = 0;
                                if (mask != 0) {
                                    do {
                                        shiftedMask = mask >> 1;
                                        if ((mask & 1) != 0) {
                                            destination[6] = source[0];
                                            destination[7] = source[1];
                                            destination[8] = source[2];
                                        }
                                        destination += 10;
                                        source += 3;
                                        mask = (u16) shiftedMask;
                                    } while (shiftedMask != 0);
                                }
                                destination += 0xA0;
                                source += 0x30;
                                dirtyMasks += 2;
                                maskCount--;
                            } while (maskCount != 0);
                        }
                        segment->lightingMode = 0;
                    }
                }
            }
            segment++;
            segmentCount--;
        } while (segmentCount != 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D820.s")
#endif
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
            /* Runtime relocation: overlay 40 +0x690 (overlay40FadeRecords). */
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
/*
 * PROVENANCE: Mickey's m2c control-flow draft and the resident object and
 * bounding-box offsets supply this route-list reconstruction. No external
 * function body is copied here; the public JFG routine is context only.
 */
typedef struct TrackRouteObject {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x16];
    s16 segmentIndex;
    u8 pad30[4];
    f32 radius;
} TrackRouteObject;

typedef struct TrackRouteResult {
    s16 segmentIndex;
    s16 flags;
    TrackRouteObject *object;
} TrackRouteResult;

extern s32 func_8000A244(s32 *resultCount);
extern s32 func_8000A39C(s32 first, s32 last);
extern TrackRouteObject *func_800056F0(s32 index);

#ifdef NON_MATCHING
/* Workbench verdict: structure-mismatch, 267 differing words, first mismatch +0x0. */
/* Candidate: 267/172 instructions with a -0x188 frame versus target -0x190; input-loop unrolling and local/register shape remain. */
/* Shape status: route traversal and bounds filtering are reconstructed, but the candidate is not shape-exact. */
s32 func_8000DB34(s32 count, u8 *indices, TrackRouteResult *results) {
    u8 map[256];
    s32 heapCount;
    s32 mapIndex;
    s32 resultCount;
    s32 lastIndex;
    s32 segmentIndex;
    s32 objectRadius;
    s32 minX;
    s32 minY;
    s32 minZ;
    s32 maxDistance;
    s16 candidateSegment;
    u8 inputIndex;
    u8 candidateIndex;
    volatile u8 *input;
    TrackRouteObject *object;
    TrackBoundingBox *bounds;

    {
        u8 *mapPointer = map;
        u8 *mapEnd = map + 256;

        do {
            *mapPointer++ = 0xFF;
        } while (mapPointer < mapEnd);
    }
    mapIndex = 0;
    if (count > 0) {
        s32 remainder = count & 3;

        if (remainder != 0) {
            input = indices;
            do {
                inputIndex = *input++;
                map[inputIndex] = mapIndex++;
            } while (remainder != mapIndex);
            if (mapIndex == count) {
                goto build_routes;
            }
        }
        input = indices + mapIndex;
        do {
            map[input[0]] = mapIndex;
            map[input[1]] = mapIndex + 1;
            map[input[2]] = mapIndex + 2;
            map[input[3]] = mapIndex + 3;
            input += 4;
            mapIndex += 4;
        } while (mapIndex != count);
    }

build_routes:
    heapCount = func_8000A244(&lastIndex);
    func_8000A39C(heapCount, lastIndex - 1);
    resultCount = 0;
    if (heapCount < lastIndex) {
        lastIndex--;
        do {
            object = func_800056F0(lastIndex);
            if ((object->segmentIndex != -1) &&
                (map[object->segmentIndex] != 0xFF) &&
                (func_800103D4(object) != 0)) {
                objectRadius = (s32) object->radius;
                candidateSegment = object->segmentIndex;
                minX = (s32) object->x - objectRadius;
                minY = (s32) object->y - objectRadius;
                minZ = (s32) object->z - objectRadius;
                maxDistance = objectRadius * 2;
                candidateIndex = map[object->segmentIndex];
                if (candidateIndex != 0) {
                    input = indices + candidateIndex - 1;
                    do {
                        inputIndex = *input--;
                        bounds = &D_800792E8->segmentBounds[inputIndex];
                        if ((minX < bounds->x2) &&
                            (minY < bounds->y2) &&
                            (minZ < bounds->z2) &&
                            (bounds->x1 < (minX + maxDistance)) &&
                            (bounds->y1 < (minY + maxDistance)) &&
                            (bounds->z1 < (minZ + maxDistance))) {
                            candidateSegment = inputIndex;
                        }
                        candidateIndex--;
                    } while (candidateIndex != 0);
                }
                results->segmentIndex = candidateSegment;
                results->flags = 0xFF;
                results->object = object;
                results++;
                resultCount++;
            }
            lastIndex--;
        } while (heapCount < lastIndex);
    }
    return resultCount;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000DB34.s")
#endif
/* Exact 118-word C: reusing the dead recordIndex carrier for the later sort
 * passes closes the 24-site allocator bijection while preserving the 0x28
 * frame and both R_MIPS_26 call identities. The complete flag lattice was
 * nonexact and one codegen-faithful allocator trace selected this lifetime
 * merge; the reference skeleton scan found no credible donor. */
s32 func_8000DDE4(s32 key, s32 recordCount, TrackKeyRecord *records,
                  TrackKeyRecord **matches) {
    s32 recordIndex;
    s32 matchCount;
    s32 compareCount;
    s32 sorted;
    TrackKeyRecord **match;
    TrackKeyRecord *current;
    TrackKeyRecord *next;
    s32 currentValue;
    s32 nextValue;

    matchCount = 0;
    for (recordIndex = 0; recordIndex < recordCount; recordIndex++) {
        if (records[recordIndex].key == key) {
            matches[matchCount++] = &records[recordIndex];
        }
    }
    if ((matchCount > 0) && (runlinkIsModuleLoaded(21) != 0)) {
        TrapDanglingJump(key, matchCount, matches);
    }
    if (matchCount >= 2) {
        recordIndex = matchCount - 1;
        if (matchCount != 0) {
            do {
                current = matches[0];
                match = matches;
                sorted = TRUE;
                compareCount = recordIndex - 1;
                currentValue = current->sortValue;
                if (recordIndex != 0) {
                    do {
                        next = match[1];
                        nextValue = next->sortValue;
                        if (nextValue < currentValue) {
                            match[0] = next;
                            match++;
                            sorted = FALSE;
                        } else {
                            match[0] = current;
                            match++;
                            current = next;
                            currentValue = nextValue;
                        }
                    } while (compareCount--);
                }
                match[0] = current;
                if (sorted) {
                    recordIndex = 0;
                }
            } while (recordIndex--);
        }
    }
    return matchCount;
}
#ifdef NON_MATCHING
/* PROVENANCE: JFG's public track.c supplies the resident track draw-loop
 * organization; Mickey's segment and display-list accesses are authoritative. */
/* Workbench verdict: structure-mismatch, 382 differing words; first mismatch is at +0x0. */
/* Target is 396 instructions/frame -112; candidate is 355 instructions/frame -120. */
/* Remaining gap is structural: batch/display-list and object-dispatch loops differ; not permuter-ready. */
struct TrackShadowObject;
struct TrackShadowInstance;
extern void func_800140CC(struct TrackShadowObject *object,
                          struct TrackShadowInstance *instance);

void func_8000DFBC(u8 segmentArg, s32 arg1, s32 arg2, s32 arg3) {
    /* Parameter types follow the top-level prototype the matched callers use. */
    s32 arg0 = segmentArg;
    TrackSegment *segment;
    TrackBatch *batch;
    TrackBatch *batchEnd;
    Gfx *gfx;
    u8 *texture;
    u8 *object;
    u8 *objectChild;
    u8 *item;
    u8 *vertex;
    u8 *triangle;
    s32 batchIndex;
    s32 batchCount;
    s32 itemIndex;
    s32 alpha;
    s32 mode;
    s32 vertexCount;
    s32 textureS;
    s32 vertexAddress;
    s32 textured;
    s32 objectMode;
    s32 value;
    s16 objectType;

    segment = &D_800792E8->segments[arg0];
    batch = segment->batches;
    batchCount = segment->batchCount;
    batchIndex = 0;
    itemIndex = 0;
    gfx = D_800C9520;
    if (batchCount <= 0 && arg2 <= 0) {
        return;
    }
    do {
        if ((batchIndex < batchCount) &&
            ((itemIndex >= arg2) ||
             (batchIndex < ((u8 **) (u32) arg3)[itemIndex][2]))) {
            batchEnd = batch;
            if ((arg1 & (1 << batchIndex)) &&
                (batchIndex == batch->unk1)) {
                if (D_8007C854 != 0) {
                    gfx->words.w0 = 0xFA000000;
                    value = D_8007C858 & 0xFF;
                    gfx->words.w1 = (value << 24) | (value << 16) |
                                     (value << 8) | 0xFF;
                    gfx++;
                    D_800C9520 = gfx;
                    batchCount = segment->batchCount;
                }
                if ((batchIndex < batchCount) &&
                    (batchIndex == batch->unk1)) {
                    do {
                        mode = batch->flags;
                        if (!(mode & 0x800)) {
                            alpha = 0;
                            texture = NULL;
                            if (batch->textureIndex != 0xFF) {
                                alpha = 1;
                                texture = (u8 *) D_800792E8->textures +
                                          (batch->textureIndex * 8);
                            }
                            textureS = batch->frame << 8;
                            vertex = (u8 *) segment->lightData +
                                     (batch->u0 * 0xA);
                            triangle = (u8 *) segment->vertexData +
                                       (batch->v0 * 0x10);
                            if ((texture != NULL) &&
                                (*(u32 *) (texture + 4) & 0x40) &&
                                ((mode & 0x30) != 0x20)) {
                                gfx->words.w0 = 0xFB000000;
                                value = (textureS >> 8) & 0xFF;
                                gfx->words.w1 = (value << 24) |
                                                 (value << 16) |
                                                 (value << 8) | value;
                                gfx++;
                                D_800C9520 = gfx;
                            } else {
                                gfx->words.w0 = 0xFB000000;
                                gfx->words.w1 = -0x100;
                                gfx++;
                                D_800C9520 = gfx;
                            }
                            if (!(mode & 0x180)) {
                                mode |= D_800C9544;
                            }
                            objectMode = mode & 0x4000;
                            if (objectMode != 0) {
                                func_800343F0(2, batchIndex);
                            }
                            func_800349A4(&D_800C9520, texture,
                                          mode | 2, textureS);
                            if (objectMode != 0) {
                                texEnableModes(2);
                            }
                            gfx = D_800C9520;
                            vertexAddress = (s32) vertex + 0x80000000;
                            vertexCount = batch->frame - batch->u0;
                            gfx->words.w1 = vertexAddress;
                            gfx->words.w0 = (((vertexCount * 0xA) + 8) & 0xFFFF) |
                                             0x04000000 |
                                             ((((vertexCount * 8) |
                                                (vertexAddress & 6)) & 0xFF) << 16);
                            gfx++;
                            D_800C9520 = gfx;
                            gfx->words.w1 = (s32) triangle + 0x80000000;
                            vertexCount = batch[1].v0 - batch->v0;
                            gfx->words.w0 = ((vertexCount * 0x10) & 0xFFFF) |
                                             0x05000000 |
                                             (((((vertexCount - 1) * 0x10) |
                                                alpha) & 0xFF) << 16);
                            gfx++;
                            D_800C9520 = gfx;
                            batch = (TrackBatch *) ((u8 *) batch + 0x10);
                            batchIndex++;
                            batchCount = segment->batchCount;
                        } else {
                            batch = (TrackBatch *) ((u8 *) batch + 0x10);
                            batchIndex++;
                        }
                    } while ((batchIndex < batchCount) &&
                             (batch->unk1 == batchEnd->unk1));
                    batchCount = segment->batchCount;
                }
                if (D_8007C854 != 0) {
                    gfx = D_800C9520;
                    gfx->words.w1 = 0;
                    gfx->words.w0 = 0xE7000000;
                    gfx++;
                    gfx->words.w1 = -1;
                    gfx->words.w0 = 0xFA000000;
                    gfx++;
                    D_800C9520 = gfx;
                }
            } else if ((batchIndex < batchCount) &&
                       (batch->unk1 == batchEnd->unk1)) {
                do {
                    batch = (TrackBatch *) ((u8 *) batch + 0x10);
                    batchIndex++;
                } while ((batchIndex < batchCount) &&
                         (batch->unk1 == batchEnd->unk1));
            }
        } else {
            item = ((u8 **) (u32) arg3)[itemIndex++];
            object = *(u8 **) (item + 4);
            objectChild = *(u8 **) (object + 0x4C);
            if ((objectChild != NULL) && (*(u16 *) (object + 0x8E) == 0)) {
                if (*(u32 *) (objectChild + 0x10) & 8) {
                    u8 *child = *(u8 **) (objectChild + 0x1C);
                    if (child != NULL) {
                        func_800140CC((struct TrackShadowObject *) object,
                                      (struct TrackShadowInstance *) child);
                    }
                }
                func_800140CC((struct TrackShadowObject *) object,
                              (struct TrackShadowInstance *) objectChild);
            }
            func_80009E78(&D_800C9520, &D_800C9524, &D_800C9528,
                          (TrackSkyObject *) object);
            value = *(s32 *) (object + 0x54);
            if (value != 0) {
                func_80049518(value, &D_800C9520);
            }
            if (*(u16 *) (object + 6) & 0x200) {
                objectType = *(s16 *) (object + 0x44);
                switch (objectType) {
                case 1:
                    func_80009414(&D_800C9520, &D_800C9524,
                                  &D_800C9528, object);
                    break;
                case 0x1D:
                case 0x49:
                case 0x3F:
                    TrapDanglingJump(&D_800C9520, &D_800C9524,
                                     &D_800C9528, object);
                    break;
                case 0x39:
                case 0x3A:
                    TrapDanglingJump(&D_800C9520, &D_800C9524, object);
                    break;
                }
            }
        }
        batchCount = segment->batchCount;
    } while (itemIndex < arg2);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000DFBC.s")
#endif
#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c draft and resident track/particle call surfaces
 * reconstruct this draw/update coordinator; no external function body is adapted.
 */
/* Workbench verdict: structure-mismatch, 284 differing words, first mismatch +0x0. */
/* Candidate: 284/205 instructions with a -0x78 frame versus target -0xD8; stack-byte, call-signature, and pointer-register structure remain unresolved. */
/* Shape status: coordinator call order and segment passes are reconstructed, but the candidate is not shape-exact. */
void func_8000E5EC(s32 updateRate, s32 arg1) {
    TrackData *track;
    u8 segmentList;
    u8 *segment;
    s32 resultCount;
    s32 visibleCount;
    s32 segmentIndex;
    s32 lastIndex;
    s32 displayOffset;
    s32 *visibility;
    s16 cameraSegment;
    s16 segmentCount;
    s8 mode;
    s32 *displayList;

    track = D_800792E8;
    visibleCount = 1;
    if (track->segmentCount >= 2) {
        if (levelGetLevel()[0x106] == 0) {
            func_8000FA2C(&visibleCount, &segmentList);
        } else {
            func_8000F57C(&visibleCount, &segmentList);
        }
    } else {
        segmentList = 0;
    }
    if (D_80079260 == 0) {
        visibleCount = 0;
    }
    D_800C95B0[0] = -1;
    segmentIndex = 1;
    if (track->segmentCount > 0) {
        displayList = D_800C95B4;
        do {
            *displayList++ = 0;
            segmentIndex++;
        } while (track->segmentCount >= segmentIndex);
    }
    if ((D_80079260 != 0) || (D_80079264 != 0)) {
        cameraSegment = camGetPtr()->segmentIndex;
        segmentCount = track->segmentCount;
        if ((cameraSegment >= 0) && (cameraSegment < segmentCount) &&
            (D_8007926C == 0)) {
            lastIndex = visibleCount - 1;
            segment = &segmentList + lastIndex;
            if (visibleCount != 0) {
                do {
                    mode = *segment--;
                    visibility = track->visibility;
                    D_800C95B0[mode] =
                        visibility[(cameraSegment * segmentCount) + mode];
                    lastIndex--;
                } while (lastIndex != 0);
            }
        } else {
            lastIndex = visibleCount - 1;
            if (visibleCount != 0) {
                segment = &segmentList + lastIndex;
                do {
                    mode = *segment--;
                    D_800C95B0[mode] = -1;
                    lastIndex--;
                } while (lastIndex != 0);
            }
        }
        if (track->segmentCount < 2) {
            D_800C95B0[1] = -1;
        }
    }
    resultCount = 0;
    displayList = D_800C9548;
    if (D_80079268 != 0) {
        resultCount = func_8000DB34(visibleCount, &segmentList,
                                    (TrackRouteResult *) displayList);
    }
    func_8000D978(0, arg1);
    func_80034920(&D_800C9520);
    if ((D_8007A124 == 0) && (camGetMode() == 0)) {
        partDraw(&D_800C9520, (s32) &D_800C9524, 1);
    }
    func_80034920(&D_800C9520);
    lastIndex = visibleCount - 1;
    if (visibleCount != 0) {
        segment = &segmentList + lastIndex;
        displayOffset = (resultCount * 8) + (s32) displayList;
        do {
            mode = *segment;
            func_8000DFBC(mode, D_800C95B0[mode],
                          func_8000DDE4(mode, resultCount,
                                        (s32) displayList, displayOffset),
                          displayOffset);
            segment--;
            lastIndex--;
        } while (lastIndex != 0);
    }
    if (runlinkIsModuleLoaded(0x22) != 0) {
        TrapDanglingJump(&D_800C9520, &D_800C9528);
    }
    if ((D_8007A124 == 0) && (camGetMode() == 0)) {
        partDraw(&D_800C9520, (s32) &D_800C9524, 0);
    }
    D_800C9544 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000E5EC.s")
#endif
#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c control-flow draft and the resident track
 * declarations reconstruct this display-list pipeline; no external function
 * body is adapted. The raw offsets retain fields absent from the local types.
 */
/* Workbench verdict: structure-mismatch, 536 differing words, first mismatch +0x0. */
/* Candidate is 554/542 instructions with the target -0xF8 frame; it is not shape-exact. */
/* Remaining gap: twelve excess instructions and unresolved command/object relocation scheduling. */
extern u8 D_80081560[];
extern u8 D_80081570[];
extern u8 D_80081580[];
extern u8 D_80081590[];
extern u8 D_800815A0[];
extern u8 D_800815B0[];
extern u8 D_800815C0[];
extern u8 D_800815D0[];
extern u8 D_800815E0[];
extern u8 D_800815F0[];
extern u8 D_80081600[];
extern u8 D_80081610[];
extern void func_8000F198(s32 segment, s32 record, s32 mode);

#define E920_U8(base, offset) (*(u8 *) ((u8 *) (base) + (offset)))
#define E920_S16(base, offset) (*(s16 *) ((u8 *) (base) + (offset)))
#define E920_S32(base, offset) (*(s32 *) ((u8 *) (base) + (offset)))
#define E920_PTR(base, offset) (*(void **) ((u8 *) (base) + (offset)))
#define E920_RECORD(segment) \
    (*(s32 *) ((u8 *) D_800C95B0 + ((segment) * 0x10) + 4))

void func_8000E920(s32 arg0, s32 arg1) {
    s32 segmentCount;
    s32 segmentEnd;
    s32 selectedCount;
    s32 index;
    s32 reverseIndex;
    s16 modeCount;
    s16 segment;
    u8 segmentIds[0x70];
    void **selectedObjects;
    void *object;
    void *surface;
    void *childSurface;
    u8 *level;
    s32 record;

    segmentCount = func_8000A244(&segmentEnd);
    selectedObjects = (void **) D_800C9548;
    level = levelGetLevel();
    if (E920_S16(D_800792E8, 0x1A) >= 2) {
        if (E920_U8(level, 0x106) == 0) {
            func_8000FA2C(&arg0, (s32) &segmentIds[0]);
        } else {
            func_8000F57C(&arg0, &segmentIds[0]);
        }
    } else {
        arg0 = 1;
        segmentIds[0] = 0;
    }
    func_8000A39C(segmentCount, segmentEnd - 1);
    func_80034920(&D_800C9520);
    func_80044BC8(D_800C9520, D_80081560, 0x58D);
    D_800C95B0[0] = -1;
    modeCount = E920_S16(D_800792E8, 0x1A);
    for (index = 1; index < modeCount; index++) {
        D_800C95B4[index * 4] = 0;
    }
    if ((D_80079260 != 0) || (D_80079264 != 0)) {
        for (reverseIndex = arg0 - 1; reverseIndex >= 0; reverseIndex--) {
            segment = segmentIds[reverseIndex];
            E920_RECORD(segment) = -1;
            func_8000F198(segment, -1, 0x4000);
        }
        modeCount = E920_S16(D_800792E8, 0x1A);
    }
    if (modeCount < 2) {
        E920_RECORD(1) = -1;
    }
    func_8000D978(0, arg1);
    func_80044BC8(D_800C9520, D_80081570, 0x5A1);
    if (D_80079260 != 0) {
        for (index = 0; index < arg0; index++) {
            segment = segmentIds[index];
            func_8000F198(segment, E920_RECORD(segment), 0);
        }
    }
    if (D_80079268 == 0) {
        segmentCount = segmentEnd;
    }
    selectedCount = 0;
    for (index = segmentCount; index < segmentEnd; index++) {
        object = func_800056F0(index);
        if ((object != NULL) &&
            (E920_RECORD(E920_S16(object, 0x2E)) != 0) &&
            (func_800103D4(object) != 0)) {
            selectedObjects[selectedCount++] = object;
        }
    }
    if (E920_U8(D_800792EC, 0xF6) != 0) {
        TrapDanglingJump(selectedCount, selectedObjects);
    }
    func_80044BC8(D_800C9520, D_80081580, 0x5D7);
    for (index = 0; index < selectedCount; index++) {
        object = selectedObjects[index];
        if ((E920_U8(object, 0x58) != 0) &&
            ((E920_U8(object, 6) & 0xC) == 0) &&
            (E920_U8(object, 0x39) == 0xFF)) {
            func_80009E78(&D_800C9520, &D_800C9524, &D_800C9528,
                          (TrackSkyObject *) object);
        }
    }
    func_80044BC8(D_800C9520, D_80081590, 0x5E3);
    for (reverseIndex = selectedCount - 1; reverseIndex >= 0;
         reverseIndex--) {
        object = selectedObjects[reverseIndex];
        surface = E920_PTR(object, 0x4C);
        if ((surface != NULL) && (E920_S16(object, 0x8E) == 0)) {
            if ((E920_S32(surface, 0x10) & 8) != 0) {
                childSurface = E920_PTR(surface, 0x1C);
                if (childSurface != NULL) {
                    func_800140CC((struct TrackShadowObject *) object,
                                  (struct TrackShadowInstance *) childSurface);
                }
            }
            func_800140CC((struct TrackShadowObject *) object,
                          (struct TrackShadowInstance *) surface);
        }
    }
    func_80044BC8(D_800C9520, D_800815A0, 0x5F7);
    for (index = 0; index < selectedCount; index++) {
        object = selectedObjects[index];
        if (((E920_U8(object, 6) & 0xC) == 0) &&
            (E920_U8(object, 0x39) == 0xFF) &&
            (E920_U8(object, 0x58) == 0)) {
            func_80009E78(&D_800C9520, &D_800C9524, &D_800C9528,
                          (TrackSkyObject *) object);
        }
    }
    func_80044BC8(D_800C9520, D_800815B0, 0x603);
    for (reverseIndex = selectedCount - 1; reverseIndex >= 0;
         reverseIndex--) {
        object = selectedObjects[reverseIndex];
        if ((E920_U8(object, 6) & 8) != 0) {
            func_80009E78(&D_800C9520, &D_800C9524, &D_800C9528,
                          (TrackSkyObject *) object);
        }
    }
    if (runlinkIsModuleLoaded(0xC) != 0) {
        TrapDanglingJump((s32) &D_800C9520, &D_800C9524, &D_800C9528);
    }
    if (E920_U8(D_800792EC, 0xF6) != 0) {
        func_80044BC8(D_800C9520, D_800815C0, 0x61A);
        TrapDanglingJump((s32) &D_800C9520, &D_800C9524, &D_800C9528);
        if (D_80079260 != 0) {
            for (reverseIndex = arg0 - 1; reverseIndex >= 0;
                 reverseIndex--) {
                segment = segmentIds[reverseIndex];
                func_8000F198(segment, E920_RECORD(segment), 0x8000);
            }
            for (reverseIndex = selectedCount - 1; reverseIndex >= 0;
                 reverseIndex--) {
                object = selectedObjects[reverseIndex];
                surface = E920_PTR(object, 0x4C);
                if ((surface != NULL) && (E920_S16(object, 0x8E) != 0)) {
                    if ((E920_S32(surface, 0x10) & 8) != 0) {
                        childSurface = E920_PTR(surface, 0x1C);
                        if (childSurface != NULL) {
                            func_800140CC(
                                (struct TrackShadowObject *) object,
                                (struct TrackShadowInstance *) childSurface);
                        }
                    }
                    func_800140CC((struct TrackShadowObject *) object,
                                  (struct TrackShadowInstance *) surface);
                }
            }
        }
    }
    func_80044BC8(D_800C9520, D_800815D0, 0x634);
    if (D_80079260 != 0) {
        for (reverseIndex = arg0 - 1; reverseIndex >= 0; reverseIndex--) {
            segment = segmentIds[reverseIndex];
            func_8000F198(segment, E920_RECORD(segment), 4);
        }
    }
    func_80044BC8(D_800C9520, D_800815E0, 0x63B);
    for (reverseIndex = selectedCount - 1; reverseIndex >= 0;
         reverseIndex--) {
        object = selectedObjects[reverseIndex];
        record = E920_S32(object, 0x54);
        if (record != 0) {
            func_80049518(record, &D_800C9520);
        }
    }
    if (runlinkIsModuleLoaded(0xD) != 0) {
        TrapDanglingJump((s32) &D_800C9520, &D_800C9524, &D_800C9528);
    }
    if (runlinkIsModuleLoaded(0x22) != 0) {
        TrapDanglingJump((s32) &D_800C9520, &D_800C9528);
    }
    func_80044BC8(D_800C9520, D_800815F0, 0x64E);
    for (reverseIndex = selectedCount - 1; reverseIndex >= 0;
         reverseIndex--) {
        object = selectedObjects[reverseIndex];
        if (((E920_U8(object, 6) & 4) != 0) ||
            ((s32) E920_U8(object, 0x39) < 0xFF)) {
            func_80009E78(&D_800C9520, &D_800C9524, &D_800C9528,
                          (TrackSkyObject *) object);
        }
        if ((E920_U8(object, 6) & 0x200) != 0) {
            switch (E920_S16(object, 0x44)) {
            case 1:
                func_80009414(&D_800C9520, &D_800C9524, &D_800C9528,
                              (TrackSkyObject *) object);
                break;
            case 0x1D:
            case 0x49:
            case 0x3F:
                TrapDanglingJump((s32) &D_800C9520, &D_800C9524,
                                 &D_800C9528, object);
                break;
            case 0x39:
            case 0x3A:
                TrapDanglingJump((s32) &D_800C9520, &D_800C9524, object);
                break;
            }
        }
    }
    func_80044BC8(D_800C9520, D_80081600, 0x678);
    if ((D_8007A124 == 0) && (camGetMode() == 0)) {
        partDraw(&D_800C9520, (s32) &D_800C9524, -1);
    }
    D_800C9544 = 0;
    func_80044BC8(D_800C9520, D_80081610, 0x680);
}
#undef E920_U8
#undef E920_S16
#undef E920_S32
#undef E920_PTR
#undef E920_RECORD
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000E920.s")
#endif
/* PROVENANCE -- JFG's public track.c supplies the surrounding display-list
 * routine and texture vocabulary, while this Mickey body follows its own
 * fields, call sites, and assembly-only command schedule. */
#ifdef NON_MATCHING
/* Workbench verdict: structure-mismatch; 228 differing words, first mismatch +0x0. */
/* Target 249 instructions/frame -112; candidate 247 instructions/frame -88. */
/* Remaining gap is stack layout and command scheduling; 24-byte frame excess remains, so it is not shape-exact. */
void func_8000F198(s32 arg0, s32 arg1, s32 arg2) {
    TrackSegment *segment;
    TrackBatch *batch;
    TrackTextureHeader *texture;
    s32 sp5C;
    s32 sp58;
    u32 vertexAddress;
    u32 positionAddress;
    s32 textureFrame;
    s32 textureFlags;
    s32 hasTexture;
    s32 vertexCount;
    s32 positionCount;
    s32 color;
    s32 batchCount;
    s32 index;

    if (D_8007C854 != 0) {
        color = D_8007C858 & 0xFF;
        gDPSetPrimColor(D_800C9520++, 0, 0, color, color, color, 0xFF);
    }

    segment = &D_800792E8->segments[arg0];
    switch (arg2) {
    case 4:
        sp5C = 0xC904;
        sp58 = 0xC800;
        break;
    case 0x4000:
        func_800343F0(2, arg0);
        sp5C = 0x4800;
        sp58 = 0x800;
        break;
    case 0x8000:
        sp5C = 0xC800;
        sp58 = 0x4800;
        break;
    default:
        sp5C = -1;
        sp58 = 0xC904;
        break;
    }

    batchCount = segment->batchCount;
    batch = segment->batches;
    index = batchCount;
    if (batchCount != 0) {
        do {
            if ((1 << batch->unk1) & arg1) {
                textureFlags = batch->flags;
                if ((textureFlags & sp5C) && !(textureFlags & sp58)) {
                    texture = NULL;
                    hasTexture = 0;
                    if (batch->textureIndex != 0xFF) {
                        hasTexture = 1;
                        texture = D_800792E8->textures[batch->textureIndex].texture;
                    }
                    textureFrame = batch->frame << 8;
                    vertexAddress = (u32) segment->lightData +
                                    (batch->u0 * 0xA);
                    positionAddress = (u32) segment->vertexData +
                                      (batch->v0 * 0x10);
                    if ((texture != NULL) && (texture->flags & 0x40) &&
                        ((textureFlags & 0x30) != 0x20)) {
                        color = (textureFrame >> 8) & 0xFF;
                        gDPSetEnvColor(D_800C9520++, color, color, color,
                                       color);
                    } else {
                        gDPSetEnvColor(D_800C9520++, 0xFF, 0xFF, 0xFF, 0);
                    }
                    if (!(textureFlags & 0x180)) {
                        textureFlags |= D_800C9544;
                    }
                    if ((textureFlags & 0x20000) && (texture != NULL)) {
                        func_80014ECC(texture, textureFrame, textureFlags);
                    } else {
                        func_800349A4(&D_800C9520, texture,
                                      textureFlags | 2, textureFrame);
                    }
                    vertexAddress += 0x80000000;
                    vertexCount = batch[1].u0 - batch->u0;
                    TRACK_SP_VERTEX(D_800C9520++, vertexAddress,
                                    vertexCount, 0);
                    positionCount = batch[1].v0 - batch->v0;
                    positionAddress += 0x80000000;
                    TRACK_SP_POLYGON(D_800C9520++, positionAddress,
                                     positionCount, hasTexture);
                    if (textureFlags & 0x20000) {
                        func_80034920(&D_800C9520);
                    }
                }
            }
            batch++;
            index--;
        } while (index != 0);
    }
    if (arg2 == 0x4000) {
        texEnableModes(2);
    }
    if (D_8007C854 != 0) {
        gDPPipeSync(D_800C9520++);
        gDPSetPrimColor(D_800C9520++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000F198.s")
#endif
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
/* Workbench verdict: structure-mismatch, 43 differing words, first mismatch +0x1c. */
/* Candidate: target/candidate 62/62 instructions with matching -0x10 frames; the residual begins in the x/z predicate. */
/* Shape status: boundary-load and non-likely branch scheduling remain structural; this is not permuter-ready. */
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
/* PROVENANCE: JFG's assembly-only object-alpha routine supplies the role and switch family;
 * Mickey's jump tables, fields, globals, and arithmetic are authoritative here. */
#ifdef NON_MATCHING
s32 func_800103D4(void *object) {
    u8 *gameMode;
    u8 *player;
    void *state;
    void *bounds;
    TrackPlane *plane;
    f32 objectX;
    f32 objectY;
    f32 objectZ;
    f32 radius;
    f32 fadeDistance;
    f32 fadeRange;
    f32 fadeScale;
    s16 kind;
    s16 distanceLimit;
    s32 visible;
    s32 alphaValue;
    u8 alpha;

    visible = 1;
    gameMode = func_80028F54();
    kind = *(s16 *) ((u8 *) object + 0x44);
    if (kind < 30) {
        switch (kind) {
        case 1:
            state = *(void **) ((u8 *) object + 0x64);
            *(u8 *) ((u8 *) object + 0x39) = *(u8 *) ((u8 *) state + 0xF);
            break;
        case 3:
            state = *(void **) ((u8 *) object + 0x64);
            *(u8 *) ((u8 *) object + 0x39) = (s32) *(f32 *) ((u8 *) state + 0x18);
            break;
        case 17:
            break;
        case 18:
            state = *(void **) ((u8 *) object + 0x64);
            *(u8 *) ((u8 *) object + 0x39) = *(u8 *) ((u8 *) state + 2);
            break;
        case 26:
            state = *(void **) ((u8 *) object + 0x64);
            *(u8 *) ((u8 *) object + 0x39) = *(u32 *) ((u8 *) state + 4);
            break;
        case 2:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 27:
        case 28:
        case 29:
        default:
            *(u8 *) ((u8 *) object + 0x39) = 0xFF;
            break;
        }
    } else if ((kind >= 63) && (kind < 89)) {
        switch (kind) {
        case 63:
            state = *(void **) ((u8 *) object + 0x64);
            *(u8 *) ((u8 *) object + 0x39) = *gameMode == 5
                ? *(u8 *) ((u8 *) state + 0x190)
                : 0xFF;
            player = (u8 *) state;
            if ((*gameMode != 5) && ((*(u16 *) (player + 0x1A8) & 1) != 0) &&
                (*(u8 *) (player + 0x170) != 0)) {
                *(u8 *) ((u8 *) object + 0x39) = *(u8 *) ((u8 *) object + 0x39);
            }
            break;
        case 84:
        case 85:
        case 86:
            break;
        case 64:
        case 65:
        case 66:
        case 67:
        case 68:
        case 69:
        case 70:
        case 71:
        case 72:
        case 73:
        case 74:
        case 75:
        case 76:
        case 77:
        case 78:
        case 79:
        case 80:
        case 81:
        case 82:
        case 83:
        case 87:
        case 88:
        default:
            *(u8 *) ((u8 *) object + 0x39) = 0xFF;
            break;
        }
    } else {
        *(u8 *) ((u8 *) object + 0x39) = 0xFF;
    }
    alpha = *(u8 *) ((u8 *) object + 0x39);
    if (alpha == 0) {
        return 0;
    }
    bounds = *(void **) ((u8 *) object + 0x40);
    if (bounds != NULL) {
        distanceLimit = *(s16 *) ((u8 *) bounds + 0x16);
        if (distanceLimit != 0) {
            fadeDistance = camDistance(*(f32 *) ((u8 *) object + 0xC),
                                       *(f32 *) ((u8 *) object + 0x10),
                                       *(f32 *) ((u8 *) object + 0x14));
            fadeRange = (f32) distanceLimit;
            if (fadeRange < fadeDistance) {
                visible = 0;
            } else {
                fadeScale = fadeRange * D_80081770;
                fadeRange -= fadeDistance;
                if (fadeRange < fadeScale) {
                    visible = 0;
                } else {
                    alphaValue = (s32) ((f32) alpha * (fadeRange / fadeScale));
                    alpha = alphaValue;
                    *(u8 *) ((u8 *) object + 0x39) = alpha;
                }
            }
        }
    }
    if (visible != 0) {
        objectX = *(f32 *) ((u8 *) object + 0xC);
        objectY = *(f32 *) ((u8 *) object + 0x10);
        objectZ = *(f32 *) ((u8 *) object + 0x14);
        radius = *(f32 *) ((u8 *) object + 0x34);
        plane = D_800C9578;
        while ((u8 *) plane < (u8 *) D_800C95A8) {
            if ((((objectX * plane->x) + (objectY * plane->y)) +
                 (objectZ * plane->z) + plane->distance + radius) < 0.0f) {
                visible = 0;
            }
            plane++;
            if (visible == 0) {
                break;
            }
        }
    }
    return visible;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800103D4.s")
#endif
#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c draft and the resident collision-node and plane
 * offsets reconstruct this intersection query; no external body is adapted.
 */
typedef struct TrackRayPoint {
    f32 x;
    f32 y;
    f32 z;
} TrackRayPoint;

typedef struct TrackRayNode {
    u8 pad00[0x1C];
    TrackPlane *planes;
} TrackRayNode;

/* Workbench verdict: structure-mismatch, 172 differing words, first mismatch +0x0. */
/* Candidate: 174/171 instructions with a -0x80 frame versus target -0x98; three instruction/FP-home residuals remain. */
/* Shape status: vector math, three plane tests, and encoded-node traversal are preserved, but it is not shape-exact. */
s32 func_80010654(TrackRayPoint *start, TrackRayPoint *end,
                  TrackPlane *result, f32 *maximum) {
    TrackRayNode *node;
    TrackRayNode *entry;
    TrackPlane *plane;
    TrackRayPoint difference;
    f32 planeX;
    f32 planeZ;
    f32 planeDistance;
    f32 startValue;
    f32 endValue;
    f32 intersection;
    f32 pointX;
    f32 pointY;
    f32 pointZ;
    f32 normalValue;
    s32 encoded;
    s32 iteration;
    s32 entryOffset;
    s32 edgeOffset;
    s32 valid;
    s32 resultValue;
    s32 sign;
    u16 edge;

    difference.x = end->x - start->x;
    difference.y = end->y - start->y;
    difference.z = end->z - start->z;
    resultValue = 0;
    iteration = 0;
    entryOffset = 0;
    if (D_800C9D3C > 0) {
        do {
            iteration++;
            encoded = *(s32 *) ((u8 *) D_800C9D2C + entryOffset);
            if (encoded > 0) {
                node = (TrackRayNode *) (encoded | (s32) 0x80000000);
            } else {
                entry = (TrackRayNode *) encoded;
                plane = (TrackPlane *) ((u8 *) node->planes +
                                        (*(u16 *) entry * 0x10));
                if (D_80081774 <= plane->distance) {
                    planeX = plane->x;
                    planeZ = plane->z;
                    planeDistance = plane->distance;
                    endValue = ((plane->x * end->x) +
                                (plane->distance * end->y)) +
                               (end->z * plane->z) + plane->distance;
                    if (endValue < 0.0f) {
                        startValue = ((plane->x * start->x) +
                                      (plane->distance * start->y)) +
                                     (start->z * plane->z) + plane->distance;
                        if (startValue >= 0.0f) {
                            intersection = startValue / (startValue - endValue);
                            if (intersection <= *maximum) {
                                pointX = start->x + (difference.x * intersection);
                                pointY = start->y + (difference.y * intersection);
                                pointZ = start->z + (difference.z * intersection);
                                valid = 1;
                                edgeOffset = 0;
                                do {
                                    edgeOffset += 2;
                                    edge = *(u16 *) ((u8 *) entry + edgeOffset);
                                    sign = edge & 0x8000;
                                    plane = (TrackPlane *) ((u8 *) node->planes +
                                                           ((edge ^ sign) * 0x10));
                                    normalValue = ((plane->x * pointX) +
                                                   (plane->y * pointY) +
                                                   (plane->z * pointZ)) +
                                                  plane->distance;
                                    if (sign != 0) {
                                        normalValue = -normalValue;
                                    }
                                    if (normalValue > 0.0f) {
                                        valid = 0;
                                    }
                                } while ((edgeOffset < 6) && (valid != 0));
                                if (valid != 0) {
                                    *maximum = intersection;
                                    result->distance = planeDistance;
                                    result->x = planeX;
                                    result->z = planeZ;
                                    resultValue = 1;
                                }
                            }
                        }
                    }
                }
            }
            entryOffset += 4;
        } while (iteration < D_800C9D3C);
    }
    return resultValue;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80010654.s")
#endif
/*
 * PROVENANCE: Mickey's m2c control-flow draft and resident collision
 * records reconstruct this wrapper; no external function body is adapted.
 */
#ifdef NON_MATCHING
typedef struct TrackRayHit {
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 distance;
    f32 x;
    f32 y;
    f32 z;
    f32 ratio;
    s32 faceData;
    u8 material;
} TrackRayHit;

typedef struct TrackRayScratch {
    TrackRayPoint direction;
    u8 result[0x1C];
    f32 length;
} TrackRayScratch;

extern s32 func_80011980(TrackRayPoint *start, TrackRayPoint *end,
                         TrackRayPoint *offset, f32 scale, f32 planeOffset,
                         f32 threshold, TrackRayHit *hit);
extern s32 func_80011CDC(u8 *arg0, u8 *arg1, f32 arg2, u8 *arg3);

/* Workbench verdict: structure-mismatch, 143 differing words, first mismatch +0x0. */
/* Candidate is 145/147 instructions with the target -0xB8 frame; it is not shape-exact. */
/* Remaining gap: structural FP/local-home scheduling; all five call sites are present. */
s32 func_80010900(TrackVec3f *arg0, TrackVec3f *arg1, f32 arg2, s32 arg3,
                  void (*arg4)(void *, void *, f32 *, f32, void *, s32)) {
    TrackRayScratch scratch;
    s32 sp6C;
    s32 sp68;
    f32 temp_f0;
    f32 temp_f18;
    f32 temp_f20;
    f32 temp_f8;
    s32 var_s2;
    s32 var_s4;
    s32 var_s7;
    s32 var_v0;
    sp6C = 0;
    sp68 = 0;
    var_s7 = 0;
    do {
        var_s2 = 0;
        var_s4 = 0;
        scratch.direction.x = arg1->f[0] - arg0->f[0];
        temp_f18 = arg1->f[1] - arg0->f[1];
        scratch.direction.y = temp_f18;
        temp_f8 = arg1->f[2] - arg0->f[2];
        scratch.direction.z = temp_f8;
        temp_f20 = (temp_f8 * temp_f8) +
                   ((scratch.direction.x * scratch.direction.x) +
                    (temp_f18 * temp_f18));
        if (temp_f20 > 0.0f) {
            temp_f0 = sqrtf(temp_f20);
            scratch.length = temp_f0;
            scratch.direction.x /= scratch.length;
            scratch.direction.y /= scratch.length;
            scratch.direction.z /= scratch.length;
            if (D_800C9D28 != 0) {
                var_v0 = func_80011980(arg0, arg1, &scratch.direction,
                                       scratch.length, arg2, 0.0f,
                                       (TrackRayHit *) scratch.result);
            } else {
                var_v0 = func_80011980(arg0, arg1, &scratch.direction,
                                       scratch.length, arg2, arg2,
                                       (TrackRayHit *) scratch.result);
            }
            if (D_800C9D28 != 0) {
                var_s4 = func_80011CDC((u8 *) arg0,
                                       (u8 *) &scratch.direction, arg2,
                                       scratch.result);
            }
            if ((var_v0 | var_s4) != 0) {
                sp68 = 1;
                arg4(arg0, arg1, (f32 *) &scratch.direction, scratch.length,
                     scratch.result, arg3);
                var_s2 = 1;
            }
            if (var_s2 != 0) {
                var_s7 += 1;
                if (var_s7 >= 6) {
                    sp68 = 0;
                    sp6C |= 0x40000000;
                    var_s2 = 0;
                    arg1->f[0] = arg0->f[0];
                    arg1->f[1] = arg0->f[1];
                    arg1->f[2] = arg0->f[2];
                }
            }
        }
    } while (var_s2 != 0);
    return sp68 | sp6C;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80010900.s")
#endif
#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c collision-response draft and the resident ray
 * helper declarations reconstruct this player-intersection loop; no external
 * function body is adapted. The record writes retain the assembly offsets.
 */
/* Workbench verdict: structure-mismatch, 677 differing words, first mismatch +0x0. */
/* Candidate is 644/678 instructions with the target -0x148 frame; it is not shape-exact. */
/* Remaining gap: 34 missing instructions and unresolved unrolled-copy/record-call scheduling. */
struct TrackCollisionSurface;
struct TrackCollisionRecord;
extern void func_800115E4(
    s32 mode, TrackVec3f *position, TrackVec3f *offset, f32 scale,
    struct TrackCollisionSurface *surface,
    struct TrackCollisionRecord *record);

#define B4C_U8(base, offset) (*(u8 *) ((u8 *) (base) + (offset)))
#define B4C_S32(base, offset) (*(s32 *) ((u8 *) (base) + (offset)))
#define B4C_F32(base, offset) (*(f32 *) ((u8 *) (base) + (offset)))

s32 func_80010B4C(s32 arg0, void *arg1, f32 *arg2, f32 *arg3,
                  s32 arg4, void *arg5) {
    f32 relative[16];
    TrackRayPoint direction;
    TrackRayHit intersection;
    u8 *start;
    u8 *end;
    u8 *record;
    f32 lengthSquared;
    f32 length;
    f32 offsetX;
    f32 offsetY;
    f32 offsetZ;
    f32 minimumLength;
    s32 minimumIndex;
    s32 count;
    s32 index;
    s32 attempt;
    s32 bit;
    s32 collisionMask;
    s32 resultMask;
    s32 collision;
    s32 queryResult;
    s32 auxiliaryResult;

    if (arg5 != NULL) {
        offsetX = B4C_F32(arg5, 0);
        offsetY = B4C_F32(arg5, 4);
        offsetZ = B4C_F32(arg5, 8);
        for (index = 0; index < arg0; index++) {
            relative[index * 3] =
                ((f32 *) arg2)[index * 3] - offsetX;
            relative[(index * 3) + 1] =
                ((f32 *) arg2)[(index * 3) + 1] - offsetY;
            relative[(index * 3) + 2] =
                ((f32 *) arg2)[(index * 3) + 2] - offsetZ;
        }
    }
    count = 0;
    if (arg0 > 0) {
        for (index = 0; index < arg0; index++) {
            record = (u8 *) ((u32) arg4 + (index * 0x40));
            B4C_U8(record, 0) = 0;
            B4C_U8(record, 0x3D) = 0;
            B4C_F32(record, 4) = 0.0f;
            B4C_F32(record, 8) = 0.0f;
            B4C_F32(record, 0xC) = 0.0f;
            B4C_F32(record, 0x10) = 0.0f;
            B4C_F32(record, 0x14) = 0.0f;
            B4C_F32(record, 0x18) = 0.0f;
            B4C_F32(record, 0x1C) = 0.0f;
            B4C_F32(record, 0x20) = 0.0f;
            B4C_F32(record, 0x24) = 0.0f;
            B4C_F32(record, 0x28) = 0.0f;
            B4C_F32(record, 0x2C) = 0.0f;
            B4C_F32(record, 0x30) = 0.0f;
            B4C_F32(record, 0x34) = 32000.0f;
            B4C_S32(record, 0x38) = 0;
            B4C_U8(record, 0x3C) = 0;
        }
    }
    resultMask = 0;
    attempt = 0;
    collisionMask = 0;
    do {
        collisionMask = 0;
        bit = 1;
        for (index = 0; index < arg0; index++) {
            start = (u8 *) arg1 + (index * 0xC);
            end = (u8 *) arg2 + (index * 0xC);
            direction.x = B4C_F32(end, 0) - B4C_F32(start, 0);
            direction.y = B4C_F32(end, 4) - B4C_F32(start, 4);
            direction.z = B4C_F32(end, 8) - B4C_F32(start, 8);
            lengthSquared = (direction.z * direction.z) +
                            ((direction.x * direction.x) +
                             (direction.y * direction.y));
            collision = 0;
            if (lengthSquared > 0.0f) {
                length = sqrtf(lengthSquared);
                direction.x /= length;
                direction.y /= length;
                direction.z /= length;
                if (D_800C9D28 != 0) {
                    queryResult = func_80011980(
                        (TrackRayPoint *) start, (TrackRayPoint *) end,
                        &direction, length, arg3[index], 0.0f,
                        &intersection);
                } else {
                    queryResult = func_80011980(
                        (TrackRayPoint *) start, (TrackRayPoint *) end,
                        &direction, length, arg3[index], arg3[index],
                        &intersection);
                }
                auxiliaryResult = 0;
                if (D_800C9D28 != 0) {
                    auxiliaryResult = func_80011CDC(
                        start, (u8 *) &direction, arg3[index],
                        (u8 *) &intersection);
                }
                if ((queryResult | auxiliaryResult) != 0) {
                    record = (u8 *) ((u32) arg4 + (index * 0x40));
                    func_800115E4(
                        (s32) start, (TrackVec3f *) end, &direction, length,
                        (struct TrackCollisionSurface *) &intersection,
                        (struct TrackCollisionRecord *) record);
                    B4C_F32(record, 0x34) = length;
                    collision = 1;
                    collisionMask |= bit;
                }
            }
            if (collision != 0) {
                count++;
                if (count >= 0xB) {
                    collisionMask = 0;
                    collision = 0;
                    resultMask |= 0x40000000;
                    for (index = index; index < arg0; index++) {
                        ((f32 *) arg2)[index * 3] =
                            ((f32 *) arg1)[index * 3];
                        ((f32 *) arg2)[(index * 3) + 1] =
                            ((f32 *) arg1)[(index * 3) + 1];
                        ((f32 *) arg2)[(index * 3) + 2] =
                            ((f32 *) arg1)[(index * 3) + 2];
                    }
                    break;
                }
            }
            bit <<= 1;
        }
        if (((collisionMask != 0) && (attempt >= 0xB)) ||
            (resultMask != 0)) {
            for (index = 0; index < arg0; index++) {
                ((f32 *) arg2)[index * 3] =
                    ((f32 *) arg1)[index * 3];
                ((f32 *) arg2)[(index * 3) + 1] =
                    ((f32 *) arg1)[(index * 3) + 1];
                ((f32 *) arg2)[(index * 3) + 2] =
                    ((f32 *) arg1)[(index * 3) + 2];
            }
            collisionMask = 0;
            if (attempt >= 0xB) {
                resultMask |= 0x80000000;
            }
        } else if (collisionMask != 0) {
            minimumLength = 32000.0f;
            minimumIndex = 0;
            if (arg5 != NULL) {
                bit = 1;
                for (index = 0; index < arg0; index++) {
                    if ((collisionMask & bit) != 0) {
                        record = (u8 *) ((u32) arg4 + (index * 0x40));
                        if (B4C_F32(record, 0x34) < minimumLength) {
                            minimumIndex = index;
                            minimumLength = B4C_F32(record, 0x34);
                        }
                    }
                    bit <<= 1;
                }
                record = (u8 *) ((u32) arg4 + (minimumIndex * 0x40));
                B4C_U8(record, 0x3D) |= 1;
                offsetX = B4C_F32(arg2, minimumIndex * 0xC) -
                          relative[minimumIndex * 3];
                offsetY = B4C_F32(arg2, (minimumIndex * 0xC) + 4) -
                          relative[(minimumIndex * 3) + 1];
                offsetZ = B4C_F32(arg2, (minimumIndex * 0xC) + 8) -
                          relative[(minimumIndex * 3) + 2];
                B4C_F32(arg5, 0) = offsetX;
                B4C_F32(arg5, 4) = offsetY;
                B4C_F32(arg5, 8) = offsetZ;
                for (index = 0; index < arg0; index++) {
                    B4C_F32(arg2, index * 0xC) =
                        relative[index * 3] + offsetX;
                    B4C_F32(arg2, (index * 0xC) + 4) =
                        relative[(index * 3) + 1] + offsetY;
                    B4C_F32(arg2, (index * 0xC) + 8) =
                        relative[(index * 3) + 2] + offsetZ;
                }
                resultMask |= collisionMask;
            }
        }
        attempt++;
    } while ((collisionMask != 0) && ((resultMask & 0xC0000000) == 0));
    return resultMask;
}
#undef B4C_U8
#undef B4C_S32
#undef B4C_F32
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80010B4C.s")
#endif
#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c collision-response draft and resident plane and
 * record offsets reconstruct this routine; no external function body is adapted.
 */
typedef struct TrackCollisionSurface {
    f32 x;
    f32 y;
    f32 z;
    f32 distance;
    f32 positionX;
    f32 positionY;
    f32 positionZ;
    f32 positionDistance;
    u32 flags;
    u8 material;
} TrackCollisionSurface;

typedef struct TrackCollisionRecord {
    f32 pointX;
    f32 pointY;
    f32 pointZ;
    f32 value0C;
    f32 value10;
    f32 value14;
    f32 value18;
    f32 value1C;
    f32 value20;
    f32 value24;
    f32 value28;
    f32 value2C;
    f32 value30;
    u8 pad34[4];
    s32 value38;
    u8 value3C;
} TrackCollisionRecord;

s16 Arctanf(f32 x, f32 y);

/* Workbench verdict: structure-mismatch, 232 differing words, first mismatch +0x0. */
/* Candidate: 236/231 instructions with a -0xA0 frame versus target -0x98; five-instruction FP schedule residual remains. */
/* Shape status: three surface branches, normalization, and collision-record flag writes are preserved, but it is not shape-exact. */
void func_800115E4(s32 mode, TrackVec3f *position, TrackVec3f *offset,
                   f32 scale, TrackCollisionSurface *surface,
                   TrackCollisionRecord *record) {
    f32 surfaceDistance;
    f32 surfaceX;
    f32 surfaceY;
    f32 surfaceZ;
    f32 productX;
    f32 productZ;
    f32 planeValue;
    f32 crossX;
    f32 crossY;
    f32 crossZ;
    f32 crossLengthSquared;
    f32 crossLength;
    f32 distance;
    f32 time;
    f32 projectedX;
    f32 projectedY;
    f32 projectedZ;
    f32 differenceX;
    f32 differenceY;
    f32 differenceZ;
    f32 angle;
    f32 horizontalLength;
    s16 angleValue;

    surfaceDistance = surface->distance;
    surfaceX = surface->x;
    surfaceY = surface->y;
    surfaceZ = surface->z;
    productZ = position->f[2] * surfaceZ;
    productX = surfaceX * position->f[0];
    planeValue = (productZ + (productX + (surfaceY * position->f[1]))) +
                 surfaceDistance;
    if ((D_80081778 <= surfaceY) || ((surface->flags << 3) < 0)) {
        crossX = offset->f[2] * surfaceY;
        crossY = (surfaceZ * offset->f[0]) -
                 (offset->f[2] * surfaceX);
        crossZ = -(offset->f[0] * surfaceY);
        crossX = (crossY * surfaceZ) - (crossZ * surfaceY);
        crossY = (crossZ * surfaceX) - (productZ * surfaceZ);
        crossZ = (productZ * surfaceY) - (crossY * surfaceX);
        crossLengthSquared = (crossX * crossX) +
                             (crossY * crossY) +
                             (crossZ * crossZ);
        if (D_8008177C < crossLengthSquared) {
            distance = sqrtf(crossLengthSquared);
            time = scale - surface->positionDistance;
            position->f[0] = surface->positionX +
                             (time * (crossX / distance));
            position->f[1] = surface->positionY +
                             (time * (crossY / distance));
            position->f[2] = surface->positionZ +
                             (time * (crossZ / distance));
        } else {
            position->f[1] = -((productZ + productX + surfaceDistance) /
                               surfaceY) + D_80081780;
        }
        record->pointY = surfaceX;
        record->pointZ = surfaceY;
        record->value0C = surfaceZ;
        record->value3C |= 2;
    } else if (surfaceY <= D_80081784) {
        distance = D_80081788 - planeValue;
        position->f[0] += distance * surfaceX;
        position->f[1] += distance * surfaceY;
        position->f[2] += distance * surfaceZ;
        record->value1C = surfaceX;
        record->value20 = surfaceY;
        record->value24 = surfaceZ;
        record->value3C |= 8;
    } else {
        projectedX = position->f[0];
        projectedY = position->f[1];
        projectedZ = position->f[2];
        distance = D_8008178C - planeValue;
        projectedX = projectedX + (distance * surfaceX);
        projectedY = projectedY + (distance * surfaceY);
        projectedZ = projectedZ + (distance * surfaceZ);
        differenceX = position->f[0] - projectedX;
        differenceY = position->f[1] - projectedY;
        differenceZ = position->f[2] - projectedZ;
        angle = sqrtf((differenceX * differenceX) +
                      (differenceZ * differenceZ));
        angleValue = Arctanf(differenceY, angle);
        angle = func_8002A8BC(angleValue);
        if (angle != 0.0f) {
            time = distance / angle;
            horizontalLength = sqrtf((surfaceX * surfaceX) +
                                     (surfaceZ * surfaceZ));
            position->f[0] += time * (surfaceX / horizontalLength);
            position->f[2] += time * (surfaceZ / horizontalLength);
        } else {
            position->f[0] = projectedX;
            position->f[1] = projectedY;
            position->f[2] = projectedZ;
        }
        record->value10 = surfaceX;
        record->value14 = surfaceY;
        record->value18 = surfaceZ;
        record->value3C |= 4;
    }
    record->value38 = surface->flags;
    record->value24 = surface->material;
    record->value28 = surfaceX;
    record->value2C = surfaceY;
    record->value30 = surfaceZ;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800115E4.s")
#endif
#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c collision-query draft and resident node/plane
 * offsets reconstruct this ray query; no external function body is adapted.
 */
typedef struct TrackRayFace {
    f32 x;
    f32 y;
    f32 z;
    f32 distance;
} TrackRayFace;

typedef struct TrackRayMeta {
    u8 material;
    u8 pad01[0x0B];
    s32 data;
} TrackRayMeta;

typedef struct TrackRayNodeExtended {
    u8 pad00[0x0C];
    TrackRayMeta *metadata;
    u8 pad10[0x0C];
    TrackRayFace *planes;
} TrackRayNodeExtended;

/* Workbench verdict: structure-mismatch, 208 differing words, first mismatch +0x0. */
/* Candidate: 214/215 instructions with a -0xD8 frame versus target -0xC8; one instruction/frame residual and FP schedule remain. */
/* Shape status: encoded-node traversal, signed edge loop, interpolation, and hit stores are preserved, but it is not shape-exact. */
s32 func_80011980(TrackRayPoint *start, TrackRayPoint *end,
                  TrackRayPoint *offset, f32 scale, f32 planeOffset,
                  f32 threshold, TrackRayHit *hit) {
    TrackRayNodeExtended *node;
    TrackRayNodeExtended *entry;
    TrackRayFace *face;
    TrackRayFace *edgeFace;
    f32 planeX;
    f32 planeY;
    f32 planeZ;
    f32 planeValue;
    f32 startValue;
    f32 endValue;
    f32 ratio;
    f32 pointX;
    f32 pointY;
    f32 pointZ;
    f32 edgeValue;
    s32 encoded;
    s32 entryOffset;
    s32 segmentIndex;
    s32 edgeOffset;
    s32 valid;
    s32 sign;
    s32 edgeIndex;
    u16 edge;

    valid = 0;
    segmentIndex = 0;
    entryOffset = 0;
    if (D_800C9D3C > 0) {
        do {
            encoded = *(s32 *) ((u8 *) D_800C9D2C + entryOffset);
            if (encoded > 0) {
                node = (TrackRayNodeExtended *) (encoded | (s32) 0x80000000);
            } else {
                entry = (TrackRayNodeExtended *) encoded;
                face = (TrackRayFace *) ((u8 *) node->planes +
                                         (*(u16 *) entry * 0x10));
                planeX = face->x;
                planeY = face->y;
                planeZ = face->z;
                planeValue = face->distance - planeOffset;
                endValue = ((end->z * planeZ) +
                            ((planeX * end->x) + (planeY * end->y))) +
                           planeValue;
                if (endValue < 0.0f) {
                    startValue = ((start->z * planeZ) +
                                  ((planeX * start->x) +
                                   (planeY * start->y))) +
                                 planeValue;
                    if (startValue >= 0.0f) {
                        ratio = (startValue / (startValue - endValue)) * scale;
                        if (ratio <= hit->ratio) {
                            pointX = ((offset->x * ratio) + start->x) -
                                     (planeOffset * planeX);
                            pointY = ((offset->y * ratio) + start->y) -
                                     (planeOffset * planeY);
                            pointZ = ((offset->z * ratio) + start->z) -
                                     (planeOffset * planeZ);
                            valid = 1;
                            edgeOffset = 0;
                            do {
                                edgeOffset += 2;
                                edge = *(u16 *) ((u8 *) entry + edgeOffset);
                                sign = edge & 0x8000;
                                edgeIndex = edge ^ sign;
                                edgeFace = (TrackRayFace *)
                                    ((u8 *) node->planes + (edgeIndex * 0x10));
                                edgeValue = edgeFace->distance +
                                             ((edgeFace->x * pointX) +
                                              (edgeFace->y * pointY) +
                                              (edgeFace->z * pointZ));
                                if (sign != 0) {
                                    edgeValue = -edgeValue;
                                }
                                if (threshold < edgeValue) {
                                    valid = 0;
                                }
                            } while ((edgeOffset < 6) && (valid != 0));
                            if (valid != 0) {
                                hit->normalX = planeX;
                                hit->normalY = planeY;
                                hit->normalZ = planeZ;
                                hit->distance = planeValue;
                                hit->x = ((D_80081790 + planeOffset) * planeX) + pointX;
                                hit->y = ((D_80081790 + planeOffset) * planeY) + pointY;
                                hit->z = ((D_80081790 + planeOffset) * planeZ) + pointZ;
                                {
                                    TrackRayMeta *meta = (TrackRayMeta *)
                                        ((u8 *) node->metadata +
                                         (D_800C9D30[segmentIndex] * 0x10));
                                    hit->faceData = meta->data;
                                    hit->material = ((u8 *) D_800792E8->textures)[
                                        (meta->material * 8) + 7];
                                }
                                hit->ratio = ratio;
                                valid = 1;
                            }
                        }
                    }
                }
            }
            segmentIndex++;
            entryOffset += 4;
        } while (segmentIndex < D_800C9D3C);
    }
    return valid;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80011980.s")
#endif
#ifdef NON_MATCHING
/* PROVENANCE: JFG's public track.c supplies the ray/edge collision role;
 * this body uses Mickey's resident edge records and output layout. */
/* Workbench verdict: structure-mismatch, 330 differing words; first mismatch is at +0x0. */
/* Target is 342 instructions/frame -192; candidate is 331 instructions/frame -200. */
/* Remaining gap is structural: edge/sphere fallback and hit metadata scheduling differ; not permuter-ready. */
extern s32 func_80012234(TrackVec3f *point, TrackVec3f *direction,
                         TrackVec3f *origin, TrackVec3f *planeDirection,
                         f32 radius, f32 *minimum, f32 *maximum);

s32 func_80011CDC(u8 *arg0, u8 *arg1, f32 arg2, u8 *arg3) {
    u8 *record;
    u8 *metadata;
    u8 *metadataEntry;
    f32 t;
    f32 tEnd;
    f32 pointX;
    f32 pointY;
    f32 pointZ;
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 planeDistance;
    s32 recordIndex;
    s32 recordCount;
    s32 hit;
    s32 edgeHit;
    s32 metadataIndex;

    hit = 0;
    recordCount = 0;
    if (D_800C9D24 > 0) {
        do {
            edgeHit = 0;
            record = (u8 *) D_800C9D20 + (recordCount * 0x2C);
            if ((func_80012234((TrackVec3f *) arg0, (TrackVec3f *) arg1,
                               (TrackVec3f *) record,
                               (TrackVec3f *) (record + 0x18),
                               arg2, &t, &tEnd) != 0) &&
                (t >= 0.0f) && (t <= *(f32 *) (arg3 + 0x1C))) {
                normalX = *(f32 *) (record + 0x18);
                normalY = *(f32 *) (record + 0x1C);
                normalZ = *(f32 *) (record + 0x20);
                pointX = (*(f32 *) (arg1 + 0) * t) +
                         *(f32 *) (arg0 + 0);
                pointY = (*(f32 *) (arg1 + 4) * t) +
                         *(f32 *) (arg0 + 4);
                pointZ = (*(f32 *) (arg1 + 8) * t) +
                         *(f32 *) (arg0 + 8);
                planeDistance = (((pointX - *(f32 *) (record + 0)) * normalX) +
                                  ((pointY - *(f32 *) (record + 4)) * normalY) +
                                  ((pointZ - *(f32 *) (record + 8)) * normalZ)) /
                                 ((normalZ * normalZ) +
                                  ((normalX * normalX) + (normalY * normalY)));
                if ((planeDistance >= 0.0f) && (planeDistance <= 1.0f)) {
                    *(f32 *) (arg3 + 0x10) = pointX;
                    *(f32 *) (arg3 + 0x14) = pointY;
                    *(f32 *) (arg3 + 0x18) = pointZ;
                    hit = 1;
                    edgeHit = 1;
                    *(f32 *) (arg3 + 0) =
                        (pointX - ((normalX * planeDistance) +
                                   *(f32 *) (record + 0))) / arg2;
                    normalX = (pointZ - ((normalZ * planeDistance) +
                                         *(f32 *) (record + 8))) / arg2;
                    *(f32 *) (arg3 + 4) =
                        (pointY - ((normalY * planeDistance) +
                                   *(f32 *) (record + 4))) / arg2;
                    *(f32 *) (arg3 + 8) = normalX;
                    *(f32 *) (arg3 + 0xC) =
                        -((normalX * pointZ) +
                          ((pointX * *(f32 *) (arg3 + 0)) +
                           (pointY * *(f32 *) (arg3 + 4))));
                    metadata = *(u8 **) (record + 0x24);
                    metadataIndex = *(s32 *) (record + 0x28);
                    metadataEntry = *(u8 **) (metadata + 0xC) +
                                    (metadataIndex * 0x10);
                    *(s32 *) (arg3 + 0x20) = *(s32 *) (metadataEntry + 0xC);
                    *(u8 *) (arg3 + 0x24) = *((u8 *) D_800792E8->textures +
                                              (*(u8 *) metadataEntry * 8) + 7);
                    *(f32 *) (arg3 + 0x1C) = t;
                }
            }
            if (edgeHit == 0) {
                if ((func_80012574((TrackVec3f *) arg0, (TrackVec3f *) arg1,
                                   (TrackVec3f *) record,
                                   arg2, &t, &tEnd) != 0) &&
                    (t >= 0.0f) && (t <= *(f32 *) (arg3 + 0x1C))) {
                    edgeHit = 1;
                    hit = 1;
                    pointX = (*(f32 *) (arg1 + 0) * t) +
                             *(f32 *) (arg0 + 0);
                    pointY = (*(f32 *) (arg1 + 4) * t) +
                             *(f32 *) (arg0 + 4);
                    pointZ = (*(f32 *) (arg1 + 8) * t) +
                             *(f32 *) (arg0 + 8);
                    *(f32 *) (arg3 + 0x10) = pointX;
                    *(f32 *) (arg3 + 0x14) = pointY;
                    *(f32 *) (arg3 + 0x18) = pointZ;
                    *(f32 *) (arg3 + 0) =
                        (pointX - *(f32 *) (record + 0)) / arg2;
                    *(f32 *) (arg3 + 4) =
                        (pointY - *(f32 *) (record + 4)) / arg2;
                    *(f32 *) (arg3 + 8) =
                        (pointZ - *(f32 *) (record + 8)) / arg2;
                    *(f32 *) (arg3 + 0xC) =
                        -((*(f32 *) (arg3 + 8) * pointZ) +
                          ((pointX * *(f32 *) (arg3 + 0)) +
                           (pointY * *(f32 *) (arg3 + 4))));
                    metadata = *(u8 **) (record + 0x24);
                    metadataIndex = *(s32 *) (record + 0x28);
                    metadataEntry = *(u8 **) (metadata + 0xC) +
                                    (metadataIndex * 0x10);
                    *(s32 *) (arg3 + 0x20) = *(s32 *) (metadataEntry + 0xC);
                    *(u8 *) (arg3 + 0x24) = *((u8 *) D_800792E8->textures +
                                              (*(u8 *) metadataEntry * 8) + 7);
                    *(f32 *) (arg3 + 0x1C) = t;
                }
            }
            if (edgeHit == 0) {
                if ((func_80012574((TrackVec3f *) arg0, (TrackVec3f *) arg1,
                                   (TrackVec3f *) (record + 0xC), arg2,
                                   &t, &tEnd) != 0) &&
                    (t >= 0.0f) && (t <= *(f32 *) (arg3 + 0x1C))) {
                    hit = 1;
                    pointX = (*(f32 *) (arg1 + 0) * t) +
                             *(f32 *) (arg0 + 0);
                    pointY = (*(f32 *) (arg1 + 4) * t) +
                             *(f32 *) (arg0 + 4);
                    pointZ = (*(f32 *) (arg1 + 8) * t) +
                             *(f32 *) (arg0 + 8);
                    *(f32 *) (arg3 + 0x10) = pointX;
                    *(f32 *) (arg3 + 0x14) = pointY;
                    *(f32 *) (arg3 + 0x18) = pointZ;
                    *(f32 *) (arg3 + 0) =
                        (pointX - *(f32 *) (record + 0xC)) / arg2;
                    *(f32 *) (arg3 + 4) =
                        (pointY - *(f32 *) (record + 0x10)) / arg2;
                    *(f32 *) (arg3 + 8) =
                        (pointZ - *(f32 *) (record + 0x14)) / arg2;
                    *(f32 *) (arg3 + 0xC) =
                        -((*(f32 *) (arg3 + 8) * pointZ) +
                          ((pointX * *(f32 *) (arg3 + 0)) +
                           (pointY * *(f32 *) (arg3 + 4))));
                    metadata = *(u8 **) (record + 0x24);
                    metadataIndex = *(s32 *) (record + 0x28);
                    metadataEntry = *(u8 **) (metadata + 0xC) +
                                    (metadataIndex * 0x10);
                    *(s32 *) (arg3 + 0x20) = *(s32 *) (metadataEntry + 0xC);
                    *(u8 *) (arg3 + 0x24) = *((u8 *) D_800792E8->textures +
                                              (*(u8 *) metadataEntry * 8) + 7);
                    *(f32 *) (arg3 + 0x1C) = t;
                }
            }
            recordCount++;
        } while (recordCount < D_800C9D24);
    }
    return hit;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80011CDC.s")
#endif
#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c FP dataflow and the resident vector layout
 * reconstruct this plane-intersection query; no external function body is adapted.
 */
/* Workbench verdict: structure-mismatch, 208 differing words, first mismatch +0x0. */
/* Candidate: 175/208 instructions with a -0xA0 frame versus target -0x60; FP temporary lifetime and cross-product schedule remain unresolved. */
/* Shape status: interval calculation and square-root paths are reconstructed, but the candidate is not shape-exact. */
s32 func_80012234(TrackVec3f *point, TrackVec3f *direction,
                  TrackVec3f *origin, TrackVec3f *planeDirection,
                  f32 radius, f32 *minimum, f32 *maximum) {
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 normalLengthSquared;
    f32 normalLength;
    f32 dot;
    f32 absoluteDot;
    f32 crossX;
    f32 crossY;
    f32 crossZ;
    f32 secondLengthSquared;
    f32 secondLength;
    f32 unitX;
    f32 unitY;
    f32 unitZ;
    f32 planeOffset;
    f32 directionDot;
    f32 interval;
    s32 result;

    deltaX = point->f[0] - origin->f[0];
    deltaY = point->f[1] - origin->f[1];
    deltaZ = point->f[2] - origin->f[2];
    normalX = (direction->f[1] * planeDirection->f[2]) -
              (planeDirection->f[1] * direction->f[2]);
    normalY = (direction->f[2] * planeDirection->f[0]) -
              (planeDirection->f[2] * direction->f[0]);
    normalZ = (direction->f[0] * planeDirection->f[1]) -
              (planeDirection->f[0] * direction->f[1]);
    normalLengthSquared = (normalZ * normalZ) +
                          ((normalX * normalX) + (normalY * normalY));
    if (normalLengthSquared == 0.0f) {
        return 0;
    }
    normalLength = sqrtf(normalLengthSquared);
    unitZ = normalZ / normalLength;
    unitX = normalX / normalLength;
    unitY = normalY / normalLength;
    normalZ = unitZ;
    normalX = unitX;
    normalY = unitY;
    dot = (unitZ * deltaZ) + ((deltaX * unitX) + (deltaY * unitY));
    absoluteDot = dot;
    if (absoluteDot < 0.0f) {
        absoluteDot = -dot;
    }
    result = 0;
    if (absoluteDot <= radius) {
        result = 1;
    }
    if (result != 0) {
        crossX = (deltaY * planeDirection->f[2]) -
                 (planeDirection->f[1] * deltaZ);
        crossY = (deltaZ * planeDirection->f[0]) -
                 (planeDirection->f[2] * deltaX);
        crossZ = (deltaX * planeDirection->f[1]) -
                 (planeDirection->f[0] * deltaY);
        planeOffset = -((normalZ * crossZ) +
                        ((crossX * normalX) + (crossY * normalY))) /
                      normalLength;
        crossX = (normalY * planeDirection->f[2]) -
                 (planeDirection->f[1] * normalZ);
        crossY = (normalZ * planeDirection->f[0]) -
                 (planeDirection->f[2] * normalX);
        crossZ = (normalX * planeDirection->f[1]) -
                 (planeDirection->f[0] * normalY);
        secondLengthSquared = (crossZ * crossZ) +
                              ((crossX * crossX) + (crossY * crossY));
        secondLength = sqrtf(secondLengthSquared);
        unitX = crossX / secondLength;
        unitY = crossY / secondLength;
        unitZ = crossZ / secondLength;
        directionDot = (unitZ * direction->f[2]) +
                       ((direction->f[0] * unitX) +
                        (direction->f[1] * unitY));
        interval = sqrtf((radius * radius) -
                         (absoluteDot * absoluteDot)) /
                   directionDot;
        if (interval < 0.0f) {
            interval = -interval;
        }
        *minimum = planeOffset - interval;
        *maximum = planeOffset + interval;
    }
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80012234.s")
#endif
s32 func_80012574(TrackVec3f *origin, TrackVec3f *direction, TrackVec3f *center, f32 radius, f32 *minimum, f32 *maximum)
{
  f32 temp_f0;
  f32 temp_f0_2;
  f32 temp_f12;
  f32 temp_f14;
  float new_var2;
  f32 temp_f16;
  f32 temp_f18;
  f32 temp_f2;
  f32 temp_f2_2;
  f32 new_var;
  s32 var_v1;
  temp_f0 = origin->f[0] - center->f[0];
  temp_f2 = origin->f[1] - center->f[1];
  var_v1 = 0;
  temp_f12 = origin->f[2] - center->f[2];
  temp_f14 = ((temp_f0 * direction->f[0]) + (temp_f2 * direction->f[1])) + (temp_f12 * direction->f[2]);
  new_var = temp_f14;
  new_var2 = (((temp_f0 * temp_f0) + (temp_f2 * temp_f2)) + (temp_f12 * temp_f12)) - (radius * radius);
  temp_f18 = new_var * new_var;
  temp_f16 = new_var2;
  if (temp_f16 <= temp_f18)
  {
    var_v1 = 1;
  }
  if (var_v1 != 0)
  {
    temp_f0_2 = sqrtf(temp_f18 - temp_f16);
 do { temp_f2_2 = -new_var; *minimum = temp_f2_2 - temp_f0_2; *maximum = temp_f2_2 + temp_f0_2; } while (0);
  }
  return var_v1;
}
#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c draft, collision-node offsets, and output-record
 * writes reconstruct this routine; no external function body is adapted.
 */
typedef struct TrackClipNode {
    void *origin;
    void *faces;
    u8 pad08[4];
    void *indices;
} TrackClipNode;

typedef struct TrackClipFace {
    u8 flags;
    u8 vertex;
    u8 pad02[0x0E];
} TrackClipFace;

typedef struct TrackClipVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 pad06[4];
} TrackClipVertex;

typedef struct TrackClipOutput {
    f32 x0;
    f32 y0;
    f32 z0;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 dx;
    f32 dy;
    f32 dz;
    TrackClipNode *node;
    s16 segment;
} TrackClipOutput;

/* Workbench verdict: structure-mismatch, 157 differing words, first mismatch +0x0. */
/* Candidate: 174/177 instructions with a -0x38 frame versus target -0x40; three instruction/frame-padding residuals remain. */
/* Shape status: encoded-node traversal, three-corner bit scan, and 0x2C output stride are preserved, but it is not shape-exact. */
void func_80012658(s32 flags) {
    TrackClipNode *node;
    TrackClipNode *entry;
    TrackClipFace *face;
    TrackClipVertex *vertex;
    TrackClipOutput *output;
    s32 encoded;
    s32 nodeIndex;
    s32 faceOffset;
    s32 vertexOffset;
    s32 corner;
    s32 nextCorner;
    s32 mask;
    s32 outputIndex;
    s16 faceIndex;
    s16 vertexIndex;
    s16 segmentIndex;

    D_800C9D24 = 0;
    if ((flags & 1) == 0) {
        D_800C9D28 = 0;
        return;
    }
    D_800C9D28 = 1;
    nodeIndex = 0;
    if (D_800C9D3C > 0) {
        do {
            faceOffset = nodeIndex * 2;
            encoded = D_800C9D2C[nodeIndex];
            if (encoded > 0) {
                node = (TrackClipNode *) (encoded | (s32) 0x80000000);
            } else {
                segmentIndex = D_800C9D34[nodeIndex];
                faceIndex = D_800C9D30[nodeIndex];
                face = (TrackClipFace *) ((u8 *) node->faces +
                                          (segmentIndex * 0x10));
                vertex = (TrackClipVertex *) ((u8 *) node->origin +
                                              (*(s16 *) ((u8 *) node->indices +
                                                         (faceIndex * 0x10) +
                                                         6) * 0x0A));
                corner = 0;
                do {
                    if (face->flags & (1 << corner)) {
                        nextCorner = corner + 1;
                        if (nextCorner >= 3) {
                            nextCorner = 0;
                        }
                        output = (TrackClipOutput *) ((u8 *) D_800C9D20 +
                                                      (D_800C9D24 * 0x2C));
                        vertexIndex = (s16) (((u8 *) face)[corner + 1]);
                        {
                            TrackClipVertex *first = (TrackClipVertex *)
                                ((u8 *) vertex + (vertexIndex * 0x0A));
                            TrackClipVertex *second = (TrackClipVertex *)
                                ((u8 *) vertex +
                                 (((u8 *) face)[nextCorner + 1] * 0x0A));
                            output->x0 = (f32) first->x;
                            output->y0 = (f32) first->y;
                            output->z0 = (f32) first->z;
                            output->x1 = (f32) second->x;
                            output->y1 = (f32) second->y;
                            output->z1 = (f32) second->z;
                            output->dx = output->x1 - output->x0;
                            output->node = node;
                            output->dy = output->y1 - output->y0;
                            output->dz = output->z1 - output->z0;
                            output->segment = D_800C9D30[nodeIndex];
                        }
                        outputIndex = D_800C9D24 + 1;
                        D_800C9D24 = outputIndex;
                        if (outputIndex >= *(s16 *) ((u8 *) D_800792EC + 0xF0)) {
                            corner = 3;
                            nodeIndex = D_800C9D3C;
                        }
                    }
                    corner++;
                } while (corner < 3);
            }
            nodeIndex++;
        } while (nodeIndex < D_800C9D3C);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80012658.s")
#endif
#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c collision trace and the resident vector/track
 * declarations reconstruct this query; no external function body is adapted.
 * Raw offsets retain the compact segment and polygon records.
 */
/* Workbench verdict: structure-mismatch, 545 differing words, first mismatch +0x0. */
/* Candidate is 517/548 instructions with frame -0x2A8 versus target -0x288; it is not shape-exact. */
/* Remaining gap: 31 missing instructions, 32 excess frame bytes, and unresolved polygon/relocation scheduling. */
extern s32 func_800131AC(TrackVec3f *origin, TrackVec3f *direction,
                         TrackVec3f *minimum, TrackVec3f *maximum,
                         f32 *nearClip, f32 *farClip);
extern u8 getYCompareMask(void *bounds, s32 y0, s32 y1);

#define E129_U8(base, offset) (*(u8 *) ((u8 *) (base) + (offset)))
#define E129_S16(base, offset) (*(s16 *) ((u8 *) (base) + (offset)))
#define E129_U16(base, offset) (*(u16 *) ((u8 *) (base) + (offset)))
#define E129_S32(base, offset) (*(s32 *) ((u8 *) (base) + (offset)))
#define E129_F32(base, offset) (*(f32 *) ((u8 *) (base) + (offset)))
#define E129_PTR(base, offset) (*(void **) ((u8 *) (base) + (offset)))

s32 func_8001291C(f32 *arg0, f32 *arg1, f32 *arg2, s32 arg3, s32 arg4) {
    void *segments[20];
    f32 entryTimes[20];
    s32 xzMasks[20];
    u8 yMasks[20];
    TrackVec3f direction;
    TrackVec3f minimum;
    TrackVec3f maximum;
    TrackBoundingBox *bounds;
    TrackData *track;
    void *segment;
    void *batch;
    void *batchRecord;
    void *surfaceBase;
    void *plane;
    void *bestPlane;
    u16 *polygon;
    f32 nearClip;
    f32 farClip;
    f32 bestDistance;
    f32 side0;
    f32 side1;
    f32 fraction;
    f32 pointX;
    f32 pointY;
    f32 pointZ;
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 planeDistance;
    f32 edgeValue;
    s32 segmentCount;
    s32 segmentIndex;
    s32 hitCount;
    s32 insertIndex;
    s32 batchCount;
    s32 batchIndex;
    s32 triangleIndex;
    s32 firstTriangle;
    s32 lastTriangle;
    s32 edgeIndex;
    s32 inside;
    s32 hit;
    s32 bestFlags;
    s32 bestTexture;
    s32 x0;
    s32 y0;
    s32 z0;
    s32 x1;
    s32 y1;
    s32 z1;
    s32 edgeNumber;
    u16 edge;
    u16 edgeSign;
    u8 *segmentBytes;
    u8 *surfaceBytes;

    track = D_800792E8;
    direction.f[0] = arg1[0] - arg0[0];
    direction.f[1] = arg1[1] - arg0[1];
    direction.f[2] = arg1[2] - arg0[2];
    hitCount = 0;
    segmentCount = E129_S16(track, 0x1A);
    if ((direction.f[0] != 0.0f) || (direction.f[1] != 0.0f) ||
        (direction.f[2] != 0.0f)) {
        for (segmentIndex = 0; segmentIndex < segmentCount; segmentIndex++) {
            bounds = track->segmentBounds + segmentIndex;
            minimum.f[0] = (f32) bounds->x1;
            minimum.f[1] = (f32) bounds->y1;
            minimum.f[2] = (f32) bounds->z1;
            maximum.f[0] = (f32) bounds->x2;
            maximum.f[1] = (f32) bounds->y2;
            maximum.f[2] = (f32) bounds->z2;
            if ((func_800131AC((TrackVec3f *) arg0, &direction,
                               &minimum, &maximum, &nearClip, &farClip) != 0) &&
                (((nearClip <= 0.0f) && (farClip >= 0.0f)) ||
                 ((nearClip >= 0.0f) && (nearClip <= 1.0f)))) {
                if (nearClip < 0.0f) {
                    nearClip = 0.0f;
                }
                if (farClip > 1.0f) {
                    farClip = 1.0f;
                }
                x0 = (s32) ((direction.f[0] * nearClip) + arg0[0]);
                y0 = (s32) ((direction.f[1] * nearClip) + arg0[1]);
                z0 = (s32) ((direction.f[2] * nearClip) + arg0[2]);
                x1 = (s32) ((direction.f[0] * farClip) + arg0[0]);
                y1 = (s32) ((direction.f[1] * farClip) + arg0[1]);
                z1 = (s32) ((direction.f[2] * farClip) + arg0[2]);
                if (x1 < x0) {
                    s32 temporary = x1;
                    x1 = x0;
                    x0 = temporary;
                }
                if (y1 < y0) {
                    s32 temporary = y1;
                    y1 = y0;
                    y0 = temporary;
                }
                if (z1 < z0) {
                    s32 temporary = z1;
                    z1 = z0;
                    z0 = temporary;
                }
                if (hitCount < 20) {
                    insertIndex = hitCount;
                    while ((insertIndex > 0) &&
                           (nearClip < entryTimes[insertIndex - 1])) {
                        entryTimes[insertIndex] = entryTimes[insertIndex - 1];
                        segments[insertIndex] = segments[insertIndex - 1];
                        xzMasks[insertIndex] = xzMasks[insertIndex - 1];
                        yMasks[insertIndex] = yMasks[insertIndex - 1];
                        insertIndex--;
                    }
                    entryTimes[insertIndex] = nearClip;
                    segments[insertIndex] =
                        (u8 *) track->segments + (segmentIndex * 0x40);
                    xzMasks[insertIndex] = getXZCompareMask(
                        bounds, x0, z0, x1, z1);
                    yMasks[insertIndex] = getYCompareMask(bounds, y0, y1);
                    hitCount++;
                }
            }
        }
    }
    hit = 0;
    bestDistance = 1.0f;
    pointX = arg1[0];
    pointY = arg1[1];
    pointZ = arg1[2];
    arg3 |= 0x1080;
    bestPlane = NULL;
    bestFlags = 0;
    bestTexture = 0;
    for (segmentIndex = 0; segmentIndex < hitCount; segmentIndex++) {
        segmentBytes = (u8 *) segments[segmentIndex];
        segment = segments[segmentIndex];
        surfaceBase = E129_PTR(segment, 0x1C);
        batch = E129_PTR(segment, 0x0C);
        batchCount = E129_S16(segment, 0x24);
        for (batchIndex = 0; batchIndex < batchCount; batchIndex++) {
            batchRecord = (u8 *) batch + (batchIndex * 0x10);
            firstTriangle = E129_S16(batchRecord, 8);
            lastTriangle = E129_S16(batchRecord, 0x18);
            bestFlags = E129_S32(batchRecord, 0x0C);
            if ((bestFlags & arg3) ||
                ((arg4 != 0) && ((bestFlags & arg4) == 0))) {
                firstTriangle = lastTriangle;
            }
            for (triangleIndex = firstTriangle;
                 triangleIndex < lastTriangle; triangleIndex++) {
                s32 visibility = E129_S32(
                    E129_PTR(segment, 0x10), triangleIndex * 4);
                visibility &= xzMasks[segmentIndex];
                if (((visibility & 0xFFFF) != 0) &&
                    ((visibility & 0xFFFF0000) != 0) &&
                    ((E129_U8(E129_PTR(segment, 0x14), triangleIndex) &
                      yMasks[segmentIndex]) != 0)) {
                    polygon = (u16 *) ((u8 *) E129_PTR(segment, 0x18) +
                                      (triangleIndex * 8));
                    plane = (u8 *) surfaceBase + (E129_U16(polygon, 0) * 0x10);
                    normalX = E129_F32(plane, 0);
                    normalY = E129_F32(plane, 4);
                    normalZ = E129_F32(plane, 8);
                    planeDistance = E129_F32(plane, 0xC);
                    side1 = (((arg1[0] * normalX) +
                              ((arg1[1] * normalY) + (arg1[2] * normalZ))) +
                             planeDistance);
                    if (side1 < 0.0f) {
                        side0 = (((arg0[0] * normalX) +
                                  ((arg0[1] * normalY) +
                                   (arg0[2] * normalZ))) + planeDistance);
                        if (side0 >= 0.0f) {
                            fraction = side0 / (side0 - side1);
                            pointX = (direction.f[0] * fraction) + arg0[0];
                            pointY = (direction.f[1] * fraction) + arg0[1];
                            pointZ = (direction.f[2] * fraction) + arg0[2];
                            inside = 1;
                            for (edgeIndex = 0; edgeIndex < 3; edgeIndex++) {
                                edge = E129_U16(polygon, (edgeIndex + 1) * 2);
                                edgeSign = edge & 0x8000;
                                edgeNumber = edge ^ edgeSign;
                                surfaceBytes = (u8 *) surfaceBase +
                                               (edgeNumber * 0x10);
                                edgeValue =
                                    (E129_F32(surfaceBytes, 0) * pointX) +
                                    (E129_F32(surfaceBytes, 4) * pointY) +
                                    (E129_F32(surfaceBytes, 8) * pointZ) +
                                    E129_F32(surfaceBytes, 0xC);
                                if (edgeSign != 0) {
                                    edgeValue = -edgeValue;
                                }
                                if (edgeValue > 0.0f) {
                                    inside = 0;
                                }
                            }
                            if ((inside != 0) && (fraction < bestDistance)) {
                                bestDistance = fraction;
                                bestPlane = plane;
                                bestFlags = E129_S32(batchRecord, 0x0C);
                                bestTexture = E129_U8(
                                    track->textures,
                                    (E129_U8(batchRecord, 0) * 8) + 7);
                                hit = 1;
                            }
                        }
                    }
                }
            }
            batch = (u8 *) batch + 0x10;
        }
    }
    if (hit != 0) {
        E129_S32(arg2, 0) = 0;
        E129_F32(arg2, 4) = pointX;
        E129_F32(arg2, 8) = pointY;
        E129_F32(arg2, 0xC) = pointZ;
        E129_F32(arg2, 0x10) = E129_F32(bestPlane, 0);
        E129_F32(arg2, 0x14) = E129_F32(bestPlane, 4);
        E129_F32(arg2, 0x18) = E129_F32(bestPlane, 8);
        E129_F32(arg2, 0x1C) = E129_F32(bestPlane, 0xC);
        E129_F32(arg2, 0x20) = sqrtf(
            (direction.f[2] * direction.f[2]) +
            ((direction.f[0] * direction.f[0]) +
             (direction.f[1] * direction.f[1]))) * bestDistance;
        E129_S32(arg2, 0x24) = bestFlags;
        E129_S32(arg2, 0x28) = bestTexture;
    } else {
        E129_F32(arg2, 0x20) = sqrtf(
            (direction.f[2] * direction.f[2]) +
            ((direction.f[0] * direction.f[0]) +
             (direction.f[1] * direction.f[1])));
    }
    return hit;
}
#undef E129_U8
#undef E129_S16
#undef E129_U16
#undef E129_S32
#undef E129_F32
#undef E129_PTR
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8001291C.s")
#endif
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
/* Workbench: structure-mismatch, 98 differing words, first mismatch +0x0. */
/* Candidate shape: 99 instructions/frame -0x98 vs target 96/-0xA0; sqrtf call present. */
/* Remaining structural gap: coordinate-load ordering, FP spill homes, and saved-FP frame. */
#ifdef NON_MATCHING
void func_800133FC(TrackVertex *arg0, TrackVertex *arg1,
                   TrackVertex *arg2, f32 *arg3, f32 *arg4,
                   f32 *arg5, f32 *arg6) {
    s32 sp9C;
    s32 sp98;
    s32 sp94;
    f32 sp54;
    f32 sp50;
    f32 sp4C;
    f32 sp34;
    f32 temp_f0;
    f32 temp_f0_2;
    f32 temp_f14;
    f32 temp_f16;
    f32 temp_f18;
    register f32 temp_f20;
    register f32 temp_f22;
    register f32 temp_f24;
    f32 temp_f2;
    f32 temp_f8;
    f32 var_f12;
    f32 var_f14;
    f32 var_f2;
    s32 temp_t0;
    s32 temp_t1;
    s32 temp_t2;
    s32 temp_t3;
    s32 temp_v0;
    s32 temp_v1;

    temp_v1 = arg0->y;
    temp_t2 = arg1->y;
    temp_t3 = arg1->z;
    temp_t0 = arg0->z;
    temp_f14 = (f32) (temp_t2 - temp_v1);
    temp_f8 = (f32) (arg2->z - temp_t3);
    temp_t1 = arg1->x;
    temp_v0 = arg0->x;
    sp98 = (s32) temp_v1;
    temp_f16 = (f32) (arg2->y - temp_t2);
    sp34 = temp_f8;
    sp94 = (s32) temp_t0;
    temp_f18 = (f32) (temp_t3 - temp_t0);
    sp9C = (s32) temp_v0;
    temp_f2 = (f32) (arg2->x - temp_t1);
    temp_f20 = (temp_f14 * temp_f8) - (temp_f18 * temp_f16);
    sp54 = temp_f20;
    temp_f0 = (f32) (temp_t1 - temp_v0);
    temp_f22 = (temp_f18 * temp_f2) - (temp_f0 * temp_f8);
    sp50 = temp_f22;
    temp_f24 = (temp_f0 * temp_f16) - (temp_f14 * temp_f2);
    sp4C = temp_f24;
    temp_f0_2 = sqrtf((temp_f20 * temp_f20) + (temp_f22 * temp_f22) +
                       (temp_f24 * temp_f24));
    var_f2 = sp54;
    var_f12 = sp50;
    var_f14 = sp4C;
    if (temp_f0_2 > 0.0f) {
        var_f2 = temp_f20 / temp_f0_2;
        var_f12 = temp_f22 / temp_f0_2;
        var_f14 = temp_f24 / temp_f0_2;
    }
    *arg3 = var_f2;
    *arg4 = var_f12;
    *arg5 = var_f14;
    *arg6 = -(((f32) temp_v0 * var_f2) + ((f32) temp_v1 * var_f12) +
              ((f32) temp_t0 * var_f14));
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800133FC.s")
#endif
/*
 * PROVENANCE: Mickey-only reconstruction from the target's collision-query
 * callers, resident track layouts, and the neighboring collision helpers;
 * no published donor body is used here.
 */
#ifdef NON_MATCHING
/* Workbench verdict: structure-mismatch; 315 differing words, first mismatch +0x0. */
/* Target 260 instructions/frame -312; candidate 330 instructions/frame -304. */
/* Remaining gap is collision-query stack/control-flow scheduling and sort-loop expansion; not shape-exact. */
u32 func_8001357C(f32 arg0, f32 arg1, f32 *arg2, s32 arg3, void *arg4) {
    s16 segmentIndices[24];
    s16 *segmentIndexPointer;
    TrackSegment *segment;
    TrackBatch *batch;
    TrackTriangle *triangle;
    TrackVertex *vertex0;
    TrackVertex *vertex1;
    TrackVertex *vertex2;
    TrackVertex *vertex3;
    TrackPlane *plane;
    TrackIntersection *intersection;
    s32 x;
    s32 z;
    s32 segmentCount;
    s32 segmentNumber;
    s32 batchCount;
    s32 batchNumber;
    s32 vertexIndex;
    s32 compareMask;
    u32 batchFlags;
    u32 resultCount;
    u32 visibility;
    f32 height;
    f32 planeX;
    f32 planeY;
    f32 planeZ;
    f32 planeDistance;
    s32 outer;
    s32 inner;
    volatile TrackIntersection *record;
    f32 temporaryHeight;
    s32 temporaryFlags;

    x = (s32) arg0;
    z = (s32) arg1;
    segmentCount = func_8000FCA4(x, z, &segmentIndices[0]);
    resultCount = 0;
    if (arg2 != NULL) {
        *arg2 = -32768.0f;
    }
    segmentNumber = 0;
    if (segmentCount > 0) {
        segmentIndexPointer = &segmentIndices[0];
        do {
            compareMask = getXZCompareMask(
                &D_800792E8->segmentBounds[*segmentIndexPointer], x, z, x,
                z);
            segment = &D_800792E8->segments[*segmentIndexPointer];
            segmentNumber++;
            segmentIndexPointer++;
            batchCount = segment->batchCount;
            batch = segment->batches;
            batchNumber = batchCount;
            if (batchCount != 0) {
                do {
                    batchFlags = batch->flags;
                    if (batchFlags & arg3) {
                        vertexIndex = batch->v0;
                        triangle = (TrackTriangle *)
                            ((u8 *) segment->vertexData + (batch->v0 * 0x10));
                        vertex0 = (TrackVertex *)
                            ((u8 *) segment->lightData + (batch->u0 * 0xA));
                        if (vertexIndex < batch[1].v0) {
                            do {
                                visibility = segment->visibilityMasks[vertexIndex];
                                visibility &= compareMask;
                                if ((visibility >> 16) != 0 &&
                                    (visibility & 0xFFFF) != 0) {
                                    vertex1 = (TrackVertex *)
                                        ((u8 *) vertex0 +
                                         (triangle->vertex0 * 0xA));
                                    vertex2 = (TrackVertex *)
                                        ((u8 *) vertex0 +
                                         (triangle->vertex1 * 0xA));
                                    vertex3 = (TrackVertex *)
                                        ((u8 *) vertex0 +
                                         (triangle->vertex2 * 0xA));
                                    if (mathXZInTri(x, z, vertex1, vertex2,
                                                    vertex3) != 0) {
                                        height = (f32) vertex1->y;
                                        if (vertex1->y != vertex2->y ||
                                            vertex1->y != vertex3->y) {
                                            if (batchFlags & 0x1080) {
                                                func_800133FC(
                                                    vertex1, vertex2, vertex3,
                                                    &planeX, &planeY, &planeZ,
                                                    &planeDistance);
                                                plane = (TrackPlane *) &planeX;
                                            } else {
                                                plane = segment->surfaces +
                                                    (segment->surfaceIndices[
                                                         vertexIndex * 4] *
                                                     1);
                                            }
                                            if (plane->y > 0.0f) {
                                                height = -(((plane->x * arg0) +
                                                             (plane->z * arg1) +
                                                             plane->distance) /
                                                            plane->y);
                                            }
                                        }
                                        if (arg4 != NULL) {
                                            if (resultCount >= 8U) {
                                                resultCount = 7;
                                            }
                                            intersection =
                                                (TrackIntersection *) arg4 +
                                                resultCount;
                                            intersection->height = height;
                                            resultCount++;
                                            intersection->flags = batchFlags;
                                        } else {
                                            *arg2 = height;
                                            return batchFlags;
                                        }
                                    }
                                }
                                vertexIndex++;
                                triangle++;
                            } while (vertexIndex < batch[1].v0);
                        }
                    }
                    batch++;
                    batchNumber--;
                } while (batchNumber != 0);
            }
        } while (segmentNumber != segmentCount);
    }
    if (resultCount >= 2U) {
        outer = (s32) resultCount - 2;
        if (outer != 0) {
            do {
                record = (TrackIntersection *) arg4;
                inner = outer + 1;
                if (inner != 0) {
                    do {
                        if (record->height < (record + 1)->height) {
                            temporaryHeight = record->height;
                            temporaryFlags = record->flags;
                            record->height = (record + 1)->height;
                            (record + 1)->height = temporaryHeight;
                            record->flags = (record + 1)->flags;
                            (record + 1)->flags = temporaryFlags;
                        }
                        record++;
                        inner--;
                    } while (inner != 0);
                }
                outer--;
            } while (outer != 0);
        }
    }
    return resultCount;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8001357C.s")
#endif
#ifdef NON_MATCHING
/* PROVENANCE: JFG's public track.c retains this collision collector as
 * assembly; Mickey's segment, batch, plane and hit-list accesses are used. */
/* Workbench verdict: structure-mismatch, 368 differing words; first mismatch is at +0x0. */
/* Target is 330 instructions/frame -320; candidate is 372 instructions/frame -360. */
/* Remaining gap is structural: collision record layout and loop/control-flow scheduling differ; not permuter-ready. */
s32 func_8001398C(f32 arg0, f32 arg1, s32 arg2, void **arg3) {
    s16 segmentIndices[32];
    s32 segmentCount;
    s32 segmentNumber;
    s32 segmentIndex;
    s32 batchNumber;
    s32 batchCount;
    s32 triangleIndex;
    s32 compareMask;
    s32 textureFlag;
    s32 resultCount;
    s32 orderIndex;
    s32 orderCount;
    s32 changed;
    s32 value;
    s32 i;
    s32 j;
    s16 firstTriangle;
    s16 lastTriangle;
    s16 textureOffset;
    u8 *segmentBytes;
    u8 *batchBytes;
    u8 *triangleBytes;
    u8 *surface;
    u8 *hit;
    void **order;
    void **orderIt;
    f32 height;
    f32 planeX;
    f32 planeY;
    f32 planeZ;
    f32 planeDistance;
    f32 planeHeight;
    TrackVertex *vertex0;
    TrackVertex *vertex1;
    TrackVertex *vertex2;

    segmentCount = func_8000FCA4((s32) arg0, (s32) arg1,
                                 segmentIndices);
    *arg3 = NULL;
    if ((segmentCount == 0) || (segmentCount >= 0x20)) {
        return 0;
    }
    arg2 |= 0x1080;
    resultCount = 0;
    segmentNumber = 0;
    orderCount = 0;
    do {
        segmentIndex = segmentIndices[segmentNumber];
        segmentBytes = (u8 *) D_800792E8->segments + (segmentIndex * 0x40);
        compareMask = getXZCompareMask(
            &D_800792E8->segmentBounds[segmentIndex], (s32) arg0,
            (s32) arg1, (s32) arg0, (s32) arg1);
        batchCount = *(s16 *) (segmentBytes + 0x24);
        batchBytes = *(u8 **) (segmentBytes + 0xC);
        batchNumber = 0;
        if (batchCount > 0) {
            do {
                u32 batchFlags = *(u32 *) (batchBytes + 0xC);
                textureOffset = *(s16 *) (batchBytes + 6);
                firstTriangle = *(s16 *) (batchBytes + 8);
                lastTriangle = *(s16 *) (batchBytes + 0x18);
                if (batchFlags & arg2) {
                    firstTriangle = lastTriangle;
                }
                triangleIndex = firstTriangle;
                textureFlag = (batchFlags & 0x10000) != 0;
                if (textureFlag == 0) {
                    textureFlag = *((u8 *) D_800792E8->textures +
                                    (*(u8 *) batchBytes * 8) + 7);
                }
                if (firstTriangle < lastTriangle) {
                    do {
                        u32 visibility = *(u32 *)
                            (segmentBytes + 0x10 + (triangleIndex * 4));
                        visibility &= compareMask;
                        if ((visibility >> 16) != 0 &&
                            (visibility & 0xFFFF) != 0) {
                            triangleBytes = *(u8 **) (segmentBytes + 4) +
                                            (triangleIndex * 0x10);
                            vertex0 = (TrackVertex *)
                                (*(u8 **) (segmentBytes + 0) +
                                 (*(u8 *) (triangleBytes + 1) +
                                  textureOffset) * 0xA);
                            vertex1 = (TrackVertex *)
                                (*(u8 **) (segmentBytes + 0) +
                                 (*(u8 *) (triangleBytes + 2) +
                                  textureOffset) * 0xA);
                            vertex2 = (TrackVertex *)
                                (*(u8 **) (segmentBytes + 0) +
                                 (*(u8 *) (triangleBytes + 3) +
                                  textureOffset) * 0xA);
                            if (mathXZInTri((s32) arg0, (s32) arg1,
                                            vertex0, vertex1, vertex2) != 0) {
                                height = (f32) vertex0->y;
                                if ((vertex0->y != vertex1->y) ||
                                    (vertex0->y != vertex2->y)) {
                                    if (batchFlags & 0x1080) {
                                        func_800133FC(vertex0, vertex1, vertex2,
                                                      &planeX, &planeY,
                                                      &planeZ, &planeDistance);
                                        surface = (u8 *) &planeX;
                                    } else {
                                        value = *(u16 *)
                                            (segmentBytes + 0x18 +
                                             (triangleIndex * 8));
                                        surface = *(u8 **) (segmentBytes + 0x1C) +
                                                  (value * 0x10);
                                    }
                                    planeHeight = *(f32 *) (surface + 4);
                                    if (planeHeight > 0.0f) {
                                        height = -((( *(f32 *) (surface + 0) * arg0) +
                                                     (*(f32 *) (surface + 8) * arg1) +
                                                     *(f32 *) (surface + 0xC)) /
                                                    planeHeight);
                                    }
                                }
                                if (arg3 != NULL) {
                                    if (resultCount >= 0x14) {
                                        resultCount = 0x13;
                                    }
                                    hit = D_800C9B90 + (resultCount * 0x10);
                                    *(f32 *) (hit + 0) = height;
                                    *(void **) (hit + 4) = surface;
                                    *(s32 *) (hit + 8) = batchFlags;
                                    *(s32 *) (hit + 0xC) = textureFlag;
                                    resultCount++;
                                } else {
                                    return batchFlags;
                                }
                            }
                        }
                        triangleIndex++;
                    } while (triangleIndex < lastTriangle);
                }
                batchNumber++;
                batchBytes += 0x10;
            } while (batchNumber < batchCount);
        }
        segmentNumber++;
    } while (segmentNumber < segmentCount);
    if (resultCount > 0) {
        orderIndex = 0;
        order = D_800C9CD0;
        do {
            order[orderIndex] = D_800C9B90 + (orderIndex * 0x10);
            orderIndex++;
        } while (orderIndex != resultCount);
    }
    orderCount = resultCount - 1;
    do {
        changed = 1;
        if (orderCount > 0) {
            orderIndex = 0;
            orderIt = D_800C9CD0;
            i = 0;
            do {
                void *left = orderIt[0];
                void *right = orderIt[1];
                if (*(f32 *) left < *(f32 *) right) {
                    orderIt[0] = right;
                    orderIt[1] = left;
                    changed = 0;
                }
                orderIndex++;
                orderIt++;
                i++;
            } while (i != orderCount);
        }
        orderCount--;
    } while (changed == 0);
    *arg3 = D_800C9CD0;
    return resultCount;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8001398C.s")
#endif
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
#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c display-list draft and resident shadow-buffer and
 * command offsets reconstruct this renderer; no external function body is adapted.
 */
typedef struct TrackShadowObject {
    u8 pad00[0x39];
    u8 alpha;
    u8 pad3A[0x0A];
    s16 kind;
    u8 pad46[0x1E];
    void *material;
} TrackShadowObject;

typedef struct TrackShadowInstance {
    u8 pad00[0x0C];
    u16 textureScale;
    u8 pad0E[2];
    u8 active;
    u8 pad11[2];
    u8 count;
    s16 shadowIndex;
    u8 pad16[2];
    s16 endIndex;
} TrackShadowInstance;

typedef struct TrackShadowBuffer {
    void *texture;
    s16 u0;
    s16 height;
    u8 pad08[4];
    s16 v0;
    s16 length;
} TrackShadowBuffer;

typedef struct TrackShadowMaterial {
    u8 pad00[0x18];
    u8 red;
    u8 green;
    u8 blue;
} TrackShadowMaterial;

/* Workbench verdict: structure-mismatch, 187 differing words, first mismatch +0x0. */
/* Candidate: 217/217 instructions with a -0x90 frame versus target -0xA8; 3/4 relocation placements align. */
/* Shape status: alpha branches, 8-byte shadow stepping, geometry commands, and FA/FB cleanup writes are preserved, but it is not shape-exact. */
void func_800140CC(TrackShadowObject *object, TrackShadowInstance *instance) {
    s32 loopIndex;
    s32 closeTexture;
    s32 closeCombiner;
    s32 commandBuffer;
    s32 indexBuffer;
    s32 vertexBuffer;
    s32 shadowCount;
    s32 alphaValue;
    s32 commandMode;
    s32 textureSpan;
    s32 indexSpan;
    s32 vertexAddress;
    s32 indexAddress;
    s16 shadowIndex;
    TrackShadowInstance *current;
    u8 *shadow;
    u8 active;
    TrackShadowMaterial *material;
    Gfx *command;

    active = instance->active;
    if (active != 0) {
        shadowGetBuffers(active, &vertexBuffer, &indexBuffer,
                         &commandBuffer);
        loopIndex = 0;
        current = instance;
        if (current->count > 0) {
            do {
                shadowIndex = current->shadowIndex;
                if (shadowIndex != -1) {
                    shadow = (u8 *) (commandBuffer + (shadowIndex * 8));
                    shadowCount = (s32) object->alpha *
                                  *(u8 *) (vertexBuffer +
                                  (*(s16 *) (shadow + 6) * 0x0A) + 9);
                    shadowCount >>= 8;
                    if (shadowCount > 0) {
                        commandMode = 0x0E;
                        if (object->kind == 0x3C) {
                            command = D_800C9520;
                            material = object->material;
                            D_800C9520 = command + 1;
                            command->words.w1 = (shadowCount & 0xFF) | ~0xFF;
                            command->words.w0 = 0xFA000000;
                            command = D_800C9520;
                            commandMode = 0x20E;
                            D_800C9520 = command + 1;
                            command->words.w0 = 0xFB000000;
                            command->words.w1 = (material->red << 24) |
                                                (material->green << 16) |
                                                (material->blue << 8);
                            closeTexture = 1;
                            closeCombiner = 1;
                        } else {
                            closeCombiner = 0;
                            if ((object->kind == 0x35) ||
                                (object->kind == 0x58)) {
                                command = D_800C9520;
                                D_800C9520 = command + 1;
                                command->words.w0 = 0xFA000000;
                                command->words.w1 = (shadowCount & 0xFF) | ~0xFF;
                                closeTexture = shadowCount != 0xFF;
                            } else {
                                command = D_800C9520;
                                D_800C9520 = command + 1;
                                command->words.w1 = shadowCount & 0xFF;
                                command->words.w0 = 0xFA000000;
                                closeTexture = 1;
                            }
                        }
                        shadowIndex = current->shadowIndex;
                        while (shadowIndex < current->endIndex) {
                            func_800349A4(&D_800C9520, *(void **) shadow,
                                          commandMode,
                                          instance->textureScale << 8);
                            command = D_800C9520;
                            D_800C9520 = command + 1;
                            textureSpan = *(s16 *) (shadow + 0xE) -
                                          *(s16 *) (shadow + 6);
                            vertexAddress = vertexBuffer +
                                            (*(s16 *) (shadow + 6) * 10) +
                                            (s32) 0x80000000;
                            command->words.w0 = (((((textureSpan * 8) |
                                                   (vertexAddress & 6)) & 0xFF) << 16) |
                                                 0x04000000 |
                                                 ((textureSpan * 10 + 8) & 0xFFFF));
                            command->words.w1 = vertexAddress;
                            command = D_800C9520;
                            D_800C9520 = command + 1;
                            indexSpan = *(s16 *) (shadow + 0xC) -
                                        *(s16 *) (shadow + 4);
                            indexAddress = (*(s16 *) (shadow + 4) * 16) +
                                           indexBuffer + (s32) 0x80000000;
                            command->words.w1 = indexAddress;
                            command->words.w0 = ((((((indexSpan - 1) * 16) |
                                                   1) & 0xFF) << 16) |
                                                 0x05000000 |
                                                 ((indexSpan * 16) & 0xFFFF));
                            shadowIndex++;
                            shadow += 8;
                        }
                        if (closeTexture != 0) {
                            command = D_800C9520;
                            D_800C9520 = command + 1;
                            command->words.w1 = -1;
                            command->words.w0 = 0xFA000000;
                        }
                        if (closeCombiner != 0) {
                            command = D_800C9520;
                            D_800C9520 = command + 1;
                            command->words.w1 = -0x100;
                            command->words.w0 = 0xFB000000;
                        }
                    }
                }
                current = (TrackShadowInstance *) ((u8 *) current + 2);
                loopIndex++;
            } while (loopIndex < instance->count);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800140CC.s")
#endif
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
                /* Inert allocation aid retained by exact C; tracked in
                 * docs/cleanup-queue.md. */
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




/* PLATEAU-HANDOFF:func_800140CC:start
 * symbol: func_800140CC
 * score: 187 differing words
 * frame: 0x90
 * relocations: 4
 * first-mismatch: +0x0
 * summary: Frame and local layout remain the primary blocker; retry explicit value lifetimes before allocator work
 * PLATEAU-HANDOFF:func_800140CC:end
 */
