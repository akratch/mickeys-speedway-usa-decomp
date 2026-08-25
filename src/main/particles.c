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
    f32 value10;
    s16 value14;
    s16 value16;
    s16 value18;
    s16 value1A;
    s16 value1C;
    s16 value1E;
    u8 pad20[2];
    s16 value22;
    s16 value24;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 value2E;
    f32 value30;
    f32 value34;
    f32 value38;
    f32 value3C;
    s16 value40;
    s16 value42;
    s16 value44;
    s16 value46;
    s16 value48;
    s16 value4A;
    s16 value4C;
    s16 value4E;
    f32 value50;
    f32 value54;
    f32 value58;
    s32 flags5C;
    s32 value60;
    s16 value64;
    s16 value66;
    u8 pad68[2];
    s16 value6A;
    s16 value6C;
    u8 pad6E[2];
    s32 value70;
    s32 value74;
    s32 value78;
    s32 value7C;
    s16 value80;
    s16 value82;
    s16 value84;
    s16 value86;
    s16 value88;
    s16 value8A;
    s32 value8C;
    s32 value90;
    s32 value94;
    u8 value98;
    u8 value99;
    u8 value9A;
    u8 pad9B;
} ParticleConfig;

typedef struct ParticleTrigger {
    ParticleConfig *config;
    s32 flags;
    union {
        s16 unk08;
        struct {
            u8 sourceRotationCounter;
            u8 directionRotationCounter;
        };
    };
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
    u8 result;
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

typedef struct ParticleVec3f {
    f32 x;
    f32 y;
    f32 z;
} ParticleVec3f;

typedef struct ParticleModelEntry {
    void *particles[15];
    s32 particleCount;
    s32 vertexCount;
    ParticleVec3f points[8];
    u8 active;
    u8 padA5[3];
    s32 animationState;
    s32 configFlags;
    f32 textureFrame;
    s16 animationSpeed;
    u8 padB6[2];
    void *resource;
    volatile ParticleTriggerSlot *trigger;
} ParticleModelEntry;

typedef struct ParticleVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} ParticleVertex;

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
    u8 pad00[4];
    s16 flags;
    u8 pad06[0x0A];
    u16 frameCount;
};

typedef struct ParticleSprite {
    u8 frameCount;
    u8 pad01[0x0F];
    ParticleTexture **textures;
} ParticleSprite;

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
    s16 lifetime;
    s16 lifetimeRange;
    u8 intensity;
    u8 targetIntensity;
    s16 intensityTimer;
    s16 colorTimer;
    f32 scale;
    u8 colorMode;
    u8 red;
    u8 green;
    u8 blue;
    u8 pad1C[3];
    u8 colorCount;
    u32 *colorTable;
    u8 pad24[2];
    u8 triggerValue;
    u8 triggerType;
} ParticleTypeDescriptor;

