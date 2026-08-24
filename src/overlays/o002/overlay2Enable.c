#include "PR/ultratypes.h"

extern void overlay2SetEnabledReloc(void *object, s32 enabled);

/* DKR v77/v80 and JFG checks found no exact donor for this mode wrapper. */
void overlay2Enable(void *object) {
    overlay2SetEnabledReloc(object, 1);
}
