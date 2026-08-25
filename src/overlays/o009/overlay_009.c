#include "overlays/overlay_009.h"
#include "tools/m2c/m2c_macros.h"

#undef NULL
#define NULL 0

/* Retail's runtime relocation table addresses this pool from offset 0x20. */
static const u8 sOverlay9ConstantPoolBase[0x20] = { 0 };

/*
 * Overlay 9, ADR 0006 consolidation. Functions remain in retail ROM order.
 * The module uses the R4300 multiply-hazard schedule; applying that flag to
 * the intervening empty overlay9Ignore function does not change its bytes.
 */

/*
 * Plateau: the best build is 36 bytes over retail and differs in 247/336
 * words from entry +0x0. The flag lattice does not move the full private
 * frame/register/schedule web, and the bounded permuter cannot run because
 * tools/permuter/import.py is absent.
 */
#ifdef NON_MATCHING
void func_overlay_009_F0000000_1866678(void *object, s32 steps) {
    f32 vector[3];
    s16 angles[3];
    void *savedEntry;
    void *entry;
    f32 savedY;
    f32 stepFloat;
    f32 current;
    f32 target;
    f32 rate;
    f32 velocity;
    void *state;
    void *entryData;
    s32 remaining;
    s32 result;
    s8 timer;

    state = M2C_FIELD(object, void **, 0x64);
    D_410 = (s16 *)((u8 *)state + 0x1B8);
    M2C_FIELD(object, s32 *, 0x80) = 0;
    savedEntry = *M2C_FIELD(object, void ***, 0x68);
    ext_o0_1ee14(state, M2C_FIELD(state, s8 *, 0));
    G_rt_458c4 = D_390;

    if (M2C_FIELD(state, f32 *, 4) < -30.0f) {
        M2C_FIELD(state, f32 *, 4) = -30.0f;
    }
    if (M2C_FIELD(state, f32 *, 4) > 30.0f) {
        M2C_FIELD(state, f32 *, 4) = 30.0f;
    }
    if (M2C_FIELD(state, f32 *, 8) < -30.0f) {
        M2C_FIELD(state, f32 *, 8) = -30.0f;
    }
    if (M2C_FIELD(state, f32 *, 8) > 30.0f) {
        M2C_FIELD(state, f32 *, 8) = 30.0f;
    }

    ext_o0_1d4c0(object, state);
    angles[0] = -M2C_FIELD(state, s16 *, 0xF0);
    angles[1] = -M2C_FIELD(object, s16 *, 2);
    vector[2] = 0.0f;
    angles[2] = -M2C_FIELD(object, s16 *, 4);
    vector[0] = 0.0f;
    vector[1] = -1.0f;
    ext_o0_29adc(angles, vector);
    M2C_FIELD(state, f32 *, 0x60) = vector[0];
    M2C_FIELD(state, f32 *, 0x64) = vector[1];
    M2C_FIELD(state, f32 *, 0x5C) = vector[2];

    result = ext_o0_1312c(M2C_FIELD(object, f32 *, 0xC),
                           M2C_FIELD(object, f32 *, 0x14),
                           (u8 *)state + 0x68, 0x10000, 0);
    if ((result & 0x10000) &&
        ((M2C_FIELD(object, f32 *, 0x10) - 16.0f) <
         M2C_FIELD(state, f32 *, 0x68))) {
        M2C_FIELD(state, f32 *, 0x6C) =
            M2C_FIELD(state, f32 *, 0x68) -
            (M2C_FIELD(object, f32 *, 0x10) - 16.0f);
    } else {
        M2C_FIELD(state, f32 *, 0x6C) = 0.0f;
    }

    if ((M2C_FIELD(state, u8 *, 0x16C) == 0) ||
        (M2C_FIELD(state, u8 *, 0x16C) == 1)) {
        func_overlay_009_F0000744_1866DBC(object, state, &D_2D0, steps);
    }
    stepFloat = (f32)steps;
    func_overlay_009_F0000CE4_186735C(object, state, &D_2D0, stepFloat);
    func_overlay_009_F00010A4_186771C(object, state, stepFloat);
    func_overlay_009_F0000F6C_18675E4(object, &D_2D0, steps);
    func_overlay_009_F0000540_1866BB8(object, state, &D_2D0, steps);

    D_2F0 = (s16)(D_2F0 + (((s32)(3072.0f * D_2EC) + 0x400) * steps));
    *D_410++ = 0x22;
    *D_410++ = D_2F0;
    D_2F2 = (s16)(D_2F2 +
        (((s32)(3072.0f * (D_2D8 - 1.0f)) + 0x400) * steps));
    *D_410++ = 0x24;
    *D_410++ = D_2F2;
    D_2FA = (s16)(D_2FA + (steps << 8));
    *D_410++ = 0x2000;

    if ((M2C_FIELD((u8 *)M2C_FIELD(object, void **, 0x40) +
                       M2C_FIELD(object, u8 *, 0x93), s8 *, 0x1E) == 0) &&
        (savedEntry != NULL) && (M2C_FIELD(savedEntry, s16 *, 8) != 0)) {
        entryData = M2C_FIELD(savedEntry, void **, 0);
        savedY = M2C_FIELD(object, f32 *, 0x10);
        velocity = M2C_FIELD(state, f32 *, 4);
        if ((velocity < -2.0f) ||
            (M2C_FIELD(state, s32 *, 0x42C) < -0x14) ||
            (velocity > 2.0f) ||
            (M2C_FIELD(state, s32 *, 0x42C) >= 0x15)) {
            target = 0.0f;
            rate = D_394;
        } else {
            target = 4.0f;
            rate = D_398;
        }
        remaining = steps - 1;
        if (steps != 0) {
            current = D_2FC;
            do {
                current += (target - current) * rate;
            } while (remaining--);
            D_2FC = current;
        }
        M2C_FIELD(object, f32 *, 0x10) +=
            D_2FC * ext_o0_2a470(D_2FA);
        ext_o0_5aac4(savedEntry, entryData, object);
        ext_o0_19668(object, savedEntry, M2C_FIELD(object, void **, 0x50),
                      M2C_FIELD((u8 *)savedEntry +
                          (M2C_FIELD(savedEntry, s16 *, 0xA) * 4), void **, 0xC));
        M2C_FIELD(savedEntry, s16 *, 8) = 0;
        M2C_FIELD(object, f32 *, 0x10) = savedY;
    }

    M2C_FIELD(state, s8 *, 0x186) = 0;
    ext_o0_1d510(object, state, NULL, NULL, steps);
    if (M2C_FIELD(state, u8 *, 0x349) != 0) {
        if (M2C_FIELD(state, u8 *, 0x16C) == 1) {
            void *handle = M2C_FIELD(state, void **, 0xB8);
            M2C_FIELD(state, u8 *, 0x16C) = 0;
            M2C_FIELD(state, s8 *, 0x16E) = 8;
            if (handle != NULL) {
                ext_o0_2d98(handle);
            }
            ext_o0_2b90(6, M2C_FIELD(object, f32 *, 0xC),
                        M2C_FIELD(object, f32 *, 0x10),
                        M2C_FIELD(object, f32 *, 0x14), 4,
                        (u8 *)state + 0xB8);
        } else {
            timer = M2C_FIELD(state, s8 *, 0x16E);
            if (timer > 0) {
                M2C_FIELD(state, s8 *, 0x16E) = timer - steps;
            }
        }
    } else {
        M2C_FIELD(state, s8 *, 0x16E) = 0;
    }
    ext_o0_3e99c(object, steps);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o009/overlay_009/func_overlay_009_F0000000_1866678.s")
#endif
/*
 * Plateau: the best build differs in 8/129 words, first at +0x4C; the body,
 * size, and CFG agree, but retail swaps the callee-saved FPR web used for the
 * two limits and two thresholds. Declaration reordering did not move the web,
 * and the bounded permuter is unavailable.
 */
#ifdef NON_MATCHING
void func_overlay_009_F0000540_1866BB8(O9Angle *angle, void *unused,
                                       O9Motion *motion, s32 steps) {
    s32 delta;

    if (steps--) {
        f32 lower;
        f32 upper;
        f32 upperThreshold;
        f32 lowerThreshold;
        f32 damping;

        upperThreshold = D_C;
        lowerThreshold = D_10;
        damping = D_14;
        upper = 16.0f;
        lower = -16.0f;
        do {
            delta = ext_o0_2a5bc(motion->angle, -angle->angle);
            if ((delta >= -0x3F) && (delta < 0x40) &&
                (motion->velocity > lower) && (motion->velocity < upper)) {
                motion->velocity = 0.0f;
                motion->angle = -angle->angle;
            } else {
                motion->velocity += 20.0f * ext_o0_2a470(delta);
                motion->angle += (s32) motion->velocity;
            }
            motion->velocity *= damping;
            if ((motion->velocity > lowerThreshold) &&
                (motion->velocity < upperThreshold)) {
                motion->velocity = 0.0f;
            }
        } while (steps--);
    }

    delta = motion->angle;
    if (delta < -0x4000) delta = -0x8000 - delta;
    if (delta >= 0x4001) delta = 0x8000 - delta;
    *D_0++ = 0xB;
    *D_0++ = motion->angle;
    *D_0++ = 0xA;
    *D_0++ = -delta;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o009/overlay_009/func_overlay_009_F0000540_1866BB8.s")
#endif

/*
 * Plateau: the R4300-multiply build differs in 5/158 words, first at +0x140;
 * every mismatch is the handle test's v1 allocation versus retail v0.
 * Direct, truth-value, scoped-local, live-local, and register-local forms all
 * retained that web or worsened it; the bounded permuter is unavailable.
 */
#ifdef NON_MATCHING
void func_overlay_009_F0000744_1866DBC(O9OutputRecord *output, O9OutputControl *control,
                                       O9OutputState *state, s32 updateCount) {
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
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o009/overlay_009/func_overlay_009_F0000744_1866DBC.s")
#endif

void func_overlay_009_F00009BC_1867034(s16 *angleOut, O9InputControl *control,
                                       O9InputState *state) {
    f32 target;
    f32 one = 1.0f;
    s32 input;

    input = control->inputX;
    if (input < -59) {
        target = -1.0f;
    } else if (input >= 60) {
        target = one;
    } else {
        target = (f32)input / 60.0f;
    }
    state->x += (target - state->x) * 0.02f;
    if (state->x > 0.0f) {
        control->lean = -state->x * 20.0f;
    } else {
        control->lean = -state->x * 10.0f;
    }

    input = control->inputY;
    if (input < -59) {
        target = -1.0f;
    } else if (input >= 60) {
        target = one;
    } else {
        target = (f32)input / 60.0f;
    }
    state->y += (-target - state->y) * 0.075f;

    if (control->flags & 0x10) {
        state->throttle += (2.0f - state->throttle) * 0.025f;
    } else {
        state->throttle += (one - state->throttle) * 0.05f;
    }

    state->turnRate +=
        ((state->throttle * state->y * 365.0f) - state->turnRate) * 0.1f;
    control->angleStep = (s16)(s32)state->turnRate;
    control->angle += control->angleStep;
    *angleOut = control->angle;

    if (control->flags & 0x8000) {
        state->acceleration += 0.1f;
        if (state->acceleration > 3.0f) {
            state->acceleration = 3.0f;
        }
    } else if (control->flags & 0x4000) {
        state->acceleration *= 0.95f;
        if ((state->acceleration > -0.01f) &&
            (state->acceleration < 0.01f)) {
            state->acceleration = 0.0f;
        }
    } else {
        state->acceleration -= 0.1f;
    }

    state->position += state->acceleration;
    if (state->position < 40.0f) {
        state->position = (40.0f - state->position) + 40.0f;
        state->acceleration *= -0.4f;
        if ((state->acceleration > -0.1f) &&
            (state->acceleration < D_4C)) {
            state->acceleration = 0.0f;
            state->position = 40.0f;
            return;
        }
    } else if (state->position > 200.0f) {
        state->position = 200.0f;
    }
}

/* Workbench: mixed constant/relocation residual; 4/162 words differ, first +0xC8.
 * Constant-audit tried explicit local layouts, padding widths, and D_50 binding.
 * Angle home remains +0x32 vs +0x2A; the 0.65f pool addend is +0x4C vs +0x50. */
#ifdef NON_MATCHING
void func_overlay_009_F0000CE4_186735C(O9IntegrateOutput *out, O9IntegrateControl *control,
                                       void *unused, f32 step) {
    f32 xVelocity;
    f32 yVelocity;
    f32 zVelocity;
    volatile f32 unusedExtra;
    f32 xExtra;
    f32 yExtra;
    f32 zExtra;
    s16 angle = control->angle;
    f32 fraction;

    if (control->active != 0) {
        f32 distance = (control->velocity * step) +
            (0.5f * control->acceleration * step * step);
        xExtra = control->dirX * distance;
        yExtra = control->dirY * distance;
        zExtra = control->dirZ * distance;
        if (distance < 0.0f) {
            control->velocity = 0.0f;
            control->acceleration = 0.0f;
            control->active = 0;
        }
        fraction = 1.0f - (control->velocity / control->speedLimit);
        control->velocity += control->acceleration * step;
        xVelocity = ext_o0_2a470(angle) * control->scaleX * fraction;
        zVelocity = ext_o0_2a46c(angle) * control->scaleX * fraction;
    } else {
        xExtra = 0.0f;
        yExtra = 0.0f;
        zExtra = 0.0f;
        xVelocity = ext_o0_2a470(angle) * control->scaleX;
        zVelocity = ext_o0_2a46c(angle) * control->scaleX;
    }
    yVelocity = 0.0f;
    out->zero = 0.0f;
    if (control->mode == 1) {
        xVelocity *= 0.65f;
        zVelocity *= 0.65f;
        if ((control->scaleX < -0.5f) || (control->scaleX > 0.5f))
            control->scaleX *= 0.65f;
        else
            control->scaleX = 0.0f;
        if ((control->scaleZ < -0.5f) || (control->scaleZ > 0.5f))
            control->scaleZ *= 0.65f;
        else
            control->scaleZ = 0.0f;
    }
    ext_o0_7cd8(out, (xVelocity * step) + xExtra,
                 (yVelocity * step) + yExtra,
                 (zVelocity * step) + zExtra);
    fraction = 1.0f / step;
    out->dx = (out->x - control->originX) * fraction;
    out->dz = (out->z - control->originZ) * fraction;
    ext_o0_1d920(out, control, step);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o009/overlay_009/func_overlay_009_F0000CE4_186735C.s")
#endif

void func_overlay_009_F0000F6C_18675E4(O9Point *point, O9Height *offset,
                                       s32 steps) {
    f32 current, result, distance, candidate;
    s32 count;
    O9Hit **hits;
    s32 i;

    count = ext_o0_1353c(point->x, point->z, 0x1000, &hits);
    current = point->y;
    result = current;
    distance = 1000.0f;
    i = count - 1;
    if (count != 0) {
        do {
            candidate = current - hits[i]->height;
            if (candidate < 0.0f) candidate = 0.0f;
            if (candidate < distance) {
                distance = candidate;
                result = hits[i]->height + offset->height;
            }
        } while (i--);
    }
    candidate = current;
    while (steps--) {
        candidate += (result - candidate) * D_54;
    }
    result += 40.0f - offset->height;
    if (candidate < result) candidate = result;
    ext_o0_7cd8(point, 0.0f, candidate - current, 0.0f);
}

void overlay9Ignore(volatile s32 arg0, volatile s32 arg1, volatile s32 arg2) {
}

/*
 * Plateau: the R4300-multiply build is 4 bytes short and differs in 80/283
 * words, first at +0x88. Plain -O2 regresses to 209 words; the remaining gap
 * is a coupled frame/register web across the smoothing loops, and the bounded
 * permuter is unavailable.
 */
#ifdef NON_MATCHING
void func_overlay_009_F00010B4_186772C(O9MotionResult *out, O9MotionOwner *owner,
                                       f32 stepsFloat) {
    O9MotionState *state = owner->state;
    s32 mode = state->mode & 3;
    s32 steps;
    s32 i;
    s32 tableIndex;
    s16 targetAngle;
    f32 baseX, baseY, baseZ;
    f32 crossA, dot, crossB;
    f32 trigA, trigB, yawA, yawB;
    f32 targetX, targetY;
    f32 cross, targetTilt, speedTarget, blend;

    ext_o0_210b4(60.0f, 0);
    if (state->flags & 8) D_388[mode]++;
    D_388[mode] &= 3;
    tableIndex = D_388[mode] + ((ext_o0_214c8() & 3) * 4);
    targetX = D_300[tableIndex] + (D_2D0 * 75.0f);
    targetY = D_340[tableIndex];
    steps = (s32) stepsFloat;
    targetAngle = D_380[mode];
    i = steps - 1;

    if (steps != 0) {
        do {
            state->angle += ext_o0_2a5bc(state->angle,
                                         0x8000 - state->angleTarget) >> 4;
        } while (i--);
        i = steps - 1;
    }
    if (steps != 0) {
        do {
            out->targetAngle += ext_o0_2a5bc(out->targetAngle,
                                             targetAngle) >> 4;
        } while (i--);
        i = steps - 1;
    }
    if (steps != 0) {
        blend = D_58;
        do {
            out->smoothX += (targetX - out->smoothX) * blend;
            out->smoothY += (targetY - out->smoothY) * blend;
        } while (i--);
        i = steps - 1;
    }

    trigA = ext_o0_2a470(0x8000 - state->angle);
    trigB = ext_o0_2a46c(0x8000 - state->angle);
    yawA = ext_o0_2a470(out->targetAngle - targetAngle);
    yawB = ext_o0_2a46c(out->targetAngle - targetAngle);
    cross = (out->smoothX * yawB) - (out->smoothY * yawA);
    crossA = cross * trigA;
    dot = (out->smoothX * yawA) + (out->smoothY * yawB);
    crossB = cross * trigB;

    if (state->direction == 0) targetTilt = -10.0f;
    else targetTilt = 10.0f;
    if (steps != 0) {
        blend = D_70;
        do {
            state->tilt += (targetTilt - state->tilt) * blend;
        } while (i--);
        i = steps - 1;
    }

    baseX = owner->x + (state->axisX * state->tilt);
    baseY = owner->y + (state->axisY * state->tilt);
    baseZ = owner->z + (state->axisZ * state->tilt);
    trigA = ext_o0_2a470(state->angle + 0x4000);
    trigB = ext_o0_2a46c(state->angle + 0x4000);
    blend = D_78;
    speedTarget = state->input;
    if (speedTarget < 0.0f) speedTarget = -speedTarget;
    if (speedTarget > 1.0f) speedTarget = 1.0f;
    speedTarget *= (f32) state->speedScale * D_7C;
    if (steps != 0) {
        do {
            state->speed += (speedTarget - state->speed) * blend;
        } while (i--);
        i = steps - 1;
    }

    baseX += state->speed * trigA;
    baseZ -= state->speed * trigB;
    out->x = baseX + crossA;
    out->y = baseY + dot;
    out->z = baseZ + crossB;
    out->angle = state->angle;
    if (steps != 0) {
        do {
            out->bank += ext_o0_2a5bc(out->bank,
                                      owner->bankLimit >> 1) >> 5;
        } while (i--);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o009/overlay_009/func_overlay_009_F00010B4_186772C.s")
#endif
