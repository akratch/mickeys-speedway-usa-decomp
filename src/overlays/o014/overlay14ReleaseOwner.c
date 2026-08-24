#include "PR/ultratypes.h"

typedef struct Overlay14Owner {
    u8 pad00[0x84];
    s32 released;
} Overlay14Owner;

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern Overlay14Owner *gOverlay14Owner;

void overlay14ReleaseOwner(void) {
    Overlay14Owner *owner;

    owner = gOverlay14Owner;
    if (owner != NULL) {
        owner->released = 1;
        gOverlay14Owner = NULL;
    }
}
