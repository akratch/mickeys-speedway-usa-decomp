#include "PR/ultratypes.h"
#include "overlay33GbiSubset.inc"

typedef struct Overlay33Locals {
    u8 pad88[2];
    u8 alpha;
    u8 blue;
    u8 green;
    u8 red;
    s16 unused;
    s16 high;
    s16 low;
    u32 height;
    u32 width;
    u32 pad9C;
    Gfx *other;
    Gfx *cursor;
} Overlay33Locals;

extern s32 gOverlay33BufferIndex;
extern u8 D_8[];
extern u8 D_1808[];
extern void *gOverlay33DisplayLists[];
extern void *gOverlay33Texture;

extern void overlay33GetDimensionsReloc(u32 *width, u32 *height);
extern void overlay33SetupPassReloc(Gfx **cursor, s32 pass, void *value);
extern void overlay33SetupGeometryReloc(Gfx **cursor);
extern void overlay33SetupViewportReloc(Gfx **cursor);
extern void overlay33ApplyMatrixReloc(Gfx **cursor);
extern void overlay33PrepareReloc(void *value);
extern void overlay33CopyPassReloc(Gfx **cursor, Gfx **other);
extern void overlay33GetColorReloc(void *value, s16 *low, s16 *high,
                                   s16 *unused, u8 *red, u8 *green,
                                   u8 *blue, u8 *alpha);
extern void overlay33SetRangeReloc(void *value, s32 range);
extern void overlay33FinishReloc(s32 value);

/* Mickey-local reconstruction using the reviewed SDK macro subset below. */
void overlay33BuildDisplayList(void) {
    Overlay33Locals locals;
    s32 range;
    s32 index;

    overlay33GetDimensionsReloc(&locals.width, &locals.height);
    index = gOverlay33BufferIndex;
    locals.cursor = (Gfx *)&D_8[index * 0xC00];
    locals.other = (Gfx *)&D_1808[index * 0xC00];

    overlay33SetupPassReloc(&locals.cursor, 0, 0);
    overlay33SetupPassReloc(&locals.cursor, 1,
                            gOverlay33DisplayLists[gOverlay33BufferIndex]);
    overlay33SetupPassReloc(&locals.cursor, 2, gOverlay33Texture);
    overlay33SetupGeometryReloc(&locals.cursor);
    overlay33SetupViewportReloc(&locals.cursor);

    gSPClearGeometryMode(locals.cursor++, 1);
    gDPSetColorImage(locals.cursor++, G_IM_FMT_RGBA, G_IM_SIZ_16b,
                     locals.width,
                     (u32)gOverlay33DisplayLists[gOverlay33BufferIndex] +
                         0x80000000U);
    gDPSetScissor(locals.cursor++, 0, 0, 0, locals.width, locals.height);
    gMoveWd(locals.cursor++, 2, 0, 0);
    gDPSetFillColor(locals.cursor++, 0);
    gDPSetCycleType(locals.cursor++, G_CYC_FILL);
    gDPFillRectangle(locals.cursor++, 0, 0, locals.width - 1,
                     locals.height - 1);
    gDPPipeSync(locals.cursor++);

    overlay33ApplyMatrixReloc(&locals.cursor);
    gMoveWd(locals.cursor++, 2, 0, 0);
    gSPClearGeometryMode(locals.cursor++, 0x1000);
    gDPSetBlendColor(locals.cursor++, 0, 0, 0, 0x64);
    gDPSetPrimColor(locals.cursor++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    gDPSetEnvColor(locals.cursor++, 0xFF, 0xFF, 0xFF, 0);
    gDPPipeSync(locals.cursor++);

    overlay33PrepareReloc(0);
    overlay33CopyPassReloc(&locals.cursor, &locals.other);
    gDPPipeSync(locals.cursor++);

    overlay33GetColorReloc(0, &locals.low, &locals.high, &locals.unused,
                           &locals.red, &locals.green, &locals.blue,
                           &locals.alpha);
    gDPSetFogColor(locals.cursor++, locals.red, locals.green, locals.blue,
                   0xFF);

    {
        Gfx *fog = locals.cursor++;
        fog->words.w0 = 0xBC000008;
        range = locals.high - locals.low;
        fog->words.w1 = ((128000 / range) << 16) |
                        ((((500 - locals.low) * 256) / range) & 0xFFFF);
    }
    overlay33SetRangeReloc(0, range);
    gMoveWd(locals.cursor++, 10, 0, 0);
    gDPFullSync(locals.cursor++);
    gSPEndDisplayList(locals.cursor++);

    gOverlay33DisplayLists[gOverlay33BufferIndex] = locals.cursor;
    overlay33FinishReloc(1);
}
