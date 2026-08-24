#include "PR/ultratypes.h"

typedef struct Overlay15Gfx { u32 w0, w1; } Overlay15Gfx;
typedef struct Overlay15Star { f32 x, y, z; } Overlay15Star;

extern s32 gOverlay15StarCount;
extern Overlay15Star *gOverlay15Stars;
extern Overlay15Gfx gOverlay15StarSetup[];
extern const f32 gOverlay15StarFadeScale;
extern void overlay15GetDimensionsReloc(s32 *width, s32 *height);
extern void overlay15FinishDisplayListReloc(Overlay15Gfx **displayList);

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
