#include "PR/ultratypes.h"

typedef struct Overlay17StripPoint {
    s16 x0, y0, z0;
    u8 r0, g0, b0, a0;
    s16 x1, y1, z1;
    u8 r1, g1, b1, a1;
} Overlay17StripPoint;

typedef struct Overlay17Chain {
    s16 count;
    u8 selectedBuffer;
    u8 pad03[0x21];
    u8 red, green, blue, alpha;
    u8 pad28[4];
    Overlay17StripPoint *buffers[2];
} Overlay17Chain;

extern void func_overlay_017_F0000000_18739B8(Overlay17Chain *chain,
                                               f32 *x0, f32 *y0, f32 *z0,
                                               f32 *x1, f32 *y1, f32 *z1);

/* Pinned DKR v77/v80 and JFG scans found no matching chain-update body. */
void overlay17AdvanceChain(Overlay17Chain *chain, s32 useAlpha) {
    u8 *writeCursor;
    s32 savedAlpha;
    f32 x0, y0, z0, x1, y1, z1;
    u16 *sourceCursor;
    u16 *destinationCursor;
    s32 count;
    u8 oldBuffer;
    u8 newBuffer;

    if (chain == 0) {
        return;
    }

    count = chain->count;
    oldBuffer = chain->selectedBuffer;
    sourceCursor = (u16 *)((u8 *)chain->buffers[oldBuffer] +
                           (((count - 1) << 1) * 10));
    newBuffer = oldBuffer ^ 1;
    chain->selectedBuffer = newBuffer;
    writeCursor = (u8 *)chain->buffers[newBuffer];
    destinationCursor = (u16 *)(writeCursor +
                                ((count << 1) * 10));
    count--;
    count = (count << 2) + count;
    count <<= 1;
    if (count--) {
        do {
            u16 value = sourceCursor[-1];
            destinationCursor--;
            sourceCursor--;
            *destinationCursor = value;
        } while (count--);
        writeCursor = (u8 *)chain->buffers[chain->selectedBuffer];
    }
    if (useAlpha != 0) {
        savedAlpha = chain->alpha;
    } else {
        savedAlpha = 0;
    }
    func_overlay_017_F0000000_18739B8(chain, &x0, &y0, &z0,
                                      &x1, &y1, &z1);

    writeCursor += 20;
    *(s16 *)(writeCursor - 20) = (s16)(s32)x0;
    *(s16 *)(writeCursor - 18) = (s16)(s32)y0;
    *(s16 *)(writeCursor - 16) = (s16)(s32)z0;
    writeCursor[-14] = chain->red;
    writeCursor[-13] = chain->green;
    writeCursor[-12] = chain->blue;
    writeCursor[-11] = (u8)savedAlpha;
    *(s16 *)(writeCursor - 10) = (s16)(s32)x1;
    *(s16 *)(writeCursor - 8) = (s16)(s32)y1;
    *(s16 *)(writeCursor - 6) = (s16)(s32)z1;
    writeCursor[-4] = chain->red;
    writeCursor[-3] = chain->green;
    writeCursor[-2] = chain->blue;
    writeCursor[-1] = (u8)savedAlpha;

    count = chain->count - 1;
    while (count--) {
        if (writeCursor[9] != 0) {
            savedAlpha = (chain->alpha * count) / (chain->count - 1);
            writeCursor[9] = (u8)savedAlpha;
            writeCursor[19] = (u8)savedAlpha;
        }
        writeCursor += 20;
    }
}
