#include "PR/ultratypes.h"

typedef struct Overlay1BestRecord {
    u8 pad00[0xC0];
    union {
        u16 flags;
        struct {
            u8 high;
            u8 type;
        } bytes;
    } header;
    u8 padC2[6];
    u32 value;
} Overlay1BestRecord;

extern Overlay1BestRecord gOverlay1BestRecords[32];
extern s32 gOverlay1SelectedType;

/* DKR v77/v80 and JFG have no exact donor for this fixed record scan. */
#ifdef NON_MATCHING
Overlay1BestRecord *overlay1FindBestRecord(void) {
    Overlay1BestRecord *record;
    Overlay1BestRecord *result;
    u32 bestValue;
    register u32 value;
    s32 remaining;
    register s32 selectedType;

    record = gOverlay1BestRecords;
    bestValue = (u32)-1;
    result = NULL;
    selectedType = gOverlay1SelectedType;
    remaining = 31;
    do {
        if (selectedType == ((u32)record->header.bytes.type >> 2)) {
            value = record->value;
            if ((value == 0) ||
                (((record->header.flags & 3) == 3) &&
                 (value < bestValue))) {
                bestValue = value;
                result = record;
            }
        }
        record++;
    } while (remaining--);
    return result;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1FindBestRecord/func_overlay_001_F0007B64_1853F44.s")
#endif
