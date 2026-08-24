#include "PR/ultratypes.h"

/* DKR v77/v80: viewport-centering idioms only; no exact source/object donor. */
void overlay56GetDimensionsReloc(u32 *width, u32 *height);

void overlay56OffsetCoordinates(u32 *x, u32 *y) {
    u32 width;
    u32 height;

    overlay56GetDimensionsReloc(&width, &height);
    *x += width >> 1;
    *y = (height >> 1) - *y;
}

void overlay56CenterCoordinates(s32 *x, s32 *y) {
    u32 width;
    u32 height;

    overlay56GetDimensionsReloc(&width, &height);
    *x -= width >> 1;
    *y = (height >> 1) - *y;
}
