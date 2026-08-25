#include "PR/ultratypes.h"

typedef struct Overlay20Command {
    u32 w0;
    u32 w1;
} Overlay20Command;

typedef struct Overlay20TileSource {
    u8 pad00[0xA];
    s16 width;
    s16 rows;
    u8 pad0E[4];
    u8 variant;
    u8 pad13[0xD];
    s32 textureOffsets[2];
    s32 outputOffset;
    void *resource;
} Overlay20TileSource;

extern void overlay20TileSetupReloc(Overlay20Command **commands,
                                    void *resource, s32 arg2, s32 arg3);

#define OVERLAY20_SHIFTL(value, shift, width)                              \
    ((u32)(((u32)(value) & ((1U << (width)) - 1)) << (shift)))

/* Source-shape crosswalk: Diddy Kong Racing include/PR/gbi.h:gDma1p and
 * include/f3ddkr.h:gSPPolygon. Mickey's ten-byte vertex records decide the
 * length expression here; the published DKR macros only supply the command
 * construction idiom. */
#define OVERLAY20_VERTEX(packet, address, count, lengthFlags)               \
    {                                                                       \
        Overlay20Command *_g = (Overlay20Command *)(packet);                \
        _g->w0 = OVERLAY20_SHIFTL(4, 24, 8) |                               \
                 OVERLAY20_SHIFTL(((count) << 3) |                          \
                                       ((u32)(address) & 6),                \
                                   16, 8) |                                 \
                 OVERLAY20_SHIFTL((((count) << 3) + ((count) << 1) + 8) |   \
                                       (lengthFlags),                       \
                                   0, 16);                                  \
        _g->w1 = (u32)(address);                                            \
    }

#define OVERLAY20_POLYGON(packet, address, count, textured)                 \
    {                                                                       \
        Overlay20Command *_g = (Overlay20Command *)(packet);                \
        _g->w0 = OVERLAY20_SHIFTL((((count) - 1) << 4) | (textured),        \
                                   16, 8) |                                 \
                 OVERLAY20_SHIFTL(5, 24, 8) |                               \
                 OVERLAY20_SHIFTL((count) << 4, 0, 16);                    \
        _g->w1 = (u32)(address);                                            \
    }

/*
 * Plateau (2026-08-25, 6 attempts): the closest exact-size -O2 experiment
 * differs in 5 of 134 words and first diverges at +0x48.  A seven-element
 * chunk array recovers the retail frame but is not retained because the
 * source cannot prove that reduced capacity; the 13-element body instead
 * differs in 7 words.  The unresolved shape is the stack allocation for the
 * three command-macro temporaries plus one doubled-width register choice.
 */
#ifdef NON_MATCHING
void overlay20BuildTileCommands(Overlay20Command **commands,
                                Overlay20TileSource *source, s32 arg2) {
    s32 chunks[13];
    s32 width;
    s32 remainingRows;
    s32 chunkCount;
    s32 *writeCursor;
    s32 *readCursor;
    s32 textureOffset;
    s32 outputOffset;
    s32 chunkWidth;

    overlay20TileSetupReloc(commands, source->resource, arg2, 0);
    width = source->width;
    chunkCount = 0;
    writeCursor = chunks;
    if (width != 0) {
        do {
            if (width >= 9) {
                *writeCursor = 8;
                chunkCount++;
                writeCursor++;
                width -= 8;
            } else {
                *writeCursor = width;
                chunkCount++;
                writeCursor++;
                width = 0;
            }
        } while (width != 0);
    }

    textureOffset = source->textureOffsets[source->variant];
    remainingRows = source->rows;
    outputOffset = source->outputOffset;
    while (remainingRows--) {
        width = 0;
        if (chunkCount > 0) {
            readCursor = chunks;
            do {
                chunkWidth = *readCursor;
                OVERLAY20_VERTEX((*commands)++,
                                 textureOffset + (s32)0x80000000,
                                 chunkWidth + 1, 0);
                width++;
                readCursor++;
                OVERLAY20_VERTEX(
                    (*commands)++,
                    textureOffset + source->width * 10 + (s32)0x8000000A,
                    chunkWidth + 1, 0x1200);
                OVERLAY20_POLYGON((*commands)++,
                                  outputOffset + (s32)0x80000000,
                                  chunkWidth << 1, 1);
                textureOffset += chunkWidth * 10;
                outputOffset += (chunkWidth << 1) << 4;
            } while (width != chunkCount);
        }
        textureOffset += 10;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o020/overlay20BuildTileCommands/func_overlay_020_F00007C4_1876D9C.s")
#endif
