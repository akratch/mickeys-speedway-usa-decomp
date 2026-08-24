#include "PR/ultratypes.h"

typedef struct Overlay34Record {
    u8 pad00[0x20];
    s32 resourceId;
    u8 pad24[0x1A];
    u8 active;
    u8 pad3F[0x29];
} Overlay34Record;

extern Overlay34Record **gOverlay34Pointers;
extern s32 gOverlay34ActiveCount;
extern void *func_80034448();

void overlay34RemoveRecord(Overlay34Record *record) {
    Overlay34Record **slot;
    s32 remaining;
    s32 shadow;

    remaining = gOverlay34ActiveCount;
    slot = gOverlay34Pointers;
    shadow = remaining;
    if (remaining != 0) {
        remaining--;
        do {
            shadow = remaining;
            if (*slot == record) {
                shadow = remaining;
                if (remaining != 0) {
                    remaining--;
                    do {
                        *slot = slot[1];
                        shadow = remaining;
                        slot++;
                    } while (remaining--);
                }
                /*
                 * The resident loader consumes only resourceId; retail keeps
                 * this logically-zero second argument live in a1.
                 */
                if (record->resourceId != 0) {
                    func_80034448(record->resourceId, shadow);
                }
                record->active = 0;
                gOverlay34ActiveCount--;
                return;
            }
            slot++;
        } while (remaining--);
    }
}
