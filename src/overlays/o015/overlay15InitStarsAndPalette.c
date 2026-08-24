#include "PR/ultratypes.h"

typedef struct Overlay15Star {
    f32 x;
    f32 y;
    f32 z;
} Overlay15Star;

typedef struct Overlay15InitBounds {
    f32 xMin;
    f32 xMax;
    f32 xRange;
    f32 yMin;
    f32 yMax;
    f32 yRange;
    f32 zBase;
    f32 zEnd;
    f32 zRange;
    f32 colorDivisor;
    f32 colorStep;
    s32 zero;
} Overlay15InitBounds;

extern s32 gOverlay15StarCount;
extern Overlay15Star *gOverlay15Stars;
extern u16 *gOverlay15StarPalette;
extern Overlay15InitBounds gOverlay15InitBounds;
extern void *overlay15Allocate(s32 size, s32 tag);
extern s32 overlay15RandomRange(s32 minimum, s32 maximum);

#ifdef NON_MATCHING
void overlay15InitStarsAndPalette(s32 count, s32 xRange, s32 yRange,
                                  s32 zRange, u32 startColor, u32 endColor,
                                  s32 colorDivisor) {
    Overlay15Star *stars;
    u16 *palette;
    s32 starIndex;
    s32 paletteIndex0;
    s32 paletteIndex1;
    s32 paletteIndex2;
    s32 paletteIndex3;
    s32 startR;
    s32 startG;
    s32 startB;
    s32 deltaR;
    s32 deltaG;
    s32 deltaB;
    s32 starCount;
    s32 previousStarIndex;

    starCount = count;
    starIndex = starCount * 12;
    stars = overlay15Allocate(starIndex + 0x200, 0x87);
    gOverlay15Stars = stars;
    gOverlay15StarPalette = (u16 *) ((u8 *) stars + starIndex);

    gOverlay15InitBounds.xRange = (f32) xRange;
    gOverlay15InitBounds.xMin = gOverlay15InitBounds.xRange * -0.5f;
    gOverlay15InitBounds.xMax = gOverlay15InitBounds.xRange * 0.5f;
    gOverlay15InitBounds.yRange = (f32) yRange;
    gOverlay15InitBounds.yMin = gOverlay15InitBounds.yRange * -0.5f;
    gOverlay15InitBounds.yMax = gOverlay15InitBounds.yRange * 0.5f;
    gOverlay15InitBounds.zRange = (f32) zRange;
    gOverlay15InitBounds.zero = 0;
    gOverlay15InitBounds.colorDivisor = (f32) colorDivisor;
    gOverlay15InitBounds.zEnd = gOverlay15InitBounds.zRange + 1.0f;
    gOverlay15StarCount = starCount;
    gOverlay15InitBounds.zBase = 1.0f;
    gOverlay15InitBounds.colorStep =
        255.0f / gOverlay15InitBounds.colorDivisor;

    xRange <<= 7;
    yRange <<= 7;
    zRange = (zRange + 1) << 8;
    starIndex = 1;
    if (starCount > 0) {
        do {
            stars->x = (f32) overlay15RandomRange(-xRange, xRange) *
                       (1.0f / 256.0f);
            stars->y = (f32) overlay15RandomRange(-yRange, yRange) *
                       (1.0f / 256.0f);
            stars->z = (f32) overlay15RandomRange(0x100, zRange) *
                       (1.0f / 256.0f);
            previousStarIndex = starIndex;
            starIndex++;
            stars++;
        } while (previousStarIndex < gOverlay15StarCount);
    }

    paletteIndex0 = 0;
    startR = (startColor >> 24) & 0xFF;
    startG = (startColor >> 16) & 0xFF;
    startB = (startColor >> 8) & 0xFF;
    deltaR = ((endColor >> 24) & 0xFF) - startR;
    deltaG = ((endColor >> 16) & 0xFF) - startG;
    deltaB = ((endColor >> 8) & 0xFF) - startB;

    palette = gOverlay15StarPalette;
    paletteIndex0 = 0;
    paletteIndex1 = 1;
    paletteIndex2 = 2;
    paletteIndex3 = 3;
    do {
        *palette++ =
            (((((deltaR * paletteIndex0) >> 8) + startR) & 0xF8) << 8) |
            (((((deltaG * paletteIndex0) >> 8) + startG) & 0xF8) << 3) |
            (((((deltaB * paletteIndex0) >> 8) + startB) & 0xF8) >> 2) | 1;
        *palette++ =
            (((((deltaR * paletteIndex1) >> 8) + startR) & 0xF8) << 8) |
            (((((deltaG * paletteIndex1) >> 8) + startG) & 0xF8) << 3) |
            (((((deltaB * paletteIndex1) >> 8) + startB) & 0xF8) >> 2) | 1;
        *palette++ =
            (((((deltaR * paletteIndex2) >> 8) + startR) & 0xF8) << 8) |
            (((((deltaG * paletteIndex2) >> 8) + startG) & 0xF8) << 3) |
            (((((deltaB * paletteIndex2) >> 8) + startB) & 0xF8) >> 2) | 1;
        *palette++ =
            (((((deltaR * paletteIndex3) >> 8) + startR) & 0xF8) << 8) |
            (((((deltaG * paletteIndex3) >> 8) + startG) & 0xF8) << 3) |
            (((((deltaB * paletteIndex3) >> 8) + startB) & 0xF8) >> 2) | 1;
        paletteIndex0 += 4;
        paletteIndex1 += 4;
        paletteIndex2 += 4;
        paletteIndex3 += 4;
    } while (paletteIndex0 != 0x100);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o015/overlay15InitStarsAndPalette/func_overlay_015_F000004C_18723E4.s")
#endif
