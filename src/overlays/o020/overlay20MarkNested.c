#include "PR/ultratypes.h"

typedef struct Overlay20NestedOwner {
    u8 pad00[0x84];
    void *nested;
    s32 marked;
} Overlay20NestedOwner;

/* DKR v77/v80 and JFG have no exact donor for this guarded state setter. */
void overlay20MarkNested(Overlay20NestedOwner *owner) {
    if (owner->nested != NULL) {
        owner->marked = 1;
    }
}
