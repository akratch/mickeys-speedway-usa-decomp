#include "PR/ultratypes.h"

typedef struct Gfx {
    struct {
        u32 w0;
        u32 w1;
    } words;
} Gfx;

#define SHIFTL(v, s, w) ((u32)(((u32)(v) & ((0x01 << (w)) - 1)) << (s)))
#define MIN2(a, b) (((a) < (b)) ? (a) : (b))
#define MAX2(a, b) (((a) > (b)) ? (a) : (b))
#define TXL2WORDS(width) MAX2(1U, (((u32)(width) * 2U) / 8U))
#define CALC_DXT(width) ((2048U + TXL2WORDS(width) - 1U) / TXL2WORDS(width))

/* Reconstructed historical-style display-list macros.  Keeping each macro's
 * block-local _g is intentional: that source topology drives the target's
 * long-lived constant and cursor allocation. */
#define CMD2(pkt, a, b) { Gfx *_g = (Gfx *)(pkt); _g->words.w0 = (u32)(a); _g->words.w1 = (u32)(b); }
#define CLEAR_GEOM(pkt, mask) CMD2(pkt, 0xB6000000U, (mask))
#define SET_GEOM(pkt, mask) CMD2(pkt, 0xB7000000U, (mask))
#define PIPE_SYNC(pkt) CMD2(pkt, 0xE7000000U, 0)
#define FULL_SYNC(pkt) CMD2(pkt, 0xE9000000U, 0)
#define END_DL(pkt) CMD2(pkt, 0xB8000000U, 0)
#define SET_LIGHTS_ZERO(pkt) CMD2(pkt, 0xBC000002U, 0)
#define SET_BLEND(pkt, a) CMD2(pkt, 0xF9000000U, (u32)(a))
#define SET_PRIM(pkt) CMD2(pkt, 0xFA000000U, 0xFFFFFFFFU)
#define SET_ENV(pkt, a) CMD2(pkt, SHIFTL(0xFBU, 24, 8), \
    SHIFTL(0xFFU, 24, 8) | SHIFTL(0xFFU, 16, 8) | \
    SHIFTL(0xFFU, 8, 8) | SHIFTL((a), 0, 8))
#define SET_COMBINE(pkt) CMD2(pkt, 0xFCFFFFFFU, 0xFFFCF67DU)
#define SET_RENDER(pkt) CMD2(pkt, 0xEF000CCFU, 0x00504340U)
#define IMMP1(pkt, cmd, p0) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = SHIFTL((cmd), 24, 8); \
    _g->words.w1 = (u32)(p0); \
}

#define SET_COLOR_IMAGE(pkt, width, image) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = 0xFF100000U | (((u32)(width) - 1U) & 0xFFFU); \
    _g->words.w1 = (u32)(image); \
}

#define SET_SCISSOR(pkt, width, height) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = 0xED000000U; \
    _g->words.w1 = (((s32)((f32)(u32)(width) * 4.0F) & 0xFFF) << 12) | \
                    ((s32)((f32)(u32)(height) * 4.0F) & 0xFFF); \
}

#define SET_TEXTURE_IMAGE(pkt, image) CMD2(pkt, 0xFD100000U, (image))
#define SET_LOAD_TILE(pkt) CMD2(pkt, 0xF5100000U, 0x07080200U)
#define LOAD_SYNC(pkt) CMD2(pkt, 0xE6000000U, 0)
#define SET_RENDER_TILE(pkt, width) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = 0xF5100000U | ((((((u32)(width) << 1) + 7U) >> 3) & 0x1FFU) << 9); \
    _g->words.w1 = 0x00080200U; \
}
#define SET_TILE_SIZE(pkt, width, height) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = 0xF2000000U; \
    _g->words.w1 = (((((u32)(width) - 1U) << 2) & 0xFFFU) << 12) | \
                   ((((u32)(height) - 1U) << 2) & 0xFFFU); \
}
#define LOAD_BLOCK(pkt, width, height) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = 0xF3000000U; \
    _g->words.w1 = 0x07000000U | \
                   ((MIN2(((u32)(height) * (u32)(width)) - 1U, 0x7FFU) & 0xFFFU) << 12) | \
                   (CALC_DXT(width) & 0xFFFU); \
}

