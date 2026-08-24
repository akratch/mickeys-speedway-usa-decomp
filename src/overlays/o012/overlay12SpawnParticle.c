#include "PR/ultratypes.h"

typedef struct Overlay12Particle {
    f32 x;
    f32 y;
    f32 z;
    f32 velocity;
    u8 pad10[4];
    u8 type;
    u8 variant;
    u8 active;
    u8 pad17;
} Overlay12Particle;

extern s32 gOverlay12Ready;
extern s32 gOverlay12ParticleCount;
extern Overlay12Particle gOverlay12Particles[5];
extern void overlay12Initialize(void);

/* Pinned DKR v77/v80 and JFG object scans found no donor. */
#ifdef NON_MATCHING
void func_overlay_012_F00002E4_186D564(f32 x, f32 y, f32 z, s32 type,
                                       s32 variant) {
    s32 i;
    volatile s32 *count;
    Overlay12Particle *particle;

    if (gOverlay12Ready == 0) {
        overlay12Initialize();
    }
    count = &gOverlay12ParticleCount;
    if (*count < 5) {
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
            *count = *count + 1;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o012/overlay12SpawnParticle/func_overlay_012_F00002E4_186D564.s")
#endif
