#include "PR/ultratypes.h"

typedef struct Overlay79Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay79Vector;

typedef struct Overlay79Object Overlay79Object;

typedef struct Overlay79MotionState {
    s16 targetAngle;
    s8 mode;
    s8 active;
    s16 event;
    s16 step;
    u32 collisionFlags;
    s32 effectTimer;
    f32 targetX;
    f32 targetZ;
    f32 acceleration;
    f32 speed;
    f32 heightOffset;
    f32 homeX;
    f32 homeY;
    f32 homeZ;
    f32 radiusSquared;
    s32 travelDistance;
    f32 previousHeight;
    void *effect;
    Overlay79Object *target;
} Overlay79MotionState;

struct Overlay79Object {
    s16 angle;
    u8 pad02[6];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    f32 floorHeight;
    u8 pad2C[0xF];
    s8 mode;
    u8 pad3C[0x28];
    Overlay79MotionState *state;
    u8 pad68[0x18];
    u32 flags;
};

typedef struct Overlay79SpawnDesc {
    s16 objectId;
    s8 count;
    s8 flags;
    s16 x;
    s16 y;
    s16 z;
    u8 pad0A[2];
    Overlay79Object *parent;
    f32 scale;
} Overlay79SpawnDesc;

extern u32 gOverlay79RaceFlags;
extern f32 gOverlay79ActiveAcceleration;
extern f32 gOverlay79InactiveAcceleration;
extern f32 gOverlay79TurnPower;
extern f32 gOverlay79ForwardAcceleration;
extern f32 gOverlay79TargetDot;
extern f32 gOverlay79LaunchHeight;
extern f32 gOverlay79ApproachPower;
extern f32 gOverlay79GravityHalf;
extern f32 gOverlay79Gravity;
extern f32 gOverlay79ModeFactors[];
extern f32 gOverlay79ModeSpeeds[];
extern void *gOverlay79CollisionData;

extern Overlay79Object *overlay79FindNearby(Overlay79Vector *position,
                                             f32 radiusSquared);
extern void func_8005AD64(Overlay79Object *object, s32 mode, s32 index,
                          f32 value);
extern f32 sqrtf(f32 value);
extern void func_800031E8(void *handle);
extern s32 mathRnd(s32 lower, s32 upper);
extern void func_80002FE0(s32 id, f32 x, f32 y, f32 z, s32 priority,
                          void **handle);
extern s32 Arctanf(f32 y, f32 x);
extern f32 Powerf(f32 value, s32 exponent);
extern s16 dAngle(s16 current, s16 target, f32 fraction);
extern void mathOneFloatRPY(Overlay79Object *object, Overlay79Vector *vector);
extern f32 func_8002A8C0(s32 angle);
extern f32 func_8002A8BC(s32 angle);
extern Overlay79Object *func_8000590C(Overlay79SpawnDesc *desc, s32 count);
extern void func_80008128(Overlay79Object *object, f32 x, f32 y, f32 z);
extern s32 func_8005ABA8(Overlay79Object *object, f32 factor, f32 updateRate);
extern void func_800031C0(void *handle, f32 x, f32 y, f32 z);
extern void partUpdateTriggers(Overlay79Object *object, s32 updateRate);
extern void trackMakePolylist(s32 mode, Overlay79Vector *start,
                              Overlay79Vector *end, f32 *height, void *unused,
                              s32 flags);
extern s32 func_80010900(Overlay79Vector *start, Overlay79Vector *end,
                         f32 height, Overlay79Object *object, void *collision);

/* Workbench: structure-mismatch (mixed), 850/882 positional words differ; first mismatch +0x4; frame deficit 8 bytes.
 * Levers: MIPS-II lattice, target flag-preserving branch, and target-assembly effect-range audit (four ranges corrected).
 * Remaining: 12 missing instructions plus shared-base/prologue, linear dispatch, relocation, and FP/register allocation drift. */
