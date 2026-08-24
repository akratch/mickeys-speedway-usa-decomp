#include "PR/ultratypes.h"

typedef struct Overlay1NearbyState {
    f32 radius;
    s32 mode;
    u16 kind;
} Overlay1NearbyState;

typedef struct Overlay1OtherState {
    s8 kind;
    u8 pad01[0x191];
    u8 pending;
    u8 pad193[0x221];
    s16 count;
} Overlay1OtherState;

typedef struct Overlay1NearbyObject {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    void *state;
} Overlay1NearbyObject;

extern Overlay1NearbyObject **overlay1GetObjectListReloc(s32 *count);

#ifdef NON_MATCHING
void overlay1ConsumeNearbyPending(void *objectArg, void *listArg) {
    Overlay1NearbyState *state;
    f32 radiusSquared;
    s32 count;
    register Overlay1OtherState *otherState;
    register Overlay1NearbyObject *other;

    state = ((Overlay1NearbyObject *)objectArg)->state;
    radiusSquared = state->radius * 4.0f;
    radiusSquared *= state->radius * 4.0f;
    {
        Overlay1NearbyObject *object = objectArg;
        listArg = overlay1GetObjectListReloc(&count);
        if (count--) {
            do {
                other = ((Overlay1NearbyObject **)listArg)[count];
                otherState = other->state;
                if (state->kind == otherState->kind) {
                    f32 dx = other->x - object->x;
                    f32 dy = other->y - object->y;
                    f32 dz = other->z - object->z;
                    if (((dx * dx) + (dy * dy) + (dz * dz) < radiusSquared) &&
                        (state->mode == 2)) {
                        u8 pending = otherState->pending;
                        if (pending) {
                            otherState->pending = 0;
                            otherState->count += pending;
                        }
                    }
                }
            } while (count--);
        }
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1ConsumeNearbyPending/func_overlay_001_F0006A14_1852DF4.s")
#endif
