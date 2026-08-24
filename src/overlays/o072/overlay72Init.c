#include "PR/ultratypes.h"

/* Overlay 72 +0x000. Fresh DKR v77/v80 and JFG scans are negative. */
typedef struct {
    f32 scale;
    f32 pairedScale;
} Overlay72Component;

typedef struct {
    f32 scale;
    f32 y;
    f32 z;
} Overlay72State;

typedef struct {
    s16 angle;
    u8 pad2[4];
    s16 flags;
    u8 pad8[4];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x34];
    Overlay72Component *component;
    u8 pad50[0x14];
    Overlay72State *state;
} Overlay72Object;

typedef struct {
    u8 pad0[0x0A];
    u8 angle;
    u8 scale;
    s8 yOffset;
    s8 zOffset;
} Overlay72Config;

extern f32 gOverlay72ComponentScale;

void overlay72Init(Overlay72Object *object, Overlay72Config *config) {
    Overlay72State *state = object->state;
    Overlay72Component *component;

    object->flags |= 0x800;
    state->scale = config->scale * 1.125f;
    state->y = object->y + config->yOffset;
    state->z = object->y + config->zOffset;
    object->angle = config->angle << 8;
    component = object->component;
    if (component != 0) {
        component->scale = state->scale * gOverlay72ComponentScale;
        component = object->component;
        component->pairedScale = component->scale;
    }
}
