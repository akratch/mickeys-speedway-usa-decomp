#include "PR/ultratypes.h"

typedef struct {
    u8 first;
    s8 second;
} Overlay59Value;

typedef struct {
    u8 pad0[8];
    Overlay59Value values[8];
    s16 count;
    s16 field1A;
    s16 field1C;
    s16 state;
    u8 pad20[0x24];
} Overlay59Record;

extern Overlay59Record gOverlay59Entries[4];

/* DKR us.v77/us.v80 and JFG scans found no exact or semantic donor. */
void overlay59Update(s32 index) {
    Overlay59Record *record;
    s32 i;

    if (index < 0 || index >= 4) {
        return;
    }
    record = &gOverlay59Entries[index];
    if (record->state == 0 || record->state == 5) {
        return;
    }
    if (record->count != 0) {
        record->count--;
        record->field1C = 0;
        record->state = 5;
        if (record->count != 0) {
            for (i = 0; i < record->count; i++) {
                record->values[i].first = record->values[i + 1].first;
                record->values[i].second = record->values[i + 1].second;
            }
        }
    }
}
