#include "PR/ultratypes.h"

typedef struct Overlay31EffectRecord {
    u8 pad00[0x24];
    u8 active;
    u8 pad25[3];
} Overlay31EffectRecord;

typedef struct Overlay31Vertex {
    u8 bytes[10];
} Overlay31Vertex;

extern void overlay31ResetReloc(void);
extern void *overlay31AllocateReloc();
extern void *overlay31CreateConfig(s32 kind, void *source, s32 width,
                                   s32 height, s32 count);
extern void *overlay31CreateRecords(s32 count);
extern void *overlay31CreatePool(s32 count);
extern void overlay31ResetLocal(void);
extern void *overlay31LoadAsset(s32 assetId);
extern void *overlay31LoadType8000();
extern void *overlay31LoadTypeC000();
extern void *overlay31LoadType0000();
extern void overlay31FreeAsset(void *asset);

extern s32 gOverlay31MaxLine;
extern s32 gOverlay31MaxPoint;
extern void *gOverlay31VertexBuffers[2];
extern u8 gOverlay31TriangleSourceA[];
extern u8 gOverlay31TriangleSourceB[];
extern u8 gOverlay31RectangleSourceA[];
extern u8 gOverlay31RectangleSourceB[];
extern void *gOverlay31Configs[4];
extern void *gOverlay31PointPool;
extern void *gOverlay31LineRecords;
extern void **gOverlay31DummyAssets;
extern s32 gOverlay31DummyCount;
extern Overlay31EffectRecord *gOverlay31EffectRecords;
extern s32 gOverlay31EffectCount;

/* PROVENANCE: Diddy Kong Racing, src/particles.c
 * (init_particle_buffers); semantic source-shape analogue only. */
#ifdef NON_MATCHING
void overlay31InitializeBuffers(s32 maxTriangle, s32 maxRectangle,
                                s32 maxSprite, s32 maxLine, s32 maxPoint,
                                s32 maxExtra, s32 maxEffects) {
    s16 *assetBuffer;
    s32 i;
    s32 vertexCount;
    s32 temp_var;

    overlay31ResetReloc();

    if (maxTriangle < 0) {
        maxTriangle = 0x10;
    }
    if (maxRectangle < 0) {
        maxRectangle = 0x10;
    }
    if (maxSprite < 0) {
        maxSprite = 0xD0;
    }
    if (maxExtra < 0) {
        maxExtra = 0xF;
    }
    if (maxLine < 0) {
        maxLine = 0xA0;
    }
    if (maxPoint < 0) {
        maxPoint = 0x40;
    }
    if (maxEffects < 0) {
        maxEffects = 0x20;
    }

    vertexCount = maxTriangle * 3 + maxRectangle * 4 + maxSprite +
                  maxLine * 0x12 + maxPoint * 0x78;
    gOverlay31MaxLine = maxLine;
    gOverlay31MaxPoint = maxPoint;
    gOverlay31VertexBuffers[0] =
        overlay31AllocateReloc(vertexCount * 2 * sizeof(Overlay31Vertex), 0x8C);
    gOverlay31VertexBuffers[1] =
        (u8 *)gOverlay31VertexBuffers[0] + vertexCount * 10;

    gOverlay31Configs[0] = overlay31CreateConfig(
        (s32)gOverlay31TriangleSourceA, gOverlay31TriangleSourceB,
        3, 1, maxTriangle);
    gOverlay31Configs[1] = overlay31CreateConfig(
        (s32)gOverlay31RectangleSourceA, gOverlay31RectangleSourceB,
        4, 2, maxRectangle);
    gOverlay31Configs[2] = overlay31CreateConfig(0, 0, 0, 0, maxSprite);
    gOverlay31Configs[3] = overlay31CreateConfig(0, 0, 0, 0, maxExtra);
    gOverlay31PointPool = overlay31CreatePool(maxPoint);
    gOverlay31LineRecords = overlay31CreateRecords(gOverlay31MaxLine);
    overlay31ResetLocal();

    if (gOverlay31DummyAssets == 0) {
        assetBuffer = overlay31LoadAsset(0x3A);
        gOverlay31DummyCount = 0;
        while (assetBuffer[gOverlay31DummyCount] != -1) {
            gOverlay31DummyCount++;
        }

        gOverlay31DummyAssets =
            overlay31AllocateReloc(gOverlay31DummyCount * 4, 0x8C);
        for (i = 0; i < gOverlay31DummyCount; i++) {
            switch (assetBuffer[i] & 0xC000) {
                case 0x8000:
                    gOverlay31DummyAssets[i] =
                        overlay31LoadType8000(assetBuffer[i] & 0x3FFF, 1);
                    break;
                case 0xC000:
                    gOverlay31DummyAssets[i] =
                        overlay31LoadTypeC000(assetBuffer[i] & 0x3FFF);
                    break;
                case 0:
                    gOverlay31DummyAssets[i] =
                        overlay31LoadType0000(assetBuffer[i], 0);
                    break;
            }
        }
        overlay31FreeAsset(assetBuffer);
    }

    gOverlay31EffectRecords =
        overlay31AllocateReloc(maxEffects * sizeof(Overlay31EffectRecord), 0x8C);
    for (i = 0; i < maxEffects; i++) {
        gOverlay31EffectRecords[i].active = 0;
    }
    gOverlay31EffectCount = maxEffects;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o031/overlay31InitializeBuffers/func_overlay_031_F00006B0_187FBD0.s")
#endif
