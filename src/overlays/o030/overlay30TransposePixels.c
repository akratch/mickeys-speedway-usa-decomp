#include "PR/ultratypes.h"

typedef struct Overlay30Groups {
    u8 g0;
    u8 g1;
    u8 g2;
    u8 g3;
    u8 pad[12];
} Overlay30Groups;

void overlay30TransposePixels(u8 *data, s32 length) {
    volatile Overlay30Groups groups;
    u8 temp;
    register s32 count;
    register u8 *cursor;
    register s32 i;

    cursor = data;
    i = 0;
    count = length >> 2;
    if (count > 0) {
        do {
            groups.g0 = ((s32)(cursor[2] & 0xC0) >> 4) |
                        ((s32)(cursor[1] & 0xC0) >> 2) |
                        (cursor[0] & 0xC0) |
                        ((s32)(cursor[3] & 0xC0) >> 6);
            groups.g1 = ((cursor[0] & 0x30) * 4) |
                        (cursor[1] & 0x30) |
                        ((s32)(cursor[3] & 0x30) >> 4) |
                        ((s32)(cursor[2] & 0x30) >> 2);
            groups.g2 = ((s32)(cursor[3] & 0x0C) >> 2) |
                        ((cursor[0] & 0x0C) * 0x10) |
                        ((cursor[1] & 0x0C) * 4) |
                        (cursor[2] & 0x0C);
            groups.g3 = (cursor[3] & 3) | (cursor[0] << 6) |
                        ((cursor[1] & 3) * 0x10) |
                        ((cursor[2] & 3) * 4);

            temp = groups.g0;
            cursor[0] = ((temp & 0x55) * 2) |
                        ((s32)(temp & 0xAA) >> 1);
            i++;
            cursor += 4;
            temp = groups.g1;
            cursor[-3] = ((temp & 0x55) * 2) |
                         ((s32)(temp & 0xAA) >> 1);
            temp = groups.g2;
            cursor[-2] = ((temp & 0x55) * 2) |
                         ((s32)(temp & 0xAA) >> 1);
            temp = groups.g3;
            cursor[-1] = ((temp & 0x55) * 2) |
                         ((s32)(temp & 0xAA) >> 1);
        } while (i != count);
    }
}
