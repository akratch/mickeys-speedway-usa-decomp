#include "PR/ultratypes.h"

extern void *gOverlay15Resource10;
extern void overlay15ReleaseReloc(void *resource);

void overlay15ReleaseResource10(void) {
    if (gOverlay15Resource10 != 0) {
        overlay15ReleaseReloc(gOverlay15Resource10);
        gOverlay15Resource10 = 0;
    }
}
