#include "PR/ultratypes.h"

extern s32 gOverlay14ValueC0;
extern s32 func_overlay_014_F0000000_186F8D8();

#define CASE_PREINC 1

#ifdef NON_MATCHING
/* PROVENANCE: structure cross-checked against JFG
 * asm/nonmatchings/overlays/o7/overlay_7/func_overlay_7_007023D4_1EFD4FC.s;
 * body reconstructed from Mickey evidence. */
/* Workbench mixed: 2 structural/3 register, five positional words, first +0x1F4.
 * Levers tried: constant audit, stack order, flag lattice, case-7 line/postincrement forms.
 * Remaining: case-7 load/increment schedule; postincrement variants compile one word short. */
s32 func_overlay_014_F0001830_1871108(s32 context, u8 *stream, s32 skip) {
    s32 remaining;
    s32 y;
    s32 x;
    s32 extra;
    s32 width;
    s32 cellWidth;
    s32 done;
    s32 result;
    u8 saved;
    void *drawArgs[7];
    s32 adjust;
    u8 *cursor;

    result = 0;
    done = 0;
    cellWidth = func_overlay_014_F0000000_186F8D8(2);
    remaining = (0x58 / cellWidth) - 1;
    func_overlay_014_F0000000_186F8D8(0, 0, 0, 0);
    func_overlay_014_F0000000_186F8D8(0xFF, 0xFF, 0xFF, 0xFF,
                                      (gOverlay14ValueC0 * 0xFF) >> 8);
    func_overlay_014_F0000000_186F8D8(2);
    y = ((0x58 - (remaining * cellWidth)) >> 1) + 0x14;
    do {
        x = 0x60; width = 0xC8; cursor = 0; adjust = 0; extra = 0;
        switch (*stream) {
        case 1:
            if (skip == 0) { y += cellWidth; remaining--; } else skip--;
            stream += 4;
            break;
        case 2:
            stream += 4;
            cursor = (u8 *)((*((u32 *)(stream - 4)) & 0xFFFFFF) | 0x80000000);
            extra = 4; x = 0xC4;
            break;
        case 3:
            stream += 4;
            cursor = (u8 *)((*((u32 *)(stream - 4)) & 0xFFFFFF) | 0x80000000);
            break;
        case 4:
            stream += 4;
            cursor = (u8 *)((*((u32 *)(stream - 4)) & 0xFFFFFF) | 0x80000000);
            extra = 1; x = 0x127;
            break;
        case 5:
            stream += 4;
            cursor = (u8 *)((*((u32 *)(stream - 4)) & 0xFFFFFF) | 0x80000000);
            adjust = 1;
            break;
        case 6:
#if CASE_PREINC
            stream += 8;
            saved = stream[-2];
            func_overlay_014_F0000000_186F8D8(stream[-7], stream[-6], stream[-5], stream[-3],
                                              (saved * gOverlay14ValueC0) >> 8);
#else
            func_overlay_014_F0000000_186F8D8(stream[1], stream[2], stream[3], stream[5],
                                              (stream[6] * gOverlay14ValueC0) >> 8);
            stream += 8;
#endif
            break;
        case 7:
            stream += 8;
            func_overlay_014_F0000000_186F8D8(stream[-7], stream[-6], stream[-5], stream[-3]);
            break;
        default:
            done = 1;
            break;
        }
        if ((cursor != 0) && (remaining >= 0)) {
            do {
                if (adjust != 0) { x += 8; width -= 8; }
                cursor = (u8 *)func_overlay_014_F0000000_186F8D8(2, cursor, width, drawArgs, 0);
                if (cursor != 0) {
                    if (skip == 0) {
                        remaining--;
                        if (remaining >= 0) {
                            saved = *cursor; *cursor = 0;
                            func_overlay_014_F0000000_186F8D8(context, x, y, drawArgs[0], extra);
                            *cursor = saved; y += cellWidth;
                        }
                    } else {
                        skip--;
                    }
                }
                if (adjust != 0) { x -= 8; width += 8; adjust = 0; }
            } while ((cursor != 0) && (remaining >= 0));
        }
        if (remaining < 0) {
            result = 1;
            done = 1;
        }
    } while (done == 0);
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o014/func_overlay_014_F0001830_1871108/func_overlay_014_F0001830_1871108.s")
#endif
