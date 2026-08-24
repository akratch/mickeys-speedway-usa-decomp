#include "PR/ultratypes.h"

typedef struct Overlay62Root {
    u8 pad0[0x25C];
    void *value;
} Overlay62Root;

extern f32 gOverlay62Float0;
extern s32 gOverlay62Value4Initial;
extern s32 gOverlay62Value4;
extern s32 gOverlay62Value8;
extern s32 gOverlay62ValueC;
extern s32 gOverlay62Value10;
extern void *gOverlay62Handle14;
extern void *gOverlay62Handle18;
extern Overlay62Root *gOverlay62Root;

extern void overlay62SetupReloc(void *);
extern void overlay62SetModeReloc(s32);
extern void overlay62SetColorReloc(s32, s32, s32, s32, s32);
extern void *overlay62CreateReloc(void *, s32, s32, s32);
extern void *overlay62LoadReloc(s32, s32);
extern void overlay62CommitReloc(void *, s32);

/* DKR v77/v80 and JFG have no exact donor for this resource initializer. */
void overlay62Initialize(void) {
    overlay62SetupReloc(&gOverlay62Float0);
    gOverlay62Value4Initial = (s32)gOverlay62Handle18;
    gOverlay62Float0 = 0.0f;
    gOverlay62Value4 = 13;
    overlay62SetModeReloc(3);
    overlay62SetColorReloc(0xFF, 0xFF, 0xFF, 0, 0xFF);
    gOverlay62Handle14 = overlay62CreateReloc(
        gOverlay62Root->value, 0x28, 0x28, 0);
    gOverlay62Handle18 = overlay62LoadReloc(0x93, 0);
    gOverlay62Value8 = 0x20;
    gOverlay62ValueC = 0;
    gOverlay62Value10 = 0;
    overlay62CommitReloc(gOverlay62Handle14, 0);
}
