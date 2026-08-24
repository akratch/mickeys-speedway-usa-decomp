#include "PR/ultratypes.h"
extern f32 overlay1EvaluateCurve(f32, f32, s32, s32, f32);
extern f32 overlay1SquareRoot(f32);
#ifdef NON_MATCHING
f32 overlay1MeasureCurves(volatile f32 startX, volatile f32 startY,
                          volatile f32 endX, volatile f32 endY,
                          volatile s32 controlX1, volatile s32 controlY1,
                          volatile s32 controlX2, volatile s32 controlY2,
                          s32 segmentCount) {
    f32 t = 0.0f;
    volatile f32 unusedLocal;
    volatile f32 total = 0.0f;
    f32 previousX = overlay1EvaluateCurve(startX, endX, controlX1, controlX2, 0.0f);
    f32 previousY = overlay1EvaluateCurve(startY, endY, controlY1, controlY2, 0.0f);
    f32 step;
    f32 x;
    f32 y;
    f32 dx;
    f32 dy;
    s32 remaining = segmentCount - 1;
    if (segmentCount != 0) {
        step = 1.0f / (f32)segmentCount;
        do {
            t += step;
            x = overlay1EvaluateCurve(startX, endX, controlX1, controlX2, t);
            y = overlay1EvaluateCurve(startY, endY, controlY1, controlY2, t);
            dx = x - previousX;
            dy = y - previousY;
            total += overlay1SquareRoot((dx * dx) + (dy * dy));
            previousX = x;
            previousY = y;
        } while (remaining--);
    }
    return total;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1MeasureCurves/func_overlay_001_F0000F84_184D364.s")
#endif
