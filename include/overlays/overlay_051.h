#ifndef OVERLAY_051_H
#define OVERLAY_051_H

#include "PR/ultratypes.h"
#include "overlays/patch_indices.h"

extern void *gOverlay51Objects[];
extern u8 gOverlay51Resource0[];
extern u8 gOverlay51Resource18[];
extern u8 gOverlay51ResourceBC[];
extern u8 gOverlay51Resource1C[];
extern u8 gOverlay51InlineResource[];
extern f32 gOverlay51InitialValue;
extern s8 gOverlay51Mode;
extern s8 gOverlay51Index;
extern s16 gOverlay51Handle;

s32 overlay51CreateReloc();
void overlay51PrepareReloc();
void overlay51ReleaseReloc(void *resource);
void overlay51FinalizeReloc(void);
void overlay51ReleaseIndexReloc(s32 index);

void overlay51Initialize(void);
void overlay51PatchIndices(OverlayPatchIndexEntry *entry);
void overlay51ReleaseState(void);

#endif
