#include "PR/ultratypes.h"

typedef struct Overlay40Entry {
    s8 state;
    s8 scales[3];
    u8 red;
    u8 green;
    u8 blue;
    u8 id;
} Overlay40Entry;

extern s32 gOverlay40Count;
extern Overlay40Entry gOverlay40Entries[8];

void overlay40RemoveEntry(s32 id, s32 savedId) {
    register Overlay40Entry *entry;

    /* The second formal is overwritten before use and preserves IDO's ABI web. */
    savedId = id;
    entry = gOverlay40Entries;
    if (gOverlay40Count > 0) {
        id = 7;
        do {
            if (entry->state != -1 && entry->id == savedId) {
                entry->state = -1;
                gOverlay40Count--;
            }
            entry++;
        } while (id--);
    }
}
