#include "overlays/overlay_012.h"

/* Pinned DKR v77/v80 and JFG object scans found no exact donor for this release path. */
void func_overlay_012_F00000C4_186D344(void) {
    s32 i;
    Overlay12Entry *entry;

    if (gOverlay12Ready != 0) {
        if (gOverlay12Resource0 != 0) {
            overlay12ReleaseResource(gOverlay12Resource0);
        }
        if (gOverlay12Resource1 != 0) {
            overlay12ReleaseResource(gOverlay12Resource1);
        }
        if (gOverlay12Resource2 != 0) {
            overlay12ReleaseResource(gOverlay12Resource2);
        }
        if (gOverlay12Resource3 != 0) {
            overlay12ReleaseResource(gOverlay12Resource3);
        }
        if (gOverlay12Resource4 != 0) {
            overlay12ReleaseResource(gOverlay12Resource4);
        }
        if (gOverlay12Resource5 != 0) {
            overlay12ReleaseResourceAlt(gOverlay12Resource5);
        }

        entry = gOverlay12Entries;
        i = 0;
        while (1) {
            i++;
            entry++;
            entry[-1].active = 0;
            if (i < 64) {
                continue;
            }
            break;
        }
        if (!i) {
        }
        gOverlay12Flag1536 = 0;
    }

    gOverlay12Ready = 0;
    gOverlay12Count = 0;
    gOverlay12Selection = 0;
}
