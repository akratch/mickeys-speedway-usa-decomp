#include "PR/ultratypes.h"

typedef struct O9Output {
    s16 pad0;
    s16 pitch;
    s16 yaw;
    u8 pad6[6];
    s32 x;
    s32 y;
    s32 z;
} O9Output;

typedef struct O9Control {
    u8 pad000[4];
    f32 lean;
    u8 pad008[0xB4];
    s32 handle;
} O9Control;

typedef struct O9State {
    f32 x;
    f32 y;
    f32 throttle;
    f32 scale;
    f32 minimum;
    u8 pad014[8];
    f32 magnitude;
} O9State;

extern f32 D_18;
extern f32 D_1C;
extern s32 ext_o0_2b90();
extern s32 ext_o0_2952c();
extern s32 ext_o0_2d70();
extern s32 ext_o0_2c64();
extern void func_overlay_009_F00009BC_1867034(O9Output *, O9Control *, O9State *);

void func_overlay_009_F0000744_1866DBC(O9Output *output, O9Control *control,
                                       O9State *state, s32 updateCount) {
    f32 level;
    f32 amount;
    s32 i;

    i = updateCount - 1;
    if (updateCount != 0) {
        do {
            func_overlay_009_F00009BC_1867034(output, control, state);
        } while (i--);
    }

    level = state->scale * D_18;
    if (level > 1.0f) {
        level = 1.0f;
    }

    output->pitch = -(s32)(state->x * 8192.0f * level);
    output->yaw = (s32)(state->throttle * state->y * D_1C * level);

    state->magnitude = control->lean / 20.0f;
    if (state->magnitude < 0.0f) {
        state->magnitude = -state->magnitude;
    }

    amount = state->minimum / 3.0f;
    if (amount < 0.0f) {
        amount = 0.0f;
    }
    if (state->magnitude < amount) {
        state->magnitude = amount;
    }

    if (control->handle == 0) {
        ext_o0_2b90(0x16, output->x, output->y, output->z, 1,
                     &control->handle);
    }

    if (control->handle != 0) {
        amount = (state->magnitude * 100.0f) + 50.0f;
        amount += (f32)ext_o0_2952c(-5, 5);
        ext_o0_2d70(control->handle, output->x, output->y, output->z);
        ext_o0_2c64(control->handle, (u8)(u32)amount);
    }
}
