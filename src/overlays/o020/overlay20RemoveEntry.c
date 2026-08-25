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
/*
 * Plateau (2026-08-25, 10 attempts plus a bounded permuter batch): the
 * natural -O2 candidate is one word short and first diverges at +0xC because
 * IDO folds the search backedge's signed comparison into equality.  The best
 * scratch candidate restores the exact size, agrees through +0x48, and
 * differs in 17 of 53 words from +0x4C; its redundant boolean no-op is not a
 * source-level explanation, and the remaining gap is the list-compaction
 * temporary/register layout.  The full flag lattice found no better group.
 */
/*
 * Current-run plateau (2026-08-25): a fresh 119-group flag sweep confirmed
 * the natural -O2 -mips2 body is four bytes short, with 42 of 53 masked words
 * different from +0xC.  The nearest DKR boolean-search spelling retained an
 * explicit found-state register.  A two-worker, ten-minute permuter batch
 * reached score 175 only through a non-idiomatic i+1 bounds guard and still
 * left the compaction register web different from +0x4C.
 */
#ifdef NON_MATCHING
void overlay20RemoveEntry(s32 owner) {
    void *entry;
    s32 i;
    void **cursor;
    void **end;

    entry = ((Overlay20RemoveOwner *)owner)->entry;
    if (entry == NULL) {
        return;
    }
    owner = gOverlay20EntryCount;
    i = 0;
    cursor = gOverlay20Entries;
    while (i < owner) {
        if (entry == *cursor) {
            break;
        }
        i++;
        cursor++;
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
