#include "PR/ultratypes.h"

typedef struct Overlay68Entry {
    s32 active;
    u8 pad4[6];
    s16 generation;
} Overlay68Entry;
extern Overlay68Entry *gOverlay68Entry;

void overlay68FinishEntry(void) {
    Overlay68Entry *entry = gOverlay68Entry;

    if (entry != 0 && entry->active != 0) {
        entry->generation++;
        entry->active = 0;
    }
}
