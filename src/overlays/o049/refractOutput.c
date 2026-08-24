#include "PR/ultratypes.h"

/*
 * Overlay 49 +0x354. JFG overlay 2 supplies the exact 0x20-byte function body
 * and name. Its three trailing alignment nops remain generated assembly and
 * receive no C credit. Mickey's call relocation targets overlay 65 +0xBC0.
 */
void overlay49RefractOutputReloc(void);

void refractOutput(void) {
    overlay49RefractOutputReloc();
}
