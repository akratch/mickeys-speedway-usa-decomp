/*
 * Resident particle system -- ROM 0x3D5F0-0x43470.
 *
 * PROVENANCE: The translation-unit attribution and the non-placeholder names
 * below come from Jet Force Gemini's public decompilation, src/particles.c and
 * its built particles.c.o. Mickey's own function order, masked-skeleton
 * similarity, and call graph establish the correspondence. JFG address-based
 * placeholders are not imported; Mickey's existing placeholders remain.
 * Adapted bodies carry a point-of-use PROVENANCE note.
 */

#include "PR/ultratypes.h"

typedef struct ParticleConfig {
    s32 flags;
    f32 x;
    f32 y;
    f32 z;
    u8 pad10[4];
    s16 value14;
    s16 value16;
    s16 value18;
    u8 pad1A[8];
    s16 value22;
    s16 value24;
    s16 value26;
} ParticleConfig;

typedef struct ParticleTrigger {
    ParticleConfig *config;
    s32 flags;
    s16 unk08;
    s16 value;
    s16 unk0C;
    s16 value0E;
    s16 value10;
    s16 value12;
    s16 value14;
    s16 value16;
    s16 value18;
    s16 x;
    s16 y;
    s16 z;
    s16 unk20;
    u8 alpha;
    s8 index;
    u8 active;
    u8 pad25[3];
} ParticleTrigger;

typedef struct ParticleTriggerSlot {
    u8 pad00[4];
    s32 flags;
    u8 pad08[0x1C];
} ParticleTriggerSlot;

typedef struct ParticleObjectHeader {
    u8 pad00[0x25];
    s8 triggerCount;
} ParticleObjectHeader;

typedef struct ParticleObject {
    u8 pad00[0x40];
    ParticleObjectHeader *header;
    u8 pad44[0x28];
    ParticleTriggerSlot *triggers;
} ParticleObject;

typedef struct CircularParticle {
    u8 pad00[0x2C];
    s16 type;
    u8 kind;
    u8 pad2F;
    f32 x;
    f32 y;
    f32 z;
    u8 pad3C[0x0C];
    void *parent;
    u8 pad4C[0x2C];
} CircularParticle;

typedef struct CircularParticlePool {
    u8 pad00[0x14];
    CircularParticle *particles;
    s32 count;
} CircularParticlePool;

typedef struct ParticlePosition {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
} ParticlePosition;

extern f32 D_8007C8F8;
extern f32 D_8007C8F0;
extern f32 D_8007C8F4;
extern void *D_8007C89C[2];
extern s32 D_8007C8B0;
extern ParticleConfig **D_8007C8B8;
extern ParticleTrigger *D_8007C8BC;
extern s32 D_8007C8C0;
extern CircularParticlePool *D_800D4120[];
extern CircularParticlePool *D_800D4134[];

void mmFree(void *ptr);
void func_8003EC8C(ParticleObject *object, s32 index);
void partInitTriggerPos(ParticleTrigger *trigger, s32 type, s32 value, s16 x, s16 y, s16 z);
void func_8003CA20(void);
void func_8003CB3C(void);
void func_8003CCE4(void);

/* PROVENANCE: body adapted from DKR src/particles.c:reset_particles. */
void reset_particles(void) {
    func_8003CB3C();
    func_8003CCE4();
    func_8003CA20();
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003CA20.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003CB3C.s")
void func_8003CCE4(void) {
    if (D_8007C89C[0] != NULL) {
        mmFree(D_8007C89C[0]);
        D_8007C89C[0] = NULL;
        D_8007C89C[1] = NULL;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003CD28.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003CE10.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003D25C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003D4FC.s")
void partInitTrigger(ParticleTrigger *trigger, s32 type, s32 value) {
    ParticleConfig *config;

    if (type < D_8007C8B0) {
        config = D_8007C8B8[type];
        partInitTriggerPos(trigger, type, value, config->x, config->y, config->z);
    }
}
void partInitTriggerSPPos(ParticleTrigger *trigger, s32 type, s32 value, s32 index) {
    if (type < D_8007C8B0) {
        partInitTriggerPos(trigger, type, value, 0, 0, 0);
        trigger->index = index;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partInitTriggerPos.s")
ParticleTrigger *func_8003E730(s32 type, s32 value) {
    ParticleTrigger *trigger;
    s32 i;

    trigger = D_8007C8BC;
    i = 0;
    if (D_8007C8C0 > 0) {
        do {
            i++;
            if (trigger->active == 0) {
                trigger->active = 1;
                trigger->unk0C = 0;
                partInitTriggerPos(trigger, type, value, 0, 0, 0);
                return trigger;
            }
            trigger++;
        } while (i < D_8007C8C0);
    }
    return NULL;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003E7B8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003E8D8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003EB08.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003EC8C.s")
#ifdef NON_MATCHING
/* One-word plateau at +0x5C: the final bne's two operands are reversed. */
void partObjFreeTriggers(ParticleObject *object) {
    s32 i;
    s32 offset;
    s8 count;

    offset = 0;
    count = object->header->triggerCount;
    i = 0;
    if (count > 0) {
        do {
            if (((ParticleTriggerSlot *)((u8 *)object->triggers + offset))->flags & 0x8000) {
                func_8003EC8C(object, i);
            }
            i++;
            offset += sizeof(ParticleTriggerSlot);
            if (offset) {}
        } while (i != count);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partObjFreeTriggers.s")
#endif
/* PROVENANCE: body adapted from JFG src/particles.c:partAdjustScaling. */
void partAdjustScaling(f32 scale) {
    D_8007C8F8 = scale;
}
void func_8003EDD4(f32 value) {
    D_8007C8F0 = value;
}
void func_8003EDE0(f32 value) {
    D_8007C8F4 = value;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partUpdateTriggers.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003EF80.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003F154.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003F5F8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partModelObjEmitModelPart.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003FB98.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8004054C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80040740.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80040878.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80040B88.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041040.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041388.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041530.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041C50.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041CE4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041F48.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041FEC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_800420E0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_800421F4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8004233C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_800423EC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partUpdateParticles.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partDraw.s")
#ifdef NON_MATCHING
/* Size-exact plateau: 27 words differ from +0x0; IDO assigns both nested-loop
 * carrier pairs oppositely, and declaration-order variants are identical. */
void partNullifyCircularParticleParents(ParticlePosition *position) {
    CircularParticle *particle;
    CircularParticlePool *pool;
    s32 i;
    CircularParticlePool **poolPtr;

    poolPtr = D_800D4120;
    do {
        pool = *poolPtr;
        poolPtr++;
        i = 0;
        particle = pool->particles;
        if (pool->count > 0) {
            do {
                i++;
                if (particle->type != 0x80 && particle->kind == 3) {
                    particle->x += position->x;
                    particle->y += position->y;
                    particle->parent = NULL;
                    particle->z += position->z;
                }
                particle++;
            } while (i < pool->count);
        }
    } while (poolPtr != D_800D4134);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partNullifyCircularParticleParents.s")
#endif
