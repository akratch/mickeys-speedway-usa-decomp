#include "PR/ultratypes.h"

typedef struct O9Angle { u8 pad000[4]; s16 angle; } O9Angle;
typedef struct O9Motion {
    u8 pad000[0x24];
    f32 velocity;
    s16 angle;
} O9Motion;

extern f32 D_C, D_10, D_14;
extern s16 *D_0;
extern s32 ext_o0_2a5bc(s32, s32);
extern f32 ext_o0_2a470(s32);

void func_overlay_009_F0000540_1866BB8(O9Angle *angle, void *unused,
                                       O9Motion *motion, s32 steps) {
    s32 delta;

    if (steps--) {
        f32 lower;
        f32 upper;
        f32 upperThreshold;
        f32 lowerThreshold;
        f32 damping;

        upperThreshold = D_C;
        lowerThreshold = D_10;
        damping = D_14;
        upper = 16.0f;
        lower = -16.0f;
        do {
            delta = ext_o0_2a5bc(motion->angle, -angle->angle);
            if ((delta >= -0x3F) && (delta < 0x40) &&
                (motion->velocity > lower) && (motion->velocity < upper)) {
                motion->velocity = 0.0f;
                motion->angle = -angle->angle;
            } else {
                motion->velocity += 20.0f * ext_o0_2a470(delta);
                motion->angle += (s32) motion->velocity;
            }
            motion->velocity *= damping;
            if ((motion->velocity > lowerThreshold) &&
                (motion->velocity < upperThreshold)) {
                motion->velocity = 0.0f;
            }
        } while (steps--);
    }

    delta = motion->angle;
    if (delta < -0x4000) delta = -0x8000 - delta;
    if (delta >= 0x4001) delta = 0x8000 - delta;
    *D_0++ = 0xB;
    *D_0++ = motion->angle;
    *D_0++ = 0xA;
    *D_0++ = -delta;
}
