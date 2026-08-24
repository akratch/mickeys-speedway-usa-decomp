#include "PR/ultratypes.h"

typedef struct Overlay11State {
    u8 mode;
} Overlay11State;

extern s32 gOverlay11GroupsActive;
extern s32 gOverlay11Variant;
extern Overlay11State *overlay11GetState(void);
extern void overlay11ReleaseGroup4(void);
extern void overlay11ReleaseGroup3A(void);
extern void overlay11ReleaseGroup6A(void);
extern void overlay11ReleaseGroup6B(void);
extern void overlay11ReleaseGroup6C(void);
extern void overlay11ReleaseGroup3B(void);

/* DKR v77/v80 contain no matching state-to-release dispatcher. */
#ifdef NON_MATCHING
void overlay11ReleaseCurrentGroup(void) {
    Overlay11State *state;

    if (gOverlay11GroupsActive != 0) {
        state = overlay11GetState();
        switch (state->mode) {
        case 0:
            overlay11ReleaseGroup4();
            break;
        case 1:
            overlay11ReleaseGroup6A();
            break;
        case 2:
            overlay11ReleaseGroup3A();
            break;
        case 3:
            if (gOverlay11Variant == 1) {
                overlay11ReleaseGroup6C();
            } else {
                overlay11ReleaseGroup6B();
            }
            break;
        case 4:
        case 5:
            overlay11ReleaseGroup3B();
            break;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o011/overlay11ReleaseCurrentGroup/func_overlay_011_F0002BF4_186B43C.s")
#endif
