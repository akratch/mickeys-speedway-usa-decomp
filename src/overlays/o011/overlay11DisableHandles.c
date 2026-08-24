#include "PR/ultratypes.h"

extern s32 gOverlay11Flag;
extern s32 gOverlay11Mode;
extern void *gOverlay11Handles[4];
extern void *gOverlay11Created[];
extern void overlay11SetValue(void *handle, s32 value);

/* DKR v77/v80 contain only generic fixed-handle update loops. */
void overlay11DisableHandles(s32 count) {
    s32 i;

    gOverlay11Flag = 0;
    gOverlay11Mode = 0;
    overlay11SetValue(gOverlay11Handles[0], 0);
    overlay11SetValue(gOverlay11Handles[1], 0);
    overlay11SetValue(gOverlay11Handles[3], 0);
    overlay11SetValue(gOverlay11Handles[2], 0);
    for (i = 0; i < count; i++) {
        overlay11SetValue(gOverlay11Created[i], 0xFF);
    }
}
