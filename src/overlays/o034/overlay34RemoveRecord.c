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
extern void *func_overlay_034_F0000000_18811A8();

/* Plateau (2026-08-28): the size-exact 44-word body matches 32 words, with
 * the first schedule/register residual at +0x14.  The target has one helper
 * call and two repeated active-count HI16/LO16 pairs; its pointer load is
 * encoded without a relocation, while the source-preserving candidate keeps
 * both pointer relocations.  Shadow, post-decrement, declaration, indexed
 * compaction, cursor, and scoped-call variants either retained the residual
 * or introduced a size/schedule regression. */
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
                 * The target helper consumes resourceId; retail keeps this
                 * logically-zero second argument live in a1.
                 */
                if (record->resourceId != 0) {
                    func_overlay_034_F0000000_18811A8(record->resourceId, shadow);
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
