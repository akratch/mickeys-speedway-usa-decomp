#include "PR/ultratypes.h"

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern void overlay1GlobalCallReloc();
extern void *gOverlay1SubmitArg4;

void overlay1CallGlobal(s32 unused0, s32 unused1, s32 unused2) {
    overlay1GlobalCallReloc(gOverlay1SubmitArg4);
}
