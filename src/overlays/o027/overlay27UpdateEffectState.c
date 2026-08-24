#include "PR/ultratypes.h"

typedef struct O27Object O27Object;

typedef struct O27State {
    u8 primaryState;
    u8 pulseState;
    s16 timer;
    u8 colorR;
    u8 colorG;
    u8 colorB;
    u8 pulseR;
    u8 pulseG;
    u8 pulseB;
    s16 intensity;
    s16 fade;
    s16 pulseTimer;
    f32 scaleTarget;
    f32 fadeFloat;
    void *primaryHandle;
    void *secondaryHandle;
    O27Object *source;
    u8 reserved24[0x184];
    u16 flags1A8;
} O27State;

struct O27Object {
    u8 reserved00[8];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 reserved18[0x16];
    s16 positionTag;
    u8 reserved30[0x34];
    O27State *state;
    void **resource;
};

extern s32 gO27Active;
extern f32 gO27Scale0;
extern f32 gO27EaseInput;
extern f32 gO27Scale8;
extern f32 gO27ScaleC;
extern void func_80036544(void *, s32 *, s32, void *, s32);
extern f32 func_8002A878(f32, s32);
extern s32 func_800299E8(s32, s32);
extern void func_800031E8(void *);
extern void func_80002FE0(s32, f32, f32, f32, s32, void **);
extern void func_8002BD58(s32, s32, f32);
extern void func_800031C0(void *, f32, f32, f32);
extern void func_8000309C(void *, u8);
extern void func_80006EA0(O27Object *);

