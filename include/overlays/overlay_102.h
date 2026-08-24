#ifndef OVERLAY_102_H
#define OVERLAY_102_H

#include "PR/ultratypes.h"

extern u32 *gOverlay102SignatureBlock;

void overlay102DmaCopyReloc(s32 size, void *address, s32 alignment);
s32 overlay102CheckSignature(void);

#endif
