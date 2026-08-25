/*
 * Resident visual effects -- ROM 0x47A70-0x4BC40 (VRAM 0x80046E70).
 *
 * PROVENANCE: the translation-unit identity and the descriptive cone/wake
 * names are adapted from Jet Force Gemini's public decompilation, src/fx.c.
 * Mickey begins at JFG's fxFreeCone portion of that TU; the matching sequence
 * of texture, allocator, trigonometry and draw calls establishes the named
 * routines below. Externally referenced functions and unresolved JFG
 * placeholders retain Mickey address names. The bodies remain Mickey's
 * extracted assembly.
 */

#include "PR/ultratypes.h"

typedef struct Wake {
    u8 pad0[0x30];
    s32 linked;
} Wake;

typedef struct FxCone {
    s32 texture;
    s32 alternateTexture;
    u8 pad8[0x24];
    s8 value2C;
    s8 value2D;
    s8 value2E;
    u8 pad2F;
    s8 value30;
    s8 value31;
    s8 value32;
} FxCone;

typedef struct FxConeTextureInfo {
    u8 pad0[6];
    u16 width;
    u16 height;
} FxConeTextureInfo;

typedef struct FxConeVertex {
    u8 pad0;
    u8 index0;
    u8 index1;
    u8 index2;
    s16 s0;
    s16 t0;
    s16 s1;
    s16 t1;
    s16 s2;
    s16 t2;
} FxConeVertex;

typedef struct FxConeCoords {
    FxConeTextureInfo *textureInfo;
    u8 pad4[0xC];
    FxConeVertex *vertices;
    u8 pad14;
    u8 segmentCount;
} FxConeCoords;

typedef struct FxGfx {
    u32 w0;
    u32 w1;
} FxGfx;

typedef struct FxDrawCone {
    s32 texture0;
    s32 texture1;
    s32 addresses[2];
    s32 vertices;
    u8 mode;
    u8 vertexCount;
    u8 addressIndex;
    u8 pad17[0x15];
    u8 primRed;
    u8 primGreen;
    u8 primBlue;
    u8 pad2F;
    u8 envRed;
    u8 envGreen;
    u8 envBlue;
} FxDrawCone;

typedef struct WakeRipple {
    u8 pad0[0x70];
    s32 linked;
    u8 pad74[0x10];
    Wake *wake;
} WakeRipple;

typedef struct FxFlags {
    u16 value;
    u8 pad2[0x1E];
} FxFlags;

typedef struct FxStatus {
    u8 value;
    u8 pad1[0x1F];
} FxStatus;

typedef struct FxScreenEffect {
    s32 type;
    s16 value4;
    s16 value6;
    s16 value8;
    s16 valueA;
    s16 valueC;
    s16 valueE;
    s32 value10;
} FxScreenEffect;

typedef struct FxSpdRecord {
    s16 value0;
    s16 value2;
    s16 value4;
    u8 value6;
    u8 value7;
} FxSpdRecord;

typedef struct FxRecord {
    u8 state;
    u8 status;
    u16 flags;
    u8 pad4[0x16];
    u8 red;
    u8 green;
    u8 blue;
    u8 pad1D[3];
} FxRecord;

typedef void (*FxTextureCallback)(s32 index, s32 value, s32 arg2);

extern void func_800347A0(s32 linked);
extern void func_800320F0(s32 callback);
extern void func_8004ACC4();
extern void mmFree(void *ptr);
extern FxFlags D_800D5F5A[];
extern FxStatus D_800D5F59[];
extern FxRecord D_800D5F58[];
extern s32 D_800D5F50;
extern s32 D_800D6038[];
extern s32 D_800D6040;
extern FxSpdRecord D_800D5FF8[][4];
extern s32 D_8007D47C[];
extern s32 D_8007D488;
extern s32 D_800D6098[];
extern s32 D_800D60A8;
extern s32 D_800D60BC;
extern s32 D_800D60C0[];
extern s32 D_800D60CC;
extern u8 D_800D60D3;
extern s32 D_8007D478;
extern FxScreenEffect D_800D6048[];
extern void TrapDanglingJump(void);
extern void fxScreenEffect(s32 arg0, s32 type, s32 value4, s32 value6,
                           s32 value8, s32 valueA, s32 valueC, s32 valueE,
                           s32 value10);
