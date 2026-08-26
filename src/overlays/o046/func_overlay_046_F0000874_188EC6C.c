#include "PR/ultratypes.h"

typedef struct Overlay46Particle {
    s16 value00;
    s16 value02;
    s16 value04;
    s16 angle06;
    f32 scale08;
    f32 baseX0C;
    f32 baseY10;
    f32 value14;
    u16 angle18;
    s16 angle1A;
    f32 positionX1C;
    f32 positionY20;
    f32 progress24;
    f32 value28;
    f32 targetX2C;
    f32 targetY30;
    s16 angle34;
    s16 variant36;
    void *resource38;
} Overlay46Particle;

typedef struct Overlay46DisplayCommand {
    u32 w0;
    u32 w1;
} Overlay46DisplayCommand;

typedef union Overlay46StateC {
    s32 state;
    u8 bytes[4];
} Overlay46StateC;

extern s32 D_10;
extern s32 D_14;
extern s32 D_18;
extern u8 D_1C[];
extern Overlay46Particle D_20[];
extern u8 D_2C[];
extern f32 D_4C;
extern f32 D_50;
extern f32 D_54;
extern s32 D_5C;
extern Overlay46StateC D_C;

extern u8 gOverlay46RenderData0[];
extern u8 gOverlay46RenderData1[];
extern u8 gOverlay46RenderData2[];
extern void *gOverlay46ParticleModel;
extern void *gOverlay46ParticleMaterial;
extern Overlay46DisplayCommand *gDisplayListHead;

extern void overlay46RenderBeginReloc(void);
extern void overlay46RenderLoadReloc(void *data);
extern void overlay46RenderBindReloc(void *data0, void *data1);
extern s32 overlay46RandomRangeReloc(s32 minimum, s32 maximum);
extern f32 overlay46SinReloc(u16 angle);
extern void overlay46SetRenderModeReloc(s32 mode);
extern void overlay46SetColorReloc(s32 red, s32 green, s32 blue, s32 alpha);
extern void overlay46SetPrimColorReloc(s32 red, s32 green, s32 blue,
                                       s32 alpha, s32 intensity);
extern void overlay46DrawPanelReloc(void *data, s32 x, s32 y, void *state,
                                    s32 size);
extern void overlay46DrawPanelExReloc(void *data0, void *data1, s32 x, s32 y,
                                      s32 red, s32 green, s32 blue, s32 alpha);
extern void overlay46LoadParticleMaterialReloc(void *material);
extern void overlay46DrawParticleReloc(void *data, void *model,
                                       void *material,
                                       Overlay46Particle *particle,
                                       void *resource, s32 flags, s32 alpha);

/* Pinned DKR v77/v80 and JFG skeleton scans found no close donor. */
/* Workbench p4: structure-mismatch; 453/450 instructions, 363 positional words, first +0x0; frame 0xE0 vs target 0xC0.
 * Levers: prior constant/particle-base forms plus this run's step-lifetime/register probes; all retained the 32-byte frame excess or worsened structure.
 * Remains: switch-local FP/result stack allocation and overlay relocation identities. */
