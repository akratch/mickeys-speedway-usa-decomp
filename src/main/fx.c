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

#include "game/fx.h"

void func_80046E70(FxCone *cone) {
    FxConeTextureInfo *texture;
    FxConeTextureInfo *alternateTexture;

    texture = cone->texture.pointer;
    if (texture != 0) {
        func_800347A0(texture);
    }
    alternateTexture = cone->alternateTexture.pointer;
    if (alternateTexture != 0) {
        func_800347A0(alternateTexture);
    }
    mmFree(cone);
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80046EC4.s")
void func_8004707C(FxCone *cone, s32 value2C, s32 value2D, s32 value2E,
                   s32 value30, s32 value31, s32 value32) {
    if (cone != 0) {
        cone->primRed = value2C;
        cone->primGreen = value2D;
        cone->primBlue = value2E;
        cone->envRed = value30;
        cone->envGreen = value31;
        cone->envBlue = value32;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_800470B0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80047304.s")
/* Workbench p5: mixed structure/register mismatch; 247/251 candidate/target instructions, 178 words from +0x0.
 * Lever: constant-audit and array declaration/loop spelling; loopunroll=0 won the flag sweep, while pointer and width/lifetime variants regressed.
 * Remains: candidate frame is 8 bytes larger and four instructions shorter, with a register/CFG cascade. */
#ifdef NON_MATCHING
/* Mickey-derived draft; JFG's corresponding fxMakeConeTextureCoords body is
 * also assembly-only and supplies no adaptable C source. */
void func_800475E8(FxCone *cone, s16 angle) {
    FxConeTextureInfo *textureInfo;
    FxConeVertex *vertex;
    s32 width;
    s32 height;
    s32 currentAngle;
    s32 angleStep;
    s32 segmentCount;
    s32 i;
    s16 y[20];
    s16 x[20];

    currentAngle = angle;
    if (cone != 0) {
        textureInfo = cone->texture.pointer;
        if (textureInfo != 0) {
            width = textureInfo->width * 16;
            height = textureInfo->height * 16;
            vertex = (FxConeVertex *) cone->vertices;
            if (cone->segmentCount == 0) {
                f32 widthEdge = (f32)(width - 1);
                f32 scale = D_80083DE8;
                f32 heightEdge = (f32)(height - 1);
                i = 0;
                do {
                    f32 sine = func_8002A8C0(currentAngle);
                    f32 cosine = func_8002A8BC(currentAngle);

                    currentAngle += 0x2000;
                    y[i + 1] = (s32)(scale * sine) + width;
                    x[i + 1] = (s32)(scale * cosine) + height;
                    y[i + 9] = (s32)(widthEdge * sine) + width;
                    x[i + 9] = (s32)(heightEdge * cosine) + height;
                    i++;
                } while (i != 8);

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
                segmentCount = cone->segmentCount;
                angleStep = 0x10000 / segmentCount;
            }

            {
                s16 *yIt = y;
                s16 *xIt = x;
                s16 *xEnd = &x[segmentCount + 1];

                if (segmentCount >= 0) {
                    do {
                        *yIt = (s32)(func_8002A8C0(angle) *
                                     (f32)(width - 1)) + width;
                        *xIt = (s32)(func_8002A8BC(angle) *
                                     (f32)(height - 1)) + height;
                        angle += angleStep;
                        xIt++;
                        yIt++;
                    } while (xIt != xEnd);
                }
            }

            i = 0;
            if (segmentCount > 0) {
                while (i != (segmentCount & 3)) {
                    vertex->s0 = y[i];
                    vertex->t0 = x[i];
                    vertex->s1 = y[i + 1];
                    vertex->t1 = x[i + 1];
                    vertex->s2 = width;
                    vertex->t2 = height;
                    vertex++;
                    i++;
                }
                while (i != segmentCount) {
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
/* Workbench diagnostic full-TU copy: allocation-mismatch, 226/234 rows exact,
 * exact 234/-104 shape, eight register words from +0x298, and zero relocation
 * differences. The temp lane is identical; the pool first diverges at slot 28
 * as v1->a0, a0->a1, and a2->v0. There is no move/copy site for a source
 * coalescing lever, and no instrumented IDO is available for the forced-color
 * oracle. The configured TU still fails before this function on the adjacent
 * func_800475E8 block-local C99 declarations; that function is outside this
 * target's ownership. Remains: one callee-saved pool-color cascade; assembly
 * fallback stays canonical. */
/* Mickey-derived draft; JFG's corresponding fxDrawCone body is assembly-only. */
void func_80047CD8(FxGfx **dList, FxCone *cone, s32 flags, u8 alpha) {
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

        if (cone->texture.value != 0) {
            hasTexture = 1;
        } else {
            hasTexture = 0;
        }
        if (cone->segmentCount == 0) {
            FX_VERTEX_JFG((*dList)++,
                          cone->addresses[cone->addressIndex] + 0x80000000,
                          17, 0);
            func_800349A4(dList, cone->alternateTexture.value, flags, 0);
            FX_POLYGON((*dList)++, cone->vertices + 0x80000000, 16,
                       hasTexture);
            func_800349A4(dList, cone->texture.value, flags, 0);
            FX_POLYGON((*dList)++, cone->vertices + 0x80000200, 8,
                       hasTexture);
            func_800349A4(dList, cone->alternateTexture.value, flags, 0);
            FX_POLYGON((*dList)++, cone->vertices + 0x80000100, 16,
                       hasTexture);
        } else {
            func_800349A4(dList, cone->texture.value, flags, 0);
            FX_VERTEX_JFG((*dList)++,
                          cone->addresses[cone->addressIndex] + 0x80000000,
                          cone->mode, 0);
            FX_POLYGON((*dList)++, cone->vertices + 0x80000000,
                       cone->segmentCount, hasTexture);
        }

        FX_PIPE_SYNC((*dList)++);
        FX_SET_PRIM((*dList)++, 0xFF, 0xFF, 0xFF, 0xFF);
        FX_SET_ENV((*dList)++, 0xFF, 0xFF, 0xFF, 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80047CD8.s")
#endif
/* Workbench: structure-mismatch, 87 differing words, first mismatch +0x0. */
/* Candidate shape: 82 instructions/frame -0x40 vs target 89/-0x48; not permuter-ready. */
/* Remaining structural gap: IDO's count/pointer loop and stack-home shape. */
#ifdef NON_MATCHING
typedef struct FxTransformInput {
    f32 x;
    f32 y;
    f32 z;
} FxTransformInput;

typedef struct FxTransformOutput {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    s8 alpha;
} FxTransformOutput;

void func_80048080(s32 count, s16 x, s16 y, s16 z, s16 angle0, s16 angle1,
                   FxTransformInput *input, FxTransformOutput *output,
                   s32 alpha) {
    f32 cos1;
    f32 sin1;
    f32 cos0;
    f32 sin0;
    f32 inputX;
    f32 inputY;
    f32 inputZ;
    f32 cross;
    register s32 var_s0;
    f32 *var_v1;
    u8 *var_v0;

    cos1 = func_8002A8C0(angle1);
    sin1 = func_8002A8BC(angle1);
    cos0 = func_8002A8C0(angle0);
    sin0 = func_8002A8BC(angle0);
    var_s0 = count - 1;
    if (count == 0) {
        goto done;
    }
    var_v1 = input;
    var_v0 = output;
loop:
    inputZ = var_v1[2];
    inputY = var_v1[1];
    inputX = var_v1[0];
    var_v1 += 3;
    var_v0[6] = 0xFF;
    var_v0[7] = 0xFF;
    var_v0[8] = 0xFF;
    var_v0[9] = alpha;
    var_v0 += 10;
    cross = (inputZ * sin1) + (inputY * cos1);
    ((s16 *)var_v0)[-5] =
        (s16)((s32)((inputX * sin0) + (cross * cos0)) + x);
    ((s16 *)var_v0)[-4] =
        (s16)((s32)((inputY * sin1) - (inputZ * cos1)) + y);
    ((s16 *)var_v0)[-3] =
        (s16)((s32)((cross * sin0) - (inputX * cos0)) + z);
    var_s0--;
    if (var_s0 != 0) {
        goto loop;
    }
    input = var_v1;
    output = var_v0;
done:
    ;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80048080.s")
#endif
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/wakeAllocate.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80048760.s")
void wakeFree(Wake *wake) {
    void *linked = wake->linked;

    if (linked != 0) {
        func_800347A0(linked);
    }
    mmFree(wake);
}
void func_80048980(WakeRipple *ripple) {
    void *linked = ripple->linked;

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
    func_8004ACC4();
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
/* Workbench: structure-mismatch, 42 differing words, first mismatch +0x0. */
/* Candidate shape: 100 instructions/frame -0x30/relocations match; opcode schedule is not shape-exact. */
/* Remaining structural gap: camera-join and FxRecord flag/field scheduling. */
#ifdef NON_MATCHING
extern s32 camGetMode(void);
extern void func_80021FB0(s32 mode, s32 camNo, s32 *x1, s32 *y1,
                          u32 *x2, u32 *y2);

void func_800498FC(s32 index, f32 value16, f32 value18, s32 red, s32 green,
                   s32 blue, s32 flags) {
    FxRecord *record;
    u8 flag80;
    u8 flag40;

    if (index < 0 || index >= 5) {
        return;
    }
    record = &D_800D5F58[index];
    if ((record->flags & 2) != 0 ||
        (record->state != 0 && (record->flags & 1) != 0)) {
        return;
    }
    if (index == 4) {
        func_80021FB0(0, 0, &record->value4, &record->value8,
                      (u32 *)&record->valueC, (u32 *)&record->value10);
    } else {
        func_80021FB0(camGetMode(), index, &record->value4, &record->value8,
                      (u32 *)&record->valueC, (u32 *)&record->value10);
    }
    record->flags = 0;
    record->value14 = 0;
    flag80 = flags & 0x80;
    record->value16 = (s16)(value16 * 60.0f);
    flag40 = flags & 0x40;
    record->value18 = (s16)(value18 * 60.0f);
    record->red = red;
    record->green = green;
    record->value1D = flags & 0xFF3F;
    record->value1E = flag80;
    record->value1F = flag40;
    record->blue = blue;
    if (flag80 != 0) {
        record->state = flag40 != 0 ? 3 : 2;
        record->status = 0xFF;
    } else {
        record->state = 1;
        record->status = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_800498FC.s")
#endif
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
/* Plateau: workbench mixed constant/structure/register, stock -O2 is 74/76
 * instructions and 65 words from +0x8; buffer size and declaration order did not move the sp+0x50 text home.
 * Remaining: target's sp+0x54 cursor base and zero-index/glyph register web; prior flag and bounded-permuter passes found no exact. */
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
void func_8004A9CC(FxGfx **dList) {
    FxScreenEffect *effect;
    s32 index;

    effect = D_800D6048;
    index = 0;
    if (D_8007D478 > 0) {
        do {
            fxScreenEffect(dList, effect->type, effect->value4,
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
/* Plateau (near-miss p6): workbench mixed(structural:10, register:5), 14 words at 28 instructions; first +0x10.
 * Lever: structure-bucket/context audit found no new source-stable schedule beyond the closed typing/order probes.
 * Remains: callback/trap pool mapping and loop-counter schedule; assembly fallback stays canonical. */
void func_8004ACC4(void) {
    FxTextureCallback *callback;
    void **value0;
    void **value1;
    u8 *available;
    s32 i;
    FxTextureCallback trap;

    D_800D60A8 = 0;
    i = 3;
    trap = (FxTextureCallback)TrapDanglingJump;
    value0 = &D_800D60BC;
    value1 = &D_800D60CC;
    available = &D_800D60D3;
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
/* Workbench: structure-mismatch, 30 differing words, first mismatch +0x0. */
/* Candidate shape: 96 instructions/relocations match; frame -0x48 vs target -0x40, not shape-exact. */
/* Remaining structural gap: the allocator/clear-loop web and the 8-byte frame home. */
#ifdef NON_MATCHING
extern void *func_8002B280(s32 size, s32 tag);

void func_8004ADE8(s32 index, FxConeTextureInfo *texture) {
    s32 offset;
    register s32 i;
    u8 *first;
    u8 *second;
    FxTextureCallback callback;

    index--;
    offset = index * 4;
    D_800D60A8 |= 1 << index;
    D_800D6098[index] = (s32)texture;
    if (D_800D60B0[index] == 0) {
        first = func_8002B280(texture->width * texture->height, 0x87);
        D_800D60B0[index] = first;
        second = func_8002B280(texture->width * texture->height, 0x87);
        D_800D60C0[index] = second;
        if (D_800D60B0[index] == 0 || second == 0) {
            D_800D60B0[index] = 0;
            D_800D60C0[index] = 0;
            return;
        }
        i = (texture->width * texture->height) - 1;
        if ((texture->width * texture->height) != 0) {
            do {
                *first = 0;
                first++;
                *second = 0;
                second++;
            } while (i-- != 0);
        }
        func_800320F0((s32)&D_8007D47C[index]);
        callback = D_8007D47C[index];
        if (callback != 0) {
            callback(index, D_800D6098[index], 1);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004ADE8.s")
#endif
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
    value0 = (s32 *)&D_800D60BC;
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
            *(FxTextureCallback *)((u8 *)D_8007D47C + offset) =
                (FxTextureCallback)TrapDanglingJump;
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
