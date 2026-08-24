#include "ultra64.h"

typedef struct Overlay36SelectObject {
    u8 pad000[0x19A];
    u8 state;
    u8 countdown;
    s32 timer;
    void *data;
} Overlay36SelectObject;

typedef struct Overlay36SelectSource {
    u8 pad00[0x64];
    Overlay36SelectObject *object;
} Overlay36SelectSource;

extern u8 gOverlay36StateValues[];
extern u8 gOverlay36StateData[];

/* Mickey state-table selection has no exact DKR v77/v80 or JFG donor. */
void overlay36SelectState(Overlay36SelectSource *source, s32 unused,
                          s32 selection, s32 limit) {
    Overlay36SelectObject *object;
    u8 *values;

    object = source->object;
    if (selection == object->state) {
        if (object->countdown < limit) {
            object->countdown++;
        }
    } else {
        object->data = &gOverlay36StateData[selection * 60];
        values = &gOverlay36StateValues[selection * 2];
        object->state = values[0];
        object->countdown = values[1];
        object->timer = 120;
    }
}
