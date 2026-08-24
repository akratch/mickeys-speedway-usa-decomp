#include "PR/ultratypes.h"

typedef struct Overlay4Object {
    u8 reserved00[0xC];
    f32 x;
    u8 reserved10[4];
    f32 z;
    u8 reserved18[0x2C];
    s16 type;
    u8 reserved46[0x1E];
    volatile s32 *state;
} Overlay4Object;

typedef struct Overlay4SearchKey {
    s8 identifier;
    u8 reserved001[0x38B];
    u8 mode;
} Overlay4SearchKey;

extern f32 gOverlay4SearchMaxDistance;
extern Overlay4Object **overlay4GetObjectRangeReloc(s32 *start, s32 *end);
extern Overlay4Object *overlay4FindCategory2Object(Overlay4SearchKey *key);

void overlay4FindSearchPosition(f32 *outX, f32 *outZ,
                                Overlay4SearchKey *key,
                                Overlay4Object *anchor) {
    Overlay4Object **objects;
    Overlay4Object *object;
    Overlay4Object *best;
    volatile s32 *state;
    f32 dx;
    f32 dz;
    f32 distance;
    f32 bestDistance;
    s32 start;
    s32 end;
    s32 i;

    objects = overlay4GetObjectRangeReloc(&start, &end);
    bestDistance = gOverlay4SearchMaxDistance;
    if (key->mode == 1) {
        best = NULL;
        i = start;
        while (i < end) {
            object = objects[i++];
            if (object->type == 0x21) {
                state = object->state;
                if (*state == 0) {
                    dx = anchor->x - object->x;
                    dz = anchor->z - object->z;
                    distance = (dx * dx) + (dz * dz);
                    if (*state != 0) {
                        distance *= 4.0f;
                    }
                    if (distance < bestDistance) {
                        bestDistance = distance;
                        best = object;
                    }
                }
            }
        }
        if (best != NULL) {
            *outX = best->x;
            *outZ = best->z;
            return;
        }
    } else {
        best = overlay4FindCategory2Object(key);
        if (best != NULL) {
            *outX = best->x;
            *outZ = best->z;
            return;
        }
    }
    *outX = 0.0f;
    *outZ = 0.0f;
}
