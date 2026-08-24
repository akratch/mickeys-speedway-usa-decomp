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

typedef struct Overlay1MotionSourceExtra {
    u8 pad00[0x5C];
    f32 height;
} Overlay1MotionSourceExtra;

typedef struct Overlay1MotionSourceState {
    u8 pad00;
    s8 index;
} Overlay1MotionSourceState;

typedef struct Overlay1MotionSource {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    u8 pad28[0x20];
    Overlay1MotionSourceExtra *extra;
    u8 pad4C[0x18];
    Overlay1MotionSourceState *state;
} Overlay1MotionSource;

typedef struct Overlay1VelocityExtra {
    u8 pad00[0xE0];
    f32 *value;
} Overlay1VelocityExtra;

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
    u8 pad28[0x18];
    Overlay1VelocityExtra *extra;
    u8 pad44[0x20];
    Overlay1TransientState *state;
    u8 pad68[0x10];
    s16 *flags;
} Overlay1TransientObject;

typedef struct Overlay1TransientOwner {
    s16 angle;
    u8 pad02[0x26];
    f32 distance;
} Overlay1TransientOwner;

typedef struct Overlay1TransientWorld {
    u8 pad00[0x193];
    u8 mode;
    u8 pad194[0x14];
    u16 flags;
    u8 pad1AA[0x1E6];
    Overlay1MotionSource *source;
    Overlay1TransientObject *object;
    u8 pad398[0x84];
    u32 status;
} Overlay1TransientWorld;

extern Overlay1TransientOwner *D_1D9C;
extern Overlay1TransientWorld *D_1DA0;
extern f32 D_4;
extern f32 D_188;
extern f32 D_18C;
extern f32 D_190;
extern f32 D_194;

extern Overlay1TransientObject *func_overlay_036_F0000694_1883B4C(
    Overlay1TransientOwner *owner, Overlay1TransientWorld *world);
extern void overlay1ReadSelection(Overlay1TransientOwner *owner, s32 index,
                                  f32 *x, f32 *y, f32 *z);
extern s16 func_overlay_001_F00064F8_18528D8(
    f32, f32, f32, f32, f32, f32, f32, f32, s32);
extern s32 func_8002A910(f32 y, f32 x);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern f32 sqrtf(f32 value);
extern void overlay1InitTimedState(Overlay1TransientOwner *owner, s32 timer);

void overlay1UpdateAimedTransient(void) {
    Overlay1TransientWorld *world;
    Overlay1TransientWorld **worldRef;
    Overlay1TransientOwner *owner;
    Overlay1TransientObject *object;
    Overlay1TransientState *state;
    Overlay1TransientState *savedState;
    Overlay1MotionSource *source;
    f32 factor;
    f32 predictedX;
    f32 predictedY;
    f32 predictedZ;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 distance;
    f32 trig;
    s32 iteration;
    s16 sourceAngle;
    s16 objectAngle;

    worldRef = &D_1DA0;
    world = *worldRef;
    object = world->object;
    source = world->source;
    if (object == 0) {
        object = func_overlay_036_F0000694_1883B4C(D_1D9C, world);
        if (object != 0) {
            state = object->state;
            state->owner = D_1D9C;
            state->type = 3;
            state->active = 1;
            state->selector = 9;
            state->pad0A = 0;
            object->scale = D_188;
            overlay1ReadSelection(D_1D9C, 9, &object->x, &object->y,
                                  &object->z);
            (*worldRef)->object = object;
            savedState = state;
        }
        world = D_1DA0;
        state = savedState;
    } else {
        state = object->state;
    }

    if ((world->mode == 0xD) && state->active &&
        (D_18C <= D_1D9C->distance)) {
        state->active = 0;
        state->mode = 0xC;
        if (source != 0) {
            state->linkedIndex = source->state->index;
            factor = 0.0f;
            iteration = 3;
            do {
                predictedX = source->x + (factor * source->velocityX);
                predictedY = source->y + (factor * source->velocityY) +
                    (source->extra->height * 0.5f);
                predictedZ = source->z + (factor * source->velocityZ);
                deltaX = predictedX - object->x;
                deltaY = predictedY - object->y;
                deltaZ = predictedZ - object->z;
                distance = sqrtf((deltaX * deltaX) + (deltaY * deltaY) +
                                 (deltaZ * deltaZ));
                factor = 30.0f / distance;
                if (factor > 0.0f) {
                    deltaX *= factor;
                    deltaZ *= factor;
                }
                factor = distance / (30.0f * D_4);
            } while (iteration--);

            sourceAngle = func_8002A910(deltaX, deltaZ);
            objectAngle = func_overlay_001_F00064F8_18528D8(
                object->x, object->y, object->z,
                predictedX, predictedY, predictedZ,
                30.0f, -*object->extra->value, 0);
            trig = func_8002A8BC(objectAngle);
            object->velocityX = func_8002A8C0(sourceAngle) * trig * 30.0f;
            object->velocityY = func_8002A8C0(objectAngle) * 30.0f;
            trig = func_8002A8BC(objectAngle);
            object->velocityZ = func_8002A8BC(sourceAngle) * trig * 30.0f;
        } else {
            state->linkedIndex = -1;
            trig = D_190;
            object->velocityX = func_8002A8C0(D_1D9C->angle) * trig * -30.0f;
            object->velocityY = D_194;
            object->velocityZ = func_8002A8BC(D_1D9C->angle) * trig * -30.0f;
        }
        *object->flags &= ~2;
        world = D_1DA0;
    }

    if (world->flags & 2) {
        if (world->mode == 0xD) {
            overlay1InitTimedState(D_1D9C, 0x78);
            world = D_1DA0;
        }
        if (world->mode == 0xD) {
            world->mode = 0xD;
            world = D_1DA0;
        }
    }
    if (!(world->status & 0x2000) && world->mode == 0xD) {
        world->mode = 0xD;
    }
}
