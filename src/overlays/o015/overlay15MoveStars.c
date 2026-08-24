#include "PR/ultratypes.h"

typedef struct Overlay15Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Overlay15Vec3f;

extern s32 gOverlay15StarCount;
extern Overlay15Vec3f *gOverlay15Stars;
extern f32 gOverlay15StarBound0;
extern f32 gOverlay15StarBound1;
extern f32 gOverlay15StarBound2;
extern f32 gOverlay15StarBound3;
extern f32 gOverlay15StarBound4;
extern f32 gOverlay15StarBound5;
extern f32 gOverlay15StarBound6;
extern f32 gOverlay15StarBound7;
extern f32 gOverlay15StarBound8;
extern Overlay15Vec3f gOverlay15StarMovement;

extern void starfieldFastMove(s32 count, Overlay15Vec3f *stars,
                              f32 movementX, f32 movementY, f32 movementZ,
                              f32 bound0, f32 bound1, f32 bound2,
                              f32 bound3, f32 bound4, f32 bound5,
                              f32 bound6, f32 bound7, f32 bound8);

/*
 * Mickey-local reconstruction; pinned DKR v77/v80 and JFG scans are negative.
 */
void overlay15MoveStars(f32 movementX, f32 movementY, f32 movementZ,
                        s32 rate) {
    f32 scale;

    gOverlay15StarMovement.x = movementX;
    gOverlay15StarMovement.y = movementY;
    gOverlay15StarMovement.z = movementZ;
    if (gOverlay15Stars != 0) {
        scale = (f32)rate;
        movementX *= scale;
        movementY *= scale;
        movementZ *= scale;
        starfieldFastMove(gOverlay15StarCount, gOverlay15Stars,
                          movementX, movementY, movementZ,
                          gOverlay15StarBound0, gOverlay15StarBound1,
                          gOverlay15StarBound2, gOverlay15StarBound3,
                          gOverlay15StarBound4, gOverlay15StarBound5,
                          gOverlay15StarBound6, gOverlay15StarBound7,
                          gOverlay15StarBound8);
    }
}
