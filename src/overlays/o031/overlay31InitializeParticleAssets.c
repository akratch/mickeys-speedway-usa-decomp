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
/* Workbench: structure mismatch; best is 133 words for 132 with the exact 0x30 frame and the first residual at +0xC.
 * Levers: DKR source-shape check plus initializer, signedness, fixed-loop, and global-count/index data-flow variants.
 * Remaining: direct count stores fold the index; retaining the indexed base adds one count-address materialization. */
#ifdef NON_MATCHING
void func_overlay_031_F00002E8_187F808(void) {
    s32 i;

    gOverlay31AssetStateCount = 0;
    i = gOverlay31AssetStateCount + 1;
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
