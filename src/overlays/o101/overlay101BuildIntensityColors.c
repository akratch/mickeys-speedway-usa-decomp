#include "PR/ultratypes.h"

void overlay101BuildIntensityColors(s32 intensity, s32 alpha, u32 *full,
                                    u32 *dim, u32 *dimmer, u32 *darkest) {
    s32 component;
    s32 blue;

    if (full != NULL) {
        component = (intensity * 0xFF) >> 8;
        *full = (component << 24) | (component << 16) | (component << 8) |
                (alpha & 0xFF);
    }
    if (dim != NULL) {
        component = (intensity * 0x78) >> 8;
        blue = (intensity * 0x40) >> 8;
        *dim = (component << 24) | (component << 16) | (blue << 8) |
               (alpha & 0xFF);
    }
    if (dimmer != NULL) {
        component = (intensity * 0xF0) >> 8;
        blue = (intensity * 0x80) >> 8;
        *dimmer = (component << 24) | (component << 16) | (blue << 8) |
                  (alpha & 0xFF);
    }
    if (darkest != NULL) {
        component = (intensity * 0x10) >> 8;
        *darkest = (component << 16) | (component << 8) | (alpha & 0xFF);
    }
}
