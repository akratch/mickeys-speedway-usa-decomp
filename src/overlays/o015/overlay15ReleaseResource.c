#include "PR/ultratypes.h"

/* DKR v77/v80 has generic resource-release wrappers but no exact donor. */
extern void *gOverlay15Resource4;
extern void *gOverlay15Resource48;
extern void overlay15ReleaseReloc(void *resource);

void overlay15ReleaseResource(void) {
    if (gOverlay15Resource4 != 0) {
        overlay15ReleaseReloc(gOverlay15Resource4);
        gOverlay15Resource4 = 0;
        gOverlay15Resource48 = 0;
    }
}
