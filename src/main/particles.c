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
    u8 pad28[0x18];
    s16 value40;
    s16 value42;
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
    u8 pad70[0x10];
    u32 triggerBits;
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
    u8 padA5[3];
    s32 animationState;
    s32 configFlags;
    f32 textureFrame;
    s16 animationSpeed;
    u8 padB6[2];
    void *resource;
    ParticleTriggerSlot *trigger;
} ParticleModelEntry;

typedef struct ParticlePointStreamEntry {
    u8 pad00[0x40];
    s32 pointCount;
    f32 points[8][3];
    u8 active;
    u8 padA5[3];
    s32 animationState;
    s32 configFlags;
    f32 textureFrame;
    s16 animationSpeed;
    u8 padB6[2];
    void *resource;
    ParticleTriggerSlot *trigger;
} ParticlePointStreamEntry;

typedef struct ParticleLinePoint ParticleLinePoint;
typedef struct ParticleTexture ParticleTexture;

typedef struct ParticleLineEntry {
    s32 pointCount;
    ParticleLinePoint *points[15];
    u8 pad40[0xE4];
    u8 active;
    u8 pad125[3];
    ParticleTexture *texture;
    ParticleConfig *config;
    union {
        u32 *colorTable;
        s32 unk130;
    };
    s16 colorCount;
    u8 pad136[2];
    s32 descriptorWord;
    s32 configFlags;
    f32 textureFrame;
    s16 value144;
    u8 pad146[2];
} ParticleLineEntry;

struct ParticleTexture {
    u8 pad00[0x10];
    u16 frameCount;
};

struct ParticleLinePoint {
    s16 x0;
    s16 y0;
    s16 z0;
    s16 x1;
    s16 y1;
    s16 z1;
    s16 intensityTimer;
    s16 intensity;
    s16 colorTimer;
    s16 colorIndex;
    s16 lifetime;
    s16 intensityVelocity;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
};

typedef struct ParticleLineVertex {
    s16 x0;
    s16 y0;
    s16 z0;
    u8 red0;
    u8 green0;
    u8 blue0;
    u8 alpha0;
    s16 x1;
    s16 y1;
    s16 z1;
    u8 red1;
    u8 green1;
    u8 blue1;
    u8 alpha1;
} ParticleLineVertex;

typedef struct ParticleLineRenderEntry {
    s32 pointCount;
    ParticleLinePoint *points[15];
    u8 pad40[0xE4];
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
} ParticleLineRenderEntry;

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
    s32 descriptorWord;
    union {
        s32 flags;
        struct {
            u8 pad04;
            u8 pointCount;
            s16 resourceId;
        };
    };
    s16 animationSpeed;
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

