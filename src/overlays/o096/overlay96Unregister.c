#include "PR/ultratypes.h"

extern s32 gO96EntryCountReloc;
extern s32 gO96EntriesReloc[16];

/*
 * Plateau (2026-08-25, refreshed): a 119-combination flag sweep found no
 * exact result. This reverse-loop spelling reaches the 0x88-byte extent but
 * differs in 27 positional words (17 opcode shapes), first at +0x0, because
 * IDO does not retain the target's count pointer/index allocation. A bounded
 * 10-minute permuter improved its own score from 2170 to 895, but the winning
 * redundant assignments remain non-exact and are not suitable source.
 */
#ifdef NON_MATCHING
void overlay96Unregister(s32 value) {
    s32 shifted;
    s32 newCount;
    s32 count;
    s32 index;
    s32 *entry;

    count = gO96EntryCountReloc;
    index = count;
    if (count != 0) {
        index--;
        entry = &gO96EntriesReloc[index];
    loop:
        newCount = count - 1;
        if (value == *entry) {
            gO96EntryCountReloc = newCount;
            if (index < newCount) {
                do {
                    shifted = entry[1];
                    entry++;
                    entry[-1] = shifted;
                } while (entry < &gO96EntriesReloc[newCount]);
            }
            return;
        }
        entry--;
        if (index-- != 0) {
            goto loop;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o096/overlay96Unregister/func_overlay_096_F0000070_18D76A8.s")
#endif
