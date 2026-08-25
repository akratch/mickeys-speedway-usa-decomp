#include "PR/ultratypes.h"

#define SHARED_DRAW_FUNCTION overlay69DrawSortedGeometry
#define SHARED_FIXED_RESOURCE_RELOC overlay69DrawFixedResourceReloc
#define SHARED_METRIC_RELOC overlay69MetricReloc
#define SHARED_TRANSFORM_RELOC overlay69PrepareTransformReloc
#define SHARED_DYNAMIC_SUBMIT_RELOC overlay69SubmitDynamicReloc
#define SHARED_FIXED_SUBMIT_RELOC overlay69SubmitFixedReloc

#define SHARED_SET_ENV_WHITE_ZERO_ALPHA(packet)             \
    {                                                        \
        SharedCommand *macroCommand = (SharedCommand *)(packet); \
        macroCommand->w0 = 0xFB000000U;                      \
        macroCommand->w1 = 0xFFFFFF00U;                      \
    }

#define SHARED_LOAD_FIXED_GEOMETRY(packet, geometry)         \
    {                                                        \
        SharedCommand *macroCommand = (SharedCommand *)(packet); \
        macroCommand->w0 = 0x01810040U;                      \
        macroCommand->w1 = (u32)(geometry);                  \
    }

#define SHARED_APPEND_TRAILING_STATE(packet)                 \
    {                                                        \
        SharedCommand *macroCommand = (SharedCommand *)(packet); \
        macroCommand->w0 = 0xBC00000AU;                     \
        macroCommand->w1 = 0;                               \
    }

typedef struct SharedCommand {
    u32 w0;
    u32 w1;
} SharedCommand;

typedef struct SharedVec3 {
    f32 x;
    f32 y;
    f32 z;
} SharedVec3;

typedef struct SharedDynamicEntry {
    void *payload;
    s8 vectorIndex;
    u8 reserved05[3];
    f32 scale;
    u8 reserved0C[8];
} SharedDynamicEntry;

typedef struct SharedResourceSet {
    u8 reserved00[0x0A];
    s16 geometryGroup;
    void *geometryBases[13];
    SharedVec3 *vectors;
} SharedResourceSet;

typedef struct SharedDrawState {
    s16 dynamicHalf0[4];
    s16 dynamicHalf8[4];
    void *fixedResources[4];
    void *fixedRefs[4];
    u8 reserved30[4];
    u8 fixedActive[4];
    s8 fixedVectorIndex[4];
    s8 fixedGeometryIndex[4];
    u8 reserved40[4];
    SharedVec3 position;
    s16 angle;
} SharedDrawState;

typedef struct SharedGateConfig {
    f32 scale;
    u8 reserved04[0x1A];
    s8 suppressBySelector[1];
} SharedGateConfig;

typedef struct SharedRenderObject {
    u8 reserved00[2];
    s16 angle2;
    s16 angle4;
    s16 flags6;
    f32 scale8;
    u8 reserved0C[0x2D];
    u8 submitFlags39;
    u8 reserved3A[6];
    SharedGateConfig *gate;
    u8 reserved44[0x0C];
    void *submitResource50;
    u8 reserved54[0x0C];
    SharedDynamicEntry *dynamicEntries;
    SharedDrawState *drawState;
    SharedResourceSet **resourcesBySelector;
    u8 reserved6C[0x20];
    u8 dynamicCount;
    u8 reserved8D[6];
    u8 selector;
} SharedRenderObject;

typedef struct SharedTransform {
    s16 stateAngle;
    s16 objectAngle2;
    s16 objectAngle4;
    s16 reserved06;
    f32 objectScale;
    SharedVec3 position;
} SharedTransform;

typedef struct SharedDynamicSubmit {
    s16 half0;
    s16 half2;
    s16 untouched4;
    s16 mode6;
    f32 scale8;
    f32 oneC;
    SharedVec3 vector;
    s32 constant1C;
    void *payload20;
} SharedDynamicSubmit;

extern void SHARED_FIXED_RESOURCE_RELOC(SharedCommand **commands,
                                        void *resource);
extern f32 SHARED_METRIC_RELOC(f32 x, f32 y, f32 z);
extern void SHARED_TRANSFORM_RELOC(SharedTransform *transform);
extern void SHARED_DYNAMIC_SUBMIT_RELOC(
    SharedCommand **commands, void *renderArg1, void *renderArg2,
    SharedTransform *transform, void *objectResource,
    SharedDynamicSubmit *submit, s32 mode, u8 flags);
extern void SHARED_FIXED_SUBMIT_RELOC(SharedCommand **commands,
                                      void *reference, s32 mode, u8 key);

/* DKR v77/v80 and JFG have no exact donor for this renderer. */
/*
 * Plateau (this run: the full flag lattice plus seven structural candidates):
 * canonical MIPS-II is exact-size with 140 differing words, first at +0x0.
 * The target frame is eight bytes smaller even though every accessed local
 * has the same offset; the remaining body differences are allocator and
 * scheduling webs.  Direct typed fixed-array indexing was four bytes short
 * with 195 differences, while partial and full typed-local aggregates were
 * 12 and 32 bytes short with 206 and 329 differences.  The eight-element
 * s16 key array is target-supported but codegen-neutral.  A bounded permuter
 * batch could not import the macro-defined shared function body.
 */
