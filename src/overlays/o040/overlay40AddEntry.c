#include "PR/ultratypes.h"

typedef struct Overlay40Entry {
    s8 state;
    s8 scales[3];
    u8 red;
    u8 green;
    u8 blue;
    u8 id;
} Overlay40Entry;

extern s32 gOverlay40Count;
extern Overlay40Entry gOverlay40Entries[8];

/*
 * Plateau: correct 0x84-byte shape at -O2 -mips2, with four register-only
 * differences starting at +0x20. The flag lattice and a bounded permuter run
 * did not change the a0/v1 pool-color swap around the loop counter.
 */
#ifdef NON_MATCHING
void overlay40AddEntry(volatile s32 id, s32 red, s32 green, s32 blue) {
    Overlay40Entry *entry;
    s32 remaining;

    entry = gOverlay40Entries;
    if (gOverlay40Count < 8) {
        remaining = 7;
        do {
            if (entry->state == -1) {
                entry->state = 0;
                entry->scales[0] = 0x1E;
                entry->scales[1] = 0x1E;
                entry->scales[2] = 0x1E;
                entry->red = red;
                entry->green = green;
                entry->blue = blue;
                entry->id = id;
                gOverlay40Count++;
                return;
            }
            entry++;
        } while (remaining--);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o040/overlay40AddEntry/func_overlay_040_F0000000_18868B0.s")
#endif
