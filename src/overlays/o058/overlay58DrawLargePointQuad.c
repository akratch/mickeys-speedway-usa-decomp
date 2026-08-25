#include "PR/ultratypes.h"
#include "overlays/overlay058.h"

typedef struct Overlay58LargePointVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Overlay58LargePointVertex;

typedef struct Overlay58LargePointGfx {
    u32 w0;
    u32 w1;
} Overlay58LargePointGfx;

extern Overlay58LargePointGfx *gOverlay58LargePointDisplayListReloc;
extern Overlay58LargePointVertex *gOverlay58LargePointVertexCursorReloc;
extern u8 gOverlay58LargePointRenderStateReloc[];
extern u8 gOverlay58LargePointPayload98Reloc[];
extern void func_overlay_058_F0000000_18AF1E8(
    Overlay58LargePointGfx **displayList, void *resource, s32 mode, s32 arg3);

/*
 * Plateau (2026-08-25): the best -O2 -mips2 candidate has the exact
 * 104-instruction shape and frame, but 70 register-allocation words differ;
 * the first instruction mismatch is +0x30.  Flag sweeping and a bounded
 * permuter run did not produce a supported allocator-pool correction.
 */
#ifdef NON_MATCHING
void overlay58DrawLargePointQuad(s32 x, s32 y, s32 z) {
    Overlay58LargePointGfx *gfx;
    Overlay58LargePointVertex *vertices;
    u32 physicalVertices;
    s32 physicalBase;
    s32 xPlus;
    s32 xMinus;
    s32 zMinus;
    s32 zPlus;

    func_overlay_058_F0000000_18AF1E8(&gOverlay58LargePointDisplayListReloc,
                                      *(void **)&gOverlay58LargePointRenderStateReloc[0x1E8],
                                      5, 0);

    gfx = gOverlay58LargePointDisplayListReloc++;
    vertices = gOverlay58LargePointVertexCursorReloc;
    physicalBase = 0x80000000U;
    physicalVertices = (u32)vertices + physicalBase;

    gfx->w0 = 0x04000000U |
              ((((u8)((physicalVertices & 6U) | 0x20U)) & 0xFFU) << 16) |
              0x30U;
    gfx->w1 = (u32)gOverlay58LargePointVertexCursorReloc + physicalBase;

    gfx = gOverlay58LargePointDisplayListReloc++;
    gfx->w0 = 0x05110020U;
    gfx->w1 = (u32)gOverlay58LargePointPayload98Reloc;

    vertices = gOverlay58LargePointVertexCursorReloc;
    vertices[1].r = 0xFF;
    vertices[1].g = 0xFF;
    vertices[1].b = 0xFF;
    vertices[1].a = 0xFF;
    vertices[2].r = 0xFF;
    vertices[2].g = 0xFF;
    vertices[2].b = 0xFF;
    vertices[2].a = 0xFF;
    vertices[3].r = 0xFF;
    vertices[3].g = 0xFF;
    vertices[3].b = 0xFF;
    vertices[3].a = 0xFF;
    vertices += 3;
    vertices[-3].r = 0xFF;
    vertices[-3].g = 0xFF;
    vertices[-3].b = 0xFF;
    vertices[-3].a = 0xFF;

    xPlus = x + 18;
    xMinus = x - 18;
    zMinus = z - 18;
    zPlus = z + 18;

    gOverlay58LargePointVertexCursorReloc->x = (s16)xMinus;
    gOverlay58LargePointVertexCursorReloc->y = (s16)y;
    gOverlay58LargePointVertexCursorReloc->z = (s16)zMinus;
    gOverlay58LargePointVertexCursorReloc++;

    gOverlay58LargePointVertexCursorReloc->x = (s16)xPlus;
    gOverlay58LargePointVertexCursorReloc->y = (s16)y;
    gOverlay58LargePointVertexCursorReloc->z = (s16)zMinus;
    gOverlay58LargePointVertexCursorReloc++;

    gOverlay58LargePointVertexCursorReloc->x = (s16)xMinus;
    gOverlay58LargePointVertexCursorReloc->y = (s16)y;
    gOverlay58LargePointVertexCursorReloc->z = (s16)zPlus;
    gOverlay58LargePointVertexCursorReloc++;

    gOverlay58LargePointVertexCursorReloc->x = (s16)xPlus;
    gOverlay58LargePointVertexCursorReloc->y = (s16)y;
    gOverlay58LargePointVertexCursorReloc->z = (s16)zPlus;
    gOverlay58LargePointVertexCursorReloc++;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o058/overlay58DrawLargePointQuad/func_overlay_058_F00050C8_18B42B0.s")
#endif
