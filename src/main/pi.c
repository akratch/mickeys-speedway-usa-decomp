/*
 * Cartridge asset DMA -- ROM 0x2ECA0-0x2F0D0.
 *
 * PROVENANCE -- the TU identity and names are adapted from Jet Force Gemini's
 * public decompilation, src/pi.c. Mickey's seven-function order and PI/DMA
 * call graph establish the correspondence. Adapted C bodies are identified in
 * docs/modules.md; all remaining functions stay as Mickey GLOBAL_ASM.
 */

#include "PR/ultratypes.h"
#include "game/pi.h"

typedef struct AssetLookupTable {
    u32 fileCount;
    u32 offsets[1];
} AssetLookupTable;

extern AssetLookupTable *D_800D2470;
extern u8 D_86760[];

void romCopy(u32 romOffset, u32 ramAddress, s32 numBytes);
void *func_8002B280();

#pragma GLOBAL_ASM("asm/nonmatchings/main/pi/piInit.s")
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/pi.c:piRomLoad. */
u32 *piRomLoad(u32 assetIndex) {
    u32 *index;
    u32 *out;
    s32 size;
    u32 start;

    if (assetIndex > D_800D2470->fileCount) {
        return NULL;
    }
    assetIndex++;
    index = assetIndex + D_800D2470->offsets - 1;
    start = index[0];
    size = index[1] - start;
    if (size == 0) {
        return NULL;
    }
    out = func_8002B280(size, 0x84);
    if (out == NULL) {
        return NULL;
    }
    romCopy((u32) (start + D_86760), (u32) out, size);
    return out;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/pi/piRomLoadCompressed.s")
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/pi.c:piRomLoadSection. */
s32 piRomLoadSection(u32 assetIndex, u32 address, s32 assetOffset, s32 size) {
    u32 *index;
    s32 start;

    if (size == 0 || D_800D2470->fileCount < assetIndex) {
        return 0;
    }

    assetIndex++;
    index = assetIndex + D_800D2470->offsets - 1;
    start = index[0] + assetOffset;
    romCopy((u32) (start + D_86760), address, size);
    return size;
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/pi.c:piRomGetSectionPtr. */
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
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/pi.c:piRomGetFileSize. */
s32 piRomGetFileSize(u32 assetIndex) {
    u32 *index;
    s32 size;

    if (assetIndex > D_800D2470->fileCount) {
        return 0;
    }

    assetIndex++;
    index = assetIndex + D_800D2470->offsets - 1;
    size = index[1] - index[0];
    return size;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/pi/romCopy.s")
