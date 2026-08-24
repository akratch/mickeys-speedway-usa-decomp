#include "PR/ultratypes.h"

typedef struct Overlay58Resource {
    u8 pad0[0x12];
    u8 count;
} Overlay58Resource;

extern void *gOverlay58CurrentResource;
extern s16 gOverlay58ResourceId;
extern Overlay58Resource gOverlay58TrackedResource;
extern s32 gOverlay58ResourceState;
extern s32 gOverlay58ResourceReady;
extern void *overlay58LoadResourceReloc(s16 id);

/* Pinned DKR v77/v80 and JFG scans found no matching donor. */
void overlay58EnsureResource(void) {
    if (gOverlay58CurrentResource == NULL) {
        gOverlay58CurrentResource = overlay58LoadResourceReloc(gOverlay58ResourceId);
        if (gOverlay58CurrentResource != NULL) {
            gOverlay58ResourceState = 2;
            gOverlay58TrackedResource.count++;
            gOverlay58ResourceReady = 1;
        } else {
            gOverlay58ResourceState = 0;
        }
    } else {
        gOverlay58ResourceState = 1;
    }
}
