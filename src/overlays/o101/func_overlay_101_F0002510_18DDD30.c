#include "PR/ultratypes.h"

typedef struct Overlay101Gfx {
    u32 w0;
    u32 w1;
} Overlay101Gfx;

typedef struct Overlay101ClipNode {
    u8 pad00[8];
    u8 type;
    u8 pad09[5];
    s16 x;
    s16 y;
    u8 pad12[4];
    u8 intensity;
    u8 alpha;
} Overlay101ClipNode;

typedef struct Overlay101Texture {
    s16 width;
    s16 height;
    u8 pad04[0xC];
    u16 pixels[1];
} Overlay101Texture;

typedef struct Overlay101TextureElement {
    u8 pad00[8];
    s16 x;
    s16 y;
    u8 pad0C[4];
    Overlay101Texture *texture;
} Overlay101TextureElement;

extern u32 D_230[];
void func_overlay_101_F0000000_18DB820();

/* PLATEAU (2026-08-25): -O2 -mips2 is 280/293 words; 276 differ, first +0x0.
 * Inverted guards plus volatile stride reproduce the s0-s7 save set, but the frame is 0xD0 vs 0xE8.
 * Flag lattice, command macros, cursor volatility, and register declarations did not close it; no donor used. */
#ifdef NON_MATCHING
void func_overlay_101_F0002510_18DDD30(Overlay101Gfx **displayList,
                                      Overlay101ClipNode *node,
                                      Overlay101TextureElement *element) {
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 x;
    s32 y;
    s32 shift;
    s32 rows;
    volatile s32 stride;
    s32 sourceX;
    s32 sourceY;
    s32 drawX;
    s32 drawY;
    s32 drawWidth;
    s32 drawHeight;
    s32 nextY;
    s32 rowOffset;
    s32 mask;
    s32 loadCount;
    s32 loadLimit;
    s32 chunkRows;
    u8 intensity;
    u8 *source;
    Overlay101Texture *texture;
    Overlay101Gfx *gfx;
    Overlay101Gfx *lastCommand;

    if ((node->type == 2) || (node->type == 4)) {
        texture = element->texture;
        if (texture != 0) {
            func_overlay_101_F0000000_18DB820(node, &left, &top, &right,
                                              &bottom);
            y = node->y + element->y;
            x = node->x + element->x;
            if (right < x) {
                goto done;
            }
            if (bottom < y) {
                goto done;
            }
            if ((x + texture->width) < left) {
                goto done;
            }
            if ((y + texture->height) < top) {
                goto done;
            }
            {
                func_overlay_101_F0000000_18DB820(displayList, left, top,
                                                  right, bottom);

                loadLimit = 0x800 / texture->width;
                if (loadLimit >= 8) {
                    shift = 3;
                } else {
                    shift = 1;
                    if (loadLimit >= 4) {
                        shift = 2;
                    }
                }
                rows = 1 << shift;
                gfx = *displayList;
                sourceX = 0;
                mask = rows - 1;
                stride = texture->width * rows;
                drawX = x;
                if (x < left) {
                    drawX = left;
                    sourceX = left - x;
                }
                drawWidth = texture->width - sourceX;
                if ((right - drawX) < drawWidth) {
                    drawWidth = right - drawX;
                }
                if (y < top) {
                    drawY = top;
                    sourceY = top - y;
                } else {
                    drawY = y;
                    sourceY = 0;
                }
                drawHeight = texture->height - sourceY;
                if ((bottom - drawY) < drawHeight) {
                    drawHeight = bottom - drawY;
                }

                drawY *= 4;
                drawX *= 4;
                gfx->w0 = 0x06000000;
                gfx->w1 = (u32)D_230;
                gfx++;
                rowOffset = (sourceY & mask) << 5;
                lastCommand = gfx;
                lastCommand->w0 = 0xFA000000;
                intensity = node->intensity;
                source = (u8 *)texture +
                         (stride * (sourceY >> shift) * 2) + 0x10;
                lastCommand->w1 = (intensity << 24) | (intensity << 16) |
                                  (intensity << 8) | node->alpha;
                gfx++;

                if (drawHeight > 0) {
                    s32 rectRight =
                        (((drawX + drawWidth * 4) & 0xFFF) << 12) |
                        0xE4000000;
                    s32 rectLeft = (drawX & 0xFFF) << 12;
                    s32 tileBottom = (mask * 4) & 0xFFF;

                    stride *= 2;
                    sourceX <<= 5;
                    do {
                        gfx->w0 = 0xFD100000;
                        gfx->w1 = (u32)source;
                        gfx++;
                        gfx->w0 = 0xF5100000;
                        gfx->w1 = 0x07080200;
                        gfx++;
                        gfx->w0 = 0xE6000000;
                        gfx->w1 = 0;
                        gfx++;
                        gfx->w0 = 0xF3000000;
                        loadCount = (texture->width * rows) - 1;
                        if (loadCount >= 0x7FF) {
                            loadCount = 0x7FF;
                        }
                        gfx->w1 = ((loadCount & 0xFFF) << 12) | 0x07000000;
                        gfx++;
                        source += stride;
                        gfx->w0 = 0xE7000000;
                        gfx->w1 = 0;
                        gfx++;
                        gfx->w0 =
                            (((((texture->width * 2) + 7) >> 3) & 0x1FF)
                             << 9) |
                            0xF5100000;
                        gfx->w1 = 0x00080200;
                        gfx++;
                        gfx->w0 = 0xF2000000;
                        gfx->w1 =
                            ((((texture->width - 1) * 4) & 0xFFF) << 12) |
                            tileBottom;
                        gfx++;
                        chunkRows = rows - (rowOffset >> 5);
                        if (drawHeight < chunkRows) {
                            chunkRows = drawHeight;
                        }
                        nextY = drawY + chunkRows * 4;
                        gfx->w0 = rectRight | (nextY & 0xFFF);
                        gfx->w1 = rectLeft | (drawY & 0xFFF);
                        gfx++;
                        gfx->w0 = 0xB3000000;
                        gfx->w1 = (sourceX << 16) | (rowOffset & 0xFFFF);
                        gfx++;
                        drawHeight -= chunkRows;
                        lastCommand = gfx;
                        lastCommand->w0 = 0xB2000000;
                        lastCommand->w1 = 0x04000400;
                        gfx++;
                        rowOffset = 0;
                        drawY = nextY;
                    } while (drawHeight > 0);
                }
                *displayList = gfx;
                func_overlay_101_F0000000_18DB820(
                    displayList, lastCommand, nextY, chunkRows);
                func_overlay_101_F0000000_18DB820(displayList, 0, 0, 1000,
                                                  1000);
            }
        }
    }
done:
    ;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o101/func_overlay_101_F0002510_18DDD30/func_overlay_101_F0002510_18DDD30.s")
#endif
