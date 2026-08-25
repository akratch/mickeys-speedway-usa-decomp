#include "PR/ultratypes.h"

typedef struct Overlay13Record {
    u8 pad00[6]; u8 state; u8 timer;
    f32 scale, x, z, y, targetZ, velocityX, velocityZ, velocityY, phase;
    s32 vertexIndex;
    u8 vertices[0x140];
} Overlay13Record;

extern f32 gOverlay13Gravity;
extern s32 gOverlay13ActiveCount;
extern void overlay13Prepare(s32, s32, s32, Overlay13Record *);

/*
 * NON_MATCHING plateau (2026-08-25): the canonical -O2 -mips2 candidate
 * is exactly 0x284 bytes, with 65/161 words exact and the first mismatch at
 * +0x2C. A fresh ten-hypothesis pass in lane cx-ov-3-a-a-r3 merged the fade
 * loop's duplicate index into its post-decrement tick counter (also fixing
 * the zero/one-tick control shape) and merged the output/return pointer,
 * improving the previous 63-word result by two words. The full 119-flag
 * lattice found no exact configuration. An earlier bounded permuter reduced
 * its score from 5030 to 3815; result/constant allocation and the coupled
 * float-loop schedule remain the blocker.
 */
#ifdef NON_MATCHING
s16 *overlay13UpdateRecord(Overlay13Record *record, s32 ticks) {
    f32 radius;
    f32 gravity;
    f32 velocityZ;
    f32 targetZ;
    s32 y;
    s32 index;
    u8 state;
    u8 timer;
    s16 *result;

    overlay13Prepare(13, 50, 10, record);
    result = (s16 *)ticks;
    state = record->state;
    if (state == 1) {
        ticks--;
        if (ticks != 0) {
            gravity = gOverlay13Gravity;
            targetZ = record->targetZ;
loop_fall:
            velocityZ = record->velocityZ;
            record->z += velocityZ;
            record->x += record->velocityX;
            record->y += record->velocityY;
            record->velocityZ = velocityZ - gravity;
            if (record->z < targetZ) {
                record->z = targetZ;
                record->state = 2;
                state = 2;
            } else {
                ticks--;
                if (ticks == 0) {
                    state = record->state;
                } else {
                    goto loop_fall;
                }
            }
        }
    }

    if (state == 2) {
        if (ticks-- != 0) {
            timer = record->timer;
loop_fade:
            timer -= 2;
            record->timer = timer;
            record->phase += 0.5f;
            if (timer == 0) {
                record->state = 0;
                gOverlay13ActiveCount--;
            } else {
                result = (s16 *)ticks;
                if (ticks-- != 0) {
                    goto loop_fade;
                }
            }
            state = record->state;
        }
        if (state != 0) {
            index = 1 - record->vertexIndex;
            record->vertexIndex = index;
            result = (s16 *)((u8 *)record + index * 0x28 + 0x30 + 0x1E);
            radius = record->scale * (24.0f - 0.1875f * (f32)record->timer);
            y = (s32)record->targetZ;
            result[-14] = y;
            result[-15] = (s32)(record->x - radius);
            result[-13] = (s32)(record->y + radius);
            result[-9] = y;
            result[-10] = (s32)(record->x + radius);
            result[-8] = (s32)(record->y + radius);
            result[-4] = y;
            result[-5] = (s32)(record->x - radius);
            result[-3] = (s32)(record->y - radius);
            result[1] = y;
            result[0] = (s32)(record->x + radius);
            result[2] = (s32)(record->y - radius);
        }
    }
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o013/overlay13ProcessRecord/func_overlay_013_F0000284_186ED9C.s")
#endif
