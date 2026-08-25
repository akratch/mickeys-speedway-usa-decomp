#include "overlays/overlay_090.h"

/* DKR v77/v80 and JFG contain no exact donor for this state initializer. */
void overlay90Initialize(Overlay90Owner *owner, Overlay90Config *config) {
    Overlay90State *state;

    state = owner->state;
    state->active = 1;
    state->flag = 0;
    state->angle = config->angle;
    state->x = config->x;
    state->y = config->y;
    state->z = config->z;
    state->value26 = 0;
    state->value28 = 0;
    state->value14 = 0.0f;
    state->value24 = -0x8000;
    state->value2A = 0;
    state->value34 = 0;
    state->value36 = 0;
    state->value38 = 0;
    state->value3C = 0x7F;
    state->value26 = -0x1F00;
    state->value28 = -0x2040;
    state->value2C = 0x20;
    state->value2E = 0x80;
    state->value10 = 0.0f;
    state->value18 = 0.0f;
    state->value1C = 0.0f;
    state->value20 = 0.0f;
    state->value30 = 0.0f;
    state->value14 = 15.0f;
    state->value1C = gOverlay90Value1C;
    state->value20 = gOverlay90Value20;
    overlay90CommitReloc(owner, 0, -1, 0.0f);
}

typedef struct Overlay90Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay90Vector;

typedef struct Overlay90AttachmentHeader {
    u8 pad00[0x2C];
    u8 count;
} Overlay90AttachmentHeader;

typedef struct Overlay90AttachmentEntry {
    s16 value;
    u16 pad02;
    u32 flags;
} Overlay90AttachmentEntry;

struct Overlay90Attachment {
    Overlay90AttachmentHeader *header;
    u8 pad04[0x48];
    Overlay90AttachmentEntry *entries;
};

typedef struct Overlay90Level {
    u8 pad00[0x8E];
    u8 sequence;
} Overlay90Level;

extern f32 gOverlay90ScaleDecayReloc;
extern f32 gOverlay90ScaleStepReloc;
extern f32 gOverlay90AccelerationReloc;
extern f32 gOverlay90AnimationRateReloc;
extern f32 gOverlay90RiseRateReloc;
extern f32 gOverlay90MotionDecayReloc;
extern f32 gOverlay90AnimationScaleReloc;
extern s32 gOverlay90SequenceStateReloc;
extern s32 gOverlay90SequenceDoneReloc;

extern void pointListRPY(s32 count, s16 *rotation, f32 *input, f32 *output);
extern void func_8005ABA8(Overlay90Owner *owner, f32 value, f32 updateRate);
extern void func_8005AD64(Overlay90Owner *owner, s32 mode, s32 index,
                          f32 startFrame);
extern void amSndPlay(s32 soundId, void **handle);
extern void overlay90SequenceReloc(s32 sequenceId);
extern s32 camGetMode(void);
extern Overlay90Level *levelGetLevel(void);
extern void func_80000510(u8 sequenceId);
extern void func_800031E8(void *handle);
extern void func_80006EA0(void *object);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern s32 func_8000FAE0(f32 x, f32 y, f32 z);
extern void func_80002FE0(s32 id, f32 x, f32 y, f32 z, s32 priority,
                          void **handle);
extern s32 mathRnd(s32 lower, s32 upper);
extern void func_800031C0(void *handle, f32 x, f32 y, f32 z);
extern void func_8000309C(void *handle, u8 volume);
extern void func_800030B4(void *handle, u8 pitch);

/*
 * Phase-B reconstruction from Mickey's own overlay and relocation metadata.
 * The nearest five-project skeleton score is only 0.067, so no donor body is
 * used here.
 *
 * Plateau (2026-08-25): four complete 119-variant flag lattices selected
 * -O2 -mips2 -Wo,-loopunroll,0. The best coherent body is 636 target words
 * against 648, with 598 positional differences and the first at +0x0. Its
 * save set and opening schedule agree, but IDO selects a 0xE0 frame instead
 * of 0xC8 and spills updateRate at +0xA8 instead of +0xB8; the remaining
 * state-machine CFG and scalar/aggregate lifetimes keep allocation global.
 */
