#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG scans found no exact moving-object donor. */

typedef struct Overlay75Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay75Vector;

typedef struct Overlay75Header {
    u8 pad00[0x0C];
    Overlay75Vector vector;
} Overlay75Header;

typedef struct Overlay75Model {
    void *entity00;
    u8 pad04[4];
    s16 field08;
    s16 tableIndex0A;
    void *table0C[13];
    Overlay75Header *header40;
} Overlay75Model;

typedef struct Overlay75State {
    s16 slot00;
    s16 active02;
    s16 phase04;
    s16 timer06;
    f32 advanceRate08;
    f32 retractRate0C;
    f32 baseX10;
    f32 baseY14;
    f32 baseZ18;
    f32 endpointX1C;
    f32 endpointY20;
    f32 endpointZ24;
    f32 progress28;
    f32 progressLimit2C;
    f32 cachedX30;
    f32 cachedY34;
    f32 cachedZ38;
    void *effectHandle3C;
} Overlay75State;

typedef struct Overlay75Status {
    u8 pad00[0x61];
    u8 alternateEvent61;
} Overlay75Status;

typedef struct Overlay75Object {
    s16 angle00;
    u8 pad02[0x0A];
    f32 x0C;
    f32 y10;
    f32 z14;
    u8 pad18[0x10];
    f32 transitionValue28;
    u8 pad2C[0x0E];
    s8 modelIndex3A;
    u8 pad3B[0x0D];
    Overlay75Status *status48;
    u8 pad4C[4];
    void *renderResource50;
    u8 pad54[0x10];
    Overlay75State *state64;
    Overlay75Model **models68;
    u8 pad6C[0x14];
    u32 eventFlags80;
} Overlay75Object;

extern s32 gOverlay75SlotFlags[];
extern f32 gOverlay75ThresholdReloc;
extern f32 overlay75Sin(s16 angle);
extern f32 overlay75Cos(s16 angle);
extern void overlay75MoveReloc(Overlay75Object *object, f32 x, f32 y, f32 z);
extern s32 overlay75TransitionReloc(Overlay75Object *object, f32 rate, f32 tick);
extern void overlay75RefreshModelReloc(Overlay75Model *model, void *entity,
                                       Overlay75Object *object);
extern void overlay75BindModelReloc(Overlay75Object *object, Overlay75Model *model,
                                    void *renderResource, void *tableEntry);
extern void overlay75CreateHandleReloc(s32 id, u32 x, u32 y, u32 z,
                                       s32 mode, void **output);
extern void overlay75SpawnCompletionReloc(f32 x, f32 y, f32 z, f32 scale,
                                          f32 lifetime);
extern void overlay75UpdateHandleReloc(void *handle, u32 x, u32 y, u32 z);
extern void overlay75ReleaseHandleReloc(void *handle);
extern void overlay75PlayEventReloc(u16 eventId, u32 x, u32 y, u32 z, s32 mode,
                                    s32 flags);
extern void overlay75FinishUpdateReloc(Overlay75Object *object, s32 updateRate);

#define O75_FLOAT_BITS(value) (*(u32 *)&(value))

typedef struct Overlay75UpdateLocals {
    s32 moved;
    s32 eventId;
    s32 completed;
    f32 previous;
    u32 pad10;
    f32 moveX;
    u32 pad18;
    f32 delta;
} Overlay75UpdateLocals;

/*
 * Plateau (9 structural attempts plus a bounded 10-minute permuter batch):
 * canonical MIPS-II is exact-size, improved from 55 differing words first at
 * +0x2C to 40 first at +0x2C by computing endpoint deltas before phase-state
 * stores and spelling the event choice as an explicit branch.  Scalar versus
 * aggregate locals, declaration scopes/order, delayed initializers, and
 * register qualifiers did not remove the extra stack home that leaves the
 * position, saved entity, and model slots four bytes above target.  The
 * permuter's separate state-pointer snapshot regressed to 71 words at +0x0 in
 * the real full-TU sweep.
 */
