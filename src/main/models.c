/*
 * Resident model loading and instance management -- ROM 0x20020-0x21DA0
 * (VRAM 0x8001F420-0x800211A0).
 *
 * The working TU is identified from three exact masked-skeleton matches to
 * Jet Force Gemini's built src/models.c.o, the first at the existing yaml
 * boundary, plus the allocator, texture, and matrix call graph of the rest of
 * the block. It is not a whole-object match; docs/modules.md section 3.4
 * records the evidence and keeps uncertain JFG correspondences as comments
 * rather than adopting names.
 *
 * PROVENANCE -- JFG's public decomp was consulted for the models.c function
 * order, names, prototypes, and structure vocabulary. No body is adapted in
 * this all-GLOBAL_ASM split. Any body later adapted from JFG must retain a
 * point-of-use PROVENANCE note, and Mickey's own bytes remain authoritative.
 *
 * Flags: -O2 -mips2 -32, via the measured src/main/ Makefile rule.
 */

#include "PR/ultratypes.h"
#include "game/math.h"
#include "game/models.h"
#include "n_audio/mbi.h"

extern s32 D_80079C00;
#ifdef NON_MATCHING
extern SuspendedModelTexture *D_80079C04;
#else
extern void *D_80079C04;
#endif
extern s16 D_80079C08;
extern s8 D_8007BD98;
extern s32 *D_800CB480;
extern s32 *D_800CB484;
extern s32 *D_800CB488;
extern s32 D_800CB48C;
extern s32 D_800CB490;
extern s32 D_800CB494;
extern s8 D_800CB498[];
extern s16 D_800CB49C[];
extern s16 D_800CB4A2[];
extern Gfx *D_800CB4A4;

void *func_8002B280(s32 size, s32 tag);
void *func_8002B314(s32 size, s32 tag);
s32 *piRomLoad(s32 assetId);
void *func_80034448(s16 textureId);
void func_800347A0(void *texture);
s32 func_8003484C(void *texture);
void func_800348A0(s32 id, s32 value);
void func_80034424(s32 enabled);
void func_80034920(Gfx **displayList);
void func_800349A4(Gfx **displayList, void *texture, s32 flags, s16 parameter);
void func_80020AD4(void);
void func_8005AAC0(void *animation);
#ifdef NON_MATCHING
struct ModelGfxSource;
s32 func_8002057C(Gfx **out, struct ModelGfxSource *model, s32 arg2, s32 arg3,
                  s32 arg4, s32 arg5, s32 arg6);
#else
u8 func_8002057C(void **out, ObjectModel *model, s32 arg2, s32 arg3, s32 arg4,
                 s32 arg5, s32 arg6);
#endif
void mmFree(void *ptr);

/*
 * PROVENANCE -- body adapted from JFG's public src/models.c
 * func_8003B870_3C470. The JFG built object carries this exact 15-word
 * skeleton at func_8003B640; Mickey's linked bytes are the authority here.
 */
void func_8001F420(u16 *src, u16 *dest, s32 len) {
    len = (len + 1) >> 1;
    while (len--) {
        *dest++ = *src++;
    }
}
/*
 * PROVENANCE -- body adapted from the initial portion of JFG's public
 * modInitModels. Mickey ends after counting the model table and does not have
 * JFG's later allocations; Mickey's globals, calls, and bytes are authoritative.
 */
