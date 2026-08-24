#include "PR/ultratypes.h"

/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern s32 gOverlay1SubmitArg2;
extern s32 gOverlay1SubmitArg3;
extern s32 gOverlay1SubmitArg4;
extern s32 gOverlay1SubmitArg5;
extern void overlay1SubmitAllReloc(s32 arg0, s32 arg1, s32 arg2, s32 arg3,
                                   s32 arg4, s32 arg5);

void overlay1SubmitAll(s32 arg0, s32 arg1) {
    overlay1SubmitAllReloc(arg0, arg1, gOverlay1SubmitArg2,
                           gOverlay1SubmitArg3, gOverlay1SubmitArg4,
                           gOverlay1SubmitArg5);
}
