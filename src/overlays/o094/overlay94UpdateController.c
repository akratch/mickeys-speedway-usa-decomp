#include "PR/ultratypes.h"

/* Pinned DKR v77/v80 and JFG scans found no exact donor for this body. */

typedef struct Overlay94Record {
    u8 pad00[0x40];
    u8 queryState[1];
} Overlay94Record;

typedef struct Overlay94Entity {
    s32 savedValue;
    u8 pad04[4];
    s16 status;
    s16 recordIndex;
    Overlay94Record *records[1];
} Overlay94Entity;

typedef struct Overlay94State {
    f32 current;
    s8 selector;
    s8 active;
    s16 velocity;
    s32 angle;
    s32 accumulator;
    s16 command;
} Overlay94State;

typedef struct Overlay94Object {
    u8 pad00[0x50];
    void *renderResource;
    u8 pad54[0x10];
    Overlay94State *state;
    Overlay94Entity **entityRef;
} Overlay94Object;

extern s32 gO94Value;
extern f32 gO94Const0;
extern f32 gO94Const4;
extern f32 gO94Const8;
extern f32 gO94ConstC;
extern f32 gO94Const10;
extern f32 gO94Const14;
extern f32 gO94Const18;
extern f32 gO94Const1C;
extern f32 gO94Const20;
extern f32 gO94Const24;

extern u32 func_800254FC(s32 selector);
extern s32 func_8002565C(s32 selector);
extern f32 func_8002A878(f32 amount, s32 updateRate);
extern void func_8005ABA8(Overlay94Object *object, f32 current, f32 rate);
extern void func_8005AF14(Overlay94Entity *entity, s32 savedValue,
                          Overlay94Object *object);
extern void func_80019AB8(Overlay94Object *object, Overlay94Entity *entity,
                          void *resource, Overlay94Record *record);
extern void func_8002B040(void *queryState, f32 x, f32 y, f32 z,
                          f32 *out0, f32 *out1, f32 *out2);
extern s32 func_8002A910(f32 z, f32 x);

/* Size-exact plateau after ten structural/lifetime forms: the ordinary
 * -O2/-mips2 build differs in 13 of 275 words, first at +0x38. Post-increment
 * command stores and clamping the target in place reproduce the retail CFG
 * and FP web; the residual is a shared spill at sp+0x38 rather than sp+0x34
 * plus the terminal negative-velocity path's private GPR coloring. */
#ifdef NON_MATCHING
void overlay94UpdateController(Overlay94Object *object, s32 updateRate) {
    Overlay94State *state;
    Overlay94Entity *entity;
    s32 savedValue;
    f32 rate;
    s16 *command;
    f32 out0;
    f32 out1;
    f32 out2;
    f32 target;
    f32 weight;
    s32 angle;

    state = object->state;
    command = &state->command;

    if (gO94Value != 0 &&
        (func_800254FC(state->selector) & 0x2000) != 0) {
        state->active = 1;
    } else if (gO94Value == 0) {
        state->active = 0;
    }

    if (state->active != 0) {
        if (state->velocity != 0) {
            state->accumulator +=
                (u32)(s32)state->velocity * (u32)updateRate;
            if (state->velocity > 0) {
                if (state->accumulator >= 0x10000) {
                    state->accumulator = 0;
                    state->velocity = 0;
                }
            } else if (state->velocity < 0 &&
                       state->accumulator < -0xFFFF) {
                state->accumulator = 0;
                state->velocity = 0;
            }

            *command++ = 4;
            *command++ = (s16)state->accumulator;

            weight = func_8002A878(gO94Const0, updateRate);
            state->current =
                ((1.0f - weight) * (0.0f - state->current)) + state->current;
        } else if ((func_800254FC(state->selector) & 0x2000) != 0) {
            target = (f32)func_8002565C(state->selector) / 60.0f;
            if (target > 1.0f) {
                target = 1.0f;
            } else if (target < 0.0f) {
                target = 0.0f;
            }
            target *= gO94Const4;
            weight = func_8002A878(gO94Const8, updateRate);
            state->current =
                ((1.0f - weight) * (target - state->current)) + state->current;
        } else {
            weight = func_8002A878(gO94ConstC, updateRate);
            state->current =
                ((1.0f - weight) * (0.0f - state->current)) + state->current;
        }
    } else {
        weight = func_8002A878(gO94Const10, updateRate);
        state->current =
            ((1.0f - weight) * (gO94Const14 - state->current)) + state->current;
    }

    *command = 0x2000;
    entity = *object->entityRef;
    savedValue = entity->savedValue;
    rate = (f32)updateRate;

    func_8005ABA8(object, state->current, rate);
    func_8005AF14(entity, savedValue, object);
    func_80019AB8(object, entity, object->renderResource,
                  entity->records[entity->recordIndex]);
    entity->status = 0;

    func_8002B040(entity->records[entity->recordIndex]->queryState,
                  0.0f, 0.0f, -1.0f,
                  &out0, &out1, &out2);
    angle = func_8002A910(out2, out0);

    if (state->active != 0 && state->velocity == 0) {
        s32 reverse;
        s32 delta;

        delta = (angle - state->angle) & 0xFFFF;
        reverse = (state->angle - angle) & 0xFFFF;
        if (reverse < delta) {
            delta = -reverse;
        }

        weight = (f32)delta / rate;
        if (weight < gO94Const18 || gO94Const1C < weight) {
            if (weight < 0.0f) {
                state->velocity = (s16)(s32)(state->current * gO94Const20);
                if (state->velocity < 500) {
                    state->velocity = 500;
                }
            } else if (weight > 0.0f) {
                state->velocity =
                    (s16)-(s32)(state->current * gO94Const24);
                if (state->velocity >= -499) {
                    state->velocity = -500;
                }
            }
        }
    }

    state->angle = angle;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o094/overlay94UpdateController/func_overlay_094_F0000110_18D6CB0.s")
#endif
