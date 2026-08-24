#include "PR/ultratypes.h"

/* Packed-colour reader; exact DKR and JFG scans are negative. */
extern u32 gOverlay56Colors[];
void overlay56UnpackColor(s32 index, u32 *red, s32 *green, s32 *blue) {
    u32 *color = &gOverlay56Colors[index];
    *red = *color >> 24;
    *green = (*color >> 16) & 0xFF;
    *blue = (*color >> 8) & 0xFF;
}
