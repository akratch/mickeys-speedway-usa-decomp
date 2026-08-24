#include "PR/ultratypes.h"

typedef struct Overlay17Resource {
    u8 pad0[4];
    void *nested;
} Overlay17Resource;

extern void overlay17ReleaseReloc(void *resource);

/* DKR v77/v80 and JFG checks found only generic nested-release semantics. */
void overlay17ReleaseChain(Overlay17Resource *resource) {
    if (resource != NULL) {
        if (resource->nested != NULL) {
            overlay17ReleaseReloc(resource->nested);
        }
        overlay17ReleaseReloc(resource);
    }
}