#ifdef NON_MATCHING
s32 func_overlay_046_F0000874_188EC6C(s32 updateRate) {
    s32 result;
    s32 finished;
    s32 count;
    s32 value;
    s32 fadeStep;
    u16 angle;
    f32 progress;
    f32 startX;
    f32 startY;
    Overlay46DisplayCommand *command;
    Overlay46Particle *particle;
    Overlay46Particle **slot;
    Overlay46Particle *particlesByVariant[19];

    overlay46RenderBeginReloc();
    result = 1;
    overlay46RenderLoadReloc(gOverlay46RenderData0);
    overlay46RenderBindReloc(gOverlay46RenderData1, gOverlay46RenderData2);

    count = 0x12;
    slot = &particlesByVariant[18];
    do {
        *slot-- = NULL;
    } while (count--);

    switch (D_C.state) {
    case 1: {
        f32 step;

        particle = D_20;
        finished = 1;
        count = 0x12;
        step = (f32)updateRate * D_4C;
        do {
            particle->progress24 += step;
            if (particle->progress24 >= 1.0f) {
                particle->progress24 = 1.0f;
            } else {
                finished = 0;
            }
            progress = particle->progress24;
            startX = particle->positionX1C;
            startY = particle->positionY20;
            particle->baseX0C =
                startX + ((particle->targetX2C - startX) * progress);
            particle->baseY10 =
                startY + ((particle->targetY30 - startY) * progress);
            particle->value04 = (s16)(2.0f * (progress * 65536.0f));
            particlesByVariant[particle->variant36] = particle;
            particle++;
        } while (count--);

        if (finished != 0) {
            D_C.state = 2;
            particle = D_20;
            count = 0x12;
            do {
                particle->progress24 = 0.0f;
                particle++;
            } while (count--);
        }
        break;
    }

    case 2: {
        f32 step;

        particle = D_20;
        finished = 1;
        count = 0x12;
        step = (f32)updateRate * D_50;
        do {
            particle->progress24 += step;
            if (particle->progress24 >= 1.0f) {
                particle->progress24 = 1.0f;
            } else {
                finished = 0;
            }

            angle = particle->angle18 + (particle->angle1A * updateRate);
            particle->angle18 = angle;
            if (angle >= 0x8001) {
                particle->angle18 = angle - 0x8000;
                particle->angle1A = overlay46RandomRangeReloc(0x600, 0xA00);
            }

            particle->angle34 += particle->angle06 * updateRate;
            if (particle->angle34 < -0x1000) {
                particle->angle34 = -0x1000;
                particle->angle06 = overlay46RandomRangeReloc(0x100, 0x200);
            } else if (particle->value04 >= 0x1001) {
                particle->angle34 = 0x1000;
                particle->angle06 =
                    -overlay46RandomRangeReloc(0x100, 0x200);
            }

            particle->baseX0C = particle->targetX2C;
            particle->baseY10 =
                (overlay46SinReloc(particle->angle18) *
                 particle->progress24 * 5.0f) + particle->targetY30;
            particle->value04 =
                (s16)((f32)particle->angle34 * particle->progress24);
            particlesByVariant[particle->variant36] = particle;
            particle++;
        } while (count--);

        if (finished != 0) {
            fadeStep = updateRate * 4;
            if (D_10 < 0xFF) {
                value = D_10 + fadeStep;
                D_10 = value;
                if (value >= 0x100) {
                    D_10 = 0xFF;
                }
            } else if (D_14 < 0xFF) {
                value = D_14 + fadeStep;
                D_14 = value;
                if (value >= 0x100) {
                    D_14 = 0xFF;
                }
            } else {
                value = D_18 + fadeStep;
                if (D_18 < 0xFE) {
                    D_18 = value;
                    if (value >= 0xFF) {
                        D_18 = 0xFE;
                    }
                } else {
                    value = D_5C - updateRate;
                    D_5C = value;
                    if (value <= 0) {
                        D_C.state = 4;
                    }
                }
            }
        }
        break;
    }

    case 4: {
        f32 step;

        particle = D_20;
        finished = 1;
        count = 0x12;
        step = (f32)updateRate * D_54;
        do {
            particle->progress24 -= step;
            if (particle->progress24 <= 0.0f) {
                particle->progress24 = 0.0f;
            } else {
                finished = 0;
            }
            progress = particle->progress24;
            startX = particle->positionX1C;
            startY = particle->positionY20;
            particle->baseX0C =
                startX + ((particle->targetX2C - startX) * progress);
            particle->baseY10 =
                startY + ((particle->targetY30 - startY) * progress);
            particle->value04 =
                particle->angle34 + (s32)(2.0f * (progress * 65536.0f));
            particlesByVariant[particle->variant36] = particle;
            particle++;
        } while (count--);

        fadeStep = updateRate * 8;
        value = D_10 - fadeStep;
        D_10 = value;
        if (value < 0) {
            D_10 = 0;
        }
        value = D_14 - fadeStep;
        D_14 = value;
        if (value < 0) {
            D_14 = 0;
        }
        value = D_18 - fadeStep;
        D_18 = value;
        if (value < 0) {
            D_18 = 0;
        }
        if (finished != 0) {
            result = 0;
        }
        break;
    }
    }

    overlay46SetRenderModeReloc(2);
    overlay46SetColorReloc(0, 0, 0, 0);
    value = D_10;
    if (value != 0) {
        overlay46SetPrimColorReloc(0xFF, 0xFF, 0xFF, 0xFF, value);
        overlay46DrawPanelReloc(gOverlay46RenderData0, 0xA0, 0xAC, &D_C,
                                0xC);
    }
    value = D_14;
    if (value != 0) {
        overlay46SetPrimColorReloc(0xFF, 0xFF, 0xFF, 0xFF, value);
        overlay46DrawPanelReloc(gOverlay46RenderData0, 0xA0, 0xB6, D_1C,
                                0xC);
    }
    value = D_18;
    if (value != 0) {
        overlay46DrawPanelExReloc(gOverlay46RenderData0, D_2C, 0xA0, 0xCC,
                                  0xFF, 0xFF, 0xFF, value);
    }

    overlay46LoadParticleMaterialReloc(gOverlay46ParticleMaterial);
    command = gDisplayListHead++;
    command->w0 = 0xFA000000;
    command->w1 = 0xFFFFFFFF;

    slot = particlesByVariant;
    do {
        particle = *slot++;
        if (particle != NULL) {
            overlay46DrawParticleReloc(
                gOverlay46RenderData0, gOverlay46ParticleModel,
                gOverlay46ParticleMaterial, particle, particle->resource38,
                0x8001, 0xFF);
        }
    } while (slot != &particlesByVariant[19]);

    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o046/func_overlay_046_F0000874_188EC6C/func_overlay_046_F0000874_188EC6C.s")
#endif
