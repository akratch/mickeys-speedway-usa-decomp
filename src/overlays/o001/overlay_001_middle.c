#include "overlays/overlay_001.h"

/* ---- overlay1FindNextAngle ---- */


typedef struct Overlay1AngleData {
    u8 pad00[0x39C];
    f32 angle;
} Overlay1AngleData;

typedef struct Overlay1AngleObject {
    u8 pad00[0x64];
    Overlay1AngleData *data;
} Overlay1AngleObject;

extern Overlay1AngleObject **overlay1GetAngleObjectsReloc(s32 *count);
extern f32 overlay1WrapOffset(f32 first, f32 second);
extern f32 gOverlay1NextAngleLimit;

/* DKR v77/v80 and JFG contain no exact donor for this angle-selection scan. */
#ifdef NON_MATCHING
Overlay1AngleObject *overlay1FindNextAngle(f32 angle) {
    s32 count;
    Overlay1AngleObject **objects;
    Overlay1AngleObject **cursor;
    Overlay1AngleObject *object;
    Overlay1AngleObject *best;
    f32 difference;
    f32 bestDifference;
    s32 remaining;
    objects = overlay1GetAngleObjectsReloc(&count);
    bestDifference = gOverlay1NextAngleLimit;
    best = (Overlay1AngleObject *)(count - count);
    if (count != 0) {
        remaining = count - 1;
        cursor = (Overlay1AngleObject **)((u8 *)objects + (remaining << 2));
        do {
            object = *cursor;
            objects = (Overlay1AngleObject **)object->data;
            difference = overlay1WrapOffset(
                angle, ((Overlay1AngleData *)objects)->angle);
            if ((difference > 0.0f) && (difference < bestDifference)) {
                bestDifference = difference;
                best = object;
            }
            cursor = (Overlay1AngleObject **)((u8 *)cursor - 4);
        } while (remaining--);
    }
    return best;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_middle/func_overlay_001_F0002744_184EB24.s")
#endif

/* ---- overlay1FindPreviousAngle ---- */


typedef Overlay1AngleData Overlay1PreviousAngleData;
typedef Overlay1AngleObject Overlay1PreviousAngleObject;
extern f32 overlay1WrapOffset(f32 first, f32 second);
extern f32 gOverlay1PreviousAngleLimit;

/* DKR v77/v80 and JFG contain no exact donor for this angle-selection scan. */
#ifdef NON_MATCHING
Overlay1PreviousAngleObject *overlay1FindPreviousAngle(f32 angle) {
    s32 count;
    Overlay1PreviousAngleObject **objects;
    Overlay1PreviousAngleObject **cursor;
    Overlay1PreviousAngleObject *object;
    Overlay1PreviousAngleObject *best;
    f32 difference;
    f32 bestDifference;
    s32 remaining;

    objects = overlay1GetAngleObjectsReloc(&count);
    bestDifference = gOverlay1PreviousAngleLimit;
    best = (Overlay1PreviousAngleObject *)(count - count);
    if (count != 0) {
        remaining = count - 1;
        cursor = (Overlay1PreviousAngleObject **)((u8 *)objects +
                                                  (remaining << 2));
        do {
            object = *cursor;
            objects = (Overlay1PreviousAngleObject **)object->data;
            difference = overlay1WrapOffset(
                ((Overlay1PreviousAngleData *)objects)->angle, angle);
            if ((difference > 0.0f) && (difference < bestDifference)) {
                bestDifference = difference;
                best = object;
            }
            cursor = (Overlay1PreviousAngleObject **)((u8 *)cursor - 4);
        } while (remaining--);
    }
    return best;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_middle/func_overlay_001_F000280C_184EBEC.s")
#endif

/* ---- overlay1RefreshMode ---- */


typedef struct Overlay1ModeObject {
    u8 pad00[0x38C];
    u8 mode;
} Overlay1ModeObject;

extern void *gOverlay1ModeSource;
extern Overlay1ModeObject *gOverlay1ModeObject;
extern s32 overlay1ReadModeReloc(void *source);

/* DKR v77/v80 and JFG have no exact donor for this mode refresh wrapper. */
void overlay1RefreshMode(s32 arg0, s32 arg1, s32 arg2) {
    if (overlay1ReadModeReloc(gOverlay1ModeSource) >= 3) {
        gOverlay1ModeObject->mode = 2;
    } else {
        gOverlay1ModeObject->mode = 1;
    }
    overlay1ReadModeReloc(gOverlay1ModeSource);
}

/* ---- overlay1CallGlobal ---- */


/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern void overlay1GlobalCallReloc();
extern void *gOverlay1SubmitArg4;

void overlay1CallGlobal(s32 unused0, s32 unused1, s32 unused2) {
    overlay1GlobalCallReloc(gOverlay1SubmitArg4);
}

/* ---- overlay1AdvanceObjectGauges ---- */


typedef struct O1GaugeState {
    u8 pad000[0x384]; s8 level; u8 pad385[0x7B]; s32 value; s32 levelValues[1];
} O1GaugeState;
typedef struct O1GaugeObject { u8 pad00[0x64]; O1GaugeState *state; } O1GaugeObject;
typedef struct O1GaugeOwner { u8 pad00[0x86]; s8 levelLimit; } O1GaugeOwner;
extern s32 D_0;
extern s32 overlay1GetGaugeObjectsRaw(s32 *count);
extern s32 overlay1GetGaugeLimit(O1GaugeObject *object);

#ifdef NON_MATCHING
s32 overlay1AdvanceObjectGauges(O1GaugeOwner *owner, s32 amount) {
    O1GaugeObject **objects;
    O1GaugeObject *object;
    O1GaugeState *state;
    s32 count;
    s32 index;
    s32 delta;
    s32 limit;
    s32 result;

    result = overlay1GetGaugeObjectsRaw(&count);
    if (count != 0) {
        index = count - 1;
        objects = (O1GaugeObject **)result + index;
        do {
            object = *objects;
            state = object->state;
            if ((D_0 == 0) && (state->level < owner->levelLimit)) {
                delta = amount * 5;
                state->value += delta;
                limit = overlay1GetGaugeLimit(object);
                if (limit < state->value) state->value = limit;
                state->levelValues[state->level] += delta;
                if (state->levelValues[state->level] >= 180001) {
                    state->levelValues[state->level] = 180000;
                }
            }
            result = index;
            objects--;
            index--;
        } while (result != 0);
    }
    return result;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_middle/func_overlay_001_F000296C_184ED4C.s")
#endif

/* ---- overlay1AdvanceGauge ---- */


typedef struct O1AdvanceGaugeState { u8 pad000[0x3FA]; s16 disabled; u8 pad3FC[4]; s32 value; } O1AdvanceGaugeState;
typedef struct O1AdvanceGaugeObject { u8 pad00[0x64]; O1AdvanceGaugeState *state; } O1AdvanceGaugeObject;
extern s32 D_0;
extern O1AdvanceGaugeObject **overlay1GetGaugeObjects(s32 *count);

#ifdef NON_MATCHING
void overlay1AdvanceGauge(s32 amount) {
    volatile s32 private;
    s32 count;
    s32 index;
    s32 loopValue;
    O1AdvanceGaugeObject **objects;
    O1AdvanceGaugeObject *object;
    O1AdvanceGaugeState *state;

    objects = overlay1GetGaugeObjects(&count);
    if (count != 0) {
        index = count - 1;
        objects += index;
        do {
            object = *objects--;
            state = object->state;
            if ((D_0 == 0) && (state->disabled == 0)) {
                state->value += amount * 5;
                if (state->value >= 540001) state->value = 540000;
            }
            loopValue = index;
            index--;
        } while (loopValue != 0);
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_middle/func_overlay_001_F0002AA4_184EE84.s")
#endif
