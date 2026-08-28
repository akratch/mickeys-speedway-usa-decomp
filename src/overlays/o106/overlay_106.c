#include "overlays/overlay_106.h"

/*
 * Overlay 106: complete 0x10-byte no-relocation module.
 *
 * PROVENANCE: identity adapted from JFG's public overlay o144, whose complete
 * text is byte-identical at the same no-relocation module offset. Mickey's
 * own compiled C already emitted the exact executable body.
 */
s32 osRamTest3_6105(void) {
    return 1;
}
