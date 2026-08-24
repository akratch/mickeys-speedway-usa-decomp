#include "ultra64.h"

extern s32 gOverlay29Current;
extern u8 *gOverlay29Base0;
extern u8 *gOverlay29Base1;
extern u8 *gOverlay29Base2;
extern u8 *gOverlay29Base3;
extern u8 *gOverlay29Selected0;
extern u8 *gOverlay29Selected1;
extern u8 *gOverlay29Selected2;
extern u8 *gOverlay29Selected3;

/* The four parallel Mickey tables have no DKR v77/v80 or JFG donor. */
void overlay29Select(s32 index) {
    s32 offset;

    if (index != 666) {
        gOverlay29Current = index;
    }
    offset = gOverlay29Current << 4; gOverlay29Selected0 = gOverlay29Base0 + offset + 0x14;
    gOverlay29Selected1 = gOverlay29Base1 + offset + 0x14;
    gOverlay29Selected2 = gOverlay29Base2 + offset + 0x14;
    gOverlay29Selected3 = gOverlay29Base3 + offset + 0x14;
}
