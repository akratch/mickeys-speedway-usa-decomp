#include "PR/ultratypes.h"

/* Trigger/update path; exact DKR and JFG scans are negative. */
typedef union Overlay81Coord {
    f32 value;
    s32 bits;
} Overlay81Coord;

typedef struct Overlay81State {
    u8 pad0[0xC];
    s32 timer;
    s32 active;
    s32 mask;
} Overlay81State;

typedef struct Overlay81Trigger {
    u8 pad0[0x61];
    u8 active;
} Overlay81Trigger;

typedef struct Overlay81Collision {
    s16 flags;
    s16 active;
    f32 yOffset;
} Overlay81Collision;

typedef struct Overlay81Object {
    u8 pad0[0xC];
    Overlay81Coord x;
    Overlay81Coord y;
    Overlay81Coord z;
    u8 pad18[0x30];
    Overlay81Trigger *trigger;
    u8 pad4C[0x18];
    Overlay81State *state;
    u8 pad68[0x10];
    Overlay81Collision *collision;
    u8 pad7C[4];
    s32 flags;
} Overlay81Object;

extern s32 gOverlay81Mask;

void overlay81SpawnEffectReloc(s32 effectId, s32 x, s32 y, s32 z,
                               s32 mode, s32 arg5);
void overlay81ActivateReloc(Overlay81Object *object, s32 mode);
s32 overlay81RandomRangeReloc(s32 minimum, s32 maximum);

void overlay81Update(Overlay81Object *object, s32 updateRate) {
    Overlay81State *state;
    f32 savedY;
    Overlay81Collision *collision;
    u8 active;

    state = object->state;
    if (state->mask & gOverlay81Mask) {
        if (state->timer != 0) {
            state->timer -= updateRate;
            if (state->timer <= 0) {
                state->timer = 0;
                collision = object->collision;
                collision->flags &= ~2;
            }
        }

        active = object->trigger->active;
        if ((active != 0) && (state->active == 0)) {
            overlay81SpawnEffectReloc(0x276, object->x.bits, object->y.bits,
                                      object->z.bits, 4, 0);
            active = object->trigger->active;
        } else if (active == 0) {
            collision = object->collision;
            if (collision->active != 0) {
                savedY = object->y.value;
                object->y.value -= collision->yOffset;
                object->flags |= 1;
                overlay81ActivateReloc(object, 1);
                overlay81SpawnEffectReloc(
                    overlay81RandomRangeReloc(0x22A, 0x22B) & 0xFFFF,
                    object->x.bits, object->y.bits, object->z.bits, 4, 0);
                object->y.value = savedY;
                active = object->trigger->active;
            }
        }
        if (active != 0) {
            state->active = 1;
        } else {
            state->active = 0;
        }
    }
}
