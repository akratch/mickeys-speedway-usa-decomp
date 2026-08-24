#include "PR/ultratypes.h"

typedef struct O1GaugeState {
    u8 pad000[0x384]; s8 level; u8 pad385[0x7B]; s32 value; s32 levelValues[1];
} O1GaugeState;
typedef struct O1GaugeObject { u8 pad00[0x64]; O1GaugeState *state; } O1GaugeObject;
typedef struct O1GaugeOwner { u8 pad00[0x86]; s8 levelLimit; } O1GaugeOwner;
extern s32 D_0;
extern s32 overlay1GetGaugeObjectsRaw(s32 *count);
extern s32 overlay1GetGaugeLimit(O1GaugeObject *object);

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
