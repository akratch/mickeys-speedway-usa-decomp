#include "overlays/overlay_025.h"

/* Pinned DKR v77/v80 and JFG scans found no exact initializer donor. */

/*
 * Plateau (2026-08-24): the closest flag-lattice result uses -O2 -mips3
 * with -Wab,-r4300_mul and is exact-size, first differing at function
 * offset 0x14.  Its bounded permuter score improved from 430 to 245 but
 * remained non-exact.  The canonical -mips2 candidate is four bytes short;
 * the remaining blocker is the register/stack-home scheduling web and its
 * trailing alignment instruction.
 */
#ifdef NON_MATCHING
void overlay25InitializeEffect(Overlay25Object *object,
                               const Overlay25Init *init) {
    Overlay25InitState *state;
    Overlay25Owner *owner;
    s32 paletteIndex;

    state = &object->state->init;
    state->lifetime = 600;
    state->duration = 60;
    state->currentValue = 0.0f;

    owner = init->owner;
    state->owner = owner;

    if (init->useOwner != 0 && owner != NULL) {
        Overlay25OwnerState *ownerState = owner->state;
        s32 combinedAngle = ownerState->baseAngle + ownerState->relativeAngle;

        state->activeDuration = 60;
        state->velocityX =
            (overlay25SinReloc(combinedAngle) * ownerState->scale) +
            (overlay25SinReloc(ownerState->baseAngle) * -32.0f);
        state->lift = 16.0f;
        state->velocityZ =
            (overlay25CosReloc(combinedAngle) * ownerState->scale) +
            (overlay25CosReloc(ownerState->baseAngle) * -32.0f);
    } else {
        state->activeDuration = 0;
        object->flags |= 0x0800;
    }

    if (gOverlay25GlobalFlagsReloc & 0x10) {
        paletteIndex = overlay25RandomReloc(0, 7) * 3;
    } else {
        paletteIndex = 9;
    }
    state->color[0] = gOverlay25ColorsReloc[paletteIndex + 0];
    state->color[1] = gOverlay25ColorsReloc[paletteIndex + 1];
    state->color[2] = gOverlay25ColorsReloc[paletteIndex + 2];

    if (object->vector != NULL) {
        object->vector->x = 0.0f;
        object->vector->y = 0.0f;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o025/overlay25InitializeEffect/func_overlay_025_F0000000_1879C88.s")
#endif

/*
 * Plateau (2026-08-24): the exact-size C candidate scored 2335, improving
 * to 1705 in the bounded permuter run; its first mismatch is at function
 * offset 0x0.  The flag lattice produced no different leading candidate.
 * The remaining blocker is the whole-function register, stack-home, and
 * scheduling web; the lowest-score permutation did not preserve semantics.
 */
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

/* No corresponding DKR/JFG source or object match was found. */
void overlay25SetVectorFlags(s32 unused0, Overlay25Vector *out, s32 unused2,
                             s32 unused3, Overlay25Source *source,
                             Overlay25VectorObject *object) {
    Overlay25VectorState *state;

    state = object->state;
    out->x = source->vector.x;
    out->y = source->vector.y;
    out->z = source->vector.z;
    if ((gOverlay25Threshold < source->value) || (source->flags & 0x10000000)) {
        state->flags |= 2;
        return;
    }
    state->flags |= 4;
}
