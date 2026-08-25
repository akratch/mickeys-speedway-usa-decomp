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
 * Workbench: mixed structure/register/frame; exact 106-instruction length, frame 0x50 vs 0x38, first +0x0.
 * Levers: three-step load size, local reuse/scopes, and alignment priority reads plateaued at 10 words.
 * Remaining: alignment spills through v1 instead of s0, shifting the load-call schedule; JFG peer is asm.
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
    loadSize = model->animationCount & ~3;
    loadSize += 4;
    loadSize *= 2;
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
/*
 * Plateau: exact 94 words/frame/CFG; 18 words differ, first +0x40, from a
 * temp-FIFO shift plus emptyIndex at 0x34(sp) rather than 0x30(sp). Flags,
 * natural bool locals and loop/access reshaping regress; explicit != 0U helps.
 */
#ifdef NON_MATCHING
u8 *func_8005A948(s16 animationId) {
    s32 emptyIndex = -1;
    s32 i = 0;
    s32 offset;
    s32 size;
    LoadedAnimation *animation;

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

    piRomLoadSection(0x2A, (u8 *)D_800D7CF8, (animationId & ~1) * 4, 0x10);
    offset = *(s32 *)(D_800D7CF8 + ((animationId & 1) * 4));
    size = *(s32 *)(D_800D7CF8 + ((animationId & 1) * 4) + 4) - offset;
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

/*
 * Mickey-only reconstruction; JFG retains modFreeAnim as assembly.
 * Workbench verdict: temp-FIFO phase shift; exact 46-word size, first +0x40.
 * Levers 15 and 16 added instructions or disturbed the pool/register shape.
 * Remaining: 14 words and one invisible temp pop before the cache loop.
 */

#ifdef NON_MATCHING
void func_8005AAC0(u8 *animation) {
    s32 index;
    s32 i;

    if (animation != NULL) {
        animation[0]--;
        if (animation[0] > 0) {
            return;
        }
        index = -1;
        if (D_800D7D04 > 0) {
            i = 0;
            do {
                if (animation == ((AnimationCacheEntry *)((u8 *)D_800D7CF4 + (i << 3)))->animation) {
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
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/models_5B300/func_8005AAC0.s")
#endif
/* PROVENANCE: adapted from JFG src/camera.c (camConvertMatrixList). */
void camConvertMatrixList(Matrix *mtx, s32 count) {
    s32 index = D_800D7CF0;
    ConvListEntry *entry = &D_800D78F0[index];

    entry->mtx = mtx;
    D_800D7CF0 = index + 1;
    entry->count = count;
}

/*
 * Plateau: the animation-frame update's closest reconstruction emits 110
 * instructions against 111 and follows the broad target CFG, but diverges at
 * +0x38 before cascading through the FP allocator. The 119-combination flag
 * lattice found no exact result; its closest alternate still differs in 59
 * words and would also perturb this TU's already-exact functions.
 */
#pragma GLOBAL_ASM("asm/nonmatchings/main/models_5B300/func_8005ABA8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models_5B300/func_8005AD64.s")

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
