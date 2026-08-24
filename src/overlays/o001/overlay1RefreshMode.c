#include "PR/ultratypes.h"

typedef struct Overlay1ModeObject {
    u8 pad00[0x38C];
    u8 mode;
} Overlay1ModeObject;

extern void *gOverlay1ModeSource;
extern Overlay1ModeObject *gOverlay1ModeObject;
extern s32 overlay1ReadModeReloc(void *source);

/* DKR v77/v80 and JFG have no exact donor for this mode refresh wrapper. */
void overlay1RefreshMode(s32 arg0, s32 arg1, s32 arg2) {
    if (overlay1ReadModeReloc(gOverlay1ModeSource) >= 3) {
        gOverlay1ModeObject->mode = 2;
    } else {
        gOverlay1ModeObject->mode = 1;
    }
    overlay1ReadModeReloc(gOverlay1ModeSource);
}
