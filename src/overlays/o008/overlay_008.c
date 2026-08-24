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

#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F0000058_185DDB0.s")

/* Plateau after 10 serious attempts: exact 0x5F4 size/frame/opcode shape,
 * 88 instruction-word register differences, first at +0x1FC. The remaining
 * mismatch is an FP temp-FIFO phase shift that later cascades into GPR temps. */
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
            x = 0.0f;
            y = 4.0f - state->lateral4 * gO8FloatD4;
        } else if (state->mode34A & 0xa) {
            effect = 2;
            x = 0.0f;
            y = state->lateral4 * gO8FloatD8 + -4.0f;
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
        flags = owner->flags80 & ~0x33;
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
    axisB = O8P2640_call_2708(anchor->helperInput0);
    tuning = &D_2110[tuningIndex];

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

/* Plateau after 10 serious attempts: exact 0x5A4 instruction count/opcodes,
 * 56 instruction-word differences, first at +0x0. The candidate frame is
 * 0x70 versus target 0x68; angle/contact stack-home order drives the FP pool. */
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
        lowerThreshold = gOverlay8ScaleLowerReloc;
        upperThreshold = gOverlay8ScaleUpperReloc;
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

#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F00034A0_18611F8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F00042A8_1862000.s")

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

/* Plateau after 10 serious attempts: exact 0x438 size/frame/opcode shape,
 * 43 instruction-word differences, first at +0x178. The normal vector's
 * stack home is four bytes high, shifting its FP pool and later temp phase. */
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
