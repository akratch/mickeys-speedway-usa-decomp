#include "PR/ultratypes.h"

/* Overlay 93 +0x000; exact donor scans are negative. */

typedef struct {
    u8 pad0[0x0A];
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    s16 arg5;
} Overlay93Config;

typedef struct {
    u8 pad0[0x0C];
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    s16 arg5;
} Overlay93UpdateConfig;

typedef struct {
    u8 pad0[0x0C];
    f32 x;
    u8 pad10[4];
    f32 z;
    u8 pad18[0x24];
    Overlay93UpdateConfig *config;
    u8 pad40[0x44];
    f32 radiusSquared;
} Overlay93Object;

Overlay93Object *overlay93FindTargetReloc(s32 type, Overlay93Object *object);
void overlay93EmitReloc(s32 x, s32 y, s32 z, s32 red, s32 green, s32 arg5);

void overlay93Init(Overlay93Object *object, Overlay93Config *config) {
    f32 radius = (f32)config->x;
    radius *= radius;
    object->radiusSquared = radius;
}

void overlay93Update(Overlay93Object *object, f32 unused) {
    Overlay93Object *target = overlay93FindTargetReloc(0, object);

    if (target != 0) {
        f32 dx = object->x - target->x;
        f32 radiusSquared = object->radiusSquared;
        Overlay93UpdateConfig *config = object->config;
        f32 dz = object->z - target->z;

        if ((dx * dx) + (dz * dz) <= radiusSquared) {
            overlay93EmitReloc(
                config->x << 8,
                config->y << 8,
                config->z << 8,
                config->red * 0x101,
                config->green * 0x101,
                config->arg5
            );
        }
    }
}
