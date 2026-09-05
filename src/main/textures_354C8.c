#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

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
    u8 spriteFlags;
    s16 flags;
    u16 width;
    u16 height;
    u8 pad0A;
    u8 posX;
    u8 pad0C;
    u8 posY;
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

void func_80034920(Gfx **dlist) {
    D_8007BD90 = 0;
    D_800D3024 = 0;
    D_800D3028 = 0;
    D_800D3020 = 0;
    D_800D302C = 0;
    D_800D3030 = 1;
    D_800D3034 = 1;
    if (dlist != NULL) {
        gDPPipeSync((*dlist)++);
        gSPSetGeometryMode((*dlist)++, G_FOG | G_SHADING_SMOOTH | G_SHADE | G_ZBUFFER);
    }
    D_8007BD8C = 0;
}

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
typedef struct Sprite {
    s16 baseTextureId;
    s16 numberOfFrames;
    s16 numberOfInstances;
    s16 drawFlags;
    u8 pad08[8];
    TextureFrameHeader **textures;
    u8 *commandOffsets;
} Sprite;

typedef struct SpriteAsset {
    s16 baseTextureId;
    s16 numberOfFrames;
    s16 anchorX;
    s16 anchorY;
    u8 pad08[0xC];
    u8 frameTexOffsets[1];
} SpriteAsset;

typedef struct SpriteVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} SpriteVertex;

typedef struct SpriteTriangle {
    u8 flags;
    u8 vi0;
    u8 vi1;
    u8 vi2;
    s16 uv0U;
    s16 uv0V;
    s16 uv1U;
    s16 uv1V;
    s16 uv2U;
    s16 uv2V;
} SpriteTriangle;

extern s32 D_800D3008;
extern s32 *D_800D2FFC;
extern void func_800347A0(TextureFrameHeader *texture);

void func_800359D4(Sprite *sprite) {
    s32 i;
    s32 frame;

    if (sprite != NULL) {
        sprite->numberOfInstances--;
        if (sprite->numberOfInstances <= 0) {
            for (i = 0; i < D_800D3008; i++) {
                if (sprite == (Sprite *)D_800D2FFC[(i << 1) + 1]) {
                    for (frame = 0; frame < sprite->numberOfFrames; frame++) {
                        func_800347A0(sprite->textures[frame]);
                    }
                    mmFree(sprite);
                    D_800D2FFC[i << 1] = -1;
                    D_800D2FFC[(i << 1) + 1] = -1;
                    break;
                }
            }
        }
    }
}
extern SpriteVertex *D_800D3010;
extern Gfx *D_800D3014;
extern SpriteTriangle *D_800D3018;

#define SPRITE_PHYSICAL(address) ((u32)((u8 *)(address) - 0x80000000))
#define SPRITE_DMA(packet, address, count)                                  \
    {                                                                       \
        Gfx *_g = (Gfx *)(packet);                                          \
        _g->words.w0 = (0x07000000 | (((count) & 0xFF) << 16) |             \
                        (((count) << 3) & 0xFFFF));                          \
        _g->words.w1 = (u32)(address);                                      \
    }
#define SPRITE_VERTEX(packet, address, count)                               \
    {                                                                       \
        Gfx *_g = (Gfx *)(packet);                                          \
        _g->words.w0 = (0x04000000 |                                        \
                        (((((count) << 3) | ((u32)(address) & 6)) & 0xFF)   \
                         << 16) |                                           \
                        (((((count) << 3) + ((count) << 1) + 8) | 0x200)   \
                         & 0xFFFF));                                        \
        _g->words.w1 = (u32)(address);                                      \
    }
#define SPRITE_POLYGON(packet, address)                                     \
    {                                                                       \
        Gfx *_g = (Gfx *)(packet);                                          \
        _g->words.w0 = 0x05110020;                                          \
        _g->words.w1 = (u32)(address);                                      \
    }

/* PROVENANCE: source shape adapted from Diddy Kong Racing's public
 * src/textures_sprites.c::sprite_init_frame and cross-checked against Jet
 * Force Gemini's public func_800577D8 object. Mickey's fields, globals, and
 * compiler output remain authoritative. */
