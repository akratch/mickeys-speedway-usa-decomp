#include "PR/ultratypes.h"

typedef struct Overlay46Header {
    f32 x;
    f32 y;
    f32 z;
    f32 scale;
    s16 index;
    u8 pad12[2];
    s32 flags;
} Overlay46Header;

typedef struct Overlay46ColorEntry {
    u8 pad0[6];
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay46ColorEntry;

typedef struct Overlay46SlotGroup {
    u8 pad0[0x14];
    s16 first;
    u8 pad16[0x16];
    s16 second;
    u8 pad2E[0x16];
    s16 third;
    u8 pad46[0x16];
    s16 fourth;
    u8 pad5E[2];
} Overlay46SlotGroup;

extern Overlay46Header gOverlay46Header[];
extern volatile Overlay46Header *gOverlay46CurrentHeader;
extern f32 gOverlay46InitialX;
extern f32 gOverlay46InitialY;
extern f32 gOverlay46InitialScale;
extern Overlay46ColorEntry gOverlay46Colors[];
extern s16 gOverlay46InitialSelection;
extern s16 gOverlay46Selection;
extern Overlay46SlotGroup gOverlay46SlotGroups[];
extern Overlay46SlotGroup gOverlay46SlotGroupsEnd[];
extern s16 gOverlay46SlotGroupCount;

/* DKR v77/v80 and JFG contain no exact donor for this title-specific setup. */
void overlay46Initialize(void) {
    volatile Overlay46ColorEntry *color;
    Overlay46SlotGroup *group;
    Overlay46Header *header;
    s32 remaining;

    header = gOverlay46Header;
    gOverlay46CurrentHeader = header;
    header->index = 0;
    gOverlay46CurrentHeader->scale = gOverlay46InitialScale;
    gOverlay46CurrentHeader->x = gOverlay46InitialX;
    gOverlay46CurrentHeader->y = gOverlay46InitialY;
    gOverlay46CurrentHeader->z = -300.0f;
    gOverlay46CurrentHeader->flags = 0;

    color = gOverlay46Colors;
    remaining = 0;
    do {
        color->red = 0xFF;
        color->green = 0xFF;
        color->blue = 0xFF;
        color->alpha = 0xFF;
        color++;
        remaining++;
    } while (remaining < 0x190);

    gOverlay46Selection = gOverlay46InitialSelection;
    group = gOverlay46SlotGroups;
    do {
        group->first = -1;
        group->second = -1;
        group->third = -1;
        group->fourth = -1;
        group++;
    } while (group != gOverlay46SlotGroupsEnd);
    gOverlay46SlotGroupCount = 0;
}
