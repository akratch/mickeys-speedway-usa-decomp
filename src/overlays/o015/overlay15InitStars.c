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
    f32 zMin;
    f32 zMax;
    f32 zRange;
    f32 colorDivisor;
    f32 colorStep;
    s32 zero;
} Overlay15InitBounds;

extern Overlay15InitBounds gOverlay15InitBounds;
extern s32 gOverlay15StarCount;
extern Overlay15Star *gOverlay15Stars;
extern u32 *gOverlay15StarColors;
extern void *overlay15Allocate(s32 size, s32 tag);
extern s32 overlay15RandomRange(s32 minimum, s32 maximum);

void overlay15InitStars(s32 count, s32 xRange, s32 yRange, s32 zRange,
                        u32 startColor, u32 endColor, s32 colorDivisor) {
    Overlay15Star *stars;
    u32 *colors;
    Overlay15Star **starsAddress;
    u32 **colorsAddress;
    s32 *countAddress;
    Overlay15InitBounds *unusedBoundsAddress;
    s32 i;
    s32 startR;
    s32 startG;
    s32 startB;
    s32 startA;
    s32 deltaR;
    s32 deltaG;
    s32 deltaB;
    s32 deltaA;
    s32 colorTime;

    unusedBoundsAddress = &gOverlay15InitBounds;
    stars = overlay15Allocate(count << 4, 0x87);

    gOverlay15InitBounds.xRange = (f32) xRange;
    gOverlay15InitBounds.xMin = gOverlay15InitBounds.xRange * -0.5f;
    gOverlay15InitBounds.xMax = gOverlay15InitBounds.xRange * 0.5f;
    gOverlay15InitBounds.yRange = (f32) yRange;
    gOverlay15InitBounds.yMin = gOverlay15InitBounds.yRange * -0.5f;
    gOverlay15InitBounds.yMax = gOverlay15InitBounds.yRange * 0.5f;
    gOverlay15InitBounds.zRange = (f32) zRange;
    gOverlay15InitBounds.zMin = gOverlay15InitBounds.zRange * -0.5f;
    gOverlay15InitBounds.zMax = gOverlay15InitBounds.zRange * 0.5f;
    gOverlay15InitBounds.colorDivisor = (f32) colorDivisor;

    colors = (u32 *) (stars + count);
    starsAddress = &gOverlay15Stars;
    colorsAddress = &gOverlay15StarColors;
    countAddress = &gOverlay15StarCount;
    xRange <<= 7;
    yRange <<= 7;
    zRange <<= 7;
    *starsAddress = stars;
    *colorsAddress = colors;
    *countAddress = count;
    gOverlay15InitBounds.zero = 0;
    gOverlay15InitBounds.colorStep = 255.0f /
                                     gOverlay15InitBounds.colorDivisor;

    i = 0;
    if (count > 0) {
        startR = (startColor >> 24) & 0xFF;
        startG = (startColor >> 16) & 0xFF;
        startB = (startColor >> 8) & 0xFF;
        startA = startColor & 0xFF;
        deltaR = ((endColor >> 24) & 0xFF) - startR;
        deltaG = ((endColor >> 16) & 0xFF) - startG;
        deltaB = ((endColor >> 8) & 0xFF) - startB;
        deltaA = (endColor & 0xFF) - startA;

        do {
            stars->x = (f32) overlay15RandomRange(-xRange, xRange) *
                       (1.0f / 256.0f);
            stars->y = (f32) overlay15RandomRange(-yRange, yRange) *
                       (1.0f / 256.0f);
            stars->z = (f32) overlay15RandomRange(-zRange, zRange) *
                       (1.0f / 256.0f);
            colorTime = overlay15RandomRange(0, 255);
            *colors = ((((deltaR * colorTime) >> 8) + startR) << 24) |
                      ((((deltaG * colorTime) >> 8) + startG) << 16) |
                      ((((deltaB * colorTime) >> 8) + startB) << 8) |
                      (((deltaA * colorTime) >> 8) + startA);
            i++;
            stars++;
            colors++;
        } while (i < gOverlay15StarCount);
    }
}
