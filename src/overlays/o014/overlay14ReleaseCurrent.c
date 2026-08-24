#include "PR/ultratypes.h"

extern s32 gOverlay14ResourceD0;
extern s32 gOverlay14ValueD4;
extern s32 gOverlay14FlagC8;
extern void overlay14ReleasePairReloc(s32 resource, s32 value);

/* DKR v77/v80 and JFG checks found no exact donor for this cleanup wrapper. */
void overlay14ReleaseCurrent(void) {
    s32 resource = gOverlay14ResourceD0;

    gOverlay14FlagC8 = 0;
    if (resource != -1) {
        overlay14ReleasePairReloc(resource, gOverlay14ValueD4);
    }
}
