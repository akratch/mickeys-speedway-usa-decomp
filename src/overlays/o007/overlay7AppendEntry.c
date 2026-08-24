#include "PR/ultratypes.h"

typedef struct Overlay7Entry {
    void *owner;
    s32 field4;
    u16 value;
    u8 type;
    u8 active;
    struct Overlay7Entry *nested;
} Overlay7Entry;

/* Pinned DKR v77/v80 and JFG scans found no exact donor. */
extern Overlay7Entry *gOverlay7Current;
extern Overlay7Entry *overlay7Acquire(void *owner, u16 value, u8 type);
extern void overlay7CreateEntry(void *owner, u16 value, u8 type);

void overlay7AppendEntry(void *owner, u16 value, u8 type) {
    Overlay7Entry *entry;
    Overlay7Entry *current;

    entry = overlay7Acquire(owner, value, type);
    current = gOverlay7Current;
    if (current == 0) {
        overlay7CreateEntry(owner, value, type);
    } else {
        current->nested = entry;
        if (entry != 0) {
            entry->owner = owner;
            entry->field4 = 0;
            entry->value = value;
            entry->type = type;
            entry->nested = 0;
            entry->active = 2;
        }
    }
}
