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

extern s32 D_80079C00;
extern void *D_80079C04;
extern s16 D_80079C08;
extern s32 *D_800CB480;
extern s32 *D_800CB484;
extern s32 *D_800CB488;
extern s32 D_800CB48C;
extern s32 D_800CB490;
extern s32 D_800CB494;
extern s8 D_800CB498[];
extern s16 D_800CB49C[];
extern s16 D_800CB4A2[];
extern s32 *D_800CB4A4;

void *func_8002B280(s32 size, s32 tag);
void *func_8002B314(s32 size, s32 tag);
s32 *piRomLoad(s32 assetId);
void *func_80034448(s16 textureId);
void func_800347A0(void *texture);
void func_800348A0(s32 id, s32 value);
void func_8005AAC0(void *animation);
u8 func_8002057C(void **out, ObjectModel *model, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8001FC50.s")
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_800204B8.s")
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8002057C.s")
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_80020B10.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_80020D8C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_80020E4C.s")
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8002109C.s")
