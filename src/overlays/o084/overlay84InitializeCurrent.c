#include "PR/ultratypes.h"

typedef struct Overlay84Template {
    s16 angle;
    s16 tilt;
    u8 pad04[8];
    f32 x;
    f32 y;
    f32 z;
} Overlay84Template;

typedef struct Overlay84State {
    u8 pad00;
    s8 index;
    u8 pad02;
    u8 value3;
    u8 value4;
    u8 pad05[4];
    u8 mode9;
    u8 pad0A[6];
    s16 tilt10;
    s16 tilt12;
    s16 value14;
    s16 value16;
    u8 pad18[8];
    s32 angle20;
    f32 position24;
    f32 position28;
    f32 scale2C;
    f32 x30;
    f32 y34;
    f32 z38;
    u8 pad3C[8];
    Overlay84Template *templates[0x20];
    u8 valueC4;
} Overlay84State;

typedef struct Overlay84Object {
    u8 pad00[0x64];
    Overlay84State *state;
} Overlay84Object;

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern Overlay84Object *gOverlay84CurrentObject;

void overlay84InitializeCurrent(void) {
    Overlay84Object *object;
    Overlay84State *state;
    Overlay84Template *template;
    s16 tilt;
    f32 position;

    object = gOverlay84CurrentObject;
    if (object != NULL) {
        state = object->state;
        template = state->templates[state->index];
        state->scale2C = 1.0f;
        state->mode9 = 2;
        state->valueC4 = 0xFF;
        state->value4 = 0;
        state->value3 = 0x14;
        tilt = -template->tilt; state->value14 = state->value16;
        state->tilt10 = tilt;
        state->tilt12 = tilt;
        state->angle20 = 0x8000 - template->angle;
        position = template->y;
        state->position24 = position;
        state->position28 = position;
        state->x30 = template->x;
        state->y34 = template->y;
        state->z38 = template->z;
    }
}
