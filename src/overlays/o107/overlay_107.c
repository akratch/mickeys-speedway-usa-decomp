#include "overlays/overlay_107.h"

/*
 * Overlay 107, entire 0x30-byte text section. JFG's overlay 156 provides the
 * exact whole-module byte match and the osRamTest4_6105 name; this C body is
 * independently expressed from Mickey's loads, comparison, and return flow.
 */
s32 osRamTest4_6105(void) {
    if (*(volatile u32 *)0xA00002E8 != 0xC86E2000) {
        return 0;
    }
    return 1;
}
