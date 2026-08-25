#include "PR/ultratypes.h"

extern s32 gO96EntryCountReloc;
extern s32 gO96EntriesReloc[16];

/* Plateau (near-miss batch 13): -O2 -g3 -mips2 is size-exact; 24/34 words
 * differ, first +0x0. Pointer/index/lifetime forms tied or regressed; the
 * 40-minute permuter retained nonzero score 1150, leaving the cached web. */
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
