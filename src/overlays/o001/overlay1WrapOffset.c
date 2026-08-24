#include "PR/ultratypes.h"

/* No corresponding DKR/JFG source or object match was found. */
extern s32 gOverlay1SegmentSize;

f32 overlay1WrapOffset(f32 a, f32 b) {
    s32 segmentSize;
    s32 halfSize;
    f32 difference;
    f32 result;

    segmentSize = gOverlay1SegmentSize;
    difference = a - b;
    halfSize = segmentSize >> 1;
    result = difference;
    if (result < (f32)-halfSize) {
        result += (f32)segmentSize;
    }
    if ((f32)halfSize < result) {
        result -= (f32)segmentSize;
    }
    return result;
}
