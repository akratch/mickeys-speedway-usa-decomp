#include "overlays/overlay_007.h"

/* Overlay 7, ADR 0006 consolidation: C before the middle assembly island. */

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
        if (current != entry) {
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

/*
 * Plateau: exact size with 17 differing words, first at +0x84. IDO retains
 * the reset head in v0 for the second-loop null check instead of using the
 * target's s0 web; the remaining allocation-tail register/layout differences
 * follow from that copy-propagation choice. The O2 flag lattice is unchanged.
 */
#ifdef NON_MATCHING
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
    }

    entry = head;
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
    } else {
        entry = 0;
    }
    return entry;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o007/overlay_007/func_overlay_007_F00000A8_185BF30.s")
#endif

/* DKR v77/v80 and JFG exact-object scans are negative for this allocator. */
void overlay7CreateEntry(void *owner, u16 value, u8 type) {
    Overlay7Entry *entry;

    entry = overlay7Acquire(owner, value, type);
    if (entry == NULL) {
        gOverlay7Current = NULL;
    } else {
        gOverlay7Current = entry;
        entry->owner = owner;
        entry->field04 = 0;
        entry->value = value;
        entry->type = type;
        entry->nested = 0;
        entry->active = 1;
    }
}

/* Pinned DKR v77/v80 and JFG scans found no exact donor. */
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
            entry->field04 = 0;
            entry->value = value;
            entry->type = type;
            entry->nested = 0;
            entry->active = 2;
        }
    }
}
