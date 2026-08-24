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
Overlay31PoolRecord *overlay31CreatePool(s32 count) {
    Overlay31PoolRecord *records;
    Overlay31PoolRecord *record;
    s32 *state;
    s32 i;
    s32 j;

    records = overlay31AllocateReloc(count * sizeof(Overlay31PoolRecord), 0x8C);
    record = records;

    for (i = 0; i < count; i++) {
        record->state[15] = 0;
        record->active = 0;
        record->state[0] = 0;
        record->state[1] = 0;
        record->state[2] = 0;

        state = &record->state[3];
        for (j = 3; j != 15; j += 4) {
            state[1] = 0;
            state[2] = 0;
            state[3] = 0;
            state += 4;
            state[-4] = 0;
        }
        record++;
    }

    gOverlay31SlotTable = overlay31CreateSlotTable(0, 0, 0, 0, count * 15);
    return records;
}
