#include "PR/ultratypes.h"

/* Overlay 102: negative DKR/JFG exact scans; five relocs prove this map. */
extern u32 *gOverlay102SignatureBlock;
void overlay102DmaCopyReloc(s32 size, void *address, s32 alignment);

s32 overlay102CheckSignature(void) {
    if (gOverlay102SignatureBlock != 0) {
        overlay102DmaCopyReloc(0xF80, gOverlay102SignatureBlock, 0x10);
        if (gOverlay102SignatureBlock[0] == 0x6E5F923A &&
            gOverlay102SignatureBlock[1] == 0xFB0BE5F6) {
            return 1;
        }
    }
    return 0;
}
