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

/*
 * NON_MATCHING: the best coherent candidate is size-exact at 0xB0 and
 * matches 32/44 instruction words with the default flags. Its first mismatch
 * is +0x14: IDO schedules the shadow count into a1 before saving ra and keeps
 * that color through both countdown loops. A two-worker, ten-minute permuter
 * run plateaued at score 165 without resolving the remaining 12 words. This
 * run's 119-combination flag lattice and coherent shadow-lifetime/loop-shape
 * variants preserved the same size-exact 12-word residual.
 */
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
