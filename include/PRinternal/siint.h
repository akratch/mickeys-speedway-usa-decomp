#ifndef _SIINT_H_
#define _SIINT_H_

/*
 * The serial-interface access lock and raw DMA, the SI counterpart of the PI
 * pair in PRinternal/piint.h.
 *
 * PROVENANCE: names follow the N64 SDK internal header as published in public
 * decomp trees (JFG's `include/PRinternal/siint.h`), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 1.3.
 */

#include "PR/ultratypes.h"

void __osSiGetAccess(void);
void __osSiRelAccess(void);
s32 __osSiRawStartDma(s32 direction, void *dramAddr);

#endif /* _SIINT_H_ */
