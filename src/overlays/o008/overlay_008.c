#include "overlays/overlay_008.h"

/* Overlay 008, ADR 0006 consolidation: functions remain in ROM order. */

void overlay8Ignore(volatile s32 unused) {
}

/* DKR v77/v80 and JFG checks found no exact donor for this indexed selector. */
void *overlay8GetIndexed(Overlay8IndexedObject *object) {
    s32 index = object->index;
    void *result;

    if (index < 0 || index >= 10) {
        index = 0;
    }
    if (gOverlay8IndexMode == 0) {
        result = gOverlay8Primary[index];
    } else {
        result = gOverlay8Secondary[index];
    }
    return result;
}

/* Workbench p3: structure-mismatch; 508/527 candidate/target instructions, 477 words from +0x0.
 * Lever: constant audit plus array scopes, query declaration order, and a register hint.
 * Remains: query call-spill ownership leaves the candidate frame 24 bytes too large. */
#ifdef NON_MATCHING
void func_overlay_008_F0000058_185DDB0(O8P0058Owner *owner,
                                       s32 updateRate) {
    O8P0058State *state;
    f32 vector[3];
    s16 angles[3];
    O8P0058Surface surfaces[8];
    O8P0058Surface *surface;
    f32 minimum;
    f32 maximum;
    f32 floorHeight;
    f32 update;
    f32 value;
    s32 count;
    s32 index;
    s32 present;
    O8P0058Query *query;

    state = owner->state64;
    gOverlay8Buffer = (s16 *)((u8 *)state + 0x1B8);
    gOverlay8Value = 0;
    D_14 = 0;
    D_18 = 0;
    owner->flags80 = 0;
    D_10 = 25.0f;
    state->value70 = 0.0f;

    if ((state->disabled18D != 0) || (state->gate158 != 0) ||
        (state->reset170 != 0) || (owner->mode3B == 0x18) ||
        (state->gate3FA != 0)) {
        o8P0058ResetReloc(state, 1);
    }
    o8P0058ModeReloc(state, state->mode0);

    if (gO8P0058MirrorGateReloc != 0) {
        state->signed428 = -state->signed428;
        state->signed430 = -state->signed430;
    }

    query = o8P0058AcquireReloc(state);
    present = gO8P0058PresentReloc;
    gO8P0058ResultReloc = query->initial0;
    if (present != 0) {
        if (((state->modeFlags420 & 0x8000) != 0) &&
            (state->active183 == 0x80)) {
            state->active183 = (u8)gO8P0058ActiveReloc;
        }
        state->value42C = 0;
        state->value434 = 0;
        state->flags41C = (state->flags41C & 0x8008) | 0x4000;
    } else {
        if ((state->active183 & 0x80) != 0) {
            if (state->active183 == 0x84) {
                if (gO8P0058SpawnGateReloc != 0) {
                    o8P0058SpawnReloc(owner, 0x18, -1, 0);
                }
            } else if (state->active183 != 0x80) {
                state->alternate184 = 1;
            }
            state->active183 = 0;
        }
    }

    minimum = D_B0;
    maximum = D_B4;
    if (state->lower4 < minimum) {
        state->lower4 = minimum;
    }
    if (maximum < state->lower4) {
        state->lower4 = maximum;
    }
    if (state->upper8 < minimum) {
        state->upper8 = minimum;
    }
    if (maximum < state->upper8) {
        state->upper8 = maximum;
    }

    o8P0058OrientReloc(owner, state);
    angles[0] = -state->angleF0;
    angles[1] = -owner->angle2;
    angles[2] = -owner->angle4;
    vector[0] = 0.0f;
    vector[1] = -1.0f;
    vector[2] = 0.0f;
    o8P0058RotateReloc(angles, vector);
    state->direction60 = vector[0];
    state->direction64 = vector[1];
    state->direction5C = vector[2];

    count = o8P0058SurfaceReloc(owner->xC, owner->z14, 0, 0x08010000,
                                surfaces);
    floorHeight = -32768.0f;
    state->surface68 = -32768.0f;
    if (count != 0) {
        index = count - 1;
        surface = &surfaces[index];
        do {
            if ((surface->flags4 & 0x10000) != 0) {
                state->surface68 = surface->height0;
            }
            if ((surface->flags4 & 0x08000000) != 0) {
                floorHeight = surface->height0;
            }
            surface--;
        } while (index-- != 0);
    }

    if (owner->y10 < state->surface68) {
        state->surfaceActive2 = 1;
        state->surface6C = state->surface68;
        if ((state->surface68 - owner->y10) >= 25.0f) {
            gO8P0058ResultReloc *= 0.125f;
        }
    } else {
        state->surfaceActive2 = 0;
        state->surface6C = 0.0f;
    }

    if (owner->y10 < floorHeight) {
        state->reset170 = 1;
        if ((state->surfaceActive2 != 0) && (state->surfaceMode3 != 1)) {
            o8P0058CollisionReloc(owner, state);
        }
    }

    if (state->active16A != 0) {
        D_8 = 1.0f;
        D_C = 1.0f;
        o8P0058EffectReloc(state, 0x28, 0.15f, &D_C);
    } else {
        D_8 = 0.0f;
        D_C = 0.0f;
        index = state->selectors320[0] & 0xF;
        D_8 += D_310[index];
        D_C += query->heights148[D_350[index]];
        index = state->selectors320[1] & 0xF;
        D_8 += D_310[index];
        D_C += query->heights148[D_350[index]];
        index = state->selectors320[2] & 0xF;
        D_8 += D_310[index];
        D_C += query->heights148[D_350[index]];
        index = state->selectors320[3] & 0xF;
        D_8 += D_310[index];
        D_C += query->heights148[D_350[index]];
        D_8 *= 0.25f;
        D_C *= 0.25f;

        if (state->override172 != 0) {
            D_C = state->override17C;
        }
        if (state->surfaceActive2 != 0) {
            D_C = D_B8;
        }
        if ((state->peerD4 != 0) && (D_BC < D_8)) {
            D_8 += (D_BC - D_8) * state->peerD4->state64->blend14;
        }
    }

    if ((state->mode16C == 0) || (state->mode16C == 1)) {
        update = (f32)updateRate;
        func_overlay_008_F0001294_185EFEC(owner, state, update);
    } else {
        update = (f32)updateRate;
    }
    func_overlay_008_F000291C_1860674((O8P291CMotion *)owner,
                                      (O8P291CState *)state, update);
    state->value70 =
        o8P0058SampleReloc(owner, state, D_10, update);

    *gOverlay8Buffer = 0x2000;
    gOverlay8Buffer++;
    o8P0058UpdateReloc(owner, state, updateRate);

    if (state->lowering349 != 0) {
        if (state->mode16C == 1) {
            state->mode16C = 0;
            state->counter16E = 8;
            if (state->resourceB8 != 0) {
                o8P0058ReleaseReloc(state->resourceB8);
            }
            o8P0058CreateReloc(6, owner->xC, owner->y10, owner->z14, 4,
                               &state->resourceB8);
            value = D_C0;
            state->bounceActive18C = 1;
            state->bounce54 = value + 1.0f;
            state->bounceVelocity3FC = value;
            o8P0058BounceReloc(state, 0x32, 0.4f);
        } else if (state->counter16E > 0) {
            state->counter16E -= updateRate;
        }
    } else {
        state->counter16E = 0;
    }

    if (state->disabled18D == 0) {
        if ((state->bounceActive18C != 0) &&
            (state->bounce54 == state->position50)) {
            state->bounceVelocity3FC *= -0.5f;
            value = state->bounceVelocity3FC;
            if ((D_C4 < value) && (value < D_C8)) {
                state->bounceActive18C = 0;
                state->bounceVelocity3FC = 0.0f;
                state->bounce54 = 1.0f;
                return;
            }
            state->bounce54 = value + 1.0f;
        }
    } else {
        state->bounceActive18C = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F0000058_185DDB0.s")
#endif

/* Workbench verdict=allocation-mismatch; 48 masked/57 raw words differ in the exact 381-word/-0x70 frame, first register mismatch +0x428.
 * Flag lattice, arm ordering, widened mask, and the bounded -mips2 permuter were tried; context lint found no undefined guard.
 * Remains: exact FP lanes but GPR temp slot 54/pool slot 64 allocation and 46 overlay relocation identities. */
#ifdef NON_MATCHING
void func_overlay_008_F0000894_185E5EC(O8Owner *owner, O8State *state,
                                       s32 updateRate) {
    void *savedResource;
    O8Node *node;
    f32 update = (f32)updateRate;
    f32 sine;
    f32 cosine;
    f32 y;
    f32 x;
    O8Node *savedNode;
    f32 sideA;
    f32 sideB;
    f32 z;
    s32 effect;
    s32 angleA;
    s32 angleB;
    s32 flags;

    node = *owner->node68;
    savedResource = node->resource;
    if (((s8 *)owner->children40)[owner->childIndex93 + 0x1e] == 0 &&
        (node != 0) && (node->active != 0)) {
        savedNode = node;
        o8Call0894Reloc(node, savedResource, owner);
        ext_o0_19668(owner, savedNode, owner->value50,
                     savedNode->items[savedNode->index]);
        ext_o8_3368(owner, state, savedResource, node, node->active);
        savedNode->active = 0;
    }

    effect = -1;
    if ((state->active181 != 0) && (owner->position28 <= gO8FloatCC)) {
        if (owner->mode3B == 0x14) effect = 0;
        else if (owner->mode3B == 0x13) effect = 1;
        else if (owner->mode3B == 0x15) effect = 2;
        if (effect != -1) {
            f32 scale = state->value84 * update;
            sideA = state->vector74 * scale;
            sideB = state->vector78 * scale;
            z = state->vector7C * scale;
            angleA = 0x1200;
            angleB = 0x1c00;
        }
        ext_o0_1eed0_target(state, 0x4b, 0x3e19999a);
    }

    if ((state->lateral4 < -3.25f) && (effect == -1)) {
        if ((state->mode34A & 3) == 3) {
            effect = 1;
            y = 0.0f;
            x = 4.0f - state->lateral4 * gO8FloatD0;
        } else if (state->mode34A & 5) {
            effect = 0;
            y = 4.0f - state->lateral4 * gO8FloatD4;
            x = 0.0f;
        } else if (state->mode34A & 0xa) {
            effect = 2;
            y = state->lateral4 * gO8FloatD8 + -4.0f;
            x = 0.0f;
        }
        if (effect != -1) {
            sine = ext_o0_2a46c(state->angle43C);
            cosine = ext_o0_2a470(state->angle43C);
            angleA = 0x2a00;
            angleB = 0x3600;
            sideA = y * sine + x * cosine;
            sideB = 0.0f;
            z = x * sine - y * cosine;
            ext_o0_1eed0_target(state, 0x28, 0x3e19999a);
        }
    }
    if (effect != -1) {
        o8Call0894EmitReloc(owner, state, effect, angleA, angleB,
                                          sideA, sideB, z, 2);
        state->timer3B3 = 0x64;
    }

    ext_o8_3278(owner, state, updateRate);
    ext_o8_2ec0(owner, state, 0, 0, updateRate);
    ext_o0_1d510(owner, state, updateRate);
    if ((state->active181 != 0) &&
        (((state->mode34A != 0) && (state->value84 == state->value80)) ||
         (owner->peer48->gate63 != 0))) {
        if (state->resourceB8 != 0) ext_o0_2d98(state->resourceB8);
        ext_o0_2b90(7, owner->valueC, owner->value10, owner->value14, 4,
                    &state->resourceB8);
        ext_o7_ccc(owner, 0x12);
    }
    ext_o8_3018(owner, state, state->value70, updateRate);

    if ((state->condition172 != 0) && (state->lateral4 < -2.0f)) {
        flags = owner->flags80 & ((~0x33) & 0xFFFFFFFFFFFFFFFFu);
        owner->flags80 = flags;
        flags |= gO8Value370;
        owner->flags80 = flags;
        owner->flags80 = flags | gO8Value3B0;
    } else if (((state->condition2 != 0) || (state->conditionD4 != 0)) &&
               ((state->lateral4 < gO8FloatDC) ||
                (state->lateral4 > gO8FloatE0))) {
        flags = owner->flags80 & ~0x33;
        owner->flags80 = flags;
        flags |= gO8Value364;
        owner->flags80 = flags;
        owner->flags80 = flags | gO8Value3A4;
    } else if (state->lateral4 < -5.0f) {
        flags = owner->flags80 | gO8Table360[state->selector322 & 0xf];
        owner->flags80 = flags;
        owner->flags80 = flags | gO8Table3A0[state->selector323 & 0xf];
    }

    flags = owner->flags80;
    if (flags & 0x10) {
        owner->flags80 = flags & ~1;
        flags = owner->flags80;
    }
    if (flags & 0x20) owner->flags80 = flags & ~2;
    if (state->angle106 >= 0x1b)
        ext_o0_3e990((f32)((0x5a - (s32)state->angle106) << 2));
    ext_o0_3e99c(owner, updateRate);
    if (state->resource134 != 0) ext_o17_668(state->resource134, gO8Pointer14);
    if (state->resource138 != 0) ext_o17_668(state->resource138, gO8Pointer18);
    if (state->resourceC4 != 0)
        ext_o0_2d70(state->resourceC4, owner->valueC, owner->value10,
                    owner->value14);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F0000894_185E5EC.s")
#endif

/* Mickey-local reconstruction; the donor scans found no exact donor. */
s32 func_overlay_008_F0000E88_185EBE0(void *peer,
                                      Overlay8MotionRecord *record) {
    s16 direction;

    if ((record->gate16A == 0) &&
        (record->activeDirection == 0) &&
        (record->gate168 == 0)) {
        direction = record->direction;
        if (direction != 0) {
            record->activeDirection = -direction;
        } else if (record->fallbackSign < 0) {
            record->activeDirection = -1;
        } else {
            record->activeDirection = 1;
        }

        if (record->resource != NULL) {
            o8StartMotionResourceReloc(record->resource);
        }
        return 1;
    }
    return 0;
}

void func_overlay_008_F0000F1C_185EC74(Overlay8ActivationOwner *owner,
                                       s32 force) {
    Overlay8ActivationState *state;

    state = owner->state;
    if ((state->activeDirection != 0) || (state->gate158 != 0)) {
        return;
    }
    if ((force == 0) && (state->active185 == 1)) {
        return;
    }

    state->active183 = 1;
    state->active185 = 1;
    state->timer187 = 0x1E;

    if (((state->flags1A8 & 1) == 0) || (state->type == 0) ||
        (gOverlay8ActivationGateTimerReloc == 0)) {
        if (state->resource != NULL) {
            overlay8ReleaseResourceReloc(state->resource);
        }
        overlay8CreateResourceReloc(8, owner->valueC, owner->value10,
                                    owner->value14, 4, &state->resource);
        gOverlay8ActivationGateTimerReloc = 0x3C;
    }

    overlay8FinalizeActivationReloc(owner, 0x17);
}

/* Plateau: canonical -O2 -mips2 -r4300_mul is 164/165 words with 159 positional differences, first +0x8 (unused a0 home).
 * Signature, alias/first-use, control-flow/goto, volatile, register, and result-local variants did not reproduce retail a3 lifetime or rollover join.
 * A 40-minute permuter's best score was 1000 by reordering independent phase-2 stores; no exact result. */
#ifdef NON_MATCHING
f32 func_overlay_008_F0001000_185ED58(void *unused, O8PhaseState *state, f32 input) {
    s32 nextCountdown;

    if (state->timer > 0) {
        if ((state->phase != 1) && (state->phase != 2)) {
            state->phase = 2;
            state->countdown = 0;
        }
    }

    switch (state->phase) {
    case 1:
        state->weight += (1.0f - state->weight) * 0.875f;
        state->flags |= 0x8000;
        state->effect = 3;
        state->countdown--;

        if (state->countdown == 0) {
            nextCountdown = 120;
            if (gO8RolloverControlReloc != 0) {
                nextCountdown =
                    (s32)(o8RolloverSampleReloc() * 6.0f + 60.0f);
                if (nextCountdown >= 181) {
                    nextCountdown = 180;
                }
            }
            state->phase = 2;
            state->countdown = (u8)nextCountdown;
            state->weight = 1.0f;
        }

        o8Phase1EmitReloc(state, 75, 0.5f);
        break;

    case 2:
        state->effect = 3;
        if (state->countdown != 0) {
            state->countdown--;
            state->flags |= 0x8000;
        } else if (state->timer > 0) {
            state->weight +=
                (gO8Phase2TargetReloc - state->weight) * 0.875f;
        } else {
            state->phase = 3;
        }

        if (gO8Phase2ScaleControlReloc == 0) {
            input += gO8Phase2ScaleReloc * state->weight;
        } else {
            input += 7.0f * state->weight;
        }
        break;

    case 3:
        state->effect = 0;
        state->weight *= gO8Phase3DecayReloc;
        if (state->weight < gO8RetireThresholdReloc) {
            state->phase = 0;
            state->weight = 0.0f;
        }

        if (gO8Phase3ScaleControlReloc == 0) {
            input += gO8Phase3ScaleReloc * state->weight;
        } else {
            input += 7.0f * state->weight;
        }
        break;

    default:
        state->effect = 0;
        break;
    }

    if (state->forceEffect != 0) {
        state->effect |= 3;
    }
    return input;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F0001000_185ED58.s")
#endif

#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F0001294_185EFEC.s")

/* Workbench: mixed constant/structure/schedule/register; exact 183 words/0xD0 frame, 62 positional words, first +0xB8.
 * Lever: constant audit isolates the 36-byte record/home inversion; register-qualifying baseValue was codegen-inert.
 * Remaining: retail homes record/base value at +0x78/+0xCC versus +0x9C/+0x80; prior aggregate/order variants remain eliminated. */
#ifdef NON_MATCHING
void func_overlay_008_F0002640_1860398(
    O8P2640Anchor *anchor, O8P2640Config *config, s32 orientation,
    s32 randomLow, s32 randomHigh, f32 distanceX, f32 unusedStackFloat,
    f32 distanceZ, s32 emissionCount) {
    O8P2640Record record;
    const O8P2640Tuning *tuning;
    f32 axisA;
    f32 axisB;
    f32 clampedDistance;
    f32 spread;
    f32 magnitudeScale;
    s32 baseValue;
    s32 randomOffset;
    s32 randomValue;
    s32 magnitudeValue;
    s32 tuningIndex;

    (void)unusedStackFloat;
    tuningIndex = config->tuningIndex1;
    if (tuningIndex >= 10) {
        tuningIndex = 0;
    }

    clampedDistance = O8P2640_call_26AC(
        distanceX * distanceX + distanceZ * distanceZ);
    if (clampedDistance < 2.0f) {
        return;
    }
    if (clampedDistance > 8.0f) {
        clampedDistance = 8.0f;
    }

    baseValue = O8P2640_call_26F0(-distanceX, -distanceZ);
    axisA = O8P2640_call_26FC(anchor->helperInput0);
    tuning = &D_2110[tuningIndex];
    axisB = O8P2640_call_2708(anchor->helperInput0);

    record.coordC = anchor->coord10 + tuning->offset4;
    record.phase14 = O8P2640_data_198;
    record.size18 = 0x80;
    record.kind1A = 5;
    record.packed1C = 0xFFFFFFFF;
    record.packed28 = 0xFFFF80FF;
    record.packed20 = 0xFFFF80E0;
    record.packed2C = 0xFF8000E0;
    record.packed24 = 0xFF800000;
    record.packed30 = 0xFF000000;

    if (emissionCount == 0) {
        return;
    }
    emissionCount--;
    magnitudeScale = O8P2640_data_19C;
    do {
        randomOffset = O8P2640_call_27BC(-0xC80, 0xC80);
        randomValue = O8P2640_call_27CC(randomLow, randomHigh);
        magnitudeValue = O8P2640_call_27DC(0x50, 0x78);
        record.value0 = (s16)(baseValue + randomOffset);
        record.value2 = (s16)randomValue;
        record.magnitude4 = (f32)magnitudeValue *
                            clampedDistance * magnitudeScale;

        spread = (f32)randomOffset * tuning->spreadScale8;
        if (orientation == 0) {
            record.coord8 = anchor->coordC - tuning->extent0 * axisB +
                            spread * axisA;
            record.coord10 = anchor->coord14 + spread * axisB +
                             tuning->extent0 * axisA;
        } else if (orientation == 2) {
            record.coord8 = anchor->coordC + tuning->extent0 * axisB +
                            spread * axisA;
            record.coord10 = anchor->coord14 + spread * axisB -
                             tuning->extent0 * axisA;
        } else {
            record.coord8 = anchor->coordC + spread * axisB;
            record.coord10 = anchor->coord14 - spread * axisA;
        }

        O8P2640_call_28C0(&record);
        record.phase14 = 1.0f;
    } while (emissionCount--);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F0002640_1860398.s")
#endif

/* PLATEAU (2026-08-25): canonical r4300_mul is exact-size with 56 differing words, first +0x0.
 * The 0x70 vs 0x68 frame leaves angle/contact at +0x6E/+0x58 vs +0x36/+0x30, shifting the FP pool.
 * Declaration blocks/order, volatility, and scalar/aggregate angle forms did not close it; no donor used. */
#ifdef NON_MATCHING
void func_overlay_008_F000291C_1860674(O8P291CMotion *motion,
                                       O8P291CState *state,
                                       f32 update) {
    s16 angle;
    s32 savedRate;
    f32 horizontal;
    f32 vertical;
    f32 invUpdate;
    s32 contact;
    f32 targetBlend;
    f32 desiredBlend;
    f32 displacementX;
    f32 displacementY;
    f32 displacementZ;
    f32 scale;

    motion->heading0 = state->angleF0 + state->angleFC + state->angle104;
    angle = state->angleF0 + state->angleFE;

    if (state->mode438 == 1) {
        f32 attenuation = o8Approach291CReloc(O8P291C_data_1A0, (s32)update);
        if ((state->control4 < -0.5f) || (state->control4 > 0.5f)) {
            state->control4 *= attenuation;
        } else {
            state->control4 = 0.0f;
        }
        if ((state->control8 < -0.5f) || (state->control8 > 0.5f)) {
            state->control8 *= attenuation;
        } else {
            state->control8 = 0.0f;
        }
    }

    savedRate = (s32)update;
    if (state->active181 != 0) {
        f32 distance = state->speed84 * update +
                       (0.5f * state->accel88 * update * update);
        displacementX = state->axis74 * distance;
        displacementY = state->axis78 * distance;
        displacementZ = state->axis7C * distance;
        if (distance < 0.0f) {
            state->speed84 = 0.0f;
            state->accel88 = 0.0f;
            state->active181 = 0;
            if (((state->flags41C & 0x8000) == 0) &&
                (state->suppress185 == 0)) {
                state->control4 = 0.0f;
                state->control8 = 0.0f;
            }
        }
        scale = 1.0f - state->speed84 / state->speed80;
        state->speed84 += state->accel88 * update;
        horizontal = O8P291C_call_sin(angle) * state->control4 * scale;
        vertical = O8P291C_call_cos(angle) * state->control4 * scale;
    } else {
        displacementX = 0.0f;
        displacementY = 0.0f;
        displacementZ = 0.0f;
        horizontal = O8P291C_call_sin(angle) * state->control4;
        vertical = O8P291C_call_cos(angle) * state->control4;
    }

    horizontal += state->control8 * O8P291C_call_cos(angle);
    vertical -= state->control8 * O8P291C_call_sin(angle);
    o8Surface291CReloc(motion, state, savedRate);

    {
        f32 nextY;
        f32 nextX;
        f32 nextZ;
        nextY = (motion->velocity20 * update) -
            (0.5f * O8P291C_gravity * update * update) + displacementY;
        invUpdate = 1.0f / update;
        nextX = horizontal * update + displacementX;
        nextZ = vertical * update + displacementZ;
        motion->velocity1C = nextX * invUpdate;
        motion->velocity20 -= O8P291C_gravity * update;
        motion->velocity24 = nextZ * invUpdate;
        motion->positionC += nextX;
        motion->position10 += nextY;
        motion->position14 += nextZ;
    }

    contact = O8P291C_call_037C(motion, state, update);
    if ((O8P291C_call_039C(motion, 0.0f, 0.0f, 0.0f) != 0) ||
        (motion->state2E == -1)) {
        state->reset170 = 1;
        motion->positionC = state->origin38;
        motion->position10 = state->origin3C;
        motion->position14 = state->origin40;
        O8P291C_call_039C(motion, 0.0f, 0.0f, 0.0f);
    }

    state->delta94 = (motion->positionC - state->origin38) * invUpdate;
    state->delta98 = (motion->position10 - state->origin3C) * invUpdate;
    state->delta9C = (motion->position14 - state->origin40) * invUpdate;

    if ((state->mode16A == 0) &&
        ((contact != 0) || (motion->link48->gate62 != 0))) {
        invUpdate = (state->flags41C & 0x8000) ? 3.0f : 0.0f;
        state->blendC +=
            (invUpdate - state->blendC) *
            (1.0f - o8Approach291CReloc(O8P291C_data_1A4, savedRate));
        targetBlend = ((volatile O8P291CBlendView *)state)->blendC;
        if (state->control4 < -targetBlend) state->control4 = -targetBlend;
        if (targetBlend < state->control4) state->control4 = targetBlend;
        if (state->control8 < -targetBlend) state->control8 = -targetBlend;
        if (targetBlend < state->control8) state->control8 = targetBlend;
    } else {
        if (D_10 < state->blendC + O8P291C_data_1A8) {
            state->blendC = D_10;
            return;
        }
        state->blendC +=
            (D_10 - state->blendC) *
            (1.0f - o8Approach291CReloc(O8P291C_data_1AC, savedRate));
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F000291C_1860674.s")
#endif

void func_overlay_008_F0002EC0_1860C18(register Overlay8UpdateOwner *owner,
                                       Overlay8UpdateInput *input,
                                       s32 updateRate) {
    Overlay8UpdateFlag *flag;
    f32 delta;
    f32 decay;
    s32 remaining;

    if (owner->child == NULL) {
        return;
    }

    flag = owner->child->flag;
    owner->child->near = 0;
    if (flag != NULL) {
        flag->flags &= ~2;
    }

    if (input->active != 0) {
        input->delta6C = input->position68 - owner->position10;
        delta = input->delta6C;

        if (delta < 25.0f) {
            if ((input->lateral < gOverlay8UpdateLowerReloc) ||
                (input->lateral > gOverlay8UpdateUpperReloc)) {
                if (flag != NULL) {
                    flag->flags |= 2;
                }
            } else if (delta < 12.0f) {
                owner->child->near = 1;
            }
            owner->child->target80 = owner->position10 + 4.0f;
        } else if ((owner->velocity20 < 0.0f) && (updateRate != 0)) {
            remaining = updateRate - 1;
            decay = gOverlay8UpdateDecayReloc;
            do {
                owner->velocity20 -= owner->velocity20 * decay;
            } while (remaining--);
        }
    }

    overlay8FinishUpdateReloc(owner, updateRate);
}

void overlay8UpdateChannels(void *unused, Overlay8ChannelState *state,
                            f32 gate, void *sampleState) {
    f32 factor;
    f32 maximumFactor;
    f32 sample;
    f32 position;
    register const f32 upper = 0.1f;
    register const f32 lower = -0.1f;
    register const f32 multiplier = -0.67f;
    s32 i;
    s32 selectorIndex;
    const f32 *selectorScales;

    selectorScales = gOverlay8SelectorScales;
    maximumFactor = 0.0f;
    for (i = 0; i < 4; i++) {
        if (gate < 8.0f) {
            if (state->modes[i] != 0) {
                state->modes[i] = 2;
                state->values[i] = state->values[i] * multiplier;
                if ((lower < state->values[i]) &&
                    (state->values[i] < upper)) {
                    state->modes[i] = 0;
                }
            } else {
                if (state->selectorMode == 1) {
                    selectorIndex = 0;
                } else {
                    selectorIndex = i;
                }
                factor = selectorScales[
                    state->selectors[selectorIndex] & 0xF];
                *(volatile f32 *) &state->values[i] =
                    gOverlay8PhaseScales[state->phases[i]] * state->position;
                state->values[i] *= factor;
                if (maximumFactor < factor) {
                    maximumFactor = factor;
                }
            }
        } else {
            sample = overlay8SampleChannel(0.95f, sampleState);
            state->values[i] +=
                (-4.0f - state->values[i]) * (1.0f - sample);
            state->modes[i] = 1;
        }

        state->phases[i] = (state->phases[i] + 1) & 0x1F;
    }

    if (0.05f <= maximumFactor) {
        position = state->position;
        if ((position < -4.5f) || (position > 4.5f)) {
            overlay8EmitChannel(state, 0x28, 0.15f);
        }
    }
}

void func_overlay_008_F0003278_1860FD0(void *unused0,
                                       Overlay8ColorState *state,
                                       void *unused2) {
    s32 red;
    s32 green;
    s32 blue;

    if ((state->flags186 & 3) == 0) {
        return;
    }

    if ((state->timer16A > 0) && (state->owner != NULL)) {
        red = state->owner->colors->red;
        green = state->owner->colors->green;
        blue = state->owner->colors->blue;
    } else {
        red = 0;
        green = 0x40;
        if (state->alternate184 != 0) {
            green = 0;
            red = 0xE0;
            blue = 0x40;
        } else {
            blue = 0xE0;
        }
    }

    if (state->target354 != NULL) {
        o8ApplyColorsReloc(state->target354, 0xFF, 0xFF, 0xFF,
                                          red, green, blue);
    }
    if (state->target360 != NULL) {
        o8ApplyColorsReloc(state->target360, 0xFF, 0xFF, 0xFF,
                                          red, green, blue);
    }
}

/* Plateau: exact 0x138 size and 76/78 words; first mismatch +0x34 swaps
 * independent lower/upper threshold loads. The 119-flag lattice, ten source
 * shapes, and 2,401-second permuter pass retained the two-word plateau. */
#ifdef NON_MATCHING
void overlay8ScaleOutputs(void *unused, Overlay8ScaleState *state,
                          Overlay8ScaleContext *context,
                          Overlay8ScaleOutput *output) {
    Overlay8ScalePair *pair;
    Overlay8ScaleRecord *record;
    s16 *cursor;
    s32 i;
    u32 index;
    f32 upperThreshold;
    f32 lowerThreshold;

    pair = output->pairs;
    cursor = output->outputs[output->outputIndex];
    i = 0;

    if (context->count > 0) {
        upperThreshold = gOverlay8ScaleUpperReloc;
        lowerThreshold = gOverlay8ScaleLowerReloc;
        do {
            output = (Overlay8ScaleOutput *)pair->selector;
            index = (u32)output & 0xFF;
            if (!index) {}
            record = context->slots[index].record;

            if (((u32)output & 0x00100000) != 0) {
                if ((state->flags41C & 0x4000) != 0) {
                    if ((state->position >= upperThreshold) &&
                        (state->value42C < -30)) {
                        pair->first = 0x100;
                    } else {
                        pair->first = 0x200;
                    }
                } else if (state->position <= lowerThreshold) {
                    pair->first = 0;
                } else {
                    pair->first = 0x100;
                }
            }

            *cursor++ = (pair->first >> 8) * record->scale;
            if (pair->second >= 0) {
                *cursor++ = (pair->second >> 8) * record->scale;
            }
            i++;
            pair++;
        } while (i < context->count);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F0003368_18610C0.s")
#endif

/* NON_MATCHING plateau (2026-08-25): owner-mode first-use and block-scoped FP temps give -O2 -mips2 -Wo,-loopunroll,0 at +0x20, 749/898 words different, first +0x0.
 * Workbench: structure-mismatch, frame 0xB0 vs 0x80; split/field-backed query, terrain-temp reuse, declaration order, register hints, and a 13,385-best permuter run did not close it. */
#ifdef NON_MATCHING
f32 func_overlay_008_F00034A0_18611F8(O8P34A0Owner *owner,
                                      O8P34A0State *state, f32 limit,
                                      f32 update) {
    O8P34A0Query query;
    f32 selectedValue;
    f32 blend;
    f32 result;
    f32 height;
    f32 strength;
    f32 delta;
    f32 trigA;
    f32 trigB;
    f32 factor;
    f32 overrideValue;
    s8 selectedMode;
    s8 ownerMode;
    s32 sampleCount;
    s32 index;
    s32 start;
    s32 target;
    s32 steps;
    s32 remaining;

    selectedMode = owner->mode3B;
    query.scratch18 = -1;
    query.scratch1C = -1;
    query.scratch20 = -1;
    query.scratch24 = -1;
    result = 0.0f;
    selectedValue = 0.0f;
    blend = 0.0f;

    if (state->motion4 < -0.5f) {
        if (state->direction100 < 0) {
            selectedMode = 6;
            if ((state->flags41C & 0x4000) != 0) {
                selectedMode = 0x16;
                selectedValue = D_1D8;
            } else {
                selectedValue = D_1DC;
            }
        } else if (state->direction100 > 0) {
            selectedMode = 7;
            if ((state->flags41C & 0x4000) != 0) {
                selectedMode = 0x17;
                selectedValue = D_1E0;
            } else {
                selectedValue = D_1E4;
            }
        } else {
            selectedValue = D_1E8;
            if ((state->flags1A8 & 1) != 0) {
                state->smoothed38A -= state->smoothed38A / 8;
                state->smoothed38A += state->steering108 / 8;
                if (state->smoothed38A >= 0xDD) {
                    selectedMode = 4;
                } else if (state->smoothed38A < -0xDC) {
                    selectedMode = 5;
                } else if ((state->smoothed38A >= 0x42) &&
                           (state->smoothed38A < 0x96)) {
                    selectedMode = 2;
                } else if ((state->smoothed38A < -0x41) &&
                           (state->smoothed38A >= -0x95)) {
                    selectedMode = 3;
                } else if ((state->smoothed38A >= -0x18) &&
                           (state->smoothed38A < 0x19)) {
                    selectedMode = 1;
                } else {
                    selectedMode = owner->mode3B;
                }
            } else if (state->steering108 >= 0x75) {
                selectedMode = 4;
            } else if (state->steering108 < -0x74) {
                selectedMode = 5;
            } else if (state->steering108 >= 0x11) {
                selectedMode = 2;
            } else if (state->steering108 < -0x10) {
                selectedMode = 3;
            } else {
                selectedMode = 1;
            }
        }
    } else if (state->motion4 > 0.5f) {
        if (state->steering108 >= 0x11) {
            if ((owner->mode3B == 8) && (owner->scale28 == 1.0f)) {
                blend = 1.0f;
                selectedMode = 0xA;
                selectedValue = D_1EC;
            } else if (owner->mode3B != 0xA) {
                selectedMode = 8;
                selectedValue = D_1F0;
            }
        } else if ((owner->mode3B == 9) && (owner->scale28 == 1.0f)) {
            blend = 1.0f;
            selectedMode = 0xB;
            selectedValue = D_1F4;
        } else if (owner->mode3B != 0xB) {
            selectedMode = 9;
            selectedValue = D_1F8;
        }
    } else {
        selectedMode = 0;
        selectedValue = D_1FC;
        if ((owner->mode3B == 0x11) || (owner->mode3B == 0x12)) {
            if (owner->scale28 != 1.0f) {
                selectedMode = owner->mode3B;
                selectedValue = D_200;
            }
        } else if ((owner->mode3B == 0) &&
                   (o8P34A0RandomReloc(0, 0x3FF) >= 0x3FB)) {
            blend = 0.0f;
            selectedValue = D_204;
            selectedMode = o8P34A0RandomReloc(0x11, 0x12);
        }
    }

    if ((state->lowering349 == 0) && (state->override172 == 0)) {
        if (owner->mode3B == 0xC) {
            selectedMode = 0xC;
            blend = 0.0f;
            gO8P34A0ScaleReloc *= D_208;
            selectedValue = D_20C;
            result = 40.0f;
        } else {
            sampleCount = o8P34A0TerrainReloc(owner->xC, owner->z14,
                                              0x1800, &query.samples0);
            if (sampleCount != 0) {
                index = 0;
                if (sampleCount > 0) {
                    do {
                        height = *query.samples0[index];
                        index++;
                    } while ((height >= owner->y10) &&
                             (index != sampleCount));
                    result = height;
                }
                result = owner->y10 - result;
                if (result > 40.0f) {
                    gO8P34A0ScaleReloc *= D_210;
                    state->mode16C = 1;
                    selectedMode = 0xC;
                    blend = 0.0f;
                    selectedValue = D_214;
                    o8P34A0EffectReloc(owner, 0x15, 0xC);
                }
            }
        }
    }

    ownerMode = owner->mode3B;
    if ((state->force185 == 1) ||
        ((ownerMode == 0xE) && (owner->scale28 != 1.0f))) {
        selectedMode = 0xE;
        blend = 0.0f;
        selectedValue = (gO8P34A0ModeReloc == 0) ? D_218 : D_21C;
    } else if (((ownerMode == 0x13) || (ownerMode == 0x14) ||
                (ownerMode == 0x15)) &&
               (owner->scale28 != 1.0f)) {
        selectedMode = ownerMode;
        blend = 0.0f;
        selectedValue = D_220;
    } else if (((state->flags41C & 0x2000) != 0) &&
               ((state->gate19B == 0) || (state->gate19C != 0))) {
        selectedMode = 0xD;
        blend = 0.0f;
        selectedValue = D_224;
    }

    if (state->overrideMode193 != 0) {
        selectedMode = state->overrideMode193;
        selectedValue = *(f32 *)((u8 *)state + 0x194);
        blend = 0.0f;
    }

    if (((owner->peer48->gate63 != 0) || (state->motion34A != 0)) &&
        (state->active181 != 0) && (ownerMode != 0x13) &&
        (ownerMode != 0x14) && (ownerMode != 0x15)) {
        state->activity3C8 += 2.0f;
        blend = 0.0f;
        selectedValue = D_228;
        if ((state->value8C <= D_22C) || (D_230 <= state->value8C)) {
            selectedMode = 0x13;
        } else if (state->value90 >= 0.0f) {
            selectedMode = 0x14;
        } else {
            selectedMode = 0x15;
        }
        ownerMode = owner->mode3B;
    }

    if ((ownerMode == 0x18) && (owner->scale28 != 1.0f)) {
        selectedMode = 0x18;
        blend = 0.0f;
        selectedValue = D_234;
    } else if (state->secondary102 > 0) {
        selectedMode = 0xF;
        blend = 0.0f;
        selectedValue = D_238;
    } else if (state->secondary102 < 0) {
        selectedMode = 0x10;
        blend = 0.0f;
        selectedValue = D_23C;
    }

    if (selectedMode != ownerMode) {
        o8P34A0SetModeReloc(owner, selectedMode, -1, blend);
    }
    if ((o8P34A0AnimateReloc(owner, selectedValue, update) != 0) &&
        (selectedValue != 0.0f)) {
        state->flags1A8 |= 2;
    } else {
        state->flags1A8 &= 0xFFFD;
    }
    if ((state->overrideMode193 != 0) &&
        (selectedMode != state->overrideMode193)) {
        o8P34A0EventReloc(owner, 0x3C, selectedMode);
    }

    start = 0;
    if (gOverlay8Value != 0) {
        start = 2;
        o8P34A0StateEffectReloc(state, 0x28, 0.15f);
    }
    for (index = start; index < 4; index++) {
        state->angles114[index] +=
            (s32)(state->motion4 * update * D_240);
    }

    target = state->steering428;
    if (target < -0x3C) {
        target = -0x1770;
    } else if (target >= 0x3D) {
        target = 0x1770;
    } else {
        target *= 0x64;
    }
    steps = (s32)update;
    if (steps != 0) {
        remaining = steps - 1;
        do {
            state->angle110 +=
                o8P34A0ApproachReloc(state->angle110, target) >> 2;
        } while (remaining-- != 0);
    }
    state->angle112 = state->angle110;

    *gOverlay8Buffer = 3;
    gOverlay8Buffer++;
    *gOverlay8Buffer = state->angle144;
    gOverlay8Buffer++;
    *gOverlay8Buffer = 9;
    gOverlay8Buffer++;
    *gOverlay8Buffer = state->angle146;
    gOverlay8Buffer++;
    func_overlay_008_F00049E8_1862740(owner, state, update);

    if (state->kind1 == 4) {
        f32 strength;
        f32 delta;
        f32 trigA;
        f32 trigB;
        f32 factor;
        s16 outputAngle;

        if ((state->motion4 < 0.0f) && (limit != 0.0f)) {
            if (state->motion4 < -limit) {
                outputAngle = -0x3000;
            } else {
                outputAngle = (s16)(s32)((12288.0f / limit) *
                                         state->motion4);
            }
            strength = -state->motion4 / limit;
            delta = (update / 60.0f) * strength * 25.0f;
            state->phase3EC += delta;
            state->phase3F0 += delta;
            if (D_244 <= state->phase3EC) {
                state->phase3EC -= D_244;
            }
            if (D_244 <= state->phase3F0) {
                state->phase3F0 -= D_244;
            }
            trigA = o8P34A0TrigAReloc(
                (s32)((state->phase3EC / D_244) * 65536.0f));
            trigB = o8P34A0TrigBReloc(
                (s32)((state->phase3F0 / D_248) * 65536.0f));
            factor = strength * 4096.0f;
            state->output3F8 = -outputAngle;
            state->output3F4 = outputAngle + (s32)(factor * trigA);
            state->output3F6 = outputAngle + (s32)(factor * trigB);
        } else {
            factor = 1.0f - o8P34A0DecayReloc(D_24C, steps);
            state->output3F4 =
                o8P34A0BlendReloc(state->output3F4, 0, factor);
            factor = 1.0f - o8P34A0DecayReloc(D_250, steps);
            state->output3F6 =
                o8P34A0BlendReloc(state->output3F6, 0, factor);
            factor = 1.0f - o8P34A0DecayReloc(D_254, steps);
            state->output3F8 =
                o8P34A0BlendReloc(state->output3F8, 0, factor);
        }

        *gOverlay8Buffer = 0x1E;
        gOverlay8Buffer++;
        *gOverlay8Buffer =
            state->output3F8 + state->angle144 + state->angle146;
        gOverlay8Buffer++;
        *gOverlay8Buffer = 0x21;
        gOverlay8Buffer++;
        *gOverlay8Buffer = state->output3F4;
        gOverlay8Buffer++;
        *gOverlay8Buffer = 0x24;
        gOverlay8Buffer++;
        *gOverlay8Buffer = state->output3F6;
        gOverlay8Buffer++;
    } else if (state->kind1 == 2) {
        f32 strength;
        f32 trigA;
        f32 factor;

        if ((state->motion4 < 0.0f) && (limit != 0.0f)) {
            strength = -state->motion4 / limit;
            state->phase3EC += (update / 60.0f) * strength * 30.0f;
            if (D_258 <= state->phase3EC) {
                state->phase3EC -= D_258;
            }
            trigA = o8P34A0TrigAReloc(
                (s32)((state->phase3EC / D_258) * 65536.0f));
            state->output3F4 = (s32)(8192.0f * strength * trigA);
        } else {
            factor = 1.0f - o8P34A0DecayReloc(D_25C, steps);
            state->output3F4 =
                o8P34A0BlendReloc(state->output3F4, 0, factor);
        }
        *gOverlay8Buffer = 0x1E;
        gOverlay8Buffer++;
        *gOverlay8Buffer = state->output3F4;
        gOverlay8Buffer++;
    }
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F00034A0_18611F8.s")
#endif

/*
 * NON_MATCHING p2: workbench structure-mismatch; the best home-pressure variant
 * has 433/447 instructions and 425 differing positional words, first +0x0.
 * Lever 26 leaves an eight-byte non-save frame deficit and 14 missing instructions.
 */
#ifdef NON_MATCHING
void func_overlay_008_F00042A8_1862000(O8P42A8Actor *actor,
                                       O8P42A8Owner *owner, f32 update) {
    O8P42A8State *state;
    volatile f32 targetMotion;
    f32 targetHeight;
    f32 phase;
    f32 smoothing;
    f32 firstTrig;
    f32 secondTrig;
    f32 relativeFirst;
    f32 relativeSecond;
    f32 lateral;
    f32 forward;
    f32 vertical;
    f32 baseX;
    f32 baseY;
    f32 baseZ;
    f32 targetTilt;
    f32 absoluteVelocity;
    f32 acceleration;
    f32 steering;
    s32 randomMode;
    s32 tableIndex;
    s32 targetAngle;
    s32 steps;
    s32 remaining;
    s16 savedAngle;
    s32 mode;

    state = owner->state64;
    mode = state->mode0 & 3;
    o8P42A8SampleReloc(60.0f, 0);

    if (state->reset170 != 0) {
        steps = (s32)update;
        savedAngle = actor->angle0;
        if (steps != 0) {
            remaining = steps - 1;
            do {
                actor->angle0 +=
                    o8P42A8ApproachReloc(actor->angle0, savedAngle) >> 3;
                actor->angle2 +=
                    o8P42A8ApproachReloc(actor->angle2, 0x800) >> 3;
                actor->angle4 -= actor->angle4 >> 3;
            } while (remaining--);
        }
        return;
    }

    if ((state->cycleFlags420 & 8) != 0) {
        D_0[mode]++;
    }
    D_0[mode] &= 3;
    tableIndex = D_0[mode];
    randomMode = o8P42A8RandomReloc() & 3;
    if ((state->reset170 != 0) || (state->lock191 != 0)) {
        tableIndex = 1;
    }

    targetAngle = ((s16 *)0x2208)[randomMode];
    targetMotion = O8_F32(0x2188 + (((randomMode * 4) + tableIndex) * 4));
    phase = D_2210[mode];
    if ((state->lowering349 == 0) && (state->lock191 == 0)) {
        phase += O8_F32(0x260) * update;
        if (phase > 1.0f) {
            phase = 1.0f;
        }
    } else {
        phase -= 0.125f * update;
        if (phase < 0.0f) {
            phase = 0.0f;
        }
    }
    D_2210[mode] = phase;

    state->directionDC = 0x8000 - state->angleF0;
    acceleration = state->accelerationE4;
    steering = state->steeringE0;
    targetAngle -= (owner->angle2 * 3) >> 2;
    if (targetAngle >= 0x2001) {
        targetAngle = 0x2000;
    }
    if (targetAngle < -0x2000) {
        targetAngle = -0x2000;
    }
    targetHeight = O8_F32(0x21C8 + (((randomMode * 4) + tableIndex) * 4));
    targetHeight += phase * O8_F32(0x2220 + (randomMode * 4));

    steps = (s32)update;
    remaining = steps - 1;
    if (steps != 0) {
        do {
            actor->angle2 +=
                o8P42A8ApproachReloc(actor->angle2, targetAngle) >> 4;
        } while (remaining--);
    }

    if (state->velocity4 < 0.0f) {
        f32 reduction = -6.0f * acceleration * state->velocity4;

        if (state->modifier100 != 0) {
            reduction *= 0.5f;
        }
        if (reduction > 65.0f) {
            reduction = 65.0f;
        }
        targetMotion -= reduction;
    }

    smoothing = steering * 60.0f;
    if (state->double184 != 0) {
        smoothing += smoothing;
    }
    if (state->modifier100 != 0) {
        smoothing *= 0.5f;
    }
    targetMotion += smoothing;
    if ((state->force185 == 1) && ((state->flags41C & 0x4000) == 0)) {
        targetMotion = 32.0f;
    }
    if (state->special18D != 0) {
        targetMotion = 200.0f;
        targetHeight = 10.0f;
    }

    if (O8_S32(0) == 0) {
        smoothing = O8_F32(0x264);
    } else {
        smoothing = O8_F32(0x268);
    }
    remaining = steps - 1;
    if (steps != 0) {
        do {
            actor->motion24 += (targetMotion - actor->motion24) * smoothing;
            actor->motion28 += (targetHeight - actor->motion28) * smoothing;
        } while (remaining--);
    }

    firstTrig = o8P42A8TrigAReloc(0x8000 - state->directionDC);
    secondTrig = o8P42A8TrigBReloc(0x8000 - state->directionDC);
    relativeFirst = o8P42A8TrigAReloc(actor->angle2 - targetAngle);
    relativeSecond = o8P42A8TrigBReloc(actor->angle2 - targetAngle);
    lateral = (actor->motion24 * relativeSecond) -
              (actor->motion28 * relativeFirst);
    baseX = lateral * firstTrig;
    baseY = (actor->motion24 * relativeFirst) +
            (actor->motion28 * relativeSecond);
    baseZ = lateral * secondTrig;

    if (state->sign102 == 0) {
        targetTilt = -10.0f;
    } else {
        targetTilt = 10.0f;
    }
    remaining = steps - 1;
    if (steps != 0) {
        do {
            state->heightE8 +=
                (targetTilt - state->heightE8) * O8_F32(0x27C);
        } while (remaining--);
    }

    baseX += owner->xC + (state->offset14 * state->heightE8);
    baseY += owner->y10 + (state->offset18 * state->heightE8);
    baseZ += owner->z14 + (state->offset1C * state->heightE8);
    firstTrig = o8P42A8TrigAReloc(state->directionDC + 0x4000);
    secondTrig = o8P42A8TrigBReloc(state->directionDC + 0x4000);

    if (state->modifier100 != 0) {
        targetTilt = (f32)(state->modifier100 * -12);
    } else if ((state->flags41C & 0x8000) != 0) {
        absoluteVelocity = state->velocity4;
        if (absoluteVelocity < 0.0f) {
            absoluteVelocity = -absoluteVelocity;
        }
        if (absoluteVelocity > 1.0f) {
            absoluteVelocity = 1.0f;
        }
        targetTilt = absoluteVelocity * (f32)state->magnitude108 *
                     O8_F32(0x288);
    } else {
        targetTilt = 0.0f;
    }

    remaining = steps - 1;
    if (steps != 0) {
        do {
            state->tiltEC +=
                (targetTilt - state->tiltEC) * O8_F32(0x284);
        } while (remaining--);
    }

    vertical = state->tiltEC;
    actor->xC = baseX + (vertical * firstTrig);
    actor->y10 = baseY;
    actor->z14 = (baseZ - (vertical * secondTrig));
    actor->angle0 = state->directionDC;

    remaining = steps - 1;
    if (steps != 0) {
        do {
            actor->angle4 +=
                o8P42A8ApproachReloc(actor->angle4, owner->angle4 >> 1) >> 5;
        } while (remaining--);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F00042A8_1862000.s")
#endif

void overlay8SetBuffer(void *base) {
    gOverlay8Buffer = (s16 *)((u8 *)base + 0x1B8);
}

void overlay8WriteCommand(volatile s32 unused) {
    *gOverlay8Buffer = 0x2000;
    gOverlay8Buffer++;
}

void overlay8SetValue(s32 value) {
    gOverlay8Value = value;
}

void overlay8UpdateMotionOutput(Overlay8MotionAnchor *anchor,
                                Overlay8MotionState *state,
                                f32 inputScale) {
    const Overlay8MotionRow *row;
    s32 span;
    Overlay8MotionAnchor *target;
    f32 delta;
    struct {
        f32 first;
        f32 second;
    } scales;

    row = &D_2230[state->rowIndex];
    target = state->target;
    span = row->firstScale + row->secondScale;

    if (target == NULL) {
        delta = 0.0f;
    } else {
        delta = (f32) overlay8ConvertDirectionReloc(
            anchor->helperInput,
            overlay8MeasureDirectionReloc(anchor->x - target->x,
                                          anchor->y - target->y));
    }

    if (span == 0) {
        return;
    }

    if ((f32) span < delta) {
        delta = (f32) span;
    } else if (delta < (f32) -span) {
        delta = (f32) -span;
    }

    if ((8000.0f < delta) || (delta < -8000.0f)) {
        if (state->outsideLatch == 0) {
            state->outsideLatch = 1;
            state->outsideValue = gOverlay8MotionOutsideValueReloc;
        }
    } else if (state->outsideLatch == 1) {
        state->outsideLatch = 0;
    }

    if (delta != (f32) state->primary) {
        register f32 step;

        step = overlay8ApproachMotionReloc(
            (s32) (delta - (f32) state->primary),
            (f32) state->secondary * 2.0f, 800.0f);
        state->secondary = (s16) (s32) ((f32) state->secondary + step);
        state->primary = (s16) (s32) (
            (f32) state->primary +
            (f32) state->secondary * inputScale * 0.5f);
    }

    scales.first = (f32) row->firstScale * (1.0f / (f32) span);
    scales.second = (f32) row->secondScale * (1.0f / (f32) span);

    *gOverlay8Buffer = (s16) ((u32) row->firstSelector * 3U + 1U);
    gOverlay8Buffer++;
    *gOverlay8Buffer =
        (s16) (s32) ((f32) state->primary * scales.first);
    gOverlay8Buffer++;
    *gOverlay8Buffer = (s16) ((u32) row->secondSelector * 3U + 1U);
    gOverlay8Buffer++;
    *gOverlay8Buffer =
        (s16) (s32) ((f32) state->primary * scales.second);
    gOverlay8Buffer++;
}

/* Plateau (2026-08-25, near-miss p3): workbench mixed residual; best remains 43 words, first +0x178.
 * Tried constant audit and stack-frame home reshaping with pads, a wider normal, and local exchange.
 * The normal home remains four bytes high, leaving the target FP pool and temp phase unresolved. */
#ifdef NON_MATCHING
void func_overlay_008_F0004CF0_1862A48(O8P4CF0Actor *actor,
                                       O8P4CF0State *state,
                                       s32 updateRate) {
    s32 start;
    s32 end;
    O8P4CF0SceneItem **items;
    register f32 motionTarget;
    f32 blendFactor;
    s32 targetB;
    register f32 horizontalA;
    f32 horizontalB;
    register f32 axisA;
    register f32 axisB;
    register f32 surfaceHeight;
    O8P4CF0Vec3f point;
    O8P4CF0Normal normal;

    state->activated173 = 0;

    if (O8P4CF0_call_4D14(20) != 0) {
        items = O8P4CF0_call_4D24(&start, &end);
        point.x = 0.0f;
        point.y = 0.0f;
        point.z = 7.0f;
        O8P4CF0_call_4D54(1, actor, &point, &point);
        point.x += actor->x00C;
        surfaceHeight = (point.y += actor->y010);
        point.z += actor->z014;
        surfaceHeight = point.y;

        if (start < end) {
            do {
                O8P4CF0SceneItem *item = items[start++];

                if (item->category044 == 0x3D) {
                    O8P4CF0Bounds *bounds = item->bounds084;

                    if (bounds != 0) {
                        if ((bounds->minX000 <= (s32)point.x) &&
                            ((s32)point.x <=
                             bounds->minX000 + bounds->extentX006)) {
                            if ((bounds->minZ004 <= (s32)point.z) &&
                                ((s32)point.z <=
                                 bounds->minZ004 + bounds->extentZ008)) {
                                if (item->callbackGate088 != 0) {
                                    O8P4CF0_call_4E50(item);
                                }
                                surfaceHeight = O8P4CF0_call_4E64(
                                    bounds, point.x, point.z, &normal);
                                break;
                            }
                        }
                    }
                }
            } while (start < end);
        }

        if (surfaceHeight > point.y) {
            s32 targetA;
            f32 factor;

            axisA = O8P4CF0_call_4E9C(-actor->angle000);
            axisB = O8P4CF0_call_4EAC(-actor->angle000);
            horizontalB = normal.x;
            horizontalA = -(normal.z * axisA + horizontalB * axisB);
            horizontalB = normal.z * axisB - horizontalB * axisA;
            targetA = O8P4CF0_call_4EE8(horizontalA, normal.y);
            targetB = O8P4CF0_call_4EF8(horizontalB, normal.y);

            factor = O8P4CF0_call_4F0C(0.9f, updateRate);
            actor->angle004 = (s16)O8P4CF0_call_4F34(
                actor->angle004, (s16)targetA, 1.0f - factor);
            factor = O8P4CF0_call_4F48(0.9f, updateRate);
            actor->angle002 = (s16)O8P4CF0_call_4F68(
                actor->angle002, (s16)targetB, 1.0f - factor);

            start = updateRate - 1;
            if (updateRate != 0) {
                do {
                    actor->vertical020 +=
                        (((surfaceHeight - state->height178 - point.y) *
                          0.5f) - actor->vertical020) * 0.25f;
                } while (start--);
            }

            O8P4CF0_data_4FD4 *= 0.4f;
            state->timer172 = 12;
            state->activated173 = 1;
        }
    }

    if (state->timer172 != 0) {
        state->timer172 =
            (s8)((u32)(s32)state->timer172 - (u32)updateRate);
        if (state->timer172 <= 0) {
            state->timer172 = 0;
            state->blend174 = (f32)1;
            state->height178 = 2.0f;
            state->derived17C = 1.0f;
            return;
        } else {
            s32 count;

            axisA = state->motion004;
            count = updateRate - 1;
            if (axisA < 0.0f) {
                axisA = -axisA;
            }
            axisA *= 0.2f;
            if (1.0f < axisA) {
                axisA = 1.0f;
            }

            start = count;
            if (count != -1) {
                blendFactor = 0.05f;

                do {
                    state->blend174 +=
                        (axisA - state->blend174) * blendFactor;
                } while (start--);
            }
            state->height178 = state->blend174 * -4.0f + 6.0f;
            state->derived17C =
                state->blend174 * 0.024999976f + 0.975f;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F0004CF0_1862A48.s")
#endif
