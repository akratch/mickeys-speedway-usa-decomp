#include "PR/ultratypes.h"

typedef struct Overlay25Object Overlay25Object;

typedef struct Overlay25Vector {
    f32 x, y, z;
} Overlay25Vector;

typedef struct Overlay25Transform {
    f32 value;
    u8 pad04[0x50];
    f32 scaleX;
    f32 scaleY;
} Overlay25Transform;

typedef struct Overlay25EffectState {
    s16 lifetime;
    s8 duration;
    s8 activeDuration;
    f32 multiplier;
    f32 velocityX;
    f32 lift;
    f32 velocityZ;
    u32 flags;
    u8 pad18[4];
    Overlay25Object *owner;
} Overlay25EffectState;

typedef struct Overlay25EntityState {
    u8 pad000[4];
    f32 height;
    u8 pad008[0x341];
    u8 enabled;
    u8 pad34A[0x6C];
    s16 ownerHitCount;
    s16 selfHitCount;
} Overlay25EntityState;

typedef union Overlay25State {
    Overlay25EffectState effect;
    Overlay25EntityState entity;
} Overlay25State;

struct Overlay25Object {
    u8 pad00[6];
    s16 flags;
    f32 value;
    f32 x;
    f32 y;
    s32 z;
    u8 pad18[0x28];
    Overlay25Transform *transform;
    u8 pad44[8];
    Overlay25Vector *vector;
    u8 pad50[0x14];
    Overlay25State *state;
};

typedef struct Overlay25Status { u8 type; } Overlay25Status;

extern void overlay25SetVectorFlagsReloc(void);
extern void overlay25DestroyReloc(Overlay25Object *object);
extern void overlay25MoveReloc(Overlay25Object *object, f32 x, f32 y, f32 z);
extern void overlay25SweepReloc(s32 mode, Overlay25Vector *position,
                                Overlay25Vector *movement, f32 *radius,
                                s32 arg4, s32 arg5);
extern s32 overlay25TraceReloc(Overlay25Vector *position,
                               Overlay25Vector *movement, f32 radius,
                               Overlay25Object *object,
                               void (*callback)(void));
extern s32 overlay25QueryObjectsReloc(f32 x, f32 y, s32 z, f32 radius,
                                      s32 mode, Overlay25Object **objects);
extern s32 overlay25CanHitReloc(Overlay25Object *object,
                                Overlay25EntityState *state);
extern void overlay25ApplyHitReloc(Overlay25Object *owner,
                                   Overlay25Object *object);
extern Overlay25Status *overlay25GetStatusReloc(void);
extern void overlay25NotifyHitReloc(Overlay25Object *object);

#ifdef NON_MATCHING
void overlay25UpdateEffect(Overlay25Object *object, s32 updateRate) {
    Overlay25EffectState *state = &object->state->effect;
    Overlay25Vector position;
    f32 radius;
    Overlay25Object *objects[10];
    s32 hitSomething;

    if (state->activeDuration != 0) {
        s32 remaining;
        f32 accum;
        f32 moveX;
        f32 moveZ;
        f32 velocityX;
        f32 velocityZ;

        state->activeDuration -= updateRate;
        object->value = object->transform->value + object->transform->value;
        if (state->activeDuration <= 0) {
            overlay25DestroyReloc(object);
            return;
        }

        accum = state->lift;
        velocityX = state->velocityX;
        velocityZ = state->velocityZ;
        moveX = velocityX;
        moveZ = velocityZ;
        state->lift = accum - 1.1034483f;
        if ((updateRate - 1) != 0) {
            remaining = updateRate - 2;
            do {
                f32 current = state->lift;
                moveX += velocityX;
                state->lift = current - 1.1034483f;
                moveZ += velocityZ;
                accum += current;
            } while (remaining--);
        }

        position.x = object->x;
        position.y = object->y;
        position.z = *(f32 *)&object->z;
        overlay25MoveReloc(object, moveX, accum, moveZ);

        radius = 4.0f;
        overlay25SweepReloc(1, &position, (Overlay25Vector *)&object->x,
                            &radius, 0, 0);
        if (overlay25TraceReloc(&position, (Overlay25Vector *)&object->x,
                                radius, object, overlay25SetVectorFlagsReloc)) {
            if (state->flags & 4) {
                overlay25DestroyReloc(object);
                return;
            }
            state->activeDuration = 0;
            object->value = object->transform->value;
            state->multiplier = 0.4f;
            object->flags |= 0x800;
        }
    } else {
        s32 count;

        state->duration -= updateRate;
        state->lifetime -= updateRate;
        if (state->duration < 0) {
            state->duration = 0;
        }

        if (state->lifetime <= 0) {
            updateRate = -state->lifetime;
            state->lifetime = 0;
            state->multiplier -= 0.4f * (f32)updateRate;
            if (state->multiplier <= 0.1f) {
                overlay25DestroyReloc(object);
                return;
            }
        } else {
            f32 queryRadius;
            s32 index;
            Overlay25Object **cursor;

            state->multiplier += 0.4f * (f32)updateRate;
            if (state->multiplier > 4.0f) {
                state->multiplier = 4.0f;
            }

            queryRadius = object->value * state->multiplier * 16.0f;
            count = overlay25QueryObjectsReloc(object->x, object->y, object->z,
                                               queryRadius, 1, objects);
            hitSomething = 0;
            if (count != 0) {
                index = count - 1;
                cursor = &objects[index];
                do {
                    Overlay25Object *other = *cursor;
                    f32 delta = other->y - object->y;

                    if ((other != state->owner) || (state->duration == 0)) {
                        Overlay25EntityState *otherState = &other->state->entity;
                        if ((otherState->height < -5.0f) &&
                            (delta > -24.0f) && (delta < 24.0f) &&
                            (otherState->enabled != 0)) {
                            hitSomething = 1;
                            if (overlay25CanHitReloc(other, otherState)) {
                                Overlay25EntityState *ownerState;
                                overlay25ApplyHitReloc(state->owner, other);
                                ownerState = &state->owner->state->entity;
                                ownerState->ownerHitCount++;
                                otherState->selfHitCount++;
                                if (overlay25GetStatusReloc()->type == 5) {
                                    overlay25NotifyHitReloc(other);
                                }
                            }
                        }
                    }
                    cursor--;
                } while (index--);
            }
            if (hitSomething != 0) {
                state->lifetime = 0;
            }
        }
    }

    if (object->vector != NULL) {
        object->vector->x = state->multiplier * object->transform->scaleX;
        object->vector->y = state->multiplier * object->transform->scaleY;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o025/overlay25UpdateEffect/func_overlay_025_F000017C_1879E04.s")
#endif
