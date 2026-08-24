#include "PR/ultratypes.h"

typedef struct Overlay7Owner {
    u8 pad00[0x64];
    s8 *priority;
} Overlay7Owner;

typedef struct Overlay7Entry {
    Overlay7Owner *owner;
    s32 field04;
    u16 value;
    u8 type;
    u8 active;
    struct Overlay7Entry *nested;
    struct Overlay7Entry *next;
} Overlay7Entry;

extern Overlay7Entry *gOverlay7ActiveHead;
extern Overlay7Entry *gOverlay7FreeHead;
extern Overlay7Entry *gOverlay7ActiveTail;
extern Overlay7Entry *gOverlay7Selected;
extern s32 gOverlay7PriorityThresholdReloc;

void overlay7ReleaseEntry(Overlay7Entry *entry) {
    Overlay7Entry *previous;
    Overlay7Entry *current;

    if (gOverlay7ActiveHead != 0 && entry != 0) {
        entry->active = 0;
        if (entry == gOverlay7Selected) {
            gOverlay7Selected = 0;
        }

        previous = 0;
        current = gOverlay7ActiveHead;
        if (gOverlay7ActiveHead != entry) {
            do {
                previous = current;
                current = current->next;
            } while (current != entry);
        }

        if (previous != 0) {
            previous->next = current->next;
        }
        if (current == gOverlay7ActiveTail) {
            gOverlay7ActiveTail = previous;
        }
        if (current == gOverlay7ActiveHead) {
            gOverlay7ActiveHead = current->next;
        }
        current->next = gOverlay7FreeHead;
        gOverlay7FreeHead = current;
    }
}

Overlay7Entry *overlay7AcquireEntry(Overlay7Owner *owner, u16 value, u8 type) {
    Overlay7Entry *head;
    Overlay7Entry *entry;
    Overlay7Entry *result;
    s8 *ownerPriority;

    head = gOverlay7ActiveHead;
    entry = head;
    if (entry != 0) {
        do {
            if (entry->active != 0 && entry->owner == owner &&
                entry->value == value) {
                return 0;
            }
            entry = entry->next;
        } while (entry != 0);
        entry = head;
    }

    ownerPriority = owner->priority;
    if (entry != 0) {
        do {
            if (entry->active != 0 &&
                ((entry->type < type && entry->owner == owner) ||
                 (*ownerPriority < gOverlay7PriorityThresholdReloc &&
                  type >= entry->type))) {
                if (entry->nested != 0) {
                    entry->nested->active = 1;
                }
                overlay7ReleaseEntry(entry);
            }
            entry = entry->next;
        } while (entry != 0);
    }

    result = gOverlay7FreeHead;
    if (result != 0) {
        entry = result;
        gOverlay7FreeHead = result->next;
        if (gOverlay7ActiveTail != 0) {
            gOverlay7ActiveTail->next = result;
            gOverlay7ActiveTail = result;
        } else {
            gOverlay7ActiveHead = entry;
        }
        entry->next = 0;
        gOverlay7ActiveTail = entry;
    }
    return entry;
}
