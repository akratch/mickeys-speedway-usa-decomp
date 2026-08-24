#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG have no overlay-1 donor for this wrapper. */
extern void overlay1ResetReloc(void);

void overlay1CallReset(void) {
    overlay1ResetReloc();
}
