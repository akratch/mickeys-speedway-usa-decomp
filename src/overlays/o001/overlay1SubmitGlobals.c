#include "PR/ultratypes.h"

/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern s32 gOverlay1SubmitArg2;
extern s32 gOverlay1SubmitArg3;
extern void overlay1SubmitReloc(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

void overlay1SubmitGlobals(s32 arg0, s32 arg1) {
    overlay1SubmitReloc(arg0, arg1, gOverlay1SubmitArg2, gOverlay1SubmitArg3);
}
