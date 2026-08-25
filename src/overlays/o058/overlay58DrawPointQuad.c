#include "PR/ultratypes.h"
#include "overlays/overlay058.h"

typedef struct Overlay58PointVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Overlay58PointVertex;

typedef struct Overlay58PointGfx {
    u32 w0;
    u32 w1;
} Overlay58PointGfx;

extern Overlay58PointGfx *gOverlay58PointDisplayListReloc;
extern Overlay58PointVertex *gOverlay58PointVertexCursorReloc;
extern u8 gOverlay58PointRenderStateReloc[];
extern u8 D_80000078[];
extern void func_overlay_058_F0000000_18AF1E8(
    Overlay58PointGfx **displayList, void *resource, s32 mode, s32 arg3);

/*
 * Plateau (2026-08-25): a fresh 119-combination lattice retains the exact
 * 104-instruction CFG, opcode schedule, size, and frame under -O2 -mips2,
 * but 70 register-allocation words differ and the first mismatch is +0x30.
 * The first pool divergence colors the long-lived vertex-cursor address in
 * v0 instead of a0.  Declaration/storage ordering, signed physical-address
 * types, an explicit cursor reference, a named color lifetime, and split
 * post-increments were either allocation-neutral or changed the instruction
 * shape.  Binding the payload to D_80000078 closes one relocation identity,
 * but no tested structural spelling moved the web without adding code.
 */
#ifdef NON_MATCHING
void overlay58DrawPointQuad(s32 x, s32 y, s32 z) {
    Overlay58PointGfx *gfx;
    Overlay58PointVertex *vertices;
    u32 physicalVertices;
    s32 physicalBase;
    s32 xPlus;
    s32 xMinus;
    s32 zMinus;
    s32 zPlus;

    func_overlay_058_F0000000_18AF1E8(&gOverlay58PointDisplayListReloc,
                                      *(void **)&gOverlay58PointRenderStateReloc[0x44],
                                      5, 0);

    gfx = gOverlay58PointDisplayListReloc++;
    vertices = gOverlay58PointVertexCursorReloc;
    physicalBase = 0x80000000U;
    physicalVertices = (u32)vertices + physicalBase;

    gfx->w0 = 0x04000000U |
              ((((u8)((physicalVertices & 6U) | 0x20U)) & 0xFFU) << 16) |
              0x30U;
    gfx->w1 = (u32)gOverlay58PointVertexCursorReloc + physicalBase;

    gfx = gOverlay58PointDisplayListReloc++;
    gfx->w0 = 0x05110020U;
    gfx->w1 = (u32)D_80000078;

    vertices = gOverlay58PointVertexCursorReloc;
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

    xPlus = x + 8;
    xMinus = x - 8;
    zMinus = z - 8;
    zPlus = z + 8;

    gOverlay58PointVertexCursorReloc->x = (s16)xMinus;
    gOverlay58PointVertexCursorReloc->y = (s16)y;
    gOverlay58PointVertexCursorReloc->z = (s16)zMinus;
    gOverlay58PointVertexCursorReloc++;

    gOverlay58PointVertexCursorReloc->x = (s16)xPlus;
    gOverlay58PointVertexCursorReloc->y = (s16)y;
    gOverlay58PointVertexCursorReloc->z = (s16)zMinus;
    gOverlay58PointVertexCursorReloc++;

    gOverlay58PointVertexCursorReloc->x = (s16)xMinus;
    gOverlay58PointVertexCursorReloc->y = (s16)y;
    gOverlay58PointVertexCursorReloc->z = (s16)zPlus;
    gOverlay58PointVertexCursorReloc++;

    gOverlay58PointVertexCursorReloc->x = (s16)xPlus;
    gOverlay58PointVertexCursorReloc->y = (s16)y;
    gOverlay58PointVertexCursorReloc->z = (s16)zPlus;
    gOverlay58PointVertexCursorReloc++;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o058/overlay58DrawPointQuad/func_overlay_058_F0004F28_18B4110.s")
#endif
