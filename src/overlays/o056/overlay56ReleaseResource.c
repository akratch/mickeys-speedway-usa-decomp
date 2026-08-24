#include "PR/ultratypes.h"

/* DKR v77/v80: generic null-guarded frees only; no semantic donor. */
extern void *gOverlay56Resource;
void overlay56ReleaseResourceReloc(void *resource);

void overlay56ReleaseResource(void) {
    if (gOverlay56Resource != 0) {
        overlay56ReleaseResourceReloc(gOverlay56Resource);
        gOverlay56Resource = 0;
    }
}