void modInitModels(void) {
    D_800CB484 = func_8002B280(0x2A8, 0x8A);
    D_800CB488 = func_8002B280(0x190, 0x8A);
    D_800CB48C = 0;
    D_800CB494 = 0;
    D_800CB4A4 = func_8002B280(0x2000, 0x8A);
    D_800CB480 = piRomLoad(0x26);
    D_800CB490 = 0;
    while (D_800CB480[D_800CB490] != -1) {
        D_800CB490++;
    }
    D_800CB490--;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8001F520.s")
/*
 * PROVENANCE -- JFG's built models.c object supplies the exact corresponding
 * skeleton at func_8003BE68, but no public C body. This body is reconstructed
 * from Mickey's own function.
 */
void func_8001FB64(s32 count, MtxF *matrices) {
    while (count > 0) {
        count--;
        (*matrices)[0][0] = 1.0f;
        (*matrices)[0][1] = 0.0f;
        (*matrices)[0][2] = 0.0f;
        (*matrices)[0][3] = 0.0f;
        (*matrices)[1][0] = 0.0f;
        (*matrices)[1][1] = 1.0f;
        (*matrices)[1][2] = 0.0f;
        (*matrices)[1][3] = 0.0f;
        (*matrices)[2][0] = 0.0f;
        (*matrices)[2][1] = 0.0f;
        (*matrices)[2][2] = 1.0f;
        (*matrices)[2][3] = 0.0f;
        (*matrices)[3][0] = 0.0f;
        (*matrices)[3][1] = 0.0f;
        (*matrices)[3][2] = 0.0f;
        (*matrices)[3][3] = 1.0f;
        matrices++;
    }
}
typedef struct ModelCopySource {
    u8 pad0[0x12];
    s16 count;
    u8 pad14[8];
    u16 *data;
} ModelCopySource;

typedef struct ModelCopyAllocation {
    ModelCopySource *source;
    u16 *data;
    s16 unk8;
    s16 unkA;
} ModelCopyAllocation;

/*
 * PROVENANCE -- body adapted from JFG's public src/models.c
 * func_8003C12C_3CD2C. Mickey's header size, field widths, allocator, tag,
 * and linked bytes are authoritative.
 */
void *func_8001FBCC(ModelCopySource *source) {
    u16 *data;
    ModelCopyAllocation *allocation;

    allocation = func_8002B314(source->count * 0xA + 0xC, 0x8A);
    if (allocation != NULL) {
        data = (u16 *)((u8 *)allocation + 0xC);
        allocation->source = source;
        allocation->data = data;
        allocation->unk8 = 2;
        allocation->unkA = 0;
        func_8001F420(source->data, data, source->count * 0xA);
    }
    return allocation;
}

typedef struct ModelInstancePoint {
    s16 x;
    s16 y;
    s16 z;
    u8 flags[4];
} ModelInstancePoint;

typedef struct ModelInstanceCopy {
    s16 value0;
    s16 value2;
    u32 value4;
} ModelInstanceCopy;

typedef struct ModelInstancePointIndex {
    u16 pointIndex;
    u16 pad2;
} ModelInstancePointIndex;

typedef struct ModelInstanceSource {
    u8 pad0[0x10];
    u8 copyCount;
    u8 hasCopies;
    s16 pointCount;
    u8 pad14[8];
    ModelInstancePoint *points;
    u8 pad20[8];
    ModelInstanceCopy *copies;
    u8 pad2C;
    u8 coordinateCount;
    u8 dataCount44;
    u8 dataCount48;
    ModelInstancePointIndex *pointIndices;
    u8 pad34[0x1A];
    s8 mode;
    s8 matrixCount;
} ModelInstanceSource;

typedef struct ModelConstructedInstance {
    ModelInstanceSource *source;
    ModelInstancePoint *points;
    s16 status;
    u8 padA[2];
    MtxF *matricesA;
    MtxF *matricesB;
    ModelInstancePoint *pointsA;
    ModelInstancePoint *pointsB;
    u8 pad1C[8];
    void *modeData;
    u8 pad28[0x18];
    f32 *coordinates;
    void *data44;
    void *data48;
    ModelInstanceCopy *copies;
    s16 *stateA;
    s16 *stateB;
} ModelConstructedInstance;

/*
 * Mickey-only reconstruction; JFG retains the corresponding instance helper
 * as GLOBAL_ASM, so no donor body is adapted here.
 * Plateau: ten coherent layout/loop/lifetime attempts and a bounded permuter
 * leave the best canonical candidate at 330 instructions versus 333, with 300
 * differing positional words and the first mismatch at +0x0. Its 0x88 frame
 * retains four more non-save words than the target's 0x78 frame. The flag
 * lattice's function-local -Wo,-loopunroll,2 diagnostic is not exact and
 * cannot justify changing this TU's already-proven canonical flag group.
 */
#ifdef NON_MATCHING
ModelConstructedInstance *func_8001FC50(ModelInstanceSource *source, s32 pointCopies) {
    ModelConstructedInstance *instance;
    ModelConstructedInstance *instanceCursor;
    ModelInstancePoint *sourcePoint;
    ModelInstancePoint *destinationPoint;
    s16 *state;
    s32 matrixBytes;
    s32 doubleMatrixBytes;
    s32 pointBytes;
    s32 copiedPointBytes;
    s32 modeBytes[1];
    s32 dataBytes44;
    s32 dataBytes48;
    s32 coordinateBytes;
    s32 extraBytes;
    s32 allocationSize;
    s32 remainder;
    s32 i;
    s32 j;

    matrixBytes = 0;
    if (source->mode != 0) {
        s8 matrixCount = source->matrixCount;

        modeBytes[0] = matrixCount * 0x1C + 0xC;
        matrixBytes = matrixCount << 6;
    } else {
        modeBytes[0] = 0;
    }

    pointBytes = source->pointCount * sizeof(ModelInstancePoint);
    remainder = pointBytes & 7;
    if (remainder != 0) {
        pointBytes = pointBytes - remainder + 8;
    }
    dataBytes44 = source->dataCount44 * 0xC;
    remainder = dataBytes44 & 3;
    if (remainder != 0) {
        dataBytes44 = dataBytes44 - remainder + 4;
    }
    dataBytes48 = source->dataCount48 * 0xC;
    remainder = dataBytes48 & 3;
    if (remainder != 0) {
        dataBytes48 = dataBytes48 - remainder + 4;
    }
    coordinateBytes = source->coordinateCount * 0xC;
    remainder = coordinateBytes & 3;
    if (remainder != 0) {
        coordinateBytes = coordinateBytes - remainder + 4;
    }
    extraBytes = 0;
    if (source->hasCopies != 0) {
        extraBytes = source->copyCount * 8 + 0xA8;
    }

    copiedPointBytes = pointBytes * pointCopies;
    doubleMatrixBytes = matrixBytes << 1;
    allocationSize = doubleMatrixBytes + copiedPointBytes + modeBytes[0] + dataBytes44 +
                     dataBytes48 + coordinateBytes + extraBytes + 0x58;
    instance = func_8002B314(allocationSize, 0x8A);
    if (instance != NULL) {
        u32 *clear = (u32 *)instance;
        s32 words = allocationSize >> 2;

        while (words--) {
            *clear++ = 0;
        }

        if (source->matrixCount != 0 && matrixBytes != 0) {
            instance->matricesA = (MtxF *)((u8 *)instance + 0x58);
            instance->matricesB = (MtxF *)((u8 *)instance->matricesA + matrixBytes);
            func_8001FB64(source->matrixCount, instance->matricesA);
            func_8001FB64(source->matrixCount, instance->matricesB);
        }

        if (pointCopies > 0) {
            instance->pointsA = (ModelInstancePoint *)((u8 *)instance + doubleMatrixBytes + 0x58);
        } else {
            instance->pointsA = source->points;
        }
        if (pointCopies >= 2) {
            instance->pointsB = (ModelInstancePoint *)((u8 *)instance->pointsA + pointBytes);
        } else {
            instance->pointsB = instance->pointsA;
        }
        if (source->matrixCount != 0 && modeBytes[0] != 0) {
            instance->modeData = (u8 *)instance + doubleMatrixBytes + copiedPointBytes + 0x58;
        }
        if (source->dataCount44 != 0) {
            instance->data44 = (u8 *)instance + doubleMatrixBytes + copiedPointBytes + modeBytes[0] + 0x58;
        }
        if (source->dataCount48 != 0) {
            instance->data48 = (u8 *)instance + doubleMatrixBytes + copiedPointBytes + modeBytes[0] + dataBytes44 + 0x58;
        }
        if (source->coordinateCount != 0) {
            instance->coordinates = (f32 *)((u8 *)instance + doubleMatrixBytes + copiedPointBytes + modeBytes[0] +
                                              dataBytes44 + dataBytes48 + 0x58);
        }
        if (source->hasCopies != 0) {
            u8 *end;

            instance->copies = (ModelInstanceCopy *)((u8 *)instance + doubleMatrixBytes + copiedPointBytes +
                                                       modeBytes[0] + dataBytes44 + dataBytes48 + coordinateBytes + 0x58);
            end = (u8 *)(instance->copies + source->copyCount);
            remainder = (s32)end & 7;
            if (remainder != 0) {
                end = end - remainder + 8;
            }
            instance->stateA = (s16 *)end;
            instance->stateB = (s16 *)(end + 0x50);

            instanceCursor = instance;
            i = 0;
            do {
                j = 0;
state_reset_loop:
                state = (s16 *)((u8 *)instanceCursor->stateA + j);
                state[0] = 0;
                state[1] = 0;
                state[2] = 0;
                state[3] = 0;
                j += 8;
                if (j != 0x50) {
                    goto state_reset_loop;
                }
                i += 4;
                instanceCursor = (ModelConstructedInstance *)((u8 *)instanceCursor + 4);
            } while (i != 8);
        }

        i = 0;
        if (pointCopies > 0) {
            instanceCursor = instance;
            do {
                sourcePoint = source->points;
                destinationPoint = instanceCursor->pointsA;
                j = 0;
                if (source->pointCount > 0) {
                    do {
                        destinationPoint->x = sourcePoint->x;
                        destinationPoint->y = sourcePoint->y;
                        destinationPoint->z = sourcePoint->z;
                        destinationPoint->flags[0] = sourcePoint->flags[0];
                        destinationPoint->flags[1] = sourcePoint->flags[1];
                        destinationPoint->flags[2] = sourcePoint->flags[2];
                        destinationPoint->flags[3] = sourcePoint->flags[3];
                        j++;
                        sourcePoint++;
                        destinationPoint++;
                    } while (j < source->pointCount);
                }
                i++;
                instanceCursor = (ModelConstructedInstance *)((u8 *)instanceCursor + 4);
            } while (i != pointCopies);
        }

        if (source->mode == 0) {
            f32 *coordinate = instance->coordinates;

            i = 0;
            if (source->coordinateCount > 0) {
                do {
                    sourcePoint = &source->points[source->pointIndices[i].pointIndex];
                    *coordinate++ = sourcePoint->x;
                    *coordinate++ = sourcePoint->y;
                    *coordinate++ = sourcePoint->z;
                    i++;
                } while (i < source->coordinateCount);
            }
        }

        if (instance->copies != NULL) {
            i = 0;
            if (source->copyCount > 0) {
                do {
                    instance->copies[i].value0 = source->copies[i].value0;
                    instance->copies[i].value2 = source->copies[i].value2;
                    instance->copies[i].value4 = source->copies[i].value4;
                    i++;
                } while (i < source->copyCount);
            }
        }
        instance->source = source;
        instance->status = 2;
        instance->points = instance->pointsA;
    }
    return instance;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8001FC50.s")
#endif
/*
 * PROVENANCE -- body adapted from JFG's public modFreeModel. Mickey omits
 * JFG's per-instance animation allocations; its model reference and cache
 * release path has the same structure and is reconstructed against Mickey.
 */
void modFreeModel(ModelInstance *modInst) {
    ObjectModel *model;
    s32 i;
    s32 modelIndex;

    if (modInst != NULL) {
        model = modInst->objModel;
        mmFree(modInst);
        model->references--;
        if (model->references <= 0) {
            i = 0;
            modelIndex = -1;
            while (i < D_800CB48C) {
                if (model == (ObjectModel *)D_800CB484[(i << 1) + 1]) {
                    modelIndex = i;
                }
                i++;
            }

            if (modelIndex != -1) {
                func_80020278(model);
                D_800CB488[D_800CB494] = modelIndex;
                D_800CB494++;
                D_800CB484[modelIndex << 1] = -1;
                D_800CB484[(modelIndex << 1) + 1] = -1;
            }
        }
    }
}
/*
 * PROVENANCE -- body adapted from JFG's public func_8003C92C_3D52C model
 * destructor. Mickey's smaller field set and offsets are reconstructed solely
 * from this function's own loads and calls.
 */
void func_80020278(ObjectModel *model) {
    s32 freed;
    s32 index;

    index = 0, freed = 0;
    if (model->numberOfTextures > 0) {
        do {
            if (model->textures[index].texture != NULL) {
                func_800347A0(model->textures[index].texture);
            }
            freed++;
            index++;
        } while (freed < model->numberOfTextures);
    }

    if (model->unk58 != NULL) {
        mmFree(model->unk58);
    }
    if (model->unk28 != NULL) {
        mmFree(model->unk28);
    }
    if (model->unk68 != NULL) {
        mmFree(model->unk68);
    }
    if (model->unk6C != NULL) {
        mmFree(model->unk6C);
    }

    if (model->animationCount != 0 && model->animations != NULL) {
        freed = 0;
        index = 0;
        do {
            func_8005AAC0(model->animations[index]);
            freed++;
            index++;
        } while (freed < model->animationCount);
        mmFree(model->animations);
    }

    if (model->nestedAllocations != NULL) {
        freed = model->nestedCount + 1;
        while (freed--) {
            mmFree(model->nestedAllocations[freed]);
        }
        mmFree(model->nestedAllocations);
    }
    mmFree(model);
}
/* Mickey-only reconstruction; JFG supplied no adoptable helper name. */
void func_800203E0(ObjectModel *model) {
    s32 offset;
    s32 loaded;

    offset = 0;
    loaded = 0;
    if (model->numberOfTextures > 0) {
        do {
            if (((ModelTexture *)((u8 *)model->textures + offset))->texture == NULL) {
                ((ModelTexture *)((u8 *)model->textures + offset))->texture =
                    func_80034448(((ModelTexture *)((u8 *)model->textures + offset))->textureId);
            }
            loaded++;
            offset += sizeof(ModelTexture);
        } while (loaded < model->numberOfTextures);
    }
    if (model->unk68 == NULL) {
        model->textureAnimationCount = func_8002057C(&model->unk68, model, 0, 0, 0, 0xFF, 0);
    }
    if (model->unk6C == NULL) {
        func_8002057C(&model->unk6C, model, 4, 0, 0, 0xFF, 0);
    }
}
/* Mickey-only reconstruction; JFG supplied no adoptable helper name or body. */
void func_800204B8(ObjectModel *model) {
    s32 offset;
    s32 i;

    offset = 0;
    i = 0;
    if (model->numberOfTextures > 0) {
        do {
            if (((ModelTexture *)((u8 *)model->textures + offset))->texture != NULL) {
                func_800347A0(((ModelTexture *)((u8 *)model->textures + offset))->texture);
                ((ModelTexture *)((u8 *)model->textures + offset))->texture = NULL;
            }
            i++;
            offset += sizeof(ModelTexture);
        } while (i < model->numberOfTextures);
    }
    if (model->unk68 != NULL) {
        mmFree(model->unk68);
        model->unk68 = NULL;
    }
    if (model->unk6C != NULL) {
        mmFree(model->unk6C);
        model->unk6C = NULL;
    }
}
/*
 * PROVENANCE -- name follows JFG's public models.c symbol at the same TU
 * position. The body is reconstructed from Mickey's three instructions.
 */
void modelSetModelFlags(s32 flags) {
    D_80079C00 = flags;
}
/*
 * PROVENANCE -- name follows JFG's public models.c symbol at the same TU
 * position. The body is reconstructed from Mickey's three instructions.
 */
s32 modelGetModelFlags(void) {
    return D_80079C00;
}

typedef struct ModelGfxPart {
    u8 textureIndex;
    s8 group;
    u8 pad2[2];
    s8 segmentEnds[2];
    s16 vertexStart;
    s16 vertexIndex;
    u8 padA;
    u8 textureParameter;
    u32 flags;
} ModelGfxPart;

typedef struct ModelGfxTextureRef {
    void *texture;
    u32 pad4;
} ModelGfxTextureRef;

typedef struct ModelGfxCacheEntry {
    s16 parameter;
    s16 frame;
    u32 flags;
} ModelGfxCacheEntry;

typedef struct ModelGfxTexture {
    u8 pad0[3];
    u8 flags;
    s16 control;
    u8 pad6[0xA];
    u16 frameLimit;
} ModelGfxTexture;

typedef struct ModelGfxSource {
    u8 pad0[0x11];
    u8 hasTextures;
    u8 pad12[4];
    s16 partCount;
    ModelGfxTextureRef *textures;
    u8 pad1C[4];
    u8 *vertices;
    ModelGfxPart *parts;
    ModelGfxCacheEntry *cacheEntries;
    u8 pad2C[0x22];
    s8 mode;
} ModelGfxSource;

struct ModelTextureUsage;
void func_80020B10(Gfx **displayList, s8 *textureIds, s8 *slots,
                   struct ModelTextureUsage *usage, s32 entryIndex,
                   volatile u32 textureBase);

/*
 * Mickey-only reconstruction of the display-list builder. JFG supplies the
 * tier-B makeModelGfx role and TU position but retains its body as assembly.
 * Plateau: the complete 119-combination flag lattice and ten coherent
 * command-emission, measured-type, copy-loop, and lifetime spellings reach
 * the target's exact 342-instruction size under canonical -O2 -mips2 -32,
 * but 257 positional words still differ from +0x0. The candidate uses a
 * 0xC8 frame versus the target's 0xD0 and retains a different saved-register
 * and stack-home allocation. A bounded permuter's lower-scoring mutation
 * reused lastParameter as a command-word temporary, changing the following
 * texture-cache comparison, so it was rejected.
 */
#ifdef NON_MATCHING
s32 func_8002057C(Gfx **out, ModelGfxSource *model, s32 flags, s32 mask,
                  s32 lowerGroup, s32 upperGroup, s32 forceSimple) {
    Gfx *sourceDisplayList;
    ModelGfxPart *part;
    ModelGfxCacheEntry *cacheEntry;
    ModelGfxTexture *texture;
    void *lastTexture;
    s32 lastParameter;
    s32 cacheCount;
    s32 partIndex;
    s32 cacheEnabled;
    s32 vertexCount;
    s32 triangleCount;
    s16 parameter;
    s32 commandCount;
    u32 i;
    s32 previousVertex;
    Gfx *command;
    s8 slots[3];
    Gfx *displayList;

    part = model->parts;
    func_80034424(1);
    if (flags & 4) {
        D_8007BD98 = 1;
    }
    mask = ~mask;
    lastTexture = (void *)-1;
    lastParameter = -1;
    displayList = D_800CB4A4;
    if (lowerGroup == 0 && forceSimple == 0) {
        func_80020AD4();
        func_80034920(&displayList);
    } else if (forceSimple != 0) {
        func_80020AD4();
        func_80034920(NULL);
        command = displayList++;
        command->words.w1 = 0;
        command->words.w0 = 0xE7000000;
        command = displayList++;
        command->words.w1 = 0x10001;
        command->words.w0 = 0xB7000000;
    }

    partIndex = 0;
    cacheEntry = model->cacheEntries;
    cacheCount = 0;
    if (model->partCount > 0) {
        do {
            s8 group = part->group;
            s32 partFlags = part->flags;

            if (group >= lowerGroup && group <= upperGroup && !(partFlags & 0x800)) {
                s32 combinedFlags;
                s16 vertexStart = part->vertexStart;
                s16 vertexIndex = part->vertexIndex;
                u8 textureIndex = part->textureIndex;
                void *address;

                vertexCount = part[1].vertexStart - vertexStart;
                triangleCount = part[1].vertexIndex - vertexIndex;
                address = model->vertices + (vertexIndex << 4);
                if (textureIndex == 0xFF || forceSimple != 0) {
                    parameter = 0;
                    texture = NULL;
                    cacheEnabled = 0;
                } else {
                    parameter = part->textureParameter << 14;
                    texture = model->textures[textureIndex].texture;
                    cacheEnabled = 1;
                }

                combinedFlags = (partFlags | flags | D_80079C00) & mask;
                if (model->hasTextures != 0 && texture != NULL &&
                    (texture != lastTexture || parameter != lastParameter)) {
                    cacheEntry->parameter = parameter;
                    cacheEntry->frame = -1;
                    cacheEntry->flags = (part->flags & ~0xFF) | textureIndex;
                    if (texture->control & 0x40) {
                        cacheEntry->frame = parameter + 0x100;
                        if (cacheEntry->frame >= texture->frameLimit) {
                            if (texture->flags & 2) {
                                cacheEntry->frame = 0;
                            } else {
                                cacheEntry->frame -= 0x100;
                            }
                        }
                    }
                    cacheEntry++;
                    cacheCount++;
                }
                lastTexture = texture;
                lastParameter = parameter;

                func_800349A4(&displayList, texture, combinedFlags, parameter);
                if (model->mode == 0) {
                    s32 vertexAddress = (vertexStart * 0xA) & 0x0FFFFFFF;

                    command = displayList++;
                    command->words.w0 = ((((vertexCount << 3) | (vertexAddress & 6)) & 0xFF) << 16) |
                                        0x04000000 | ((vertexCount * 0xA + 8) & 0xFFFF);
                    command->words.w1 = vertexAddress;
                } else {
                    s32 slotIndex = 0;

                    previousVertex = 0;
                    func_80020B10(&displayList, &part->group, slots,
                                   (struct ModelTextureUsage *)model, partIndex, 0);
                    if (vertexCount > 0) {
                        s32 nextVertex;

                        do {
                            s32 segmentVertexCount;
                            s32 vertexAddress;

                            nextVertex = vertexCount;
                            command = displayList++;
                            command->words.w0 = 0xBC00000A;
                            command->words.w1 = slots[slotIndex] << 6;
                            if (slotIndex < 2) {
                                nextVertex = part->segmentEnds[slotIndex];
                            }
                            segmentVertexCount = nextVertex - previousVertex;
                            vertexAddress = ((vertexStart + previousVertex) * 0xA) & 0x0FFFFFFF;
                            command = displayList++;
                            command->words.w0 = ((((segmentVertexCount << 3) | (vertexAddress & 6)) & 0xFF) << 16) |
                                                0x04000000 |
                                                (((segmentVertexCount * 0xA + 8) |
                                                  (previousVertex << 9)) & 0xFFFF);
                            command->words.w1 = vertexAddress;
                            previousVertex = nextVertex;
                            slotIndex++;
                        } while (nextVertex < vertexCount);
                    }
                }

                command = displayList++;
                command->words.w0 = (((((triangleCount - 1) << 4) | cacheEnabled) & 0xFF) << 16) |
                                    0x05000000 | ((triangleCount << 4) & 0xFFFF);
                command->words.w1 = (s32)address & 0x0FFFFFFF;
            }
            partIndex++;
            part++;
        } while (partIndex < model->partCount);
    }

    command = displayList++;
    command->words.w1 = 0;
    command->words.w0 = 0xE7000000;
    command = displayList++;
    command->words.w1 = 0;
    command->words.w0 = 0xB8000000;

    commandCount = displayList - D_800CB4A4;
    displayList = *out = func_8002B314(commandCount * sizeof(Gfx), 0x8A);
    i = 0;
    if (displayList != NULL) {
        sourceDisplayList = D_800CB4A4;
        if (commandCount != 0) {
            do {
                displayList->words.w0 = sourceDisplayList->words.w0;
                displayList->words.w1 = sourceDisplayList->words.w1;
                sourceDisplayList++;
                displayList++;
                i++;
            } while (i < (u32)commandCount);
        }
    }
    func_80034424(0);
    D_8007BD98 = 0;
    return cacheCount;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8002057C.s")
#endif
/*
 * PROVENANCE -- JFG's built models.c object supplies the exact corresponding
 * skeleton at func_8003E100, but no public C body. This body is reconstructed
 * from Mickey's own function.
 */
void func_80020AD4(void) {
    s32 i;

    i = 0;
    do {
        i++;
        D_800CB498[i - 1] = -1;
        D_800CB49C[i - 1] = 1000;
    } while (D_800CB4A2 != &D_800CB49C[i]);
}

typedef struct ModelTextureUsageEntry {
    u8 pad0;
    s8 textureIds[3];
    u8 pad4[0xC];
} ModelTextureUsageEntry;

typedef struct ModelTextureUsage {
    u8 pad0[0x16];
    s16 entryCount;
    u8 pad18[0xC];
    ModelTextureUsageEntry *entries;
} ModelTextureUsage;

/* Plateau: canonical -O2 -mips2 is 160 instructions versus retail 159, with 156 positional differences from +0x0.
 * The 0x20-byte/six-save candidate misses retail's 0x10-byte/three-save shape after width, scope, gSP, and lifetime variants.
 * A 40-minute permuter improved 4520 to 2815 only through synthetic temporaries and an empty loop; no result was imported. */
#ifdef NON_MATCHING
void func_80020B10(Gfx **displayList, s8 *textureIds, s8 *slots,
                   ModelTextureUsage *usage, s32 entryIndex, volatile u32 textureBase) {
    s16 *count;
    s16 currentCount;
    s32 bestCount;
    ModelTextureUsageEntry *entry;
    s32 usageIndex;
    s32 textureIndex;
    s32 i;
    s8 *slotOut;
    s8 *textureId;
    s8 *cache;
    s8 cachedId;
    s32 slot;

    cache = D_800CB498;
    i = 0;
    do {
        cachedId = *cache;
        count = &D_800CB49C[i];
        if (cachedId != -1) {
            (*count)--;
            if (*count <= 0) {
                *count = 1;
                usageIndex = entryIndex + 1;
                if (usageIndex < usage->entryCount) {
                    do {
                        entry = &usage->entries[usageIndex];
                        if (cachedId == entry->textureIds[0] ||
                            cachedId == entry->textureIds[1] ||
                            cachedId == entry->textureIds[2]) {
                            usageIndex = usage->entryCount;
                        } else {
                            (*count)++;
                        }
                        usageIndex++;
                    } while (usageIndex < usage->entryCount);
                }
            }
        }
        i++;
        cache++;
    } while (i != 3);

    textureIndex = 0;
    slotOut = slots;
    textureId = textureIds;
    do {
        *slotOut = -1;
        textureIndex++;
        i = 0;
        if (*textureId != -1) {
            cache = D_800CB498;
            do {
                if (*cache == *textureId) {
                    *slotOut = i + 1;
                }
                i++;
                cache++;
            } while (i < 3);

            slot = -1;
            i = 0;
            if (*slotOut == -1) {
                cache = D_800CB498;
                do {
                    if (*cache == -1) { slot = i; } i++;
                    cache++;
                } while (i < 3 && slot == -1);

                if (slot == -1) {
                    cache = D_800CB498;
                    i = 0;
                    bestCount = 0;
                    do {
                        cachedId = *cache;
                        if (cachedId != textureIds[0] &&
                            cachedId != textureIds[1] &&
                            cachedId != textureIds[2]) {
                            count = &D_800CB49C[i];
                            currentCount = *count;
                            if (bestCount < currentCount) {
                                bestCount = currentCount;
                                slot = i;
                            }
                        }
                        i++;
                        cache++;
                    } while (i != 3);
                }

                D_800CB498[slot] = *textureId;
                D_800CB49C[slot] = 0;
                *slotOut = slot + 1;
                gSPMatrix((*displayList)++, ((*textureId << 6) + textureBase) & 0x0FFFFFFF,
                          *slotOut | 0x80);
            }
        }
        slotOut++;
        textureId++;
    } while (textureIndex != 3);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_80020B10.s")
#endif
#ifdef NON_MATCHING
typedef struct ModelFrameEntry {
    s16 frame;
    s16 nextFrame;
    u32 textureIndex;
} ModelFrameEntry;

typedef struct ModelTextureHeader {
    u8 pad0[0xE];
    u16 frameScale;
    u16 frameCount;
} ModelTextureHeader;

typedef struct ModelFrameInstance {
    ObjectModel *model;
    u8 pad4[6];
    s16 outputIndex;
    u8 padC[0x40];
    ModelFrameEntry *entries;
    u16 *outputs[1];
} ModelFrameInstance;

/* Mickey-only reconstruction; JFG's modSetTextureFrame remains assembly. */
void func_80020D8C(ModelFrameInstance *instance, s32 textureIndex, s32 frame) {
    ObjectModel *model;
    ModelFrameEntry *entry;
    u16 *output;
    s32 remaining;
    s32 remainingCopy;

    model = instance->model;
    entry = instance->entries;
    output = instance->outputs[instance->outputIndex];
    remaining = *((u8 *)model + 0x2C);
    remainingCopy = remaining;
    remaining--;
    if (remainingCopy != 0) {
        do {
            s32 index = entry->textureIndex & 0xFF;
            ModelTextureHeader *texture = model->textures[index].texture;
            s16 nextFrame;
            s16 outputValue;
            u16 frameScale;

            if (index == textureIndex && frame < texture->frameCount) {
                entry->frame = frame;
            }
            frameScale = texture->frameScale;
            nextFrame = entry->nextFrame;
            outputValue = (entry->frame >> 8) * frameScale;
            output++;
            remainingCopy = remaining;
            entry++;
            output[-1] = outputValue;
            if (nextFrame >= 0) {
                output++;
                output[-1] = (nextFrame >> 8) * frameScale;
            }
            remaining--;
        } while (remainingCopy != 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_80020D8C.s")
#endif
#ifdef NON_MATCHING
typedef struct ModelCacheEntry {
    s32 id;
    ObjectModel *model;
} ModelCacheEntry;

/* Mickey-only reconstruction; JFG's modSuspendModelTextures remains assembly.
 * Plateau: 112/113 words, 77 differ, first +0x64; the exception-index scale
 * remains absent after type, assignment, induction, flag, and permuter sweeps. */
void func_80020E4C(s16 *exceptions) {
    SuspendedModelTexture *saved;
    s32 modelIndex;

    D_80079C08 = 0;
    saved = D_80079C04 = func_8002B280(0x3E8, 0x8A);
    modelIndex = 0;
    if (D_800CB48C > 0) {
        if (D_80079C08 < 0x7D) {
            do {
                ModelCacheEntry *cache = (ModelCacheEntry *)((u8 *)D_800CB484 + (modelIndex << 3));
                s32 modelId = cache->id;

                if (modelId != -1) {
                    ObjectModel *model = cache->model;
                    s32 excluded = 0;
                    s32 exceptionIndex = 0;

                    if (*exceptions != -1) {
                        do {
                            if (exceptions[exceptionIndex] == modelId) {
                                excluded = 1;
                            }
                            exceptionIndex++;
                        } while (exceptions[exceptionIndex] != -1 && excluded == 0);
                    }
                    if (excluded == 0) {
                        s32 textureIndex = 0;

                        if (model->numberOfTextures > 0) {
                            if (D_80079C08 < 0x7D) {
                                do {
                                    saved->value = (s32)model->textures[textureIndex].texture;
                                    saved->id = func_8003484C(model->textures[textureIndex].texture);
                                    func_800347A0(model->textures[textureIndex].texture);
                                    textureIndex++;
                                    D_80079C08++;
                                    saved++;
                                } while (textureIndex < model->numberOfTextures && D_80079C08 < 0x7D);
                            }
                        }
                    }
                }
                modelIndex++;
            } while (modelIndex < D_800CB48C && D_80079C08 < 0x7D);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_80020E4C.s")
#endif
/*
 * PROVENANCE -- name and TU position follow JFG's public
 * modResumeModelTextures symbol. JFG has no public C body; Mickey is the body
 * and global-layout authority.
 */
void modResumeModelTextures(void) {
    SuspendedModelTexture *saved = D_80079C04;

    if (saved != NULL) {
        SuspendedModelTexture *entry = saved;
        s32 i = 0;
        if (D_80079C08 > 0) {
            do {
                if (entry->value != 0) {
                    func_800348A0(entry->id, entry->value);
                }
                i++;
                entry++;
            } while (i < D_80079C08);
        }
        mmFree(D_80079C04);
        D_80079C08 = 0;
    }
}

typedef struct ModelPointRecord {
    s16 x;
    s16 y;
    s16 z;
    u8 pad6[4];
} ModelPointRecord;

typedef struct ModelPointIndex {
    u16 pointIndex;
    u16 pad2;
} ModelPointIndex;

typedef struct ModelPointSource {
    u8 pad0[0x1C];
    ModelPointRecord *points;
    u8 pad20[0xD];
    u8 pointCount;
    u8 pad2E[2];
    ModelPointIndex *indices;
} ModelPointSource;

typedef struct ModelPointOutput {
    ModelPointSource *source;
    u8 pad4[0x3C];
    f32 *points;
} ModelPointOutput;

typedef struct ModelPointOwner {
    u8 pad0[0x68];
    ModelPointOutput **output;
} ModelPointOwner;

void func_8002AA50(void *transform, MtxF matrix);
void mtxf_transform_point(MtxF matrix, f32 x, f32 y, f32 z,
                          f32 *outX, f32 *outY, f32 *outZ);

/* Mickey-only reconstruction; JFG's candidate model helpers remain assembly. */
void func_8002109C(ModelPointOwner *owner) {
    MtxF matrix;
    ModelPointOutput *output;
    ModelPointSource *source;
    ModelPointRecord *point;
    f32 *outputPoint;
    s32 i;

    output = *owner->output;
    source = output->source;
    func_8002AA50(owner, matrix);
    outputPoint = output->points;
    /* IDO's zero-initialization register schedule depends on this line grouping. */
    i = 0; if (source->pointCount > 0) { do {
            point = &source->points[source->indices[i].pointIndex];
            mtxf_transform_point(matrix, point->x, point->y, point->z,
                                 outputPoint, outputPoint + 1, outputPoint + 2);
            i++;
            outputPoint += 3;
        } while (i < source->pointCount);
    }
}
