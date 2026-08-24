#include "PR/ultratypes.h"

typedef struct Overlay43Image {
    u8 pad0[0x20];
    u8 *pixels;
} Overlay43Image;

/* DKR v77/v80 and JFG contain no exact donor for this image filter. */
void overlay43FilterImage(Overlay43Image *image) {
    u8 *pixel;
    u32 *word;
    register s32 row;
    register s32 column;
    register u16 sum;

    pixel = image->pixels;
    row = 0x3D;
    do {
        column = 0x3D;
        do {
            if (!pixel) {
            }
            if (!image->pixels) {
            }
            sum = (((((((0, pixel[1] + pixel[0])) + pixel[0x40]) +
                    pixel[0x42]) + pixel[0x80]) + pixel[0x81]) +
                    pixel[0x82]) + pixel[2];
            pixel[0x41] = sum >> 3;
            pixel++;
        } while (column--);
        pixel += 2;
    } while (row--); word = (u32 *)image->pixels; row = 0x3FF; do {
        *word = (*word & 0xF0F0F0F0) >> 4;
        word++;
    } while (row--);
}
