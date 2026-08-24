#include "PR/ultratypes.h"

extern u8 gOverlay55ResourceA[];
extern u8 gOverlay55ResourceB[];
extern void overlay55ReleaseReloc(void *resource);
extern void overlay55FinalizeReloc(void);

/* DKR v77/v80 and JFG checks found only generic resource cleanup. */
void overlay55ReleaseAll(void) {
    overlay55ReleaseReloc(gOverlay55ResourceA);
    overlay55ReleaseReloc(gOverlay55ResourceB);
    overlay55FinalizeReloc();
}
