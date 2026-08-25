#include "PR/ultratypes.h"

typedef struct Overlay40Entry {
    s8 state;
    s8 scales[3];
    u8 red;
    u8 green;
    u8 blue;
    u8 id;
} Overlay40Entry;

typedef struct Overlay40Node {
    u8 pad00[0x64];
    void *effect64;
} Overlay40Node;

typedef struct Overlay40Object {
    u8 pad00[8];
    Overlay40Node *node;
} Overlay40Object;

extern Overlay40Entry gOverlay40Entries[8];
extern Overlay40Object **gOverlay40Objects;

/*
 * Plateau: the best natural -O2 -mips2 candidate is one word long before
 * trim, with 44 masked differences starting at +0x08. IDO copies the live
 * amount parameter from a0 to a2, cascading the loop schedule and allocation;
 * the flag lattice and bounded permuter found no exact source spelling.
 */
#ifdef NON_MATCHING
void overlay40UpdateEntries(s32 amount, s32 remaining) {
    Overlay40Entry *entry;
    Overlay40Object *object;
    s32 previous;
    s32 older;

    entry = gOverlay40Entries;
    remaining = 7;
    do {
        if (entry->state != -1) {
            if (entry->state < 8) {
                entry->state++;
            }
            previous = entry->scales[0];
            older = entry->scales[1];
            entry->scales[0] = previous - amount;
            entry->scales[1] = previous;
            entry->scales[2] = older;
            if (entry->scales[0] < 0) {
                entry->scales[0] = 0;
            }
            object = gOverlay40Objects[entry->id];
            if (object != 0 && object->node != 0) {
                if (entry->state & 1) {
                    object->node->effect64 = &entry->red;
                } else {
                    object->node->effect64 = 0;
                }
            }
        }
        entry++;
    } while (remaining--);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o040/overlay40UpdateEntries/func_overlay_040_F00000E8_1886998.s")
#endif
