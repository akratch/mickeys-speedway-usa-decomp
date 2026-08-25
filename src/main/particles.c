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
#include "n_audio/mbi.h"

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
    ParticleConfig *config;
    s32 flags;
    u8 unk08;
    u8 unk09;
    s16 type;
    s16 unk0C;
    s16 value0E;
    s16 value10;
    s16 value12;
    s16 value14;
    s16 value16;
    s16 value18;
    s16 value1A;
    s16 value1C;
    s16 value1E;
    s16 value20;
    s8 result;
    s8 index;
} ParticleTriggerSlot;

typedef struct ParticleObjectHeader {
    u8 pad00[0x25];
    s8 triggerCount;
} ParticleObjectHeader;

typedef struct ParticleObject {
    u8 pad00[0x1A];
    s16 activeTriggerCount;
    u8 pad1C[0x24];
    ParticleObjectHeader *header;
    u8 pad44[0x28];
    ParticleTriggerSlot *triggers;
} ParticleObject;

typedef struct ParticleResourceEntry {
    u8 pad00[0x2C];
    s16 type;
    u8 pad2E[0x1E];
    void *resource;
    u8 pad50[0x28];
} ParticleResourceEntry;

typedef struct ParticleResourceList {
    u8 pad00[0x14];
    ParticleResourceEntry *entries;
    s32 count;
} ParticleResourceList;

typedef struct ParticleModelEntry {
    void *particles[15];
    s32 particleCount;
    u8 pad40[0x64];
    u8 active;
    u8 padA5[0x1B];
} ParticleModelEntry;

typedef struct ParticleLineEntry {
    u8 pad00[0x124];
    u8 active;
    u8 pad125[3];
    void *texture;
    ParticleConfig *config;
    s32 unk130;
    u8 pad134[4];
    s32 descriptorWord;
    s32 configFlags;
    f32 textureFrame;
    s16 value144;
    u8 pad146[2];
} ParticleLineEntry;

typedef struct ParticleTexture {
    u8 pad00[0x10];
    u16 frameCount;
} ParticleTexture;

typedef struct ParticleModelPartConfig {
    s16 type;
    s16 triggerType;
    s16 particleType;
} ParticleModelPartConfig;

typedef struct ParticleModelObjectHeader {
    u8 pad00[0xB0];
    ParticleModelPartConfig *parts;
} ParticleModelObjectHeader;

typedef struct ParticleModelObject {
    u8 pad00[0x40];
    ParticleModelObjectHeader *header;
    u8 pad44[0x38];
    ParticleTriggerSlot *triggers;
    u8 pad80[0x0D];
    u8 triggerCount;
} ParticleModelObject;

typedef struct ParticleTypeDescriptor {
    u8 pad00[4];
    s32 flags;
} ParticleTypeDescriptor;

typedef struct BasicParticle {
    s16 rotationY;
    s16 rotationX;
    s16 rotationZ;
    u8 pad06[2];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    f32 scaleVelocity;
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    u8 pad28[8];
    f32 localX;
    f32 localY;
    f32 localZ;
    f32 movementValue;
    f32 gravity;
    u8 pad44[4];
    void *parent;
    u8 pad4C[0x12];
    s16 angularVelocityY;
    s16 angularVelocityX;
    s16 angularVelocityZ;
} BasicParticle;

typedef struct ParticleParent {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
} ParticleParent;

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
    void *resource;
    u8 pad50[8];
    ParticleTrigger *trigger;
    u8 pad5C[0x1C];
} CircularParticle;

