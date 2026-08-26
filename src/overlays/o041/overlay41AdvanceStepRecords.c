#include "PR/ultratypes.h"

typedef struct Overlay41StepRecord {
    u8 id;
    u8 residual;
    s16 remaining;
    s32 x;
    s32 dx;
    s32 y;
    s32 dy;
} Overlay41StepRecord;

extern Overlay41StepRecord gOverlay41StepRecords[8];
extern void overlay41EmitStep(s32 id, s32 x, s32 y);

/* Workbench: allocation-mismatch, exact 73/-48 shape, 26 register words from +0x54; temp phase +1.
 * Lever: temp-FIFO structure/lifetime/width/scope probes left the ring topology unchanged.
 * Remains: one temp-ring pop/death and three overlay relocation identities; assembly fallback stays canonical. */
#ifdef NON_MATCHING
void func_overlay_041_F0000000_1887338(s32 amount) {
    Overlay41StepRecord *record;
    s32 i;
    s32 x;
    s32 y;
    s32 step;

    record = gOverlay41StepRecords;
    i = 7;
    do {
        if (record->id != 0xFF) {
            step = record->remaining;
            if (step != 0) {
                if (amount < step) {
                    step = amount;
                }
                record->remaining -= step;
                record->x += record->dx * step;
                record->y += record->dy * step;
            }
            x = ((record->x * amount) >> 8) + (record->residual & 0xF);
            y = ((record->y * amount) >> 8) + (record->residual >> 4);
            overlay41EmitStep(record->id, x >> 3, y >> 3);
            record->residual = x & 7;
            record->residual |= (y & 7) << 4;
        }
        record++;
    } while (i--);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o041/overlay41AdvanceStepRecords/func_overlay_041_F0000000_1887338.s")
#endif
