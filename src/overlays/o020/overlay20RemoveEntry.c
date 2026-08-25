#include "PR/ultratypes.h"

typedef struct Overlay20RemoveOwner {
    u8 pad0[0x84];
    void *entry;
} Overlay20RemoveOwner;

extern void *gOverlay20Entries[];
extern void *gOverlay20ShiftEntries[];
extern s32 gOverlay20EntryCount;
extern u8 gOverlay20MarkerEnd;
extern u32 gOverlay20ActiveBits;

/* DKR v77/v80 and JFG have no exact donor; only generic list compaction. */
/* Current-run plateau (2026-08-25): 119 flags, 10 forms, and a 40-minute
 * permuter (best 75) leave exact 0xD4, 18 register-only words, first +0x4C;
 * signed search is exact, blocked on newCount v0 vs t7 and compaction colors. */
#ifdef NON_MATCHING
void overlay20RemoveEntry(s32 owner) {
    void *entry;
    s32 i;
    s32 newCount;
    void **cursor;
    void **end;

    entry = ((Overlay20RemoveOwner *)owner)->entry;
    if (entry == NULL) {
        return;
    }
    owner = gOverlay20EntryCount;
    i = 0;
    cursor = gOverlay20Entries;
    if (owner > 0) {
        do {
            if (entry == *cursor) {
                break;
            }
            i++;
            cursor++;
            if (i < owner) {
                continue;
            }
            break;
        } while (1);
    }
    if (i >= owner) {
        return;
    }
    newCount = owner - 1;
    gOverlay20EntryCount = newCount;
    if (i < newCount) {
        cursor = &gOverlay20ShiftEntries[i];
        end = &gOverlay20ShiftEntries[newCount];
        do {
            *cursor = cursor[1];
            cursor++;
        } while (cursor < end);
    }

    owner = (s32)&gOverlay20MarkerEnd;
    i = 31;
    do {
        if (owner != 0) {
            gOverlay20ActiveBits &= ~(1U << i);
            return;
        }
        owner -= 0x24;
    } while (i--);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o020/overlay20RemoveEntry/func_overlay_020_F0001018_18775F0.s")
#endif
