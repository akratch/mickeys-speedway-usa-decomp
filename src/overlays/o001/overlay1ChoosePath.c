#include "PR/ultratypes.h"

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

extern O1ChoiceState *D_1DA0;
extern O1ChoiceObject *D_1D9C;
extern O1ControlTable *D_1D60;
extern O1ControlTable *D_1D64;
extern O1ControlTable *D_1D68;
extern O1ControlTable *D_1D68Read;
extern O1ControlTable *D_1D6C;
extern O1ControlPoint *D_208;
extern O1ControlPoint *D_20C;
extern O1ControlPoint *D_210;
extern O1ControlPoint *D_214;
extern O1Pair D_1BA8[8][6];
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
