#include "PR/ultratypes.h"

/* Generic scalar predicates only; pinned DKR objects provide no donor. */
extern s32 gOverlay1Mode;

s32 overlay1ModeIsOne(void) { return gOverlay1Mode == 1; }
s32 overlay1ModeIsTwo(void) { return gOverlay1Mode == 2; }
s32 overlay1ModeIsThree(void) { return gOverlay1Mode == 3; }
