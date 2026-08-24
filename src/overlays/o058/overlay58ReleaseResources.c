#include "PR/ultratypes.h"

extern u8 gOverlay58InlineResource[];
extern void *gOverlay58OptionalResource;
extern void overlay58ReleaseReloc(void *resource);

/* DKR v77/v80 and JFG checks found only generic resource cleanup. */
void overlay58ReleaseResources(void) {
    void *resource;

    overlay58ReleaseReloc(gOverlay58InlineResource);
    resource = gOverlay58OptionalResource;
    if (resource != NULL) {
        overlay58ReleaseReloc(resource);
    }
}
