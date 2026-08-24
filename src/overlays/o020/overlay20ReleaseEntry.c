#include "PR/ultratypes.h"

typedef struct Overlay20ReleaseOwner {
    u8 pad0[0x84];
    s32 entry;
    s32 marked;
} Overlay20ReleaseOwner;

extern void overlay20ReleaseReloc(void *);

/* DKR v77/v80 and JFG have no exact donor for this guarded release. */
void overlay20ReleaseEntry(Overlay20ReleaseOwner *owner) {
    if (owner->entry != 0 && owner->marked != 0) {
        overlay20ReleaseReloc((void *)owner->entry);
    }
    owner->marked = 0;
}