#ifdef NON_MATCHING
void overlay75UpdateMovingObject(Overlay75Object *object,
                                       s32 updateRate) {
    register Overlay75Model *model;
    void *savedEntity;
    Overlay75Vector *position;
    Overlay75State *state;

    object->eventFlags80 = 0;
    model = object->models68[object->modelIndex3A];
    state = object->state64;
    position = &model->header40->vector;

    if (gOverlay75SlotFlags[state->slot00] != 0) {
        gOverlay75SlotFlags[state->slot00] = 0;
        state->active02 = 1;
    }

    {
        Overlay75UpdateLocals locals;
        f32 tick;
        f32 moveZ;

        locals.moved = 0;
        locals.eventId = -1;
        locals.completed = 0;

        if (state->active02 == 0) {
            goto cache_position;
        }
        tick = (f32)updateRate;
        savedEntity = model->entity00;

        if (state->phase04 == 0) {
            f32 limit;

            locals.moved = 1;
            limit = state->progressLimit2C;
            locals.delta = state->advanceRate08 * tick;
            state->progress28 += locals.delta;
            if (limit <= state->progress28) {
                locals.moveX = state->endpointX1C - object->x0C;
                moveZ = state->endpointZ24 - object->z14;
                state->progress28 = limit;
                state->phase04 = 1;
            } else {
                locals.moveX = -(overlay75Sin(object->angle00) * locals.delta);
                moveZ = -(overlay75Cos(object->angle00) * locals.delta);
            }
            overlay75MoveReloc(object, locals.moveX, 0.0f, moveZ);
        } else if (state->phase04 == 1) {
            locals.previous = object->transitionValue28;
            if (overlay75TransitionReloc(object,
                              state->advanceRate08 / 1000.0f,
                              tick) != 0) {
                state->phase04 = 2;
                state->timer06 = 240;
                locals.completed = 1;
            } else {
                locals.moved = 1;
            }
            if (locals.previous < gOverlay75ThresholdReloc &&
                gOverlay75ThresholdReloc <= object->transitionValue28) {
                if (object->status48->alternateEvent61 != 0) {
                    locals.eventId = 0x1BC;
                } else {
                    locals.eventId = 0x1B9;
                }
            }
        } else if (state->phase04 == 2) {
            if (object->status48->alternateEvent61 != 0) {
                state->timer06 = 0;
            }
            state->timer06 -= updateRate;
            if (state->timer06 <= 0) {
                state->timer06 = 0;
                state->phase04 = 3;
            }
        } else if (state->phase04 == 3) {
            if (overlay75TransitionReloc(object,
                              -(state->retractRate0C / 1000.0f),
                              tick) != 0) {
                state->phase04 = 4;
            } else {
                locals.moved = 1;
            }
        } else if (state->phase04 == 4) {
            locals.delta = state->retractRate0C * tick;
            state->progress28 -= locals.delta;
            if (state->progress28 <= 0.0f) {
                locals.moveX = state->baseX10 - object->x0C;
                moveZ = state->baseZ18 - object->z14;
                state->progress28 = 0.0f;
                state->phase04 = 0;
                state->active02 = 0;
            } else {
                locals.moveX = overlay75Sin(object->angle00) * locals.delta;
                moveZ = overlay75Cos(object->angle00) * locals.delta;
                locals.moved = 1;
            }
            overlay75MoveReloc(object, locals.moveX, 0.0f, moveZ);
        }

        overlay75RefreshModelReloc(model, savedEntity, object);
        overlay75BindModelReloc(object, model, object->renderResource50,
                      model->table0C[model->tableIndex0A]);
        model->field08 = 0;

        if (locals.moved != 0 && state->effectHandle3C == 0) {
            overlay75CreateHandleReloc(0x1BA, O75_FLOAT_BITS(position->x),
                                       O75_FLOAT_BITS(position->y),
                                       O75_FLOAT_BITS(position->z), 1,
                                       &state->effectHandle3C);
        } else if (locals.moved != 0 && state->effectHandle3C != 0) {
            overlay75UpdateHandleReloc(state->effectHandle3C,
                          O75_FLOAT_BITS(position->x),
                          O75_FLOAT_BITS(position->y),
                          O75_FLOAT_BITS(position->z));
        } else if (locals.moved == 0 && state->effectHandle3C != 0) {
            overlay75ReleaseHandleReloc(state->effectHandle3C);
        }

        if (locals.eventId != -1) {
            overlay75PlayEventReloc(locals.eventId, O75_FLOAT_BITS(position->x),
                          O75_FLOAT_BITS(position->y),
                          O75_FLOAT_BITS(position->z), 4, 0);
        }
        if (locals.completed != 0) {
            overlay75SpawnCompletionReloc(position->x, position->y,
                                           position->z,
                                           600.0f, 20.0f);
            object->eventFlags80 |= 1;
            overlay75FinishUpdateReloc(object, updateRate);
        }
    }

cache_position:
    state->cachedX30 = position->x;
    state->cachedY34 = position->y;
    state->cachedZ38 = position->z;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o075/overlay75UpdateMovingObject/func_overlay_075_F0000214_18CC17C.s")
#endif
