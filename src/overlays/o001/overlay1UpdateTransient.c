#include "PR/ultratypes.h"

typedef struct Overlay1TransientState {
    void *owner;
    s16 mode;
    u8 type;
    u8 active;
    u8 selector;
    s8 linkedIndex;
    u8 pad0A;
} Overlay1TransientState;

typedef struct Overlay1TransientObject {
    u8 pad00[8];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    u8 pad28[0x3C];
    Overlay1TransientState *state;
    u8 pad68[0x10];
    s16 *flags;
} Overlay1TransientObject;

typedef struct Overlay1TransientOwner {
    u8 pad00[0x28];
    f32 distance;
} Overlay1TransientOwner;

typedef struct Overlay1TransientWorld {
    u8 pad00[0x193];
    u8 mode;
    u8 pad194[0x14];
    u16 flags;
    u8 pad1AA[0x1EA];
    Overlay1TransientObject *object;
} Overlay1TransientWorld;

extern Overlay1TransientOwner *gOverlay1TransientOwner;
extern Overlay1TransientWorld *gOverlay1TransientWorld;
extern f32 gOverlay1TransientScale;
extern f32 gOverlay1TransientThreshold;
extern f32 gOverlay1TransientVelocityY;

extern Overlay1TransientObject *func_overlay_036_F0000694_1883B4C(
    Overlay1TransientOwner *owner, Overlay1TransientWorld *world);
extern void overlay1ReadSelection(Overlay1TransientOwner *owner, s32 index,
                                  f32 *x, f32 *y, f32 *z);
extern void overlay1InitTimedState(Overlay1TransientOwner *owner, s32 timer);

#ifdef NON_MATCHING
void overlay1UpdateTransient(void) {
    Overlay1TransientObject *object;
    Overlay1TransientState *state;

    object = gOverlay1TransientWorld->object;
    if (object == 0) {
        object = func_overlay_036_F0000694_1883B4C(
            gOverlay1TransientOwner, gOverlay1TransientWorld);
        if (object != 0) {
            state = object->state;
            state->type = 2;
            state->active = 1;
            state->selector = 9;
            state->pad0A = 0;
            state->owner = gOverlay1TransientOwner;
            object->scale = gOverlay1TransientScale;
            overlay1ReadSelection(gOverlay1TransientOwner, 9, &object->x,
                                  &object->y, &object->z);
            gOverlay1TransientWorld->object = object;
        }
    } else {
        state = object->state;
    }

    if (gOverlay1TransientWorld->flags & 2) {
        overlay1InitTimedState(gOverlay1TransientOwner, 0x78);
    }
    if ((gOverlay1TransientWorld->mode == 0xD) &&
        (gOverlay1TransientOwner->distance >= gOverlay1TransientThreshold) &&
        state->active) {
        state->active = 0;
        state->linkedIndex = -1;
        state->mode = 0xC;
        object->velocityX = 0.0f;
        object->velocityY = gOverlay1TransientVelocityY;
        object->velocityZ = 0.0f;
        *object->flags &= ~2;
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1UpdateTransient/func_overlay_001_F0007130_1853510.s")
#endif
