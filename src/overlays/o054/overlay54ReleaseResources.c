#include "PR/ultratypes.h"

/* Fresh pinned DKR v77/v80 and JFG scans found no exact donor for this cluster. */
extern void overlay54ReleaseReloc();
extern void *gOverlay54Current;
extern u8 gOverlay54ResourceA[];
extern u8 gOverlay54ResourceB[];

void overlay54ReleaseResources(void) {
    if (gOverlay54Current != NULL) {
        overlay54ReleaseReloc(gOverlay54Current);
    }
    overlay54ReleaseReloc(gOverlay54ResourceA);
    overlay54ReleaseReloc(gOverlay54ResourceB);
    overlay54ReleaseReloc();
}
