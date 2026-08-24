#include "PR/ultratypes.h"

/* Pinned DKR v77/v80 and JFG object scans have no exact donor. */
s32 overlay2IntersectSegments(f32 arg0, f32 arg1, f32 arg2, f32 arg3,
                              f32 arg4, f32 arg5, f32 arg6, f32 arg7,
                              f32 *arg8, f32 *arg9) {
    f32 a1;
    f32 b1;
    f32 c1;
    f32 side1;
    f32 side2;
    f32 a2;
    f32 b2;
    f32 c2;
    f32 denominator;
    f32 bias;
    f32 numerator;

    a1 = arg3 - arg1;
    b1 = arg0 - arg2;
    c1 = (arg2 * arg1) - (arg0 * arg3);
    side1 = (a1 * arg4) + (b1 * arg5) + c1;
    side2 = (a1 * arg6) + (b1 * arg7) + c1;
    if (side1 != 0.0f) {
        if ((side2 != 0.0f) &&
            (((side1 > 0.0f) && (side2 > 0.0f)) ||
             ((side1 < 0.0f) && (side2 < 0.0f)))) {
            return 0;
        }
    }
    b2 = arg4 - arg6;
    a2 = arg7 - arg5;
    c2 = (arg6 * arg5) - (arg4 * arg7);
    side1 = (a2 * arg0) + (b2 * arg1) + c2;
    side2 = (a2 * arg2) + (b2 * arg3) + c2;
    if ((side1 != 0.0f) && (side2 != 0.0f) &&
        (((side1 > 0.0f) && (side2 > 0.0f)) ||
         ((side1 < 0.0f) && (side2 < 0.0f)))) {
        return 0;
    }
    side1 = a2 * b1;
    side2 = a1 * b2;
    if (side1 == side2) {
        *arg8 = arg0;
        *arg9 = arg1;
        return 2;
    }
    if (arg8 != NULL) {
        denominator = side2 - side1;
        if (denominator < 0.0f) {
            bias = -denominator * 0.5f;
        } else {
            bias = denominator * 0.5f;
        }
        side1 = b1 * c2;
        side2 = b2 * c1;
        if (side1 < side2) {
            numerator = (side1 - side2) - bias;
        } else {
            numerator = (side1 - side2) + bias;
        }
        *arg8 = numerator / denominator;
        side1 = a2 * c1;
        side2 = a1 * c2;
        if (side1 < side2) {
            numerator = (side1 - side2) - bias;
        } else {
            numerator = (side1 - side2) + bias;
        }
        *arg9 = numerator / denominator;
    }
    return 1;
}
