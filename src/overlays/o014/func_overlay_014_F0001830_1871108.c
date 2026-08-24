#include "PR/ultratypes.h"

extern s32 gOverlay14ValueC0;
extern s32 overlay14Dispatch();

#define CASE_PREINC 1

#ifdef NON_MATCHING
s32 func_overlay_014_F0001830_1871108(s32 context, u8 *stream, s32 skip) {
    s32 result = 0;
    s32 done = 0;
    s32 cellWidth;
    s32 extra;
    s32 remaining;
    s32 y;
    s32 x;
    s32 width;
    s32 adjust;
    u8 *cursor;
    void *drawArgs[7];
    u8 saved;

    cellWidth = overlay14Dispatch(2);
    remaining = (0x58 / cellWidth) - 1;
    overlay14Dispatch(0, 0, 0, 0);
    overlay14Dispatch(0xFF, 0xFF, 0xFF, 0xFF,
                      (gOverlay14ValueC0 * 0xFF) >> 8);
    overlay14Dispatch(2);
    y = ((0x58 - (remaining * cellWidth)) >> 1) + 0x14;
    do {
        x = 0x60; width = 0xC8; cursor = 0; adjust = 0; extra = 0;
        switch (*stream) {
        case 1:
            if (skip == 0) { y += cellWidth; remaining--; } else skip--;
            stream += 4;
            break;
        case 2:
            cursor = (u8 *)((*((u32 *)stream) & 0xFFFFFF) | 0x80000000);
            stream += 4; extra = 4; x = 0xC4;
            break;
        case 3:
            cursor = (u8 *)((*((u32 *)stream) & 0xFFFFFF) | 0x80000000);
            stream += 4;
            break;
        case 4:
            cursor = (u8 *)((*((u32 *)stream) & 0xFFFFFF) | 0x80000000);
            stream += 4; extra = 1; x = 0x127;
            break;
        case 5:
            cursor = (u8 *)((*((u32 *)stream) & 0xFFFFFF) | 0x80000000);
            stream += 4; adjust = 1;
            break;
        case 6:
#if CASE_PREINC
            stream += 8;
            overlay14Dispatch(stream[-7], stream[-6], stream[-5], stream[-3],
                              (stream[-2] * gOverlay14ValueC0) >> 8);
#else
            overlay14Dispatch(stream[1], stream[2], stream[3], stream[5],
                              (stream[6] * gOverlay14ValueC0) >> 8);
            stream += 8;
#endif
            break;
        case 7:
#if CASE_PREINC
            stream += 8;
            overlay14Dispatch(stream[-7], stream[-6], stream[-5], stream[-3]);
#else
            overlay14Dispatch(stream[1], stream[2], stream[3], stream[5]);
            stream += 8;
#endif
            break;
        default:
            done = 1;
            break;
        }
        if ((cursor != 0) && (remaining >= 0)) {
            do {
                if (adjust != 0) { x += 8; width -= 8; }
                cursor = (u8 *)overlay14Dispatch(2, cursor, width, drawArgs, 0);
                if (cursor != 0) {
                    if (skip != 0) skip--;
                    else {
                        remaining--;
                        if (remaining >= 0) {
                            saved = *cursor; *cursor = 0;
                            overlay14Dispatch(context, x, y, drawArgs[0], extra);
                            *cursor = saved; y += cellWidth;
                        }
                    }
                }
                if (adjust != 0) { x -= 8; width += 8; adjust = 0; }
            } while ((cursor != 0) && (remaining >= 0));
        }
        if (remaining < 0) { result = 1; done = 1; }
    } while (done == 0);
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o014/func_overlay_014_F0001830_1871108/func_overlay_014_F0001830_1871108.s")
#endif
