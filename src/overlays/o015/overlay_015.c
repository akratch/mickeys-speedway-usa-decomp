#include "overlays/overlay_015.h"

/* The shipped LOCAL relocations address this block through the initialized
 * section base; text-side proxy symbols remain the relocation carriers. */
typedef struct Overlay15InitializedData {
    u32 prefix[6];
    Overlay15Gfx starSetup[5];
    f32 starFadeScale;
} Overlay15InitializedData;

Overlay15InitializedData gOverlay15InitializedData = {
    { 0, 0, 0, 1, 0, 0 },
    {
        { 0xE7000000U, 0 },
        { 0xB6000000U, 0x00010001U },
        { 0xFCFFFFFFU, 0xFFFDF6FBU },
        { 0xEF000C0FU, 0x0F0A4000U },
        { 0xB8000000U, 0 },
    },
    0.8732876777648926F,
};

/* The primary bound scalars lead the BSS exactly as they do in the retail
 * object. Other particle modes alias these words through text-side proxies. */
f32 gOverlay15StarBound0;
f32 gOverlay15StarBound1;
f32 gOverlay15StarBound2;
f32 gOverlay15StarBound3;
f32 gOverlay15StarBound4;
f32 gOverlay15StarBound5;
f32 gOverlay15StarBound6;
f32 gOverlay15StarBound7;
f32 gOverlay15StarBound8;
u32 gOverlay15BssPad24;
u8 gOverlay15BssTail[0x78];

/*
 * Overlay 15, ADR 0006 consolidation. Functions remain in retail ROM order.
 * The R4300 multiply-hazard flag is harmless for the active resource/value
 * wrappers and is required by the fallback-heavy drawing translation unit.
 */

void *overlay15GetResource4(void) {
    return gOverlay15Resource4;
}

/* DKR v77/v80 has generic resource-release wrappers but no exact donor. */
void overlay15ReleaseResource(void) {
    if (gOverlay15Resource4 != 0) {
        overlay15ReleaseReloc(gOverlay15Resource4);
        gOverlay15Resource4 = 0;
        gOverlay15Resource48 = 0;
    }
}

/* Plateau (2026-08-25, batch 36): canonical -O2 -mips2 is four bytes
 * short; best 230/247 words differ, first at +0x4. Field-order/count-address
 * lifetimes improved 238 to 230; lattice, playbook, and 40m permuter found no exact. */
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
    s32 *countAddress;

    starIndex = count * 12;
    starCount = count;
    stars = overlay15Allocate(starIndex + 0x200, 0x87);
    gOverlay15Stars = stars;
    gOverlay15StarPalette = (u16 *) ((u8 *) stars + starIndex);

    gOverlay15InitBounds.xRange = (f32) xRange;
    gOverlay15InitBounds.xMin = gOverlay15InitBounds.xRange * -0.5f;
    xRange <<= 7;
    gOverlay15InitBounds.xMax = gOverlay15InitBounds.xRange * 0.5f;
    gOverlay15InitBounds.yRange = (f32) yRange;
    gOverlay15InitBounds.yMin = gOverlay15InitBounds.yRange * -0.5f;
    yRange <<= 7;
    gOverlay15InitBounds.yMax = gOverlay15InitBounds.yRange * 0.5f;
    gOverlay15InitBounds.zRange = (f32) zRange;
    zRange = (zRange + 1) << 8;
    countAddress = &gOverlay15StarCount;
    *countAddress = starCount;
    gOverlay15InitBounds.zero = 0;
    gOverlay15InitBounds.colorDivisor = (f32) colorDivisor;
    gOverlay15InitBounds.zMax = gOverlay15InitBounds.zRange + 1.0f;
    gOverlay15InitBounds.zMin = 1.0f;
    gOverlay15InitBounds.colorStep =
        255.0f / gOverlay15InitBounds.colorDivisor;

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
        } while (previousStarIndex < *countAddress);
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
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o015/overlay_015/func_overlay_015_F000004C_18723E4.s")
#endif

typedef struct Overlay15StarMovementView {
    u8 pad00[0x30];
    Overlay15Star movement;
} Overlay15StarMovementView;

typedef struct Overlay15StarPointerView {
    u8 pad00[4];
    Overlay15Star *stars;
} Overlay15StarPointerView;

