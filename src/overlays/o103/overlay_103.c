#include "overlays/overlay_103.h"

/*
 * Overlay 103, entire 0x70-byte text section. No exact DKR v77/v80 or JFG
 * donor exists. The function map is instead proved by its five relocation
 * records: two pairs load reserved-section symbol 0xFFD:0x14A4 and the sole
 * external call targets resident +0x2DF90 (func_8002E3E0).
 */
s32 overlay103CheckSignature(void) {
    if (gOverlay103SignatureBlock != 0) {
        overlay103DmaCopyReloc(0xDC0, gOverlay103SignatureBlock, 0x10);
        if (gOverlay103SignatureBlock[0] == 0xB06DA99E &&
            gOverlay103SignatureBlock[1] == 0x5A0B4670) {
            return 1;
        }
    }
    return 0;
}
