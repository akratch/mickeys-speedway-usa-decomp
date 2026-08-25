/*
 * Overlay 88 and overlay 69 ship the same renderer body.  Keep the public
 * symbols overlay-local while sharing the reviewed implementation spelling.
 */
#define overlay69DrawSortedGeometry overlay88DrawSortedGeometry
#define overlay69DrawFixedResourceReloc overlay88DrawFixedResourceReloc
#define overlay69MetricReloc overlay88MetricReloc
#define overlay69PrepareTransformReloc overlay88PrepareTransformReloc
#define overlay69SubmitDynamicReloc overlay88SubmitDynamicReloc
#define overlay69SubmitFixedReloc overlay88SubmitFixedReloc

/* Plateau: the shared Mickey body has the exact 0x59C boundary, CFG, FP
 * topology, and call surface, but 140/359 positional words differ from +0x0.
 * Its frame is 0x150 bytes versus the target's 0x148.  The 119-combination
 * flag lattice found no improvement; a bounded source-only permuter batch
 * improved its internal score from 1350 to 875 without changing that frame.
 * Lower scores changed the sort semantics.  The remaining blocker is the
 * reconstructed aggregate/lifetime layout, not a compiler-flag mismatch. */
#ifdef NON_MATCHING
#include "src/overlays/o069/overlay69DrawSortedGeometry.c"
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o088/overlay88DrawSortedGeometry/func_overlay_088_F00001A4_18D3C2C.s")
#endif
