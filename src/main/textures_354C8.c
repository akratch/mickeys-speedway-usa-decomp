#include "PR/ultratypes.h"
#include "n_audio/gbi.h"

/*
 * PROVENANCE: the texture-TU order and direct helper bodies below were
 * compared with Jet Force Gemini's public src/textures.c. Mickey's field
 * offsets, globals, boundaries, and compiler output remain authoritative.
 */

extern s32 D_8007BD84;
extern s32 D_8007BD88;
extern s32 D_8007BD8C;
extern s32 D_8007BD90;
extern s32 D_800D3020;
extern s32 D_800D3024;
extern s32 D_800D3028;
extern s32 D_800D302C;
extern s32 D_800D3030;
extern s32 D_800D3034;
extern s32 D_800D3004;
extern void *D_800D3000;
extern u8 D_800D3038;
extern u8 D_800D3039;
extern u8 D_800D303A;
extern u8 D_800D303B;
extern u8 D_800D303C;
extern u8 D_800D303D;
extern s32 D_8007BD9C;

typedef struct TextureFrameHeader {
    u8 pad00[2];
    u8 format;
    u8 pad03;
    s16 flags;
    u16 width;
    u16 height;
    u8 pad0A[4];
    u16 textureSize;
    u16 numOfTextures;
    u16 frameAdvanceDelay;
    Gfx *cmd;
    u16 numberOfCommands;
    u8 pad1A;
    u8 unk1B;
    u8 unk1C;
    u8 isCompressed;
    u8 unk1E;
    u8 unk1F;
} TextureFrameHeader;

#ifdef NON_MATCHING
#define TEXTURE_FIELD(expr, type_ptr, offset) (*(type_ptr)((u8 *)(expr) + (offset)))
#endif

extern u8 D_8007BDA0;
extern s32 func_800299E8(s32 minimum, s32 maximum);
extern void mmFree(void *ptr);

void func_80035F48(u8 **dlist, u8 *tex, s32 rtile, s32 tmem);

void func_800348C8(s32 tagId) {
    D_8007BD84 = tagId;
}

TextureFrameHeader *func_800348D4(TextureFrameHeader *arg0, s32 arg1) {
    TextureFrameHeader *ret = arg0 + 1;
    if ((arg1 > 0) && (arg1 < arg0->numOfTextures << 8)) {
        ret = (TextureFrameHeader *)(((u8 *)arg0) +
                                     ((arg1 >> 16) * arg0->textureSize)) + 1;
    }
    return ret;
}

void func_80034910(void) {
    D_8007BD8C = 1;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_354C8/func_80034920.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_354C8/func_800349A4.s")

void func_80034DE4(s32 value) {
    D_8007BD88 = value;
}

void func_80034DF0(u8 red, u8 green, u8 blue, u8 alternateRed,
                   u8 alternateGreen, u8 alternateBlue) {
    D_800D3038 = red;
    D_800D3039 = green;
    D_800D303A = blue;
    D_800D303B = alternateRed;
    D_800D303C = alternateGreen;
    D_800D303D = alternateBlue;
    D_8007BD9C = 1;
}

void func_80034E48(void) {
    D_8007BD9C = 0;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_354C8/func_80034E54.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_354C8/func_800355A0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_354C8/func_800359D4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_354C8/func_80035ADC.s")
/* PROVENANCE: body adapted from Jet Force Gemini's public
 * src/textures.c::func_80057B8C; Mickey's fields, calls, and compiled bytes
 * remain authoritative. The donor's empty condition is retained because it
 * advances IDO's temporary FIFO without emitting an instruction. */
void func_80035E88(TextureFrameHeader *tex, Gfx *displayList) {
    Gfx *dlist = displayList;

    if (tex) {
    }
    tex->cmd = dlist;
    func_80035F48((u8 **)&dlist, (u8 *)tex, 0, 0);
    if (tex->unk1B < 2 && (tex->flags & 0x40)) {
        if (!(tex->format & 0xF)) {
            func_80035F48((u8 **)&dlist, (u8 *)tex, 1,
                          (0x1000 - tex->textureSize) >> 3);
        } else {
            func_80035F48((u8 **)&dlist, (u8 *)tex, 1, 0x100);
        }
    }
    tex->numberOfCommands = dlist - tex->cmd;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_354C8/func_80035F48.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_354C8/func_80036544.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_354C8/func_800367A4.s")

/* JFG's texAnimateTexture body, with Mickey's four-bit flag relocation and
 * random-number entry point retained as local target-specific evidence. */
void func_800367E8(TextureFrameHeader *texture, u32 *triangleBatchInfoFlags,
                   s32 *arg2, s32 updateRate) {
    s32 breakVar;
    u16 *frameAdvanceDelay;
    u8 blink;
    s32 arg2Temp = *arg2;
    s32 flags = *triangleBatchInfoFlags;

    if (flags & (1 << 21)) {
        blink = D_8007BDA0;
        if (!(flags & (1 << 22))) {
            if (blink == 0) {
                if (func_800299E8(0, 0x3FF) > 0x3EF) {
                    flags &= ~(1 << 23);
                    flags |= (1 << 22);
                }
            } else if (blink != 2) {
                flags &= ~(1 << 23);
                flags |= (1 << 22);
            }
        } else if (!(flags & (1 << 23))) {
            arg2Temp += texture->frameAdvanceDelay * updateRate;
            if (arg2Temp >= texture->numOfTextures) {
                if (blink == 3) {
                    arg2Temp = texture->numOfTextures - 1;
                } else {
                    arg2Temp = ((texture->numOfTextures * 2) - arg2Temp) - 1;
                    if (arg2Temp < 0) {
                        arg2Temp = 0;
                        flags &= ~((1 << 23) | (1 << 22));
                    } else {
                        flags |= (1 << 23);
                    }
                }
            }
        } else {
            arg2Temp -= texture->frameAdvanceDelay * updateRate;
            if (arg2Temp < 0) {
                arg2Temp = 0;
                flags &= ~((1 << 23) | (1 << 22));
            }
        }
        D_8007BDA0 = 0;
    } else if (flags & (1 << 22)) {
        if (!(flags & (1 << 23))) {
            arg2Temp += texture->frameAdvanceDelay * updateRate;
        } else {
            frameAdvanceDelay = &texture->frameAdvanceDelay;
            arg2Temp -= (*frameAdvanceDelay) * updateRate;
        }
        do {
            breakVar = FALSE;
            if (arg2Temp < 0) {
                arg2Temp = -arg2Temp;
                flags &= ~(1 << 23);
                breakVar = TRUE;
            }
            if (arg2Temp >= texture->numOfTextures) {
                arg2Temp = ((texture->numOfTextures * 2) - arg2Temp) - 1;
                flags |= (1 << 23);
                breakVar = TRUE;
            }
        } while (breakVar);
    } else if (!(flags & (1 << 23))) {
        arg2Temp += texture->frameAdvanceDelay * updateRate;
        while (arg2Temp >= texture->numOfTextures) {
            arg2Temp -= texture->numOfTextures;
        }
    } else {
        arg2Temp -= texture->frameAdvanceDelay * updateRate;
        while (arg2Temp < 0) {
            arg2Temp += texture->numOfTextures;
        }
    }
    *arg2 = arg2Temp;
    *triangleBatchInfoFlags = flags;
}
#ifdef NON_MATCHING
#undef TEXTURE_FIELD
#endif
