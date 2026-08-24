#include "PR/ultratypes.h"

extern void *gOverlay16Buffer;
extern void overlay16ReleaseReloc(void *buffer);

/* DKR v77/v80 and JFG have no donor for this ownership wrapper. */
void overlay16ReleaseBuffer(void) {
    if (gOverlay16Buffer != NULL) {
        overlay16ReleaseReloc(gOverlay16Buffer);
        gOverlay16Buffer = NULL;
    }
}
