#include "PR/ultratypes.h"

typedef struct Overlay1ValueEntry {
    s16 keyA;
    s16 keyB;
    u32 value;
} Overlay1ValueEntry;

extern Overlay1ValueEntry gOverlay1ValueCache[64];

#ifdef NON_MATCHING
s32 overlay1UpdateValueCache(s16 keyA, s16 keyB, f32 value) {
    register s32 searchKeyA = keyA;
    register s32 searchKeyB = keyB;
    Overlay1ValueEntry *entry;
    s32 remaining;

    entry = gOverlay1ValueCache;
    remaining = 0x3F;
    do {
        if ((entry->value != 0) && (searchKeyA == entry->keyA) &&
            (searchKeyB == entry->keyB)) {
            if (value < (f32)entry->value) {
                entry->value = (u32)value;
                return 1;
            }
            return 0;
        }
        entry++;
    } while (remaining--);

    entry = gOverlay1ValueCache;
    remaining = 0x3F;
    do {
        if (entry->value == 0) {
            entry->keyA = searchKeyA;
            entry->keyB = searchKeyB;
            entry->value = (u32)value;
            return 1;
        }
        entry++;
    } while (remaining--);

    return 0;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1UpdateValueCache/func_overlay_001_F00073A0_1853780.s")
#endif
