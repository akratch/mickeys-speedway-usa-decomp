#include "PR/ultratypes.h"

extern s32 gO96EntryCountReloc;
extern s32 gO96EntriesReloc[16];

/*
 * Plateau (2026-08-25, refreshed in this lane): the nearest skeleton is only
 * a semantic fixed-list removal relative; this body is reconstructed from
 * Mickey's reverse walk. A fresh 119-combination flag sweep selects
 * -O2 -g3 -mips2 and reaches the exact 0x88-byte extent with 24 of 34 masked
 * words different, first at +0x0 (the ordinary overlay flags differ in 25).
 * Prefix/post-decrement, pointer, integer-address, volatile, and explicit-end
 * loop forms do not reproduce the target's cached count-base/index lifetime.
 */
#ifdef NON_MATCHING
void overlay96Unregister(s32 value) {
    s32 count;
    s32 index;
    s32 *entry;
    s32 *countPointer;

    countPointer = &gO96EntryCountReloc;
    count = *countPointer;
    index = count;
    if (index != 0) {
        index--;
        entry = &gO96EntriesReloc[index];
        do {
            if (value == *entry) {
                *countPointer = count - 1;
                if (index < count - 1) {
                    do {
                        *entry = entry[1];
                        entry++;
                    } while (entry < &gO96EntriesReloc[count - 1]);
                }
                return;
            }
            entry--;
        } while (index-- != 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o096/overlay96Unregister/func_overlay_096_F0000070_18D76A8.s")
#endif
