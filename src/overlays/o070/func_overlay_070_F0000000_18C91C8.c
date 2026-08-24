#include "PR/ultratypes.h"

typedef struct O70State {
    u8 active;
    u8 type;
    u8 timer;
    s8 countdown;
    s16 angle;
    s16 angleStep;
    s16 verticalStep;
    u8 pad0A[2];
    s32 flags;
    f32 threshold;
    void *related;
} O70State;

typedef struct O70Object {
    u8 pad00[4];
    s16 angle;
    u8 pad06[0x22];
    f32 value28;
    u8 pad2C[0x38];
    O70State *state;
} O70Object;

typedef struct O70Input {
    u8 pad00[0xA];
    u8 type;
    u8 pad0B;
    void *related;
} O70Input;

extern f32 gOverlay70FloatTableReloc[];
extern s8 gOverlay70VerticalStepReloc[];
extern s16 gOverlay70AngleReloc[];
extern s32 overlay70RandomRange(s32 lower, s32 upper);

#ifdef NON_MATCHING
void func_overlay_070_F0000000_18C91C8(O70Object *object, O70Input *input) {
    O70State *state;
    u8 type;

    state = object->state;
    state->related = input->related;
    state->type = input->type;
    state->angle = overlay70RandomRange(-0x8000, 0x7FFF);
    if (overlay70RandomRange(0, 1) == 0) {
        state->angleStep = overlay70RandomRange(-0x200, -0x180);
    } else {
        state->angleStep = overlay70RandomRange(0x180, 0x200);
    }
    state->countdown = 0;
    state->active = 0;
    state->timer = 0xBE;
    state->flags = 0xD;
    object->value28 = gOverlay70FloatTableReloc[state->type];
    type = state->type;
    state->verticalStep = gOverlay70VerticalStepReloc[type];
    object->angle = gOverlay70AngleReloc[type];
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o070/func_overlay_070_F0000000_18C91C8/func_overlay_070_F0000000_18C91C8.s")
#endif