#define LOAD_TEXTURE_BLOCK(pkt, source, width, height) { \
    SET_TEXTURE_IMAGE((pkt), (source)); \
    SET_LOAD_TILE(pkt); \
    LOAD_SYNC(pkt); \
    LOAD_BLOCK(pkt, (width), (height)); \
    PIPE_SYNC(pkt); \
    SET_RENDER_TILE(pkt, (width)); \
    SET_TILE_SIZE(pkt, (width), (height)); \
}

#define TEXTURE_RECTANGLE(pkt, width, row, nextRow) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = 0xE4000000U | ((((u32)(width) << 2) & 0xFFFU) << 12) | \
                   (((u32)(nextRow) << 2) & 0xFFFU); \
    _g->words.w1 = ((u32)(row) << 2) & 0xFFFU; \
    IMMP1(pkt, 0xB3, SHIFTL(0, 16, 16) | SHIFTL(0, 0, 16)); \
    IMMP1(pkt, 0xB2, SHIFTL((1 << 10), 16, 16) | \
                         SHIFTL((1 << 10), 0, 16)); \
}

extern s32 overlay42ReadinessReloc();
extern s32 overlay42GetSizeReloc();
extern s32 overlay42RspSegmentReloc();
extern s32 overlay42SetupDisplayAReloc();
extern s32 overlay42SetupDisplayBReloc();
#define ready_call overlay42ReadinessReloc
#define size_call overlay42GetSizeReloc
#define segment_call overlay42RspSegmentReloc
#define setup_a_call overlay42SetupDisplayAReloc
#define setup_b_call overlay42SetupDisplayBReloc

extern Gfx *gOverlay42Buffers[2];
extern u8 gO42BufferIndexBaseReloc[];
extern Gfx *gO42PublishedStartReloc;
extern void *gO42Segment1BaseReloc;
extern u8 gO42PublishedSourceBaseReloc[];
extern void *gO42Segment2BaseReloc;
extern u16 *gO42TextureSourceReloc;
extern u8 gO42ImageStateReloc[];
extern u8 gO42PublishedEndBaseReloc[];

#define gOverlay42BufferIndex (*(s32 *)(gO42BufferIndexBaseReloc + 8))
#define gO42PublishedSourceReloc (*(void **)(gO42PublishedSourceBaseReloc + 8))
#define gO42ImageAlphaReloc (gO42ImageStateReloc[12])
#define gO42PublishedEndReloc (*(Gfx **)(gO42PublishedEndBaseReloc + 4))

void overlay42DrawCapturedBuffer(s32 callbackArgument) {
    s32 row;
    u16 *source;
    u32 width;
    u32 height;
    s32 rows;
    Gfx *displayList;

    gO42PublishedStartReloc = 0;
    if (ready_call() == 0) {
        return;
    }

    displayList = gOverlay42Buffers[gOverlay42BufferIndex];
    size_call(&width, &height);
    gO42PublishedStartReloc = displayList;
    gO42PublishedSourceReloc = gO42Segment1BaseReloc;

    segment_call(&displayList, 0, 0);
    segment_call(&displayList, 1, gO42Segment1BaseReloc);
    segment_call(&displayList, 2, gO42Segment2BaseReloc);
    setup_a_call(&displayList);
    setup_b_call(&displayList);

    source = (u16 *)displayList++;
    CLEAR_GEOM((Gfx *)source, 0x00010001U);
    SET_COLOR_IMAGE(displayList++, width,
                    (u32)gO42Segment1BaseReloc + 0x80000000U);
    SET_SCISSOR(displayList++, width, height);
    SET_LIGHTS_ZERO(displayList++);
    PIPE_SYNC(displayList++);
    CLEAR_GEOM(displayList++, 0x00001000U);
    SET_BLEND(displayList++, 0x64U);
    SET_PRIM(displayList++);
    SET_ENV(displayList++, gO42ImageAlphaReloc);
    PIPE_SYNC(displayList++);
    CLEAR_GEOM(displayList++, 0x00010001U);
    SET_GEOM(displayList++, 0x00000204U);
    SET_COMBINE(displayList++);
    SET_RENDER(displayList++);

    source = gO42TextureSourceReloc;
    row = 0;
    while ((u32)row < (u32)height) {
        rows = height - row;
        if (rows >= 5) {
            rows = 4;
        }
        LOAD_TEXTURE_BLOCK(displayList++, source, width, rows);
        TEXTURE_RECTANGLE(displayList++, width, row, row + rows);
        row += rows;
        source += width * (u32)rows;
    }

    FULL_SYNC(displayList++);
    END_DL(displayList++);
    gO42PublishedEndReloc = displayList;
}
