#ifndef PERMUTER
#include "PR/ultratypes.h"
#endif

typedef union Overlay77Coord {
    f32 value;
    s32 bits;
} Overlay77Coord;

typedef struct Overlay77State {
    s16 kind;
    s16 sequence;
    f32 acceleration;
    f32 scale;
    f32 targetY;
    f32 targetX;
    f32 targetYCopy;
    f32 targetZ;
} Overlay77State;

typedef struct Overlay77Object {
    s16 angle;
    u8 pad2[4];
    s16 flags;
    f32 scale;
    Overlay77Coord x;
    Overlay77Coord y;
    Overlay77Coord z;
    u8 pad18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    f32 field28;
    u8 pad2C[0x38];
    Overlay77State *state;
    void **path;
} Overlay77Object;

extern s32 gOverlay77Handle;
extern s32 gOverlay77Selection;
extern void *gOverlay77CallbackArgument;
extern f32 gOverlay77PositiveDivisor;
extern f32 gOverlay77PositiveAcceleration;
extern f32 gOverlay77NegativeDivisor;
extern f32 gOverlay77NegativeAcceleration;
extern f32 gOverlay77Gravity;

void overlay77PathReloc(void *path, s32 *mode, s16 kind, f32 *field,
                        s32 updateRate);
void overlay77SpawnReloc(s32 objectId, s32 x, s32 y, s32 z, s32 mode,
                         void *argument);
void overlay77ContinueReloc(void *handle, s32 x, s32 y, s32 z);
s32 overlay77MoveReloc(Overlay77Object *object, f32 x, f32 y, f32 z);
void overlay77OrientReloc(Overlay77Object *object, f32 x, f32 y, f32 z);
void overlay77StopReloc(void *handle);

/*
 * Overlay 77 +0x130. The pinned DKR v77/v80 and JFG object ledgers are exact
 * negative. DKR's object initializers and projectile physics are useful
 * semantic references only; no donor name or source was adopted.
 */
void overlay77Update(Overlay77Object *object, volatile s32 updateRate) {
    Overlay77State *state;
    volatile f32 unused;
    s32 mode;
    f32 temp_f0;
    f32 temp_f2;
    f32 temp_f12;
    f32 moveX;
    f32 moveZ;
    f32 temp_f14;

    state = object->state;
    mode = 9;
    overlay77PathReloc(*object->path, &mode, state->kind, &object->field28,
                       updateRate);

    if (gOverlay77Handle != 0) {
        if (gOverlay77Selection == state->sequence) { if (gOverlay77CallbackArgument == 0) { overlay77SpawnReloc(0x217, object->x.bits, object->y.bits, object->z.bits, 1, &gOverlay77CallbackArgument); } else { overlay77ContinueReloc(gOverlay77CallbackArgument, object->x.bits, object->y.bits, object->z.bits); } } object->flags &= ~0x400;
        temp_f0 = object->velocityY;
        temp_f12 = 0.0f;
        moveX = updateRate;
        if (temp_f0 >= temp_f12) {
            temp_f2 = (-(temp_f0 * temp_f0)) / gOverlay77PositiveDivisor;
            if (state->targetY <= (object->y.value + temp_f2)) {
                state->acceleration = temp_f12;
            } else {
                {
                    state->acceleration = gOverlay77PositiveAcceleration;
                }
            }
        } else {
            temp_f2 = (-(temp_f0 * temp_f0)) / gOverlay77NegativeDivisor;
            if ((object->y.value + temp_f2) < state->targetY) {
                state->acceleration = gOverlay77NegativeAcceleration;
            } else {
                state->acceleration = temp_f12;
            }
        }

        temp_f0 = ((0, object))->velocityY;
        temp_f12 = state->acceleration + gOverlay77Gravity;
        temp_f2 = (temp_f0 * moveX) + (((0.5f * temp_f12) * moveX) * moveX);
        object->velocityY = (0, temp_f0) + (temp_f12 * moveX);
        temp_f12 = 0.0f;
        {
            temp_f14 = object->velocityX * moveX;
            moveZ = object->velocityZ * moveX;
            if (overlay77MoveReloc(object, temp_f14, temp_f2, moveZ)) {
                gOverlay77Handle--;
            }
        }
    }

    if (gOverlay77Handle == 0) {
        temp_f14 = state->targetX - object->x.value;
        temp_f2 = state->targetYCopy - object->y.value;
        moveZ = state->targetZ - object->z.value;
        overlay77OrientReloc(object, temp_f14, temp_f2, moveZ);
        object->flags |= 0x400;
        if (gOverlay77CallbackArgument != 0) {
            overlay77StopReloc(gOverlay77CallbackArgument);
        }
    }
}
