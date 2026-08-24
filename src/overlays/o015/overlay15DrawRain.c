#include "PR/ultratypes.h"

typedef struct Overlay15Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Overlay15Vec3f;

typedef struct Overlay15CameraState {
    s16 angle;
} Overlay15CameraState;

extern s32 gOverlay15RainCapacity;
extern s32 gOverlay15RainEnabled;
extern Overlay15Vec3f *gOverlay15RainPositions;
extern f32 gOverlay15RainOffsetX;
extern f32 gOverlay15RainOffsetY;
extern f32 gOverlay15RainOffsetZ;
extern u32 *gOverlay15RainColors;

extern Overlay15CameraState *overlay15GetActiveCameraReloc(void);
extern void rainFastDraw(void *framebuffer, s32 width, s32 height, s32 count,
                         Overlay15Vec3f *positions, u32 *colors, s32 angle,
                         f32 offsetX, f32 offsetY, f32 offsetZ,
                         f32 projectionScale);

/* Mickey-local reconstruction; the pinned DKR v77/v80 and JFG scans are negative. */
#ifdef NON_MATCHING
void overlay15DrawRain(void *framebuffer, s32 width, s32 height,
                       f32 projectionScale, f32 intensity) {
    Overlay15CameraState *camera;
    s32 visibleCount;

    if (gOverlay15RainEnabled != 0) {
        visibleCount = (s32)((f32)gOverlay15RainCapacity * intensity);
        if ((gOverlay15RainPositions != 0) && (visibleCount > 0)) {
            camera = overlay15GetActiveCameraReloc();
            rainFastDraw(framebuffer, width, height, visibleCount,
                         gOverlay15RainPositions, gOverlay15RainColors,
                         camera->angle + 0x8000, gOverlay15RainOffsetX,
                         gOverlay15RainOffsetY, gOverlay15RainOffsetZ,
                         projectionScale);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o015/overlay15DrawRain/func_overlay_015_F0000B94_1872F2C.s")
#endif
