#include "PR/ultratypes.h"

typedef struct Overlay59Node {
    s32 active;
    s32 includeSecond;
    s32 value8;
    s16 valueC;
    s16 valueE;
} Overlay59Node;

typedef struct Overlay59Entry {
    u8 pad00[0x20];
    Overlay59Node *nodes;
    s32 values[8];
} Overlay59Entry;

typedef struct Overlay59Output {
    s32 first;
    s32 second;
    s32 value8;
    s16 valueC;
    s16 valueE;
} Overlay59Output;

extern Overlay59Entry gOverlay59Entries[4];

/* DKR v77/v80 and JFG contain no exact donor for this list projection. */
void overlay59BuildList(s32 index, Overlay59Output *output) {
    Overlay59Entry *entry;
    Overlay59Node *node;
    s32 *values;

    if (index >= 0 && index < 4) {
        entry = &gOverlay59Entries[index];
        node = entry->nodes;
        if (node != NULL && node->active != 0) {
            values = (s32 *) entry;
            do {
                output->first = values[9];
                values++;
                if (node->includeSecond != 0) {
                    output->second = values[9];
                    values++;
                } else {
                    output->second = 0;
                }
                output->value8 = node->value8;
                output->valueC = node->valueC;
                output->valueE = node->valueE;
                output++;
                node++;
            } while (node->active != 0);
        }
    }
    output->first = 0;
}
