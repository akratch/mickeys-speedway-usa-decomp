#include "PR/ultratypes.h"

extern s16 *gOverlay8Buffer;

void overlay8WriteCommand(volatile s32 unused) {
    *gOverlay8Buffer = 0x2000;
    gOverlay8Buffer++;
}