typedef struct EmittedParticle {
    u8 pad00[0x1C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad28[0x28];
    s32 flags;
} EmittedParticle;

typedef struct ParticleParent {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
} ParticleParent;

typedef struct CircularParticle {
    s16 rotationY;
    s16 rotationX;
    s16 rotationZ;
    u8 pad06[2];
    f32 scale;
    f32 renderX;
    f32 renderY;
    f32 renderZ;
    u8 pad18[0x10];
    f32 textureFrame;
    s16 type;
    u8 kind;
    u8 pad2F;
    f32 x;
    f32 y;
    f32 z;
    u8 pad3C[8];
    u8 *colorTable;
    void *parent;
    void *resource;
    s32 flags;
    u8 pad54[4];
    ParticleTrigger *trigger;
    s16 lifetime;
    u8 pad5E[6];
    s16 intensity;
    s16 intensityVelocity;
    s16 intensityTimer;
    s16 colorIndex;
    s16 colorTimer;
    u8 colorCount;
    u8 alpha;
    u8 colorMode;
    u8 red;
    u8 green;
    u8 blue;
    u8 alternateRed;
    u8 alternateGreen;
    u8 alternateBlue;
    u8 updateTexture;
} CircularParticle;

typedef struct ParticleRenderTransform {
    s16 rotationY;
    s16 rotationX;
    s16 rotationZ;
    s16 flags;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[8];
} ParticleRenderTransform;

typedef struct ParticleRenderResourceHeader {
    u8 pad00[0x68];
    void *displayList;
} ParticleRenderResourceHeader;

typedef struct ParticleRenderResource {
    ParticleRenderResourceHeader *header;
    void *vertices;
    u8 pad08[2];
    s16 triangleIndex;
    void *triangles[1];
} ParticleRenderResource;

typedef struct ParticleSpriteResource {
    u8 pad00[6];
    s16 flags;
    u8 red;
    u8 green;
    u8 blue;
    u8 alternateRed;
    u8 alternateGreen;
    u8 alternateBlue;
} ParticleSpriteResource;

typedef struct CircularParticlePool {
    u8 pad00[0x14];
    CircularParticle *particles;
    s32 count;
    s32 activeCount;
    s32 *freeBits;
    s32 lastBitWord;
    s32 exhausted;
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
extern s32 D_8007C854;
extern s32 D_8007C85C;
extern f32 D_80082A48;
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
extern s32 D_8007C8C4;
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
extern f32 D_800D4134;
extern f32 D_800D4138;
extern f32 D_800D413C;
extern void *D_8007CA60;
extern f32 *D_8007CA90[];
extern void *D_8007CA98;

void mmFree(void *ptr);
void func_800347A0(void *resource);
void func_800359D4(void *resource);
void modFreeModel(void *resource);
void func_80036544(void *texture, void *state, s32 speed, f32 *frame, s32 updateRate);
void mathOneFloatPY(void *rotation, void *vector);
void pointListRPY(s32 count, s16 *rotation, f32 *input, f32 *output);
void *piRomLoad(s32 assetId);
ParticleTexture *func_80034448(s16 resourceId);
s32 mathRnd(s32 minimum, s32 maximum);
void camSetNo(s32 camera);
void func_800221E8(void **dList, s32 arg1);
void func_800244EC(Gfx **dList, s32 renderContext, ParticleRenderTransform *transform, f32 scale, f32 extra);
void func_800245EC(Gfx **dList);
void func_80023A08(Gfx **dList, s32 renderContext, void **vertices, CircularParticle *particle,
                   ParticleSpriteResource *resource, s32 flags, s32 intensity);
void func_80034DF0(u8 red, u8 green, u8 blue, u8 alternateRed, u8 alternateGreen, u8 alternateBlue);
void func_80034E48(void);
void func_800349A4(Gfx **dList, void *texture, s32 mode, s32 flags);
void func_8003D4FC(void **dList, void **vertices, void *pool);
s32 func_8003CE10(Gfx **dList, s32 arg1, void **vertices, CircularParticlePool *pool, s32 mode);
void func_8003D25C(Gfx **dList, s32 arg1, void **vertices, CircularParticlePool *pool);
void func_80041CE4(void **dList, void **vertices);
void func_80041F48(s32 arg0, ParticleTrigger *trigger);
s32 func_80040878(CircularParticle *particle, s32 updateRate);
void func_80040B88(void);
void func_80041040(ParticleLineEntry *particle, s32 updateRate);
void func_80041388(ParticleModelEntry *particle, s32 updateRate);
void func_800367A4(void *texture, void *state, s16 speed, f32 *frame, s32 updateRate);
void func_8003EC8C(ParticleObject *object, s32 index);
void func_8003E7B8(ParticleObject *object, s32 index);
void func_8003EF80(ParticleObject *object, ParticleTriggerSlot *trigger);
s32 func_8003E8D8(ParticleTypeDescriptor *descriptor, ParticleConfig *config, ParticleTriggerSlot *trigger);
s32 func_8003EB08(ParticleTypeDescriptor *descriptor, ParticleConfig *config);
void partInitTriggerPos(ParticleTrigger *trigger, s32 type, s32 value, s16 x, s16 y, s16 z);
void func_8003CA20(void);
void func_8003CB3C(void);
void func_8003CD28(ParticleResourceList **listPtr);
void func_8003CCE4(void);
void *func_8003FB98(s32 arg0, ParticleTrigger *trigger, s32 arg2);
void func_80041530(s32 arg0, s32 arg1, ParticleModelEntry *entry);
void func_80041FEC(BasicParticle *particle);
void func_800420E0(BasicParticle *particle);
void func_800421F4(BasicParticle *particle);
void func_8004233C(BasicParticle *particle);
void func_800423EC(BasicParticle *particle);
extern u8 D_7C900[];

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
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/func_8005DA70.s; body reconstructed from Mickey evidence. */
void func_8003CB3C(void) {
    CircularParticlePool **pool;
    CircularParticlePool **poolEnd;
    ParticleLineEntry *line;
    ParticleModelEntry *model;
    s32 i;

    pool = D_800D4120, poolEnd = (CircularParticlePool **)&D_800D4134;
    do {
        func_8003CD28((ParticleResourceList **)pool);
        pool++;
    } while (pool < poolEnd);

    D_8007C8C4 = 0;
    if (D_8007C894 != NULL) {
        line = D_8007C894;
        i = 0;
        if (D_8007C88C > 0) {
            do {
                if (line->active && line->texture != NULL) {
                    func_800347A0(line->texture);
                }
                i++;
                line++;
            } while (i < D_8007C88C);
        }
        mmFree(D_8007C894);
        D_8007C894 = NULL;
    }

    if (D_8007C898 != NULL) {
        model = D_8007C898;
        i = 0;
        if (D_8007C890 > 0) {
            do {
                if (model->active && model->resource != NULL) {
                    func_800347A0(model->resource);
                }
                i++;
                model++;
            } while (i < D_8007C890);
        }
        mmFree(D_8007C898);
        D_8007C898 = NULL;
    }

    if (D_8007CA60 != NULL) {
        mmFree(D_8007CA60);
        D_8007CA60 = NULL;
    }
    if (D_8007CA98 != NULL) {
        mmFree(D_8007CA98);
        D_8007CA98 = NULL;
    }
    if (D_8007C8BC != NULL) {
        mmFree(D_8007C8BC);
        D_8007C8BC = NULL;
        D_8007C8C0 = 0;
    }
}
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
#ifdef NON_MATCHING
/*
 * Exact opcode schedule, instruction count, and relocation identities; 154
 * allocation/stack-operand words remain from +0x0 because IDO emits a 0x98
 * frame instead of the target's 0x90 frame.
 *
 * PROVENANCE: structure cross-checked against JFG's assembly-only
 * asm/nonmatchings/particles/func_8005DD88.s sibling; body reconstructed
 * from Mickey evidence.
 */
s32 func_8003CE10(Gfx **dList, s32 renderContext, void **vertices, CircularParticlePool *pool, s32 mode) {
    CircularParticle *particle;
    ParticleSpriteResource *resource;
    Gfx *command;
    void *firstVertex;
    s32 requiredFlags;
    s32 excludedFlags;
    s32 count;
    s32 i;
    s32 flags;
    s32 fade;
    s32 renderFlags;
    s32 intensity;
    s32 red;
    s32 green;
    s32 blue;
    s32 alternateRed;
    s32 alternateGreen;
    s32 alternateBlue;
    u8 colorMode;

    if (pool == NULL) {
        return 0;
    }

    particle = pool->particles;
    i = 0;
    if (mode == 1) {
        requiredFlags = 0;
        excludedFlags = 0x8000;
    } else if (mode == 0) {
        requiredFlags = 0x8000;
        excludedFlags = 0;
    } else {
        requiredFlags = 0;
        excludedFlags = 0;
    }
    firstVertex = *vertices;
    count = pool->count;
    if (count > 0) {
        do {
            if (particle->type == 2) {
                flags = particle->flags;
                if ((flags & requiredFlags) == requiredFlags && !(flags & excludedFlags)) {
                    renderFlags = 2;
                    colorMode = particle->colorMode;
                    resource = particle->resource;
                    red = particle->red;
                    green = particle->green;
                    blue = particle->blue;
                    if (colorMode != 0) {
                        alternateRed = particle->alternateRed;
                        alternateGreen = particle->alternateGreen;
                        alternateBlue = particle->alternateBlue;
                    }
                    intensity = (particle->intensity >> 8) & 0xFF;
                    if (D_8007C854 != 0) {
                        if (flags & 0x800) {
                            fade = (particle->alpha * D_8007C85C) >> 8;
                        } else {
                            fade = D_8007C85C;
                        }
                    } else {
                        if (flags & 0x800) {
                            fade = particle->alpha;
                        } else {
                            fade = 0xFF;
                        }
                    }
                    if (fade < 0xFE) {
                        red = (red * fade) >> 8;
                        green = (green * fade) >> 8;
                        blue = (blue * fade) >> 8;
                        if (colorMode != 0) {
                            alternateRed = (alternateRed * fade) >> 8;
                            alternateGreen = (alternateGreen * fade) >> 8;
                            alternateBlue = (alternateBlue * fade) >> 8;
                        }
                    }
                    if (resource->flags & 0x200) {
                        if (colorMode == 0) {
                            alternateRed = (resource->alternateRed * red) >> 8;
                            alternateGreen = (resource->alternateGreen * green) >> 8;
                            alternateBlue = (resource->alternateBlue * blue) >> 8;
                            red = (resource->red * red) >> 8;
                            green = (resource->green * green) >> 8;
                            blue = (resource->blue * blue) >> 8;
                        }
                        if (colorMode != 2) {
                            func_80034DF0(red, green, blue, alternateRed, alternateGreen, alternateBlue);
                        } else {
                            func_80034DF0(alternateRed, alternateGreen, alternateBlue, red, green, blue);
                        }
                    } else if ((red << 24 | green << 16 | blue << 8 | intensity) != 0) {
                        gDPPipeSync((*dList)++);
                        command = (*dList)++;
                        command->words.w0 = 0xFA000000;
                        command->words.w1 = (red & 0xFFFFU) << 24 | (green & 0xFF) << 16 |
                                            (blue & 0xFF) << 8 | (intensity & 0xFF);
                    }
                    if (intensity != 0xFF) {
                        renderFlags = 6;
                    }
                    func_80023A08(dList, renderContext, vertices, particle, resource, renderFlags, intensity);
                    if (resource->flags & 0x200) {
                        func_80034E48();
                    }
                }
            }
            i++;
            particle++;
            count = pool->count;
        } while (i < count);
    }
    gDPPipeSync((*dList)++);
    gDPSetPrimColor((*dList)++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    gDPSetEnvColor((*dList)++, 0xFF, 0xFF, 0xFF, 0);
    return ((u8 *)*vertices - (u8 *)firstVertex) / 10;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003CE10.s")
#endif
#ifdef NON_MATCHING
/*
 * PROVENANCE: structure cross-checked against JFG's assembly-only
 * func_8005E100 sibling; body reconstructed from Mickey evidence and the
 * resident render-command pattern in src/main/menu.c.
 */
void func_8003D25C(Gfx **dList, s32 renderContext, void **vertices, CircularParticlePool *pool) {
    ParticleRenderTransform transform;
    CircularParticle *particle;
    ParticleRenderResource *resource;
    Gfx *command;
    s32 count;
    s32 i;
    s32 color;
    u8 intensity;
    u8 red;
    u8 green;
    u8 blue;

    if (pool != NULL) {
        particle = pool->particles;
        gDPPipeSync((*dList)++);
        i = 0;
        count = pool->count;
        if (count > 0) {
            do {
                if (particle->type == 3) {
                    resource = particle->resource;
                    transform.rotationZ = particle->rotationZ;
                    transform.rotationX = particle->rotationX;
                    transform.rotationY = particle->rotationY;
                    transform.x = particle->renderX;
                    transform.y = particle->renderY;
                    transform.z = particle->renderZ;
                    transform.scale = particle->scale;
                    func_800244EC(dList, renderContext, &transform, 1.0f, 0.0f);
                    gDPPipeSync((*dList)++);
                    if (particle->flags & 0x800) {
                        intensity = particle->intensity;
                        red = particle->red;
                        green = particle->green;
                        blue = particle->blue;
                        color = (((red * intensity) >> 8) << 24) |
                                ((((green * intensity) >> 8) & 0xFF) << 16) |
                                ((((blue * intensity) >> 8) & 0xFF) << 8) | 0xFF;
                        command = (*dList)++;
                        command->words.w0 = 0xFA000000;
                        command->words.w1 = color;
                    } else {
                        gDPSetPrimColor((*dList)++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
                    }
                    command = (*dList)++;
                    command->words.w0 = (((s32)resource->triangles[resource->triangleIndex] + 0x80000000) &
                                         0xFFFFFF) |
                                        0xBF000000;
                    command->words.w1 = (s32)resource->vertices + 0x80000000;
                    gSPDisplayList((*dList)++, (s32)resource->header->displayList + 0x80000000);
                    gSP1Triangle((*dList)++, 0, 0, 0, 0);
                    count = pool->count;
                }
                i++;
                particle++;
            } while (i < count);
        }
        gDPPipeSync((*dList)++);
        gDPSetPrimColor((*dList)++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
        func_800245EC(dList);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003D25C.s")
#endif
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
#ifdef NON_MATCHING
/*
 * Exact-size plateau: the configured TU emits all 140 target opcodes but a
 * 0x30 frame instead of 0x38, leaving 22 aligned residuals (19 stack/frame,
 * two register, one branch-target) from +0x8. The 119-entry flag lattice did
 * not improve the default flags. A bounded permuter found a standalone zero
 * that did not reproduce in the configured TU and was not promoted.
 * PROVENANCE: structure cross-checked against JFG
 * asm/nonmatchings/particles/func_8005FAE8.s; body reconstructed from Mickey
 * evidence.
 */
s32 func_8003E8D8(ParticleTypeDescriptor *descriptor, ParticleConfig *config, ParticleTriggerSlot *trigger) {
    ParticlePointStreamEntry *entry;
    f32 *pointData;
    f32 *point;
    s32 result;
    s32 i;
    ParticleModelEntry *modelEntries;
    s32 pointIndex;
    s32 frameCount;

    if (D_8007C898 == NULL) {
        return 0xFF;
    }

    modelEntries = D_8007C898;
    entry = (ParticlePointStreamEntry *)modelEntries;
    result = 0xFF;
    i = 0;
    if (D_8007C890 > 0) {
        do {
            if (trigger == entry->trigger && entry->active == 1) {
                entry->active = 2;
                return i;
            }
            i++;
            entry++;
        } while (i < D_8007C890);
    }

    entry = (ParticlePointStreamEntry *)D_8007C898;
    i = 0;
    if (D_8007C890 > 0) {
        do {
            if (entry->active == 0) {
                result = i;
                i = D_8007C890;
            } else {
                entry++;
            }
            i++;
        } while (i < D_8007C890);
    }

    if (result != 0xFF) {
        entry->active = 2;
        point = &entry->points[0][0];
        pointIndex = 0;
        entry->pointCount = (u32)descriptor->pointCount >> 4;
        pointData = D_8007CA90[(u32)descriptor->pointCount >> 4];
        if (entry->pointCount > 0) {
            do {
                point[0] = pointData[0];
                point[1] = pointData[1];
                point[2] = 0.0f;
                pointIndex++;
                pointData += 2;
                point += 3;
            } while (pointIndex < entry->pointCount);
        }
        entry->animationState = descriptor->descriptorWord;
        entry->configFlags = config->flags;
        if (descriptor->resourceId == -1) {
            entry->resource = NULL;
            entry->animationSpeed = 0;
            entry->textureFrame = 0.0f;
        } else {
            entry->resource = func_80034448(descriptor->resourceId);
            entry->animationSpeed = descriptor->animationSpeed;
            frameCount = ((ParticleTexture *)entry->resource)->frameCount >> 8;
            if (config->flags & 0x800) {
                entry->textureFrame = mathRnd(0, frameCount - 1);
            } else if ((entry->animationState & 1) == 2) {
                entry->textureFrame = (f32)frameCount - D_80082A48;
            } else {
                entry->textureFrame = 0.0f;
            }
        }
        entry->trigger = trigger;
    }
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003E8D8.s")
#endif
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
#ifdef NON_MATCHING
/*
 * Instruction-count and opcode-exact plateau: six register-only words differ,
 * first at +0xE4, from one late pointer-color choice and the final commutative
 * loop comparison.
 * PROVENANCE: body adapted from JFG src/particles.c:partUpdateTriggers.
 */
void partUpdateTriggers(ParticleObject *object, s32 updateRate) {
    ParticleTriggerSlot *base;
    s32 offset;
    s32 i;
    s32 flags;
    s8 count;
    u32 triggerBits;
    ParticleTriggerSlot *trigger;

    count = object->header->triggerCount;
    i = 0;
    triggerBits = object->triggerBits;
    offset = 0;
    if (count > 0) {
        do {
            trigger = (ParticleTriggerSlot *)((u8 *)object->triggers + offset);
            if (triggerBits & 1) {
                flags = trigger->flags;
                if (!(flags & 0x8000)) {
                    func_8003E7B8(object, i);
                    trigger = (ParticleTriggerSlot *)((u8 *)object->triggers + offset);
                    flags = trigger->flags;
                }
                if (flags & 0x4000) {
                    trigger->unk0C += updateRate;
                    func_8003EF80(object, (ParticleTriggerSlot *)((u8 *)object->triggers + offset));
                } else if (flags & 0x400) {
                    trigger->unk0C += updateRate;
                    func_8003EF80(object, (ParticleTriggerSlot *)((u8 *)object->triggers + offset));
                } else {
                    trigger->unk0C += updateRate;
                    base = object->triggers;
                    trigger = (ParticleTriggerSlot *)((u8 *)base + offset);
                    if (trigger->unk0C >= trigger->config->value40) {
                        func_8003EF80(object, (ParticleTriggerSlot *)((u8 *)object->triggers + offset));
                    }
                }
            } else if (trigger->flags & 0x8000) {
                func_8003EC8C(object, i);
            }
            i++;
            offset += sizeof(ParticleTriggerSlot);
            triggerBits >>= 1;
        } while (i != count);
    }
    D_8007C8F0 = 0.0f;
    D_8007C8F4 = 255.0f;
    D_8007C8F8 = 1.0f;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partUpdateTriggers.s")
#endif
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/func_8006020C.s; body reconstructed from Mickey evidence. */
void func_8003EF80(ParticleObject *object, ParticleTriggerSlot *trigger) {
    ParticleConfig *config;
    EmittedParticle *particle;
    s32 i;

    config = trigger->config;
    if (trigger->flags & 0x4000) {
        if (trigger->unk0C >= config->value40) {
            trigger->unk0C -= config->value40;
            func_80040B88();
        }
    } else if (trigger->flags & 0x400) {
        if (trigger->unk0C >= config->value40) {
            trigger->unk0C -= config->value40;
            func_80041F48((s32)object, (ParticleTrigger *)trigger);
        }
    } else if (trigger->unk0C >= config->value40) {
        do {
            trigger->unk0C -= config->value40;
            i = 0;
            if (config->value42 > 0) {
                do {
                    particle = func_8003FB98((s32)object, (ParticleTrigger *)trigger, -1);
                    if (particle != NULL) {
                        if (trigger->unk0C != 0) {
                            particle->x -= D_800D4134;
                            particle->y -= D_800D4138;
                            particle->z -= D_800D413C;
                            func_80040878((CircularParticle *)particle, trigger->unk0C);
                            particle->x += D_800D4134;
                            particle->y += D_800D4138;
                            particle->z += D_800D413C;
                        }
                        particle->flags |= 0x20000;
                    }
                    i++;
                } while (i < config->value42);
            }
        } while (trigger->unk0C >= config->value40);
    }
}
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
#ifdef NON_MATCHING
/*
 * One-word-short plateau: the best 124-word candidate first differs at +0x4C
 * and retains 43 aligned residuals. IDO folds the initial free-bit scan
 * address to a move instead of the target shift/add pair, then assigns the
 * scan and particle-index scratch registers differently. The full 119-entry
 * flag lattice found no improvement; a bounded ten-minute permuter batch
 * improved its internal score from 1065 to 705 without reaching an exact
 * result.
 * PROVENANCE: structure cross-checked against JFG
 * asm/nonmatchings/particles/func_80061948.s; body reconstructed from Mickey
 * evidence.
 */
CircularParticle *func_8004054C(s32 type, s32 direction) {
    CircularParticlePool *pool;
    CircularParticle *particle;
    s32 *freeBits;
    s32 *wordPtr;
    s32 bits;
    s32 particleIndex;
    s32 wordIndex;
    s32 bitIndex;

    particle = NULL;
    wordIndex = 0;
    if (type >= 5) {
        return NULL;
    }
    pool = D_800D4120[type];
    if (pool != NULL) {
        if (pool->activeCount >= pool->count) {
            if (pool->exhausted == 0) {
                pool->exhausted = 1;
            }
        } else {
            if (direction == -1) {
                freeBits = pool->freeBits;
                if (*freeBits == 0) {
                    wordPtr = (s32 *)((u8 *)freeBits + (wordIndex << 2));
                    if (pool->lastBitWord >= wordIndex) {
                        do {
                            wordIndex++;
                            wordPtr++;
                        } while (*wordPtr == 0 && wordIndex <= pool->lastBitWord);
                    }
                }
                wordPtr = freeBits + wordIndex;
                if (pool->lastBitWord < wordIndex) {
                    return NULL;
                }
                bits = *wordPtr;
                bitIndex = 0;
                particleIndex = wordIndex << 5;
                if (!(bits & 1)) {
                    do {
                        bitIndex++;
                    } while (!(bits & (1 << bitIndex)));
                }
                *wordPtr = bits & ~(1 << bitIndex);
                particleIndex += bitIndex;
            } else {
                wordIndex = pool->lastBitWord;
                if (wordIndex > 0) {
                    wordPtr = pool->freeBits;
                    wordPtr = wordPtr + wordIndex;
                    if (*wordPtr == 0) {
                        do {
                            wordIndex--;
                            wordPtr--;
                        } while (wordIndex > 0 && *wordPtr == 0);
                    }
                }
                freeBits = pool->freeBits;
                wordPtr = freeBits + wordIndex;
                bits = *wordPtr;
                particleIndex = wordIndex << 5;
                if (bits == 0) {
                    return NULL;
                }
                bitIndex = 0x1F;
                if (!(bits & 0x80000000)) {
                    do {
                        bitIndex--;
                    } while (!(bits & (1 << bitIndex)));
                }
                *wordPtr = bits & ~(1 << bitIndex);
                particleIndex += bitIndex;
            }
            if (particleIndex >= pool->count) {
                return NULL;
            }
            particle = &pool->particles[particleIndex];
            particle->type = type;
            pool->activeCount++;
        }
    }
    return particle;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8004054C.s")
#endif
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
    if (pool->activeCount > 0) {
        if (particle->resource != NULL) {
            func_800347A0(particle->resource);
        }
        pool->activeCount--;
        particle->type = 0x80;
        index = (particle - pool->particles);
        pool->freeBits[index >> 5] |= 1 << (index & 0x1F);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80040740.s")
#endif
/*
 * PROVENANCE: body adapted from DKR src/particles.c:particle_update and
 * cross-checked against JFG asm/nonmatchings/particles/func_80061228_61E28.s.
 * Mickey's resource-update and colour-table paths are reconstructed from its
 * own evidence.
 */
s32 func_80040878(CircularParticle *particle, s32 updateRate) {
    s32 flags;

    flags = particle->flags;
    if (flags & 0x80000) {
        particle->flags = flags & ~0x80000;
        return 0;
    }

    D_800D4140 = updateRate;
    flags = particle->flags;
    if (flags & 0x20000) {
        particle->flags = flags ^ 0x20000;
        if (particle->kind == 3) {
            D_800D4140 = 0;
            func_800420E0((BasicParticle *)particle);
            D_800D4140 = updateRate;
        }
        goto done;
    }

    if (particle->type == 5) {
        goto done;
    }

    particle->lifetime -= D_800D4140;
    if (particle->lifetime <= 0) {
        func_80040740(particle);
        return 1;
    }

    if (particle->type == 2 && (particle->flags & 1)) {
        func_80036544(particle->resource, &particle->flags, particle->updateTexture,
                      &particle->textureFrame, D_800D4140);
    } else if (particle->type == 1 || particle->type == 0) {
        if (particle->resource != NULL) {
            func_800367A4(particle->resource, &particle->flags, particle->updateTexture,
                          &particle->textureFrame, D_800D4140);
        }
    }

    if (particle->kind == 2) {
        func_800421F4((BasicParticle *)particle);
    } else if (particle->kind == 3) {
        func_800420E0((BasicParticle *)particle);
    } else if (particle->kind == 4) {
        func_80041FEC((BasicParticle *)particle);
    } else if (particle->kind == 5) {
        func_800423EC((BasicParticle *)particle);
    } else {
        func_8004233C((BasicParticle *)particle);
    }

    if (particle->flags & 0x400) {
        if (particle->colorTimer == 0) {
            particle->colorIndex += D_800D4140;
            while (particle->colorIndex >= particle->colorCount) {
                particle->colorIndex -= particle->colorCount;
            }
            particle->red = particle->colorTable[particle->colorIndex * 4];
            particle->green = particle->colorTable[(particle->colorIndex * 4) + 1];
            particle->blue = particle->colorTable[(particle->colorIndex * 4) + 2];
        } else {
            particle->colorTimer -= D_800D4140;
            if (particle->colorTimer < 0) {
                particle->colorIndex -= particle->colorTimer;
            }
        }
    }

    if (particle->intensityTimer == 0) {
        particle->intensity += D_800D4140 * particle->intensityVelocity;
    } else {
        particle->intensityTimer -= D_800D4140;
        if (particle->intensityTimer < 0) {
            particle->intensity -= particle->intensityTimer * particle->intensityVelocity;
            particle->intensityTimer = 0;
        }
    }

    if (particle->trigger != NULL) {
        particle->trigger->unk0C += updateRate;
        func_8003EF80((ParticleObject *)particle, (ParticleTriggerSlot *)particle->trigger);
    }
done:
    return 0;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80040B88.s")
/*
 * PROVENANCE: structure cross-checked against JFG
 * asm/nonmatchings/particles/func_80061C30_62830.s and DKR
 * src/particles.c:update_line_particle; body reconstructed from Mickey
 * evidence.
 */
void func_80041040(ParticleLineEntry *entry, s32 updateRate) {
    ParticleLinePoint *point;
    s32 i;
    s32 j;
    ParticleLinePoint *removedPoints[9];

    i = 0;
    if (entry->pointCount > 0 && entry->points[0]->lifetime < updateRate) {
        do {
            i++;
        } while (i < entry->pointCount && entry->points[i]->lifetime < updateRate);
    }

    if (i > 0) {
        entry->pointCount -= i;
        for (j = 0; j < i; j++) {
            removedPoints[j] = entry->points[j];
        }
        j = 0;
        while (i < 9) {
            j++;
            i++;
            entry->points[j - 1] = entry->points[i - 1];
        }
        i = 0;
        while (j < 9) {
            j++;
            entry->points[j - 1] = removedPoints[i];
            i++;
        }
    }

    if (entry->active == 1 && entry->pointCount == 0) {
        entry->active = 0;
        if (entry->texture != NULL) {
            func_800347A0(entry->texture);
        }
    } else {
        for (i = 0; i < entry->pointCount; i++) {
            point = entry->points[i];
            point->lifetime -= updateRate;
            if (entry->colorTable != NULL) {
                if (point->colorTimer == 0) {
                    point->colorIndex += updateRate;
                    if (point->colorIndex >= entry->colorCount) {
                        point->colorIndex -= entry->colorCount;
                    }
                    point->red = (entry->colorTable[point->colorIndex] & 0xFF000000) >> 24;
                    point->green = (entry->colorTable[point->colorIndex] & 0xFF0000) >> 16;
                    point->blue = (entry->colorTable[point->colorIndex] & 0xFF00) >> 8;
                } else {
                    point->colorTimer -= updateRate;
                    if (point->colorTimer < 0) {
                        point->colorIndex -= point->colorTimer;
                    }
                }
            }
            if (point->intensityTimer == 0) {
                point->intensity += updateRate * point->intensityVelocity;
            } else {
                point->intensityTimer -= updateRate;
                if (point->intensityTimer < 0) {
                    point->intensity -= point->intensityTimer * point->intensityVelocity;
                    point->intensityTimer = 0;
                }
            }
        }

        if (entry->active != 0) {
            if (entry->texture != NULL) {
                if (entry->configFlags & 0x800) {
                    entry->textureFrame = mathRnd(0, (entry->texture->frameCount >> 8) - 1);
                } else {
                    func_800367A4(entry->texture, &entry->descriptorWord, entry->value144,
                                  &entry->textureFrame, updateRate);
                }
            }
        }
    }
}
/* PROVENANCE: structure cross-checked against JFG asm/nonmatchings/particles/func_80062A4C.s; body reconstructed from Mickey evidence. */
void func_80041388(ParticleModelEntry *entry, s32 updateRate) {
    void **particle;
    s32 changed;
    s32 found;
    s32 i;
    s32 j;
    ParticleTexture *texture;

    changed = 0;
    i = 0;
    if (entry->particleCount > 0) {
        particle = entry->particles;
        do {
            if (func_80040878((CircularParticle *)*particle, updateRate) != 0) {
                *particle = NULL;
                changed = 1;
            }
            i++;
            particle++;
        } while (i < entry->particleCount);
    }

    if (changed != 0) {
        entry->particleCount = 0;
        i = 0;
        do {
            found = 0;
            j = i;
            if (i < 15) {
                do {
                    particle = &entry->particles[j];
                    if (*particle != NULL) {
                        found = 1;
                        if (i != j) {
                            entry->particles[i] = *particle;
                            *particle = NULL;
                        }
                        j = 15;
                        entry->particleCount++;
                    }
                    j++;
                } while (j < 15);
            }
            if (found == 0) {
                entry->particles[i] = NULL;
            }
            i++;
        } while (i != 15);
    }

    if (entry->active == 1 && entry->particleCount == 0) {
        texture = entry->resource;
        entry->active = 0;
        if (texture != NULL) {
            func_800347A0(texture);
            entry->resource = NULL;
        }
    }
    if (entry->active != 0) {
        texture = entry->resource;
        if (entry->configFlags & 0x800) {
            entry->textureFrame = mathRnd(0, (texture->frameCount >> 8) - 1);
        } else {
            func_800367A4(texture, &entry->animationState, entry->animationSpeed, &entry->textureFrame, updateRate);
        }
    }
}
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
#ifdef NON_MATCHING
/*
 * Opcode-, size-, frame-, and relocation-exact plateau: 34 words differ from
 * +0x48. The target colors the outer count in a3 rather than a2 and places
 * the address-taken display-list local at sp+0x6C rather than sp+0x7C; the
 * resulting pool-register rotation continues through the two command words.
 * The flag lattice found no alternative, and the bounded permuter imported
 * the TU as inadmissible -mips1 and only improved its own score with a dummy
 * label.
 *
 * PROVENANCE: structure cross-checked against JFG's assembly-only
 * func_80063514 sibling; body reconstructed from Mickey evidence.
 */
void func_80041CE4(void **dList, void **vertices) {
    Gfx *displayList;
    Gfx *command;
    ParticleLineVertex *vertex;
    ParticleLineVertex *vertexStart;
    ParticleLinePoint *point;
    ParticleLineRenderEntry *line;
    s32 i;
    s32 j;
    s32 pointCount;
    s32 vertexAddress;
    ParticleLinePoint **pointPtr;

    if (D_8007C894 != NULL) {
        displayList = *dList;
        vertex = *vertices;
        line = (ParticleLineRenderEntry *)D_8007C894;
        i = 0;
        if (D_8007C88C > 0) {
            do {
                if (line->active != 0) {
                    pointCount = line->pointCount;
                    vertexStart = vertex;
                    j = 0;
                    if (pointCount >= 2) {
                        if (pointCount > 0) {
                            pointPtr = (ParticleLinePoint **)line;
                            do {
                                point = pointPtr[1];
                                j++;
                                pointPtr++;
                                vertex->x0 = point->x0;
                                vertex->y0 = point->y0;
                                vertex->z0 = point->z0;
                                vertex->red0 = point->red;
                                vertex->green0 = point->green;
                                vertex->blue0 = point->blue;
                                vertex->alpha0 = point->alpha;
                                vertex->x1 = point->x1;
                                vertex->y1 = point->y1;
                                vertex->z1 = point->z1;
                                vertex->red1 = point->red;
                                vertex->green1 = point->green;
                                vertex->blue1 = point->blue;
                                vertex->alpha1 = point->alpha;
                                vertex++;
                            } while (j < line->pointCount);
                        }
                        func_800349A4(&displayList, line->texture, 0x12,
                                      (s32)(line->textureFrame * 65536.0f));
                        pointCount = line->pointCount;
                        vertexAddress = (s32)vertexStart + 0x80000000;
                        pointCount *= 2;
                        command = displayList++;
                        command->words.w0 = ((((pointCount << 3) | (vertexAddress & 6)) & 0xFF) << 16) |
                                            0x04000000 |
                                            ((((pointCount << 3) + (pointCount << 1)) + 8) & 0xFFFF);
                        command->words.w1 = vertexAddress;
                        command = displayList++;
                        command->words.w0 = (((((pointCount - 3) << 4) | 1) & 0xFF) << 16) |
                                            0x05000000 | (((pointCount - 2) << 4) & 0xFFFF);
                        command->words.w1 = (s32)D_7C900;
                    }
                }
                i++;
                line++;
            } while (i < D_8007C88C);
        }
        gDPPipeSync(displayList++);
        *dList = displayList;
        *vertices = vertex;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041CE4.s")
#endif
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
        D_8007C8EC = func_8003CE10(dList, arg1, &vertices, D_800D4128, 1);
        return;
    }
    camSetNo(0);
    func_800221E8((void **)dList, arg1);
    func_8003D4FC((void **)dList, &vertices, D_800D4120[0]);
    func_8003D4FC((void **)dList, &vertices, D_800D4124);
    func_80041CE4((void **)dList, &vertices);
    func_80041C50((s32)dList, (s32)&vertices);
    func_8003CE10(dList, arg1, &vertices, D_800D4128, mode);
    func_8003D25C(dList, arg1, &vertices, D_800D412C);
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
    } while (poolPtr != (CircularParticlePool **)&D_800D4134);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partNullifyCircularParticleParents.s")
#endif