extern f32 func_8002A8BC(s16 angle);
extern f32 func_8002A8C0(s16 angle);
extern f32 D_80083DE8;
extern void func_800349A4(FxGfx **dList, s32 texture, s32 flags, s32 arg3);
extern void func_8004A10C(s32 screen, u8 glyph, s32 x, s32 y, s32 arg4);
extern s32 sprintf(char *buffer, const char *format, ...);
extern u8 D_8007D364[];
extern char D_80083DE0[];
extern s32 D_800D2FA0;

void func_80046E70(FxCone *cone) {
    s32 texture;
    s32 alternateTexture;

    texture = cone->texture;
    if (texture != 0) {
        func_800347A0(texture);
    }
    alternateTexture = cone->alternateTexture;
    if (alternateTexture != 0) {
        func_800347A0(alternateTexture);
    }
    mmFree(cone);
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80046EC4.s")
void func_8004707C(FxCone *cone, s32 value2C, s32 value2D, s32 value2E,
                   s32 value30, s32 value31, s32 value32) {
    if (cone != 0) {
        cone->value2C = value2C;
        cone->value2D = value2D;
        cone->value2E = value2E;
        cone->value30 = value30;
        cone->value31 = value31;
        cone->value32 = value32;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_800470B0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80047304.s")
#ifdef NON_MATCHING
/* Mickey-derived draft; JFG's corresponding fxMakeConeTextureCoords body is
 * also assembly-only and supplies no adaptable C source. */
void func_800475E8(FxConeCoords *cone, s16 angle) {
    s16 x[20];
    s16 y[20];
    FxConeTextureInfo *textureInfo;
    FxConeVertex *vertex;
    s16 width;
    s16 height;
    s16 currentAngle;
    u8 segmentCount;
    s32 angleStep;
    s32 i;

    currentAngle = angle;
    if (cone != 0) {
        textureInfo = cone->textureInfo;
        if (textureInfo != 0) {
            width = textureInfo->width * 16;
            height = textureInfo->height * 16;
            segmentCount = cone->segmentCount;
            vertex = cone->vertices;
            if (segmentCount == 0) {
                f32 widthEdge = (f32)(width - 1);
                f32 scale = D_80083DE8;
                f32 heightEdge = (f32)(height - 1);
                s16 *xIt = x;
                s16 *yIt = y;

                do {
                    f32 sine = func_8002A8C0(currentAngle);
                    f32 cosine = func_8002A8BC(currentAngle);

                    xIt++;
                    yIt++;
                    currentAngle += 0x2000;
                    *yIt = (s32)(scale * sine) + width;
                    *xIt = (s32)(scale * cosine) + height;
                    yIt[8] = (s32)(widthEdge * sine) + width;
                    xIt[8] = (s32)(heightEdge * cosine) + height;
                } while (xIt != &x[8]);

                i = 31;
                do {
                    vertex->s0 = y[vertex->index0];
                    vertex->t0 = x[vertex->index0];
                    vertex->s1 = y[vertex->index1];
                    vertex->t1 = x[vertex->index1];
                    vertex->s2 = y[vertex->index2];
                    vertex->t2 = x[vertex->index2];
                    vertex++;
                    i--;
                } while (i != 0);
                segmentCount = 8;
                angleStep = 0x2000;
            } else {
                angleStep = 0x10000 / segmentCount;
            }

            i = 0;
            do {
                y[i] = (s32)(func_8002A8C0(angle) * (f32)(width - 1)) +
                       width;
                x[i] = (s32)(func_8002A8BC(angle) * (f32)(height - 1)) +
                       height;
                angle += angleStep;
                i++;
            } while (i <= segmentCount);

            i = 0;
            while (i < (segmentCount & 3)) {
                vertex->s0 = y[i];
                vertex->t0 = x[i];
                vertex->s1 = y[i + 1];
                vertex->t1 = x[i + 1];
                vertex->s2 = width;
                vertex->t2 = height;
                vertex++;
                i++;
            }
            while (i < segmentCount) {
                vertex[0].s0 = y[i + 0];
                vertex[0].t0 = x[i + 0];
                vertex[0].s1 = y[i + 1];
                vertex[0].t1 = x[i + 1];
                vertex[0].s2 = width;
                vertex[0].t2 = height;
                vertex[1].s0 = y[i + 1];
                vertex[1].t0 = x[i + 1];
                vertex[1].s1 = y[i + 2];
                vertex[1].t1 = x[i + 2];
                vertex[1].s2 = width;
                vertex[1].t2 = height;
                vertex[2].s0 = y[i + 2];
                vertex[2].t0 = x[i + 2];
                vertex[2].s1 = y[i + 3];
                vertex[2].t1 = x[i + 3];
                vertex[2].s2 = width;
                vertex[2].t2 = height;
                vertex[3].s0 = y[i + 3];
                vertex[3].t0 = x[i + 3];
                vertex[3].s1 = y[i + 4];
                vertex[3].t1 = x[i + 4];
                vertex[3].s2 = width;
                vertex[3].t2 = height;
                vertex += 4;
                i += 4;
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_800475E8.s")
#endif
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_800479D4.s")

/*
 * PROVENANCE: the block-local display-list macro spelling below is adapted
 * from Jet Force Gemini include/f3ddkr.h. Mickey's own bytes establish every
 * invocation, argument, constant and operation order in func_80047CD8.
 */
#define FX_SHIFTL(value, shift, width) \
    ((u32)(((u32)(value) & ((1U << (width)) - 1U)) << (shift)))
#define FX_PIPE_SYNC(packet) { \
    FxGfx *_g = (FxGfx *)(packet); \
    _g->w0 = FX_SHIFTL(0xE7, 24, 8); \
    _g->w1 = 0; \
}
#define FX_SET_PRIM(packet, red, green, blue, alpha) { \
    FxGfx *_g = (FxGfx *)(packet); \
    _g->w0 = FX_SHIFTL(0xFA, 24, 8); \
    _g->w1 = FX_SHIFTL(red, 24, 8) | FX_SHIFTL(green, 16, 8) | \
             FX_SHIFTL(blue, 8, 8) | FX_SHIFTL(alpha, 0, 8); \
}
#define FX_SET_ENV(packet, red, green, blue, alpha) { \
    FxGfx *_g = (FxGfx *)(packet); \
    _g->w0 = FX_SHIFTL(0xFB, 24, 8); \
    _g->w1 = FX_SHIFTL(red, 24, 8) | FX_SHIFTL(green, 16, 8) | \
             FX_SHIFTL(blue, 8, 8) | FX_SHIFTL(alpha, 0, 8); \
}
#define FX_VERTEX_JFG(packet, address, count, first) { \
    FxGfx *_g = (FxGfx *)(packet); \
    _g->w0 = FX_SHIFTL(4, 24, 8) | \
             FX_SHIFTL(((count) << 3) | ((u32)(address) & 6) | (first), \
                       16, 8) | \
             FX_SHIFTL(((count) << 3) + ((count) << 1) + 8, 0, 16); \
    _g->w1 = (u32)(address); \
}
#define FX_POLYGON(packet, address, count, textured) { \
    FxGfx *_g = (FxGfx *)(packet); \
    _g->w0 = FX_SHIFTL((((count) - 1) << 4) | (textured), 16, 8) | \
             FX_SHIFTL(5, 24, 8) | FX_SHIFTL((count) * 16, 0, 16); \
    _g->w1 = (u32)(address); \
}

#ifdef NON_MATCHING
/* Mickey-derived draft; JFG's corresponding fxDrawCone body is assembly-only. */
void func_80047CD8(FxGfx **dList, FxDrawCone *cone, s32 flags, u8 alpha) {
    s32 hasTexture;

    if (cone != 0) {
        FX_PIPE_SYNC((*dList)++);
        if (flags & 0x200) {
            FX_SET_PRIM((*dList)++, cone->primRed, cone->primGreen,
                        cone->primBlue, alpha);
            FX_SET_ENV((*dList)++, cone->envRed, cone->envGreen,
                       cone->envBlue, 0);
        } else {
            FX_SET_PRIM((*dList)++, 0xFF, 0xFF, 0xFF, alpha);
            FX_SET_ENV((*dList)++, 0xFF, 0xFF, 0xFF, 0);
        }

        if (cone->texture0 != 0) {
            hasTexture = 1;
        } else {
            hasTexture = 0;
        }
        if (cone->vertexCount == 0) {
            FX_VERTEX_JFG((*dList)++,
                          cone->addresses[cone->addressIndex] + 0x80000000,
                          17, 0);
            func_800349A4(dList, cone->texture1, flags, 0);
            FX_POLYGON((*dList)++, cone->vertices + 0x80000000, 16,
                       hasTexture);
            func_800349A4(dList, cone->texture0, flags, 0);
            FX_POLYGON((*dList)++, cone->vertices + 0x80000200, 8,
                       hasTexture);
            func_800349A4(dList, cone->texture1, flags, 0);
            FX_POLYGON((*dList)++, cone->vertices + 0x80000100, 16,
                       hasTexture);
        } else {
            func_800349A4(dList, cone->texture0, flags, 0);
            FX_VERTEX_JFG((*dList)++,
                          cone->addresses[cone->addressIndex] + 0x80000000,
                          cone->mode, 0);
            FX_POLYGON((*dList)++, cone->vertices + 0x80000000,
                       cone->vertexCount, hasTexture);
        }

        FX_PIPE_SYNC((*dList)++);
        FX_SET_PRIM((*dList)++, 0xFF, 0xFF, 0xFF, 0xFF);
        FX_SET_ENV((*dList)++, 0xFF, 0xFF, 0xFF, 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80047CD8.s")
#endif
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80048080.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/wakeAllocate.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80048760.s")
void wakeFree(Wake *wake) {
    s32 linked = wake->linked;

    if (linked != 0) {
        func_800347A0(linked);
    }
    mmFree(wake);
}
void func_80048980(WakeRipple *ripple) {
    s32 linked = ripple->linked;

    if (linked != 0) {
        func_800347A0(linked);
    }
    if (ripple->wake != 0) {
        wakeFree(ripple->wake);
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/wakeUpdate.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80049000.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/wakeDraw.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80049518.s")
void fxInit(void) {
    FxRecord *record;
    s32 i;

    record = D_800D5F58;
    i = 5;
    while (i--) {
        record->state = 0;
        record->flags = 0;
        record->status = 0;
        record++;
    }
    D_800D5F50 = 0;
    func_8004ACC4(i);
}
/* Mickey-derived body; JFG's corresponding fx.c routine is assembly-only. */
void func_8004978C(s32 index, s32 mask, s32 enable) {
    s32 unused[2];
    FxRecord *record;
    s32 count = 0;
    s32 andMask;
    s32 orMask;

    if (index == -1) {
        record = D_800D5F58;
        count = 5;
    } else if (index >= 0 && index < 5) {
        record = &D_800D5F58[index];
        count = 1;
    }
    if (count != 0) {
        andMask = ~mask;
        if (enable != 0) {
            andMask = -1;
            orMask = mask;
        } else {
            orMask = 0;
        }
        while (count--) {
            record->flags = (record->flags & andMask) | orMask;
            record++;
        }
    }
}
s32 func_80049828(s32 index, s32 mask) {
    if (index >= 0 && index < 5 && (D_800D5F5A[index].value & mask) != 0) {
        return 1;
    }
    return 0;
}
s32 func_80049864(s32 index) {
    if (index >= 0 && index < 5 && D_800D5F59[index].value != 0) {
        return 1;
    }
    return 0;
}
s32 func_8004989C(s32 index) {
    FxRecord *record;
    s32 color;

    if (index < 0 || index >= 5) {
        return 0;
    }
    record = &D_800D5F58[index];
    color = ((record->red & 0xF8) << 8) |
            ((record->green & 0xF8) << 3) |
            ((record->blue & 0xF8) >> 2);
    color |= color << 16;
    return color;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_800498FC.s")
void func_80049A8C(s32 index) {
    s32 count = 0;
    FxRecord *record;

    if (index == -1) {
        count = 5;
        record = D_800D5F58;
    } else if (index >= 0 && index < 5) {
        count = 1;
        record = &D_800D5F58[index];
    }
    while (count--) {
        record->state = 0;
        record->flags &= ~5;
        record->status = 0;
        record++;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80049B14.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80049E4C.s")
void func_8004A0F0(void) {
    D_800D6038[0] = 0;
    D_800D6038[1] = 0;
    D_800D6040 = 0;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004A10C.s")
/* Plateau (batch 29): exact 0x80 frame, two instructions short; 65 words
 * differ from +0x08 after 119 flags, nine shapes, and a 40-minute permuter.
 * IDO puts text at sp+0x50 in s2; target keeps index zero and uses sp+0x54. */
#ifdef NON_MATCHING
/* PROVENANCE: role adapted from JFG src/fx.c::func_8006DF90; both bodies are
 * assembly-only, so this reconstruction is Mickey-derived. */
void func_8004A380(s32 x, s32 y, s32 value, s32 minimumWidth, s32 arg4) {
    s32 length;
    s32 index;
    char *cursor;
    u8 glyph;
    u8 character;
    char text[32];

    length = 0;
    index = 0;
    cursor = text + index;
    sprintf(text, D_80083DE0, value);
    if (text[length] != '\0') {
        do {
            length++;
        } while (text[length] != '\0');
    }
    if (minimumWidth >= length) {
        do {
            glyph = D_8007D364[11];
            if (length < minimumWidth) {
                length++;
            } else {
                character = *cursor++;
                if (character == '-') {
                    glyph = D_8007D364[10];
                } else if (character >= '0' && character < ':') {
                    glyph = D_8007D364[character - '0'];
                }
            }
            func_8004A10C(D_800D2FA0, glyph, x, y, arg4);
            x += 10;
        } while (*cursor != '\0');
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004A380.s")
#endif
/* Mickey-derived body; JFG's corresponding fx.c function is assembly-only. */
void func_8004A4B0(s32 value0, s32 value2, s32 value4, s32 value6,
                   s32 value7) {
    s32 group;
    s32 *countPtr;
    FxSpdRecord *record;

    group = D_800D6040;
    countPtr = &D_800D6038[group];
    if (*countPtr < 4) {
        record = &D_800D5FF8[group][(*countPtr)++];
        record->value0 = value0;
        record->value2 = value2;
        record->value4 = value4;
        record->value6 = value6;
        record->value7 = value7;
    }
}
/* Mickey-derived body; JFG's corresponding fx.c function is assembly-only. */
void func_8004A51C(void) {
    s32 group;
    s32 count;
    FxSpdRecord *record;

    group = D_800D6040;
    count = D_800D6038[group];
    record = D_800D5FF8[group];
    D_800D6040 = group ^ 1;
    D_800D6038[D_800D6040] = 0;
    while (count--) {
        func_8004A380(record->value0, record->value2, record->value4,
                      record->value6, record->value7);
        record++;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/fxSPDPRipple.s")
void fxQueueScreenEffect(s32 type, s32 value4, s32 value6, s32 value8,
                         s32 valueA, s32 valueC, s32 valueE, s32 value10) {
    FxScreenEffect *effect;

    if (D_8007D478 < 4) {
        effect = &D_800D6048[D_8007D478++];
        effect->type = type;
        effect->value4 = value4;
        effect->value6 = value6;
        effect->value8 = value8;
        effect->valueA = valueA;
        effect->valueC = valueC;
        effect->valueE = valueE;
        effect->value10 = value10;
    }
}
void func_8004A9CC(s32 arg0) {
    FxScreenEffect *effect;
    s32 index;

    effect = D_800D6048;
    index = 0;
    if (D_8007D478 > 0) {
        do {
            fxScreenEffect(arg0, effect->type, effect->value4,
                           effect->value6, effect->value8, effect->valueA,
                           effect->valueC, effect->valueE, effect->value10);
            index++;
            effect++;
        } while (index < D_8007D478);
    }
    D_8007D478 = 0;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/fxScreenEffect.s")
#ifdef NON_MATCHING
/*
 * PROVENANCE: the descending four-slot loop skeleton is adapted from Jet
 * Force Gemini asm/nonmatchings/fx/func_8006FFF8.s. Mickey's own symbols and
 * instruction schedule establish the assignment order below.
 */
void func_8004ACC4(void) {
    s32 *callback;
    s32 *value0;
    s32 *value1;
    u8 *available;
    s32 i;
    s32 trap;
    s32 trapValue;

    D_800D60A8 = 0;
    i = 3;
    trapValue = (s32) TrapDanglingJump;
    value0 = &D_800D60BC;
    value1 = &D_800D60CC;
    available = &D_800D60D3;
    trap = trapValue;
    callback = &D_8007D488;
    do {
        *value0 = 0;
        *value1 = 0;
        *available = trap == *callback;
        value0--;
        value1--;
        available--;
        callback--;
    } while (i--);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004ACC4.s")
#endif
s32 func_8004AD34(void) {
    FxTextureCallback callback;
    s32 index;

    index = 4;
    while (index--) {
        if ((1 << index) & D_800D60A8) {
            func_800320F0((s32)D_8007D47C + (index << 2));
            callback = (FxTextureCallback)D_8007D47C[index];
            if (callback != 0) {
                callback(index, D_800D6098[index], 0);
            }
        }
    }
    D_800D60A8 = 0;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004ADE8.s")
/* Workbench: structure-mismatch, 54/52 words, 48 positional differences from +0x04.
 * Tried constant audit, context lint, pool-vs-temp inlining, and pointer-lifetime placement.
 * The D_800D60C0 base remains a saved web, adding s7 and two boundary words. */
#ifdef NON_MATCHING
/* Mickey-derived body; JFG's fxCpuTextureFlush is assembly-only. */
void func_8004AF68(void) {
    register s32 offset;
    register s32 *value0;
    register s32 i;
    register u8 *available;
    s32 *value1;
    void *allocation;

    offset = 12;
    value0 = &D_800D60BC;
    i = 3;
    available = &D_800D60D3;
    do {
        allocation = (void *)*value0;
        if (allocation != 0) {
            value1 = (s32 *)(offset + (s32)D_800D60C0);
            mmFree(allocation);
            mmFree((void *)*value1);
            *value0 = 0;
            *value1 = 0;
        }
        if (*available != 0) {
            *(s32 *)((u8 *)D_8007D47C + offset) = (s32)TrapDanglingJump;
        }
        value0--;
        available--;
        offset -= 4;
    } while (i--);
    D_800D60A8 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004AF68.s")
#endif
