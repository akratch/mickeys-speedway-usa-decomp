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

/*
 * Exact reconstruction (2026-08-28). The original
 * -O2 -mips2 body had the correct 0x84-byte shape but a four-site a0/v1
 * pool-color swap starting at +0x20. Seven explicit loop-counter lifetime
 * and declaration variants left that allocation unchanged. The retained
 * output-20-1 permuter source adds constant intermediates and a no-op green
 * expression, reaching the target register allocation.
 *
 * The real-TU object is 33 instructions (0x84 bytes) with four relocations:
 * gOverlay40Count HI16/LO16 at 0x00/0x04 and gOverlay40Entries HI16/LO16 at
 * 0x0C/0x1C. `tools/wb_compare.sh --rom overlay40AddEntry` reports identical
 * linked instruction words and relocation-kind layout; `gmake verify`
 * reproduces the expected US ROM hash.
 */
void overlay40AddEntry(volatile s32 id, s32 red, s32 green, s32 blue) {
    int new_var;
    Overlay40Entry *entry;
    int new_var2;
    s32 remaining;

    entry = gOverlay40Entries;
    new_var = 0;
    if (gOverlay40Count < 8) {
        green += (((((new_var & 0xFFFFFFFFu) & 0xFFFFFFFFu) & 0xFFFFFFFFu) & 0xFFFFFFFFu) & 0xFFFFFFFFu) & 0xFFFFFFFFu;
        new_var2 = 0x1E;
        remaining = 7;
        do {
            if (entry->state == -1) {
                entry->state = new_var;
                entry->scales[new_var] = 0x1E;
                entry->scales[1] = new_var2;
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
