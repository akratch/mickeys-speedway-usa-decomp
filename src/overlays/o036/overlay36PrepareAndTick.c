#include "ultra64.h"

typedef struct Overlay36PrepareObject {
    u8 pad000;
    s8 kind;
    u8 pad002[0x198];
    u8 state;
    u8 countdown;
    u8 pad19C[4];
    s32 owner;
    u8 pad1A4[4];
    u16 flags;
} Overlay36PrepareObject;

typedef struct Overlay36PrepareSource {
    u8 pad00[0x64];
    Overlay36PrepareObject *object;
} Overlay36PrepareSource;

extern void overlay36ActionReloc();

/* Title-specific countdown/action dispatch; no DKR v77/v80 or JFG donor. */
void overlay36PrepareAndTick(Overlay36PrepareSource *source) {
    Overlay36PrepareObject *object;

    overlay36ActionReloc(source, 1);
    object = source->object;
    object->countdown--;
    if (object->countdown == 0) {
        object->state = 255;
        object->owner = 0;
    }
    if (!(object->flags & 1)) {
        if (object->kind != 6 && object->kind != 7 && object->kind != 8) {
            overlay36ActionReloc(6);
        }
    }
}
