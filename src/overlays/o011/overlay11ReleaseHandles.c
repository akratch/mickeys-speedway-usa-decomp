#include "PR/ultratypes.h"

extern void *gOverlay11Handles[4];
extern void overlay11ReleaseReloc(void *handle);

/* DKR v77/v80 and JFG checks found only generic fixed-handle cleanup. */
void overlay11ReleaseHandles(void) {
    overlay11ReleaseReloc(gOverlay11Handles[0]);
    overlay11ReleaseReloc(gOverlay11Handles[1]);
    overlay11ReleaseReloc(gOverlay11Handles[2]);
    overlay11ReleaseReloc(gOverlay11Handles[3]);
    gOverlay11Handles[0] = NULL;
    gOverlay11Handles[1] = NULL;
    gOverlay11Handles[2] = NULL;
    gOverlay11Handles[3] = NULL;
}
