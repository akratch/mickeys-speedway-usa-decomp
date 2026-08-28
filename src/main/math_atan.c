/*
 * PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * src/math/math_atan.c. Mickey's own linked bytes and relocations remain
 * authoritative.
 */

#include "PR/ultratypes.h"

#define MATH_PI 3.141592741f
#define DTOR(degrees) ((degrees) * MATH_PI / 180.0f)

extern f32 sqrtf(f32 value);
extern f32 acosf(f32 value);

f32 atan2f(f32 x, f32 z) {
    f32 result = 0;

    if (x == 0) {
        if (z >= 0) {
            result = 0;
        } else {
            result = DTOR(180);
        }
    } else if (z == 0) {
        if (x > 0) {
            result = DTOR(90);
        } else {
            result = DTOR(270);
        }
    } else {
        f32 sqrt = sqrtf(x * x + z * z);

        if (x > z) {
            result = acosf(z / sqrt);

            if (x < 0) {
                result = DTOR(360) - result;
            }
        } else {
            result = DTOR(90) - acosf(x / sqrt);

            if (z < 0) {
                result = DTOR(180) - result;
            }

            if (result < 0) {
                result += DTOR(360);
            }
        }
    }

    return result;
}
