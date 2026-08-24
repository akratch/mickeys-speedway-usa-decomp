#include "PR/ultratypes.h"

typedef struct Overlay1SearchRecord {
    u8 pad00[0x44];
    s16 type;
    u8 pad46[0x3E];
    s32 key;
} Overlay1SearchRecord;

extern Overlay1SearchRecord **overlay1SearchRangeReloc(s32 *start, s32 *end);

/* The pinned DKR v77/v80 and JFG object scans contain no exact donor. */
#ifdef NON_MATCHING
Overlay1SearchRecord *overlay1FindType5ByKey(const s8 *key) {
    s32 start;
    s32 end;
    s32 wantedKey;
    Overlay1SearchRecord *record;
    Overlay1SearchRecord **cursor;

    cursor = overlay1SearchRangeReloc(&start, &end) + start;
    if (start < end) {
        do {
            record = *cursor++;
            start++;
            if (record->type == 5) {
                wantedKey = *key;
                if (wantedKey == record->key) {
                    return record;
                }
            }
        } while (start < end);
    }
    return NULL;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1FindType5ByKey/func_overlay_001_F0000378_184C758.s")
#endif
