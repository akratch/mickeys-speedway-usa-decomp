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

void overlay40AddEntry(volatile s32 id, s32 red, s32 green, s32 blue) {
    Overlay40Entry *entry;
    s32 remaining;

    entry = gOverlay40Entries;
    if (gOverlay40Count < 8) {
        remaining = 7;
        do {
            if (entry->state == -1) {
                entry->state = 0;
                entry->scales[0] = 0x1E;
                entry->scales[1] = 0x1E;
                entry->scales[2] = 0x1E;
                entry->red = red;
                entry->green = green;
                entry->blue = blue;
                entry->id = id;
                gOverlay40Count++;
                return;
            }
            entry++;
        } while (remaining--);
    }
}
