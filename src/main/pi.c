/*
 * Cartridge asset DMA -- ROM 0x2ECA0-0x2F0D0.
 *
 * PROVENANCE -- the TU identity and names are adapted from Jet Force Gemini's
 * public decompilation, src/pi.c. Mickey's seven-function order and PI/DMA
 * call graph establish the correspondence. Adapted C bodies are identified in
 * docs/modules.md; all remaining functions stay as Mickey GLOBAL_ASM.
 */

#include "PR/ultratypes.h"

typedef struct AssetLookupTable {
    u32 fileCount;
    u32 offsets[1];
} AssetLookupTable;

extern AssetLookupTable *D_800D2470;
extern u8 D_86760[];

#pragma GLOBAL_ASM("asm/nonmatchings/main/pi/piInit.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/pi/piRomLoad.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/pi/piRomLoadCompressed.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/pi/piRomLoadSection.s")
u8 *piRomGetSectionPtr(u32 assetIndex, u32 assetOffset) {
    u32 *index;
    u32 start;

    if (assetIndex > D_800D2470->fileCount) {
        return NULL;
    }

    assetIndex++;
    index = assetIndex + D_800D2470->offsets - 1;
    start = index[0] + assetOffset;
    return start + D_86760;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/pi/piRomGetFileSize.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/pi/romCopy.s")
