#include "PR/ultratypes.h"

/*
 * DKR v77/v80 have related timed interpolation in weather.c, but no matching
 * object donor or identical scalar update routine.
 */
extern s16 gOverlay40BlendTimer;
extern s16 gOverlay40BlendCurrent;
extern s16 gOverlay40BlendOutput;
extern s16 gOverlay40BlendTarget;
extern s16 gOverlay40BlendDuration;

void overlay40Interpolate(s32 amount) {
    if (gOverlay40BlendTimer != 0) {
        gOverlay40BlendOutput = gOverlay40BlendCurrent;
        gOverlay40BlendTimer -= amount;
        if (gOverlay40BlendTimer > 0) {
            gOverlay40BlendOutput +=
                ((gOverlay40BlendTarget - gOverlay40BlendCurrent) *
                 gOverlay40BlendTimer) /
                gOverlay40BlendDuration;
        } else {
            gOverlay40BlendTimer = 0;
        }
    }
}
