#include "ultra64.h"

/*
 * Overlay 68 +0x000. Fresh DKR v77/v80 source searches for 0x2EF0 and
 * payload-limit/copy-clamp shapes were negative.
 */
s32 overlay68PayloadLimit(void) {
    return 0x2EF0;
}
