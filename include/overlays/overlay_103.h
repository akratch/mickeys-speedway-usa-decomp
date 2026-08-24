#ifndef OVERLAY_103_H
#define OVERLAY_103_H

#include "PR/ultratypes.h"

extern u32 *gOverlay103SignatureBlock;

void overlay103DmaCopyReloc(s32 size, void *address, s32 alignment);
s32 overlay103CheckSignature(void);

#endif
