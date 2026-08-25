#include "PR/ultratypes.h"

typedef struct Overlay3TouchEntry {
    void *object;
    s32 timer;
} Overlay3TouchEntry;

extern void *gOverlay3Objects[];
extern Overlay3TouchEntry gOverlay3TouchEntries[][32];

/* DKR v77/v80 and JFG contain no exact donor for this fixed object registry. */
/*
 * Plateau: the 34-word candidate has the exact size, CFG, allocation, and
 * relocation shape, with one differing word at +0x34: IDO commutes the two
 * operands of the first object comparison. The flag lattice was neutral;
 * extending base's lifetime (found by the bounded permuter, score 110 -> 30)
 * closed the other 12 masked differences. Reversed equality, explicit
 * inequality/goto, integer/subtraction forms, volatile access, register
 * hints, and alternate base lifetimes did not close the final operand order.
 */
#ifdef NON_MATCHING
void overlay3TouchObject(s32 group, s32 remaining) {
    void *object = gOverlay3Objects[group];
    Overlay3TouchEntry *base;
    Overlay3TouchEntry *entry;

    remaining = 31;
    if (object == 0) {
        return;
    }
    base = gOverlay3TouchEntries[group];
    entry = &base[remaining];
    do {
        if (entry->object == object) {
            entry->timer = 300;
            return;
        }
        entry--;
    } while (remaining--);

    remaining = 31;
    base += 31;
    entry = base;
    do {
        if (entry->object == 0) {
            entry->object = object;
            entry->timer = 300;
            return;
        }
        entry--;
    } while (remaining--);
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o003/overlay3TouchObject/func_overlay_003_F00006D8_185A408.s")
#endif
