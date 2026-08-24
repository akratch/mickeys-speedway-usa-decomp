#include "PR/ultratypes.h"

/* DKR v77 object and semantic-source searches found no exact donor. */
extern s32 gOverlay75SlotFlags[];

void overlay75MarkSlot(s32 slot) {
    if (slot < 5) {
        gOverlay75SlotFlags[slot] = 1;
    }
}
