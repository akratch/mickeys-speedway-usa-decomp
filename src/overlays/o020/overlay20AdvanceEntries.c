#include "PR/ultratypes.h"

typedef struct Overlay20Entry {
    u8 pad0[0x18];
    s16 value;
    s16 velocity;
} Overlay20Entry;

extern Overlay20Entry *gOverlay20Entries[];
extern s32 gOverlay20EntryCount;

/* Exact at +0x10EC. DKR has only generic update-rate/velocity relatives. */
void overlay20AdvanceEntries(s32 amount) {
    s32 i;
    Overlay20Entry *current;

    i = 0;
    for (; i < gOverlay20EntryCount; i++) {
        current = gOverlay20Entries[i];
        current->value += current->velocity * amount;
    }
}
