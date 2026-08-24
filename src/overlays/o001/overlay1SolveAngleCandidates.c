#include "PR/ultratypes.h"

extern f32 overlay1SqrtReloc(f32 value);
extern s32 overlay1AngleReloc(f32 y, f32 x);

#ifdef NON_MATCHING
s16 overlay1SolveAngleCandidates(
    f32 x0, f32 y0, f32 x1, f32 y1,
    f32 y2, f32 x2, f32 radius, f32 slope, s32 chooseHigh) {
    f32 dx;
    f32 dy;
    f32 distance;
    f32 sum;
    f32 discriminant;
    f32 discriminantRoot;
    f32 denominator;
    f32 root;
    f32 angleX;
    s16 solutions[2];
    s32 solutionCount;
    s32 sign;

    solutionCount = 0;
    dx = x0 - y1;
    dy = x1 - x2;
    distance = overlay1SqrtReloc((dx * dx) + (dy * dy));
    dy = y2 - y0;
    sum = (dy * slope) + (radius * radius);
    discriminant = (sum * sum) -
        ((slope * slope) * ((distance * distance) + (dy * dy)));

    if (discriminant >= 0.0f) {
        discriminantRoot = overlay1SqrtReloc(discriminant);
        denominator = (((dy * dy) / (distance * distance)) + 1.0f) * 2.0f;

        sign = solutionCount + 2;
        while (sign--) {
            if (sign != 0) {
                root = discriminantRoot;
            } else {
                root = -discriminantRoot;
            }
            root = (root + sum) / denominator;
            if (root >= 0.0f) {
                angleX = overlay1SqrtReloc(root);
                if (distance < 0.0f) {
                    angleX = -angleX;
                }
                solutions[solutionCount] = overlay1AngleReloc(
                    ((dy / distance) * angleX) -
                    ((slope * distance) / (angleX + angleX)), angleX);
                solutionCount++;
            }
        }
    }

    if (solutionCount == 1) {
        return solutions[0];
    }
    if (solutionCount == 2) {
        if (solutions[0] < solutions[1]) {
            return chooseHigh ? solutions[1] : solutions[0];
        }
        return chooseHigh ? solutions[0] : solutions[1];
    }
    return 0x2000;
}

s32 overlay1LoopControlCarrier(s32 value) {
    if (value == 0) {
        return 2;
    }
    return value;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1SolveAngleCandidates/func_overlay_001_F00064F8_18528D8.s")
#endif
