#include "PR/ultratypes.h"

/* PROVENANCE: adapted from Diddy Kong Racing, src/objects.c (decrypt_magic_codes). */
void overlay30TransposePixels(u8 *data, s32 length) {
    s32 i;
    s32 j;
    u8 *cursor = data;
    u8 temp[4];

    for (i = 0; i < (length >> 2); i++) {
        temp[0] = (cursor[0] & 0xC0) |
                  ((cursor[1] & 0xC0) >> 2) |
                  ((cursor[2] & 0xC0) >> 4) |
                  ((cursor[3] & 0xC0) >> 6);
        temp[1] = ((cursor[0] & 0x30) << 2) |
                  (cursor[1] & 0x30) |
                  ((cursor[2] & 0x30) >> 2) |
                  ((cursor[3] & 0x30) >> 4);
        temp[2] = ((cursor[0] & 0x0C) << 4) |
                  ((cursor[1] & 0x0C) << 2) |
                  (cursor[2] & 0x0C) |
                  ((cursor[3] & 0x0C) >> 2);
        temp[3] = ((cursor[0] & 3) << 6) |
                  ((cursor[1] & 3) << 4) |
                  ((cursor[2] & 3) << 2) |
                  (cursor[3] & 3);

        for (j = 0; j < 4; j++) {
            *cursor++ = ((temp[j] & 0xAA) >> 1) |
                        ((temp[j] & 0x55) << 1);
        }
    }
}
