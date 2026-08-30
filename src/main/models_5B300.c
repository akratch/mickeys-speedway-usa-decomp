/*
 * Model animation loading and matrix generation -- ROM 0x5B300-0x5C310
 * (VRAM 0x8005A700-0x8005B710).
 *
 * The TU name is Tier B/D: its callers and data flow establish animation-table
 * loading, reference-counted animation allocation, frame selection and model
 * matrix construction. JFG models.c supplies the nearest non-exact skeletons
 * for the loader/free pair. camConvertMatrixList alone is Tier A against JFG
 * camera.c; that isolated helper does not turn the full range into camera.c.
 *
 * PROVENANCE: JFG's permitted src/models.c, models.h and camera.c were read
 * for names, layouts and comparison. The initial split adapted no body;
 * point-of-use notes identify the later JFG adaptations. The func_8005A948
 * flag lattice showed that this TU's cache loop has target length only with
 * `-Wo,-loopunroll,0`, recorded as a per-file Makefile override.
 */

#include "PR/ultratypes.h"
#include "game/pi.h"

typedef f32 Matrix[4][4];

typedef struct ConvListEntry {
    Matrix *mtx;
    s16 count;
} ConvListEntry;

typedef struct AnimationCacheEntry {
    s32 id;
    u8 *animation;
} AnimationCacheEntry;

typedef struct LoadedAnimation {
    u8 references;
    u8 pad1[3];
    s16 id;
} LoadedAnimation;

typedef struct ModelMatrixNode {
    s16 parent;
    u8 pad2[2];
    f32 x;
    f32 y;
    f32 z;
} ModelMatrixNode;

typedef struct ModelAnimationTable {
    u8 pad0[0x4E];
    s8 animationCount;
    u8 pad4F;
    u8 **animations;
} ModelAnimationTable;

typedef struct ModelAnimationFrame {
    u8 pad0;
    u8 flags;
    s16 offset;
    u8 pad4[2];
    u8 loop;
    u8 pad7;
    u8 count;
} ModelAnimationFrame;

typedef struct ModelAnimationInfo {
    u8 pad0[0x4E];
    s8 frameCount;
    u8 pad4F;
    ModelAnimationFrame **frames;
} ModelAnimationInfo;

typedef struct ModelAnimationState {
    ModelAnimationInfo *info;
    u8 pad4[0x18];
    ModelAnimationFrame *frame;
    void *frameData;
    u8 pad24[4];
    f32 frameValue;
    f32 pad2C;
    f32 blendStart;
    f32 blendEnd;
    f32 blendValue;
    s16 frameIndex;
    s8 transition;
    u8 hasNext;
} ModelAnimationState;

typedef struct ModelAnimationInstance {
    u8 pad0[0x28];
    f32 frameValue;
    u8 pad2C[0xE];
    s8 animationIndex;
    s8 frame;
    u8 pad3C[0x2C];
    ModelAnimationState **states;
} ModelAnimationInstance;

extern s32 D_800D7CF0;
extern s32 D_800D7CF4;
extern s32 D_800D7CF8;
extern s32 D_800D7CFC;
extern s32 D_800D7D00;
extern s32 D_800D7D04;
extern ConvListEntry D_800D78F0[];

s32 func_8002B280(s32 size, s32 tag);
u8 *func_8002B314(s32 size, s32 tag);
void func_80058FF0(ConvListEntry *entries, s32 count);
void func_8002A82C(void *mtx);
void mtxf_mul(void *lhs, void *rhs, void *dest);
void mmFree(void *ptr);
u8 *func_8005A948(s16 animationId);
void func_8005AAC0(u8 *animation);

/* PROVENANCE: adapted from the modelsInit tail in JFG src/models.c. */
void func_8005A700(void) {
    s32 allocation;

    allocation = func_8002B280(0xA0, 0x80);
    D_800D7CFC = allocation;
    D_800D7D00 = allocation + 0x80;
    D_800D7CF8 = allocation + 0x90;
    D_800D7CF4 = func_8002B280(0x800, 0x80);
    D_800D7D04 = 0;
    D_800D7CF0 = 0;
}
void func_8005A764(void) {
    D_800D7CF0 = 0;
}
void func_8005A770(void) {
    func_80058FF0(D_800D78F0, D_800D7CF0);
    D_800D7CF0 = 0;
}
/*
 * Plateau: workbench mixed constant/structure/register, 106/106 instructions, candidate frame -0x50 vs target -0x38, first +0x0.
 * Levers tried: frame/register storage, parameter-local coalescing, compound-and shape, and a scoped model-index lifetime; none improved.
 * Remaining: alignment stays in v1/sp+0x30 instead of target s0/sp+0x34, shifting the second load-call schedule.
 */
