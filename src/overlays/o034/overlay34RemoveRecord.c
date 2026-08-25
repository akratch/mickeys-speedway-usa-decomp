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

/* Plateau: 32/44 words exact; 12 schedule/register words remain, first +0x14.
 * Shadow assignment, post-decrement, declaration, and four-argument call
 * shapes either retained the residual or introduced an s0 spill/larger frame. */
#ifdef NON_MATCHING
void overlay34RemoveRecord(Overlay34Record *record) {
    s32 shadow;
    s32 remaining;
    Overlay34Record **slot;

    slot = gOverlay34Pointers;
    remaining = gOverlay34ActiveCount;
    shadow = remaining;
    if (remaining != 0) {
        remaining--;
        do {
            if (*slot == record) {
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
            slot++;
            slot--;
        } while (remaining--);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o034/overlay34RemoveRecord/func_overlay_034_F00002C8_1881470.s")
#endif
