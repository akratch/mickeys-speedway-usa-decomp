#include "PR/ultratypes.h"

extern s32 gOverlay2BoundaryAxis;
extern f32 gOverlay2BoundaryValue;

/* Exact at +0x400; DKR v77/v80 and JFG have no exact donor for this helper. */
void overlay2IntersectBoundary(f32 x0, f32 y0, f32 x1, f32 y1,
                               f32 *outX, f32 *outY) {
    f32 ratio;

    if (gOverlay2BoundaryAxis == 0) {
        ratio = (gOverlay2BoundaryValue - y0) / (y1 - y0);
        *outX = ((x1 - x0) * ratio) + x0;
        *outY = gOverlay2BoundaryValue;
    } else {
        ratio = (gOverlay2BoundaryValue - x0) / (x1 - x0);
        *outX = gOverlay2BoundaryValue;
        *outY = ((y1 - y0) * ratio) + y0;
    }
}
