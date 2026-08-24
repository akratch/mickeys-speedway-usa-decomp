#include "overlays/overlay_051.h"

/*
 * Overlay 51, ADR 0006 consolidation: one translation unit in ROM order.
 * The pinned DKR v77/v80 and JFG scans found no exact donor for the matched
 * C functions; the unresolved middle function remains GLOBAL_ASM.
 */

void overlay51Initialize(void) {
    overlay51CreateReloc(gOverlay51Resource0);
    overlay51CreateReloc(gOverlay51Resource18);
    overlay51CreateReloc(4);
    overlay51CreateReloc(11);
    overlay51PrepareReloc(gOverlay51ResourceBC);
    overlay51PrepareReloc(gOverlay51Resource1C);
    gOverlay51InitialValue = -80.0f;
    overlay51CreateReloc();
    gOverlay51Mode = -1;
    gOverlay51Handle = overlay51CreateReloc();
}

void overlay51PatchIndices(OverlayPatchIndexEntry *entry) {
    while (entry->first != 0) {
        entry->first = (s32) gOverlay51Objects[entry->first];
        if (entry->second != 0) {
            entry->second = (s32) gOverlay51Objects[entry->second];
        }
        entry++;
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o051/overlay_051/func_overlay_051_F00000D0_18999D0.s")

void overlay51ReleaseState(void) {
    s32 index;

    overlay51ReleaseReloc(gOverlay51InlineResource);
    overlay51FinalizeReloc();
    index = gOverlay51Index;
    if (index != -1) {
        overlay51ReleaseIndexReloc(index);
        gOverlay51Index = -1;
    }
}