void func_overlay_027_F0000064_187BA3C(O27Object *object, s32 updateRate) {
    O27State *state;
    O27Object *source;
    union {
        O27State *sourceState;
        s32 intensity;
    } tail;
    s32 pulseStep;
    s32 initialPhase;
    s32 phase;
    s32 value;
    f32 fraction;
    f32 scaleFactor;

    state = object->state;
    pulseStep = updateRate;
    source = state->source;
    if (source != 0) {
        object->x = source->x;
        object->y = source->y;
        object->z = source->z;
        object->positionTag = source->positionTag;
    } else {
        state->primaryState = 4;
    }

    gO27Active = 1;
    initialPhase = 9;
    func_80036544(*object->resource, &initialPhase, 10, &object->reserved30[-8],
                  updateRate);

    if (updateRate != 0) {
        scaleFactor = gO27Scale0;
        do {
            switch (state->primaryState) {
                case 0:
                    state->timer += updateRate;
                    fraction = 1.0f - func_8002A878(gO27EaseInput, updateRate);
                    scaleFactor = gO27Scale8;
                    state->scaleTarget +=
                        (32.0f - state->scaleTarget) * fraction;
                    value = state->timer;
                    updateRate = value - 120;
                    if (value >= 120) {
                        state->primaryState = 1;
                        state->timer = 0;
                        state->scaleTarget = 32.0f;
                        phase = 0x10000;
                    } else {
                        phase = (value << 16) / 120;
                        updateRate = 0;
                    }
                    value = (((-95 * phase) >> 16) + 0xFF);
                    state->colorR = value;
                    state->colorG = value;
                    state->colorB = value;
                    value = (((-64 * phase) >> 16) + 0x40);
                    state->pulseR = value;
                    state->pulseG = value;
                    state->pulseB = value;
                    if (phase >= 0x8000) {
                        state->intensity = 0xFF;
                    } else {
                        state->intensity = (phase * 0xFF) >> 15;
                    }
                    object->scale =
                        1.0f + ((f32)state->intensity * scaleFactor);
                    break;

                case 1:
                    state->timer += updateRate;
                    value = state->timer;
                    updateRate = value - 60;
                    if (value >= 60) {
                        state->primaryState = 2;
                        state->timer = 0;
                        state->fade = 0xFF;
                        state->fadeFloat = 1.0f;
                    } else {
                        updateRate = 0;
                        fraction = (f32)value / 60.0f;
                        state->fadeFloat = fraction;
                        state->fade = (s16)(255.0f * fraction);
                    }
                    break;

                case 2:
                    state->timer += updateRate;
                    value = state->timer;
                    updateRate = value - 480;
                    if (value >= 480) {
                        state->primaryState = 4;
                        state->timer = 0;
                    } else {
                        updateRate = 0;
                    }
                    break;

                case 3:
                    if (state->intensity < 0xFF) {
                        state->intensity += updateRate * 4;
                        value = state->intensity;
                        updateRate = 0;
                        if (value >= 0x100) {
                            state->intensity = 0xFF;
                            object->scale = 2.0f;
                        } else {
                            object->scale =
                                1.0f + ((f32)value * scaleFactor);
                        }
                    } else {
                        state->fade += updateRate * 4;
                        value = state->fade;
                        updateRate = 0;
                        if (value >= 0xFF) {
                            state->primaryState = 2;
                            state->timer = 0;
                            state->fade = 0xFF;
                            state->fadeFloat = 1.0f;
                        } else {
                            state->fadeFloat =
                                (f32)value * scaleFactor;
                        }
                    }
                    break;

                default:
                    value = state->fade;
                    if (value >= updateRate * 8) {
                        state->fade = value - (updateRate * 8);
                        updateRate = 0;
                        state->fadeFloat =
                            (f32)state->fade * scaleFactor;
                    } else {
                        state->intensity -= updateRate * 4;
                        state->fade = 0;
                        updateRate = 0;
                        state->fadeFloat = 0.0f;
                        if (state->intensity <= 0) {
                            func_80006EA0(object);
                            scaleFactor = gO27ScaleC;
                        } else {
                            object->scale = 1.0f +
                                ((f32)state->intensity * scaleFactor);
                        }
                    }
                    break;
            }
        } while (updateRate != 0);
    }

    if (state->fade == 0) {
        return;
    }

    if (state->pulseState == 0) {
        if (state->fade == 0xFF && func_800299E8(0, 0x1FFF) >= 0x1FD7) {
            tail.sourceState = source->state;
            state->pulseState = 1;
            if (state->secondaryHandle != 0) {
                func_800031E8(state->secondaryHandle);
            }
            func_80002FE0(0x1BB, object->x, object->y, object->z, 4,
                          &state->secondaryHandle);
            if (!(tail.sourceState->flags1A8 & 1)) {
                func_8002BD58(*(s8 *)&tail.sourceState->primaryState,
                              0x32, 0.4f);
            }
        }
    } else {
        if (state->pulseState == 1) {
            state->pulseTimer += pulseStep << 6;
        } else {
            state->pulseTimer -= pulseStep << 5;
        }
        value = state->pulseTimer;
        if (value >= 0x100) {
            state->pulseTimer = 0xFF;
            state->pulseState = 2;
            state->pulseR = 0x80;
            state->pulseG = 0x80;
            state->pulseB = 0;
        } else if (value < 0) {
            state->pulseTimer = 0;
            state->pulseState = 0;
            state->pulseR = 0;
            state->pulseG = 0;
            state->pulseB = 0;
        } else {
            value = (value << 7) >> 8;
            state->pulseR = value;
            state->pulseG = value;
            state->pulseB = 0;
        }
    }

    if (state->primaryHandle == 0) {
        func_80002FE0(0x1B8, object->x, object->y, object->z, 1,
                      &state->primaryHandle);
    }
    tail.intensity = state->fade >> 1;
    if (tail.intensity >= 0x80) {
        tail.intensity = 0x7F;
    }
    if (state->primaryHandle != 0) {
        func_800031C0(state->primaryHandle, object->x, object->y, object->z);
        func_8000309C(state->primaryHandle, tail.intensity);
    }
    if (state->secondaryHandle != 0) {
        func_800031C0(state->secondaryHandle, object->x, object->y, object->z);
        func_8000309C(state->secondaryHandle, tail.intensity);
    }
}
