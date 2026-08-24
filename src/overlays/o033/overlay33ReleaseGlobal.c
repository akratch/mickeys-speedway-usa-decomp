#include "PR/ultratypes.h"

extern s32 gOverlay33Handle;

void overlay33ReleaseReloc(void *handle);

/* DKR v77/v80 has the generic optional-release idiom, but no exact donor. */
void overlay33ReleaseGlobal(void) {
    if (gOverlay33Handle != 0) {
        overlay33ReleaseReloc((void *)gOverlay33Handle);
        gOverlay33Handle = 0;
    }
}