void func_80035ADC(SpriteAsset *spriteAsset, Sprite *sprite, s32 frameId) {
    s32 pad[2];
    s32 anchorX;
    s32 anchorY;
    s32 tileEnd;
    s32 tileOffsetX;
    s32 tileOffsetY;
    s32 left;
    s32 numQuads;
    s32 curVertIndex;
    s32 texWidth;
    s32 texHeight;
    s32 tileIndex;
    SpriteVertex *vertex;
    SpriteVertex *curVerts;
    SpriteTriangle *triangle;
    Gfx *dlptr;
    Gfx *batchStart;
    TextureFrameHeader *tex;
    s32 commandOffsetCount;

    anchorX = spriteAsset->anchorX;
    anchorY = spriteAsset->anchorY;
    dlptr = D_800D3014;
    vertex = D_800D3010;
    triangle = D_800D3018;
    tileIndex = spriteAsset->frameTexOffsets[frameId];
    tileEnd = spriteAsset->frameTexOffsets[frameId + 1];

    if (frameId == 0 || tileIndex < tileEnd) {
        tex = sprite->textures[tileIndex];
        ((u8 *)sprite)[1] = tex->spriteFlags & 2;
        sprite->drawFlags = tex->flags & 0xC07B;
    }

    curVertIndex = 0;
    numQuads = 0;
    batchStart = dlptr;
    commandOffsetCount = 0;
    while (tileIndex < tileEnd) {
        curVerts = vertex;
        tex = sprite->textures[tileIndex];
        vertex += 4;
        texWidth = tex->width;
        texHeight = tex->height;
        tileOffsetX = tex->posX - anchorX;
        tileOffsetY = anchorY - tex->posY;

        vertex[-4].x = tileOffsetX;
        vertex[-4].y = tileOffsetY - 1;
        vertex[-4].z = 0;
        vertex[-4].r = 255;
        vertex[-4].g = 255;
        vertex[-4].b = 255;
        vertex[-4].a = 255;
        vertex[-3].x = tileOffsetX + texWidth - 1;
        vertex[-3].y = tileOffsetY - 1;
        vertex[-3].z = 0;
        vertex[-3].r = 255;
        vertex[-3].g = 255;
        vertex[-3].b = 255;
        vertex[-3].a = 255;
        vertex[-2].x = tileOffsetX + texWidth - 1;
        vertex[-2].y = tileOffsetY - texHeight;
        vertex[-2].z = 0;
        vertex[-2].r = 255;
        vertex[-2].g = 255;
        vertex[-2].b = 255;
        vertex[-2].a = 255;
        vertex[-1].x = tileOffsetX;
        vertex[-1].y = tileOffsetY - texHeight;
        vertex[-1].z = 0;
        vertex[-1].r = 255;
        vertex[-1].g = 255;
        vertex[-1].b = 255;
        vertex[-1].a = 255;

        if (sprite->drawFlags & 0x40) {
            if (batchStart != dlptr) {
                sprite->commandOffsets[commandOffsetCount++] =
                    (dlptr - batchStart);
                batchStart = dlptr;
            }
        } else {
            SPRITE_DMA(dlptr++, SPRITE_PHYSICAL(tex->cmd),
                       tex->numberOfCommands);
        }

        if (numQuads == 0) {
            left = tileEnd - tileIndex;
            if (left > 5) {
                left = 5;
            }
            SPRITE_VERTEX(dlptr++, SPRITE_PHYSICAL(curVerts), left * 4);
        }

        SPRITE_POLYGON(dlptr++, SPRITE_PHYSICAL(triangle));
        triangle[0].flags = 0x40;
        triangle[0].vi0 = curVertIndex + 3;
        triangle[0].vi1 = curVertIndex + 2;
        triangle[0].vi2 = curVertIndex + 1;
        triangle[0].uv0U = (texWidth - 1) << 5;
        triangle[0].uv0V = (texHeight - 1) << 5;
        triangle[0].uv1U = (texWidth - 1) << 5;
        triangle[0].uv1V = 0;
        triangle[0].uv2U = 1;
        triangle[0].uv2V = 0;
        triangle[1].flags = 0x40;
        triangle[1].vi0 = curVertIndex + 4;
        triangle[1].vi1 = curVertIndex + 3;
        triangle[1].vi2 = curVertIndex + 1;
        /* Inert IDO allocation cue found by the bounded permuter. */
        triangle++;
        triangle--;
        triangle[1].uv0U = 1;
        triangle[1].uv0V = (texHeight - 1) << 5;
        triangle[1].uv1U = (texWidth - 1) << 5;
        triangle[1].uv1V = (texHeight - 1) << 5;
        triangle[1].uv2U = 1;
        triangle[1].uv2V = 0;
        triangle += 2;

        curVertIndex += 4;
        numQuads++;
        tileIndex++;
        if (numQuads >= 5) {
            numQuads = 0;
            curVertIndex = 0;
        }
    }

    if (sprite->drawFlags & 0x40) {
        sprite->commandOffsets[commandOffsetCount++] = dlptr - batchStart;
        sprite->commandOffsets[commandOffsetCount] = 0;
    }
    gDPPipeSync(dlptr++);
    gSPEndDisplayList(dlptr++);
    D_800D3014 = dlptr;
    D_800D3010 = vertex;
    D_800D3018 = triangle;
}
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
