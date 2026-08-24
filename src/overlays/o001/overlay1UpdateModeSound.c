#include "PR/ultratypes.h"

typedef struct Overlay1ModeObject {
    u8 pad0[0x193];
    u8 mode;
    u8 pad194[0x14];
    u16 flags;
} Overlay1ModeObject;

extern Overlay1ModeObject *gOverlay1ModeObject;
extern void *gOverlay1ModeResource;
extern void overlay1ModeSoundReloc(void *, s32);

/* DKR v77/v80 and JFG contain no exact donor for this mode-gated wrapper. */
void overlay1UpdateModeSound(void) {
    Overlay1ModeObject *object;

    object = gOverlay1ModeObject;
    if (object->mode == 13) {
        if (object->flags & 2) {
            overlay1ModeSoundReloc(gOverlay1ModeResource, 0x78);
        }
    } else {
        overlay1ModeSoundReloc(gOverlay1ModeResource, 0x78);
    }
}
