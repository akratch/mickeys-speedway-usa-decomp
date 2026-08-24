#include "PR/ultratypes.h"

typedef struct Overlay4SearchPayload {
    u8 reserved00[4];
    s32 category;
    u8 identifier;
} Overlay4SearchPayload;

typedef struct Overlay4Object {
    u8 reserved00[0x44];
    s16 type;
    u8 reserved46[0x1E];
    Overlay4SearchPayload *payload;
} Overlay4Object;

extern Overlay4Object **overlay4GetObjectRangeReloc(s32 *start, s32 *end);

Overlay4Object *overlay4FindCategory2Object(s8 *identifier) {
    s32 start;
    s32 end;
    Overlay4Object **objects;
    Overlay4Object *object;
    Overlay4SearchPayload *payload;
    s32 wantedIdentifier;
    s32 i;

    objects = overlay4GetObjectRangeReloc(&start, &end);
    i = start;
    while (i < end) {
        object = objects[i++];
        if (object->type == 0x30) {
            payload = object->payload;
            if (payload->category == 2) {
                wantedIdentifier = *identifier;
                if (wantedIdentifier == payload->identifier) {
                    return object;
                }
            }
        }
    }
    return NULL;
}
