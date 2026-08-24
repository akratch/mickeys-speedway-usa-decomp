#include "PR/ultratypes.h"

typedef struct Overlay15MovingStarVec3f {
    f32 x;
    f32 y;
    f32 z;
} Overlay15MovingStarVec3f;

typedef struct Overlay15MovingStarCamera {
    u8 pad[0xC];
    f32 x;
    f32 y;
    f32 z;
} Overlay15MovingStarCamera;

/* Both access names bind to the proven same BSS base +0x7C runtime field. */
extern s32 gOverlay15CameraReadyRead;
extern s32 gOverlay15CameraReadyWrite;
extern f32 gOverlay15CurrentPositionX;
extern f32 gOverlay15CurrentPositionY;
extern f32 gOverlay15CurrentPositionZ;
extern f32 gOverlay15PreviousCameraX;
extern f32 gOverlay15PreviousCameraY;
extern f32 gOverlay15PreviousCameraZ;
extern s32 gOverlay15MovingStarCount;
extern Overlay15MovingStarVec3f *gOverlay15MovingStars;
extern f32 gOverlay15MovingBound0;
extern f32 gOverlay15MovingBound1;
extern f32 gOverlay15MovingBound2;
extern f32 gOverlay15MovingBound3;
extern f32 gOverlay15MovingBound4;
extern f32 gOverlay15MovingBound5;
extern f32 gOverlay15MovingBound6;
extern f32 gOverlay15MovingBound7;
extern f32 gOverlay15MovingBound8;

extern Overlay15MovingStarCamera *overlay15GetActiveCameraReloc(void);
extern void starfieldFastMove(s32 count, Overlay15MovingStarVec3f *stars,
                              f32 movementX, f32 movementY, f32 movementZ,
                              f32 bound0, f32 bound1, f32 bound2,
                              f32 bound3, f32 bound4, f32 bound5,
                              f32 bound6, f32 bound7, f32 bound8);

/* Mickey-local reconstruction; pinned DKR v77/v80 and JFG scans are negative. */
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
        positionY *= scale;
        positionZ *= scale;
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
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o015/overlay15UpdateMovingStars/func_overlay_015_F00009E0_1872D78.s")
#endif
