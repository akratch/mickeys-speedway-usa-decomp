#include "PR/ultratypes.h"

typedef struct O1GaugeState {
    s8 type; u8 pad001[0x1A7]; u16 flags; u8 pad1AA[0x250];
    s16 enabled; u8 pad3FC[4]; s32 value;
} O1GaugeState;
typedef struct O1GaugeObject { u8 pad00[0x64]; O1GaugeState *state; } O1GaugeObject;
typedef struct O1GaugeTableEntry { u8 pad00[8]; s32 value; u8 pad0C[0x1C]; } O1GaugeTableEntry;
extern O1GaugeTableEntry *overlay1GetGaugeTable(void);
extern O1GaugeObject **overlay1GetGaugeObjects(s32 *count);
extern s32 overlay1RandomRange(s32 minimum, s32 maximum);

#ifdef NON_MATCHING
void overlay1InitializeGaugeObjects(void) {
    O1GaugeTableEntry *table;
    O1GaugeObject **objects;
    O1GaugeObject **firstCursor;
    O1GaugeObject **secondCursor;
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
        firstCursor = objects + index;
        do {
            state = (*firstCursor--)->state;
            if ((state->enabled != 0) && (maximum < state->value)) maximum = state->value;
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
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1InitializeGaugeObjects/func_overlay_001_F0003578_184F958.s")
#endif
