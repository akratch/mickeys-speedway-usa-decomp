#include "PR/ultratypes.h"

/* Pinned DKR v77/v80 and JFG searches found no exact donor. */
void overlay34InterpolateColor(s32 position, s32 length, const u8 *start,
                               const u8 *end, u8 *output) {
    if (position > length) {
        position = length;
    }
    position = (position << 16) / length;
    output[0] = start[0] +
                ((s32)((end[0] - start[0]) * (u32)position) >> 16);
    output[1] = start[1] +
                ((s32)((end[1] - start[1]) * (u32)position) >> 16);
    output[2] = start[2] +
                ((s32)((end[2] - start[2]) * (u32)position) >> 16);
    output[3] = start[3] +
                ((s32)((end[3] - start[3]) * (u32)position) >> 16);
}
