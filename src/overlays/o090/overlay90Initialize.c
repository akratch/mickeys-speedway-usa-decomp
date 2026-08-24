#include "PR/ultratypes.h"

typedef struct Overlay90State {
    u8 active;
    u8 flag;
    s16 angle;
    f32 x;
    f32 y;
    f32 z;
    f32 value10;
    f32 value14;
    f32 value18;
    f32 value1C;
    f32 value20;
    s16 value24;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 value2E;
    f32 value30;
    s16 value34;
    s16 value36;
    s32 value38;
    s16 value3C;
} Overlay90State;

typedef struct Overlay90Owner {
    u8 pad00[0x64];
    Overlay90State *state;
} Overlay90Owner;

typedef struct Overlay90Config {
    u8 pad0[4];
    s16 x;
    s16 y;
    s16 z;
    s16 angle;
} Overlay90Config;

extern f32 gOverlay90Value1C;
extern f32 gOverlay90Value20;
extern void overlay90CommitReloc(Overlay90Owner *, s32, s32, f32);

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
