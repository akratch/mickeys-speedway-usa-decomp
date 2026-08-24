#include "PR/ultratypes.h"

typedef struct Overlay8ColorSource {
    u8 pad00[0x38];
    u8 red;
    u8 green;
    u8 blue;
} Overlay8ColorSource;

typedef struct Overlay8ColorOwner {
    u8 pad00[0x64];
    Overlay8ColorSource *colors;
} Overlay8ColorOwner;

typedef struct Overlay8ColorState {
    u8 pad000[0xD0];
    Overlay8ColorOwner *owner;
    u8 pad0D4[0x96];
    s16 timer16A;
    u8 pad16C[0x18];
    u8 alternate184;
    u8 pad185;
    u8 flags186;
    u8 pad187[0x1CD];
    void *target354;
    u8 pad358[8];
    void *target360;
} Overlay8ColorState;

extern void func_overlay_008_F0000000_185DD58(void *target, s32 alpha0,
                                               s32 alpha1, s32 alpha2,
                                               s32 red, s32 green, s32 blue);

void func_overlay_008_F0003278_1860FD0(void *unused0,
                                       Overlay8ColorState *state,
                                       void *unused2) {
    s32 red;
    s32 green;
    s32 blue;

    if ((state->flags186 & 3) == 0) {
        return;
    }

    if ((state->timer16A > 0) && (state->owner != NULL)) {
        red = state->owner->colors->red;
        green = state->owner->colors->green;
        blue = state->owner->colors->blue;
    } else {
        red = 0;
        green = 0x40;
        if (state->alternate184 != 0) {
            green = 0;
            red = 0xE0;
            blue = 0x40;
        } else {
            blue = 0xE0;
        }
    }

    if (state->target354 != NULL) {
        func_overlay_008_F0000000_185DD58(state->target354, 0xFF, 0xFF, 0xFF,
                                          red, green, blue);
    }
    if (state->target360 != NULL) {
        func_overlay_008_F0000000_185DD58(state->target360, 0xFF, 0xFF, 0xFF,
                                          red, green, blue);
    }
}
