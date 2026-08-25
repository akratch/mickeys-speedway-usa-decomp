#include "PR/ultratypes.h"

typedef struct Overlay31Behaviour {
    u8 pad00[0x9C];
    void *colourLoop;
} Overlay31Behaviour;

extern s32 gOverlay31AssetStateCount;
extern s32 gOverlay31AssetStates[];
extern s32 gOverlay31ParticleAssetCount;
extern u8 *gOverlay31ParticleAssets;
extern void **gOverlay31ParticleAssetTable;
extern s32 gOverlay31BehaviourAssetCount;
extern u8 *gOverlay31BehaviourAssets;
extern Overlay31Behaviour **gOverlay31BehaviourAssetTable;

extern void *overlay31LoadAssetTableReloc(s32 assetId);
extern void overlay31BuildPalettes(void);
extern void *overlay31GetMiscAssetReloc(s32 assetId);
extern void func_overlay_031_F0000000_187F520(void);

/* Preserve the raw split carrier identity in a removable private island. */
static void *const overlay31RuntimeCarrier =
    func_overlay_031_F0000000_187F520;

/* PROVENANCE: Diddy Kong Racing, src/particles.c (init_particle_assets);
 * semantic source-shape analogue only. Mickey's ROM decides every detail. */
/*
 * Plateau (2026-08-25, independently rechecked in the overlay 31/38/46 lane):
 * the natural -O2 -mips2 body emits 129 words for the 132-word target, first
 * differing at +0x4. Retail retains the initial state index in s1, forms one
 * indexed base, and reuses it for four stores; IDO folds the same value into
 * constant offsets. The 119-point flag lattice had no exact result (-O2
 * -mips1 was only the least-bad ranking at 129 differing words). A typed base,
 * chained initialization, preincrement/embedded-assignment forms, and source
 * line regrouping were codegen-inert; a volatile index enlarged the frame and
 * worsened the ABI shape. The bounded two-thread permuter improved score 1030
 * to 320 in 601 seconds but found no zero. The nearest permitted skeleton is
 * JFG partInitLib at 0.433, itself GLOBAL_ASM with no usable C lifetime proof.
 */
#ifdef NON_MATCHING
void func_overlay_031_F00002E8_187F808(void) {
    s32 i;

    i = 1;
    gOverlay31AssetStateCount = 0;
    gOverlay31AssetStates[i + 1] = 0;
    gOverlay31AssetStates[i + 2] = 0;
    gOverlay31AssetStates[i + 3] = 0;
    gOverlay31AssetStates[i] = 0;

    gOverlay31ParticleAssetTable = overlay31LoadAssetTableReloc(0x33);
    while ((s32)gOverlay31ParticleAssetTable[gOverlay31ParticleAssetCount + 1] != -1) {
        gOverlay31ParticleAssetCount++;
    }

    gOverlay31ParticleAssets = overlay31LoadAssetTableReloc(0x34);
    for (i = 0; i < gOverlay31ParticleAssetCount; i++) {
        gOverlay31ParticleAssetTable[i] =
            gOverlay31ParticleAssets + (s32)gOverlay31ParticleAssetTable[i];
    }

    overlay31BuildPalettes();

    gOverlay31BehaviourAssetTable = overlay31LoadAssetTableReloc(0x35);
    while ((s32)gOverlay31BehaviourAssetTable[gOverlay31BehaviourAssetCount + 1] != -1) {
        gOverlay31BehaviourAssetCount++;
    }

    gOverlay31BehaviourAssets = overlay31LoadAssetTableReloc(0x36);
    for (i = 0; i < gOverlay31BehaviourAssetCount; i++) {
        gOverlay31BehaviourAssetTable[i] =
            (Overlay31Behaviour *)(gOverlay31BehaviourAssets +
                                   (s32)gOverlay31BehaviourAssetTable[i]);
        if ((u32)gOverlay31BehaviourAssetTable[i]->colourLoop != 0xFFFFFFFF) {
            gOverlay31BehaviourAssetTable[i]->colourLoop =
                overlay31GetMiscAssetReloc(
                    (s32)gOverlay31BehaviourAssetTable[i]->colourLoop);
        } else {
            gOverlay31BehaviourAssetTable[i]->colourLoop = NULL;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o031/overlay31InitializeParticleAssets/func_overlay_031_F00002E8_187F808.s")
#endif
