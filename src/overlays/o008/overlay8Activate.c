#include "PR/ultratypes.h"

typedef struct Overlay8ActivationState {
    s8 type;
    u8 pad001[0xC3];
    void *resource;
    u8 pad0C8[0x3A];
    s16 activeDirection;
    u8 pad104[0x54];
    s16 gate158;
    u8 pad15A[0x29];
    u8 active183;
    u8 pad184;
    u8 active185;
    u8 pad186;
    u8 timer187;
    u8 pad188[0x20];
    u16 flags1A8;
} Overlay8ActivationState;

typedef struct Overlay8ActivationOwner {
    u8 pad00[0x0C];
    s32 valueC;
    s32 value10;
    s32 value14;
    u8 pad18[0x4C];
    Overlay8ActivationState *state;
} Overlay8ActivationOwner;

extern s32 gOverlay8ActivationGateTimerReloc;
extern void overlay8ReleaseResourceReloc(void *resource);
extern void overlay8CreateResourceReloc(s32 kind, s32 valueC, s32 value10,
                                        s32 value14, s32 count,
                                        void **resourceOut);
extern void overlay8FinalizeActivationReloc(Overlay8ActivationOwner *owner,
                                            s32 code);

void func_overlay_008_F0000F1C_185EC74(Overlay8ActivationOwner *owner,
                                       s32 force) {
    Overlay8ActivationState *state;

    state = owner->state;
    if ((state->activeDirection != 0) || (state->gate158 != 0)) {
        return;
    }
    if ((force == 0) && (state->active185 == 1)) {
        return;
    }

    state->active183 = 1;
    state->active185 = 1;
    state->timer187 = 0x1E;

    if (((state->flags1A8 & 1) == 0) || (state->type == 0) ||
        (gOverlay8ActivationGateTimerReloc == 0)) {
        if (state->resource != NULL) {
            overlay8ReleaseResourceReloc(state->resource);
        }
        overlay8CreateResourceReloc(8, owner->valueC, owner->value10,
                                    owner->value14, 4, &state->resource);
        gOverlay8ActivationGateTimerReloc = 0x3C;
    }

    overlay8FinalizeActivationReloc(owner, 0x17);
}