/* Workbench: size-mismatch, +16 bytes (58 vs 54 instructions), first +0x30.
 * Target CFG/FP/call shape is recovered; four redundant BSS address producers remain.
 * Not shape-exact: contiguous bound-address reuse is unresolved. */
#ifdef NON_MATCHING
void overlay15MoveStars(f32 movementX, f32 movementY, f32 movementZ,
                        s32 rate) {
    f32 scale;

    ((Overlay15StarMovementView *)&gOverlay15StarMovement)->movement.x =
        movementX;
    ((Overlay15StarMovementView *)&gOverlay15StarMovement)->movement.y =
        movementY;
    ((Overlay15StarMovementView *)&gOverlay15StarMovement)->movement.z =
        movementZ;
    if (((Overlay15StarPointerView *)&gOverlay15Stars)->stars != 0) {
        scale = (f32)rate;
        movementX *= scale;
        movementY *= scale;
        movementZ *= scale;
        starfieldFastMove(gOverlay15StarCount,
                          ((Overlay15StarPointerView *)&gOverlay15Stars)->stars,
                          movementX, movementY, movementZ,
                          gOverlay15StarBound0, gOverlay15StarBound1,
                          gOverlay15StarBound2, gOverlay15StarBound3,
                          gOverlay15StarBound4, gOverlay15StarBound5,
                          gOverlay15StarBound6, gOverlay15StarBound7,
                          gOverlay15StarBound8);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o015/overlay_015/func_overlay_015_F0000428_18727C0.s")
#endif

/*
 * Plateau (2026-08-30): configured -O2 -mips2 with -Wab,-r4300_mul remains
 * exact-size at 105 words with the exact 0x58 frame, 13 relocation-masked
 * differences (14 raw), and first mismatch +0x18. Ten fresh command,
 * fade-lifetime, and packed-expression forms were neutral or worse. Preserve
 * this semantic baseline; the remaining blockers are the entry setup/fade-load
 * schedule, final packed-command association, and the unauthenticated
 * LOCAL/data relocation identities.
 */
#ifdef NON_MATCHING
void overlay15DrawScreenStars(Overlay15Gfx **displayList, f32 projectionScale) {
    Overlay15Gfx *command;
    Overlay15Star *star;
    s32 remaining;
    s32 screenX;
    s32 screenWidth;
    s32 screenHeight;
    s32 screenY;
    s32 shade;
    f32 inverseDepth;
    Overlay15Gfx *initialCommand;
    f32 fadeScale;

    overlay15GetDimensionsReloc(&screenWidth, &screenHeight);
    remaining = gOverlay15StarCount;
    command = *displayList;
    star = gOverlay15Stars;
    initialCommand = command++;
    initialCommand->w0 = 0x06000000;
    initialCommand->w1 = (u32) gOverlay15StarSetup;
    fadeScale = gOverlay15StarFadeScale;

    while (remaining--) {
        if ((star->z >= 8.0f) && (star->z < 300.0f)) {
            inverseDepth = projectionScale / star->z;
            screenX = (s32) (star->x * inverseDepth) +
                      (s32) (((u32) screenWidth) >> 1);
            screenY = (s32) (((u32) screenHeight) >> 1) -
                      (s32) (star->y * inverseDepth);
            if ((screenX >= 0) && (screenY >= 0) &&
                (screenX < screenWidth) && (screenY < screenHeight)) {
                shade = 255 - (s32) ((star->z - 8.0f) * fadeScale);
                command->w0 = 0xFA000000;
                command->w1 = (shade << 24) | (shade << 16) |
                              (shade << 8) | 0xFF;
                command++;
                command->w0 = 0xF6000000 | ((screenX + 1) << 14) |
                              ((screenY + 1) << 2);
                command->w1 = (screenX << 14) | (screenY << 2);
                command++;
            }
        }
        star++;
    }

    *displayList = command;
    overlay15FinishDisplayListReloc(displayList);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o015/overlay_015/func_overlay_015_F0000500_1872898.s")
#endif

void *overlay15GetResource10(void) {
    return gOverlay15Resource10;
}

void overlay15ReleaseResource10(void) {
    if (gOverlay15Resource10 != 0) {
        overlay15ReleaseReloc(gOverlay15Resource10);
        gOverlay15Resource10 = 0;
    }
}

#ifdef NON_MATCHING
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
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o015/overlay_015/func_overlay_015_F00006E8_1872A80.s")
#endif

/* Mickey-local reconstruction; pinned DKR v77/v80 and JFG scans are negative.
 * Plateau: canonical C is 110/103 words (+28 bytes), with 84 differing from
 * +0x30; scalar BSS symbols retain seven HIs after pair, flag, and permuter sweeps. */
#ifdef NON_MATCHING
void overlay15UpdateMovingStars(f32 positionX, f32 positionY, f32 positionZ,
                                s32 updateRate) {
    Overlay15MovingStarCamera *camera;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 scale;

    camera = overlay15GetActiveCameraReloc();
    if (gOverlay15CameraReadyRead != 0 && updateRate != 0) {
        scale = 1.0f / (f32)updateRate;
        deltaX = (gOverlay15PreviousCameraX - camera->x) * scale;
        deltaY = (gOverlay15PreviousCameraY - camera->y) * scale;
        deltaZ = (gOverlay15PreviousCameraZ - camera->z) * scale;
    } else {
        deltaX = 0.0f;
        deltaY = 0.0f;
        deltaZ = 0.0f;
    }

    gOverlay15PreviousCameraX = camera->x;
    gOverlay15PreviousCameraY = camera->y;
    gOverlay15PreviousCameraZ = camera->z;
    positionX += deltaX;
    positionY += deltaY;
    positionZ += deltaZ;
    gOverlay15CameraReadyWrite = 1;
    gOverlay15CurrentPositionX = positionX;
    gOverlay15CurrentPositionY = positionY;
    gOverlay15CurrentPositionZ = positionZ;

    if (gOverlay15MovingStars != 0) {
        scale = (f32)updateRate;
        positionX *= scale;
        positionZ *= scale;
        positionY *= scale;
        starfieldFastMove(gOverlay15MovingStarCount, gOverlay15MovingStars,
                          positionX, positionY, positionZ,
                          gOverlay15MovingBound0, gOverlay15MovingBound1,
                          gOverlay15MovingBound2, gOverlay15MovingBound3,
                          gOverlay15MovingBound4, gOverlay15MovingBound5,
                          gOverlay15MovingBound6, gOverlay15MovingBound7,
                          gOverlay15MovingBound8);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o015/overlay_015/func_overlay_015_F00009E0_1872D78.s")
#endif

void overlay15SetValueC(s32 value) {
    gOverlay15ValueC = value;
}

void overlay15ClearValue7C(void) {
    gOverlay15Value7C = 0;
}

/* Mickey-local reconstruction; the pinned DKR v77/v80 and JFG scans are negative. */
typedef struct Overlay15RainOffsets {
    u8 pad00[0x80];
    f32 x;
    f32 y;
    f32 z;
} Overlay15RainOffsets;

extern Overlay15RainOffsets gOverlay15RainOffsets;

/* Plateau (2026-08-25, ownership): linked full-BSS candidate is exact-size
 * 0xD8 with 13 differing words, first +0x74; the 119-flag lattice is flat.
 * Workbench: structure-mismatch; post-call load scheduling remains. */
#ifdef NON_MATCHING
void overlay15DrawRain(void *framebuffer, s32 width, s32 height,
                       f32 projectionScale, f32 intensity) {
    s32 visibleCount;
    Overlay15CameraState *camera;

    if (gOverlay15RainEnabled != 0) {
        visibleCount = (s32)((f32)gOverlay15RainCapacity * intensity);
        if ((gOverlay15RainPositions != 0) && (visibleCount > 0)) {
            camera = overlay15GetActiveCameraReloc();
            rainFastDraw(framebuffer, width, height, visibleCount,
                         gOverlay15RainPositions, gOverlay15RainColors,
                         camera->angle + 0x8000, gOverlay15RainOffsets.x,
                         gOverlay15RainOffsets.y, gOverlay15RainOffsets.z,
                         projectionScale);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o015/overlay_015/func_overlay_015_F0000B94_1872F2C.s")
#endif

/* PLATEAU-HANDOFF:overlay15DrawScreenStars:start
 * symbol: overlay15DrawScreenStars
 * score: 92/105 words
 * frame: 0x58
 * relocations: 10
 * first-mismatch: +0x18
 * summary: Ten source-authentic command, fade-lifetime, and packed-expression forms found no gain; entry scheduling and local-data identity proof remain.
 * PLATEAU-HANDOFF:overlay15DrawScreenStars:end
 */
