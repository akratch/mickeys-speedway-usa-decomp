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

/* PROVENANCE: JFG's public assembly-only
 * src/overlays/o10/overlay_10.c:sparkUpdate supplied structural context for
 * the shared update/remove scaffold. This body and its field layout were
 * reconstructed and matched against Mickey's own bytes. */
void overlay34UpdateRecords(s32 updateRate) {
    Overlay34Record *record;
    s32 index;
    s32 offset;
    u8 timer;
    f32 velocityY;

    if (updateRate != 0) {
        index = 0;
        offset = 0;
        if (gOverlay34ActiveCount > 0) {
            do {
                index++;
                record = *(Overlay34Record **)((u8 *)gOverlay34Pointers + offset);
                offset += 4;
                timer = record->timer + updateRate;
                record->timer = timer;
                if ((timer & 0xFF) < record->lifetime) {
                    s32 remaining = updateRate - 1;

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
