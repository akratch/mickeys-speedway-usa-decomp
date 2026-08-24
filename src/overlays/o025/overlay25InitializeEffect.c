#include "PR/ultratypes.h"

/* Pinned DKR v77/v80 and JFG scans found no exact initializer donor. */

typedef struct Overlay25OwnerState {
    u8 pad00[4];
    f32 scale;
    u8 pad08[0xE8];
    s16 baseAngle;
    u8 padF2[0x0C];
    s16 relativeAngle;
} Overlay25OwnerState;

typedef struct Overlay25Owner {
    u8 pad00[0x64];
    Overlay25OwnerState *state;
} Overlay25Owner;

typedef struct Overlay25State {
    s16 lifetime;
    u8 duration;
    u8 activeDuration;
    f32 currentValue;
    f32 velocityX;
    f32 lift;
    f32 velocityZ;
    u8 pad14[4];
    u8 color[3];
    u8 pad1B;
    Overlay25Owner *owner;
} Overlay25State;

typedef struct Overlay25Vector { f32 x, y; } Overlay25Vector;

typedef struct Overlay25Object {
    u8 pad00[6];
    s16 flags;
    u8 pad08[0x44];
    Overlay25Vector *vector;
    u8 pad50[0x14];
    Overlay25State *state;
} Overlay25Object;

typedef struct Overlay25Init {
    u8 pad00[0x0A];
    s16 useOwner;
    Overlay25Owner *owner;
} Overlay25Init;

extern u16 gOverlay25GlobalFlagsReloc;
extern const u8 gOverlay25ColorsReloc[];
extern f32 overlay25SinReloc(s32 angle);
extern f32 overlay25CosReloc(s32 angle);
extern s32 overlay25RandomReloc(s32 lower, s32 upper);

#ifdef NON_MATCHING
void overlay25InitializeEffect(Overlay25Object *object,
                               const Overlay25Init *init) {
    Overlay25State *state;
    Overlay25Owner *owner;
    s32 paletteIndex;

    state = object->state;
    state->lifetime = 600;
    state->duration = 60;
    state->currentValue = 0.0f;

    owner = init->owner;
    state->owner = owner;

    if (init->useOwner != 0 && owner != NULL) {
        Overlay25OwnerState *ownerState = owner->state;
        s32 combinedAngle = ownerState->baseAngle + ownerState->relativeAngle;

        state->activeDuration = 60;
        state->velocityX =
            (overlay25SinReloc(combinedAngle) * ownerState->scale) +
            (overlay25SinReloc(ownerState->baseAngle) * -32.0f);
        state->lift = 16.0f;
        state->velocityZ =
            (overlay25CosReloc(combinedAngle) * ownerState->scale) +
            (overlay25CosReloc(ownerState->baseAngle) * -32.0f);
    } else {
        state->activeDuration = 0;
        object->flags |= 0x0800;
    }

    if (gOverlay25GlobalFlagsReloc & 0x10) {
        paletteIndex = overlay25RandomReloc(0, 7) * 3;
    } else {
        paletteIndex = 9;
    }
    state->color[0] = gOverlay25ColorsReloc[paletteIndex + 0];
    state->color[1] = gOverlay25ColorsReloc[paletteIndex + 1];
    state->color[2] = gOverlay25ColorsReloc[paletteIndex + 2];

    if (object->vector != NULL) {
        object->vector->x = 0.0f;
        object->vector->y = 0.0f;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o025/overlay25InitializeEffect/func_overlay_025_F0000000_1879C88.s")
#endif
