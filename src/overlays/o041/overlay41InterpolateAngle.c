#include "PR/ultratypes.h"

/* Fresh pinned DKR v77/v80 and JFG object scans found no exact donor. */
s16 overlay41InterpolateAngle(f32 amount, s16 start, s16 end) {
    s32 delta;
    s32 reverse;

    delta = (u16)(end - start);
    reverse = (u16)(start - end);
    if (reverse < delta) {
        delta = -reverse;
    }
    return start + (s32)(delta * amount);
}
