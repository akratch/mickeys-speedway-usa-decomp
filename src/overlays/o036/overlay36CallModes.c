#include "PR/ultratypes.h"

/* Generic mode wrappers only; no exact DKR donor. */
extern void overlay36CallModeReloc(void *object, s32 mode);

void overlay36CallModeZero(void *object) { overlay36CallModeReloc(object, 0); }
void overlay36CallModeOne(void *object) { overlay36CallModeReloc(object, 1); }
