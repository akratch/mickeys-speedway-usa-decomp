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

/* THE JOINED LINE IS LOAD-BEARING. `remaining = 7; do {` shares one physical
 * line deliberately; splitting it costs the match.
 *
 * IDO hoists the loop-invariant `gOverlay40Objects` address into the loop
 * preheader and stamps the hoisted record with the *loop header's* source
 * line, not the line of the statement that uses it. The count initializer and
 * that address are independent -- no dependence edge orders them -- so as1's
 * reorganizer separates them on its minimized `lineno` key, and any
 * initializer written on an earlier line wins. Sharing the loop header's line
 * removes the separation, and the later key then reproduces the target order.
 *
 * Measured with `decomp-workbench trace-emit` on an emit-provenance
 * instrumented ugen (workbench improvement-backlog item #3(b)): block 0 emits
 * the count at line 45 and the hoisted address at line 46, one line apart and
 * adjacent in emission order. Before the join, 44/46 words with the swap at
 * +0xC/+0x10; after it, all 46 words and all four relocation records compare
 * exact. Promotion out of NON_MATCHING is the remaining step. */
void overlay40UpdateEntries(s32 amount, s32 remaining) {
    Overlay40Entry *entry;
    s8 previous;
    Overlay40Object *object;

    entry = gOverlay40Entries;
    remaining = 7; do {
        if (entry->state != -1) {
            if (entry->state < 8) {
                entry->state++;
            }
            previous = entry->scales[0];
            entry->scales[2] = entry->scales[1];
            entry->scales[0] = previous - amount;
            entry->scales[1] = previous;
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
