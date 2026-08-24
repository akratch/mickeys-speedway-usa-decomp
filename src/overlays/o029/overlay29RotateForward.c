#include "ultra64.h"

extern s32 gOverlay29Value0;
extern s32 gOverlay29Value1;
extern s32 gOverlay29Value2;
extern s32 gOverlay29Value3;
extern s32 overlay29AdvanceReloc(s32 value);

/* This title-specific four-value rotation has no DKR v77/v80 or JFG donor. */
void overlay29RotateForward(s32 count) {
    while (count--) {
        gOverlay29Value0 = gOverlay29Value1;
        gOverlay29Value1 = gOverlay29Value2;
        gOverlay29Value2 = gOverlay29Value3;
        gOverlay29Value3 = overlay29AdvanceReloc(gOverlay29Value3);
    }
    overlay29AdvanceReloc(666);
}
