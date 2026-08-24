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
extern u8 gOverlay58PointPayload78Reloc[];
extern void func_overlay_058_F0000000_18AF1E8(
    Overlay58PointGfx **displayList, void *resource, s32 mode, s32 arg3);

void overlay58DrawPointQuad(s32 x, s32 y, s32 z) {
    Overlay58PointVertex *vertices;
    Overlay58PointGfx *gfx;
    void *resource;
    u32 physicalVertices;
    s32 physicalBase;
    s32 xPlus;
    s32 xMinus;
    s32 zMinus;
    s32 zPlus;

    resource = *(void **)&gOverlay58PointRenderStateReloc[0x44];
    func_overlay_058_F0000000_18AF1E8(&gOverlay58PointDisplayListReloc,
                                      resource, 5, 0);

    vertices = gOverlay58PointVertexCursorReloc;
    gfx = gOverlay58PointDisplayListReloc++;
    physicalBase = 0x80000000U;
    physicalVertices = (u32)vertices + physicalBase;

    gfx->w0 = 0x04000000U |
              ((((physicalVertices & 6U) | 0x20U) & 0xFFU) << 16) | 0x30U;
    gfx->w1 = (u32)gOverlay58PointVertexCursorReloc + physicalBase;

    gfx = gOverlay58PointDisplayListReloc++;
    gfx->w1 = (u32)gOverlay58PointPayload78Reloc;
    gfx->w0 = 0x05110020U;

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
