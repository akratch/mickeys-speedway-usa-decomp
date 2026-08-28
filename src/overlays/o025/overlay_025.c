#include "overlays/overlay_025.h"

/* Workbench: structure-mismatch, 70 raw differing words, first mismatch +0x14.
 * Frame/CFG/five calls and FP operation shape are exact; target retains one pipeline nop.
 * Structural gap: s0/s1 carrier allocation and that one target nop remain. */
#ifdef NON_MATCHING
void overlay25InitializeEffect(Overlay25Object *object,
                               const Overlay25Init *init) {
    Overlay25OwnerState *ownerState;
    s32 combinedAngle;
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
        ownerState = owner->state;
        combinedAngle = ownerState->baseAngle + ownerState->relativeAngle;

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
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o025/overlay_025/func_overlay_025_F0000000_1879C88.s")
#endif

/*
 * Plateau (2026-08-25): -O2 -mips2 emits the exact 0x40C-byte size. Naming
 * the two update-rate terms reduced the residual from 140 to 127 differing
 * words (18 opcode mismatches), with the first mismatch at function offset
 * 0x0. The candidate frame is 0xC8 bytes versus the target's 0xA0 bytes;
 * mutually exclusive scopes and declaration reordering regressed to +0x10
 * bytes and 216 differing words. A bounded ten-minute permuter run lowered
 * its imported score from 3090 to 2605 only by adding a vacuous guard and a
 * shared constant; the clean shared-constant form regressed to 164 differing
 * words. The blocker remains the combined stack-home and register schedule
 * across the movement and query branches.
 * R3 revisit: all 119 flag groups reconfirmed 127 words and first mismatch
 * +0x0 at the exact 0x40C boundary. Reversing the position/radius/object-array
 * declarations and scoping those locals to either branch emitted the same
 * object, so neither accounts for the target's 0xA0 versus 0xC8 frame.
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
        s32 extraSteps;
        f32 accum;
        f32 moveX;
        f32 moveZ;
        f32 velocityX;
        f32 velocityZ;

        state->activeDuration -= updateRate;
        object->value = object->transform->value + object->transform->value;
        remaining = updateRate - 2;
        extraSteps = updateRate - 1;
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
        if (extraSteps != 0) {
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
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o025/overlay_025/func_overlay_025_F000017C_1879E04.s")
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