typedef struct CircularParticlePool {
    u8 pad00[0x14];
    CircularParticle *particles;
    s32 count;
    u32 *freeBits;
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
extern f32 D_80082A4C;
extern void **D_8007C884;
extern s32 D_8007C888;
extern s32 D_8007C88C;
extern ParticleLineEntry *D_8007C894;
extern void *D_8007C89C[2];
extern s32 D_8007C8B0;
extern ParticleConfig **D_8007C8B8;
extern ParticleTrigger *D_8007C8BC;
extern s32 D_8007C8C0;
extern s32 D_8007C890;
extern s32 D_8007C8E8;
extern s32 D_8007C8EC;
extern ParticleModelEntry *D_8007C898;
extern ParticleTypeDescriptor **D_8007C8AC;
extern s32 D_800D4140;
extern CircularParticlePool *D_800D4120[];
extern CircularParticlePool *D_800D4124;
extern CircularParticlePool *D_800D4128;
extern CircularParticlePool *D_800D412C;
extern CircularParticlePool *D_800D4130[];
extern CircularParticlePool *D_800D4134[];

void mmFree(void *ptr);
void func_800347A0(void *resource);
void func_800359D4(void *resource);
void modFreeModel(void *resource);
void mathOneFloatPY(void *rotation, void *vector);
void pointListRPY(s32 count, s16 *rotation, f32 *input, f32 *output);
void *piRomLoad(s32 assetId);
ParticleTexture *func_80034448(s16 resourceId);
s32 mathRnd(s32 minimum, s32 maximum);
void camSetNo(s32 camera);
void func_800221E8(void **dList, s32 arg1);
void func_8003D4FC(void **dList, void **vertices, void *pool);
s32 func_8003CE10(void **dList, s32 arg1, void **vertices, void *pool, s32 mode);
void func_8003D25C(void **dList, s32 arg1, void **vertices, void *pool);
void func_80041CE4(void **dList, void **vertices);
void func_80040878(CircularParticle *particle, s32 updateRate);
void func_80041040(ParticleLineEntry *particle, s32 updateRate);
void func_80041388(ParticleModelEntry *particle, s32 updateRate);
void func_8003EC8C(ParticleObject *object, s32 index);
void func_8003E7B8(ParticleObject *object, s32 index);
s8 func_8003E8D8(ParticleTypeDescriptor *descriptor, ParticleConfig *config, ParticleTriggerSlot *trigger);
s32 func_8003EB08(ParticleTypeDescriptor *descriptor, ParticleConfig *config);
void partInitTriggerPos(ParticleTrigger *trigger, s32 type, s32 value, s16 x, s16 y, s16 z);
void func_8003CA20(void);
void func_8003CB3C(void);
void func_8003CCE4(void);
void *func_8003FB98(s32 arg0, ParticleTrigger *trigger, s32 arg2);
void func_80041530(s32 arg0, s32 arg1, ParticleModelEntry *entry);

/* PROVENANCE: body adapted from DKR src/particles.c:reset_particles. */
void reset_particles(void) {
    func_8003CB3C();
    func_8003CCE4();
    func_8003CA20();
}
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/func_8005D930.s; body reconstructed from Mickey evidence. */
void func_8003CA20(void) {
    s32 pad;
    s16 *types;
    s16 *typePtr;
    s32 i;
    s32 count;

    if (D_8007C884 != NULL) {
        types = piRomLoad(0x3A);
        count = D_8007C888;
        i = 0;
        typePtr = types;
        if (count > 0) {
            do {
                switch (*typePtr & 0xC000) {
                    case 0x8000:
                        func_800359D4(D_8007C884[i]);
                        break;
                    case 0xC000:
                        func_800347A0(D_8007C884[i]);
                        break;
                    case 0:
                        modFreeModel(D_8007C884[i]);
                        break;
                }
                i++;
                typePtr++;
                count = D_8007C888;
            } while (i < count);
        }
        mmFree(types);
        mmFree(D_8007C884);
        D_8007C884 = NULL;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003CB3C.s")
void func_8003CCE4(void) {
    if (D_8007C89C[0] != NULL) {
        mmFree(D_8007C89C[0]);
        D_8007C89C[0] = NULL;
        D_8007C89C[1] = NULL;
    }
}
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/func_8005DC7C.s; body reconstructed from Mickey evidence. */
void func_8003CD28(ParticleResourceList **listPtr) {
    ParticleResourceList *list;
    ParticleResourceEntry *entry;
    s32 i;

    list = *listPtr;
    if (list != NULL) {
        entry = list->entries;
        i = 0;
        if (list->count > 0) {
            do {
                switch (entry->type) {
                    case 2:
                        func_800359D4(entry->resource);
                        break;
                    case 0:
                    case 1:
                        func_800347A0(entry->resource);
                        break;
                    case 3:
                        modFreeModel(entry->resource);
                        break;
                }
                i++;
                entry++;
            } while (i < list->count);
        }
        mmFree(list);
        *listPtr = NULL;
    }
}
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
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/partInitTriggerPos.s; body reconstructed from Mickey evidence. */
void partInitTriggerPos(ParticleTrigger *trigger, s32 type, s32 value, s16 x, s16 y, s16 z) {
    s32 flags;
    ParticleConfig *config;

    config = D_8007C8B8[type];
    trigger->value = value;
    trigger->unk0C = 0;
    trigger->x = x;
    trigger->config = config;
    trigger->y = y;
    trigger->z = z;
    trigger->unk20 = 0;
    trigger->index = -1;
    flags = config->flags;
    if (flags & 0x4000) {
        trigger->flags = 0x4000;
        trigger->alpha = 0xFF;
        return;
    }
    if (flags & 0x400) {
        trigger->flags = 0x400;
        trigger->alpha = 0xFF;
        return;
    }
    trigger->flags = 0;
    trigger->value0E = config->value14;
    trigger->value10 = config->value16;
    trigger->value12 = config->value18;
    trigger->value14 = config->value22;
    trigger->value16 = config->value24;
    trigger->value18 = config->value26;
    trigger->alpha = 0;
}
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
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/func_8005F99C.s; body reconstructed from Mickey evidence. */
void func_8003E7B8(ParticleObject *object, s32 index) {
    ParticleConfig *config;
    ParticleTriggerSlot *trigger;
    ParticleTypeDescriptor *descriptor;
    s32 flags;

    trigger = &object->triggers[index];
    descriptor = D_8007C8AC[trigger->type];
    trigger->unk0C = 0;
    config = trigger->config;
    if (trigger->flags & 0x4000) {
        trigger->result = func_8003EB08(descriptor, config);
    } else {
        if (config->flags & 1) {
            trigger->unk08 = 0;
            trigger->value0E = config->value14;
            trigger->value10 = config->value16;
            trigger->value12 = config->value18;
        }
        if (config->flags & 4) {
            trigger->unk09 = 0;
            trigger->value14 = config->value22;
            trigger->value16 = config->value24;
            trigger->value18 = config->value26;
        }
    }
    flags = trigger->flags;
    if (flags & 0x400) {
        trigger->result = func_8003E8D8(descriptor, config, trigger);
        flags = trigger->flags;
    }
    trigger->flags = flags | 0x8000;
    object->activeTriggerCount++;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003E8D8.s")
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/func_8005FD34.s; body reconstructed from Mickey evidence. */
s32 func_8003EB08(ParticleTypeDescriptor *descriptor, ParticleConfig *config) {
    s32 result;
    s32 i;
    ParticleTexture *texture;
    ParticleLineEntry *line;
    s32 frameCount;

    if (D_8007C894 == NULL) {
        return 0xFF;
    }
    line = D_8007C894;
    result = 0xFF;
    i = 0;
    if (D_8007C88C > 0) {
        do {
            if (line->active == 0) {
                line->active = 2;
                line->config = config;
                line->unk130 = 0;
                result = i;
                i = D_8007C88C;
            } else {
                line++;
            }
            i++;
        } while (i < D_8007C88C);
    }
    if (result != 0xFF) {
        line->descriptorWord = *(s32 *)descriptor;
        line->configFlags = config->flags;
        if (*(s16 *)((u8 *)descriptor + 6) == -1) {
            line->texture = NULL;
            line->value144 = 0;
            line->textureFrame = 0.0f;
        } else {
            texture = func_80034448(*(s16 *)((u8 *)descriptor + 6));
            line->texture = texture;
            line->value144 = *(s16 *)((u8 *)descriptor + 8);
            frameCount = texture->frameCount >> 8;
            if (config->flags & 0x800) {
                line->textureFrame = mathRnd(0, frameCount - 1);
            } else if ((line->descriptorWord & 1) == 2) {
                line->textureFrame = frameCount - D_80082A4C;
            } else {
                line->textureFrame = 0.0f;
            }
        }
    }
    return result;
}
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
#ifdef NON_MATCHING
/*
 * Size-exact plateau: IDO schedules the trigger-stride shift ahead of the
 * descriptor-table load and then reorders the initialization stores.
 * PROVENANCE: structure cross-checked against JFG
 * asm/nonmatchings/particles/partModelObjEmitModelPart.s; body reconstructed
 * from Mickey evidence.
 */
void partModelObjEmitModelPart(ParticleModelObject *object, f32 velocityX, f32 velocityY, f32 velocityZ, s32 index) {
    s32 offset;
    u8 *trigger;
    ParticleConfig *config;
    BasicParticle *particle;

    if (index < object->triggerCount) {
        offset = index * sizeof(ParticleModelPartConfig);
        trigger = (u8 *)object->triggers + (index * sizeof(ParticleTriggerSlot));
        config = D_8007C8B8[((ParticleModelPartConfig *)((u8 *)object->header->parts + offset))->type];
        *(ParticleConfig **)(trigger + 0x00) = config;
        *(s32 *)(trigger + 0x04) = 0;
        *(s16 *)(trigger + 0x0C) = 0;
        *(s16 *)(trigger + 0x1A) = 0;
        *(s16 *)(trigger + 0x1C) = 0;
        *(s16 *)(trigger + 0x1E) = 0;
        *(s16 *)(trigger + 0x20) = 0;
        *(s16 *)(trigger + 0x0A) =
            ((ParticleModelPartConfig *)((u8 *)object->header->parts + offset))->triggerType;
        *(s16 *)(trigger + 0x0E) = config->value14;
        *(s16 *)(trigger + 0x10) = config->value16;
        *(s16 *)(trigger + 0x12) = config->value18;
        *(s16 *)(trigger + 0x14) = config->value22;
        *(s16 *)(trigger + 0x16) = config->value24;
        *(s8 *)(trigger + 0x23) = -1;
        *(s16 *)(trigger + 0x18) = config->value26;
        particle = (BasicParticle *)func_8003FB98(
            (s32)object, (ParticleTrigger *)trigger,
            ((ParticleModelPartConfig *)((u8 *)object->header->parts + offset))->particleType);
        if (particle != NULL && (velocityX != 0.0f || velocityY != 0.0f || velocityZ != 0.0f)) {
            particle->velocityX = velocityX;
            particle->velocityY = velocityY;
            particle->velocityZ = velocityZ;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partModelObjEmitModelPart.s")
#endif
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003FB98.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8004054C.s")
#ifdef NON_MATCHING
/*
 * Text-exact plateau: the compiler emits a duplicate jump table rather than
 * binding the separately extracted jtbl_80082A58 resident-rodata table.
 * PROVENANCE: structure cross-checked against JFG
 * asm/nonmatchings/particles/func_80061B50.s; body reconstructed from Mickey
 * evidence.
 */
void func_80040740(CircularParticle *particle) {
    CircularParticlePool *pool;
    s32 index;

    if (particle->type >= 5) {
        if (particle->type == 0x80) {}
        return;
    }
    switch (particle->type) {
        case 2:
            if (particle->resource != NULL) {
                func_800359D4(particle->resource);
                particle->resource = NULL;
            }
            break;
        case 3:
            if (particle->resource != NULL) {
                modFreeModel(particle->resource);
                particle->resource = NULL;
            }
            break;
        case 0:
        case 1:
        case 4:
            break;
    }
    if (particle->trigger != NULL) {
        particle->trigger->active = 0;
    }
    pool = D_800D4120[particle->type];
    if (pool->count > 0) {
        if (particle->resource != NULL) {
            func_800347A0(particle->resource);
        }
        pool->count--;
        particle->type = 0x80;
        index = (particle - pool->particles);
        pool->freeBits[index >> 5] |= 1 << (index & 0x1F);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80040740.s")
#endif
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80040878.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80040B88.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041040.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041388.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041530.s")
void func_80041C50(s32 arg0, s32 arg1) {
    ParticleModelEntry *entry;
    s32 i;

    if (D_8007C898 != NULL) {
        entry = D_8007C898;
        i = 0;
        if (D_8007C890 > 0) {
            do {
                if (entry->active != 0) {
                    func_80041530(arg0, arg1, entry);
                }
                i++;
                entry++;
            } while (i < D_8007C890);
        }
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041CE4.s")
void func_80041F48(s32 arg0, ParticleTrigger *trigger) {
    void *particle;
    ParticleModelEntry *entry;
    ParticleTypeDescriptor *descriptor;
    u8 index;

    descriptor = D_8007C8AC[trigger->value];
    index = trigger->alpha;
    if (index != 0xFF && ((u32)descriptor->flags >> 28) == 4) {
        entry = &D_8007C898[index];
        if (entry->particleCount != 15) {
            particle = func_8003FB98(arg0, trigger, -1);
            if (particle != NULL) {
                entry->particles[entry->particleCount] = particle;
                entry->particleCount++;
            }
        }
    }
}
/* PROVENANCE: body adapted from DKR src/particles.c:move_particle_basic_parent. */
void func_80041FEC(BasicParticle *particle) {
    s32 i;
    ParticleParent *parent;

    i = D_800D4140;
    while (i-- > 0) {
        particle->localX += particle->velocityX;
        particle->localY += particle->velocityY;
        particle->velocityY -= particle->gravity;
        particle->localZ += particle->velocityZ;
        particle->rotationY += particle->angularVelocityY;
        particle->rotationX += particle->angularVelocityX;
        particle->rotationZ += particle->angularVelocityZ;
        particle->scale += particle->scaleVelocity;
    }
    particle->x = particle->localX;
    particle->y = particle->localY;
    particle->z = particle->localZ;
    parent = particle->parent;
    if (parent != NULL) {
        particle->x += parent->x;
        particle->y += parent->y;
        particle->z += parent->z;
    }
}
/* PROVENANCE: body adapted from DKR src/particles.c:move_particle_attached_to_parent. */
void func_800420E0(BasicParticle *particle) {
    s32 i = D_800D4140;
    ParticleParent *parent;

    while (i-- > 0) {
        particle->rotationY += particle->angularVelocityY;
        particle->rotationX += particle->angularVelocityX;
        particle->rotationZ += particle->angularVelocityZ;
        particle->scale += particle->scaleVelocity;
    }

    particle->x = 0.0f;
    particle->y = -particle->movementValue;
    particle->z = 0.0f;
    pointListRPY(1, (s16 *)particle, &particle->x, &particle->x);
    particle->x += particle->localX;
    particle->y += particle->localY;
    particle->z += particle->localZ;

    parent = particle->parent;
    if (parent != NULL) {
        particle->x += parent->x;
        particle->y += parent->y;
        particle->z += parent->z;
    }
}
/* PROVENANCE: body adapted from DKR src/particles.c:move_particle_with_acceleration. */
void func_800421F4(BasicParticle *particle) {
    s32 i = 0;
    f32 acceleration[3];

    while (i++ < D_800D4140) {
        particle->x += particle->velocityX;
        particle->y += particle->velocityY;
        particle->z += particle->velocityZ;
        particle->scale += particle->scaleVelocity;
        particle->rotationY += particle->angularVelocityY;
        particle->rotationX += particle->angularVelocityX;
        particle->rotationZ += particle->angularVelocityZ;

        acceleration[0] = 0.0f;
        acceleration[1] = -particle->movementValue;
        acceleration[2] = 0.0f;
        pointListRPY(1, (s16 *)particle, acceleration, acceleration);
        particle->velocityX += acceleration[0];
        particle->velocityY += acceleration[1];
        particle->velocityY -= particle->gravity;
        particle->velocityZ += acceleration[2];
    }
}
/* PROVENANCE: body adapted from DKR src/particles.c:move_particle_basic. */
void func_8004233C(BasicParticle *particle) {
    s32 i = 0;

    while (i++ < D_800D4140) {
        particle->x += particle->velocityX;
        particle->y += particle->velocityY;
        particle->velocityY -= particle->gravity;
        particle->z += particle->velocityZ;
        particle->scale += particle->scaleVelocity;
        particle->rotationY += particle->angularVelocityY;
        particle->rotationX += particle->angularVelocityX;
        particle->rotationZ += particle->angularVelocityZ;
    }
}
/* PROVENANCE: body adapted from DKR src/particles.c:move_particle_forward. */
void func_800423EC(BasicParticle *particle) {
    s32 i = 0;

    while (i++ < D_800D4140) {
        particle->velocityX = 0.0f;
        particle->velocityY = 0.0f;
        particle->velocityZ = -particle->movementValue;
        mathOneFloatPY(particle, &particle->velocityX);
        particle->x += particle->velocityX;
        particle->y += particle->velocityY - particle->gravity;
        particle->z += particle->velocityZ;
        particle->scale += particle->scaleVelocity;
        particle->rotationY += particle->angularVelocityY;
        particle->rotationX += particle->angularVelocityX;
        particle->rotationZ += particle->angularVelocityZ;
    }
}
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/partUpdateParticles.s; body reconstructed from Mickey evidence. */
void partUpdateParticles(s32 updateRate) {
    CircularParticlePool **poolPtr;
    CircularParticlePool **poolEnd;
    CircularParticlePool *pool;
    CircularParticle *particle;
    ParticleLineEntry *line;
    ParticleModelEntry *pointStream;
    s32 i;

    poolPtr = D_800D4120, poolEnd = D_800D4130;
    do {
        pool = *poolPtr;
        if (pool != NULL) {
            particle = pool->particles;
            i = 0;
            if (pool->count > 0) {
                do {
                    if (particle->type != 0x80) {
                        func_80040878(particle, updateRate);
                    }
                    i++;
                    particle++;
                } while (i < pool->count);
            }
        }
        poolPtr++;
    } while (poolPtr < poolEnd);

    if (D_8007C894 != NULL) {
        line = D_8007C894;
        i = 0;
        if (D_8007C88C > 0) {
            do {
                if (line->active != 0) {
                    func_80041040(line, updateRate);
                }
                i++;
                line++;
            } while (i < D_8007C88C);
        }
    }

    if (D_8007C898 != NULL) {
        pointStream = D_8007C898;
        i = 0;
        if (D_8007C890 > 0) {
            do {
                if (pointStream->active != 0) {
                    func_80041388(pointStream, updateRate);
                }
                i++;
                pointStream++;
            } while (i < D_8007C890);
        }
    }
}
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/partDraw.s; body reconstructed from Mickey evidence. */
void partDraw(Gfx **dList, s32 arg1, s32 mode) {
    void *vertices;
    s32 pad;

    if (mode != 0) {
        D_8007C8EC = 0;
    }
    vertices = (u8 *)D_8007C89C[D_8007C8E8] + (D_8007C8EC * 10);
    gDPPipeSync((*dList)++);
    if (mode == 1) {
        D_8007C8EC = func_8003CE10((void **)dList, arg1, &vertices, D_800D4128, 1);
        return;
    }
    camSetNo(0);
    func_800221E8((void **)dList, arg1);
    func_8003D4FC((void **)dList, &vertices, D_800D4120[0]);
    func_8003D4FC((void **)dList, &vertices, D_800D4124);
    func_80041CE4((void **)dList, &vertices);
    func_80041C50((s32)dList, (s32)&vertices);
    func_8003CE10((void **)dList, arg1, &vertices, D_800D4128, mode);
    func_8003D25C((void **)dList, arg1, &vertices, D_800D412C);
    D_8007C8E8 ^= 1;
}
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