#ifdef NON_MATCHING
#define OVERLAY69_COMPILE_SHARED_BODY
#endif
#ifdef overlay69DrawSortedGeometry
#define OVERLAY69_COMPILE_SHARED_BODY
#endif

#ifdef OVERLAY69_COMPILE_SHARED_BODY
void SHARED_DRAW_FUNCTION(SharedCommand **commands, void *renderArg1,
                          void *renderArg2, SharedRenderObject *object) {
    register SharedDrawState *state;
    register SharedResourceSet *resources;
    register SharedDynamicEntry *entry;
    register SharedVec3 *vector;
    s16 order[8];
    s16 fixedKeys[8];
    f32 inverseScale;
    SharedTransform transform;
    SharedDynamicSubmit submit;
    void *fixedGeometry[8];
    f32 metrics[8];
    register void *reference;
    void *fixedRefs[8];
    register SharedCommand **commandList;
    register s16 accepted;
    register s16 index;
    register s16 inner;
    register s16 left;
    register s16 right;
    register s16 count;
    s32 outputOffset;

    commandList = commands;
    if (object->flags6 & 0x400) {
        return;
    }

    state = object->drawState;
    resources = object->resourcesBySelector[object->selector];

    SHARED_SET_ENV_WHITE_ZERO_ALPHA((*commandList)++);

    for (index = 0; index < 4; index++) {
        if (state->fixedResources[index] != NULL) {
            SHARED_FIXED_RESOURCE_RELOC(
                commandList, state->fixedResources[index]);
        }
    }
    entry = object->dynamicEntries;
    if (entry != NULL) {
        if (!object->gate->suppressBySelector[object->selector]) {
            index = 0;
            accepted = 0;
            while ((index < object->dynamicCount) && (index < 8)) {
                vector = &resources->vectors[entry->vectorIndex];
                metrics[accepted] = SHARED_METRIC_RELOC(
                    vector->x, vector->y, vector->z);
                order[accepted] = accepted;
                index++;
                accepted++;
                entry++;
            }

            for (index = accepted - 1; index > 0; index--) {
                for (inner = 0; inner < index; inner++) {
                    right = order[inner + 1];
                    left = order[inner];
                    if (metrics[right] < metrics[left]) {
                        order[inner] = right;
                        order[inner + 1] = left;
                    }
                }
            }

            transform.stateAngle = state->angle;
            transform.objectAngle2 = object->angle2;
            transform.objectAngle4 = object->angle4;
            transform.objectScale = object->scale8;
            transform.position.x = state->position.x;
            transform.position.y = state->position.y;
            transform.position.z = state->position.z;
            SHARED_TRANSFORM_RELOC(&transform);

            inverseScale = 1.0f / object->gate->scale;
            submit.mode6 = 3;
            submit.constant1C = 0x3333;
            for (index = 0; index < accepted; index++) {
                count = order[index];
                entry = &object->dynamicEntries[count];
                vector = &resources->vectors[entry->vectorIndex];
                submit.half0 = state->dynamicHalf0[count];
                submit.half2 = state->dynamicHalf8[count];
                submit.scale8 = entry->scale * inverseScale;
                submit.vector.x = vector->x;
                submit.vector.y = vector->y;
                submit.vector.z = vector->z;
                submit.oneC = 1.0f;
                submit.payload20 = entry->payload;
                SHARED_DYNAMIC_SUBMIT_RELOC(
                    commandList, renderArg1, renderArg2, &transform,
                    object->submitResource50, &submit, 0xE,
                    object->submitFlags39);
            }
        }
    }

    if (object->gate->suppressBySelector[object->selector]) {
        return;
    }

    index = 0;
    accepted = 0;
    while (index < 4) {
        if ((state->fixedActive[index] != 0) &&
            ((reference = state->fixedRefs[index]) != NULL)) {
            vector = &resources->vectors[state->fixedVectorIndex[index]];
            order[accepted] = accepted;
            metrics[accepted] = SHARED_METRIC_RELOC(
                vector->x, vector->y, vector->z);
            outputOffset = accepted << 2;
            *(void **)((u8 *)fixedRefs + outputOffset) = reference;
            *(void **)((u8 *)fixedGeometry + outputOffset) =
                (u8 *)resources->geometryBases[resources->geometryGroup] +
                (state->fixedGeometryIndex[index] * 64);
            fixedKeys[accepted] = state->fixedActive[index];
            accepted++;
        }
        index++;
    }

    if (accepted > 0) {
        for (index = accepted - 1; index > 0; index--) {
            for (inner = 0; inner < index; inner++) {
                right = order[inner + 1];
                left = order[inner];
                if (metrics[right] < metrics[left]) {
                    order[inner] = right;
                    order[inner + 1] = left;
                }
            }
        }

        for (index = 0; index < accepted; index++) {
            SHARED_LOAD_FIXED_GEOMETRY(
                (*commandList)++,
                (void *)((u32)fixedGeometry[order[index]] +
                         0x80000000U));
            SHARED_FIXED_SUBMIT_RELOC(
                commandList, fixedRefs[order[index]], 6,
                (u8)fixedKeys[order[index]]);
        }

        SHARED_APPEND_TRAILING_STATE((*commandList)++);
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o069/overlay69DrawSortedGeometry/func_overlay_069_F0000170_18C8BD8.s")
#endif

#undef OVERLAY69_COMPILE_SHARED_BODY
