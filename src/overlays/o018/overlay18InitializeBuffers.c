#include "PR/ultratypes.h"

extern s32 gOverlay18Width;
extern s32 gOverlay18Height;
extern s32 gOverlay18Depth;
extern s32 gOverlay18Layers;
extern s32 gOverlay18Mode;
extern u8 *gOverlay18Buffer00;
extern u8 *gOverlay18Buffer01;
extern u8 *gOverlay18Buffer1[2];
extern u8 *gOverlay18Buffer2[2];
extern u8 *gOverlay18Buffer3[2];
extern s32 gOverlay18SavedWidth;
extern s32 gOverlay18SavedHeight;
extern s32 gOverlay18SavedDepth;
extern s32 gOverlay18SavedLayers;

extern void *overlay18AllocateReloc(s32 size, s32 tag);

/* Pinned DKR v77/v80 and JFG searches found no exact donor. */
#ifdef NON_MATCHING
void overlay18InitializeBuffers(void) {
    s32 size;
    u8 *buffer;

    size = (gOverlay18Layers * 16) + (gOverlay18Width * 8) +
           (gOverlay18Height * 64) + (gOverlay18Depth * 10);
    gOverlay18Mode = 3;
    buffer = gOverlay18Buffer00 = overlay18AllocateReloc(size, 0x87);
    buffer = gOverlay18Buffer1[0] = buffer + (gOverlay18Width * 8);
    buffer = gOverlay18Buffer2[0] = buffer + (gOverlay18Height * 64);
    gOverlay18Buffer3[0] = buffer + (gOverlay18Depth * 10);

    buffer = gOverlay18Buffer01 = overlay18AllocateReloc(size, 0x87);
    buffer = gOverlay18Buffer1[1] = buffer + (gOverlay18Width * 8);
    buffer = gOverlay18Buffer2[1] = buffer + (gOverlay18Height * 64);
    gOverlay18Buffer3[1] = buffer + (gOverlay18Depth * 10);

    gOverlay18SavedWidth = gOverlay18Width;
    gOverlay18SavedHeight = gOverlay18Height;
    gOverlay18SavedLayers = gOverlay18Layers;
    gOverlay18SavedDepth = gOverlay18Depth;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o018/overlay18InitializeBuffers/func_overlay_018_F00004F4_1874AAC.s")
#endif
