#include "PR/ultratypes.h"

extern s32 gOverlay14FlagC4;
extern s32 gOverlay14FlagCC;

void overlay14ResetFlags(void) {
    gOverlay14FlagC4 = 0;
    gOverlay14FlagCC = 0;
}
