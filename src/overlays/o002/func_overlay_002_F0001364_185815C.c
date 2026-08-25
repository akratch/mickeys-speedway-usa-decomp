#include "PR/ultratypes.h"

/*
 * Plateau (2026-08-25): canonical -O2 -mips2 produces a 0x68-byte frame
 * instead of 0x60, with the first mismatch at +0x0. The standard line-
 * intersection coefficient form is structurally close, but the remaining
 * delta is coefficient lifetime/spill scheduling. The full flag lattice,
 * m2c register-variable form, semantic temporary reuse, nested condition
 * spelling, and selective register qualifiers did not close the difference.
 * Fresh lane recheck: all 119 flag combinations still favor
 * -Wab,-r4300_mul, but its 188-instruction candidate remains one instruction
 * short, differs in 164/189 words from +0x0, and retains the 0x68 frame. A
 * full m2c lifetime spelling regressed to a 0x90 frame and 181 differences;
 * comparing the denominator products before subtracting preserved 164
 * differences but moved the candidate two instructions short. The 0x60
 * target frame remains the unresolved spill-allocation boundary.
 */
#ifdef NON_MATCHING
s32 func_overlay_002_F0001364_185815C(f32 x1, f32 y1, f32 x2, f32 y2,
                                      f32 x3, f32 y3, f32 x4, f32 y4,
                                      f32 *x, f32 *y) {
    f32 a1;
    f32 b1;
    f32 c1;
    f32 a2;
    f32 b2;
    f32 c2;
    f32 r1;
    f32 r2;
    f32 r3;
    f32 r4;
    f32 denom;
    f32 offset;
    f32 num1;
    f32 num2;

    a1 = y2 - y1;
    b1 = x1 - x2;
    c1 = (x2 * y1) - (x1 * y2);
    r3 = (a1 * x3) + (b1 * y3) + c1;
    r4 = (a1 * x4) + (b1 * y4) + c1;
    if ((r3 != 0.0f) && (r4 != 0.0f) &&
        (((r3 > 0.0f) && (r4 > 0.0f)) ||
         ((r3 < 0.0f) && (r4 < 0.0f)))) {
        return 0;
    }

    a2 = y4 - y3;
    b2 = x3 - x4;
    c2 = (x4 * y3) - (x3 * y4);
    r1 = (a2 * x1) + (b2 * y1) + c2;
    r2 = (a2 * x2) + (b2 * y2) + c2;
    if ((r1 != 0.0f) && (r2 != 0.0f) &&
        (((r1 > 0.0f) && (r2 > 0.0f)) ||
         ((r1 < 0.0f) && (r2 < 0.0f)))) {
        return 0;
    }

    denom = (a1 * b2) - (a2 * b1);
    if (denom == 0.0f) {
        *x = x1;
        *y = y1;
        return 2;
    }

    if (x != NULL) {
        if (denom < 0.0f) {
            offset = -denom * 0.5f;
        } else {
            offset = denom * 0.5f;
        }

        num1 = b1 * c2;
        num2 = b2 * c1;
        if (num1 < num2) {
            *x = ((num1 - num2) - offset) / denom;
        } else {
            *x = ((num1 - num2) + offset) / denom;
        }

        num1 = a2 * c1;
        num2 = a1 * c2;
        if (num1 < num2) {
            *y = ((num1 - num2) - offset) / denom;
        } else {
            *y = ((num1 - num2) + offset) / denom;
        }
    }

    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o002/func_overlay_002_F0001364_185815C/func_overlay_002_F0001364_185815C.s")
#endif
