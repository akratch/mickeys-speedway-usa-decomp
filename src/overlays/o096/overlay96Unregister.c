#include "PR/ultratypes.h"

extern s32 gO96EntryCountReloc;
extern s32 gO96EntriesReloc[16];

/*
 * Plateau (2026-08-25): canonical -O2 -mips2 emits 0x7C bytes for the
 * 0x88-byte target and first diverges at +0x8. The flag lattice and a
 * 10-minute permuter run (best score 715, but not semantics-preserving)
 * did not reproduce the target's count/index allocation and reverse loop.
 */
#ifdef NON_MATCHING
void overlay96Unregister(s32 value) {
    s32 count;
    s32 index;
    s32 *entry;
    s32 *end;

    count = gO96EntryCountReloc;
    index = count;
    if (index != 0) {
        index--;
        entry = &gO96EntriesReloc[index];
        do {
            if (value == *entry) {
                count--;
                gO96EntryCountReloc = count;
                if (index < count) {
                    end = &gO96EntriesReloc[count];
                    do {
                        *entry = entry[1];
                        entry++;
                    } while (entry < end);
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
