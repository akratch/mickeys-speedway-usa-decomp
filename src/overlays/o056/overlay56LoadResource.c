#include "PR/ultratypes.h"

/* DKR v77/v80: generic asset-id sentinel patterns only; no semantic donor. */
typedef struct Overlay56Context {
    u8 pad0[0x12C];
    s16 resourceId;
} Overlay56Context;

extern void *gOverlay56Resource;
extern s16 gOverlay56ResourceState;
Overlay56Context *overlay56GetContextReloc(void);
void *overlay56LoadResourceReloc(s16 resourceId);

void overlay56LoadResource(void) {
    Overlay56Context *context;

    context = overlay56GetContextReloc();
    if (context->resourceId != -1) {
        gOverlay56Resource = overlay56LoadResourceReloc(context->resourceId);
    } else {
        gOverlay56Resource = 0;
    }
    gOverlay56ResourceState = 0;
}
