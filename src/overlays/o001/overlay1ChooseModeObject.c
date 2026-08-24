#include "PR/ultratypes.h"

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
extern O1SelectObject *D_1D9C;
extern O1SelectWorld *D_1DA0;
extern O1SelectRow D_1BA8[];

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