typedef struct BasicParticle {
    s16 rotationY;
    s16 rotationX;
    s16 rotationZ;
    s16 transformFlags;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    f32 scaleVelocity;
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    u8 pad28[6];
    u8 type;
    u8 pad2F;
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

typedef struct ParticleEmitterTransformIndex {
    u16 pad00;
    u16 matrixIndex;
} ParticleEmitterTransformIndex;

typedef struct ParticleEmitterHeader {
    u8 pad00[0x30];
    ParticleEmitterTransformIndex *transformIndices;
    u8 pad34[0x1A];
    s8 transformedPoints;
} ParticleEmitterHeader;

typedef struct ParticleEmitterPointSet {
    u8 pad00[0x1E];
    s8 transformedPoints;
} ParticleEmitterPointSet;

typedef struct ParticleEmitterResource {
    ParticleEmitterHeader *header;
    u8 pad04[4];
    s16 disableTransform;
    s16 matrixTableIndex;
    void *matrices[13];
    f32 (*points)[3];
} ParticleEmitterResource;

typedef struct ParticleEmitterObject {
    s16 rotationY;
    s16 rotationX;
    s16 rotationZ;
    u8 pad06[2];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    u8 pad28[0x12];
    s8 resourceIndex;
    u8 pad3B[5];
    u8 *header;
    u8 pad44[0x0C];
    f32 *shading;
    u8 pad54[0x14];
    ParticleEmitterResource **resources;
    u8 pad6C[0x26];
    u8 hasMovingParticle;
    u8 pointSetIndex;
} ParticleEmitterObject;

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
    s16 transformFlags;
    f32 scale;
    f32 renderX;
    f32 renderY;
    f32 renderZ;
    f32 scaleVelocity;
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    f32 textureFrame;
    s16 type;
    u8 kind;
    u8 state;
    f32 x;
    f32 y;
    f32 z;
    f32 movementValue;
    f32 gravity;
    u8 *colorTable;
    void *parent;
    void *resource;
    s32 flags;
    s32 value54;
    ParticleTrigger *trigger;
    s16 lifetime;
    s16 angularVelocityY;
    s16 angularVelocityX;
    s16 angularVelocityZ;
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
extern f32 D_80082A50;
extern f32 D_80082A54;
extern f32 D_80082A6C;
extern f32 D_80082A70;
extern f32 D_8007C8C8[];
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
ParticleTexture *func_80034448(s32 resourceId);
ParticleSprite *func_800355A0(s32 resourceId, s32 arg1);
void *func_8001F520(s32 resourceId, s32 arg1);
s32 mathRnd(s32 minimum, s32 maximum);
f32 sqrtf(f32 value);
void mtxf_transform_dir(void *matrix, f32 *input, f32 *output, ParticleEmitterHeader *header);
void camSetNo(s32 camera);
void func_800221E8(void **dList, s32 arg1);
void camPushModelMtx(Gfx **dList, s32 renderContext, ParticleRenderTransform *transform, f32 scale, f32 extra);
void camRestoreModelMtx(Gfx **dList);
void func_80023A08(Gfx **dList, s32 renderContext, void **vertices, CircularParticle *particle,
                   ParticleSpriteResource *resource, s32 flags, s32 intensity);
void func_80034DF0(u8 red, u8 green, u8 blue, u8 alternateRed, u8 alternateGreen, u8 alternateBlue);
void func_80034E48(void);
void func_800349A4(Gfx **dList, void *texture, s32 mode, s32 flags);
void func_8003D4FC(void **dList, void **vertices, void *pool);
s32 func_8003CE10(Gfx **dList, s32 arg1, void **vertices, CircularParticlePool *pool, s32 mode);
void func_8003D25C(Gfx **dList, s32 arg1, void **vertices, CircularParticlePool *pool);
void func_8003F154(BasicParticle *particle, ParticleEmitterObject *object, ParticleTriggerSlot *trigger,
                   ParticleConfig *config);
void func_8003F5F8(BasicParticle *particle, ParticleEmitterObject *object, ParticleTriggerSlot *trigger,
                   ParticleConfig *config);
void func_80041CE4(void **dList, void **vertices);
void func_80041F48(s32 arg0, ParticleTrigger *trigger);
s32 func_80040878(CircularParticle *particle, s32 updateRate);
CircularParticle *func_8004054C(s32 type, s32 direction);
void func_80040B88(ParticleEmitterObject *object, ParticleTriggerSlot *trigger);
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
CircularParticle *func_8003FB98(ParticleEmitterObject *object, ParticleTrigger *trigger, s32 resourceId);
void func_80041530(s32 arg0, s32 arg1, ParticleModelEntry *entry);
void func_80041FEC(BasicParticle *particle);
void func_80040740(CircularParticle *particle);
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
                    camPushModelMtx(dList, renderContext, &transform, 1.0f, 0.0f);
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
        camRestoreModelMtx(dList);
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
#ifdef NON_MATCHING
/* PROVENANCE: structure cross-checked against JFG
 * asm/nonmatchings/particles/func_8005FED8.s; body reconstructed from Mickey
 * evidence. */
/* Size-exact 47-word plateau: 24 words differ, first at +0x30. The target
 * splits the line-table address materialization across the first branch;
 * typed base locals either move its load or disturb both temp-register rings. */
void func_8003EC8C(ParticleObject *object, s32 index) {
    ParticleTriggerSlot *trigger;
    s32 flags;

    trigger = &object->triggers[index];
    trigger->flags &= ~0x8000;
    object->activeTriggerCount--;
    flags = object->triggers[index].flags;
    if (flags & 0x4000) {
        u8 childIndex;

        childIndex = trigger->result;
        trigger->result = -1;
        D_8007C894[childIndex].active = 1;
        return;
    }
    if (flags & 0x400) {
        u8 childIndex;

        childIndex = trigger->result;
        trigger->result = -1;
        D_8007C898[childIndex].active = 1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003EC8C.s")
#endif
/* PROVENANCE: body adapted from JFG src/particles.c:partObjFreeTriggers. */
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
            offset += sizeof(ParticleTriggerSlot);
            if (offset) {}
        } while (++i != count);
    }
}
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
 * Workbench: register-permutation; 100/101 words exact, first at +0x13C.
 * Lever 18 plus the canonical-flag permuter fixed the late pointer web.
 * Remains: one commutative loop-branch web; condition spellings were inert.
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
    ParticleTriggerSlot *callBase;

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
                    func_8003EF80(object, (ParticleTriggerSlot *)(offset + (u8 *)object->triggers));
                } else if (flags & 0x400) {
                    trigger->unk0C += updateRate;
                    func_8003EF80(object, (ParticleTriggerSlot *)((u8 *)object->triggers + offset));
                } else {
                    trigger->unk0C += updateRate;
                    base = object->triggers;
                    callBase = object->triggers;
                    trigger = (ParticleTriggerSlot *)((u8 *)base + offset);
                    if (trigger->unk0C >= trigger->config->value40) {
                        func_8003EF80(object, (ParticleTriggerSlot *)((u8 *)callBase + offset));
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
            func_80040B88((ParticleEmitterObject *)object, trigger);
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
                    particle = (EmittedParticle *)func_8003FB98(
                        (ParticleEmitterObject *)object, (ParticleTrigger *)trigger, -1);
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
#ifdef NON_MATCHING
/*
 * Frame-exact plateau: 294 instructions against the 297-instruction target,
 * with 252 aligned rows matching. The first raw mismatch is the end-branch
 * displacement at +0x204; the first substantive mismatch at +0x20C assigns
 * the zero-vector web to f6 instead of f0. The remaining cluster is the
 * resource-header copy/branch shape and FP normalization schedule. The full
 * flag lattice, ten structural hypotheses, and a canonical-mips2 permuter
 * found no exact spelling; asm remains canonical.
 *
 * PROVENANCE: structure cross-checked against JFG's assembly-only
 * asm/nonmatchings/particles/func_80060400.s sibling; body reconstructed
 * from Mickey evidence.
 */
void func_8003F154(BasicParticle *particle, ParticleEmitterObject *object, ParticleTriggerSlot *trigger,
                   ParticleConfig *config) {
    s32 flags;
    f32 offset[3];
    ParticleEmitterResource *resource;
    s16 rotation[2];
    f32 magnitude;
    f32 scale;
    s32 randomRange;
    f32 speed;
    ParticleEmitterHeader *header;
    s8 pointIndex;

    if (config->flags & 0x70) {
        particle->velocityX = config->value30;
        particle->velocityY = config->value34;
        particle->velocityZ = config->value38;
    } else {
        particle->velocityX = 0.0f;
        particle->velocityY = 0.0f;
        particle->velocityZ = 0.0f;
    }

    flags = config->flags5C & 0x700;
    if (flags != 0) {
        if (flags & 0x100) {
            randomRange = config->value74;
            particle->velocityX += mathRnd(-randomRange, randomRange) * 0.000015258789f;
        }
        if (flags & 0x200) {
            randomRange = config->value78;
            particle->velocityY += mathRnd(-randomRange, randomRange) * 0.000015258789f;
        }
        if (flags & 0x400) {
            randomRange = config->value7C;
            particle->velocityZ += mathRnd(-randomRange, randomRange) * 0.000015258789f;
        }
    }

    switch (config->flags & 0x70) {
        case 0x10:
            particle->velocityX += object->velocityX;
            particle->velocityY += object->velocityY;
            particle->velocityZ += object->velocityZ;
            break;
        case 0x40:
            particle->velocityX *= object->velocityX;
            particle->velocityY *= object->velocityY;
            particle->velocityZ *= object->velocityZ;
            break;
    }

    D_800D4134 = particle->velocityX;
    D_800D4138 = particle->velocityY;
    D_800D413C = particle->velocityZ;

    if (config->flags & 4) {
        offset[0] = (flags = config->flags5C, 0.0f);
        offset[1] = offset[0];
        speed = config->value3C;
        offset[2] = -speed;
        if (flags & 0x10) {
            randomRange = config->value70;
            offset[2] += mathRnd(-randomRange, randomRange) * 0.000015258789f;
        }
        speed = -offset[2];
        if (flags & 0x60) {
            rotation[0] = trigger->value14;
            if (flags & 0x20) {
                rotation[0] += mathRnd(-config->value6A, config->value6A);
            }
            rotation[1] = trigger->value16;
            if (flags & 0x40) {
                rotation[1] += mathRnd(-config->value6C, config->value6C);
            }
        } else {
            rotation[0] = trigger->value14;
            rotation[1] = trigger->value16;
        }
        mathOneFloatPY(rotation, offset);

        if (((ParticleEmitterPointSet *)(object->header + object->pointSetIndex))->transformedPoints != 0) {
            resource = object->resources[object->resourceIndex];
        } else {
            resource = NULL;
        }
        pointIndex = trigger->index;
        if (pointIndex != -1 && resource != NULL &&
            (header = resource->header, header->transformedPoints != 0)) {
            if (resource->disableTransform != 0) {
                offset[0] = 0.0f;
                offset[1] = 0.0f;
                offset[2] = 0.0f;
            } else {
                mtxf_transform_dir(
                    (u8 *)resource->matrices[resource->matrixTableIndex] +
                        (header->transformIndices[pointIndex].matrixIndex << 6),
                    offset, offset, header);
                magnitude = sqrtf((offset[2] * offset[2]) +
                                  ((offset[0] * offset[0]) + (offset[1] * offset[1])));
                if (magnitude == 0.0f) {
                    scale = speed;
                } else {
                    scale = speed / magnitude;
                }
                offset[0] *= scale;
                offset[1] *= scale;
                offset[2] *= scale;
            }
        } else if (particle->parent != NULL) {
            pointListRPY(1, particle->parent, offset, offset);
        }

        particle->velocityX += offset[0];
        particle->velocityY += offset[1];
        particle->velocityZ += offset[2];
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003F154.s")
#endif
#ifdef NON_MATCHING
/*
 * One instruction short: 262 of 276 aligned target rows match. The target has
 * an otherwise redundant branch at +0xA4; IDO also places the flags spill at
 * sp+0x44 and the rotation pair at sp+0x30, while this spelling swaps them.
 * The full flag lattice and a bounded canonical-mips2 permuter found no exact
 * alternative; the 0x48 frame and all integer/FP register lanes match.
 *
 * PROVENANCE: structure cross-checked against JFG's assembly-only
 * asm/nonmatchings/particles/func_800608EC.s sibling; body reconstructed
 * from Mickey evidence.
 */
void func_8003F5F8(BasicParticle *particle, ParticleEmitterObject *object, ParticleTriggerSlot *trigger,
                   ParticleConfig *config) {
    s16 rotation[2];
    f32 offset[3];
    ParticleEmitterResource *resource;
    s32 flags;
    s32 randomRange;
    s8 pointIndex;

    pointIndex = trigger->index;
    if (pointIndex == -1) {
        particle->localX = trigger->value1A;
        particle->localY = trigger->value1C;
        particle->localZ = trigger->value1E;
    } else {
        resource = NULL;
        if (((ParticleEmitterPointSet *)(object->header + object->pointSetIndex))->transformedPoints == 0) {
            resource = object->resources[object->resourceIndex];
        }
        if (resource != NULL && resource->points != NULL) {
            particle->localX = resource->points[pointIndex][0];
            particle->localY = resource->points[trigger->index][1];
            particle->localZ = resource->points[trigger->index][2];
            if (resource->header->transformedPoints == 0) {
                particle->localX *= object->scale;
                particle->localY *= object->scale;
                particle->localZ *= object->scale;
                pointListRPY(1, (s16 *)object, &particle->localX, &particle->localX);
            } else {
                particle->localX -= object->x;
                particle->localY -= object->y;
                particle->localZ -= object->z;
            }
        } else {
            particle->localX = 0.0f;
            particle->localY = 0.0f;
            particle->localZ = 0.0f;
        }
    }

    particle->movementValue = config->value58;
    if (config->flags5C & 0x80000) {
        randomRange = config->value94;
        particle->movementValue += mathRnd(-randomRange, randomRange) * 0.000015258789f;
    }
    if (config->flags & 1) {
        offset[0] = 0.0f;
        offset[1] = 0.0f;
        offset[2] = -config->value10;
        flags = config->flags5C;
        if (flags & 1) {
            randomRange = config->value60;
            offset[2] += mathRnd(-randomRange, randomRange) * 0.000015258789f;
        }
        if (flags & 6) {
            rotation[0] = trigger->value0E;
            if (flags & 2) {
                rotation[0] += mathRnd(-config->value64, config->value64);
            }
            rotation[1] = trigger->value10;
            if (flags & 4) {
                rotation[1] += mathRnd(-config->value66, config->value66);
            }
            mathOneFloatPY(rotation, offset);
        } else {
            pointListRPY(1, &trigger->value0E, offset, offset);
        }
        particle->localX += offset[0];
        particle->localY += offset[1];
        particle->localZ += offset[2];
    }
    if (particle->type != 4 && trigger->index == -1) {
        pointListRPY(1, (s16 *)object, &particle->localX, &particle->localX);
    }
    particle->x = particle->localX;
    particle->y = particle->localY;
    particle->z = particle->localZ;
    if (particle->type == 4) {
        pointListRPY(1, (s16 *)object, &particle->x, &particle->x);
    }
    if (D_8007C8F8 != 1.0f) {
        particle->x *= D_8007C8F8;
        particle->y *= D_8007C8F8;
        particle->z *= D_8007C8F8;
    }
    particle->x += object->x;
    particle->y += object->y;
    particle->z += object->z;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003F5F8.s")
#endif
/*
 * PROVENANCE: structure cross-checked against JFG
 * asm/nonmatchings/particles/partModelObjEmitModelPart.s; body reconstructed
 * from Mickey evidence.
 */
void partModelObjEmitModelPart(ParticleModelObject *object, f32 velocityX, f32 velocityY, f32 velocityZ, s32 index) {
    s32 offset;
    ParticleTriggerSlot *trigger;
    ParticleConfig *config;
    BasicParticle *particle;

    if (index < object->triggerCount) {
        offset = index * sizeof(ParticleModelPartConfig);
        trigger = &object->triggers[index];
        config = D_8007C8B8[((ParticleModelPartConfig *)((u8 *)object->header->parts + offset))->type];
        trigger->config = config;
        trigger->type =
            ((ParticleModelPartConfig *)((u8 *)object->header->parts + offset))->triggerType;
        trigger->flags = 0;
        trigger->unk0C = 0;
        trigger->value1A = 0;
        trigger->value1C = 0;
        trigger->value1E = 0;
        trigger->value20 = 0;
        trigger->value0E = config->value14;
        trigger->value10 = config->value16;
        trigger->value12 = config->value18;
        trigger->value14 = config->value22;
        trigger->value16 = config->value24;
        trigger->value18 = config->value26;
        trigger->index = -1;
        particle = (BasicParticle *)func_8003FB98(
            (ParticleEmitterObject *)object, (ParticleTrigger *)trigger,
            ((ParticleModelPartConfig *)((u8 *)object->header->parts + offset))->particleType);
        if (particle != NULL && (velocityX != 0.0f || velocityY != 0.0f || velocityZ != 0.0f)) {
            particle->velocityX = velocityX;
            particle->velocityY = velocityY;
            particle->velocityZ = velocityZ;
        }
    }
}
/*
 * PROVENANCE: body adapted from DKR src/particles.c:create_general_particle
 * and cross-checked against JFG
 * asm/nonmatchings/particles/func_80060ED4.s. Mickey-specific resource,
 * colour, trigger, and floating-point paths are reconstructed from Mickey
 * evidence.
 */
CircularParticle *func_8003FB98(ParticleEmitterObject *object, ParticleTrigger *trigger, s32 resourceId) {
    s32 randomizationFlags;
    ParticleTypeDescriptor *descriptor;
    CircularParticle *particle;
    void *resource;
    f32 scale;
    ParticleConfig *config;
    s32 kind;

    descriptor = D_8007C8AC[trigger->value];
    kind = (u32)descriptor->flags >> 28;
    if (kind == 5) {
        return NULL;
    }
    if (resourceId == -1) {
        resourceId = descriptor->resourceId;
    }
    config = trigger->config;
    particle = func_8004054C(kind, resourceId);
    if (particle == NULL) {
        return particle;
    }

    particle->kind = descriptor->pad04 & 0xF;
    particle->flags = descriptor->descriptorWord;
    particle->state = 0;
    particle->parent = object;
    particle->colorTable = (u8 *)descriptor->colorTable;
    particle->colorCount = descriptor->colorCount;
    if ((particle->flags & 0x800) && object->shading != NULL) {
        particle->alpha = (s32)(*object->shading * D_8007C8F4);
    } else {
        particle->alpha = 0xFF;
    }

    scale = config->value50;
    if (config->flags5C & 0x20000) {
        scale += mathRnd(-config->value8C, config->value8C) * 0.000015258789f;
    }
    particle->scale = descriptor->scale * scale;

    scale = config->value54;
    if (config->flags5C & 0x40000) {
        scale += mathRnd(-config->value90, config->value90) * 0.000015258789f;
    }
    if (config->flags & 0x1000) {
        particle->scaleVelocity =
            sqrtf((object->velocityX * object->velocityX) +
                  (object->velocityY * object->velocityY) +
                  (object->velocityZ * object->velocityZ)) *
            scale * D_80082A50;
    } else {
        particle->scaleVelocity = descriptor->scale * scale;
    }
    if (D_8007C8F0 != 0.0f) {
        particle->scale *= D_8007C8F0;
        particle->scaleVelocity *= D_8007C8F0;
    }

    particle->lifetime =
        mathRnd(-descriptor->lifetimeRange, descriptor->lifetimeRange) + descriptor->lifetime;
    particle->colorMode = descriptor->colorMode;
    if (particle->flags & 0x400) {
        particle->red = (*(u32 *)particle->colorTable & 0xFF000000) >> 24;
        particle->green = (*(u32 *)particle->colorTable & 0xFF0000) >> 16;
        particle->blue = (*(u32 *)particle->colorTable & 0xFF00) >> 8;
    } else {
        particle->red = descriptor->red;
        particle->green = descriptor->green;
        particle->blue = descriptor->blue;
    }
    particle->alternateRed = descriptor->pad1C[0];
    particle->alternateGreen = descriptor->pad1C[1];
    particle->alternateBlue = descriptor->pad1C[2];

    randomizationFlags = config->flags5C & 0xF00000;
    if (randomizationFlags != 0) {
        if (randomizationFlags & 0x100000) {
            particle->red += mathRnd(-config->value98, config->value98);
        }
        if (randomizationFlags & 0x200000) {
            particle->green += mathRnd(-config->value99, config->value99);
        }
        if (randomizationFlags & 0x400000) {
            particle->blue += mathRnd(-config->value9A, config->value9A);
        }
    }

    particle->intensityTimer = descriptor->intensityTimer;
    particle->colorTimer = descriptor->colorTimer;
    particle->colorIndex = 0;
    particle->intensity = descriptor->intensity << 8;
    if (particle->intensityTimer < particle->lifetime) {
        particle->intensityVelocity =
            ((descriptor->targetIntensity - descriptor->intensity) << 8) /
            (particle->lifetime - particle->intensityTimer);
    } else {
        particle->intensityVelocity = 0;
    }
    particle->updateTexture = descriptor->animationSpeed;

    if (particle->type == 2) {
        particle->resource = func_800355A0(resourceId, 0);
        if (particle->resource != NULL) {
            if ((*((ParticleSprite *)particle->resource)->textures)->flags & 4) {
                if (particle->flags & 0x1000) {
                    particle->transformFlags |= 8;
                } else {
                    particle->transformFlags |= 4;
                }
            }
            if (config->flags & 0x800) {
                particle->textureFrame =
                    mathRnd(0, ((ParticleSprite *)particle->resource)->frameCount - 1);
            } else if ((particle->flags & 1) == 2) {
                particle->textureFrame =
                    (f32)((ParticleSprite *)particle->resource)->frameCount - D_80082A54;
            } else {
                particle->textureFrame = 0.0f;
            }
        }
    } else if (particle->type == 1 || particle->type == 0) {
        if (resourceId < 0) {
            particle->resource = NULL;
        } else {
            resource = func_80034448(resourceId);
            if (((ParticleTexture *)resource)->flags & 4) {
                if (particle->flags & 0x1000) {
                    particle->transformFlags |= 8;
                } else {
                    particle->transformFlags |= 4;
                }
            }
            particle->textureFrame = 0.0f;
            particle->resource = resource;
        }
    } else if (particle->type == 3) {
        resource = func_8001F520(resourceId, 0);
        particle->resource = resource;
    }

    func_8003F5F8((BasicParticle *)particle, object, (ParticleTriggerSlot *)trigger, config);
    if (config->flags & 0x80) {
        particle->rotationY = config->value44;
        particle->rotationX = config->value46;
        particle->rotationZ = config->value48;
    } else {
        particle->rotationY = object->rotationY + config->value44;
        particle->rotationX = object->rotationX + config->value46;
        particle->rotationZ = object->rotationZ + config->value48;
    }

    randomizationFlags = config->flags5C & 0x3800;
    if (randomizationFlags != 0) {
        if (randomizationFlags & 0x800) {
            particle->rotationY += mathRnd(-config->value80, config->value80);
        }
        if (randomizationFlags & 0x1000) {
            particle->rotationX += mathRnd(-config->value82, config->value82);
        }
        if (randomizationFlags & 0x2000) {
            particle->rotationZ += mathRnd(-config->value84, config->value84);
        }
    }

    particle->angularVelocityY = config->value4A;
    particle->angularVelocityX = config->value4C;
    particle->angularVelocityZ = config->value4E;
    randomizationFlags = config->flags5C & 0x1C000;
    if (randomizationFlags != 0) {
        if (randomizationFlags & 0x4000) {
            particle->angularVelocityY += mathRnd(-config->value86, config->value86);
        }
        if (randomizationFlags & 0x8000) {
            particle->angularVelocityX += mathRnd(-config->value88, config->value88);
        }
        if (randomizationFlags & 0x10000) {
            particle->angularVelocityZ += mathRnd(-config->value8A, config->value8A);
        }
    }

    func_8003F154((BasicParticle *)particle, object, (ParticleTriggerSlot *)trigger, config);
    particle->gravity = D_8007C8C8[(particle->flags >> 4) & 7];
    if (particle->flags & 0x80) {
        particle->gravity = -particle->gravity;
    }
    if (particle->kind == 5) {
        particle->movementValue =
            sqrtf((particle->velocityX * particle->velocityX) +
                  (particle->velocityY * particle->velocityY) +
                  (particle->velocityZ * particle->velocityZ));
    }
    if (descriptor->descriptorWord & 0x10000) {
        particle->trigger = func_8003E730(descriptor->triggerType, descriptor->triggerValue);
    } else {
        particle->trigger = NULL;
    }
    if (config->flags & 2) {
        trigger->sourceRotationCounter++;
        if (trigger->sourceRotationCounter >= config->value1A) {
            trigger->value0E += config->value1C;
            trigger->value10 += config->value1E;
            trigger->value12 += config->value18;
            trigger->sourceRotationCounter -= config->value1A;
        }
    }
    if (config->flags & 8) {
        trigger->directionRotationCounter++;
        if (trigger->directionRotationCounter >= config->value28) {
            trigger->value14 += config->value2A;
            trigger->value16 += config->value2C;
            trigger->value18 += config->value2E;
            trigger->directionRotationCounter -= config->value28;
        }
    }
    if (particle->resource == NULL && particle->type != 4) {
        func_80040740(particle);
        return NULL;
    }
    particle->value54 = 8;
    if (config->flags & 0x10000) {
        particle->flags |= 0x80000;
    }
    if (particle->kind == 3) {
        object->hasMovingParticle = 1;
    }
    return particle;
}
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
#ifdef NON_MATCHING
/*
 * Frame-shape plateau: the best canonical-mips2 candidate emits 300 of the
 * target's 302 instructions with an 0x70 frame instead of 0x68. Alignment
 * needs seven insertions and nine deletions (no replacements); the first
 * substantive divergence preserves the trigger in t7 where the target
 * spills and reloads it, rotating the remaining temporary lanes. The full
 * flag lattice and a bounded permuter found no exact spelling.
 *
 * PROVENANCE: orientation and velocity-normalization structure adapted from
 * DKR src/particles.c:update_line_particle and cross-checked against JFG's
 * assembly-only particles sibling; point allocation and initialization are
 * reconstructed from Mickey evidence.
 */
void func_80040B88(ParticleEmitterObject *object, ParticleTriggerSlot *trigger) {
    ParticleTypeDescriptor *descriptor;
    ParticleLineEntry *entry;
    ParticleLinePoint *point;
    u32 *colorTable;
    f32 scale;
    ParticleVec3f position;
    ParticleVec3f offset;
    register f32 temp;
    s32 orientation;
    s32 pointCount;
    u8 entryIndex;

    descriptor = D_8007C8AC[trigger->type];
    if ((u32)descriptor->flags >> 28 == 5) {
        entryIndex = trigger->result;
        if (entryIndex != 0xFF) {
            entry = &D_8007C894[entryIndex];
            pointCount = entry->pointCount;
            if (pointCount != 9) {
                point = entry->points[pointCount];
                scale = descriptor->scale * trigger->config->value50;
                entry->pointCount = pointCount + 1;
                point->lifetime = descriptor->lifetime;
                if (descriptor->descriptorWord & 0x400) {
                    colorTable = entry->colorTable;
                    if (colorTable == NULL) {
                        entry->colorCount = descriptor->colorCount;
                        colorTable = descriptor->colorTable;
                        entry->colorTable = colorTable;
                    }
                    point->red = (*colorTable & 0xFF000000) >> 24;
                    point->green = (*entry->colorTable & 0xFF0000) >> 16;
                    point->blue = (*entry->colorTable & 0xFF00) >> 8;
                } else {
                    point->red = descriptor->red;
                    point->green = descriptor->green;
                    point->blue = descriptor->blue;
                }
                point->intensityTimer = descriptor->intensityTimer;
                point->colorIndex = 0;
                point->colorTimer = descriptor->colorTimer;
                point->intensity = descriptor->intensity << 8;
                if (point->intensityTimer < point->lifetime) {
                    point->intensityVelocity =
                        ((descriptor->targetIntensity - descriptor->intensity) << 8) /
                        (point->lifetime - point->intensityTimer);
                } else {
                    point->intensityVelocity = 0;
                }

                position.x = trigger->value1A;
                position.y = trigger->value1C;
                position.z = trigger->value1E;
                pointListRPY(1, (s16 *)object, &position.x, &position.x);
                position.x += object->x;
                position.y += object->y;
                position.z += object->z;
                if (trigger->config->flags & 0x1000) {
                    scale *= sqrtf((object->velocityX * object->velocityX) +
                                   (object->velocityY * object->velocityY) +
                                   (object->velocityZ * object->velocityZ)) *
                             D_80082A6C;
                }

                orientation = *(u16 *)&descriptor->flags & 0xF;
                if (!(descriptor->descriptorWord & 0x4000)) {
                    offset.x = 0.0f;
                    offset.y = 0.0f;
                    offset.z = 0.0f;
                    switch (orientation) {
                        default:
                            offset.x = scale;
                            break;
                        case 2:
                            offset.z = scale;
                            break;
                        case 1:
                            offset.y = scale;
                            break;
                    }
                    pointListRPY(1, (s16 *)object, &offset.x, &offset.x);
                } else {
                    offset.x = object->velocityX;
                    offset.y = object->velocityY;
                    offset.z = object->velocityZ;
                    temp = (offset.z * offset.z) +
                           ((offset.x * offset.x) + (offset.y * offset.y));
                    if (temp < D_80082A70) {
                        temp = 1.0f;
                    } else {
                        temp = scale / sqrtf(temp);
                    }
                    offset.x *= temp;
                    offset.y *= temp;
                    offset.z *= temp;
                    switch (orientation) {
                        case 0:
                            temp = offset.x;
                            offset.x = -offset.z;
                            offset.z = temp;
                            break;
                        case 1:
                            temp = offset.y;
                            offset.y = -offset.z;
                            offset.z = temp;
                            break;
                    }
                }

                point->x0 = position.x + offset.x;
                point->y0 = position.y + offset.y;
                point->z0 = position.z + offset.z;
                point->x1 = position.x - offset.x;
                point->y1 = position.y - offset.y;
                point->z1 = position.z - offset.z;
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80040B88.s")
#endif
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
#ifdef NON_MATCHING
/* Workbench: structure-mismatch; 457/456 instructions, 279 words, first +0x0.
 * Levers: local declaration order improved one word; swapping the final OR worsened.
 * Remains: the 0x160/0x168 frame and input/vector scalar stack allocation.
 *
 * PROVENANCE: structure cross-checked against JFG
 * asm/nonmatchings/particles/func_80062BFC.s; body reconstructed from Mickey
 * evidence.
 */
void func_80041530(s32 arg0, s32 arg1, ParticleModelEntry *entry) {
    Gfx *displayList;
    ParticleVertex *vertices;
    ParticleVertex *vertexStart;
    ParticleVec3f *outputPtr;
    CircularParticle *particle;
    ParticleVec3f output[8];
    ParticleVec3f input[8];
    void **particlePtr;
    Gfx *command;
    s32 vertexCount;
    s32 triangleCount;
    void *triangleLists[2];
    s32 triangleListIndex;
    s32 vertexIndex;
    volatile s32 vertexCommandCount;
    volatile s32 vertexCommandLength;
    s32 particleIndex;
    s32 i;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;

    if (entry->particleCount >= 2) {
        displayList = *(Gfx **)arg0;
        vertexCount = entry->vertexCount;
        vertices = *(ParticleVertex **)arg1;
        triangleLists[0] = ((void **)((u8 *)&D_8007CA60 + (vertexCount * 8)))[-4];
        triangleLists[1] = ((void **)((u8 *)&D_8007CA60 + (vertexCount * 8)))[-3];
        if (entry->vertexCount == 2) {
            triangleCount = 2;
        } else {
            triangleCount = entry->vertexCount * 2;
        }
        vertexIndex = 0;
        triangleListIndex = 0;
        func_800349A4(&displayList, entry->resource, 0x12,
                      (s32)(entry->textureFrame * 65536.0f));

        particleIndex = 0;
        particlePtr = entry->particles;
        if (entry->particleCount > 0) {
            vertexCommandLength = (vertexCount * 10) + 8;
            vertexCommandCount = vertexCount * 8;
            do {
                particle = *particlePtr;
                vertexStart = vertices;
                outputPtr = output;
                i = 0;
                while (i < vertexCount) {
                    input[i].x = entry->points[i].x * particle->scale;
                    input[i].y = entry->points[i].y * particle->scale;
                    input[i].z = entry->points[i].z * particle->scale;
                    i++;
                }

                red = particle->red;
                green = particle->green;
                blue = particle->blue;
                alpha = (particle->intensity >> 8) & 0xFF;
                pointListRPY(vertexCount, (s16 *)particle, &input[0].x, &output[0].x);
                i = 0;
                if (vertexCount > 0) {
                    do {
                        vertices->x = outputPtr->x + particle->renderX;
                        vertices->y = outputPtr->y + particle->renderY;
                        vertices->z = outputPtr->z + particle->renderZ;
                        vertices->red = red;
                        vertices->green = green;
                        vertices->blue = blue;
                        vertices->alpha = alpha;
                        vertices++;
                        outputPtr++;
                        i++;
                    } while (i < vertexCount);
                }

                command = displayList++;
                command->words.w0 =
                    (((vertexCommandCount | (((s32)vertexStart + 0x80000000) & 6)) & 0xFF) << 16) |
                    0x04000000 | ((vertexCommandLength | (vertexIndex << 9)) & 0xFFFF);
                command->words.w1 = (s32)vertexStart + 0x80000000;
                if (particleIndex > 0) {
                    command = displayList++;
                    command->words.w0 = ((((((triangleCount - 1) * 16) | 1) & 0xFF) << 16) |
                                         0x05000000 | ((triangleCount * 16) & 0xFFFF));
                    command->words.w1 = (s32)triangleLists[triangleListIndex] + 0x80000000;
                    triangleListIndex ^= 1;
                }
                if (vertexIndex == 0) {
                    vertexIndex = vertexCount;
                } else {
                    vertexIndex = 0;
                }
                particleIndex++;
                particlePtr++;
            } while (particleIndex < entry->particleCount);
        }
        *(Gfx **)arg0 = displayList;
        *(ParticleVertex **)arg1 = vertices;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041530.s")
#endif
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
            particle = func_8003FB98((ParticleEmitterObject *)arg0, trigger, -1);
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
/* Opcode- and size-exact plateau: 25/42 positional words differ, first at
 * +0x0, all in the two nested-loop carrier pairs. An explicit end pointer
 * changes the frame; the implicit end/start schedule seeds opposite colors. */
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
                    particle->z += position->z;
                    particle->parent = NULL;
                }
                particle++;
            } while (i < pool->count);
        }
    } while (poolPtr != (CircularParticlePool **)&D_800D4134);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partNullifyCircularParticleParents.s")
#endif
