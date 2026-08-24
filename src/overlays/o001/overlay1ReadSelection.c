#include "PR/ultratypes.h"

typedef struct Overlay1Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay1Vector;

typedef struct Overlay1Descriptor {
    u8 pad00[0x2D];
    u8 count;
} Overlay1Descriptor;

typedef struct Overlay1Selection {
    Overlay1Descriptor *descriptor;
    u8 pad04[4];
    s16 useObjectPosition;
    u8 pad0A[0x36];
    Overlay1Vector *vectors;
} Overlay1Selection;

typedef struct Overlay1Object {
    u8 pad00[0x0C];
    Overlay1Vector position;
    u8 pad18[0x22];
    s8 selectedIndex;
    u8 pad3B[0x0D];
    u8 *fallback;
    u8 pad4C[0x1C];
    Overlay1Selection **selections;
} Overlay1Object;

void overlay1ReadSelection(Overlay1Object *object, s32 index, f32 *outX,
                           f32 *outY, f32 *outZ) {
    Overlay1Selection *selection;
    Overlay1Descriptor *descriptor;
    Overlay1Vector *vectors;
    s32 offset;

    selection = object->selections[object->selectedIndex];
    if (selection != 0) {
        descriptor = selection->descriptor;
        if (selection->useObjectPosition != 0) {
            *outX = object->position.x;
            *outY = object->position.y;
            *outZ = object->position.z;
            return;
        }
        if (descriptor->count >= index) {
            vectors = selection->vectors;
            offset = index * sizeof(Overlay1Vector);
            if (vectors == 0) {
                return;
            }
            *outX = *(f32 *)((u8 *)vectors + offset + 0);
            *outY = *(f32 *)((u8 *)selection->vectors + offset + 4);
            *outZ = *(f32 *)((u8 *)selection->vectors + offset + 8);
            return;
        }
    }

    *outX = *(f32 *)(object->fallback + 0x24);
    *outY = *(f32 *)(object->fallback + 0x28);
    *outZ = *(f32 *)(object->fallback + 0x2C);
}
