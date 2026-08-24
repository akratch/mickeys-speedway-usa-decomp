#include "PR/ultratypes.h"

extern void *gOverlay45ResourceHead;

/* Pinned DKR v77/v80 and JFG scans contain no exact donor. */

void overlay45ResetState(void) {
    gOverlay45ResourceHead = NULL;
}
