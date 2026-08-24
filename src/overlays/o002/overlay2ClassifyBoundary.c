#include "PR/ultratypes.h"

extern s32 gOverlay2BoundaryAxis;
extern f32 gOverlay2BoundaryValue;

/* Pinned DKR v77/v80 and JFG scans found no boundary classifier donor. */
#ifdef NON_MATCHING
s32 overlay2ClassifyBoundary(f32 x1, f32 y1, f32 x2, f32 y2, s32 *side1,
                             s32 *side2) {
    s32 first;
    s32 second;

    if (gOverlay2BoundaryAxis == 0) {
        first = 0;
        if (y1 < gOverlay2BoundaryValue) {
            first = 1;
        }
        *side1 = first;
        second = 0;
        if (y2 < gOverlay2BoundaryValue) {
            second = 1;
        }
        *side2 = second;
        if (y1 == gOverlay2BoundaryValue) {
            *side1 = second;
            return 1;
        }
        if (y2 == gOverlay2BoundaryValue) {
            *side2 = *side1;
            return 1;
        }
    } else {
        first = 0;
        if (x1 < gOverlay2BoundaryValue) {
            first = 1;
        }
        *side1 = first;
        second = 0;
        if (x2 < gOverlay2BoundaryValue) {
            second = 1;
        }
        *side2 = second;
        if (x1 == gOverlay2BoundaryValue) {
            *side1 = second;
            return 1;
        }
        if (x2 == gOverlay2BoundaryValue) {
            *side2 = *side1;
        }
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o002/overlay2ClassifyBoundary/func_overlay_002_F00002C4_18570BC.s")
#endif
