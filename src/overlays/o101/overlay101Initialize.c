#include "PR/ultratypes.h"

extern s32 gOverlay101ConfigEnabled;
extern s32 gOverlay101CurrentConfig;
extern s32 gOverlay101ConfigValue;
extern s32 gOverlay101CurrentResource;
extern s32 gOverlay101Ready;

extern void overlay101ConfigureReloc(s32 mode, s32 arg1, s32 arg2, s32 value,
                                     s32 enabled, s32 arg5);
extern void overlay101SetScaleReloc(f32 scale, s32 arg1);
extern void overlay101SelectDefaultReloc(s32 mode);
extern void overlay101ActivateResourceReloc(s32 resource);
extern void overlay101FinalizeReloc(void);

/* Pinned DKR v77/v80 and JFG scans classify overlay 101 as no donor. */
void overlay101Initialize(void) {
    if (gOverlay101ConfigEnabled > 0) {
        overlay101ConfigureReloc(1, 0, 0, 15, 1, 0);
        return;
    }

    overlay101SetScaleReloc(0.5f, 0);
    if (gOverlay101CurrentConfig == -1) {
        overlay101SelectDefaultReloc(1);
    } else {
        overlay101ConfigureReloc(gOverlay101CurrentConfig, 0, 0,
                                 gOverlay101ConfigValue, 1, 0);
        overlay101ActivateResourceReloc(gOverlay101CurrentResource);
    }
    gOverlay101Ready = 0;
    overlay101FinalizeReloc();
}
