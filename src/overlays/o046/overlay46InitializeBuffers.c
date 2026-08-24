#include "PR/ultratypes.h"

typedef struct Overlay46State {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
    s16 value10;
    u8 pad12[2];
    s32 value14;
} Overlay46State;

typedef struct Overlay46ColorEntry {
    u8 pad0[6];
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay46ColorEntry;

typedef struct Overlay46BufferSlot {
    u8 pad00[0x14];
    s16 value14;
    u8 pad16[0x16];
    s16 value2C;
    u8 pad2E[0x16];
    s16 value44;
    u8 pad46[0x16];
    s16 value5C;
    u8 pad5E[2];
} Overlay46BufferSlot;

extern Overlay46State *gOverlay46State;
extern Overlay46State gOverlay46StateStorage;
extern f32 gOverlay46InitialX;
extern f32 gOverlay46InitialY;
extern f32 gOverlay46InitialW;
extern Overlay46ColorEntry gOverlay46ColorEntries[];
extern s16 gOverlay46InitialValue;
extern s16 gOverlay46Value;
extern Overlay46BufferSlot gOverlay46BufferSlots[];
extern Overlay46BufferSlot gOverlay46BufferSlotsEnd[];
extern s16 gOverlay46BufferEnd;

/* Pinned DKR v77/v80 and JFG searches found no exact donor. */
void overlay46InitializeBuffers(void) {
    Overlay46ColorEntry *color;
    Overlay46BufferSlot *slot;
    s32 i;

    gOverlay46State = &gOverlay46StateStorage;
    gOverlay46State->value10 = 0;
    gOverlay46State->w = gOverlay46InitialW;
    gOverlay46State->x = gOverlay46InitialX;
    gOverlay46State->y = gOverlay46InitialY;
    gOverlay46State->z = -300.0f;
    gOverlay46State->value14 = 0;

    color = gOverlay46ColorEntries;
    i = 0;
    while (i < 400) {
        i++;
        color->red = 0xFF;
        color->green = 0xFF;
        color->blue = 0xFF;
        color->alpha = 0xFF;
        color++;
    }
    i = 0;

    gOverlay46Value = gOverlay46InitialValue;

    slot = &gOverlay46BufferSlots[0]; do {
        slot->value2C = -1;
        slot->value44 = -1;
        slot->value5C = -1;
        slot->value14 = -1;
        slot++;
    } while (slot != &gOverlay46BufferSlotsEnd[0]);

    gOverlay46BufferEnd = 0;
}
