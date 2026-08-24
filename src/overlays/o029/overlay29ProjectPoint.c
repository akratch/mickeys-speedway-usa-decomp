#include "PR/ultratypes.h"

typedef struct Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Vec3f;

typedef struct Overlay29PathState {
    u8 pad00[0x14];
    u32 flags;
    u8 pad18[4];
    Vec3f direction;
} Overlay29PathState;

typedef struct Overlay29Transform {
    Vec3f direction;
    u8 pad0C[4];
    Vec3f position;
    f32 height;
    u32 flags;
} Overlay29Transform;

typedef struct Overlay29Owner {
    u8 pad00[0x64];
    Overlay29PathState *state;
} Overlay29Owner;

extern f32 gOverlay29MinimumYReloc;
extern f32 overlay29SqrtReloc(f32 value);

void func_overlay_029_F0000EE0_187E190(
    s32 unused, Vec3f *output, Vec3f *axis, f32 height,
    Overlay29Transform *transform, Overlay29Owner *owner) {
    Overlay29PathState *state;
    f32 dirX;
    f32 dirY;
    f32 dirZ;
    f32 crossX;
    f32 crossY;
    f32 crossZ;
    f32 projectedX;
    f32 projectedY;
    f32 projectedZ;
    f32 lengthSquared;
    f32 length;
    f32 delta;

    dirY = transform->direction.y;
    dirX = transform->direction.x;
    dirZ = transform->direction.z;
    state = owner->state;
    if ((gOverlay29MinimumYReloc <= dirY) ||
        ((transform->flags & 0x10000000) != 0)) {
        crossX = (axis->z * dirY) - (axis->y * dirZ);
        crossY = (axis->x * dirZ) - (axis->z * dirX);
        crossZ = (axis->y * dirX) - (axis->x * dirY);
        projectedX = (crossY * dirZ) - (crossZ * dirY);
        projectedY = (crossZ * dirX) - (crossX * dirZ);
        projectedZ = (crossX * dirY) - (crossY * dirX);
        lengthSquared = (projectedX * projectedX) +
                        (projectedY * projectedY) +
                        (projectedZ * projectedZ);
        if (lengthSquared > 0.0f) {
            length = overlay29SqrtReloc(lengthSquared);
            projectedY /= length;
            projectedX /= length;
            projectedZ /= length;
            delta = height - transform->height;
            output->x = transform->position.x + (delta * projectedX);
            output->y = transform->position.y + (delta * projectedY);
            output->z = transform->position.z + (delta * projectedZ);
        } else {
            output->x = transform->position.x;
            output->y = transform->position.y;
            output->z = transform->position.z;
        }
        state->flags |= 2;
    } else {
        output->x = transform->position.x;
        output->y = transform->position.y;
        output->z = transform->position.z;
        state->direction.x = dirX;
        state->direction.y = dirY;
        state->direction.z = dirZ;
        state->flags |= 4;
    }
}
