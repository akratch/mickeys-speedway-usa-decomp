#include "PR/ultratypes.h"

extern f32 overlay99SqrtReloc(f32 value);

/* Pinned DKR v77/v80 and JFG object/source checks found no donor. */
f32 overlay99ProjectVector(f32 x, f32 y, f32 z, f32 dx, f32 dy) {
    f32 result;

    result = (x * x) + (y * y);
    if (result > 0.0f) {
        result = ((x * dx) + (y * dy) + z) / overlay99SqrtReloc(result);
    }
    return result;
}
