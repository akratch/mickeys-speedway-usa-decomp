#include "ultra64.h"

extern s32 gOverlay61Counter98;
extern s32 gOverlay61Counter9C;
extern s32 gOverlay61CounterA0;

/*
 * Overlay 61 +0x1C0. Fresh DKR v77/v80 searches for the matching consecutive
 * +0x98/+0x9C/+0xA0 zeroing helper were negative.
 */
void overlay61ResetCounters(void) {
    gOverlay61Counter98 = 0;
    gOverlay61Counter9C = 0;
    gOverlay61CounterA0 = 0;
}
