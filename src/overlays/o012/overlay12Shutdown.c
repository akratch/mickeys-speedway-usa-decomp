#include "PR/ultratypes.h"

typedef struct Overlay12Entry {
    u8 pad00[0x4A];
    u8 active;
    u8 pad4B[9];
} Overlay12Entry;

extern s32 gOverlay12Count;
extern s32 gOverlay12Selection;
extern void *gOverlay12Resource0;
extern void *gOverlay12Resource1;
extern void *gOverlay12Resource2;
extern void *gOverlay12Resource3;
extern void *gOverlay12Resource4;
extern void *gOverlay12Resource5;
extern Overlay12Entry gOverlay12Entries[];
extern u8 gOverlay12Flag1536;
extern s32 gOverlay12Ready;
extern void overlay12ReleaseResource(void *resource);
extern void overlay12ReleaseResourceAlt(void *resource);

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
void func_overlay_012_F00000C4_186D344(void) {
    s32 i;
    Overlay12Entry *entry;

    if (gOverlay12Ready != 0) {
        if (gOverlay12Resource0 != 0) {
            overlay12ReleaseResource(gOverlay12Resource0);
        }
        if (gOverlay12Resource1 != 0) {
            overlay12ReleaseResource(gOverlay12Resource1);
        }
        if (gOverlay12Resource2 != 0) {
            overlay12ReleaseResource(gOverlay12Resource2);
        }
        if (gOverlay12Resource3 != 0) {
            overlay12ReleaseResource(gOverlay12Resource3);
        }
        if (gOverlay12Resource4 != 0) {
            overlay12ReleaseResource(gOverlay12Resource4);
        }
        if (gOverlay12Resource5 != 0) {
            overlay12ReleaseResourceAlt(gOverlay12Resource5);
        }

        entry = gOverlay12Entries;
        i = 0;
        do {
            i++;
            entry++;
            entry[-1].active = 0;
        } while (i < 64);
        if (!i) {
        }
        gOverlay12Flag1536 = 0;
    }

    gOverlay12Ready = 0;
    gOverlay12Count = 0;
    gOverlay12Selection = 0;
}
