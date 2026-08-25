#include "overlays/overlay_012.h"

/* Pinned DKR v77/v80 and JFG object scans found no donor. */
void func_overlay_012_F00002E4_186D564(f32 x, f32 y, f32 z, s32 type,
                                       s32 variant) {
    s32 i;
    Overlay12Particle *particle;

    if (gOverlay12Ready == 0) {
        overlay12Initialize();
    }
    if (gOverlay12ParticleCount < 5) {
        particle = gOverlay12Particles;
        for (i = 0; i < 5; i++, particle++) {
            if (particle->active == 0) {
                break;
            }
        }
        if (i < 5) {
            particle->x = x;
            particle->y = y;
            particle->z = z;
            particle->type = type;
            particle->variant = variant;
            particle->active = 1;
            particle->velocity = 0.0f;
            gOverlay12ParticleCount++;
        }
    }
}
