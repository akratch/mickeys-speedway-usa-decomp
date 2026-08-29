#include "PR/ultratypes.h"

typedef struct Overlay34Record {
    u8 pad00[0x3C];
    u8 timer;
    u8 lifetime;
    u8 active;
    u8 pad3F;
    f32 x;
    f32 y;
    f32 z;
    f32 previousX;
    f32 previousY;
    f32 previousZ;
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    f32 value;
} Overlay34Record;

extern Overlay34Record **gOverlay34Pointers;
extern s32 gOverlay34ActiveCount;
extern f32 gOverlay34Value10;
extern void overlay34RemoveRecord(Overlay34Record *record);

/* Public-claim reproof: untouched IDO emits 80 instructions / 0x140 bytes
 * with a 0x38 frame versus the retail owner's 77 / 0x134 and 0x30 frame.
 * All 119 flag configurations are nonexact. The retired wrapper reached
 * equality only by deleting three instructions, reordering instructions,
 * and editing register/immediate fields, so the fallback remains canonical. */
#ifdef NON_MATCHING
void overlay34UpdateRecords(s32 updateRate) {
    Overlay34Record *record;
    s32 index;
    s32 offset;
    s32 remaining;
    u8 timer;
    f32 velocityY;

    if (updateRate != 0) {
        index = 0;
        offset = 0;
        if (gOverlay34ActiveCount > 0) {
            do {
                index++;
                remaining = updateRate - 1;
                record = *(Overlay34Record **)((u8 *)gOverlay34Pointers + offset);
                offset += 4;
                timer = record->timer + updateRate;
                record->timer = timer;
                if ((timer & 0xFF) < record->lifetime) {
                    record->previousX = record->x;
                    record->previousY = record->y;
                    record->previousZ = record->z;
                    if (updateRate != 0) {
                        do {
                            record->x += record->velocityX;
                            velocityY = record->velocityY;
                            record->y += record->velocityY;
                            record->z += record->velocityZ;
                            record->velocityY = velocityY - gOverlay34Value10;
                        } while (remaining--);
                    }
                } else {
                    overlay34RemoveRecord(record);
                    index--;
                    offset -= 4;
                }
            } while (index < gOverlay34ActiveCount);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o034/overlay34UpdateRecords/func_overlay_034_F000040C_18815B4.s")
#endif