#ifdef NON_MATCHING
void func_overlay_090_F00000FC_18D4BF4(Overlay90Owner *owner,
                                        s32 updateRate) {
    Overlay90State *state;
    f32 sine;
    f32 cosine;
    f32 radial;
    f32 side;
    f32 forward;
    f32 animationValue;
    f32 soundPitch;
    f32 maximumAnimation;
    f32 animationRate;
    f32 riseRate;
    f32 motionDecay;
    f32 zero;
    s32 originalUpdateRate;
    s32 remaining;
    s32 steps;
    s32 displayIndex;
    s32 transitioned;
    s32 value;
    Overlay90Vector delta;

    state = owner->state;
    originalUpdateRate = updateRate;
    state->value34 += 0x200;
    displayIndex = 0;

    if ((state->active == 1) || (state->active == 7)) {
        remaining = updateRate - 1;
        if (updateRate != 0) {
            do {
                state->value30 *= gOverlay90ScaleDecayReloc;
                remaining--;
            } while (remaining != 0);
        }
    } else {
        remaining = updateRate - 1;
        if (updateRate != 0) {
            do {
                state->value30 +=
                    (8.0f - state->value30) * gOverlay90ScaleStepReloc;
                remaining--;
            } while (remaining != 0);
        }
    }

    maximumAnimation = gOverlay90AccelerationReloc;
    animationRate = gOverlay90AnimationRateReloc;
    riseRate = gOverlay90RiseRateReloc;
    zero = 0.0f;
    motionDecay = gOverlay90MotionDecayReloc;

process_mode:
    transitioned = 0;
    switch (state->active) {
    case 1:
        steps = 0xF0 - state->flag;
        if (updateRate < steps) {
            steps = updateRate;
        }
        remaining = steps - 1;
        if (steps != 0) {
            do {
                state->value26 += state->value2C;
                state->value28 += state->value2E;
                state->value14 *= motionDecay;
                if (state->value2E > 0) {
                    state->value2E--;
                }
                delta.x = zero;
                delta.y = zero;
                delta.z = -state->value14;
                pointListRPY(1, &state->value24, &delta.x, &delta.x);
                state->value18 += delta.x;
                state->value1C += delta.y;
                state->value20 += delta.z;
                remaining--;
            } while (remaining != 0);
        }
        animationValue =
            state->value14 * gOverlay90AnimationScaleReloc +
            animationRate;
        if (animationValue > maximumAnimation) {
            animationValue = maximumAnimation;
        }
        func_8005ABA8(owner, animationValue, (f32)updateRate);
        state->flag += updateRate;
        if (state->flag >= 0xF0) {
            state->flag -= 0xF0;
            state->active = 2;
            state->value14 = 0.0f;
            func_8005AD64(owner, 2, -1, zero);
            transitioned = 1;
        }
        break;
    case 2:
        func_8005ABA8(owner, 0.016669f, (f32)updateRate);
        state->flag += updateRate;
        if (state->flag >= 0x78) {
            state->flag -= 0x78;
            state->active = 3;
            func_8005AD64(owner, 1, -1, zero);
            amSndPlay(0x12, 0);
            overlay90SequenceReloc(0x1D);
        }
        break;
    case 3:
        displayIndex = 1;
        func_8005ABA8(owner, animationRate, (f32)updateRate);
        state->flag += updateRate;
        if (state->flag >= 0x3C) {
            state->flag -= 0x3C;
            state->active = 4;
            amSndPlay(0x13, 0);
            transitioned = 1;
        }
        break;
    case 4:
        displayIndex = 2;
        func_8005ABA8(owner, animationRate, (f32)updateRate);
        state->flag += updateRate;
        if (state->flag >= 0x3C) {
            state->flag -= 0x3C;
            state->active = 5;
            amSndPlay(0x14, 0);
            overlay90SequenceReloc(0x1E);
            gOverlay90SequenceStateReloc = 0;
            transitioned = 1;
        }
        break;
    case 5:
        displayIndex = 3;
        func_8005ABA8(owner, animationRate, (f32)updateRate);
        state->flag += updateRate;
        value = state->flag;
        if (value >= 0x1E) {
            gOverlay90SequenceStateReloc = 0x83;
        } else if (value >= 0xF) {
            gOverlay90SequenceStateReloc = 0x84;
        }
        if (value >= 0x3C) {
            state->flag = value - 0x3C;
            state->active = 6;
            amSndPlay(0x15, 0);
            overlay90SequenceReloc(0x1F);
            gOverlay90SequenceDoneReloc = 0;
            transitioned = 1;
        }
        break;
    case 6:
        displayIndex = 4;
        func_8005ABA8(owner, animationRate, (f32)updateRate);
        state->flag += updateRate;
        if (state->flag >= 0x3C) {
            state->flag -= 0x3C;
            state->active = 7;
            state->value2E = 0;
            state->value2C = -0x40;
            func_8005AD64(owner, 0, -1, zero);
            if (camGetMode() == 0) {
                Overlay90Level *level;

                level = levelGetLevel();
                if (level->sequence == 0) {
                    func_80000510(2);
                } else {
                    func_80000510(level->sequence);
                }
            }
            transitioned = 1;
        }
        break;
    case 7:
        displayIndex = 4;
        remaining = updateRate - 1;
        if (updateRate != 0) {
            do {
                state->value10 += riseRate;
                state->value14 += state->value10;
                state->value24 += state->value2A;
                state->value26 += state->value2C;
                state->value28 += state->value2E;
                if (state->value2A < 0x40) {
                    state->value2A += 2;
                }
                if (state->value2C < 0x80) {
                    state->value2C += 8;
                }
                if (state->value2E < 0x80) {
                    state->value2E += 8;
                }
                remaining--;
            } while (remaining != 0);
        }
        if (state->value14 > 5.0f) {
            state->value14 = 5.0f;
        }
        if (state->value26 > 0x4000) {
            state->value26 = 0x4000;
        }
        if (state->value28 > 0x2000) {
            state->value28 = 0x2000;
        }
        delta.x = zero;
        delta.y = zero;
        delta.z = -state->value14 * (f32)updateRate;
        pointListRPY(1, &state->value24, &delta.x, &delta.x);
        state->value18 += delta.x;
        state->value1C += delta.y;
        state->value20 += delta.z;
        animationValue = state->value14 * animationRate;
        if (animationValue > maximumAnimation) {
            animationValue = maximumAnimation;
        }
        func_8005ABA8(owner, animationValue, (f32)updateRate);
        state->value3C -= updateRate;
        if (state->value3C <= 0) {
            state->value3C = 1;
        }
        state->flag += updateRate;
        if (state->flag >= 0xB5) {
            if (state->value38 != 0) {
                func_800031E8(state->value38);
            }
            func_80006EA0(owner);
            return;
        }
        break;
    default:
        break;
    }

    if (transitioned != 0) {
        updateRate = 0;
        goto process_mode;
    }

    if (owner->attachment != 0) {
        Overlay90Attachment *attachment;
        Overlay90AttachmentEntry *entry;

        attachment = *owner->attachment;
        if ((attachment != 0) && (attachment->entries != 0)) {
            entry = attachment->entries;
            remaining = attachment->header->count - 1;
            if (attachment->header->count != 0) {
                do {
                    if (entry->flags & 0x100000) {
                        entry->value = displayIndex << 8;
                    }
                    entry++;
                    remaining--;
                } while (remaining != 0);
            }
        }
    }

    value = (s32)(state->value14 * 1024.0f) + 0x800;
    if (value > 0x1800) {
        value = 0x1800;
    }
    state->value36 += value * originalUpdateRate;
    state->value3E = 0xD;
    state->value40 = state->value36;
    state->value42 = 0x13;
    state->value44 = state->value36;
    state->value46 = 0x2000;

    sine = func_8002A8BC(state->angle);
    cosine = func_8002A8C0(state->angle);
    side = state->value18;
    forward = state->value20 - 100.0f;
    radial = func_8002A8C0(state->value34) * state->value30 +
             state->value1C + 80.0f;
    owner->x = side * sine + forward * cosine + state->x;
    owner->y = state->y + radial;
    owner->z = forward * sine - side * cosine + state->z;
    owner->rotationX = state->value24 + state->angle + 0x800;
    owner->rotationY = state->value26;
    owner->rotationZ = state->value28;
    owner->positionTag = func_8000FAE0(owner->x, owner->y, owner->z);

    if (state->value38 == 0) {
        func_80002FE0(0x16, owner->x, owner->y, owner->z, 1,
                      &state->value38);
    }
    if (state->value38 != 0) {
        soundPitch = state->value14 * 10.0f + 100.0f;
        if (soundPitch > 150.0f) {
            soundPitch = 150.0f;
        }
        soundPitch += (f32)mathRnd(-5, 5);
        func_800031C0(state->value38, owner->x, owner->y, owner->z);
        func_8000309C(state->value38, ((u8 *)&state->value3C)[1]);
        func_800030B4(state->value38, (u8)soundPitch);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o090/overlay_090/func_overlay_090_F00000FC_18D4BF4.s")
#endif
