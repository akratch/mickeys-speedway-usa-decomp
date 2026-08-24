#include "PR/ultratypes.h"
typedef struct W { u8 p0[0xD4]; void *object; u8 pD8[0xC3]; u8 enabled; s32 state; } W;
extern W *D_1DA0[];
extern void *D_1D9C[];
extern s32 D_83E4;
extern s32 overlay27CanUse(void *);
extern s32 overlay3RunCachedModeAction(void *, W *);
extern s32 overlay1DispatchMode(void);
#ifdef NON_MATCHING
s32 overlay1HandleCachedMode(void) {
    W *world = D_1DA0[0];
    s32 result = 0;
    if (world->enabled == 0) goto clear;
    if (overlay27CanUse(world->object) != 0) goto clear;
    if (D_83E4 == 3) result = overlay3RunCachedModeAction(D_1D9C[0], D_1DA0[0]);
    else result = overlay1DispatchMode();
    return result;
clear:
    D_1DA0[0]->state = 0;
    return result;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1HandleCachedMode/func_overlay_001_F00061F0_18525D0.s")
#endif
