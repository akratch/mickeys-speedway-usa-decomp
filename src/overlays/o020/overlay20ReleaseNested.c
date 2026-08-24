#include "PR/ultratypes.h"

typedef struct Overlay20NestedOwner {
    u8 pad00[0x84];
    void * volatile nested;
} Overlay20NestedOwner;

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern void overlay20ReleaseReloc(void *nested);

s32 overlay20ReleaseNested(Overlay20NestedOwner *owner) {
    void *nested;

    nested = owner->nested;
    if (nested != 0) {
        overlay20ReleaseReloc(nested);
    }
}
