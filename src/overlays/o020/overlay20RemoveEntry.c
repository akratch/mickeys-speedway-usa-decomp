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
#ifdef NON_MATCHING
void overlay20RemoveEntry(s32 owner) {
    void *entry;
    s32 i;
    s32 more;
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
searchLoop:
        if (entry != *cursor) {
            i++;
            more = i < owner;
            if (more) {
                cursor++;
                goto searchLoop;
            }
        }
    }
    if (i >= owner) {
        return;
    }
    owner--;
    gOverlay20EntryCount = owner;
    if (i < owner) {
        cursor = &gOverlay20ShiftEntries[i];
        end = &gOverlay20ShiftEntries[owner];
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
