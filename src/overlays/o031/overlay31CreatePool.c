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
 * Plateau (2026-08-25, independently rechecked): the best 50-word candidate
 * has exact size, control flow, and linked relocation surface at 98.42%
 * objdiff. The first mismatch is +0x0: the target frame is 0x38 bytes versus
 * 0x30, followed by one pointer/counter register family (a1/a2 versus a0/a1).
 * The full flag lattice was neutral. Named size temporaries, register
 * qualifiers, and local-lifetime changes were codegen-inert; an extra formal
 * argument worsened the result to +4 bytes/47 differing words and contradicts
 * the caller ABI. The remaining code-free dead-web lever would be a fabricated
 * register-control construct, so it was rejected.
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
