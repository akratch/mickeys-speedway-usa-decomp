#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG exact-object scans are negative for this initializer. */
typedef struct Overlay87InitState {
    f32 value00;
    f32 value04;
    f32 value08;
    f32 value0C;
    f32 value10;
    f32 value14;
    u8 pad18[4];
    f32 value1C;
    u8 pad20[0xA];
    s16 field2A;
    s16 field2C;
    s16 field2E;
    u8 pad30[2];
    s16 field32;
    s16 field34;
} Overlay87InitState;

typedef struct Overlay87InitObject {
    s16 angle;
    u8 pad02[6];
    f32 value08;
    f32 value0C;
    f32 value10;
    f32 value14;
    u8 pad18[0x28];
    f32 *scale;
    u8 pad44[0x20];
    Overlay87InitState *state;
} Overlay87InitObject;

typedef struct Overlay87InitConfig {
    u8 pad00[0xA];
    s16 angle;
    s16 scale;
    s16 value04;
    s16 value00;
    s16 value08;
} Overlay87InitConfig;

extern f32 gOverlay87InitScaleReloc;
extern s32 mathRnd(s32 lower, s32 upper);
extern void func_8005AD64(Overlay87InitObject *object, s32 mode, s32 index,
                          f32 value);

#ifdef NON_MATCHING
void overlay87InitializeObject(Overlay87InitObject *object,
                               Overlay87InitConfig *config, void *unused) {
    Overlay87InitState *state;
    f32 value;

    (void)unused;
    state = object->state;
    object->value08 = config->scale * gOverlay87InitScaleReloc * *object->scale;
    state->value00 = config->value00;
    state->value08 = config->value08;
    state->field2A = 0;
    value = state->value08;
    state->value08 = value * value;
    state->value0C = object->value0C;
    state->value10 = object->value10;
    state->value14 = object->value14;
    state->value04 = config->value04;
    state->field2C = config->angle;
    state->field2E = config->angle;
    state->field32 = 0;
    state->field34 = 0x300;
    state->value1C = object->value10;
    object->angle = state->field2C + 0x4000;
    func_8005AD64(object, 2, -1, (f32)mathRnd(0, 0x63) / 100.0f);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o087/overlay87InitializeObject/func_overlay_087_F0000000_18D2F68.s")
#endif