#ifdef NON_MATCHING
s32 func_8005A7A0(ModelAnimationTable *model, s32 modelId) {
    s32 alignment;
    s32 firstAnimation;
    s32 lastAnimation;
    s32 loadSize;
    s32 loaded;
    s32 inputOffset;
    u16 *bounds;

    piRomLoadSection(0x28, (void *)D_800D7D00, (modelId & ~3) * 2, 0x10);
    bounds = (u16 *)D_800D7D00 + (modelId & 3);
    firstAnimation = bounds[0] >> 1;
    lastAnimation = bounds[1] >> 1;
    model->animationCount = lastAnimation - firstAnimation;
    if (firstAnimation == lastAnimation) {
        return TRUE;
    }

    alignment = firstAnimation & 3;
    loadSize = ((model->animationCount & ~3) + 4) << 1;
    if (alignment != 0) {
        loadSize += 8;
    }
    piRomLoadSection(0x29, (void *)D_800D7CFC, (firstAnimation & ~3) * 2, loadSize);
    model->animations = (u8 **)func_8002B314(model->animationCount * 4, 0x80);
    if (model->animations == NULL) {
        return FALSE;
    }

    loaded = 0;
    inputOffset = alignment * 2;
    alignment = 0;
    do {
        *(u8 **)((u8 *)model->animations + alignment) =
            func_8005A948(*(s16 *)(D_800D7CFC + inputOffset));
        if (*(u8 **)((u8 *)model->animations + alignment) == NULL) {
            alignment = 0;
            if (loaded > 0) {
                inputOffset = 0;
                do {
                    func_8005AAC0(*(u8 **)((u8 *)model->animations + inputOffset));
                    alignment++;
                    inputOffset += 4;
                } while (alignment != loaded);
            }
            mmFree(model->animations);
            model->animations = NULL;
            return FALSE;
        }
        loaded++;
        inputOffset += 2;
        alignment += 4;
    } while (loaded < model->animationCount);
    return TRUE;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/models_5B300/func_8005A7A0.s")
#endif
/* Plateau (near-miss p8): workbench register-permutation, 11 register-only words at 94 instructions/frame -0x38.
 * Separate indices, pointer traversal, scoped/direct existing-entry forms, and final-entry materialization were rechecked.
 * They were inert or changed the target shape; the cache-index temp phase and final empty-index allocation stay unresolved. */
#ifdef NON_MATCHING
u8 *func_8005A948(s16 animationId) {
    s32 i;
    s32 emptyIndex;
    s32 offset;
    s32 size;
    LoadedAnimation *animation;

    emptyIndex = -1;
    i = 0;
    if (D_800D7D04 > 0) {
        do {
            AnimationCacheEntry *entry = &((AnimationCacheEntry *)D_800D7CF4)[i];

            if ((animationId == entry->id) != 0U) {
                u8 *existing = entry->animation;

                existing[0]++;
                return existing;
            }
            if (entry->id == -1) {
                emptyIndex = i;
            }
            i++;
        } while (i < D_800D7D04);
    }

    if (emptyIndex == -1) {
        emptyIndex = D_800D7D04;
        if (D_800D7D04 >= 0x100) {
            return NULL;
        }
        D_800D7D04++;
    }

    i = (animationId & 1) * 4;
    piRomLoadSection(0x2A, (u8 *)D_800D7CF8, (animationId & ~1) * 4, 0x10);
    offset = *(s32 *)(D_800D7CF8 + i);
    size = *(s32 *)(D_800D7CF8 + i + 4) - offset;
    animation = (LoadedAnimation *)func_8002B314(size, 0x80);
    if (animation == NULL) {
        return NULL;
    }

    piRomLoadSection(0x2B, animation, offset, size);
    animation->references = 1;
    animation->id = animationId;
    ((AnimationCacheEntry *)D_800D7CF4)[emptyIndex].id = animationId;
    ((AnimationCacheEntry *)D_800D7CF4)[emptyIndex].animation = (u8 *)animation;
    return (u8 *)animation;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/models_5B300/func_8005A948.s")
#endif

/* PROVENANCE: adapted from Jet Force Gemini's public src/models.c:modFreeAnim;
 * Mickey's cache entry layout and teardown call establish this exact body. */
void func_8005AAC0(u8 *animation) {
    s32 i;
    s32 index;

    if (animation != NULL) {
        animation[0]--;
        if (animation[0] > 0) {
            return;
        }
        index = -1;
        if (D_800D7D04 > 0) {
            i = 0;
            do {
                if (animation == ((u8 **)D_800D7CF4)[(i << 1) + 1]) {
                    index = i;
                }
                i++;
            } while (i < D_800D7D04);
        }
        if (index != -1) {
            mmFree(animation);
            ((s32 *)D_800D7CF4)[index * 2] = -1;
            ((s32 *)D_800D7CF4)[index * 2 + 1] = -1;
        }
    }
}
/* PROVENANCE: adapted from JFG src/camera.c (camConvertMatrixList). */
void camConvertMatrixList(Matrix *mtx, s32 count) {
    s32 index = D_800D7CF0;
    ConvListEntry *entry = &D_800D78F0[index];

    entry->mtx = mtx;
    D_800D7CF0 = index + 1;
    entry->count = count;
}

/* Keep the original TU order: func_8005ABA8 precedes func_8005AD64. */
/* Workbench: structure-mismatch, 97 differing words, first mismatch +0x38. */
/* Candidate shape: 110 instructions/no frame vs target 111/no frame; not permuter-ready. */
/* Remaining structural gap: preserve the validated frame pointer in a2 before the split. */
/* PROVENANCE: Mickey-only reconstruction from func_8005ABA8.s and the
 * existing models TU layouts; no external function body is copied. */
#ifdef NON_MATCHING
s32 func_8005ABA8(ModelAnimationInstance *instance, f32 arg1, f32 arg2) {
    s32 var_v1;
    f32 temp_f0;
    f32 temp_f0_2;
    f32 temp_f2;
    f32 temp_f2_2;
    void *temp_a1;
    ModelAnimationState *temp_v0;

    temp_v0 = instance->states[(s32)instance->animationIndex];
    var_v1 = 0;
    temp_a1 = temp_v0->frame;
    if (temp_a1 == NULL) {
        return 0;
    }
    if (temp_v0->transition != 0) {
        if (temp_v0->hasNext != 0) {
            temp_f2 = temp_v0->blendEnd;
            temp_f0 = temp_v0->blendValue + arg2;
            temp_v0->blendValue = 0.0f;
            temp_v0->blendEnd = temp_f2 - temp_f0;
            temp_v0->blendStart = temp_f0 / temp_f2;
        } else {
            temp_v0->blendValue = temp_v0->blendValue + arg2;
        }
        temp_f2_2 = temp_v0->blendEnd;
        if ((temp_f2_2 <= 0.0f) || (temp_f2_2 <= temp_v0->blendValue)) {
            temp_v0->transition = 0;
            temp_v0->blendStart = 0.0f;
            temp_v0->blendValue = 0.0f;
            instance->frameValue = (f32)temp_v0->frameIndex /
                                   temp_v0->frameValue;
        }
    } else {
        instance->frameValue += arg1 * arg2;
        temp_f0_2 = instance->frameValue;
        if (temp_f0_2 >= 1.0f) {
            if (((ModelAnimationFrame *)temp_a1)->loop != 0) {
                if (temp_f0_2 >= 1.0f) {
                    do {
                        instance->frameValue -= 1.0f;
                    } while (instance->frameValue >= 1.0f);
                    var_v1 = 1;
                } else {
                    goto animation_done;
                }
            } else {
                instance->frameValue = 1.0f;
animation_done:
                var_v1 = 1;
            }
        } else if (temp_f0_2 < 0.0f) {
            var_v1 = 1;
            if (((ModelAnimationFrame *)temp_a1)->loop != 0) {
                if (temp_f0_2 < 0.0f) {
                    do {
                        instance->frameValue += 1.0f;
                    } while (instance->frameValue < 0.0f);
                }
            } else {
                instance->frameValue = 0.0f;
            }
        }
    }
    return var_v1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/models_5B300/func_8005ABA8.s")
#endif
/* PROVENANCE: Mickey-only reconstruction from func_8005AD64.s and the
 * existing models TU layouts; no external function body is copied. */
/* Exact configured C: 108 words, no stack frame or relocations. The canonical
 * linked resident range and full ROM are byte-identical. */
void func_8005AD64(ModelAnimationInstance *instance, s32 frame, s32 arg2,
                   f32 value) {
    f32 temp_f0;
    s32 temp_f6;
    s32 var_a1;
    s32 temp_a1;
    s32 var_v1;
    ModelAnimationFrame *temp_a0;
    ModelAnimationState *temp_v0;
    ModelAnimationInfo *temp_v1;

    temp_v0 = instance->states[(s32)instance->animationIndex];
    temp_v1 = temp_v0->info;
    if (temp_v1->frameCount != 0) {
        if (value > 1.0f) {
            value = 1.0f;
        } else if (value < 0.0f) {
            value = 0.0f;
        }
        instance->frameValue = value;
        temp_a1 = temp_v1->frameCount;
        if (frame >= temp_a1) {
            frame = temp_a1 - 1;
        } else if (frame < 0) {
            frame = 0;
        }
        instance->frame = frame;
        var_a1 = 0;
        if ((temp_v0->frame != NULL) && (temp_v0->hasNext != 0)) {
            var_a1 = 1;
        }
        temp_a0 = temp_v1->frames[frame];
        temp_v0->frame = temp_a0;
        temp_v0->frameData = (u8 *)temp_a0 + temp_a0->offset + 0x14;
        temp_v0->frameValue = (f32)temp_a0->count;
        if (temp_a0->loop == 0) {
            temp_v0->frameValue = temp_v0->frameValue - 1.0f;
        }
        if (arg2 != -1) {
            var_v1 = arg2;
        } else {
            var_v1 = temp_a0->flags;
        }
        if ((var_a1 != 0) && (var_v1 != 0)) {
            temp_v0->blendEnd = (f32)var_v1;
            temp_f0 = temp_v0->frameValue * value;
            temp_v0->transition = 1;
            temp_v0->blendStart = 0.0f;
            temp_f6 = (s32)temp_f0;
            if ((temp_f0 - (f32)temp_f6) >= 0.5f) {
                temp_v0->frameIndex = temp_f6 + 1;
                return;
            }
            temp_v0->frameIndex = temp_f6;
        }
    }
}

/*
 * Plateau: the animation-frame update's closest reconstruction emits 110
 * instructions against 111 and follows the broad target CFG, but diverges at
 * +0x38 before cascading through the FP allocator. The 119-combination flag
 * lattice found no exact result; its closest alternate still differs in 59
 * words and would also perturb this TU's already-exact functions.
 */
/*
 * Plateau: this 0x730-byte matrix/attachment builder remains blocked on
 * unknown model-node and attachment layouts. The permitted JFG peer is also
 * assembly, and the Mickey m2c draft cannot establish the field semantics
 * needed for a clean-room C reconstruction.
 */
#pragma GLOBAL_ASM("asm/nonmatchings/main/models_5B300/func_8005AF14.s")

/* Mickey-derived parented matrix-list builder; JFG retains its peer as asm. */
void func_8005B644(Matrix *matrices, Matrix *root, ModelMatrixNode *node, s32 count) {
    Matrix temp;
    Matrix *output;
    Matrix *parent;
    s32 i;

    output = matrices;
    i = 0;
    if (count > 0) {
        do {
            func_8002A82C((u8 *)temp - 8);
            *(f32 *)((u8 *)temp + 0x28) = node->x;
            *(f32 *)((u8 *)temp + 0x2C) = node->y;
            *(f32 *)((u8 *)temp + 0x30) = node->z;
            if ((node->parent == -1) != FALSE) {
                parent = root;
            } else {
                parent = &matrices[node->parent];
            }
            mtxf_mul((u8 *)temp - 8, parent, output);
            i++;
            output++;
            node++;
        } while (i != count);
    }
}
