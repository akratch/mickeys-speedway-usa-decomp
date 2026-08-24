#include "PR/ultratypes.h"

typedef struct O9Output {
    u8 pad000[0xC];
    f32 x;
    u8 pad010[4];
    f32 z;
    u8 pad018[4];
    f32 dx;
    f32 zero;
    f32 dz;
} O9Output;

typedef struct O9Control {
    u8 pad000[4];
    f32 scaleX;
    f32 scaleZ;
    u8 pad00C[0x2C];
    f32 originX;
    u8 pad03C[4];
    f32 originZ;
    u8 pad044[0x30];
    f32 dirX;
    f32 dirY;
    f32 dirZ;
    f32 speedLimit;
    f32 velocity;
    f32 acceleration;
    u8 pad08C[0x64];
    s16 angle;
    u8 pad0F2[0x8F];
    u8 active;
    u8 pad182[0x2B6];
    s32 mode;
} O9Control;

extern f32 ext_o0_2a470(s32);
extern f32 ext_o0_2a46c(s32);
extern void ext_o0_7cd8(O9Output *, f32, f32, f32);
extern void ext_o0_1d920(O9Output *, O9Control *, f32);

void func_overlay_009_F0000CE4_186735C(O9Output *out, O9Control *control,
                                       void *unused, f32 step) {
    f32 xVelocity;
    f32 yVelocity;
    f32 zVelocity;
    f32 xExtra;
    f32 yExtra;
    f32 zExtra;
    f32 fraction;
    s16 angle = control->angle;

    if (control->active != 0) {
        f32 distance = (control->velocity * step) +
            (0.5f * control->acceleration * step * step);
        xExtra = control->dirX * distance;
        yExtra = control->dirY * distance;
        zExtra = control->dirZ * distance;
        if (distance < 0.0f) {
            control->velocity = 0.0f;
            control->acceleration = 0.0f;
            control->active = 0;
        }
        fraction = 1.0f - (control->velocity / control->speedLimit);
        control->velocity += control->acceleration * step;
        xVelocity = ext_o0_2a470(angle) * control->scaleX * fraction;
        zVelocity = ext_o0_2a46c(angle) * control->scaleX * fraction;
    } else {
        xExtra = 0.0f;
        yExtra = 0.0f;
        zExtra = 0.0f;
        xVelocity = ext_o0_2a470(angle) * control->scaleX;
        zVelocity = ext_o0_2a46c(angle) * control->scaleX;
    }
    yVelocity = 0.0f;
    out->zero = 0.0f;
    if (control->mode == 1) {
        xVelocity *= 0.65f;
        zVelocity *= 0.65f;
        if ((control->scaleX < -0.5f) || (control->scaleX > 0.5f))
            control->scaleX *= 0.65f;
        else
            control->scaleX = 0.0f;
        if ((control->scaleZ < -0.5f) || (control->scaleZ > 0.5f))
            control->scaleZ *= 0.65f;
        else
            control->scaleZ = 0.0f;
    }
    ext_o0_7cd8(out, (xVelocity * step) + xExtra,
                 (yVelocity * step) + yExtra,
                 (zVelocity * step) + zExtra);
    fraction = 1.0f / step;
    out->dx = (out->x - control->originX) * fraction;
    out->dz = (out->z - control->originZ) * fraction;
    ext_o0_1d920(out, control, step);
}
