#include "PR/ultratypes.h"

extern void *gOverlay13Resource0;
extern void *gOverlay13Resource1;
extern void *gOverlay13Resource2;
extern s32 gOverlay13Active;

extern void overlay13ReleaseReloc(void *resource);

/* DKR v77/v80 and JFG contain only generic null-guarded release patterns. */
void overlay13Release(void) {
    void *resource;

    resource = gOverlay13Resource0;
    if (resource != NULL) {
        overlay13ReleaseReloc(resource);
    }
    resource = gOverlay13Resource1;
    if (resource != NULL) {
        overlay13ReleaseReloc(resource);
    }
    resource = gOverlay13Resource2;
    if (resource != NULL) {
        overlay13ReleaseReloc(resource);
    }
    gOverlay13Active = 0;
}
