#include "PR/ultratypes.h"

typedef struct Overlay31PoolRecord {
    s32 state[16];
    u8 reserved40[0x64];
    u8 active;
    u8 reservedA5[0x1B];
} Overlay31PoolRecord;

extern void *overlay31AllocateReloc(s32 size, s32 tag);
extern void *overlay31CreateSlotTable(s32 kind, s32 flags, s32 width,
                                      s32 height, s32 slotCount);
extern void *gOverlay31SlotTable;

/* DKR v77/v80 and JFG contain no exact donor for this pool allocator. */
/*
 * Plateau (2026-08-25): the best 50-word candidate has exact size, control
 * flow, and relocation surface at 98.42% objdiff.  The first non-relocation
 * mismatch is +0x0 (frame allocation); the rest are one pointer/counter
 * register family.  The flag lattice was neutral.  A bounded ten-minute
 * permuter run reduced cost 85 to 20 only by adding a redundant empty pointer
 * guard, which was rejected; legitimate loop and lifetime spellings did not
 * close the allocation difference.
 */
#ifdef NON_MATCHING
Overlay31PoolRecord *overlay31CreatePool(s32 count) {
    Overlay31PoolRecord *records;
    Overlay31PoolRecord *record;
    s32 *state;
    s32 i;
    s32 j;

    records = overlay31AllocateReloc(count * sizeof(Overlay31PoolRecord), 0x8C);
    record = records;

    i = 0;
    if (count > 0) {
        do {
            record->state[15] = 0;
            record->active = 0;
            record->state[0] = 0;
            record->state[1] = 0;
            record->state[2] = 0;

            j = 3;
            state = &record->state[3];
            do {
                j += 4;
                state[1] = 0;
                state[2] = 0;
                state[3] = 0;
                state += 4;
                state[-4] = 0;
            } while (j != 15);
            i++;
            record++;
        } while (i != count);
    }

    gOverlay31SlotTable = overlay31CreateSlotTable(0, 0, 0, 0, count * 15);
    return records;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o031/overlay31CreatePool/func_overlay_031_F0000E7C_188039C.s")
#endif
