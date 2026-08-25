#include "overlays/overlay_001.h"

/* ---- overlay1InitializeGaugeObjects ---- */


typedef struct O1GaugeState {
    s8 type; u8 pad001[0x1A7]; u16 flags; u8 pad1AA[0x250];
    s16 enabled; u8 pad3FC[4]; s32 value;
} O1GaugeState;
typedef struct O1GaugeObject { u8 pad00[0x64]; O1GaugeState *state; } O1GaugeObject;
typedef struct O1GaugeTableEntry { u8 pad00[8]; s32 value; u8 pad0C[0x1C]; } O1GaugeTableEntry;
extern O1GaugeTableEntry *overlay1GetGaugeTable(void);
extern O1GaugeObject **overlay1GetGaugeObjects(s32 *count);
extern s32 overlay1RandomRange(s32 minimum, s32 maximum);

/* Plateau: exact 74 words/frame; best is 18 words different, first +0x1C.
 * Byte-form cursor plus named object/loopValue temporaries improve allocation;
 * count's stack home and the retained objects alias remain divergent. */
#ifdef NON_MATCHING
void overlay1InitializeGaugeObjects(void) {
    O1GaugeTableEntry *table;
    O1GaugeObject **objects;
    O1GaugeObject **firstCursor;
    O1GaugeObject **secondCursor;
    O1GaugeObject *object;
    O1GaugeState *state;
    volatile s32 count;
    s32 initialIndex;
    s32 index;
    s32 loopValue;
    s32 maximum;

    table = overlay1GetGaugeTable();
    objects = overlay1GetGaugeObjects((s32 *)&count);
    maximum = 0;
    initialIndex = count - 1;
    index = initialIndex;
    if (count != 0) {
        firstCursor = (O1GaugeObject **)((u8 *)objects + (index * 4));
        do {
            object = *firstCursor--;
            state = object->state;
            loopValue = state->enabled;
            if ((loopValue != 0) && (maximum < state->value)) maximum = state->value;
            loopValue = index;
            index--;
        } while (loopValue != 0);
        index = initialIndex;
    }
    secondCursor = objects + index;
    if (count != 0) {
        do {
            state = (*secondCursor)->state;
            state->flags |= 1;
            if (table[state->type].value == 0) {
                table[state->type].value = overlay1RandomRange(100, 1000) + maximum;
            }
            loopValue = index;
            secondCursor--;
            index--;
        } while (loopValue != 0);
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F0003578_184F958.s")
#endif

/* ---- overlay1AssignRecordIndex ---- */


typedef struct O1VariableRecord { s16 type; u8 size; u8 pad03[9]; u16 index; } O1VariableRecord;
typedef struct O1RecordOwner { u8 pad00[0xC]; u16 index; } O1RecordOwner;
extern s32 D_1D8CRead;
extern void overlay1GetVariableRecords(O1VariableRecord **records, s32 *length,
                                       s32 enabled, O1RecordOwner *owner);

#ifdef NON_MATCHING
void overlay1AssignRecordIndex(s32 unused, O1RecordOwner *owner) {
    volatile s32 private;
    O1VariableRecord *records;
    O1VariableRecord *record;
    s32 length;
    s32 offset;
    s32 next;
    u8 size;

    if (owner->index == 0xFFFF) {
        overlay1GetVariableRecords(&records, &length, 1, owner);
        offset = 0;
        record = records;
        if (length > 0) {
            do {
                if (record->type == 0xCA) {
                    next = record->index + 1;
                    if (D_1D8CRead < next) *(s32 *)0x1D8C = next;
                }
                size = record->size;
                offset += size;
                record = (O1VariableRecord *)((u8 *)record + size);
            } while (offset < length);
        }
        owner->index = (u16)D_1D8CRead;
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F00036A0_184FA80.s")
#endif

/* ---- overlay1ChoosePath ---- */


typedef struct O1ChoiceState {
    s8 playerIndex;
    s8 relationIndex;
    u8 pad002[6];
    u16 objectValue;
    u16 scoreIndex;
    u8 pad00C[0x18E];
    u8 mode;
    u8 active;
    u8 pad19C[0x0C];
    u16 flags;
    u8 pad1AA[0x1D4];
    u8 selector;
    u8 previousSelector;
    u8 transition;
    u8 pad381[0x1B];
    f32 progress;
    u8 pad3A0[8];
    u8 relationModes[8];
} O1ChoiceState;

typedef struct O1ChoiceObject {
    u8 pad000[0x64];
    O1ChoiceState *state;
} O1ChoiceObject;

typedef struct O1ControlPoint {
    f32 x;
    f32 z;
    u8 pad008[4];
    s8 enabled;
    u8 pad00D[3];
} O1ControlPoint;

typedef struct O1ControlTable {
    u8 pad000[0x10];
    u16 flags;
    u8 pad012[2];
    O1ControlPoint points[8];
} O1ControlTable;

typedef struct O1Pair {
    f32 distance;
    f32 value;
    s32 valid;
} O1Pair;

extern O1ControlTable *D_1D60;
extern O1ControlTable *D_1D68;
extern O1ControlTable *D_1D68Read;
extern O1ControlTable *D_1D6C;
extern O1ControlPoint *D_208;
extern O1ControlPoint *D_20C;
extern O1ControlPoint *D_210;
extern O1ControlPoint *D_214;
extern s32 D_1D94;
extern f32 D_E8;
extern f32 D_EC;
extern f32 D_F0;
extern f32 D_F4;

extern f32 overlay1RandomWave(s32 value);
extern O1ChoiceObject **overlay1GetChoiceObjects(s32 *count);
extern void overlay1SubmitChoice(O1ChoiceObject *object);
extern void overlay1InterpolatePath(f32 *outX, f32 *outZ, s32 path,
                                    f32 offset);
extern O1ChoiceObject *overlay1FindChoice(f32 progress,
                                         O1ControlTable *table,
                                         s32 minimum, s32 *scores);
extern f32 overlay1MeasureChoice(f32 first, f32 second);

/* Plateau (2026-08-24): -O2 -mips2 -Wab,-r4300_mul was best across the
 * flag lattice, but remains 0x4 bytes long with 360 of 446 words differing
 * and a first mismatch at +0xC.  The gap spans the full state-machine
 * register schedule rather than one local expression-order choice. */
#ifdef NON_MATCHING
void func_overlay_001_F0003750_184FB30(f32 *outX, f32 *outZ) {
    O1ChoiceState *otherState;
    O1ChoiceState *chosenState;
    O1ChoiceObject **objects;
    O1ChoiceObject **cursor;
    O1ChoiceObject *object;
    O1ChoiceObject *found;
    O1ControlTable *table;
    s32 i;
    s32 scores[8];
    s32 selected;
    s32 loopValue;
    s32 step;
    s32 value;
    register u8 path;
    f32 weight;
    f32 difference;
    f32 temporaryX;
    f32 temporaryZ;

    if (D_1DA0->transition != 0) {
        weight = (overlay1RandomWave((D_1DA0->transition << 7) + 0x8000) +
                  1.0f) * 0.5f;
        overlay1InterpolatePath(outX, outZ, D_1DA0->previousSelector, 0.5f);
        overlay1InterpolatePath(&temporaryX, &temporaryZ, D_1DA0->selector,
                                0.5f);
        *outX = ((*outX - temporaryX) * weight) + temporaryX;
        *outZ = ((*outZ - temporaryZ) * weight) + temporaryZ;
        value = D_1D94 * 8;
        if (D_1DA0->transition >= value) {
            D_1DA0->transition -= value;
        } else {
            D_1DA0->transition = 0;
        }
        return;
    }

    if (D_1D64->flags & 1) {
        table = D_1D68;
        path = D_1DA0->selector;
        if (table->points[path].enabled == 0) {
            i = 7;
            do {
                if (table->points[i].enabled != 0 &&
                    i != D_1DA0->selector &&
                    table->points[D_1DA0->selector].x == table->points[i].x &&
                    table->points[D_1DA0->selector].z == table->points[i].z) {
                    D_1DA0->previousSelector = i;
                    D_1DA0->selector = i;
                    D_1DA0->transition = 0;
                    path = D_1DA0->selector;
                    break;
                }
                loopValue = i;
                i--;
            } while (loopValue != 0);
        }
        path = D_1DA0->selector;
        overlay1InterpolatePath(outX, outZ, path, 0.5f);
        return;
    }

    i = 7;
    do {
        difference = (f32)(i - D_1DA0->selector);
        if (difference < 0.0f) difference = -difference;
        scores[i] = (s32)(48.0f - difference * 6.0f);
        loopValue = i;
        i--;
    } while (loopValue != 0);

    objects = overlay1GetChoiceObjects(&i);
    if (i-- != 0) {
        cursor = objects + i;
        do {
            object = *cursor--;
            otherState = object->state;
            if (otherState != D_1DA0 && !(otherState->flags & 8)) {
                difference =
                    D_1BA8[D_1DA0->playerIndex][otherState->playerIndex].value;
                if (((difference > -2.0f) && (difference < 2.0f)) ||
                    ((D_E8 < difference) && (difference < 0.0f) &&
                     (D_1DA0->relationModes[otherState->relationIndex] == 1))) {
                    value = otherState->selector;
                    step = 1;
                    if (value < D_1DA0->selector) step = -1;
                    do {
                        if (difference > 0.0f) {
                            weight = difference;
                        } else {
                            weight = -difference;
                        }
                        scores[value] =
                            (s32)((f32)scores[value] -
                                  ((2.0f - weight) * 64.0f));
                        value += step;
                    } while (value >= 0 && value < 8);
                }
                if ((difference > -2.0f) && (difference < 0.0f) &&
                    D_1DA0->relationModes[otherState->relationIndex] >= 4) {
                    scores[otherState->selector] =
                        (s32)((f32)scores[otherState->selector] +
                              D_EC);
                }
            }
            loopValue = i;
            i--;
        } while (loopValue != 0);
    }

    table = D_1D68Read;
    i = 7;
    do {
        scores[i] += table->points[i].enabled;
        loopValue = i;
        i--;
    } while (loopValue != 0);

    i = 7;
    do {
        if (table->points[i].enabled == 0) scores[i] = -1000000;
        loopValue = i;
        i--;
    } while (loopValue != 0);

    value = -1000000;
    if (D_1DA0->active != 0 && D_1DA0->mode == 6) {
        found = overlay1FindChoice(D_1DA0->progress, table, value, scores);
        if (found != 0) {
            chosenState = found->state;
            weight = (f32)chosenState->objectValue * D_F0;
            difference = overlay1MeasureChoice(weight, D_1DA0->progress);
            if (difference < D_F4) {
                overlay1SubmitChoice(D_1D9C);
            } else if (difference < 3.0f) {
                scores[chosenState->scoreIndex] += 1000;
            }
        }
    }

    selected = -1;
    i = 0;
    do {
        if (value < scores[i]) {
            selected = i;
            value = scores[i];
            D_208 = &D_1D60->points[i];
            D_20C = &D_1D64->points[i];
            D_210 = &D_1D68->points[i];
            D_214 = &D_1D6C->points[i];
        }
        i++;
    } while (i < 8);

    overlay1InterpolatePath(outX, outZ, D_1DA0->selector, 0.5f);
    if (selected != -1 && selected != D_1DA0->selector) {
        D_1DA0->previousSelector = D_1DA0->selector;
        D_1DA0->selector = selected;
        D_1DA0->transition = 0xFF;
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F0003750_184FB30.s")
#endif

/* ---- overlay1SubmitGlobals ---- */


/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern s32 gOverlay1SubmitArg2;
extern s32 gOverlay1SubmitArg3;
extern void overlay1SubmitReloc(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

void overlay1SubmitGlobals(s32 arg0, s32 arg1) {
    overlay1SubmitReloc(arg0, arg1, gOverlay1SubmitArg2, gOverlay1SubmitArg3);
}

/* ---- overlay1SubmitAll ---- */


/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern s32 gOverlay1SubmitArg2;
extern s32 gOverlay1SubmitArg3;
extern s32 gOverlay1SubmitArg4;
extern s32 gOverlay1SubmitArg5;
extern void overlay1SubmitAllReloc(s32 arg0, s32 arg1, s32 arg2, s32 arg3,
                                   s32 arg4, s32 arg5);

void overlay1SubmitAll(s32 arg0, s32 arg1) {
    overlay1SubmitAllReloc(arg0, arg1, gOverlay1SubmitArg2,
                           gOverlay1SubmitArg3, gOverlay1SubmitArg4,
                           gOverlay1SubmitArg5);
}

/* ---- overlay1AngleBetweenSamples ---- */


typedef struct Overlay1SampleState {
    u8 pad0[0x37E];
    u8 selector;
} Overlay1SampleState;

/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern Overlay1SampleState *gOverlay1SampleState;
extern void overlay1SampleReloc(f32 *x, f32 *y, s32 selector, f32 scale);
extern s32 overlay1SampleAngleReloc(f32 x, f32 y);

s32 overlay1AngleBetweenSamples(f32 unusedX, f32 unusedY) {
    f32 firstX;
    f32 firstY;
    f32 secondX;
    f32 secondY;

    overlay1SampleReloc(&firstX, &firstY, gOverlay1SampleState->selector, 1.0f);
    overlay1SampleReloc(&secondX, &secondY, gOverlay1SampleState->selector,
                        2.5f);
    return (s16)(overlay1SampleAngleReloc(firstX - secondX,
                                          firstY - secondY) - 0x8000);
}

/* ---- overlay1RelativeAngles ---- */


typedef struct Overlay1Position {
    u8 pad0[0xC];
    f32 x;
    u8 pad10[4];
    f32 y;
} Overlay1Position;

/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern Overlay1Position *gOverlay1Position;
extern s32 overlay1AngleReloc(f32 x, f32 y);

s32 overlay1RelativeAngleA(f32 x, f32 y) {
    s32 angle;
    f32 deltaX;
    f32 deltaY;

    deltaX = x - gOverlay1Position->x;
    deltaY = y - gOverlay1Position->y;
    angle = overlay1AngleReloc(deltaX, deltaY);
    return (s16)(angle + 0x8000);
}

s32 overlay1RelativeAngleB(f32 x, f32 y) {
    s32 angle;
    f32 deltaX;
    f32 deltaY;

    deltaX = x - gOverlay1Position->x;
    deltaY = y - gOverlay1Position->y;
    angle = overlay1AngleReloc(deltaX, deltaY);
    return (s16)(angle + 0x8000);
}

/* ---- overlay1TransitionState ---- */


typedef struct Transform {
    s16 rotY;
    s16 rotX;
    s16 rotZ;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x21];
    u8 alpha;
    u8 pad3A[0xE];
    struct ObjectHeader *header;
} Transform;

typedef struct ObjectHeader {
    u8 pad00[6];
    u16 flags;
} ObjectHeader;

typedef struct Spawned {
    s16 angle;
    u8 pad02[2];
    f32 y;
    u8 pad08[4];
    f32 x;
    union {
        f32 y2;
        u16 flags;
    } at10;
    f32 z;
    u8 pad18[0x2C];
    f32 x2;
    f32 z2;
} Spawned;

typedef struct State {
    u8 pad000[0x16C];
    u8 done;
    u8 pad16D[3];
    u8 phase;
    u8 pad171[0x1F];
    u8 fade;
    u8 active;
    u8 pad192[0x16];
    u16 flags;
    u8 pad1AA[0x1D2];
    s16 pathId;
    u8 selectorA;
    u8 selectorB;
    u8 selectorC;
    u8 pad381[0x5F];
    Spawned *spawned;
} State;

extern s32 G_o1_83e4;
extern void ext_o7_ccc(Transform *, s32);
extern Spawned *local_378(State *);
extern void ext_o0_1bed0(Transform *, f32, f32, f32, s16, s16, s16);
extern void ext_o0_1c6bc(Transform *, State *);
extern void ext_o0_5a914(Transform *, s32, s32, s32);
extern Spawned *local_414(s16, Spawned **);
extern s16 local_c0(Spawned *);

/* Workbench: structure-mismatch; best is 234/237 words, 160 positional differences, first +0x20.
 * Tried constant audit, phase widths, stack-slot census, and indexed/typed pointer ASTs.
 * The local slot stays +0x10 high; an index fold leaves three words missing and early pool/temp drift. */
#ifdef NON_MATCHING
void func_overlay_001_F0003FD8_18503B8(Transform *obj, State *state, s32 updateRate) {
    Spawned *sp3C;
    s32 value;
    u8 phaseValue;
    s32 phase;
    u8 index;
    u8 *point;
    Spawned *spawned;

    if (G_o1_83e4 == 3) {
        phase = state->phase;
        if (phase == 0) {
            return;
        }
        phaseValue = phase;
        if (phase == 1) {
                ext_o7_ccc(obj, 0x13);
                state->spawned = local_378(state);
                state->phase = 2;
                return;
            }
            if (phaseValue == 2) {
                value = state->fade - (updateRate * 4);
                if (value <= 0) {
                    state->phase = 3;
                    return;
                }
                state->fade = value;
                return;
            }
            if (phaseValue == 3) {
                spawned = state->spawned;
                obj->x = spawned->x;
                obj->y = spawned->at10.y2 + 100.0f;
                obj->z = spawned->z;
                obj->rotX = 0;
                obj->rotZ = 0;
                obj->rotY = spawned->angle;
                ext_o0_1bed0(obj, obj->x, obj->y, obj->z, obj->rotY, obj->rotX, obj->rotZ);
                ext_o0_1c6bc(obj, state);
                state->flags &= ~8;
                obj->header->flags &= ~1;
                state->fade = 0;
                state->active = 1;
                state->phase = 4;
                return;
            }
            if (phaseValue == 4) {
                value = state->fade + (updateRate * 4);
                if (value >= 255) {
                    state->fade = 255;
                    state->phase = 5;
                    return;
                }
                state->fade = value;
                return;
            }
            if (phaseValue == 5) {
                obj->header->flags |= 1;
                state->phase = 0;
                state->active = 0;
                state->done = 1;
                ext_o0_5a914(obj, 12, -1, 0);
                state->spawned = 0;
        }
    } else {
        phase = state->phase;
        if (phase == 0) {
            return;
        }
        phaseValue = phase;
        if (phase == 1) {
                ext_o7_ccc(obj, 0x13);
                state->spawned = local_414(state->pathId, &sp3C);
                state->pathId = local_c0(sp3C);
                state->spawned->at10.flags |= 8;
                state->phase = 2;
                return;
            }
            if (phaseValue == 2) {
                value = obj->alpha - (updateRate * 4);
                if (value <= 0) {
                    state->phase = 3;
                    return;
                }
                obj->alpha = value;
                return;
            }
            if (phaseValue == 3) {
                spawned = state->spawned;
                index = 3;
                state->selectorA = index;
                state->selectorB = index;
                state->selectorC = 0;
                point = (u8 *)spawned + (index << 4);
                obj->x = *(f32 *)(point + 0x14);
                obj->y = spawned->y + 100.0f;
                point += 0x14;
                obj->z = *(f32 *)(point + 4);
                obj->rotX = 0;
                obj->rotZ = 0;
                obj->rotY = *(s16 *)((u8 *)spawned + 0xC) + 0x4000;
                ext_o0_1bed0(obj, obj->x, obj->y, obj->z, obj->rotY, obj->rotX, obj->rotZ);
                ext_o0_1c6bc(obj, state);
                state->flags &= ~8;
                obj->header->flags &= ~1;
                obj->alpha = 0;
                state->active = 1;
                state->phase = 4;
                return;
            }
            if (phaseValue == 4) {
                value = obj->alpha + (updateRate * 4);
                if (value >= 255) {
                    obj->alpha = 255;
                    state->phase = 5;
                    return;
                }
                obj->alpha = value;
                return;
            }
            if (phaseValue == 5) {
                obj->header->flags |= 1;
                state->phase = 0;
                state->active = 0;
                state->done = 1;
                ext_o0_5a914(obj, 12, -1, 0);
                state->spawned->at10.flags &= 0xFFF7;
                state->spawned = 0;
        }
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F0003FD8_18503B8.s")
#endif

/* ---- overlay1UpdateObjectPhysics ---- */

#include "tools/m2c/m2c_macros.h"
#undef NULL
#define NULL 0
#undef M2C_BITWISE
#define M2C_BITWISE(type, expr) ((type)(s32)(expr))

struct _m2c_stack_func_overlay_001_F000438C_185076C {
    /* 0x000 */ char pad0[0x4C];
    /* 0x04C */ s16 sp4C;                           /* inferred */
    /* 0x04E */ s16 sp4E;                           /* inferred */
    /* 0x050 */ M2C_UNK *sp50;                      /* inferred */
    /* 0x054 */ M2C_UNK *sp54;                      /* inferred */
    /* 0x058 */ s32 *sp58;                          /* inferred */
    /* 0x05C */ char pad5C[4];
    /* 0x060 */ s32 sp60;                           /* inferred */
    /* 0x064 */ char pad64[4];
    /* 0x068 */ f32 sp68;                           /* inferred */
    /* 0x06C */ char pad6C[0xE];                    /* maybe part of sp68[4]? */
    /* 0x07A */ s16 sp7A;                           /* inferred */
    /* 0x07C */ s16 sp7C;                           /* inferred */
    /* 0x07E */ s16 sp7E;                           /* inferred */
    /* 0x080 */ f32 sp80;                           /* inferred */
    /* 0x084 */ char pad84[4];
    /* 0x088 */ f32 sp88;                           /* inferred */
    /* 0x08C */ f32 sp8C;                           /* inferred */
    /* 0x090 */ f32 sp90;                           /* inferred */
    /* 0x094 */ char pad94[4];
    /* 0x098 */ f32 sp98;                           /* inferred */
    /* 0x09C */ char pad9C[4];
    /* 0x0A0 */ f32 spA0;                           /* inferred */
    /* 0x0A4 */ f32 spA4;                           /* inferred */
    /* 0x0A8 */ f32 spA8;                           /* inferred */
    /* 0x0AC */ f32 spAC;                           /* inferred */
    /* 0x0B0 */ char padB0[0x10];                   /* maybe part of spAC[5]? */
    /* 0x0C0 */ s32 spC0;                           /* inferred */
    /* 0x0C4 */ char padC4[0x10];                   /* maybe part of spC0[5]? */
    /* 0x0D4 */ f32 *spD4;                          /* inferred */
    /* 0x0D8 */ s16 *spD8;                          /* inferred */
    /* 0x0DC */ s16 spDC;                           /* inferred */
    /* 0x0DE */ s16 spDE;                           /* inferred */
    /* 0x0E0 */ s16 spE0;                           /* inferred */
    /* 0x0E2 */ char padE2[2];
    /* 0x0E4 */ f32 spE4;                           /* inferred */
    /* 0x0E8 */ f32 spE8;                           /* inferred */
    /* 0x0EC */ f32 spEC;                           /* inferred */
    /* 0x0F0 */ M2C_UNK spF0;                       /* inferred */
    /* 0x0F0 */ char padF0[0x40];
    /* 0x130 */ s32 *sp130;                         /* inferred */
    /* 0x134 */ char pad134[4];
};                                                  /* size = 0x138 */

s32 func_overlay_001_F00004B4_184C894();            /* extern */
M2C_UNK func_overlay_001_F0007D6C_185414C(s32, s32, s16, s16, s16, s16, s16 *, s16 *, void *); /* extern */
f32 *ext_o8_8();
void ext_o0_1ee0c();
void ext_o8_49dc();
void ext_o0_1d4c0();
void ext_o0_29adc();
s32 ext_o0_1312c(f32, f32, void *, u32, void *);
void ext_o0_1ecfc();
s32 ext_o2_123c(f32, f32, void *);
s16 ext_o0_2a4c0(f32, f32);
s16 ext_o0_2a5bc(s16, s16);
void ext_o7_edc();
f32 ext_o8_1000(void *, void *, f32);
s32 ext_o0_2630c();
void ext_o0_29598();
f32 ext_o0_2a46c(s16);
void ext_o0_2d98();
void ext_o0_2b90();
void ext_o0_2d70();
f32 ext_o0_2a428(f32, s32);
f32 ext_o0_2a470(s16);
s32 *ext_o0_1e174(void *, void *, f32);
s32 *ext_o0_1d920(void *, void *, f32);
s32 ext_o0_7cd8(void *, f32, f32, f32);
void ext_o8_49a4(f32, void *);
f32 ext_o8_34a0(void *, void *, f32, f32);
void ext_o8_49b4();
void ext_o0_1cfcc();
void ext_o8_3278();
void ext_o0_1d510();
void ext_o8_2ec0();
void ext_o8_3018();
void ext_o0_3e99c();
extern f32 D_4;
extern M2C_UNK D_8C;
extern M2C_UNK D_D4;
extern s32 G_o1_83e4;
extern f32 G_rt_458c4;
extern s32 G_rt_43a3c;
extern u8 G_offd_31a4;
extern u8 LOCAL_BSS[];
extern f32 LOCAL_DATA_4;
extern s32 LOCAL_BSS_1D78;
extern s32 LOCAL_BSS_1D94;
extern void *LOCAL_BSS_1BA4;
extern void *LOCAL_BSS_1D9C;
extern f32 LOCAL_RODATA_F8;
extern f32 LOCAL_RODATA_FC;
extern f32 LOCAL_RODATA_100;
extern f32 LOCAL_RODATA_104;
extern f32 LOCAL_RODATA_108;
extern f32 LOCAL_RODATA_10C;
extern f32 LOCAL_RODATA_110;
extern f32 LOCAL_RODATA_114;
extern f32 LOCAL_RODATA_118;
extern f32 LOCAL_RODATA_11C;
extern f32 LOCAL_RODATA_120;
extern f32 LOCAL_RODATA_124;
extern f32 LOCAL_RODATA_128;
extern f32 LOCAL_RODATA_12C;
extern f32 LOCAL_RODATA_130;
extern f32 LOCAL_RODATA_134;
extern f32 LOCAL_RODATA_138;
extern f32 LOCAL_RODATA_13C;
extern f32 LOCAL_RODATA_140;
extern f32 LOCAL_RODATA_144;
extern f32 LOCAL_RODATA_148;
extern f32 LOCAL_RODATA_14C;
extern f32 LOCAL_RODATA_150;
extern f32 LOCAL_RODATA_154;
extern f32 LOCAL_RODATA_158;
extern f32 LOCAL_RODATA_15C;

typedef struct HitRecord {
    f32 value;
    u32 flags;
} HitRecord;

/* Plateau (2026-08-24): the complete flag lattice favored
 * -O2 -mips2 -Wo,-loopunroll,0, but the candidate was 0x38 bytes short,
 * differed in 1462 of 1542 words, and first diverged at +0x0.  This m2c
 * draft needs a typed ABI/frame rewrite; local expression permutation is
 * not a credible route to the retail register and control-flow shape. */
#ifdef NON_MATCHING
void func_overlay_001_F000438C_185076C(f32 *arg0, s32 arg1) {
    f32 *sp130;
    HitRecord spF0[8];
    f32 spEC;
    f32 spE8;
    f32 spE4;
    s16 spE0;
    s16 spDE;
    s16 spDC;
    f32 spD8;
    f32 spD4;
    s32 spC0;
    f32 spAC;
    f32 spA8;
    f32 spA4;
    f32 spA0;
    f32 sp98;
    f32 sp90;
    f32 sp8C;
    f32 sp88;
    f32 sp80;
    s16 sp7E;
    s16 sp7C;
    s16 sp7A;
    f32 sp68;
    s32 sp60;
    s32 *sp58;
    M2C_UNK *sp54;
    M2C_UNK *sp50;
    s16 sp4E;
    s16 sp4C;
    M2C_UNK (*temp_a0_4)(M2C_UNK, M2C_UNK *);
    M2C_UNK (*temp_v0_15)(u8, M2C_UNK *);
    M2C_UNK *var_a0_2;
    M2C_UNK *var_a1;
    f32 temp_f0;
    f32 temp_f0_2;
    f32 temp_f0_3;
    f32 temp_f0_4;
    f32 temp_f0_5;
    f32 temp_f0_6;
    f32 temp_f0_7;
    f32 temp_f0_8;
    f32 temp_f10_2;
    f32 temp_f12;
    f32 temp_f12_3;
    f32 temp_f12_4;
    f32 temp_f12_5;
    f32 temp_f12_6;
    f32 temp_f12_7;
    f32 temp_f12_8;
    f32 temp_f14_2;
    f32 temp_f14_3;
    f32 temp_f16;
    f32 temp_f18;
    f32 temp_cos;
    f32 temp_f2;
    f32 temp_f2_2;
    f32 temp_f2_3;
    f32 temp_f2_4;
    f32 temp_f2_5;
    f32 temp_f2_6;
    f32 temp_f2_7;
    f32 temp_f2_8;
    f32 temp_f8_2;
    f32 var_f0;
    f32 var_f14;
    f32 var_f14_2;
    f32 var_f14_3;
    f32 var_f14_4;
    f32 var_f14_5;
    f32 var_f14_6;
    f32 var_f16;
    f32 var_f2;
    f32 var_f2_2;
    s16 *temp_a0;
    s16 *temp_a0_2;
    s16 *temp_a0_3;
    s16 *temp_a2;
    s16 *temp_s0;
    s16 temp_v0_14;
    s16 temp_v0_2;
    s16 temp_v1;
    s16 var_v0_2;
    s16 var_v0_3;
    s16 var_v0_4;
    s16 var_v0_5;
    s32 (*temp_v0_4)(M2C_UNK *);
    f32 *temp_v0;
    s32 *temp_v0_12;
    s32 temp_v0_3;
    s32 *var_v0_7;
    s32 temp_f10;
    s32 temp_f12_2;
    s32 temp_f14;
    s32 temp_f8;
    s32 temp_t6;
    s32 temp_v0_10;
    s32 temp_v0_13;
    s32 temp_v0_9;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 var_a0;
    s32 var_v0_6;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    s32 var_v1_5;
    s8 temp_v0_8;
    u8 temp_a0_5;
    u8 temp_v0_11;
    u8 temp_v0_6;
    u8 var_v0;
    void *temp_v0_5;
    void *temp_v0_7;
    HitRecord *var_v1;

    temp_s0 = M2C_FIELD(arg0, s16 **, 0x64);
    if (func_overlay_001_F00004B4_184C894() != 0) {
        temp_v0 = ext_o8_8(temp_s0);
        sp130 = temp_v0;
        temp_t6 = G_o1_83e4;
        G_rt_458c4 = *temp_v0;
        if (temp_t6 == 1) {
            if ((G_rt_43a3c == 0) && (M2C_FIELD(temp_s0, s32 *, 0x438) == 0) && (M2C_FIELD(temp_s0, s16 *, 0x102) == 0) && !(M2C_FIELD(temp_s0, u16 *, 0x1A8) & 8)) {
                temp_v0_2 = M2C_FIELD(temp_s0, s16 *, 0x37C);
                if (temp_v0_2 != M2C_FIELD(temp_s0, s16 *, 0x3CC)) {
                    M2C_FIELD(temp_s0, s16 *, 0x3CC) = temp_v0_2;
                    M2C_FIELD(temp_s0, s16 *, 0x3CE) = 0;
                } else {
                    M2C_FIELD(temp_s0, s16 *, 0x3CE) = (s16) (M2C_FIELD(temp_s0, s16 *, 0x3CE) + arg1);
                }
                if ((f32) M2C_FIELD(temp_s0, s16 *, 0x3CE) > 360.0f) {
                    if (M2C_FIELD(temp_s0, u8 *, 0x170) == 0) {
                        M2C_FIELD(temp_s0, u8 *, 0x170) = 1U;
                    }
                    goto block_13;
                }
            } else {
block_13:
                M2C_FIELD(temp_s0, s16 *, 0x3CE) = 0;
            }
        }
        M2C_FIELD(temp_s0, f32 *, 0x70) = 0.0f;
        if ((M2C_FIELD(temp_s0, s8 *, 0x18D) != 0) || (M2C_FIELD(temp_s0, s16 *, 0x158) != 0) || (M2C_FIELD(temp_s0, u8 *, 0x170) != 0) || (M2C_FIELD(temp_s0, s16 *, 0x3FA) != 0)) {
            ext_o0_1ee0c(temp_s0, (f32 *)1);
        }
        LOCAL_BSS_1D94 = arg1;
        LOCAL_DATA_4 = (f32) arg1;
        sp68 = -M2C_FIELD(temp_s0, f32 *, 4);
        ext_o8_49dc(NULL);
        temp_f0 = LOCAL_RODATA_F8;
        M2C_FIELD(arg0, s32 *, 0x80) = 0;
        M2C_FIELD(temp_s0, s32 *, 0x428) = 0;
        M2C_FIELD(temp_s0, s32 *, 0x42C) = 0;
        M2C_FIELD(temp_s0, s32 *, 0x41C) = 0;
        M2C_FIELD(temp_s0, s32 *, 0x420) = 0;
        if (M2C_FIELD(temp_s0, f32 *, 4) < temp_f0) {
            M2C_FIELD(temp_s0, f32 *, 4) = temp_f0;
        }
        temp_f12 = LOCAL_RODATA_FC;
        if (temp_f12 < M2C_FIELD(temp_s0, f32 *, 4)) {
            M2C_FIELD(temp_s0, f32 *, 4) = temp_f12;
        }
        if (M2C_FIELD(temp_s0, f32 *, 8) < temp_f0) {
            M2C_FIELD(temp_s0, f32 *, 8) = temp_f0;
        }
        if (temp_f12 < M2C_FIELD(temp_s0, f32 *, 8)) {
            M2C_FIELD(temp_s0, f32 *, 8) = temp_f12;
        }
        ext_o0_1d4c0(arg0, temp_s0);
        spDC = -M2C_FIELD(temp_s0, s16 *, 0xF0);
        spDE = -M2C_FIELD(arg0, s16 *, 2);
        spEC = 0.0f;
        spE4 = 0.0f;
        spE0 = -M2C_FIELD(arg0, s16 *, 4);
        spE8 = -1.0f;
        ext_o0_29adc(&spDC, &spE4);
        M2C_FIELD(temp_s0, f32 *, 0x60) = spE4;
        M2C_FIELD(temp_s0, f32 *, 0x64) = spE8;
        M2C_FIELD(temp_s0, f32 *, 0x5C) = spEC;
        temp_v0_3 = ext_o0_1312c(M2C_FIELD(arg0, f32 *, 0xC), M2C_FIELD(arg0, f32 *, 0x14), NULL, 0x08010000, spF0);
        var_f0 = -32768.0f;
        var_a0 = temp_v0_3 - 1;
        M2C_FIELD(temp_s0, f32 *, 0x68) = -32768.0f;
        if (temp_v0_3 != NULL) {
            var_v1 = &spF0[var_a0];
            do {
                if (M2C_FIELD(var_v1, s32 *, 4) & 0x10000) {
                    M2C_FIELD(temp_s0, f32 *, 0x68) = (f32) M2C_FIELD(var_v1, f32 *, 0);
                }
                if (M2C_FIELD(var_v1, s32 *, 4) & 0x08000000) {
                    var_f0 = M2C_FIELD(var_v1, f32 *, 0);
                }
                var_v1 -= 1;
                var_a0 -= 1;
            } while (var_a0 != 0);
        }
        temp_f2 = M2C_FIELD(temp_s0, f32 *, 0x68);
        if (M2C_FIELD(arg0, f32 *, 0x10) < temp_f2) {
            M2C_FIELD(temp_s0, u8 *, 2) = 1U;
            M2C_FIELD(temp_s0, f32 *, 0x6C) = temp_f2;
        } else {
            M2C_FIELD(temp_s0, u8 *, 2) = 0U;
            M2C_FIELD(temp_s0, f32 *, 0x6C) = 0.0f;
        }
        if (M2C_FIELD(arg0, f32 *, 0x10) < var_f0) {
            if (M2C_FIELD(temp_s0, u8 *, 0x170) == 0) {
                M2C_FIELD(temp_s0, u8 *, 0x170) = 1U;
            }
            if ((M2C_FIELD(temp_s0, u8 *, 2) != 0) && (M2C_FIELD(temp_s0, u8 *, 3) != 1)) {
                ext_o0_1ecfc((s16 *) arg0, (f32 *) temp_s0);
            }
        }
        var_v0 = M2C_FIELD(temp_s0, u8 *, 0x192);
        if ((s32) var_v0 >= 0xB) {
            var_v0 = 0xA;
        }
        var_a0_2 = &D_8C;
        var_v1_2 = 1;
        spAC = (M2C_FIELD(sp130, f32 *, 0x40) + ((f32) var_v0 * M2C_FIELD(sp130, f32 *, 8))) * M2C_FIELD(temp_s0, f32 *, 0x3A0);
        do {
            temp_v0_4 = M2C_FIELD(var_a0_2, s32 (**)(M2C_UNK *), 0);
            if ((temp_v0_4 != NULL) && (M2C_FIELD(var_a0_2, u16 *, 0xC) & (1 << M2C_FIELD(temp_s0, u8 *, 0x381)))) {
                sp60 = var_v1_2;
                sp54 = var_a0_2;
                if (temp_v0_4(var_a0_2) != 0) {
                    M2C_FIELD(temp_s0, u8 *, 0x381) = (u8) var_v1_2;
                }
            }
            var_v1_2 += 1;
            var_a0_2 += 0x10;
        } while (var_v1_2 != 4);
        M2C_FIELD(LOCAL_BSS + (M2C_FIELD(temp_s0, u8 *, 0x381) * 0x10), M2C_UNK (**)(f32 *, f32 *), 0x80)(&spD8, &spD4);
        temp_a2 = (s16 *) LOCAL_BSS_1BA4;
        if (temp_a2 != NULL) {
            temp_v0_5 = LOCAL_BSS_1D9C;
            if (ext_o2_123c(M2C_FIELD(temp_v0_5, f32 *, 0xC), M2C_FIELD(temp_v0_5, f32 *, 0x14), temp_a2) != NULL) {
                temp_v0_5 = D_1D9C;
                M2C_FIELD(D_1DA0, f32 *, 0x3D0) = M2C_FIELD(temp_v0_5, f32 *, 0xC);
                M2C_FIELD(D_1DA0, f32 *, 0x3D4) = M2C_FIELD(temp_v0_5, f32 *, 0x14);
            }
            if (ext_o2_123c(spD8, spD4, LOCAL_BSS_1BA4) != NULL) {
                M2C_FIELD(D_1DA0, f32 *, 0x3D8) = spD8;
                M2C_FIELD(D_1DA0, f32 *, 0x3DC) = spD4;
            }
            temp_f14 = (s32) M2C_FIELD(D_1DA0, f32 *, 0x3D8);
            temp_f12_2 = (s32) M2C_FIELD(D_1DA0, f32 *, 0x3DC);
            func_overlay_001_F0007D6C_185414C(temp_f12_2, temp_f14, (s16) (s32) M2C_FIELD(D_1DA0, f32 *, 0x3D0), (s16) (s32) M2C_FIELD(D_1DA0, f32 *, 0x3D4), (s16) temp_f14, (s16) temp_f12_2, &sp4E, &sp4C, D_1D9C);
            spD8 = (f32) sp4E;
            spD4 = (f32) sp4C;
        }
        sp7C = ext_o0_2a4c0(spD8 - M2C_FIELD(arg0, f32 *, 0xC), spD4 - M2C_FIELD(arg0, f32 *, 0x14)) + 0x8000;
        sp7A = M2C_FIELD(LOCAL_BSS + (M2C_FIELD(temp_s0, u8 *, 0x381) * 0x10), s16 (**)(f32, f32), 0x84)(spD8, spD4);
        M2C_FIELD(temp_s0, f32 *, 0x3BC) = spD8;
        M2C_FIELD(temp_s0, f32 *, 0x3C0) = spD4;
        temp_f0_2 = (f32) ext_o0_2a5bc(M2C_FIELD(arg0, s16 *, 0), sp7C) * LOCAL_RODATA_100;
        var_f2 = temp_f0_2;
        M2C_FIELD(temp_s0, s32 *, 0x428) = (s32) (temp_f0_2 * LOCAL_RODATA_104);
        if (temp_f0_2 < 0.0f) {
            var_f2 = -temp_f0_2;
        }
        if (G_o1_83e4 == 1) {
            if (var_f2 > 24576.0f) {
                var_v0_2 = 0x6000;
            } else {
                var_v0_2 = (s16) (s32) var_f2;
            }
            temp_f8 = (s32) (var_f2 - 8192.0f);
            if (sp68 < (25.0f - ((f32) var_v0_2 * 0.0009765625f))) {
                M2C_FIELD(temp_s0, s32 *, 0x41C) = (s32) (M2C_FIELD(temp_s0, s32 *, 0x41C) | 0x8000);
            }
            var_v0_3 = (s16) temp_f8;
            if ((s16) temp_f8 < 0) {
                var_v0_3 = (s16) temp_f8 * -1;
            }
            if (((f32) var_v0_3 * LOCAL_RODATA_108) < sp68) {
                M2C_FIELD(temp_s0, s32 *, 0x41C) = (s32) (M2C_FIELD(temp_s0, s32 *, 0x41C) | 0x4000);
            }
            temp_f0_3 = (f32) ext_o0_2a5bc(sp7C, sp7A);
            var_f2_2 = temp_f0_3;
            if (temp_f0_3 < 0.0f) {
                var_f2_2 = -temp_f0_3;
            }
            if ((var_f2_2 * LOCAL_RODATA_10C) < sp68) {
                M2C_FIELD(temp_s0, s32 *, 0x41C) = (s32) (M2C_FIELD(temp_s0, s32 *, 0x41C) | 0x4000);
            }
        } else {
            if (var_f2 > 16384.0f) {
                var_v0_4 = 0x4000;
            } else {
                var_v0_4 = (s16) (s32) var_f2;
            }
            temp_f10 = (s32) (var_f2 - 16384.0f);
            if (sp68 < (25.0f - ((f32) var_v0_4 * 0.0014648438f))) {
                M2C_FIELD(temp_s0, s32 *, 0x41C) = (s32) (M2C_FIELD(temp_s0, s32 *, 0x41C) | 0x8000);
            }
            var_v0_5 = (s16) temp_f10;
            if ((s16) temp_f10 < 0) {
                var_v0_5 = (s16) temp_f10 * -1;
            }
            if (((f32) var_v0_5 * 0.006713867f) < sp68) {
                M2C_FIELD(temp_s0, s32 *, 0x41C) = (s32) (M2C_FIELD(temp_s0, s32 *, 0x41C) | 0x4000);
            }
            if (sp68 < LOCAL_RODATA_110) {
                M2C_FIELD(temp_s0, s32 *, 0x41C) = (s32) (M2C_FIELD(temp_s0, s32 *, 0x41C) | 0x8000);
            }
            temp_v0_6 = M2C_FIELD(temp_s0, u8 *, 0x171);
            if (temp_v0_6 != 0) {
                if (arg1 < (s32) temp_v0_6) {
                    M2C_FIELD(temp_s0, u8 *, 0x171) = (u8) (temp_v0_6 - arg1);
                } else {
                    M2C_FIELD(temp_s0, u8 *, 0x171) = 0U;
                }
                M2C_FIELD(temp_s0, s32 *, 0x42C) = -0x64;
                M2C_FIELD(temp_s0, s32 *, 0x41C) = 0x4000;
            }
        }
        if (G_rt_43a3c != 0) {
            M2C_FIELD(temp_s0, s32 *, 0x41C) = 0x4000;
            M2C_FIELD(arg0, f32 *, 0x1C) = 0.0f;
            M2C_FIELD(arg0, f32 *, 0x20) = 0.0f;
            M2C_FIELD(arg0, f32 *, 0x24) = 0.0f;
            M2C_FIELD(temp_s0, f32 *, 4) = 0.0f;
        }
        if ((LOCAL_BSS_1D78 != 0) && (G_rt_43a3c == 0)) {
            M2C_FIELD(temp_s0, s8 *, 0x183) = 1;
            M2C_FIELD(temp_s0, u8 *, 0x185) = 2U;
            M2C_FIELD(temp_s0, s8 *, 0x187) = 6;
            M2C_FIELD(temp_s0, f32 *, 0x188) = 1.0f;
        }
        if (G_rt_43a3c != 0) {
            ext_o7_edc();
        }
        if (M2C_FIELD(temp_s0, s32 *, 0x438) != 0) {
            M2C_FIELD(temp_s0, s32 *, 0x428) = 0;
            M2C_FIELD(temp_s0, s32 *, 0x42C) = 0;
            M2C_FIELD(temp_s0, s32 *, 0x41C) = 0;
            M2C_FIELD(temp_s0, s32 *, 0x420) = 0;
        }
        temp_v0_7 = M2C_FIELD(temp_s0, void **, 0xD4);
        if (temp_v0_7 != NULL) {
            spAC *= 1.0f + (LOCAL_RODATA_114 * M2C_FIELD(M2C_FIELD(temp_v0_7, void **, 0x64), f32 *, 0x14));
        }
        if (M2C_FIELD(temp_s0, u8 *, 0x185) == 0) {
            temp_f0_4 = M2C_FIELD(temp_s0, f32 *, 0x5C);
            if (temp_f0_4 != 0.0f) {
                var_f14 = 1.0f - (temp_f0_4 * 0.5f * M2C_FIELD(sp130, f32 *, 0xC));
                if (var_f14 < LOCAL_RODATA_118) {
                    var_f14 = LOCAL_RODATA_11C;
                }
                spAC *= var_f14;
            }
        }
        temp_v1 = M2C_FIELD(temp_s0, s16 *, 0x102);
        if ((temp_v1 != 0) && ((temp_v0_8 = M2C_FIELD(arg0, s8 *, 0x3B), (temp_v0_8 == 0x10)) || (temp_v0_8 == 0xF))) {
            var_f14_2 = M2C_FIELD(arg0, f32 *, 0x28) * 1.5f;
            if (var_f14_2 > 1.0f) {
                var_f14_2 = 1.0f;
            }
            if (temp_v1 > 0) {
                var_f14_2 = -var_f14_2;
            }
            M2C_FIELD(temp_s0, s16 *, 0x104) = (s16) (s32) (65536.0f * var_f14_2);
            if (M2C_FIELD(arg0, f32 *, 0x28) == 1.0f) {
                M2C_FIELD(temp_s0, s16 *, 0x102) = 0;
                M2C_FIELD(temp_s0, s16 *, 0x104) = 0;
            } else {
                M2C_FIELD(temp_s0, u8 *, 0x185) = 0U;
                M2C_FIELD(temp_s0, f32 *, 0x188) = 0.0f;
            }
        }
        temp_v1_2 = LOCAL_BSS_1D94;
        if (temp_v1_2 != 0) {
            spC0 = temp_v1_2 - 1;
            do {
                temp_f0_5 = ext_o8_1000(arg0, temp_s0, spAC);
                spAC = temp_f0_5;
                temp_v0_9 = M2C_FIELD(temp_s0, s32 *, 0x41C);
                temp_v1_3 = temp_v0_9 & 0x4000;
                if ((temp_v1_3 == 0) && (M2C_FIELD(temp_s0, f32 *, 0x5C) > 0.0f) && (M2C_FIELD(temp_s0, f32 *, 4) < -temp_f0_5)) {
                    var_v0_6 = 1;
                } else if ((temp_v1_3 == 0) && (M2C_FIELD(temp_s0, f32 *, 0x5C) < 0.0f)) {
                    var_v0_6 = 1;
                } else {
                    var_v0_6 = 0;
                    if (!(temp_v0_9 & 0xC000) && (M2C_FIELD(temp_s0, f32 *, 0x5C) > 0.0f)) {
                        var_v0_6 = 1;
                    }
                }
                if ((var_v0_6 != 0) && (M2C_FIELD(temp_s0, u8 *, 0x185) == 0)) {
                    M2C_FIELD(temp_s0, f32 *, 4) = (f32) (M2C_FIELD(temp_s0, f32 *, 4) + (G_rt_458c4 * M2C_FIELD(temp_s0, f32 *, 0x5C)));
                    temp_f2_2 = M2C_FIELD(sp130, f32 *, 0x1C);
                    if (temp_f2_2 < M2C_FIELD(temp_s0, f32 *, 4)) {
                        M2C_FIELD(temp_s0, f32 *, 4) = temp_f2_2;
                    }
                }
                if (M2C_FIELD(temp_s0, s16 *, 0x102) != 0) {
                    temp_f2_3 = LOCAL_RODATA_120;
                    M2C_FIELD(temp_s0, s32 *, 0x428) = 0;
                    M2C_FIELD(temp_s0, s32 *, 0x42C) = 0;
                    M2C_FIELD(temp_s0, s32 *, 0x41C) = 0;
                    M2C_FIELD(temp_s0, s32 *, 0x420) = 0;
                    M2C_FIELD(temp_s0, f32 *, 4) = (f32) (M2C_FIELD(temp_s0, f32 *, 4) * temp_f2_3);
                    M2C_FIELD(temp_s0, f32 *, 8) = (f32) (M2C_FIELD(temp_s0, f32 *, 8) * temp_f2_3);
                }
                temp_v0_10 = M2C_FIELD(temp_s0, s32 *, 0x41C);
                M2C_FIELD(temp_s0, s16 *, 0x100) = 0;
                if (temp_v0_10 & 0x4000) {
                    if (M2C_FIELD(temp_s0, f32 *, 4) < -5.0f) {
                        ext_o8_49dc((s16 *)1);
                    }
                    if (M2C_FIELD(temp_s0, f32 *, 4) < 0.0f) {
                        M2C_FIELD(temp_s0, f32 *, 4) = (f32) (M2C_FIELD(temp_s0, f32 *, 4) + M2C_FIELD(&sp130[(s32) -M2C_FIELD(temp_s0, f32 *, 4)], f32 *, 0xC8));
                        if ((M2C_FIELD(temp_s0, f32 *, 4) > 0.0f) && (M2C_FIELD(temp_s0, s32 *, 0x42C) >= -0x1E)) {
                            M2C_FIELD(temp_s0, f32 *, 4) = 0.0f;
                        }
                    } else if (M2C_FIELD(temp_s0, s32 *, 0x42C) < -0x1E) {
                        M2C_FIELD(temp_s0, f32 *, 4) = (f32) (M2C_FIELD(temp_s0, f32 *, 4) + M2C_FIELD(&sp130[(s32) M2C_FIELD(temp_s0, f32 *, 4)], f32 *, 0x20));
                        if (M2C_FIELD(temp_s0, f32 *, 4) > 6.0f) {
                            M2C_FIELD(temp_s0, f32 *, 4) = 6.0f;
                        }
                    } else {
                        M2C_FIELD(temp_s0, f32 *, 4) = (f32) (M2C_FIELD(temp_s0, f32 *, 4) - M2C_FIELD(sp130, f32 *, 0xC8));
                        if (M2C_FIELD(temp_s0, f32 *, 4) <= 0.0f) {
                            M2C_FIELD(temp_s0, f32 *, 4) = 0.0f;
                        }
                    }
                } else if (temp_v0_10 & 0x8000) {
                    temp_v0_11 = M2C_FIELD(temp_s0, u8 *, 0x185);
                    temp_f16 = -temp_f0_5;
                    if ((temp_v0_11 == 1) || (temp_v0_11 == 2)) {
                        if (G_offd_31a4 == 0) {
                            var_f14_3 = LOCAL_RODATA_124;
                        } else {
                            var_f14_3 = 0.5f;
                        }
                    } else {
                        temp_f2_4 = M2C_FIELD(temp_s0, f32 *, 4);
                        if (temp_f2_4 > 0.0f) {
                            var_v1_3 = (s32) temp_f2_4;
                            var_f14_4 = temp_f2_4 - (f32) var_v1_3;
                        } else {
                            temp_f12_3 = -temp_f2_4;
                            var_v1_3 = (s32) temp_f12_3;
                            var_f14_4 = temp_f12_3 - (f32) var_v1_3;
                        }
                        temp_v0_12 = &sp130[var_v1_3];
                        temp_f12_4 = M2C_FIELD(temp_v0_12, f32 *, 0x44);
                        var_f14_3 = ((M2C_FIELD(temp_v0_12, f32 *, 0x48) - temp_f12_4) * var_f14_4) + temp_f12_4;
                    }
                    if (M2C_FIELD(temp_s0, f32 *, 4) < temp_f16) {
                        M2C_FIELD(temp_s0, f32 *, 4) = (f32) (M2C_FIELD(temp_s0, f32 *, 4) * LOCAL_RODATA_128);
                        if (temp_f16 < M2C_FIELD(temp_s0, f32 *, 4)) {
                            goto block_160;
                        }
                    } else {
                        M2C_FIELD(temp_s0, f32 *, 4) = (f32) (M2C_FIELD(temp_s0, f32 *, 4) - (var_f14_3 * M2C_FIELD(temp_s0, f32 *, 0x3A0)));
                        if (M2C_FIELD(temp_s0, f32 *, 4) < temp_f16) {
block_160:
                            M2C_FIELD(temp_s0, f32 *, 4) = temp_f16;
                        }
                    }
                    if ((G_o1_83e4 == 1) && (G_offd_31a4 == 0) && (M2C_FIELD(temp_s0, u16 *, 0x1A8) & 1) && (ext_o0_2630c() == (s32)0x21) && (M2C_FIELD(temp_s0, s16 *, 0x37C) == 0x2A) && (LOCAL_RODATA_12C < M2C_FIELD(temp_s0, f32 *, 0x398))) {
                        M2C_FIELD(temp_s0, f32 *, 4) = (f32) (M2C_FIELD(temp_s0, f32 *, 4) - 2.0f);
                    }
                } else {
                    temp_f2_5 = M2C_FIELD(temp_s0, f32 *, 4);
                    if ((LOCAL_RODATA_130 < temp_f2_5) && (temp_f2_5 < LOCAL_RODATA_134)) {
                        M2C_FIELD(temp_s0, f32 *, 4) = 0.0f;
                    } else {
                        M2C_FIELD(temp_s0, f32 *, 4) = (f32) (temp_f2_5 * LOCAL_RODATA_138);
                    }
                }
                if ((LOCAL_RODATA_13C < M2C_FIELD(temp_s0, f32 *, 4)) && (M2C_FIELD(temp_s0, f32 *, 4) < LOCAL_RODATA_140)) {
                    ext_o0_29598(NULL, (f32 *)0x7F);
                }
                temp_v0_13 = M2C_FIELD(temp_s0, s32 *, 0x428);
                if (temp_v0_13 >= 0x42) {
                    var_v1_4 = -0x1F4;
                } else if (temp_v0_13 < -0x41) {
                    var_v1_4 = 0x1F4;
                } else {
                    var_v1_4 = (s32) (temp_v0_13 * -0x1F4) / 65;
                }
                temp_v0_14 = M2C_FIELD(temp_s0, s16 *, 0x108);
                var_f14_5 = -2.0f - M2C_FIELD(temp_s0, f32 *, 4);
                M2C_FIELD(temp_s0, s16 *, 0x108) = (s16) (temp_v0_14 + ((s32) (var_v1_4 - temp_v0_14) >> 1));
                if (var_f14_5 > 0.0f) {
                    var_f14_6 = (ext_o0_2a46c((s16) (s32) (var_f14_5 * LOCAL_RODATA_144)) * 0.25f) + 0.75f;
                } else {
                    if (var_f14_5 < 0.0f) {
                        var_f14_5 = -var_f14_5;
                    }
                    if (var_f14_5 > 2.0f) {
                        var_f14_5 = 2.0f;
                    }
                    var_f14_6 = (ext_o0_2a46c((s16) (s32) (var_f14_5 * 16384.0f)) + 1.0f) * 0.5f;
                }
                if (M2C_FIELD(temp_s0, f32 *, 4) > 0.0f) {
                    var_f14_6 = -var_f14_6;
                }
                M2C_FIELD(temp_s0, s16 *, 0xF0) = (s16) (s32) ((f32) M2C_FIELD(temp_s0, s16 *, 0xF0) + ((f32) M2C_FIELD(temp_s0, s16 *, 0x108) * var_f14_6));
                M2C_FIELD(temp_s0, f32 *, 8) = (f32) (M2C_FIELD(temp_s0, f32 *, 8) * LOCAL_RODATA_148);
                temp_f2_6 = M2C_FIELD(temp_s0, f32 *, 8);
                if ((LOCAL_RODATA_14C < temp_f2_6) && (temp_f2_6 < LOCAL_RODATA_150)) {
                    M2C_FIELD(temp_s0, f32 *, 8) = 0.0f;
                }
                spC0 -= 1;
            } while (spC0 != 0);
        }
        if (M2C_FIELD(temp_s0, s32 *, 0x420) & 0x2000) {
            temp_a0 = M2C_FIELD(temp_s0, s16 **, 0xA8);
            if (temp_a0 != NULL) {
                ext_o0_2d98(temp_a0);
            }
            ext_o0_2b90(4, M2C_FIELD(arg0, u32 *, 0xC), M2C_FIELD(arg0, u32 *, 0x10), M2C_FIELD(arg0, u32 *, 0x14), 4, temp_s0 + 0xA8);
        }
        if ((M2C_FIELD(temp_s0, s32 *, 0x41C) & 0x4000) && (M2C_FIELD(temp_s0, f32 *, 4) < 0.0f)) {
            temp_a0_2 = M2C_FIELD(temp_s0, s16 **, 0xAC);
            if (temp_a0_2 == NULL) {
                ext_o0_2b90(3, M2C_FIELD(arg0, u32 *, 0xC), M2C_FIELD(arg0, u32 *, 0x10), M2C_FIELD(arg0, u32 *, 0x14), 1, temp_s0 + 0xAC);
            } else {
                ext_o0_2d70(temp_a0_2, M2C_FIELD(arg0, u32 *, 0xC), M2C_FIELD(arg0, u32 *, 0x10), M2C_FIELD(arg0, u32 *, 0x14));
            }
        }
        temp_a0_3 = M2C_FIELD(temp_s0, s16 **, 0xAC);
        if (temp_a0_3 != NULL) {
            ext_o0_2d98(temp_a0_3);
        }
        M2C_FIELD(arg0, s16 *, 0) = (s16) (M2C_FIELD(temp_s0, s16 *, 0xF0) + M2C_FIELD(temp_s0, s16 *, 0x104));
        sp7E = M2C_FIELD(temp_s0, s16 *, 0xF0);
        if (M2C_FIELD(temp_s0, s32 *, 0x438) == 1) {
            temp_f0_6 = ext_o0_2a428(LOCAL_RODATA_154, LOCAL_BSS_1D94);
            temp_f2_7 = M2C_FIELD(temp_s0, f32 *, 4);
            if ((temp_f2_7 < -0.5f) || (temp_f2_7 > 0.5f)) {
                M2C_FIELD(temp_s0, f32 *, 4) = (f32) (temp_f2_7 * temp_f0_6);
            } else {
                M2C_FIELD(temp_s0, f32 *, 4) = 0.0f;
            }
            temp_f2_8 = M2C_FIELD(temp_s0, f32 *, 8);
            if ((temp_f2_8 < -0.5f) || (temp_f2_8 > 0.5f)) {
                M2C_FIELD(temp_s0, f32 *, 8) = (f32) (temp_f2_8 * temp_f0_6);
            } else {
                M2C_FIELD(temp_s0, f32 *, 8) = 0.0f;
            }
        }
        if (M2C_FIELD(temp_s0, u8 *, 0x181) != 0) {
            temp_f0_7 = (M2C_FIELD(temp_s0, f32 *, 0x84) * D_4) + (0.5f * M2C_FIELD(temp_s0, f32 *, 0x88) * D_4 * D_4);
            sp90 = M2C_FIELD(temp_s0, f32 *, 0x74) * temp_f0_7;
            sp8C = M2C_FIELD(temp_s0, f32 *, 0x78) * temp_f0_7;
            sp88 = M2C_FIELD(temp_s0, f32 *, 0x7C) * temp_f0_7;
            if (temp_f0_7 < 0.0f) {
                M2C_FIELD(temp_s0, f32 *, 0x84) = 0.0f;
                M2C_FIELD(temp_s0, f32 *, 0x88) = 0.0f;
                M2C_FIELD(temp_s0, u8 *, 0x181) = 0U;
                if (!(M2C_FIELD(temp_s0, s32 *, 0x41C) & 0x8000)) {
                    M2C_FIELD(temp_s0, f32 *, 4) = 0.0f;
                    M2C_FIELD(temp_s0, f32 *, 8) = 0.0f;
                }
            }
            temp_f0_8 = M2C_FIELD(temp_s0, f32 *, 0x84);
            sp80 = 1.0f - (temp_f0_8 / M2C_FIELD(temp_s0, f32 *, 0x80));
            M2C_FIELD(temp_s0, f32 *, 0x84) = (f32) (temp_f0_8 + (M2C_FIELD(temp_s0, f32 *, 0x88) * D_4));
            spA8 = ext_o0_2a470(sp7E) * M2C_FIELD(temp_s0, f32 *, 4) * sp80;
            var_f16 = ext_o0_2a46c(sp7E) * M2C_FIELD(temp_s0, f32 *, 4) * sp80;
        } else {
            sp90 = 0.0f;
            sp8C = 0.0f;
            sp88 = 0.0f;
            spA8 = ext_o0_2a470(sp7E) * M2C_FIELD(temp_s0, f32 *, 4);
            var_f16 = ext_o0_2a46c(sp7E) * M2C_FIELD(temp_s0, f32 *, 4);
        }
        spA4 = var_f16;
        spA8 += M2C_FIELD(temp_s0, f32 *, 8) * ext_o0_2a46c(sp7E);
        temp_cos = ext_o0_2a470(sp7E);
        temp_f14_2 = M2C_FIELD(arg0, f32 *, 0x20);
        temp_f12_5 = (spA8 * D_4) + sp90;
        temp_f10_2 = (var_f16 - (M2C_FIELD(temp_s0, f32 *, 8) * temp_cos)) * D_4;
        sp98 = ((temp_f14_2 * D_4) - (0.5f * G_rt_458c4 * D_4 * D_4)) + sp8C;
        temp_f8_2 = 1.0f / D_4;
        temp_f18 = temp_f10_2 + sp88;
        spA0 = temp_f8_2;
        M2C_FIELD(arg0, f32 *, 0x1C) = (f32) (temp_f12_5 * temp_f8_2);
        M2C_FIELD(arg0, f32 *, 0x20) = (f32) (temp_f14_2 - (G_rt_458c4 * D_4));
        M2C_FIELD(arg0, f32 *, 0xC) += temp_f12_5;
        M2C_FIELD(arg0, f32 *, 0x24) = (f32) (temp_f18 * spA0);
        M2C_FIELD(arg0, f32 *, 0x10) = (f32) (M2C_FIELD(arg0, f32 *, 0x10) + sp98);
        M2C_FIELD(arg0, f32 *, 0x14) += temp_f18;
        if (M2C_FIELD(temp_s0, s32 *, 0x2BC) == 1) {
            var_v0_7 = ext_o0_1e174(arg0, temp_s0, D_4);
        } else {
            var_v0_7 = ext_o0_1d920(arg0, temp_s0, D_4);
        }
        sp58 = var_v0_7;
        if ((ext_o0_7cd8(arg0, 0.0f, 0.0f, 0.0f) != NULL) || (M2C_FIELD(arg0, s16 *, 0x2E) == -1)) {
            if (M2C_FIELD(temp_s0, u8 *, 0x170) == 0) {
                M2C_FIELD(temp_s0, u8 *, 0x170) = 1U;
            }
            M2C_FIELD(arg0, f32 *, 0xC) = M2C_FIELD(temp_s0, f32 *, 0x38);
            M2C_FIELD(arg0, f32 *, 0x10) = (f32) M2C_FIELD(temp_s0, f32 *, 0x3C);
            M2C_FIELD(arg0, f32 *, 0x14) = M2C_FIELD(temp_s0, f32 *, 0x40);
            sp58 = var_v0_7;
            ext_o0_7cd8(arg0, 0.0f, 0.0f, 0.0f);
        }
        if (M2C_FIELD(temp_s0, s16 *, 0x166) != 0) {
            if (G_o1_83e4 == 3) {
                M2C_FIELD(temp_s0, u8 *, 0x171) = 0x78U;
                M2C_FIELD(temp_s0, s16 *, 0x166) = 0;
            } else if (M2C_FIELD(temp_s0, u8 *, 0x170) == 0) {
                M2C_FIELD(temp_s0, u8 *, 0x170) = 1U;
            }
        }
        M2C_FIELD(temp_s0, f32 *, 0x94) = (M2C_FIELD(arg0, f32 *, 0xC) - M2C_FIELD(temp_s0, f32 *, 0x38)) * spA0;
        M2C_FIELD(temp_s0, f32 *, 0x98) = (f32) ((M2C_FIELD(arg0, f32 *, 0x10) - M2C_FIELD(temp_s0, f32 *, 0x3C)) * spA0);
        M2C_FIELD(temp_s0, f32 *, 0x9C) = (M2C_FIELD(arg0, f32 *, 0x14) - M2C_FIELD(temp_s0, f32 *, 0x40)) * spA0;
        if ((M2C_FIELD(temp_s0, s16 *, 0x16A) == 0) && (var_v0_7 != NULL)) {
            temp_f12_6 = M2C_FIELD(temp_s0, f32 *, 0xC);
            M2C_FIELD(temp_s0, f32 *, 0xC) = (f32) (temp_f12_6 + ((3.0f - temp_f12_6) * (1.0f - ext_o0_2a428(LOCAL_RODATA_158, LOCAL_BSS_1D94))));
            temp_f12_7 = M2C_FIELD(temp_s0, f32 *, 0xC);
            temp_f14_3 = -temp_f12_7;
            if (M2C_FIELD(temp_s0, f32 *, 4) < temp_f14_3) {
                M2C_FIELD(temp_s0, f32 *, 4) = temp_f14_3;
            }
            if (temp_f12_7 < M2C_FIELD(temp_s0, f32 *, 4)) {
                M2C_FIELD(temp_s0, f32 *, 4) = temp_f12_7;
            }
            if (M2C_FIELD(temp_s0, f32 *, 8) < temp_f14_3) {
                M2C_FIELD(temp_s0, f32 *, 8) = temp_f14_3;
            }
            if (temp_f12_7 < M2C_FIELD(temp_s0, f32 *, 8)) {
                M2C_FIELD(temp_s0, f32 *, 8) = temp_f12_7;
            }
        } else {
            temp_f12_8 = M2C_FIELD(temp_s0, f32 *, 0xC);
            M2C_FIELD(temp_s0, f32 *, 0xC) = (f32) (temp_f12_8 + ((25.0f - temp_f12_8) * (1.0f - ext_o0_2a428(LOCAL_RODATA_15C, LOCAL_BSS_1D94))));
        }
        ext_o8_49a4(M2C_FIELD(temp_s0, f32 *, 0xC), temp_s0);
        M2C_FIELD(temp_s0, f32 *, 0x70) = ext_o8_34a0(arg0, temp_s0, spAC, LOCAL_DATA_4);
        ext_o8_49b4(temp_s0);
        ext_o0_1cfcc((s16 *) arg0, (f32 *) temp_s0, (s16 *) LOCAL_BSS_1D94);
        var_a1 = &D_D4;
        var_v1_5 = 2;
        do {
            if (var_v1_5 != M2C_FIELD(temp_s0, u8 *, 0x382)) {
                temp_a0_4 = M2C_FIELD(var_a1, M2C_UNK (**)(M2C_UNK, M2C_UNK *), 0);
                if ((temp_a0_4 != NULL) && (M2C_FIELD(var_a1, u16 *, 8) & (1 << M2C_FIELD(temp_s0, u8 *, 0x382)))) {
                    sp60 = var_v1_5;
                    sp50 = var_a1;
                    temp_a0_4(temp_a0_4, var_a1);
                }
            }
            var_v1_5 += 1;
            var_a1 += 0xC;
            temp_a0_5 = M2C_FIELD(temp_s0, u8 *, 0x382);
        } while (var_v1_5 != 6);
        temp_v0_15 = M2C_FIELD(LOCAL_BSS + (temp_a0_5 * 0xC), M2C_UNK (**)(u8, M2C_UNK *), 0xC0);
        if (temp_v0_15 != NULL) {
            temp_v0_15(temp_a0_5, var_a1);
        }
        ext_o8_3278((s16 *) arg0, (f32 *) temp_s0, (s16 *) LOCAL_BSS_1D94);
        ext_o0_1d510((s16 *) arg0, (f32 *) temp_s0, NULL, (f32 *)3, (M2C_UNK *) LOCAL_BSS_1D94);
        ext_o8_2ec0((s16 *) arg0, (f32 *) temp_s0, (s16 *) LOCAL_BSS_1D94);
        ext_o8_3018(arg0, temp_s0, M2C_FIELD(temp_s0, u32 *, 0x70), LOCAL_BSS_1D94);
        ext_o0_3e99c((s16 *) arg0, (f32 *) LOCAL_BSS_1D94);
        if ((M2C_FIELD(temp_s0, u8 *, 0x349) != 0) && (M2C_FIELD(temp_s0, u8 *, 0x16C) == 1)) {
            M2C_FIELD(temp_s0, u8 *, 0x16C) = 0U;
        }
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F000438C_185076C.s")
#endif

/* ---- overlay1InitTimedState ---- */


/* DKR v77/v80 contains only generic object-state initialization patterns. */
typedef struct Overlay1TimedState {
    u8 pad0[0x193];
    u8 field193;
    u8 pad194[0x1EE];
    u8 enabled;
    u8 pad383[0xD];
    s16 timer;
    u8 pad392[0x52];
    s32 value3E4;
} Overlay1TimedState;

typedef struct Overlay1TimedObject {
    u8 pad0[0x64];
    Overlay1TimedState *state;
} Overlay1TimedObject;

void overlay1InitTimedState(Overlay1TimedObject *object, s32 timer) {
    Overlay1TimedState *state = object->state;

    state->enabled = 1;
    state->timer = timer;
    state->field193 = 0;
    state->value3E4 = 0;
}

/* ---- overlay1ConsumeTimer ---- */


typedef struct Overlay1TimerState {
    u8 pad000[0x382];
    u8 active;
    u8 pad383[0xD];
    u16 timer;
} Overlay1TimerState;

/* Fresh pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern s32 gOverlay1TimerStep;

void overlay1ConsumeTimer(void) {
    if (((Overlay1TimerState *)gOverlay1TimerState)->timer <=
        gOverlay1TimerStep) {
        ((Overlay1TimerState *)gOverlay1TimerState)->active = 0;
    } else {
        ((Overlay1TimerState *)gOverlay1TimerState)->timer -=
            gOverlay1TimerStep;
    }
}

/* ---- overlay1StartTimerCallbacks ---- */


typedef void (*Overlay1Callback)(void);

typedef struct Overlay1CallbackState {
    u8 pad000[0x382];
    u8 mode;
} Overlay1CallbackState;

typedef struct Overlay1CallbackObject {
    u8 pad00[0x64];
    Overlay1CallbackState *state;
} Overlay1CallbackObject;

typedef struct Overlay1CallbackEntry {
    Overlay1Callback callback;
    s32 pad4;
    u16 modeMask;
    u16 padA;
} Overlay1CallbackEntry;

/* Fresh pinned DKR v77/v80 and JFG scans found no Overlay 1 donor. */
extern s32 overlay1IsObjectActive(void *object);
extern s32 gOverlay1TimerStep;
extern f32 gOverlay1CallbackStepFloat;
extern Overlay1CallbackEntry gOverlay1CallbackDescriptor[];
extern Overlay1CallbackEntry gOverlay1ModeCallbacks[];

#ifdef NON_MATCHING
void overlay1StartTimerCallbacks(Overlay1CallbackObject *object, s32 amount) {
    Overlay1CallbackEntry *entry;
    Overlay1Callback callback;
    s32 index;
    u8 mode;

    if (overlay1IsObjectActive(object) != 0) {
        gOverlay1TimerStep = amount;
        gOverlay1CallbackStepFloat = amount;
        entry = gOverlay1CallbackDescriptor;
        for (index = 5; index != 6; index++, entry++) {
            mode = gOverlay1TimerState->mode;
            if (index != mode) {
                callback = entry->callback;
                if (callback != NULL) {
                    if ((entry->modeMask & (1 << mode)) != 0) {
                        callback();
                    }
                }
            }
        }
        mode = gOverlay1TimerState->mode;
        callback = gOverlay1ModeCallbacks[mode].callback;
        if (callback != NULL) {
            callback();
        }
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F0005BF4_1851FD4.s")
#endif

/* ---- overlay1FindDirectionalObject ---- */


#ifndef DOT_CONDITION
#define DOT_CONDITION (threshold < dot)
#endif

typedef struct Overlay1ObjectState { s8 tableIndex; } Overlay1ObjectState;
typedef struct Overlay1DirectionalObject {
    s16 angle; u8 pad02[0xA]; f32 x; u8 pad10[4]; f32 z;
    u8 pad18[0x4C]; Overlay1ObjectState *state;
} Overlay1DirectionalObject;
typedef struct Overlay1ValueEntry { f32 value; u8 pad04[8]; } Overlay1ValueEntry;
typedef struct Overlay1ValueRow { Overlay1ValueEntry entries[6]; } Overlay1ValueRow;

extern Overlay1DirectionalObject **overlay1GetObjectList(s32 *count);
extern f32 sqrtf(f32 value);
extern f32 overlay1TrigX(s32 angle);
extern f32 overlay1TrigY(s32 angle);
extern f32 D_160;
extern Overlay1ValueRow D_1BA8[];

Overlay1DirectionalObject *overlay1FindDirectionalObject(
    Overlay1DirectionalObject *object, void *unused1, void *unused2,
    f32 threshold, f32 maxValue) {
    s32 count;
    Overlay1DirectionalObject **objects;
    Overlay1DirectionalObject *other;
    Overlay1DirectionalObject *best;
    Overlay1ObjectState *otherState;
    Overlay1ValueEntry *entry;
    s32 remaining;
    f32 dx, dz, distance, directionX, directionY, dot, value, bestValue;

    objects = overlay1GetObjectList(&count);
    best = 0;
    bestValue = D_160;
    if (overlay1IsObjectActive(object)) {
        goto active;
    }
    return 0;
active:
    while (remaining = count--) {
        other = objects[count];
        if (other == object) {
        } else {
            dx = other->x - object->x;
            otherState = other->state;
            dz = other->z - object->z;
            distance = sqrtf((dx * dx) + (dz * dz));
            if (distance > 0.0f) {
                dx /= distance;
                dz /= distance;
            }
            directionX = -overlay1TrigX(object->angle);
            directionY = -overlay1TrigY(object->angle);
            dot = (directionX * dx) + (directionY * dz);
            if (DOT_CONDITION) {
                entry = &D_1BA8[*(s8 *)D_1DA0].entries[otherState->tableIndex];
                value = entry->value;
                if ((value <= maxValue) && (value < bestValue)) {
                    bestValue = value;
                    best = other;
                }
            }
        }
    }
    return best;
}

/* ---- overlay1ReturnZero ---- */


/* DKR v77/v80 and JFG have no overlay-1 donor; this is a generic leaf. */
s32 overlay1ReturnZero(void) {
    return 0;
}

/* ---- overlay1DispatchMode ---- */


typedef struct Overlay1ModeState {
    u8 pad00;
    s8 index;
    u8 pad02[0x198];
    u8 mode;
    u8 timer;
    u8 pad19C[4];
    s32 task;
    u8 pad1A4[0x1DA];
    u8 group;
    u8 pad37F[0x1D];
    f32 angle;
    u8 pad3A0[8];
    u8 status[1];
} Overlay1ModeState;

typedef struct Overlay1ModeObject {
    u8 pad00[0x64];
    Overlay1ModeState *state;
} Overlay1ModeObject;

#ifndef WORLD_GLOBAL_DECL
#define WORLD_GLOBAL_DECL extern Overlay1ModeState *D_1DA0_array[];
#define WORLD D_1DA0_array[0]
#endif
#ifndef CASE_END
#define CASE_END return 0
#endif
WORLD_GLOBAL_DECL
extern u8 D_6C[];
extern void overlay1ModeAction2(void *object, s32 arg);
extern s32 overlay1ModeRandom(s32 minimum, s32 maximum);
extern Overlay1ModeObject *overlay1ModeFind(f32 angle);
extern void overlay1ModeAction3(void *object);
extern void overlay1ModeAction4(void *object);
extern void overlay1ModeAction5(void *object);
extern void overlay1ModeAction6(void *object);
extern void overlay1ModeAction7(void *object);
extern void overlay1ModeAction8(void *object);
extern void overlay1ModeAction9(void *object);
extern f32 overlay1WrapOffset(f32 first, f32 second);

/* Plateau (2026-08-24): every -O2 -mips2 flag variant is retail-sized and
 * reaches the same first mismatch at +0x6C; 28 of 199 words remain, almost
 * entirely allocator choices across switch cases.  Nine declaration-order,
 * register-storage, and world-symbol spelling attempts did not improve the
 * best schedule; the unavailable local permuter is the next useful search. */
#ifdef NON_MATCHING
s32 overlay1DispatchMode(void) {
    Overlay1ModeState *world;
    Overlay1ModeObject *object;
    Overlay1ModeState *state;
    f32 difference;

    world = WORLD;
    switch (world->mode) {
        case 2:
            overlay1ModeAction2(D_1D9C, 1);
            WORLD->timer--;
            world = WORLD;
            if (world->timer == 0) {
                world->mode = 0xFF;
                WORLD->task = 0;
            }
            CASE_END;
        case 3:
            if (D_6C[WORLD->index] < overlay1ModeRandom(1, 100)) {
                object = overlay1ModeFind(WORLD->angle);
                if (object != 0) {
                    state = object->state;
                    if (WORLD->status[state->index] >= 3) {
                        overlay1ModeAction3(D_1D9C);
                    }
                }
            }
            CASE_END;
        case 4:
            if (D_6C[WORLD->index] < overlay1ModeRandom(1, 100)) {
                object = overlay1ModeFind(WORLD->angle);
                if (object != 0) {
                    state = object->state;
                    if (WORLD->status[state->index] >= 3) {
                        difference = overlay1WrapOffset(WORLD->angle,
                                                        object->state->angle);
                        if ((0.5f <= difference) && (difference <= 4.0f)) {
                            overlay1ModeAction4(D_1D9C);
                        }
                    }
                }
            }
            CASE_END;
        case 5:
            if (D_6C[WORLD->index] < overlay1ModeRandom(1, 100)) {
                object = overlay1ModeFind(WORLD->angle);
                if (object != 0) {
                    state = object->state;
                    if (WORLD->status[state->index] >= 3) {
                        overlay1ModeAction5(D_1D9C);
                    }
                }
            }
            CASE_END;
        case 6:
            object = overlay1ModeFind(WORLD->angle);
            if (object != 0) {
                state = object->state;
                difference = overlay1WrapOffset(WORLD->angle, state->angle);
                if ((difference <= 3.0f) &&
                    (WORLD->status[state->index] >= 3) &&
                    (state->group == WORLD->group)) {
                    overlay1ModeAction6(D_1D9C);
                }
            }
            CASE_END;
        case 7:
            overlay1ModeAction7(D_1D9C);
            CASE_END;
        case 8:
            overlay1ModeAction8(D_1D9C);
            CASE_END;
        case 9:
            overlay1ModeAction9(D_1D9C);
            CASE_END;
    }
    return 0;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F0005ED4_18522B4.s")
#endif

/* ---- overlay1HandleCachedMode ---- */

typedef struct W { u8 p0[0xD4]; void *object; u8 pD8[0xC3]; u8 enabled; s32 state; } W;
extern s32 D_83E4;
extern s32 overlay27CanUse(void *);
extern s32 overlay3RunCachedModeAction(void *, W *);
extern s32 overlay1DispatchMode(void);
#ifdef NON_MATCHING
s32 overlay1HandleCachedMode(void) {
    W *world = D_1DA0[0];
    s32 result = 0;
    if (world->enabled == 0) goto clear;
    if (overlay27CanUse(world->object) != 0) goto clear;
    if (D_83E4 == 3) result = overlay3RunCachedModeAction(D_1D9C[0], D_1DA0[0]);
    else result = overlay1DispatchMode();
    return result;
clear:
    D_1DA0[0]->state = 0;
    return result;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F00061F0_18525D0.s")
#endif

/* ---- overlay1ChooseModeObject ---- */


typedef struct O1SelectState { s8 tableIndex; } O1SelectState;
typedef struct O1SelectObject { u8 pad00[0x64]; O1SelectState *state; } O1SelectObject;
typedef struct O1SelectEntry { f32 value; u8 pad04[8]; } O1SelectEntry;
typedef struct O1SelectRow { O1SelectEntry entries[6]; } O1SelectRow;
typedef struct O1Selection { O1SelectObject *object; s16 value; } O1Selection;
typedef struct O1SelectWorld {
    s8 row;
    u8 pad01[0x381];
    u8 mode;
    u8 pad383[0xD];
    O1Selection selection;
    u8 pad398[0x4C];
    O1SelectObject *selected;
} O1SelectWorld;

extern O1SelectObject **overlay1GetSelectObjects(s32 *count);
extern s32 overlay1SelectRandom(s32 minimum, s32 maximum);
extern s16 overlay1SelectValue(s32 minimum, s32 maximum);

#ifdef NON_MATCHING
s32 overlay1ChooseModeObject(void) {
    O1SelectObject **objects;
    O1SelectObject *object;
    s32 count;
    s32 remaining;
    s32 choiceCount;
    O1SelectObject *choices[5];
    O1Selection *selection;

    objects = overlay1GetSelectObjects(&count);
    choiceCount = 0;
    remaining = count--;
    while (remaining != 0) {
        object = objects[count];
        {
            O1SelectState *state = object->state;
            if (object != D_1D9C) {
                if (D_1BA8[D_1DA0->row].entries[state->tableIndex].value < 600.0f) {
                    choices[choiceCount++] = object;
                }
            }
        }
        remaining = count--;
    }
    if (choiceCount != 0) {
        count = overlay1SelectRandom(1, choiceCount) - 1;
        object = choices[count];
        selection = &D_1DA0->selection;
        selection->object = object;
        selection->value = overlay1SelectValue(0x5A, 0x84);
        D_1DA0->mode = 5;
        D_1DA0->selected = object;
        return 1;
    }
    return 0;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F0006270_1852650.s")
#endif

/* ---- overlay1UpdateCountdown ---- */


typedef struct Overlay1Countdown {
    u8 pad0[4];
    u16 value;
} Overlay1Countdown;

typedef struct Overlay1CountdownObject {
    u8 pad0[0x390];
    Overlay1Countdown countdown;
} Overlay1CountdownObject;

extern Overlay1CountdownObject *gOverlay1CountdownObject;
extern s32 gOverlay1CountdownAmount;
extern void *gOverlay1CountdownResource;
extern void overlay1CountdownReloc(void *, s32);

/* DKR v77/v80 and JFG contain no exact donor for this countdown update. */
void overlay1UpdateCountdown(void) {
    u8 *object;
    s32 amount;
    register s32 mode;
    u16 countdown;

    object = (u8 *)gOverlay1CountdownObject;
    amount = gOverlay1CountdownAmount;
    countdown = *(u16 *)(object + 0x394); mode = 0x78; object += 0x390;
    if ((((u32)object & mode) != 0) && (object == 0)) {
    }
    if (countdown <= amount) {
        overlay1CountdownReloc(gOverlay1CountdownResource, mode);
    } else {
        *(u16 *)(object + 4) = countdown - amount;
    }
}

/* ---- overlay1ReadSelection ---- */


typedef struct Overlay1Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay1Vector;

typedef struct Overlay1Descriptor {
    u8 pad00[0x2D];
    u8 count;
} Overlay1Descriptor;

typedef struct Overlay1Selection {
    Overlay1Descriptor *descriptor;
    u8 pad04[4];
    s16 useObjectPosition;
    u8 pad0A[0x36];
    Overlay1Vector *vectors;
} Overlay1Selection;

typedef struct Overlay1Object {
    u8 pad00[0x0C];
    Overlay1Vector position;
    u8 pad18[0x22];
    s8 selectedIndex;
    u8 pad3B[0x0D];
    u8 *fallback;
    u8 pad4C[0x1C];
    Overlay1Selection **selections;
} Overlay1Object;

void overlay1ReadSelection(Overlay1Object *object, s32 index, f32 *outX,
                           f32 *outY, f32 *outZ) {
    Overlay1Selection *selection;
    Overlay1Descriptor *descriptor;
    Overlay1Vector *vectors;
    s32 offset;

    selection = object->selections[object->selectedIndex];
    if (selection != 0) {
        descriptor = selection->descriptor;
        if (selection->useObjectPosition != 0) {
            *outX = object->position.x;
            *outY = object->position.y;
            *outZ = object->position.z;
            return;
        }
        if (descriptor->count >= index) {
            vectors = selection->vectors;
            offset = index * sizeof(Overlay1Vector);
            if (vectors == 0) {
                return;
            }
            *outX = *(f32 *)((u8 *)vectors + offset + 0);
            *outY = *(f32 *)((u8 *)selection->vectors + offset + 4);
            *outZ = *(f32 *)((u8 *)selection->vectors + offset + 8);
            return;
        }
    }

    *outX = *(f32 *)(object->fallback + 0x24);
    *outY = *(f32 *)(object->fallback + 0x28);
    *outZ = *(f32 *)(object->fallback + 0x2C);
}

/* ---- overlay1SolveAngleCandidates ---- */


extern f32 overlay1SqrtReloc(f32 value);
extern s32 overlay1AngleReloc(f32 y, f32 x);

/* Plateau (2026-08-25): -O2 -mips2 -Wab,-r4300_mul remains 0x8 short; first mismatch +0xA8.
 * Declaration order now gives the target array slot and the tail CFG is closer, but the loop
 * preheader still lacks two instructions; loop, return, temporary, and qualifier spellings failed. */
#ifdef NON_MATCHING
s16 overlay1SolveAngleCandidates(
    f32 x0, f32 y0, f32 x1, f32 y1,
    f32 y2, f32 x2, f32 radius, f32 slope, s32 chooseHigh) {
    f32 dx;
    f32 dy;
    f32 distance;
    f32 sum;
    f32 discriminant;
    f32 discriminantRoot;
    f32 denominator;
    f32 root;
    f32 angleX;
    s32 solutionCount;
    s16 solutions[2];
    s32 sign;

    solutionCount = 0;
    dx = x0 - y1;
    dy = x1 - x2;
    distance = overlay1SqrtReloc((dx * dx) + (dy * dy));
    dy = y2 - y0;
    sum = (dy * slope) + (radius * radius);
    discriminant = (sum * sum) -
        ((slope * slope) * ((distance * distance) + (dy * dy)));

    if (discriminant >= 0.0f) {
        discriminantRoot = overlay1SqrtReloc(discriminant);
        denominator = (((dy * dy) / (distance * distance)) + 1.0f) * 2.0f;

        sign = solutionCount + 2;
        while (sign--) {
            if (sign != 0) {
                root = discriminantRoot;
            } else {
                root = -discriminantRoot;
            }
            root = (root + sum) / denominator;
            if (root >= 0.0f) {
                angleX = overlay1SqrtReloc(root);
                if (distance < 0.0f) {
                    angleX = -angleX;
                }
                solutions[solutionCount] = overlay1AngleReloc(
                    ((dy / distance) * angleX) -
                    ((slope * distance) / (angleX + angleX)), angleX);
                solutionCount++;
            }
        }
    }

    if (solutionCount != 1) {
        if (solutionCount == 2) {
            if (solutions[0] < solutions[1]) {
                return chooseHigh ? solutions[1] : solutions[0];
            }
            return chooseHigh ? solutions[0] : solutions[1];
        }
        return 0x2000;
    }
    return solutions[0];
}

s32 overlay1LoopControlCarrier(s32 value) {
    if (value == 0) {
        return 2;
    }
    return value;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F00064F8_18528D8.s")
#endif

/* ---- overlay1UpdateModeSound ---- */


typedef struct Overlay1SoundModeObject {
    u8 pad0[0x193];
    u8 mode;
    u8 pad194[0x14];
    u16 flags;
} Overlay1SoundModeObject;

extern Overlay1SoundModeObject *gOverlay1ModeObject;
extern void *gOverlay1ModeResource;
extern void overlay1ModeSoundReloc(void *, s32);

/* DKR v77/v80 and JFG contain no exact donor for this mode-gated wrapper. */
void overlay1UpdateModeSound(void) {
    Overlay1SoundModeObject *object;

    object = gOverlay1ModeObject;
    if (object->mode == 13) {
        if (object->flags & 2) {
            overlay1ModeSoundReloc(gOverlay1ModeResource, 0x78);
        }
    } else {
        overlay1ModeSoundReloc(gOverlay1ModeResource, 0x78);
    }
}

/* ---- overlay1CopyBytes ---- */


/* DKR v77/v80 contains only generic byte-copy initialization patterns. */
typedef struct Overlay1ByteState {
    u8 values[6];
} Overlay1ByteState;

typedef struct Overlay1ByteObject {
    u8 pad0[0x64];
    Overlay1ByteState *state;
} Overlay1ByteObject;

typedef struct Overlay1ByteInit {
    u8 pad0[0xA];
    u8 values[6];
} Overlay1ByteInit;

void overlay1CopyBytes(Overlay1ByteObject *object, Overlay1ByteInit *init) {
    Overlay1ByteState *state = object->state;

    state->values[0] = init->values[0];
    state->values[1] = init->values[1];
    state->values[2] = init->values[2];
    state->values[3] = init->values[3];
    state->values[4] = init->values[4];
    state->values[5] = init->values[5];
}

/* ---- overlay1UpdateRangeFlags ---- */


typedef struct Overlay1RangeConfig {
    u8 angleHigh;
    u8 horizontalScale;
    u8 verticalScale;
    u8 mode;
    u8 soundId;
} Overlay1RangeConfig;

typedef struct Overlay1RangeState {
    u8 pad000[0x1A8];
    u16 flags;
} Overlay1RangeState;

typedef struct Overlay1HeightData {
    u8 pad000[0x5C];
    f32 height;
} Overlay1HeightData;

typedef struct Overlay1RangeObject {
    u8 pad000[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad018[0x30];
    Overlay1HeightData *heightData;
    u8 pad04C[0x18];
    void *state;
} Overlay1RangeObject;

extern Overlay1RangeObject **overlay1GetObjectListReloc(s32 *count);
extern s32 overlay1GetAngleValueReloc(f32 dz, f32 dx);
extern void overlay1ActivateObjectReloc(Overlay1RangeObject *object);
extern void overlay1PlaySoundReloc(u8 soundId);

#ifdef NON_MATCHING
void overlay1UpdateRangeFlags(Overlay1RangeObject *object, void *unused) {
    Overlay1RangeConfig *config;
    register s32 clearMask;
    Overlay1RangeObject **objects;
    s32 count;

    config = object->state;
    clearMask = ~8;
    objects = overlay1GetObjectListReloc(&count);
    if (count--) {
        do {
            Overlay1RangeObject *other;
            Overlay1RangeState *otherState;
            f32 dx;
            f32 dz;
            u32 horizontalRange;
            s16 angle;

            other = objects[count];
            otherState = other->state;
            dx = other->x - object->x;
            dz = other->z - object->z;
            horizontalRange = (u32)config->horizontalScale * 10U;
            if ((dx * dx + dz * dz) <
                (f32)(s32)(horizontalRange * horizontalRange)) {
                angle = (s16)(((u32)config->angleHigh << 8) +
                              overlay1GetAngleValueReloc(dz, dx));
                if ((angle < -0x4000) || (angle >= 0x4001)) {
                    f32 otherY;
                    f32 objectY;

                    otherY = other->y;
                    objectY = object->y;
                    if ((objectY <= otherY + other->heightData->height) &&
                        (otherY <= objectY +
                         (f32)(s32)((u32)config->verticalScale * 10U))) {
                        switch (config->mode) {
                            case 0: {
                                u16 flags;
                                flags = otherState->flags;
                                if (!(flags & 8)) {
                                    otherState->flags = flags | 8;
                                }
                                break;
                            }
                            case 1: {
                                u16 flags;
                                flags = otherState->flags;
                                if (flags & 8) {
                                    otherState->flags = flags & clearMask;
                                    overlay1ActivateObjectReloc(other);
                                    overlay1PlaySoundReloc(config->soundId);
                                }
                                break;
                            }
                        }
                    }
                }
            }
        } while (count--);
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F00067C0_1852BA0.s")
#endif

/* ---- overlay1InitMotion ---- */


/* No corresponding DKR/JFG source or object match was found. */
typedef struct Overlay1MotionState {
    f32 magnitude;
    s32 mode;
    u16 first;
    u16 second;
} Overlay1MotionState;

typedef struct Overlay1MotionObject {
    u8 pad0[8];
    f32 scaledMagnitude;
    u8 padC[0x58];
    Overlay1MotionState *state;
} Overlay1MotionObject;

typedef struct Overlay1MotionInit {
    u8 pad0[0xA];
    u8 magnitude;
    u8 mode;
    u16 first;
    u16 second;
} Overlay1MotionInit;

extern f32 gOverlay1MotionScale;

void overlay1InitMotion(Overlay1MotionObject *object, Overlay1MotionInit *init) {
    Overlay1MotionState *state;

    state = object->state;
    object->scaledMagnitude = (f32)init->magnitude * gOverlay1MotionScale;
    state->magnitude = (f32)init->magnitude;
    state->mode = init->mode;
    state->first = init->first;
    state->second = init->second;
}

/* ---- overlay1ConsumeNearbyPending ---- */


typedef struct Overlay1NearbyState {
    f32 radius;
    s32 mode;
    u16 kind;
} Overlay1NearbyState;

typedef struct Overlay1OtherState {
    s8 kind;
    u8 pad01[0x191];
    u8 pending;
    u8 pad193[0x221];
    s16 count;
} Overlay1OtherState;

typedef struct Overlay1NearbyObject {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    void *state;
} Overlay1NearbyObject;

#ifdef NON_MATCHING
void overlay1ConsumeNearbyPending(void *objectArg, void *listArg) {
    Overlay1NearbyState *state;
    f32 radiusSquared;
    s32 count;
    register Overlay1OtherState *otherState;
    register Overlay1NearbyObject *other;

    state = ((Overlay1NearbyObject *)objectArg)->state;
    radiusSquared = state->radius * 4.0f;
    radiusSquared *= state->radius * 4.0f;
    {
        Overlay1NearbyObject *object = objectArg;
        listArg = overlay1GetObjectListReloc(&count);
        if (count--) {
            do {
                other = ((Overlay1NearbyObject **)listArg)[count];
                otherState = other->state;
                if (state->kind == otherState->kind) {
                    f32 dx = other->x - object->x;
                    f32 dy = other->y - object->y;
                    f32 dz = other->z - object->z;
                    if (((dx * dx) + (dy * dy) + (dz * dz) < radiusSquared) &&
                        (state->mode == 2)) {
                        u8 pending = otherState->pending;
                        if (pending) {
                            otherState->pending = 0;
                            otherState->count += pending;
                        }
                    }
                }
            } while (count--);
        }
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F0006A14_1852DF4.s")
#endif

/* ---- overlay1InitRange ---- */


/* Compact range initializer; exact DKR and JFG scans are negative. */
typedef struct Overlay1InitRangeState { s16 a, b; s32 value; u8 c, d; } Overlay1InitRangeState;
typedef struct Overlay1InitRangeObject { u8 pad0[0x64]; Overlay1InitRangeState *state; } Overlay1InitRangeObject;
typedef struct Overlay1RangeInit { u8 pad0[0xA]; u8 a, b, value, c, d; } Overlay1RangeInit;
void overlay1InitRange(Overlay1InitRangeObject *object, Overlay1RangeInit *init) {
    Overlay1InitRangeState *state = object->state;
    state->a = init->a * 10; state->b = init->b * 10;
    state->value = init->value; state->c = init->c; state->d = init->d;
}

/* ---- overlay1SearchNearby ---- */


typedef struct Overlay1SearchState {
    u16 xRange;
    u16 zRange;
    u8 pad04[2];
    u8 flags;
    u8 pad07;
    u8 lookupKey;
    u8 pad09[2];
    u8 active;
    u8 pad0C[0x3A8];
    s16 counter;
} Overlay1SearchState;

typedef struct Overlay1SearchObject {
    u8 pad00[0xC];
    f32 x;
    u8 pad10[4];
    f32 z;
    u8 pad18[0x2C];
    s16 type;
    u8 pad46[0x1E];
    Overlay1SearchState *state;
} Overlay1SearchObject;

extern Overlay1SearchObject **func_8000572C(s32 *first, s32 *limit);
extern Overlay1SearchObject *func_80005820(u8 key);
extern void overlay4RemoveObject(Overlay1SearchObject *object);

void overlay1SearchNearby(Overlay1SearchObject *object, void *unused) {
    Overlay1SearchState *range;
    s32 first;
    s32 limit;
    s32 index;
    Overlay1SearchObject **objects;
    Overlay1SearchObject *candidate;
    Overlay1SearchState *state;
    Overlay1SearchObject *linked;
    f32 delta;
    f32 threshold;

    (void)unused;
    range = object->state;
    objects = func_8000572C(&first, &limit);
    index = first;
    if (index < limit) {
        do {
            candidate = objects[index];
            if (candidate->type == 0x21) {
                delta = candidate->x - object->x;
                state = candidate->state;
                threshold = range->xRange;
                if (delta < 0.0f) {
                    delta = -delta;
                }
                if (delta <= threshold) {
                    delta = candidate->z - object->z;
                    threshold = range->zRange;
                    if (delta < 0.0f) {
                        delta = -delta;
                    }
                    if (delta <= threshold) {
                        linked = func_80005820(range->lookupKey);
                        if (linked != 0) {
                            linked->state->counter++;
                        }
                        state->flags |= 4;
                        state->active = 1;
                        overlay4RemoveObject(candidate);
                        return;
                    }
                }
            }
            index++;
        } while (index != limit);
    }
}

/* ---- overlay1SelectMaskedMode ---- */


typedef struct Overlay1MaskedState {
    u8 pad0[0x382];
    u8 bitIndex;
    u8 pad383[0xD];
    u8 timer;
} Overlay1MaskedState;

typedef struct Overlay1MaskedObject {
    u8 pad0[0x64];
    Overlay1MaskedState *state;
} Overlay1MaskedObject;

extern u8 gOverlay1ModeMasks[];
extern void overlay1SelectModeReloc(void *, s32);

/* DKR v77/v80 and JFG contain no exact donor for this table-mask selector. */
s32 overlay1SelectMaskedMode(Overlay1MaskedObject *object, s32 index) {
    Overlay1MaskedState *state;

    state = object->state;
    if (*(u16 *)(gOverlay1ModeMasks + index * 12 + 0xC4) &
        (1 << state->bitIndex)) {
        state->bitIndex = index;
        overlay1SelectModeReloc(&state->timer, 8);
        return 1;
    }
    return 0;
}

/* ---- overlay1UpdateAimedTransient ---- */


typedef struct Overlay1TransientState {
    void *owner;
    s16 mode;
    u8 type;
    u8 active;
    u8 selector;
    s8 linkedIndex;
    u8 pad0A;
} Overlay1TransientState;

typedef struct Overlay1MotionSourceExtra {
    u8 pad00[0x5C];
    f32 height;
} Overlay1MotionSourceExtra;

typedef struct Overlay1MotionSourceState {
    u8 pad00;
    s8 index;
} Overlay1MotionSourceState;

typedef struct Overlay1MotionSource {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    u8 pad28[0x20];
    Overlay1MotionSourceExtra *extra;
    u8 pad4C[0x18];
    Overlay1MotionSourceState *state;
} Overlay1MotionSource;

typedef struct Overlay1VelocityExtra {
    u8 pad00[0xE0];
    f32 *value;
} Overlay1VelocityExtra;

typedef struct Overlay1TransientObject {
    u8 pad00[8];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    u8 pad28[0x18];
    Overlay1VelocityExtra *extra;
    u8 pad44[0x20];
    Overlay1TransientState *state;
    u8 pad68[0x10];
    s16 *flags;
} Overlay1TransientObject;

typedef struct Overlay1TransientOwner {
    s16 angle;
    u8 pad02[0x26];
    f32 distance;
} Overlay1TransientOwner;

typedef struct Overlay1TransientWorld {
    u8 pad00[0x193];
    u8 mode;
    u8 pad194[0x14];
    u16 flags;
    u8 pad1AA[0x1E6];
    Overlay1MotionSource *source;
    Overlay1TransientObject *object;
    u8 pad398[0x84];
    u32 status;
} Overlay1TransientWorld;

extern f32 D_4;
extern f32 D_188;
extern f32 D_18C;
extern f32 D_190;
extern f32 D_194;

extern Overlay1TransientObject *func_overlay_036_F0000694_1883B4C(
    Overlay1TransientOwner *owner, Overlay1TransientWorld *world);
extern s16 func_overlay_001_F00064F8_18528D8(
    f32, f32, f32, f32, f32, f32, f32, f32, s32);
extern s32 func_8002A910(f32 y, f32 x);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern f32 sqrtf(f32 value);

/* Plateau (2026-08-24): -O2 -g3 -mips2 -Wab,-r4300_mul reaches the
 * retail size, but 56 of 249 words differ from +0x0.  The remaining shape
 * has an 8-byte larger non-save frame and 17 alignment gaps; world-pointer
 * volatility, explicit register storage, and saved-state lifetime variants
 * did not improve it.  It needs a frame/early-load representation change. */
#ifdef NON_MATCHING
void overlay1UpdateAimedTransient(void) {
    Overlay1TransientWorld *world;
    Overlay1TransientWorld **worldRef;
    Overlay1TransientOwner *owner;
    Overlay1TransientObject *object;
    Overlay1TransientState *state;
    Overlay1TransientState *savedState;
    Overlay1MotionSource *source;
    f32 factor;
    f32 predictedX;
    f32 predictedY;
    f32 predictedZ;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 distance;
    f32 trig;
    s32 iteration;
    s16 sourceAngle;
    s16 objectAngle;

    worldRef = &D_1DA0;
    world = *worldRef;
    object = world->object;
    source = world->source;
    if (object == 0) {
        object = func_overlay_036_F0000694_1883B4C(D_1D9C, world);
        if (object != 0) {
            state = object->state;
            state->owner = D_1D9C;
            state->type = 3;
            state->active = 1;
            state->selector = 9;
            state->pad0A = 0;
            object->scale = D_188;
            overlay1ReadSelection(D_1D9C, 9, &object->x, &object->y,
                                  &object->z);
            (*worldRef)->object = object;
            savedState = state;
        }
        world = D_1DA0;
        state = savedState;
    } else {
        state = object->state;
    }

    if ((world->mode == 0xD) && state->active &&
        (D_18C <= D_1D9C->distance)) {
        state->active = 0;
        state->mode = 0xC;
        if (source != 0) {
            state->linkedIndex = source->state->index;
            factor = 0.0f;
            iteration = 3;
            do {
                predictedX = source->x + (factor * source->velocityX);
                predictedY = source->y + (factor * source->velocityY) +
                    (source->extra->height * 0.5f);
                predictedZ = source->z + (factor * source->velocityZ);
                deltaX = predictedX - object->x;
                deltaY = predictedY - object->y;
                deltaZ = predictedZ - object->z;
                distance = sqrtf((deltaX * deltaX) + (deltaY * deltaY) +
                                 (deltaZ * deltaZ));
                factor = 30.0f / distance;
                if (factor > 0.0f) {
                    deltaX *= factor;
                    deltaZ *= factor;
                }
                factor = distance / (30.0f * D_4);
            } while (iteration--);

            sourceAngle = func_8002A910(deltaX, deltaZ);
            objectAngle = func_overlay_001_F00064F8_18528D8(
                object->x, object->y, object->z,
                predictedX, predictedY, predictedZ,
                30.0f, -*object->extra->value, 0);
            trig = func_8002A8BC(objectAngle);
            object->velocityX = func_8002A8C0(sourceAngle) * trig * 30.0f;
            object->velocityY = func_8002A8C0(objectAngle) * 30.0f;
            trig = func_8002A8BC(objectAngle);
            object->velocityZ = func_8002A8BC(sourceAngle) * trig * 30.0f;
        } else {
            state->linkedIndex = -1;
            trig = D_190;
            object->velocityX = func_8002A8C0(D_1D9C->angle) * trig * -30.0f;
            object->velocityY = D_194;
            object->velocityZ = func_8002A8BC(D_1D9C->angle) * trig * -30.0f;
        }
        *object->flags &= ~2;
        world = D_1DA0;
    }

    if (world->flags & 2) {
        if (world->mode == 0xD) {
            overlay1InitTimedState(D_1D9C, 0x78);
            world = D_1DA0;
        }
        if (world->mode == 0xD) {
            world->mode = 0xD;
            world = D_1DA0;
        }
    }
    if (!(world->status & 0x2000) && world->mode == 0xD) {
        world->mode = 0xD;
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F0006D4C_185312C.s")
#endif

/* ---- overlay1UpdateTransient ---- */


typedef struct Overlay1SimpleTransientState {
    void *owner;
    s16 mode;
    u8 type;
    u8 active;
    u8 selector;
    s8 linkedIndex;
    u8 pad0A;
} Overlay1SimpleTransientState;

typedef struct Overlay1SimpleTransientObject {
    u8 pad00[8];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    u8 pad28[0x3C];
    Overlay1SimpleTransientState *state;
    u8 pad68[0x10];
    s16 *flags;
} Overlay1SimpleTransientObject;

typedef struct Overlay1SimpleTransientOwner {
    u8 pad00[0x28];
    f32 distance;
} Overlay1SimpleTransientOwner;

typedef struct Overlay1SimpleTransientWorld {
    u8 pad00[0x193];
    u8 mode;
    u8 pad194[0x14];
    u16 flags;
    u8 pad1AA[0x1EA];
    Overlay1SimpleTransientObject *object;
} Overlay1SimpleTransientWorld;

extern Overlay1SimpleTransientOwner *gOverlay1SimpleTransientOwner;
extern Overlay1SimpleTransientWorld *gOverlay1SimpleTransientWorld;
extern f32 gOverlay1TransientScale;
extern f32 gOverlay1TransientThreshold;
extern f32 gOverlay1TransientVelocityY;

#ifdef NON_MATCHING
void overlay1UpdateTransient(void) {
    Overlay1SimpleTransientObject *object;
    Overlay1SimpleTransientState *state;

    object = gOverlay1SimpleTransientWorld->object;
    if (object == 0) {
        object = func_overlay_036_F0000694_1883B4C(
            gOverlay1SimpleTransientOwner, gOverlay1SimpleTransientWorld);
        if (object != 0) {
            state = object->state;
            state->type = 2;
            state->active = 1;
            state->selector = 9;
            state->pad0A = 0;
            state->owner = gOverlay1SimpleTransientOwner;
            object->scale = gOverlay1TransientScale;
            overlay1ReadSelection(gOverlay1SimpleTransientOwner, 9, &object->x,
                                  &object->y, &object->z);
            gOverlay1SimpleTransientWorld->object = object;
        }
    } else {
        state = object->state;
    }

    if (gOverlay1SimpleTransientWorld->flags & 2) {
        overlay1InitTimedState(gOverlay1SimpleTransientOwner, 0x78);
    }
    if ((gOverlay1SimpleTransientWorld->mode == 0xD) &&
        (gOverlay1SimpleTransientOwner->distance >= gOverlay1TransientThreshold) &&
        state->active) {
        state->active = 0;
        state->linkedIndex = -1;
        state->mode = 0xC;
        object->velocityX = 0.0f;
        object->velocityY = gOverlay1TransientVelocityY;
        object->velocityZ = 0.0f;
        *object->flags &= ~2;
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F0007130_1853510.s")
#endif

/* ---- overlay1AllocateRecord ---- */


typedef struct Overlay1PoolRecord {
    u8 pad00[0xC0];
    u16 flags;
    u8 padC2[0xA];
} Overlay1PoolRecord;

extern Overlay1PoolRecord *gOverlay1PoolCursor;
extern Overlay1PoolRecord gOverlay1PoolStart[];
extern Overlay1PoolRecord gOverlay1PoolEnd[];
extern s32 gOverlay1PoolGroup;
extern s32 gOverlay1PoolExhausted;

/* Plateau (2026-08-25): the isolated 119-combination flag lattice and eight
 * source variants are exact-sized but retain 9 differing words, first at
 * +0x6C. The prefix and relocation surface are exact; only the post-predicate
 * temporary FIFO web differs. Explicit temporaries, typed union/byte access,
 * operand association, pointer aliases, and register/volatile hints all keep
 * the same coloring. The configured tools/permuter checkout is absent. */
#ifdef NON_MATCHING
Overlay1PoolRecord *overlay1AllocateRecord(void) {
    Overlay1PoolRecord *cursor;
    Overlay1PoolRecord *result;

    cursor = gOverlay1PoolCursor;
    result = cursor;
    do {
        cursor = (gOverlay1PoolCursor = cursor + 1);
        if (cursor >= gOverlay1PoolEnd) {
            gOverlay1PoolCursor = gOverlay1PoolStart;
            cursor = gOverlay1PoolStart;
        }
        if (result == cursor) {
            gOverlay1PoolExhausted = 1;
            return 0;
        }
    } while ((((u32)*((u8 *)cursor + 0xC1) >> 2) ==
              gOverlay1PoolGroup) && ((cursor->flags & 1) == 0));

    *((u8 *)result + 0xC1) = (*((u8 *)result + 0xC1) & 0xFF03) |
                                (gOverlay1PoolGroup << 2);
    return result;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F00072A4_1853684.s")
#endif

/* ---- overlay1CloneRecord ---- */


extern void *overlay1AllocateRecordReloc(u32 *source);

/* DKR v77/v80 and JFG have generic copy loops, but no exact donor. */
/* Plateau (2026-08-25): the isolated 119-combination flag lattice finds
 * -O2/-mips1 exact-sized with 2 differing words, first at +0x24; -mips2 is
 * one word worse. Retail schedules the destination setup before the loop
 * count, while IDO reverses that adjacent pair. Declaration/assignment order,
 * register hints, comma association, block lifetimes, initialized locals, and
 * a typed whole-record copy either retain the pair or disturb the exact loop.
 * The bounded permuter cannot import this internal consolidated-TU boundary. */
#ifdef NON_MATCHING
void *overlay1CloneRecord(u32 *source) {
    u32 *destination;
    register u32 remaining;
    void *result;

    result = overlay1AllocateRecordReloc(source);
    if (result == 0) {
        return 0;
    }
    remaining = 50;
    {
        u32 *input;

        input = source;
        destination = result;
        do {
            *destination++ = *input++;
        } while (remaining--);
    }
    return result;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F0007344_1853724.s")
#endif

/* ---- overlay1UpdateValueCache ---- */


typedef struct Overlay1CacheValueEntry {
    s16 keyA;
    s16 keyB;
    u32 value;
} Overlay1CacheValueEntry;

extern Overlay1CacheValueEntry gOverlay1ValueCache[64];

#ifdef NON_MATCHING
s32 overlay1UpdateValueCache(s16 keyA, s16 keyB, f32 value) {
    register s32 searchKeyA = keyA;
    register s32 searchKeyB = keyB;
    Overlay1CacheValueEntry *entry;
    s32 remaining;

    entry = gOverlay1ValueCache;
    remaining = 0x3F;
    do {
        if ((entry->value != 0) && (searchKeyA == entry->keyA) &&
            (searchKeyB == entry->keyB)) {
            if (value < (f32)entry->value) {
                entry->value = (u32)value;
                return 1;
            }
            return 0;
        }
        entry++;
    } while (remaining--);

    entry = gOverlay1ValueCache;
    remaining = 0x3F;
    do {
        if (entry->value == 0) {
            entry->keyA = searchKeyA;
            entry->keyB = searchKeyB;
            entry->value = (u32)value;
            return 1;
        }
        entry++;
    } while (remaining--);

    return 0;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F00073A0_1853780.s")
#endif

/* ---- overlay1AppendPathPoint ---- */


typedef struct Overlay1PathState {
    s16 x[32];
    s16 y[32];
    u8 primary[32];
    u8 secondary[32];
    u8 count;
    u8 flags;
    u8 padC2[2];
    f32 length;
    u32 anchorDistanceSquared;
} Overlay1PathState;

extern f32 sqrtf(f32 value);
extern s32 overlay1UpdateValueCache(s16 x, s16 y, f32 value);
extern s16 overlay1AnchorX;
extern s16 overlay1AnchorY;

#ifdef NON_MATCHING
void overlay1AppendPathPoint(Overlay1PathState *state, s16 x, s16 y,
                             u8 primary, u8 secondary) {
    register s32 pointX = x;
    register s32 pointY = y;
    u8 index = state->count;
    s16 dx = pointX - state->x[index];
    s16 dy;
    s16 anchorX;
    s16 anchorDx;

    dy = pointY - state->y[index];
    state->count = index + 1;
    state->x[state->count] = pointX;
    state->y[state->count] = pointY;
    state->primary[state->count] = primary;
    state->secondary[state->count] = secondary;
    state->length += sqrtf((f32)((dx * dx) + (dy * dy)));
    state->flags = (state->flags & ~3) | 1;

    if ((state->count >= 2) &&
        (overlay1UpdateValueCache(pointX, pointY, state->length) == 0)) {
        state->flags &= ~3;
        return;
    }

    anchorX = overlay1AnchorX;
    anchorDx = pointX - anchorX;
    if ((pointX == anchorX) && (pointY == overlay1AnchorY)) {
        state->anchorDistanceSquared = 0;
    } else {
        s16 anchorDy = pointY - overlay1AnchorY;
        state->anchorDistanceSquared =
            (anchorDx * anchorDx) + (anchorDy * anchorDy);
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F0007580_1853960.s")
#endif

/* ---- overlay1BendPathPoint ---- */


typedef struct Overlay1PathPoint {
    s16 x;
    s16 y;
} Overlay1PathPoint;

typedef struct Overlay1Path {
    Overlay1PathPoint *points;
    u32 count;
} Overlay1Path;

/* Fresh pinned DKR v77/v80 and JFG scans found no Overlay 1 donor. */
extern Overlay1Path *overlay1GetPathReloc(u8 selector);
extern s32 overlay1AngleReloc(f32 y, f32 x);
extern s32 overlay1AngleDifferenceReloc(s16 first, s16 second);
extern f32 overlay1TrigXReloc(s32 angle);
extern f32 overlay1TrigYReloc(s32 angle);

/* Plateau: exact 107 words/frame; best is 25 words different, first +0xC.
 * Angle-local order and a split previous-index decrement improve allocation;
 * parameter stack homes and integer/pointer registers remain divergent. */
#ifdef NON_MATCHING
void overlay1BendPathPoint(s16 *x, s16 *y, u8 index, u8 selector) {
    Overlay1PathPoint *next, *previous, *current;
    Overlay1Path *path;
    s16 firstAngle, secondAngle, midpointAngle;
    volatile u8 localIndex;
    s32 nextIndex, previousIndex, currentIndex;

    localIndex = index;
    path = overlay1GetPathReloc(selector);
    index = localIndex;
    current = &path->points[index];
    if (index != 0) {
        currentIndex = index;
        previousIndex = index - 1;
    } else {
        previousIndex = path->count;
        previousIndex--;
        currentIndex = 0;
    }
    previous = &path->points[previousIndex];
    if (currentIndex >= path->count) {
        nextIndex = 0;
    } else {
        nextIndex = currentIndex + 1;
    }
    next = &path->points[nextIndex];
    firstAngle = (s16)(overlay1AngleReloc((f32)(current->y - previous->y),
                                         (f32)(current->x - previous->x)) -
                       0x8000);
    secondAngle = (s16)(overlay1AngleReloc((f32)(current->y - next->y),
                                          (f32)(current->x - next->x)) -
                        0x8000);
    midpointAngle = firstAngle +
        (overlay1AngleDifferenceReloc(firstAngle, secondAngle) >> 1);
    *x = (s16)((f32)*x - overlay1TrigXReloc(midpointAngle) * 50.0f);
    *y = (s16)((f32)*y - overlay1TrigYReloc(midpointAngle) * 50.0f);
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F0007730_1853B10.s")
#endif

/* ---- overlay1AdvancePath ---- */


typedef struct Overlay1Point {
    s16 x;
    s16 y;
} Overlay1Point;

typedef struct Overlay1PathEntry {
    Overlay1Point *points;
} Overlay1PathEntry;

typedef struct Overlay1TraceResult {
    s16 x;
    s16 y;
    u16 base;
    u16 first;
    u16 second;
    u16 secondary;
    s32 changed;
} Overlay1TraceResult;

extern s32 overlay2TracePath(f32 x, f32 y, f32 anchorX, f32 anchorY,
                             void *arg5, Overlay1TraceResult *result,
                             u8 primary, u8 secondary);
extern Overlay1PathEntry *overlay1GetEntry(u16 index);
extern Overlay1PathState *overlay1CloneRecord(Overlay1PathState *source);
extern void overlay1AppendPathPoint(Overlay1PathState *state, s32 x, s32 y,
                                    u8 primary, u8 secondary);
extern s16 overlay1AnchorX;
extern s16 overlay1AnchorY;
extern s32 gOverlay1PoolExhausted;

/* DKR v77/v80 and JFG have no exact donor for this bounded path advance. */
/* Plateau (2026-08-24): all -O2 -mips2 flag variants are exact-sized, but
 * 84 of 162 words differ and the first mismatch is +0x10.  The residual
 * crosses the allocator loop, endpoint update, and callback path, beyond a
 * bounded temporary-order search. */
#ifdef NON_MATCHING
s32 overlay1AdvancePath(Overlay1PathState *state) {
    s16 currentX;
    s16 currentY;
    Overlay1PathEntry *entry;
    Overlay1TraceResult result;
    Overlay1PathState *child;
    register s32 nextX;
    u8 count;

    count = state->count;
    currentX = state->x[count];
    currentY = state->y[count];

    state->flags = (state->flags & ~3) |
                   (*(u16 *)&state->count & 1);
    if (count >= 31) {
        return 1;
    }

    if (!overlay2TracePath((f32)currentX, (f32)currentY,
                           (f32)overlay1AnchorX, (f32)overlay1AnchorY,
                           (void *)gOverlay1SubmitArg5, &result,
                           state->primary[count], state->secondary[count]) ||
        ((result.x == overlay1AnchorX) && (result.y == overlay1AnchorY))) {
        overlay1AppendPathPoint(state, overlay1AnchorX, overlay1AnchorY, 0xFF, 0);
        return 1;
    }

    entry = overlay1GetEntry(result.secondary);
    nextX = result.x;
    if ((currentX != nextX) || (currentY != result.y)) {
        overlay1AppendPathPoint(state, nextX, result.y,
                                *((u8 *)&result + 5), result.secondary);
        if (result.changed != 0) {
            state->flags = (state->flags & ~3) |
                           ((*(u16 *)&state->count | 2) & 3);
        }
    }

    if ((result.base != result.first) && (gOverlay1PoolExhausted == 0)) {
        child = overlay1CloneRecord(state);
        if (child != NULL) {
            overlay1AppendPathPoint(child, entry->points[result.first].x,
                                    entry->points[result.first].y,
                                    *((u8 *)&result + 7), result.secondary);
            child->flags = (child->flags & ~3) |
                           ((*(u16 *)&child->count | 2) & 3);
        }
    }

    if ((result.base != result.second) && (gOverlay1PoolExhausted == 0)) {
        child = overlay1CloneRecord(state);
        if (child != NULL) {
            overlay1AppendPathPoint(child, entry->points[result.second].x,
                                    entry->points[result.second].y,
                                    *((u8 *)&result + 9), result.secondary);
            child->flags = (child->flags & ~3) |
                           ((*(u16 *)&child->count | 2) & 3);
        }
    }
    return 1;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F00078DC_1853CBC.s")
#endif

/* ---- overlay1FindBestRecord ---- */


typedef struct Overlay1BestRecord {
    u8 pad00[0xC0];
    union {
        u16 flags;
        struct {
            u8 high;
            u8 type;
        } bytes;
    } header;
    u8 padC2[6];
    u32 value;
} Overlay1BestRecord;

extern Overlay1BestRecord gOverlay1BestRecords[32];
extern s32 gOverlay1SelectedType;

/* DKR v77/v80 and JFG have no exact donor for this fixed record scan. */
#ifdef NON_MATCHING
Overlay1BestRecord *overlay1FindBestRecord(void) {
    Overlay1BestRecord *record;
    Overlay1BestRecord *result;
    u32 bestValue;
    register u32 value;
    s32 remaining;
    register s32 selectedType;

    record = gOverlay1BestRecords;
    bestValue = (u32)-1;
    result = NULL;
    selectedType = gOverlay1SelectedType;
    remaining = 31;
    do {
        if (selectedType == ((u32)record->header.bytes.type >> 2)) {
            value = record->value;
            if ((value == 0) ||
                (((record->header.flags & 3) == 3) &&
                 (value < bestValue))) {
                bestValue = value;
                result = record;
            }
        }
        record++;
    } while (remaining--);
    return result;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F0007B64_1853F44.s")
#endif
