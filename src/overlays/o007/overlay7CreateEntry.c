#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG exact-object scans are negative for this allocator. */
typedef struct Overlay7Entry {
    void *owner;
    s32 field4;
    u16 value;
    u8 type;
    u8 active;
    s32 nested;
} Overlay7Entry;

extern Overlay7Entry *gOverlay7Current;
extern Overlay7Entry *overlay7Acquire(void *owner, u16 value, u8 type);

void overlay7CreateEntry(void *owner, u16 value, u8 type) {
    Overlay7Entry *entry;

    entry = overlay7Acquire(owner, value, type);
    if (entry == NULL) {
        gOverlay7Current = NULL;
    } else {
        gOverlay7Current = entry;
        entry->owner = owner;
        entry->field4 = 0;
        entry->value = value;
        entry->type = type;
        entry->nested = 0;
        entry->active = 1;
    }
}
