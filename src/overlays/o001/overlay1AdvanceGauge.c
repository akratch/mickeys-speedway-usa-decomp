#include "PR/ultratypes.h"

typedef struct O1GaugeState { u8 pad000[0x3FA]; s16 disabled; u8 pad3FC[4]; s32 value; } O1GaugeState;
typedef struct O1GaugeObject { u8 pad00[0x64]; O1GaugeState *state; } O1GaugeObject;
extern s32 D_0;
extern O1GaugeObject **overlay1GetGaugeObjects(s32 *count);

#ifdef NON_MATCHING
void overlay1AdvanceGauge(s32 amount) {
    volatile s32 private;
    s32 count;
    s32 index;
    s32 loopValue;
    O1GaugeObject **objects;
    O1GaugeObject *object;
    O1GaugeState *state;

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
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1AdvanceGauge/func_overlay_001_F0002AA4_184EE84.s")
#endif
