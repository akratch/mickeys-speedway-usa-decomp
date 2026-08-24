#include "PR/ultratypes.h"

typedef struct Overlay68TimerEntry {
    s32 active;
    u8 pad4;
    s8 timer;
} Overlay68TimerEntry;
extern Overlay68TimerEntry *gOverlay68TimerEntry;

void overlay68StartTimer(void) {
    Overlay68TimerEntry *entry = gOverlay68TimerEntry;

    if (entry != 0 && entry->active != 0 && entry->timer == 0) {
        entry->timer = 120;
    }
}
