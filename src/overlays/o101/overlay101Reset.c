#include "PR/ultratypes.h"

extern void *gOverlay101Value220;
extern s32 gOverlay101Value224;
extern s32 gOverlay101Value228;

void overlay101Reset(void *value) {
    gOverlay101Value220 = value;
    gOverlay101Value224 = 0;
    gOverlay101Value228 = 0;
}
