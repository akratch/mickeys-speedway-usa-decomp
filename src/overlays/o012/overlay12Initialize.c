#include "PR/ultratypes.h"

typedef struct Overlay12Entry {
    u8 pad00[0x4A];
    u8 active;
    u8 pad4B[9];
} Overlay12Entry;

extern void *gOverlay12ResourceF3;
extern void *gOverlay12ResourceF2;
extern void *gOverlay12ResourceF4;
extern void *gOverlay12ResourceF5;
extern void *gOverlay12ResourceF6;
extern void *gOverlay12Resource39;
extern Overlay12Entry gOverlay12Entries[];
extern u8 gOverlay12Flag1536;
extern s32 gOverlay12Ready;
extern s32 gOverlay12Count;
extern s32 gOverlay12Selection;
extern s32 gOverlay12Value1598;

void *overlay12LoadReloc();

/* DKR v77/v80 and JFG contain no exact donor for this resource initializer. */
void overlay12Initialize(void) {
    s32 remaining;
    Overlay12Entry *entry;

    gOverlay12ResourceF3 = overlay12LoadReloc(0xF3);
    gOverlay12ResourceF2 = overlay12LoadReloc(0xF2);
    gOverlay12ResourceF4 = overlay12LoadReloc(0xF4);
    gOverlay12ResourceF5 = overlay12LoadReloc(0xF5);
    gOverlay12ResourceF6 = overlay12LoadReloc(0xF6);
    gOverlay12Resource39 = overlay12LoadReloc(0x39, 1);

    entry = gOverlay12Entries;
    remaining = 0;
    do {
        remaining++;
        entry++;
        entry[-1].active = 0;
    } while (remaining < 0x40);
    if (!remaining) {
    }

    gOverlay12Flag1536 = 0;
    gOverlay12Ready = 1;
    gOverlay12Count = 0;
    gOverlay12Selection = 0;
    remaining = 0;
    gOverlay12Value1598 = remaining;
}
