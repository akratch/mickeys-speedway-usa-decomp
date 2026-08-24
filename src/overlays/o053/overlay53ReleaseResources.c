#include "PR/ultratypes.h"

/* Fresh pinned DKR v77/v80 and JFG scans found no exact donor for this cluster. */
extern void overlay53ReleaseReloc();
extern u8 gOverlay53ResourceA[];
extern s32 gOverlay53Handle0;
extern s32 gOverlay53HandleEnd;

void overlay53ReleaseResources(void) {
    s32 *handle;
    s32 *end;

    overlay53ReleaseReloc(gOverlay53ResourceA);
    overlay53ReleaseReloc();
    handle = &gOverlay53Handle0; end = &gOverlay53HandleEnd;
    do {
        if (*handle != -1) {
            overlay53ReleaseReloc(*handle);
            *handle = -1;
        }
        handle++;
    } while (handle != end);
}
