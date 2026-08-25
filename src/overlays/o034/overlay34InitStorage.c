#include "PR/ultratypes.h"

typedef struct Overlay34Record {
    u8 bytes[0x68];
} Overlay34Record;

extern Overlay34Record *gOverlay34Records;
extern s32 **gOverlay34Pointers;
extern s32 gOverlay34Count;
extern void *func_8002B280(s32 size, s32 tag);

/* Pinned DKR v77/v80 and JFG scans found no exact donor. */
/*
 * Plateau: exact 0xC8 size, 46/50 instruction words, first mismatch +0x24.
 * The target assigns the reused size spill to sp+0x1C; IDO assigns this body
 * to sp+0x18. Declaration, lifetime, expression, and type variants either
 * preserve those four differences or grow the frame. A 10-minute, two-worker
 * permuter run found no candidate below its baseline score of 40. This run's
 * full 119-combination flag lattice and typed scalar/aggregate layout checks
 * preserved the same four-word residual and +0x24 first mismatch.
 */
#ifdef NON_MATCHING
void overlay34InitStorage(s32 count) {
    s32 *word;
    s32 countdown;
    s32 remaining;
    s32 size;

    size = count * sizeof(Overlay34Record);
    gOverlay34Records = func_8002B280(size, 0x87);
    word = (s32 *)gOverlay34Records;
    remaining = size >> 2;
    countdown = remaining - 1;
    if (remaining != 0) {
        do {
            *word++ = 0;
        } while (countdown--);
    }

    size = count * sizeof(*gOverlay34Pointers);
    gOverlay34Pointers = func_8002B280(size, 0x87);
    word = (s32 *)gOverlay34Pointers;
    remaining = size >> 2;
    countdown = remaining - 1;
    if (remaining != 0) {
        do {
            *word++ = 0;
        } while (countdown--);
    }
    gOverlay34Count = count;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o034/overlay34InitStorage/func_overlay_034_F0000000_18811A8.s")
#endif
