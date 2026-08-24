#ifndef OVERLAY_090_H
#define OVERLAY_090_H

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

void overlay90CommitReloc(Overlay90Owner *owner, s32 arg1, s32 arg2,
                          f32 arg3);
void overlay90Initialize(Overlay90Owner *owner, Overlay90Config *config);

#endif
