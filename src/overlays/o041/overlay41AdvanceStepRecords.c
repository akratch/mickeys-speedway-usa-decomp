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

/* Workbench: register-ring-only, 26 register words, first +0x54; phase +1.
 * Condition/polarity/lifetime levers were inert; s16 step added four instructions.
 * Remaining: one temp-pop/death before the inner record-update block. */
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
