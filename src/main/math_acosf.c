/*
 * PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * src/math/math_acosf.c. Mickey's own linked bytes and relocations remain
 * authoritative.
 */

#include "PR/ultratypes.h"

#define MATH_PI 3.141592741f

extern u16 acoss(s16 value);

f32 acosf(f32 value) {
    s16 intval;
    u16 uintval;

    if (value >= 1) {
        intval = 0x7FFF;
    } else if (value <= -1) {
        intval = -0x7FFF;
    } else {
        intval = value * 0x7FFF;
    }

    uintval = acoss(intval);
    return (uintval * MATH_PI) / 0xFFFF;
}
