#include "PR/ultratypes.h"

typedef struct Overlay8UpdateFlag {
    u8 flags;
} Overlay8UpdateFlag;

typedef struct Overlay8UpdateChild {
    u8 pad00[0x75];
    u8 near;
    u8 pad76[0x0A];
    f32 target80;
    Overlay8UpdateFlag *flag;
} Overlay8UpdateChild;

typedef struct Overlay8UpdateOwner {
    u8 pad00[0x10];
    f32 position10;
    u8 pad14[0x0C];
    f32 velocity20;
    u8 pad24[0x30];
    Overlay8UpdateChild *child;
} Overlay8UpdateOwner;

typedef struct Overlay8UpdateInput {
    u8 pad00[2];
    u8 active;
    u8 pad03;
    f32 lateral;
    u8 pad08[0x60];
    f32 position68;
    f32 delta6C;
} Overlay8UpdateInput;

extern const f32 gOverlay8UpdateLowerReloc;
extern const f32 gOverlay8UpdateUpperReloc;
extern const f32 gOverlay8UpdateDecayReloc;
extern void overlay8FinishUpdateReloc(Overlay8UpdateOwner *owner,
                                      s32 updateRate);

void func_overlay_008_F0002EC0_1860C18(register Overlay8UpdateOwner *owner,
                                       Overlay8UpdateInput *input,
                                       s32 updateRate) {
    Overlay8UpdateFlag *flag;
    f32 delta;
    f32 decay;
    s32 remaining;

    if (owner->child == NULL) {
        return;
    }

    flag = owner->child->flag;
    owner->child->near = 0;
    if (flag != NULL) {
        flag->flags &= ~2;
    }

    if (input->active != 0) {
        input->delta6C = input->position68 - owner->position10;
        delta = input->delta6C;

        if (delta < 25.0f) {
            if ((input->lateral < gOverlay8UpdateLowerReloc) ||
                (input->lateral > gOverlay8UpdateUpperReloc)) {
                if (flag != NULL) {
                    flag->flags |= 2;
                }
            } else if (delta < 12.0f) {
                owner->child->near = 1;
            }
            owner->child->target80 = owner->position10 + 4.0f;
        } else if ((owner->velocity20 < 0.0f) && (updateRate != 0)) {
            remaining = updateRate - 1;
            decay = gOverlay8UpdateDecayReloc;
            do {
                owner->velocity20 -= owner->velocity20 * decay;
            } while (remaining--);
        }
    }

    overlay8FinishUpdateReloc(owner, updateRate);
}
