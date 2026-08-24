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

#ifdef NON_MATCHING
void *overlay13UpdateRecord(Overlay13Record *record, s32 ticks) {
    f32 radius;
    f32 gravity;
    f32 velocityZ;
    f32 targetZ;
    void *result;
    s32 y;
    s32 index;
    s16 *out;
    u8 state;
    u8 timer;

    overlay13Prepare(13, 50, 10, record);
    state = record->state;
    result = (void *)ticks;
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
        index = ticks - 1;
        if (ticks != 0) {
            timer = record->timer;
loop_fade:
            timer -= 2;
            record->timer = timer;
            record->phase += 0.5f;
            if (timer == 0) {
                record->state = 0;
                gOverlay13ActiveCount--;
            } else {
                result = (void *)index;
                index--;
                if (index != 0) {
                    goto loop_fade;
                }
            }
            state = record->state;
        }
        if (state != 0) {
            index = 1 - record->vertexIndex;
            record->vertexIndex = index;
            out = (s16 *)((u8 *)record + index * 0x28 + 0x30 + 0x1E);
            result = out;
            radius = record->scale * (24.0f - 0.1875f * (f32)record->timer);
            y = (s32)record->targetZ;
            out[-14] = y;
            out[-15] = (s32)(record->x - radius);
            out[-13] = (s32)(record->y + radius);
            out[-9] = y;
            out[-10] = (s32)(record->x + radius);
            out[-8] = (s32)(record->y + radius);
            out[-4] = y;
            out[-5] = (s32)(record->x - radius);
            out[-3] = (s32)(record->y - radius);
            out[1] = y;
            out[0] = (s32)(record->x + radius);
            out[2] = (s32)(record->y - radius);
        }
    }
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o013/overlay13ProcessRecord/func_overlay_013_F0000284_186ED9C.s")
#endif
