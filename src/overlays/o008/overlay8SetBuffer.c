#include "PR/ultratypes.h"

extern void *gOverlay8Buffer;

void overlay8SetBuffer(void *base) {
    gOverlay8Buffer = (u8 *) base + 0x1B8;
}
