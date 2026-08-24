#include "PR/ultratypes.h"

typedef struct Overlay4Group {
    void *objects[32];
    s32 count;
} Overlay4Group;

typedef struct Overlay4Header {
    s8 group;
} Overlay4Header;

typedef struct Overlay4State {
    u8 pad00[0x64];
    Overlay4Header *header;
} Overlay4State;

typedef struct Overlay4Link {
    Overlay4State *state;
} Overlay4Link;

typedef struct Overlay4Object {
    u8 pad00[0x64];
    Overlay4Link *link;
} Overlay4Object;

extern Overlay4Group gOverlay4Groups[];

/* DKR v77/v80 and JFG contain no exact donor for this group-list removal. */
void overlay4RemoveObject(Overlay4Object *object) {
    Overlay4Group *group;
    Overlay4Link *link;
    Overlay4State *state;
    Overlay4Header *header;
    s32 index;
    s32 shiftIndex;

    link = object->link;
    state = link->state;
    header = state->header;
    group = &gOverlay4Groups[header->group];
    index = group->count;
    if (index--) {
        do {
            if (group->objects[index] == object) {
                break;
            }
        } while (index--);
    }
    group->count--;
    shiftIndex = index;
    while (shiftIndex < group->count) {
        group->objects[shiftIndex] = group->objects[shiftIndex + 1];
        shiftIndex++;
    }
}