#ifdef NON_MATCHING
void func_overlay_079_F0000134_18CD0D4(Overlay79Object *object,
                                       s32 updateRate) {
    u8 *dataBase;
    Overlay79MotionState *state;
    Overlay79Object *nearby;
    Overlay79Object *spawned;
    Overlay79SpawnDesc desc;
    Overlay79Vector forward;
    Overlay79Vector start;
    Overlay79Vector end;
    f32 update;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distance;
    f32 verticalDistance;
    f32 factor;
    f32 range;
    s32 raceActive;
    s32 delta;

    update = updateRate;
    dataBase = (u8 *)&gOverlay79RaceFlags;
    state = object->state;
    raceActive = 0;
    if (((*(u16 *)(dataBase + 0xE) & 0x1C0) >> 6) >= 3 &&
        (((*(u32 *)dataBase << 5) >> 28) == 0xF) &&
        !(*(u32 *)dataBase & 0x40000)) {
        raceActive = 1;
    }

    nearby = overlay79FindNearby((Overlay79Vector *)&object->x, 22500.0f);
    if (nearby != NULL) {
        state->active = 1;
    }

    if (state->active != 0) {
        if (state->collisionFlags & 2) {
            if (object->mode != 0) {
                func_8005AD64(object, 0, -1, 0.0f);
            }
        } else if (((state->collisionFlags & 2) == 0) &&
                   (object->mode != 3)) {
            func_8005AD64(object, 3, -1, 0.0f);
        }

        if (nearby != NULL) {
            if (state->speed > -25.0f) {
                state->acceleration = gOverlay79ActiveAcceleration;
            } else {
                state->acceleration = 0.0f;
            }
            dx = nearby->x - object->x;
            dz = nearby->z - object->z;
            if ((sqrtf((dx * dx) + (dz * dz)) < 70.0f) &&
                (state->collisionFlags & 2)) {
                object->velocityY = 5.0f;
                if (state->effect != NULL) {
                    func_800031E8(state->effect);
                }
                func_80002FE0(mathRnd(0x21F, 0x226), object->x, object->y,
                              object->z, 4, &state->effect);
                state->effectTimer = mathRnd(0x78, 0xF0);
            }
        } else if (state->speed < 0.0f) {
            state->acceleration = gOverlay79InactiveAcceleration;
        } else {
            state->active = 0;
            state->step = 0;
            state->mode = 2;
            state->acceleration = 0.0f;
            state->speed = 0.0f;
        }

        dx = object->x - state->homeX;
        dz = object->z - state->homeZ;
        if ((state->radiusSquared <= ((dx * dx) + (dz * dz))) ||
            (nearby == NULL)) {
            state->targetAngle = Arctanf(dx, dz);
        } else {
            state->targetAngle = Arctanf(nearby->x - object->x,
                                         nearby->z - object->z);
        }
        object->angle = dAngle(object->angle, state->targetAngle,
                               1.0f - Powerf(gOverlay79TurnPower, updateRate));
        delta = (object->angle - state->targetAngle) & 0xFFFF;
        if (((state->targetAngle - object->angle) & 0xFFFF) < delta) {
            delta = -((state->targetAngle - object->angle) & 0xFFFF);
        }
        if ((delta >= -99) && (delta < 100)) {
            object->angle = state->targetAngle;
        }
        if (state->target != NULL) {
            dx = state->target->x - object->x;
            dy = state->target->y - object->y;
            dz = state->target->z - object->z;
            if (sqrtf((dx * dx) + (dy * dy) + (dz * dz)) < 30.0f) {
                *(s32 *)state->target->state = 1;
            }
        }
        if (state->effectTimer != 0) {
            state->effectTimer -= updateRate;
            if (state->effectTimer < 0) {
                state->effectTimer = 0;
            }
        }
        if ((state->effect == NULL) && (state->effectTimer == 0)) {
            func_80002FE0(mathRnd(0x227, 0x229), object->x, object->y,
                          object->z, 4, &state->effect);
            state->effectTimer = mathRnd(0x78, 0xF0);
        }
    } else {
        switch (state->mode) {
            case 0:
                state->acceleration =
                    (state->speed > -1.0f) ? gOverlay79ForwardAcceleration
                                           : 0.0f;
                forward.x = 0.0f;
                forward.y = 0.0f;
                forward.z = -1.0f;
                mathOneFloatRPY(object, &forward);
                dx = state->targetX - object->x;
                dz = state->targetZ - object->z;
                if (((forward.z * dz) + (dx * forward.x) < 0.0f) ||
                    (state->collisionFlags & 4)) {
                    state->mode = 1;
                } else if (state->target != NULL) {
                    dx = object->x - state->target->x;
                    dz = object->z - state->target->z;
                    distance = sqrtf((dx * dx) + (dz * dz));
                    forward.x = 0.0f;
                    forward.y = 0.0f;
                    forward.z = -1.0f;
                    mathOneFloatRPY(object, &forward);
                    dx = state->target->x - object->x;
                    dz = state->target->z - object->z;
                    if ((distance < 50.0f) &&
                        (gOverlay79TargetDot <
                         ((forward.z * dz) + (dx * forward.x)))) {
                        state->mode = 1;
                    }
                }
                if (state->effectTimer != 0) {
                    state->effectTimer -= updateRate;
                    if (state->effectTimer < 0) {
                        state->effectTimer = 0;
                    }
                }
                if ((state->effect == NULL) && (state->effectTimer == 0)) {
                    func_80002FE0(mathRnd(0x21B, 0x21E), object->x, object->y,
                                  object->z, 4, &state->effect);
                    state->effectTimer = mathRnd(0x78, 0xF0);
                }
                break;
            case 1:
                if (state->speed < 0.0f) {
                    state->acceleration = gOverlay79InactiveAcceleration;
                } else {
                    state->acceleration = 0.0f;
                    state->speed = 0.0f;
                    state->mode = (state->target == NULL && raceActive != 0)
                                      ? 3
                                      : 2;
                }
                break;
            case 2:
                if (state->step == 0) {
                    func_8005AD64(object, 1, -1, 0.0f);
                    state->step++;
                } else if ((state->step == 1) && (state->event != 0)) {
                    if (state->effect == NULL) {
                        func_80002FE0(mathRnd(0x218, 0x21A), object->x,
                                      object->y, object->z, 4, &state->effect);
                    }
                    func_8005AD64(object, 3, -1, 0.0f);
                    state->step++;
                } else if ((state->step == 2) && (state->event != 0)) {
                    func_8005AD64(object, 1, -1, 0.0f);
                    state->step++;
                } else if ((state->step == 3) && (state->event != 0)) {
                    func_8005AD64(object, 0, -1, 0.0f);
                    state->step = 0;
                    delta = mathRnd(-0x7FFF, 0x8000);
                    range = mathRnd(0, state->travelDistance);
                    state->targetX =
                        state->homeX - (func_8002A8C0(delta) * range);
                    state->targetZ =
                        state->homeZ - (func_8002A8BC(delta) * range);
                    state->targetAngle =
                        Arctanf(object->x - state->targetX,
                                object->z - state->targetZ);
                    state->mode = 4;
                }
                break;
            case 3:
                if (state->step == 0) {
                    delta = mathRnd(-0x7FFF, 0x8000);
                    range = mathRnd(0, state->travelDistance);
                    state->targetX =
                        state->homeX - (func_8002A8C0(delta) * range);
                    state->targetZ =
                        state->homeZ - (func_8002A8BC(delta) * range);
                    state->targetAngle =
                        Arctanf(object->x - state->targetX,
                                object->z - state->targetZ);
                    state->step = 1;
                } else if (state->step == 1) {
                    object->angle =
                        dAngle(object->angle, state->targetAngle,
                               1.0f - Powerf(gOverlay79ApproachPower,
                                             updateRate));
                    delta = (object->angle - state->targetAngle) & 0xFFFF;
                    if (((state->targetAngle - object->angle) & 0xFFFF) <
                        delta) {
                        delta = -((state->targetAngle - object->angle) &
                                  0xFFFF);
                    }
                    if ((delta >= -99) && (delta < 100)) {
                        object->angle = state->targetAngle;
                        state->step = 2;
                    }
                } else if (state->step == 2) {
                    func_8005AD64(object, 6, -1, 0.0f);
                    if (state->effect == NULL) {
                        func_80002FE0(mathRnd(0x270, 0x272), object->x,
                                      object->y, object->z, 4, &state->effect);
                    }
                    state->step = 3;
                } else if ((state->step == 3) &&
                           (state->previousHeight < gOverlay79LaunchHeight) &&
                           (gOverlay79LaunchHeight <= object->floorHeight)) {
                    desc.objectId = 0x144;
                    desc.count = 0x14;
                    desc.flags = 0;
                    desc.x = (func_8002A8C0(object->angle) * 18.0f) + object->x;
                    desc.y = object->y;
                    desc.z = (func_8002A8BC(object->angle) * 18.0f) + object->z;
                    desc.parent = object;
                    desc.scale = object->scale;
                    spawned = func_8000590C(&desc, 1);
                    if (spawned != NULL) {
                        spawned->mode = 0;
                        state->target = spawned;
                    }
                    func_80002FE0(0x273, object->x, object->y, object->z, 4,
                                  NULL);
                    state->step = 4;
                } else if ((state->step == 4) && (state->event != 0)) {
                    func_8005AD64(object, 0, -1, 0.0f);
                    state->step = 0;
                    state->mode = 0;
                }
                break;
            case 4:
                object->angle =
                    dAngle(object->angle, state->targetAngle,
                           1.0f - Powerf(gOverlay79ApproachPower, updateRate));
                delta = (object->angle - state->targetAngle) & 0xFFFF;
                if (((state->targetAngle - object->angle) & 0xFFFF) < delta) {
                    delta = -((state->targetAngle - object->angle) & 0xFFFF);
                }
                if ((delta >= -99) && (delta < 100)) {
                    object->angle = state->targetAngle;
                    state->mode = 0;
                }
                break;
        }
    }

    distance = (state->speed * update) +
               (0.5f * state->acceleration * update * update);
    state->speed += state->acceleration * update;
    verticalDistance = (object->velocityY * update) +
                       (gOverlay79GravityHalf * update * update);
    object->velocityY += gOverlay79Gravity * update;
    dx = func_8002A8C0(object->angle) * distance;
    dz = func_8002A8BC(object->angle) * distance;

    start.x = object->x;
    start.y = object->y + state->heightOffset;
    start.z = object->z;
    end.x = start.x + dx;
    end.y = start.y + verticalDistance;
    end.z = start.z + dz;
    state->collisionFlags = 0;
    trackMakePolylist(1, &start, &end, &state->heightOffset, NULL, 1);
    if (func_80010900(&start, &end, state->heightOffset, object,
                      gOverlay79CollisionData) != 0) {
        object->velocityY = 0.0f;
    }
    func_80008128(object, end.x - start.x, end.y - start.y, end.z - start.z);
    object->velocityX = (end.x - start.x) / update;
    object->velocityZ = (end.z - start.z) / update;
    factor = gOverlay79ModeFactors[object->mode];
    if (object->mode == 0) {
        factor *= -state->speed;
    }
    state->previousHeight = object->floorHeight;
    state->event = func_8005ABA8(object, factor, update);
    if (state->effect != NULL) {
        func_800031C0(state->effect, object->x, object->y, object->z);
    }
    if (object->mode == 3) {
        object->flags |= 1;
        partUpdateTriggers(object, updateRate);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o079/func_overlay_079_F0000134_18CD0D4/func_overlay_079_F0000134_18CD0D4.s")
#endif
