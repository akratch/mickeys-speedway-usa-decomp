#include "PR/ultratypes.h"

/* DKR v77/v80 semantic lead: video.c's one-byte mode setter; not an object donor. */
extern u8 gOverlay56Mode;

void overlay56SetMode(s32 mode) {
    gOverlay56Mode = mode;
}
