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

extern s32 D_800D7CF0;
extern s32 D_800D7CF4;
extern s32 D_800D7CF8;
extern s32 D_800D7CFC;
extern s32 D_800D7D00;
extern s32 D_800D7D04;
extern ConvListEntry D_800D78F0[];

s32 func_8002B280(s32 size, s32 tag);
u8 *func_8002B314(s32 size, s32 tag, s32 offset);
void piRomLoadSection(s32 assetId, void *dst, s32 offset, s32 size);
void func_80058FF0(ConvListEntry *entries, s32 count);

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
#pragma GLOBAL_ASM("asm/nonmatchings/main/models_5B300/func_8005A7A0.s")
/*
 * Plateau: with -Wo,-loopunroll,0 this reconstruction has the target's exact
 * 94 instructions, frame, opcode sequence, CFG and relocations. Its best
 * object still differs in 31 words: 28 register operands and three accesses
 * to emptyIndex at 0x34(sp) instead of the target's 0x30(sp). The first
 * mismatch is +0x40, where IDO assigns the cache-index shift to t7 instead of
 * the target's t8. Natural declaration order, a separate entry-offset local,
 * a pointer-form scan, a struct-form empty slot and reordered declarations
 * either preserve that allocator split or regress it, so the best C remains
 * available for future compiler-allocation work without entering the ROM.
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

            if (animationId == entry->id) {
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
    animation = (LoadedAnimation *)func_8002B314(size, 0x80, offset);
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/models_5B300/func_8005AAC0.s")
/* PROVENANCE: adapted from JFG src/camera.c (camConvertMatrixList). */
void camConvertMatrixList(Matrix *mtx, s32 count) {
    s32 index = D_800D7CF0;
    ConvListEntry *entry = &D_800D78F0[index];

    entry->mtx = mtx;
    D_800D7CF0 = index + 1;
    entry->count = count;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/models_5B300/func_8005ABA8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models_5B300/func_8005AD64.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models_5B300/func_8005AF14.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models_5B300/func_8005B644.s")
