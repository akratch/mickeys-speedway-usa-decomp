#include "PR/ultratypes.h"

extern s32 gOverlay66Timer;
extern s32 gOverlay66Current;
extern s32 gOverlay66Flag;

/* Pinned DKR v77/v80 and JFG scans contain no exact donor. */
void overlay66Select(s32 selection) {
    if (selection != gOverlay66Current) {
        gOverlay66Flag = 0;
        gOverlay66Current = selection;
        gOverlay66Timer = 5;
    }
}
