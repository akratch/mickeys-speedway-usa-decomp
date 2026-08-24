#include "PR/ultratypes.h"

extern s32 gOverlay14TransitionValue;
extern s32 gOverlay14FlagC4;
extern s32 gOverlay14FlagC8;
extern s32 gOverlay14ModeE4;
extern s32 gOverlay14EnabledF8;
extern s32 gOverlay14CommandHeader;

extern void overlay14InitializeReloc(void);
extern void overlay14SetActiveReloc(s32 active);
extern void overlay14PrepareReloc(s32 step);
extern void overlay14AdvanceReloc(s32 step);
extern void overlay14RetreatReloc(s32 step);
extern void overlay14DrawPrimaryReloc(s32 context);
extern void overlay14DrawAlternateReloc(s32 context);

/* Pinned DKR v77/v80 and JFG scans found no exact donor for this body. */
#ifdef NON_MATCHING
void overlay14UpdateTransition(s32 context, s32 step) {
    s32 raised;
    s32 lowered;

    overlay14PrepareReloc(step);
    if ((gOverlay14FlagC4 != 0) && (gOverlay14FlagC8 == 0)) {
        if ((gOverlay14ModeE4 != 1) && (gOverlay14EnabledF8 != 0)) {
            overlay14AdvanceReloc(step);
        } else if (gOverlay14CommandHeader != 0) {
            overlay14RetreatReloc(step);
        }
        raised = gOverlay14TransitionValue + (step * 0x10);
        gOverlay14TransitionValue = raised;
        if (raised >= 0x101) {
            gOverlay14TransitionValue = 0x100;
        }
    } else if (gOverlay14TransitionValue > 0) {
        lowered = gOverlay14TransitionValue - (step * 0x10);
        gOverlay14TransitionValue = lowered;
        if (lowered <= 0) {
            if (gOverlay14FlagC8 == 0) {
                overlay14InitializeReloc();
            } else {
                gOverlay14TransitionValue = 0;
            }
        }
    }
    overlay14SetActiveReloc(1);
    if (gOverlay14TransitionValue != 0) {
        if (gOverlay14ModeE4 != 1) {
            overlay14DrawPrimaryReloc(context);
        } else {
            overlay14DrawAlternateReloc(context);
        }
    }
    overlay14SetActiveReloc(0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o014/overlay14UpdateTransition/func_overlay_014_F0001184_1870A5C.s")
#endif
