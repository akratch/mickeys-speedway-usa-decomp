#include "PR/ultratypes.h"

/* Separate lvalues preserve the original phase-write register allocation. */
extern s32 gOverlay11ModeStart;
extern s32 gOverlay11ModeFinish;
extern void *gOverlay11Handles[4];
extern void *gOverlay11Created[];
extern void overlay11SetValue(void *handle, s32 value);

/* DKR v77/v80 contain only generic fixed-handle update loops. */
void overlay11EnableHandles(s32 count) {
    s32 i;

    gOverlay11ModeStart = 1;
    overlay11SetValue(gOverlay11Handles[0], 0xFF);
    overlay11SetValue(gOverlay11Handles[1], 0xFF);
    overlay11SetValue(gOverlay11Handles[3], 0xFF);
    overlay11SetValue(gOverlay11Handles[2], 0xFF);
    overlay11SetValue(gOverlay11Handles[0], 0);
    overlay11SetValue(gOverlay11Handles[1], 0);
    overlay11SetValue(gOverlay11Handles[3], 0);
    overlay11SetValue(gOverlay11Handles[2], 0);
    gOverlay11ModeFinish = 3;
    for (i = 0; i < count; i++) {
        overlay11SetValue(gOverlay11Created[i], 0);
    }
}
