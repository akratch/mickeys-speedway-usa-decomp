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

typedef struct FxConePoint {
    f32 x;
    f32 y;
    f32 z;
} FxConePoint;

typedef FxCone FxConeCoords;

typedef struct FxWakeRippleData {
    u8 pad0[0x70];
    void *texture;
    u8 mode;
    u8 active;
    s16 fade;
    s16 angle;
    s16 angleStep;
    f32 value7C;
    f32 value80;
    void *update;
} FxWakeRippleData;

typedef struct FxWakeTexture {
    u8 pad0[0x10];
    u16 length;
} FxWakeTexture;

typedef struct FxWakeUpdateOwner {
    u8 pad0[0x0C];
    f32 valueC;
    u8 pad10[4];
    f32 value14;
    u8 pad18[4];
    f32 value1C;
    u8 pad20[4];
    f32 value24;
    u8 pad28[0x2C];
    FxWakeRippleData *ripple;
} FxWakeUpdateOwner;

typedef struct FxWakeSegment {
    s32 x;
    s32 y;
    s32 z;
    u8 padC[2];
    s16 length;
} FxWakeSegment;

extern void func_80048080(s32 count, s16 arg1, s16 arg2, s16 arg3,
                          s32 arg4, s32 arg5, FxConePoint *points,
                          void *vertices, s32 alpha);
extern void viGetCurrentSize(s32 *width, s32 *height);
extern s16 Arctanf(f32 x, f32 y);
extern void wakeUpdate(s32 update, f32 x, f32 height, f32 z, s32 angle,
                       s32 delta);
extern f32 D_80083DE4;
extern void mathOneFloatPY(void *source, f32 *result, s16 angle);
extern void camSetScissor(FxGfx **dlist);
extern void func_80034920(FxGfx **dlist, void *table, FxGfx **arg2);
extern void *func_8002B314(s32 size, s32 tag);

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
/* Workbench: structure-mismatch, 61 differing words, first mismatch +0x2c. */
/* Candidate shape: 111/110 instructions, frame -0x48/-0x48; one address-base instruction remains. */
/* Remaining gap: target preserves the stored cone-end base for address construction; registers remain. */
#ifdef NON_MATCHING
extern void *func_8002B280(s32 size, s32 tag);
extern void *func_80034448(s32 resourceId);
extern void func_800470B0(FxCone *, s16, s16, s16, s16, s16,
                          f32, f32, f32);
extern void func_80047304(FxCone *, s16, s16, s16, s16, s16,
                          f32, f32, f32);
extern void func_800475E8(FxCone *, s16);

void *func_80046EC4(s16 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4,
                    f32 arg5, f32 arg6, f32 arg7, s32 arg8, s32 arg9,
                    s32 argA) {
    s32 sp44;
    s32 sp40;
    s32 sp38;
    s32 temp_a0;
    FxCone *cone;
    u8 *temp_v1;

    sp38 = arg8 & 0x80;
    arg8 = arg8 & 0x7F;
    if (arg8 == 0) {
        sp44 = 0x280;
        sp40 = 0x154;
    } else {
        sp44 = arg8 * 0x10;
        sp40 = (arg8 * 0xA) + 0xA;
    }
    cone = (FxCone *) func_8002B280(sp44 + (sp40 * 2) + 0x38, 0x87);
    if (cone != NULL) {
        if (arg9 >= 0) {
            cone->texture.value = (s32) func_80034448(arg9);
        } else {
            cone->texture.value = 0;
        }
        if (argA >= 0) {
            cone->alternateTexture.value = (s32) func_80034448(argA);
        } else {
            cone->alternateTexture.value = 0;
        }
        cone->vertices = (u8 *) ((s32) cone + 0x38);
        temp_a0 = arg8 + 1;
        temp_v1 = cone->vertices;
        temp_v1 += sp44;
        cone->addresses[0] = (u8 *) temp_v1;
        temp_v1 += sp40;
        cone->addresses[1] = (u8 *) temp_v1;
        cone->mode = temp_a0;
        cone->segmentCount = arg8;
        cone->addressIndex = 0;
        cone->flags = sp38;
        cone->value22 = arg4;
        cone->value20 = arg3;
        cone->value24 = (s16) (s32) arg7;
        cone->value18 = arg5;
        cone->value1C = arg6;
        cone->value2A = arg2;
        cone->value28 = arg1;
        cone->value26 = arg0;
        if (temp_a0 == 1) {
            func_80047304(cone, arg0, arg1, arg2, (s32) arg3,
                          (s32) arg4, arg5, arg6, arg7);
        } else {
            func_800470B0(cone, arg0, arg1, arg2, (s32) arg3,
                          (s32) arg4, arg5, arg6, arg7);
        }
        func_800475E8(cone, 0);
    }
    return cone;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80046EC4.s")
#endif
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
/* Workbench verdict: structure-mismatch, 121 differing words, first mismatch +0x44. */
/* Candidate: 150/149 instructions with the target -0x168 frame; relocation and CFG residuals remain, so it is not shape-exact. */
/* Shape status: one-word length delta; the helper loop and signed angle path are preserved for the permuter-ready pass. */
/* PROVENANCE: JFG's public src/fx.c establishes the corresponding cone routine and call roles; this body is reconstructed from Mickey's own m2c draft and typed layouts. */
#ifdef NON_MATCHING
void func_800470B0(FxCone *cone, s16 arg1, s16 arg2, s16 arg3, s16 arg4,
                   s16 arg5, f32 arg6, f32 arg7, f32 arg8) {
    FxConePoint points[17];
    FxConePoint *point;
    u8 *address;
    u8 *vertex;
    s32 angleStep;
    f32 var_f0;
    f32 var_f24;
    f32 temp_f6;
    s32 i;
    s32 j;

    if (cone->flags != 0) {
        angleStep = -0x10000 / (s32) cone->segmentCount;
        var_f0 = 0.0f;
        var_f24 = -arg8;
    } else {
        angleStep = 0x10000 / (s32) cone->segmentCount;
        var_f24 = 0.0f;
        var_f0 = -arg8;
    }
    points[0].x = 0.0f;
    points[0].y = 0.0f;
    points[0].z = var_f0;
    point = points + 1;
    i = 0;
    j = cone->segmentCount - 1;
    if (cone->segmentCount != 0) {
        do {
            point->x = (f32) (func_8002A8C0(i) * arg6);
            temp_f6 = func_8002A8BC(i) * arg7;
            point->z = var_f24;
            point++;
            i += angleStep;
            point[-1].y = temp_f6;
            j--;
        } while (j != 0);
    }
    address = (u8 *) cone;
    i = 0;
    do {
        func_80048080(cone->mode, arg1, arg2, arg3, (s32) arg4,
                      (s32) arg5, points, *(void **)(address + 8), 0xFF);
        i += 4;
        address += 4;
    } while (i < 8);
    vertex = cone->vertices;
    {
        s8 index;
        s8 next;

        index = 1;
        if ((s32) cone->segmentCount > 0) {
            do {
                next = index + 1;
                vertex[0] = 0;
                vertex[1] = index;
                vertex[2] = next;
                vertex[3] = 0;
                index = next;
                vertex += 0x10;
            } while ((s32) cone->segmentCount >= next);
        }
    }
    vertex[-0xE] = 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_800470B0.s")
#endif

#ifdef NON_MATCHING
/* Workbench verdict: structure-mismatch, 176 differing words, first mismatch +0x4. */
/* Candidate: 183/185 instructions with the target -0x180 frame; 57 structural words remain, so it is not shape-exact. */
/* Shape status: extended cone-point and three vertex-table loops are preserved; structural gap remains in the setup/call schedule. */
void func_80047304(FxCone *cone, s16 arg1, s16 arg2, s16 arg3, s16 arg4,
                   s16 arg5, f32 arg6, f32 arg7, f32 arg8) {
    u8 work[0x98];
    u8 *point;
    u8 *vertex;
    FxCone *address;
    FxCone *base;
    f32 angle;
    f32 scaleZ;
    f32 scaleX;
    f32 scaleY;
    f32 sine;
    f32 cosine;
    f32 yScale;
    s32 i;
    s32 j;
    s32 value;

    angle = arg8;
    *(f32 *) (work + 8) = -angle;
    point = work + 0xC;
    i = 0;
    scaleZ = -(angle * D_80083DE4);
    *(f32 *) work = 0.0f;
    *(f32 *) (work + 4) = 0.0f;
    do {
        value = i << 0xD;
        sine = func_8002A8C0(value);
        cosine = func_8002A8BC(value);
        i += 1;
        point += 0xC;
        *(f32 *) (point - 0xC) = arg6 * sine;
        *(f32 *) (point - 4) = 0.0f;
        *(f32 *) (point + 0x5C) = scaleZ;
        yScale = arg7 * 4.0f * cosine;
        *(f32 *) (point - 8) = arg7 * cosine;
        scaleX = arg6 * 4.0f * sine;
        *(f32 *) (point + 0x54) = 2.0f * scaleX;
        *(f32 *) (point + 0x58) = 2.0f * yScale;
    } while (i < 8);

    base = cone;
    address = cone;
    j = 0;
    point = work;
    do {
        func_80048080(0x11, arg1, arg2, arg3, (s32) arg4, (s32) arg5,
                      (FxConePoint *) point, *(void **) ((u8 *) address + 8),
                      0xFF);
        j += 4;
        address = (FxCone *) ((u8 *) address + 4);
    } while (j < 8);

    vertex = base->vertices;
    i = 1;
    do {
        s32 index;
        s32 next;

        index = i & 7;
        next = i + 8;
        vertex[1] = (u8) i;
        vertex[0x11] = (u8) i;
        i += 1;
        vertex[0] = 0;
        vertex[2] = (u8) next;
        vertex[3] = (u8) (index + 9);
        vertex[0x10] = 0;
        vertex[0x12] = (u8) (index + 9);
        vertex[0x13] = (u8) (index + 1);
        vertex += 0x20;
    } while (i < 9);
    i = 1;
    do {
        s32 index;
        s32 next;

        index = i & 7;
        next = i + 8;
        vertex[1] = (u8) i;
        vertex[0x11] = (u8) i;
        i += 1;
        vertex[0] = 0;
        vertex[2] = (u8) (index + 9);
        vertex[3] = (u8) next;
        vertex[0x10] = 0;
        vertex[0x12] = (u8) (index + 1);
        vertex[0x13] = (u8) (index + 9);
        vertex += 0x20;
    } while (i < 9);
    i = 1;
    do {
        s32 index;
        s32 next;

        index = i + 1;
        next = i + 2;
        value = i + 3;
        vertex[1] = (u8) i;
        i += 4;
        vertex[0x32] = (u8) ((value & 7) + 1);
        vertex[0x22] = (u8) ((next & 7) + 1);
        vertex[0x12] = (u8) ((index & 7) + 1);
        vertex[0x31] = (u8) value;
        vertex[0x21] = (u8) next;
        vertex[0x11] = (u8) index;
        vertex[0x10] = 0;
        vertex[0x13] = 0;
        vertex[0x20] = 0;
        vertex[0x23] = 0;
        vertex[0x30] = 0;
        vertex[0x33] = 0;
        vertex += 0x40;
        vertex[-0x40] = 0;
        vertex[-0x3E] = (u8) ((i - 4) + 1);
        vertex[-0x3D] = 0;
    } while (i != 9);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80047304.s")
#endif
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
/* Workbench verdict: structure-mismatch, 176 differing words, first mismatch +0x40. */
/* Candidate: 179/193 instructions with the target -0x150 frame; 80 structural words remain, so it is a structural plateau. */
/* Shape status: branch split and point/call surface are preserved; setup and loop schedule remain short. */
/* PROVENANCE: JFG's fxMakeConeLength role identifies the routine; this body is reconstructed from Mickey's target offsets and m2c control flow. */
#ifdef NON_MATCHING
void func_800479D4(FxCone *cone, s16 height, f32 radius, f32 depth,
                   s32 alpha) {
    FxConePoint points[15];
    FxConePoint *point;
    u8 *vertices;
    u8 addressIndex;
    u8 count;
    s32 angle;
    s32 remaining;
    s32 i;
    f32 originZ;
    f32 scale;
    f32 scaleX;
    f32 scaleY;
    f32 factor;
    f32 temp;

    if (cone != 0) {
        point = points;
        addressIndex = cone->addressIndex ^ 1;
        cone->addressIndex = addressIndex;
        vertices = cone->addresses[addressIndex];
        if (cone->flags != 0) {
            count = cone->segmentCount;
            angle = 0;
            remaining = count - 1;
            if (count != 0) {
                do {
                    point->x = func_8002A8C0(angle) * radius;
                    temp = func_8002A8BC(angle) * depth;
                    point->z = (f32) -height;
                    point++;
                    angle += 0xFFFF0000 / (s32) count;
                    point[-1].y = temp;
                    remaining--;
                } while (remaining != 0);
                count = cone->segmentCount;
            }
            func_80048080(count, cone->value26, cone->value28,
                          cone->value2A, (s32) cone->value20,
                          (s32) cone->value22, points, vertices + 0xA, 0);
            return;
        }
        originZ = (f32) -height;
        mathOneFloatPY((u8 *) cone + 0x20, &points[0].x, height);
        *(s16 *) vertices = (s16) ((s32) points[0].x + cone->value26);
        *(s16 *) (vertices + 2) =
            (s16) ((s32) points[0].y + cone->value28);
        *(s16 *) (vertices + 4) =
            (s16) ((s32) originZ + cone->value2A);
        if (cone->segmentCount == 0) {
            if (alpha < 0x80) {
                factor = 0.0f;
            } else if (alpha >= 0x100) {
                factor = 1.0f;
            } else {
                factor = (f32) (alpha - 0x7F) * 0.0078125f;
            }
            scale = 1.0f + (2.0f * factor);
            i = 0;
            scaleX = cone->value18 * scale;
            scaleY = cone->value1C * scale;
            temp = -((f32) cone->value24 * (0.25f * factor));
            do {
                angle = i << 0xD;
                point->x = func_8002A8C0(angle) * scaleX;
                originZ = func_8002A8BC(angle) * scaleY;
                i++;
                point->z = temp;
                point++;
                point[-1].y = originZ;
            } while (i != 8);
            func_80048080(8, cone->value26, cone->value28, cone->value2A,
                          (s32) cone->value20, (s32) cone->value22, points,
                          vertices + 0x5A, 0xFF);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_800479D4.s")
#endif

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
/* Workbench: allocation-mismatch, 8 differing words, first mismatch +0x298. */
/* Candidate shape: 234 instructions/frame -0x68; CFG and relocations exact, permuter-ready. */
/* Remaining gap: one callee-saved pool-color cascade (v1->a0, a0->a1, a2->v0). */
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

void func_80048080(s32 count, s16 x, s16 y, s16 z, s32 angle0Arg, s32 angle1Arg,
                   FxConePoint *inputArg, void *outputArg, s32 alpha) {
    /* Parameter types follow the top-level prototype the matched callers use. */
    s16 angle0 = angle0Arg;
    s16 angle1 = angle1Arg;
    FxTransformInput *input = (FxTransformInput *) inputArg;
    FxTransformOutput *output = (FxTransformOutput *) outputArg;
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
#ifdef NON_MATCHING
/* Workbench verdict: structure-mismatch, 343 differing words; first mismatch is at +0x0. */
/* Target is 351 instructions/frame -144; candidate is 288 instructions/frame -168. */
/* Remaining gap is structural: allocator initialization/unrolled setup is abbreviated; not permuter-ready. */
Wake *wakeAllocate(s8 wakeType, f32 wakeValue88, s32 wakeValue80,
                   s32 wakeValue84, s16 wakeValue8C, f32 wakeValue8E) {
    s32 arg0 = wakeType;
    f32 arg1 = wakeValue88;
    f32 arg2 = (f32) wakeValue80;
    f32 arg5 = wakeValue8E;
    s32 arg4 = wakeValue8C;
    Wake *wake;
    u8 *wakeBytes;
    u8 *vertexArea;
    u8 *sampleArea;
    s32 frameCount;
    s32 segmentCount;
    s32 segmentBytes;
    s32 vertexBytes;
    s32 sampleBytes;
    s32 textureBytes;
    s32 groupCount;
    s32 i;
    s32 j;
    s32 size;
    s32 alpha;

    frameCount = (s32) (arg1 * 60.0f);
    segmentCount = (frameCount + 5) >> 1;
    groupCount = segmentCount * 2;
    alpha = arg0 == 0 ? 4 : 2;
    segmentBytes = groupCount * 0xA;
    vertexBytes = segmentCount * 0x14;
    sampleBytes = segmentCount * 0x10;
    textureBytes = groupCount * 0x10;
    size = (segmentCount * 0x24) + (alpha * segmentBytes) +
           (textureBytes * 2) + 0x40;
    wake = func_8002B314(size, 0x87);
    if (wake != NULL) {
        wakeBytes = (u8 *) wake;
        vertexArea = wakeBytes + 0x40;
        sampleArea = vertexArea + (groupCount * 0x10);
        *(u8 **) (wakeBytes + 0x10) = vertexArea + vertexBytes;
        *(u8 **) (wakeBytes + 0x14) = sampleArea + sampleBytes;
        for (i = 0; i < 2; i++) {
            *(u8 **) (wakeBytes + 0x18 + (i * 4)) =
                vertexArea + (i * vertexBytes);
        }
        for (i = 0; i < alpha; i++) {
            *(u8 **) (wakeBytes + 0x8 + (i * 4)) =
                sampleArea + (i * segmentBytes);
        }
        wake->vertices = vertexArea;
        wake->samples = sampleArea;
        wake->value4 = arg1;
        wake->value8 = 0;
        wake->valueC = arg2;
        wake->flags = arg0 != 0;
        wake->segmentCount = segmentCount;
        wake->state = 0;
        wake->textureIndex = (s8) frameCount;
        wake->value34 = 0;
        wake->value36 = 0;
        wake->value38 = 0;
        wake->value39 = 0;
        wake->value3A = 0;
        wake->value3B = 0;
        wake->value3C = 0;
        wake->linked = ((void *(*)(s32, s32, void *, s32)) func_80034448)(
            arg4, segmentCount, sampleArea, groupCount);
        if (wake->linked == NULL) {
            mmFree(wake);
            return NULL;
        }
        for (i = 0; i < alpha; i++) {
            u8 *samples = *(u8 **) (wakeBytes + 0x8 + (i * 4));
            for (j = 0; j < groupCount; j++) {
                u8 *sample = samples + (j * 0x14);
                sample[0x6] = 0xFF;
                sample[0x7] = 0xFF;
                sample[0x8] = 0xFF;
                sample[0x10] = 0xFF;
                sample[0x11] = 0xFF;
                sample[0x12] = 0xFF;
                sample[0x1A] = 0xFF;
                sample[0x1B] = 0xFF;
                sample[0x1C] = 0xFF;
                sample[0x24] = 0xFF;
                sample[0x25] = 0xFF;
                sample[0x26] = 0xFF;
            }
        }
        for (i = 0; i < 2; i++) {
            u8 *vertices = *(u8 **) (wakeBytes + 0x18 + (i * 4));
            for (j = 0; j < groupCount; j++) {
                *(u8 *) (vertices + (j * 0x10)) = 0x40;
                *(u8 *) (vertices + (j * 0x10) + 0x10) = 0x40;
                *(u8 *) (vertices + (j * 0x10) + 0x20) = 0x40;
                *(u8 *) (vertices + (j * 0x10) + 0x30) = 0x40;
            }
        }
        wake->value34 = 0;
        wake->value38 = 0;
        wake->value39 = 0;
        wake->value3A = 0;
        wake->value3B = 0;
        wake->value36 = (s16) ((arg5 * 256.0f) / 60.0f);
    }
    return wake;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/wakeAllocate.s")
#endif
/* Workbench: structure-mismatch; 58 words differ, first mismatch +0x08. */
/* Candidate is not opcode-shape exact: 121/121 instructions, frame -72/-72 bytes, exact call relocations; 8 init-schedule words remain, 50 are register-only. */
/* PROVENANCE: Mickey field layouts/control flow reconstructed from target accesses; JFG wakeSetupRipple is assembly-only and supplies only TU/name context. */
#ifdef NON_MATCHING
typedef struct FxRippleSource {
    u8 pad00[0x73];
    s8 wakeType;
    u8 pad74[4];
    f32 textureScale;
    s16 textureId;
    s16 wakeValue7E;
    s32 wakeValue80;
    s32 wakeValue84;
    f32 wakeValue88;
    s16 wakeValue8C;
    s16 wakeValue8E;
} FxRippleSource;

typedef struct FxRippleSetup {
    u8 pad00[0xC];
    f32 valueC;
    u8 pad10[4];
    f32 value14;
    u8 pad18[0x28];
    FxRippleSource *source;
    u8 pad44[0x10];
    u8 *output;
} FxRippleSetup;

typedef struct FxRippleFrame {
    u8 value0;
    u8 value1;
    u8 value2;
    u8 value3;
    s16 value4;
    s16 value6;
    s16 value8;
    s16 valueA;
    s16 valueC;
    s16 valueE;
    u8 value10;
    u8 value11;
    u8 value12;
    u8 value13;
    s16 value14;
    s16 value16;
    s16 value18;
    s16 value1A;
    s16 value1C;
    s16 value1E;
    u8 pad20[8];
} FxRippleFrame;

typedef struct FxRippleOutput {
    FxRippleFrame frames[2];
    u8 pad50[0x20];
    FxConeTextureInfo *texture;
    u8 value74;
    u8 value75;
    s16 value76;
    s16 value78;
    s16 value7A;
    f32 value7C;
    f32 value80;
    Wake *wake;
} FxRippleOutput;

extern void func_8001357C(f32 valueC, f32 value14, void *output,
                          s32 value, s32 zero);
extern Wake *wakeAllocate(s8 wakeType, f32 wakeValue88, s32 wakeValue80,
                          s32 wakeValue84, s16 wakeValue8C,
                          f32 wakeValue8E);

s32 func_80048760(void *arg0, s32 arg1) {
    u8 pad[16];
    s32 size;
    s32 var_a0;
    FxRippleOutput *var_s0;
    FxRippleSource *temp_t0;
    FxConeTextureInfo *temp_a2;
    s16 temp_t7;
    s16 temp_t8;
    u8 *var_v0;
    FxRippleFrame *frame;

    size = arg1 & 7;
    var_s0 = (FxRippleOutput *) arg1;
    if (size != 0) {
        size = 8 - size;
        var_s0 = (FxRippleOutput *) (arg1 + size);
    } else {
        size = 0;
    }
    size += (s32) align4((u8 *) 0x88);
    temp_t0 = ((FxRippleSetup *) arg0)->source;
    ((FxRippleSetup *) arg0)->output = (u8 *) var_s0;
    var_s0->texture = func_80034448(temp_t0->textureId);
    if (var_s0->texture == 0) {
        return 0;
    }
    temp_a2 = var_s0->texture;
    temp_t7 = (temp_a2->width - 1) << 5;
    temp_t8 = (temp_a2->height - 1) << 5;
    frame = (FxRippleFrame *) var_s0;
    frame->value0 = 0x40;
    frame->value1 = 0;
    frame->value4 = temp_t7;
    frame->value6 = 0;
    frame->value2 = 1;
    frame->value8 = 0;
    frame->valueA = 0;
    frame->value3 = 2;
    frame->valueC = temp_t7;
    frame->valueE = temp_t8;
    frame->value10 = 0x40;
    frame->value11 = 1;
    frame->value14 = 0;
    frame->value16 = 0;
    frame->value12 = 2;
    frame->value18 = temp_t7;
    frame->value1A = temp_t8;
    frame->value13 = 3;
    frame->value1C = 0;
    frame->value1E = temp_t8;

    var_a0 = 0;
    var_v0 = (u8 *) var_s0;
    do {
        var_a0++;
        var_v0 += 0x28;
        var_v0[0x8] = 0xFF;
        var_v0[0x9] = 0xFF;
        var_v0[0xA] = 0xFF;
        var_v0[0xB] = 0xFF;
        var_v0[0x12] = 0xFF;
        var_v0[0x13] = 0xFF;
        var_v0[0x14] = 0xFF;
        var_v0[0x15] = 0xFF;
        var_v0[0x1C] = 0xFF;
        var_v0[0x1D] = 0xFF;
        var_v0[0x1E] = 0xFF;
        var_v0[0x1F] = 0xFF;
        var_v0[-2] = 0xFF;
        var_v0[-1] = 0xFF;
        var_v0[0] = 0xFF;
        var_v0[1] = 0xFF;
    } while (var_a0 != 2);

    var_s0->value74 = 0;
    var_s0->value75 = 0;
    var_s0->value76 = 0;
    var_s0->value78 = 0;
    var_s0->value7A = temp_t0->wakeValue7E;
    var_s0->value7C = temp_t0->textureScale;
    func_8001357C(((FxRippleSetup *) arg0)->valueC,
                  ((FxRippleSetup *) arg0)->value14,
                  (u8 *) var_s0 + 0x80,
                  0x10000, 0);
    var_s0->wake = 0;
    if (temp_t0->wakeType != -1) {
        var_s0->wake = wakeAllocate(temp_t0->wakeType, temp_t0->wakeValue88,
                                    temp_t0->wakeValue80, temp_t0->wakeValue84,
                                    temp_t0->wakeValue8C,
                                    (f32) temp_t0->wakeValue8E);
    }
    return size;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80048760.s")
#endif
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
#ifdef NON_MATCHING
/* Workbench verdict: structure-mismatch, 386 differing words; first mismatch is at +0x0. */
/* Target is 398 instructions/frame -144; candidate is 380 instructions/frame -176. */
/* Remaining gap is structural: wake display-list/polygon state and local frame shape differ; not permuter-ready. */
void wakeUpdate(s32 update, f32 arg1, f32 arg2, f32 arg3, s32 angle, s32 arg5) {
    /* Parameter types follow the top-level prototype the matched callers use. */
    Wake *wake = (Wake *) update;
    s16 arg4 = (s16) angle;
    u8 *wakeBytes = (u8 *) wake;
    u8 *samples = *(u8 **) (wakeBytes + 0x14);
    s32 temp_lo;
    s32 var_v0;
    s32 value;
    s32 polygonOffset;
    s32 vertexCount;
    s32 stripWords;
    s32 mark;
    s8 stripIndex;
    s16 outputCount;
    s16 outputOffset;
    u8 index;
    u8 nextIndex;
    u8 currentState;
    u8 *sample;
    u8 *vertices;
    u8 *secondaryVertices;
    u8 *display;
    u8 *polygon;
    f32 sine;
    f32 cosine;
    f32 distance;

    index = wake->value39;
    var_v0 = wake->value3B - 1;
    if (wake->value3B != 0) {
        do {
            temp_lo = index * 0x14;
            nextIndex = index + 1;
            if (nextIndex >= wake->segmentCount) {
                nextIndex = 0;
            }
            if (arg5 >= (s32) *(s16 *) (samples + temp_lo)) {
                wake->value39 = nextIndex;
                wake->value3B--;
            } else {
                var_v0 = 0;
            }
            index = nextIndex;
            var_v0--;
        } while (var_v0 != 0);
        index = wake->value39;
    }
    sample = samples + (index * 0x14);
    sample[1] |= 0x80;
    mark = 0;
    if (wake->flags & 2) {
        if (wake->value8 == 0) {
            mark = 1;
        }
        if (wake->value8 < 0xBF) {
            wake->value8 += 0x40;
        } else {
            wake->value8 = 0xFF;
        }
        value = wake->value3C + (arg5 * 0x10);
        wake->value3C = value;
        if (value >= 0x100) {
            wake->value3C = 0xFF;
        }
    } else {
        if (wake->value8 >= 0x41) {
            wake->value8 -= 0x40;
        } else {
            wake->value8 = 0;
        }
        value = wake->value3C - (arg5 * 0x10);
        wake->value3C = value;
        if (value < 0) {
            wake->value3C = 0;
        }
    }
    if ((wake->value8 != 0) && (wake->value3B < wake->segmentCount)) {
        sample = samples + (wake->value3A * 0x14);
        sample[0] = wake->textureIndex;
        value = wake->value8 >> 1;
        sample[1] = value;
        if (mark != 0) {
            sample[1] = value | 0x80;
        }
        *(s16 *) (sample + 2) = arg4;
        *(f32 *) (sample + 8) = arg1;
        *(f32 *) (sample + 0xC) = arg3;
        *(s16 *) (sample + 4) = (s16) ((*(s16 *)
            ((u8 *) wake->linked + 8) - 1) << 8);
        *(s16 *) (sample + 6) = (s16) arg2;
        *(f32 *) (sample + 0x10) = (f32) wake->value4;
        wake->value3A++;
        if (wake->value3A >= wake->segmentCount) {
            wake->value3A = 0;
        }
        wake->value3B++;
    }
    wake->state = 1 - wake->state;
    wake->value38 = 0;
    if (wake->value3B != 0) {
        currentState = wake->state;
        vertices = *(u8 **) (wakeBytes + 0x18 + (currentState * 4));
        secondaryVertices = *(u8 **) (wakeBytes + 0x20 + (currentState * 4));
        polygon = *(u8 **) (wakeBytes + 0x28 + (currentState * 4));
        outputCount = 0;
        outputOffset = 0;
        polygonOffset = (*(s16 *) ((u8 *) wake->linked + 6) - 1) << 5;
        stripWords = 0;
        stripIndex = 0;
        if (wake->value39 != wake->value3A) {
            index = wake->value39;
            do {
                temp_lo = index * 5;
                nextIndex = index + 1;
                sample = samples + (temp_lo * 4);
                if (nextIndex >= wake->segmentCount) {
                    nextIndex = 0;
                }
                if (sample[1] & 0x80) {
                    display = *(u8 **) (wakeBytes + 0x10) +
                              (wake->value38 * 0x10);
                    stripIndex = 0;
                    outputOffset = 1;
                    if (outputOffset != 0) {
                        *(s16 *) (display + 0xC) = outputCount;
                        *(s16 *) (display + 0xE) = vertexCount;
                        wake->value38++;
                    }
                    outputCount = 0;
                    *(u32 *) (display + 0x0) = (u32) vertices;
                    *(u32 *) (display + 0x4) = (u32) secondaryVertices;
                    *(u32 *) (display + 0x8) = (u32) polygon;
                    stripWords = 0;
                }
                sample[0] -= arg5;
                *(f32 *) (sample + 0x10) += wake->valueC * (f32) arg5;
                *(s16 *) (sample + 4) -= wake->value8 * arg5;
                sine = func_8002A8C0(*(s16 *) (sample + 2)) *
                       *(f32 *) (sample + 0x10);
                cosine = func_8002A8BC(*(s16 *) (sample + 2)) *
                         *(f32 *) (sample + 0x10);
                value = ((sample[1] & 0x7F) * wake->value3C) >> 7;
                vertices += 0xA;
                *(s16 *) (vertices - 0xA) =
                    (s16) (*(f32 *) (sample + 8) - cosine);
                *(s16 *) (vertices - 8) = *(s16 *) (sample + 6);
                *(s8 *) (vertices - 1) = value;
                *(s16 *) (vertices - 6) =
                    (s16) (*(f32 *) (sample + 0xC) + sine);
                if (secondaryVertices == NULL) {
                    *(s16 *) (vertices + 0) =
                        (s16) (*(f32 *) (sample + 8) + cosine);
                    *(s16 *) (vertices + 4) =
                        (s16) (*(f32 *) (sample + 0xC) - sine);
                } else {
                    secondaryVertices += 0x14;
                    *(s16 *) (vertices + 0) = (s16) *(f32 *) (sample + 8);
                    *(s16 *) (vertices + 4) =
                        (s16) *(f32 *) (sample + 0xC);
                    *(s16 *) (secondaryVertices - 0x14) =
                        (s16) (*(f32 *) (sample + 8) + cosine);
                    *(s16 *) (secondaryVertices - 0x12) =
                        *(s16 *) (sample + 6);
                    *(s8 *) (secondaryVertices - 0xB) = value;
                    *(s16 *) (secondaryVertices - 0x10) =
                        (s16) (*(f32 *) (sample + 0xC) - sine);
                    *(s16 *) (secondaryVertices - 0xA) =
                        *(s16 *) (vertices + 0);
                    *(s16 *) (secondaryVertices - 8) =
                        *(s16 *) (sample + 6);
                    *(s8 *) (secondaryVertices - 1) = value;
                    *(s16 *) (secondaryVertices - 6) =
                        *(s16 *) (vertices + 4);
                }
                *(s8 *) (vertices - 1) = value;
                outputCount += 2;
                vertexCount = (*(s16 *) (sample + 4)) >> 3;
                if (stripIndex != 0) {
                    polygon[3] = stripIndex;
                    *(s16 *) (polygon + 0xC) = 0;
                    *(s16 *) (polygon + 0xE) = vertexCount;
                    *(s16 *) (polygon + 0x18) = 0;
                    *(s16 *) (polygon + 0x1A) = vertexCount;
                    polygon[0x13] = stripIndex + 1;
                    *(s16 *) (polygon + 0x1C) = polygonOffset;
                    *(s16 *) (polygon + 0x1E) = vertexCount;
                    polygon += 0x20;
                    stripWords += 2;
                    if ((stripIndex + 2) >= 0x11) {
                        stripIndex = 0;
                    }
                }
                stripIndex++;
                polygon[1] = stripIndex - 1;
                *(s16 *) (polygon + 4) = 0;
                *(s16 *) (polygon + 6) = vertexCount;
                polygon[2] = stripIndex;
                *(s16 *) (polygon + 8) = polygonOffset;
                *(s16 *) (polygon + 0xA) = vertexCount;
                polygon[0x11] = stripIndex;
                *(s16 *) (polygon + 0x14) = polygonOffset;
                *(s16 *) (polygon + 0x16) = vertexCount;
                stripIndex += 2;
                index = nextIndex;
            } while (index != wake->value3A);
        }
        display = *(u8 **) (wakeBytes + 0x10) + (wake->value38 * 0x10);
        *(s16 *) (display + 0xC) = outputCount;
        *(s16 *) (display + 0xE) = vertexCount;
        wake->value38++;
        distance = (f32) wake->value36 * (f32) arg5;
        wake->value34 = (s16) (wake->value34 + distance);
        while (wake->value34 >= *(s16 *) ((u8 *) wake->linked + 0x10)) {
            wake->value34 -= *(s16 *) ((u8 *) wake->linked + 0x10);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/wakeUpdate.s")
#endif
/* Workbench verdict: structure-mismatch, 125 differing words, first mismatch +0x0. */
/* Candidate: 150/149 instructions with a -0x30 frame versus target -0x38; 88 structural words remain, so it is not shape-exact. */
/* Shape status: ripple fade/angle/vertex updates and both calls are present; frame and pointer-layout gap remains. */
/* PROVENANCE: JFG names the corresponding routine wakeUpdateRipple; this Mickey body uses only Mickey target offsets and calls. */
#ifdef NON_MATCHING
void func_80049000(FxWakeUpdateOwner *owner, s32 delta) {
    FxWakeRippleData *ripple;
    u8 mode;
    s16 angle;
    s16 step;
    s32 height;
    u8 *vertex;

    ripple = owner->ripple;
    if (ripple != 0) {
        if (ripple->active != 0) {
            ripple->fade = (s16) (ripple->fade + 0x20);
            if (ripple->fade >= 0x100) {
                ripple->fade = 0xFF;
            }
        } else {
            ripple->fade = (s16) (ripple->fade - 0x20);
            if (ripple->fade < 0) {
                ripple->fade = 0;
            }
        }
        step = ripple->angleStep;
        if (step != 0) {
            if (ripple->update != 0) {
                ripple->angle = (s16) (ripple->angle + (step * delta));
                while (ripple->angle >= (s32) ((FxWakeTexture *) ripple->texture)->length) {
                    ripple->angle = (s16) (ripple->angle - ((s32) ((FxWakeTexture *) ripple->texture)->length));
                }
            }
        }
        if (ripple->fade != 0) {
            mode = 1 - ripple->mode;
            ripple->mode = mode;
            height = (s32) ripple->value80;
            vertex = (u8 *) ripple + ((mode & 0xFF) * 0x28);
            *(s16 *) (vertex + 0x22) = (s16) height;
            vertex += 0x3E;
            *(s16 *) (vertex - 0x1E) = (s16) (s32) (owner->valueC + ripple->value7C);
            *(s16 *) (vertex - 0x1A) = (s16) (s32) (owner->value14 - ripple->value7C);
            *(s16 *) (vertex - 0x12) = (s16) height;
            *(s16 *) (vertex - 0x14) = (s16) (s32) (owner->valueC - ripple->value7C);
            *(s16 *) (vertex - 0x10) = (s16) (s32) (owner->value14 - ripple->value7C);
            *(s16 *) (vertex - 8) = (s16) height;
            *(s16 *) (vertex - 0xA) = (s16) (s32) (owner->valueC + ripple->value7C);
            *(s16 *) (vertex - 6) = (s16) (s32) (owner->value14 + ripple->value7C);
            *(s16 *) (vertex + 2) = (s16) height;
            *(s16 *) vertex = (s16) (s32) (owner->valueC - ripple->value7C);
            *(s16 *) (vertex + 4) = (s16) (s32) (owner->value14 + ripple->value7C);
        }
        angle = Arctanf(owner->value1C, owner->value24);
        if (ripple->update != 0) {
            wakeUpdate((s32) ripple->update, owner->valueC, ripple->value80,
                       owner->value14, angle, delta);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80049000.s")
#endif
/* Workbench verdict: structure-mismatch, 177 differing words, first mismatch +0x0. */
/* Candidate: 163/177 instructions with a -0x38 frame versus target -0x88; command-loop structural gap remains, so it is not shape-exact. */
/* Shape status: display-list commands and chunk emission are reconstructed; target's live-variable/stack shape is unresolved. */
/* PROVENANCE: JFG's wakeDraw role supplies the display-list idiom; this body is reconstructed from Mickey's target offsets and FxGfx type. */
#ifdef NON_MATCHING
void wakeDraw(Wake *wake, FxGfx **dlist) {
    s32 alpha;
    s32 outerOffset;
    s32 outerIndex;
    s16 remaining;
    s16 chunk;
    s32 x;
    s32 y;
    s32 z;
    s32 chunkWidth;
    s32 texelWidth;
    s32 command0;
    FxGfx *cmd;
    FxWakeSegment *segment;

    if ((s32) wake->value38 > 0) {
        func_800349A4(dlist, (s32) wake->linked, 0x1F,
                      (s32) wake->value34 << 8);
        if ((((FxGfx *) wake->linked)->w1 & 0x40) != 0) {
            alpha = wake->value34 & 0xFF;
        } else {
            alpha = 0xFF;
        }
        cmd = *dlist;
        *dlist = cmd + 1;
        cmd->w0 = 0xFB000000;
        alpha &= 0xFF;
        cmd->w1 = (alpha << 24) | (alpha << 16) | (alpha << 8) | alpha;
        cmd = *dlist;
        *dlist = cmd + 1;
        cmd->w1 = -1;
        cmd->w0 = 0xFA000000;
        outerIndex = 0;
        outerOffset = 0;
        if ((s32) wake->value38 > 0) {
            do {
                segment = (FxWakeSegment *) ((u8 *) wake->vertices + outerOffset);
                remaining = segment->length;
                x = segment->x;
                y = segment->y;
                z = segment->z;
                if (remaining != 0) {
                    do {
                        s32 shiftedX;
                        s32 shiftedY;
                        s32 shiftedZ;

                        shiftedX = x + 0x80000000;
                        if (remaining >= 0x11) {
                            remaining -= 0x10;
                            chunk = 0x10;
                        } else {
                            chunk = remaining;
                            remaining = 0;
                        }
                        chunkWidth = (chunk + 2) * 8;
                        texelWidth = ((chunk + 2) * 0xA) + 8;
                        cmd = *dlist;
                        *dlist = cmd + 1;
                        command0 = (((chunkWidth | (shiftedX & 6)) & 0xFF) << 16) |
                                   0x04000000 | (texelWidth & 0xFFFF);
                        cmd->w0 = command0;
                        cmd->w1 = shiftedX;
                        cmd = *dlist;
                        *dlist = cmd + 1;
                        chunkWidth = chunk * 0x10;
                        cmd->w0 = (((((chunk - 1) * 0x10) | 1) & 0xFF) << 16) |
                                   0x05000000 | (chunkWidth & 0xFFFF);
                        shiftedZ = z + 0x80000000;
                        cmd->w1 = shiftedZ;
                        x += chunk * 0xA;
                        if (y != 0) {
                            cmd = *dlist;
                            shiftedY = y + 0x80000000;
                            *dlist = cmd + 1;
                            cmd->w0 = (((chunkWidth | (shiftedY & 6)) & 0xFF) << 16) |
                                       0x04000000 | (texelWidth & 0xFFFF);
                            cmd->w1 = shiftedY;
                            cmd = *dlist;
                            y += chunk * 0xA;
                            *dlist = cmd + 1;
                            cmd->w1 = shiftedZ;
                            cmd->w0 = (((((chunk - 1) * 0x10) | 1) & 0xFF) << 16) |
                                       0x05000000 | (chunkWidth & 0xFFFF);
                        }
                        z += chunkWidth;
                    } while (remaining != 0);
                    outerIndex = wake->value38;
                }
                outerIndex++;
                outerOffset += 0x10;
            } while (outerIndex < (s32) wake->value38);
        }
        if (alpha != 0xFF) {
            cmd = *dlist;
            *dlist = cmd + 1;
            cmd->w1 = 0;
            cmd->w0 = 0xE7000000;
            cmd = *dlist;
            *dlist = cmd + 1;
            cmd->w1 = -1;
            cmd->w0 = 0xFB000000;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/wakeDraw.s")
#endif
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
/* Workbench: structure-mismatch, 33 differing words, first mismatch +0xD0. */
/* Candidate shape: 98/100 instructions/frame -0x30; camera join and field stores are aligned. */
/* Remaining gap: flag/state tail has four structural words; register residuals remain. */
#ifdef NON_MATCHING
extern s32 camGetMode(void);
extern void func_80021FB0(s32 mode, s32 camNo, s32 *x1, s32 *y1,
                          u32 *x2, u32 *y2);

void func_800498FC(s32 index, f32 value16, f32 value18, s32 red, s32 green,
                   s32 blue, s32 flags) {
    FxRecord *record;

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
    record->value16 = (s16)(value16 * 60.0f);
    record->value18 = (s16)(value18 * 60.0f);
    record->red = red;
    record->green = green;
    record->blue = blue;
    record->value1D = flags & 0xFF3F;
    record->value1E = flags & 0x80;
    record->value1F = flags & 0x40;
    if ((flags & 0x80) != 0) {
        if ((flags & 0x40) != 0) {
            record->state = 3;
        } else {
            record->state = 2;
        }
        record->status = 0xFF;
        return;
    }
    record->state = 1;
    record->status = 0;
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
/* Workbench verdict: structure-mismatch, 216 differing words, first mismatch +0x8. */
/* Candidate: 219/206 instructions with the target -0x18 frame; switch-state structural gap remains, so it is not shape-exact. */
/* Shape status: four-record state machine and signed timing fields are reconstructed; branch/constant schedule remains. */
/* PROVENANCE: Mickey's own FxRecord layout and m2c draft supply the state transitions; no external body is adapted here. */
#ifdef NON_MATCHING
s32 func_80049B14(s16 delta) {
    FxRecord *record;
    s32 bit;
    s16 carry;
    s16 duration;
    s16 current;
    s16 next;
    u16 flags;
    u8 mode;

    D_800D5F50 = 0;
    record = D_800D5F58;
    bit = 4;
    do {
        if (record->state != 0) {
            flags = record->flags;
            carry = delta;
            if ((flags & 4) != 0) {
                record->flags = flags & ~4;
            } else if (delta != 0) {
                do {
                    switch (record->state) {
                    case 0:
                        carry = 0;
                        record->status = 0;
                        break;
                    case 1:
                        duration = record->value16;
                        record->value14 = (s16) (record->value14 + carry);
                        current = record->value14;
                        if (current >= duration) {
                            if (record->value18 != 0) {
                                next = current - duration;
                                carry = next;
                                record->state = 2;
                                record->value14 = next;
                                record->status = 0xFF;
                            } else {
                                carry = 0;
                                if ((record->value1F != 0) &&
                                    (record->value1E == 0)) {
                                    next = current - duration;
                                    carry = next;
                                    record->state = 3;
                                    record->value14 = next;
                                    record->status = 0xFF;
                                } else {
                                    record->state = 0;
                                    record->status = 0;
                                }
                            }
                        } else {
                            carry = 0;
                            record->status =
                                (u8) ((current * 0xFF) / duration);
                        }
                        break;
                    case 2:
                        duration = record->value18;
                        if (duration < 0) {
                            carry = 0;
                        } else {
                            record->value14 = (s16) (record->value14 + carry);
                            current = record->value14;
                            carry = 0;
                            if (current >= duration) {
                                mode = record->value1E;
                                if (((mode != 0) && (record->value1F == 0)) ||
                                    ((mode == 0) && (record->value1F != 0))) {
                                    carry = current - duration;
                                    record->state = 3;
                                    record->value14 = carry;
                                } else {
                                    if (record->value1F != 0) {
                                        carry = current - duration;
                                        record->state = 1;
                                        record->value14 = carry;
                                    } else {
                                        record->state = 0;
                                        record->status = 0;
                                    }
                                }
                            }
                        }
                        break;
                    case 3:
                        duration = record->value16;
                        record->value14 = (s16) (record->value14 + carry);
                        current = record->value14;
                        if (current >= duration) {
                            carry = 0;
                            if ((record->value1E != 0) &&
                                (record->value1F != 0) &&
                                (record->value18 != 0)) {
                                next = current - duration;
                                carry = next;
                                record->state = 2;
                                record->value14 = next;
                                record->status = 0;
                            } else {
                                record->state = 0;
                                record->status = 0;
                            }
                        } else {
                            carry = 0;
                            record->status =
                                (u8) (((duration - current) * 0xFF) / duration);
                        }
                        break;
                    }
                } while (carry != 0);
            }
            if (record->state != 0) {
                D_800D5F50 |= 0x10 >> bit;
            }
        }
        record++;
        bit--;
    } while (bit != 0);
    return D_800D5F50;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80049B14.s")
#endif
/* Workbench verdict: structure-mismatch, 159 differing words, first mismatch +0x8. */
/* Candidate: 167/169 instructions with a -0x50 frame versus target -0x60; display-list structural gap remains, so it is not shape-exact. */
/* Shape status: VI setup, table selection, per-record commands, and scissor calls are reconstructed. */
/* PROVENANCE: Mickey's own target command words, globals, and m2c CFG supply this reconstruction. */
#ifdef NON_MATCHING
void func_80049E4C(FxGfx **dlist, s32 arg1) {
    s32 width;
    s32 height;
    s32 count;
    s32 remaining;
    s32 u;
    s32 v;
    s32 color;
    f32 widthFloat;
    f32 heightFloat;
    FxGfx *cmd;
    u8 *table;
    u8 *entry;

    if (D_800D5F50 != 0) {
        viGetCurrentSize(&width, &height);
        cmd = *dlist;
        *dlist = cmd + 1;
        cmd->w1 = 0;
        cmd->w0 = 0xE7000000;
        cmd = *dlist;
        *dlist = cmd + 1;
        cmd->w0 = 0xED000000;
        widthFloat = (f32) width;
        if (width < 0) {
            widthFloat += 4294967296.0f;
        }
        heightFloat = (f32) height;
        if (height < 0) {
            heightFloat += 4294967296.0f;
        }
        u = (s32) (widthFloat * 4.0f) & 0xFFF;
        v = (s32) (heightFloat * 4.0f) & 0xFFF;
        cmd->w1 = (u << 12) | v;
        cmd = *dlist;
        *dlist = cmd + 1;
        cmd->w0 = 0xB6000000;
        cmd->w1 = 0x10001;
        cmd = *dlist;
        *dlist = cmd + 1;
        cmd->w1 = 0xFFFDF6FB;
        cmd->w0 = 0xFCFFFFFF;
        if (arg1 == 0) {
            table = (u8 *) D_800D5F58;
            count = 4;
        } else {
            table = (u8 *) D_800D5FD8;
            count = 1;
        }
        remaining = count - 1;
        if (count != 0) {
            do {
                entry = table;
                if (entry[1] != 0) {
                    cmd = *dlist;
                    *dlist = cmd + 1;
                    if (entry[1] == 0xFF) {
                        cmd->w1 = 0x0F0A4000;
                    } else {
                        cmd->w1 = 0x504340;
                    }
                    cmd->w0 = 0xEF002C0F;
                    cmd = *dlist;
                    *dlist = cmd + 1;
                    cmd->w0 = 0xFA000000;
                    color = (entry[0x1A] << 24) | (entry[0x1B] << 16) |
                            (entry[0x1C] << 8) | entry[1];
                    cmd->w1 = color;
                    cmd = *dlist;
                    *dlist = cmd + 1;
                    cmd->w0 = ((*(s32 *) (entry + 0xC) & 0x3FF) << 14) |
                              0xF6000000 |
                              ((*(s32 *) (entry + 0x10) & 0x3FF) * 4);
                    cmd->w1 = ((*(s32 *) (entry + 4) & 0x3FF) << 14) |
                              ((*(s32 *) (entry + 8) & 0x3FF) * 4);
                    cmd = *dlist;
                    *dlist = cmd + 1;
                    cmd->w1 = 0;
                    cmd->w0 = 0xE7000000;
                }
                table += 0x20;
                remaining--;
            } while (remaining != 0);
        }
        func_80034920(dlist, table, dlist);
        camSetScissor(dlist);
        cmd = *dlist;
        *dlist = cmd + 1;
        cmd->w1 = -1;
        cmd->w0 = 0xFA000000;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80049E4C.s")
#endif
void func_8004A0F0(void) {
    D_800D6038[0] = 0;
    D_800D6038[1] = 0;
    D_800D6040 = 0;
}
/* Workbench verdict: structure-mismatch, 155 differing words, first mismatch +0x0. */
/* Candidate: 156/157 instructions with a -0x60 frame versus target -0x58; 29 structural words remain, so it is not shape-exact. */
/* Shape status: nine-pixel glyph loop and VI/table relocation surface are preserved; stack/register gap remains. */
/* PROVENANCE: JFG's corresponding routine is assembly-only; this body is reconstructed from Mickey's own m2c draft and headers. */
#ifdef NON_MATCHING
void func_8004A10C(s32 *screen, u8 glyph, s32 x, s32 y, s32 arg4) {
    s32 width;
    s32 height;
    u32 *pattern;
    u16 *pixel;
    s32 glyphValue;
    s32 colorMask;
    s32 shift;
    s32 column;
    s32 rowBits;
    s32 bit;
    s32 intensity;
    s32 oldPixel;
    s32 maskedPixel;
    s32 value;

    glyphValue = glyph;
    viGetCurrentSize(&width, &height);
    colorMask = 0x7C0;
    shift = 6;
    pattern = D_8007D320;
    pixel = (u16 *) ((u8 *) screen + ((((y * width) + x) * 2)));
    if (arg4 != 0) {
        colorMask = 0xF800;
        shift = 0xB;
    }
    do {
        rowBits = *pattern;
        column = 1;
        intensity = 4;
        bit = rowBits & 7;
        rowBits >>= 3;
        if (bit != 0) {
            if (glyphValue & (1 << bit)) {
                intensity = 0x10;
            }
            oldPixel = *pixel;
            maskedPixel = oldPixel & colorMask;
            value = maskedPixel + (intensity << shift);
            if ((~colorMask & value) != 0) {
                value = colorMask;
            }
            *pixel = (oldPixel ^ maskedPixel) | value;
        }
        pixel++;
    loop_9:
        bit = rowBits & 7;
        rowBits >>= 3;
        if (bit != 0) {
            intensity = 4;
            if (glyphValue & (1 << bit)) {
                intensity = 0x10;
            }
            oldPixel = *pixel;
            maskedPixel = oldPixel & colorMask;
            value = maskedPixel + (intensity << shift);
            if ((~colorMask & value) != 0) {
                value = colorMask;
            }
            *pixel = (oldPixel ^ maskedPixel) | value;
        }
        bit = rowBits & 7;
        rowBits >>= 3;
        pixel++;
        if (bit != 0) {
            intensity = 4;
            if (glyphValue & (1 << bit)) {
                intensity = 0x10;
            }
            oldPixel = *pixel;
            maskedPixel = oldPixel & colorMask;
            value = maskedPixel + (intensity << shift);
            if ((~colorMask & value) != 0) {
                value = colorMask;
            }
            *pixel = (oldPixel ^ maskedPixel) | value;
        }
        bit = rowBits & 7;
        rowBits >>= 3;
        pixel++;
        if (bit != 0) {
            intensity = 4;
            if (glyphValue & (1 << bit)) {
                intensity = 0x10;
            }
            oldPixel = *pixel;
            maskedPixel = oldPixel & colorMask;
            value = maskedPixel + (intensity << shift);
            if ((~colorMask & value) != 0) {
                value = colorMask;
            }
            *pixel = (oldPixel ^ maskedPixel) | value;
        }
        bit = rowBits & 7;
        rowBits >>= 3;
        pixel++;
        if (bit != 0) {
            intensity = 4;
            if (glyphValue & (1 << bit)) {
                intensity = 0x10;
            }
            oldPixel = *pixel;
            maskedPixel = oldPixel & colorMask;
            value = maskedPixel + (intensity << shift);
            if ((~colorMask & value) != 0) {
                value = colorMask;
            }
            *pixel = (oldPixel ^ maskedPixel) | value;
        }
        column += 4;
        pixel++;
        if (column != 9) {
            goto loop_9;
        }
        pattern++;
        pixel += width - 9;
    } while (pattern != (u32 *) D_8007D364);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004A10C.s")
#endif
/* PROVENANCE: role adapted from JFG src/fx.c::func_8006DF90; both bodies are
 * assembly-only, so this reconstruction is Mickey-derived. */
/* Workbench: instruction-words-identical, 0 differing words; 76 instructions/frame -0x80. */
void func_8004A380(s32 x, s32 y, s32 value, s32 minimumWidth, s32 arg4) {
    s32 length;
    s32 index;
    u8 glyph;
    u8 character;
    char text[32];

    length = 0;
    index = 0;
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
                character = text[index++];
                if (character == '-') {
                    glyph = D_8007D364[10];
                } else if (character >= '0' && character < ':') {
                    glyph = D_8007D364[character - '0'];
                }
            }
            func_8004A10C(D_800D2FA0, glyph, x, y, arg4);
            x += 10;
        } while (text[index] != '\0');
    }
}
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
#ifdef NON_MATCHING
/* PROVENANCE -- Jet Force Gemini's public fx.c places the same-named
 * fxSPDPRipple routine at this TU boundary, but publishes assembly only.
 * Mickey's target assembly supplies the fields, constants, and call order. */
typedef struct FxRippleLevel {
    u8 pad00[0xFA];
    u8 rippleEnabled;
} FxRippleLevel;

extern FxRippleLevel *levelGetLevel(void);
extern s32 func_8002A204(s32 angle);

/* Workbench verdict: structure-mismatch; 224 differing words, first mismatch +0x10. */
/* Target 232 instructions/frame -168; candidate 234 instructions/frame -168. */
/* Remaining gap is prologue/global and command-loop schedule; not shape-exact. */
void fxSPDPRipple(FxGfx **dList, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                  s32 arg5) {
    FxGfx *command;
    FxRippleLevel *level;
    s32 sp8C;
    s32 sp84;
    s32 sp44;
    s16 temp_t0;
    s16 var_a2;
    s16 var_a3;
    s16 var_s4;
    s16 var_s5;
    s16 var_s6;
    s32 temp_s2;
    s32 temp_s3;
    s32 temp_t3;
    s32 temp_t3_2;
    s32 temp_v1_2;
    s32 var_a0;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a2_2;
    s32 var_a3_2;
    s32 var_s1;
    s32 var_t0;
    u8 temp_v1;

    level = levelGetLevel();
    if ((level != NULL) && (level->rippleEnabled != 0)) {
        func_800349A4(dList, 0, 4, 0);
        command = *dList;
        var_a1 = arg5;
        *dList = command + 1;
        command->w0 = 0xFCFFFFFF;
        command->w1 = 0xFFFDF6FB;
        temp_v1 = level->rippleEnabled;
        var_a2 = D_8007D370[0] + ((var_a1 << 0xD) >> 4);
        var_a3 = D_8007D374[0] + ((var_a1 * -0x3C00) >> 4);
        temp_t0 = D_8007D378[0] + ((var_a1 * 0x1800) >> 4);
        D_8007D370[0] = var_a2;
        var_s4 = var_a2 + (arg2 << 0xA);
        D_8007D374[0] = var_a3;
        D_8007D378[0] = temp_t0;
        temp_t3 = (s32)(temp_v1 * 0x50) >> 7;
        var_s5 = var_a3 + (arg2 * 0xBA2);
        var_s6 = temp_t0 + (arg2 * 0x28F);
        sp8C = (s32)(temp_v1 * 0x58) >> 7;
        sp84 = (s32)(temp_v1 * 0x48) >> 7;
        var_s1 = arg2;
        if (arg2 < arg4) {
            sp44 = (arg1 & 0x3FF) << 0xE;
            do {
                temp_s2 = func_8002A204(var_s5);
                temp_s3 = func_8002A204(var_s4);
                temp_t3_2 = ((func_8002A204(var_s6) << 6) +
                             (temp_s3 * 0xC0) + (temp_s2 * 0x60)) >> 8;
                var_a0 = temp_t3_2;
                if (temp_t3_2 < 0) {
                    var_a0 = -temp_t3_2;
                    var_a1_2 = 8;
                    var_a2_2 = 0x20;
                    var_a3_2 = 0xA0;
                    var_t0 = sp84;
                } else {
                    var_a1_2 = 0x80;
                    var_a2_2 = 0xC0;
                    var_a3_2 = 0xFF;
                    var_t0 = sp8C;
                }
                if (var_a0 >= 0x10001) {
                    var_a0 = 0x10000;
                }
                command = *dList;
                temp_v1_2 = var_s1 + 1;
                var_s4 += 0x400;
                var_s5 += 0xBA2;
                var_s6 += 0x28F;
                var_a1 = (((var_a1_2 - 0x20) * var_a0) >> 0x10) + 0x20;
                *dList = command + 1;
                command->w0 = 0xFA000000;
                var_a2 = (((var_a2_2 - 0x78) * var_a0) >> 0x10) + 0x78;
                var_a3 = (((var_a3_2 - 0xFF) * var_a0) >> 0x10) + 0xFF;
                command->w1 = (s32)((var_a1 << 0x18) |
                                    ((var_a2 & 0xFF) << 0x10) |
                                    ((var_a3 & 0xFF) << 8) |
                                    (((((var_t0 - temp_t3) * var_a0) >> 0x10) +
                                      temp_t3) & 0xFF));
                command = *dList;
                *dList = command + 1;
                command->w0 = (s32)(((arg3 & 0x3FF) << 0xE) |
                                    0xF6000000 |
                                    ((temp_v1_2 & 0x3FF) * 4));
                command->w1 = (s32)(sp44 | ((var_s1 & 0x3FF) * 4));
                command = *dList;
                var_s1 = temp_v1_2;
                *dList = command + 1;
                command->w1 = 0;
                command->w0 = 0xE7000000;
            } while (temp_v1_2 != arg4);
        }
        /* Adapted to this TU's top-level prototype; the target call site
           passes only the display-list pointer. */
        func_80034920(dList, NULL, NULL);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/fxSPDPRipple.s")
#endif
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
 * PROVENANCE: the descending loop skeleton is adapted from Jet Force
 * Gemini's public fx.c context; Mickey's target establishes the expressions.
 */
/* Workbench: structure-mismatch, 16 differing words, first mismatch +0x10. */
/* Candidate shape: 28 instructions with no frame delta; improved, not exact. */
/* Remaining gap: callback/trap and loop-counter register webs plus schedule. */
void func_8004ACC4(void) {
    void **value0;
    void **value1;
    u8 *available;
    FxTextureCallback trap;
    FxTextureCallback *callback;
    s32 i;
    s32 matches;

    D_800D60A8 = 0;
    i = 3;
    trap = (FxTextureCallback) TrapDanglingJump;
    value0 = &D_800D60BC;
    value1 = &D_800D60CC;
    available = &D_800D60D3;
    callback = &D_8007D488;
    do {
        matches = trap == *callback;
        *value0 = 0;
        *value1 = 0;
        *available = matches;
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
/* Workbench verdict: structure-mismatch, 19 differing words, first mismatch +0x60. */
/* Candidate shape: 96 instructions/frame -0x40; three store/branch structural words remain, not shape-exact. */
/* Remaining gap: second-allocation store scheduling and stack homes; five register residuals remain. */
#ifdef NON_MATCHING
extern void *func_8002B280(s32 size, s32 tag);

void func_8004ADE8(s32 index, FxConeTextureInfo *texture) {
    s32 offset;
    s32 i;
    s8 *first;
    s8 *second;

    index--;
    offset = index * 4;
    D_800D60A8 |= 1 << index;
    D_800D6098[index] = (s32)texture;
    if (D_800D60B0[index] == 0) {
        first = func_8002B280(texture->width * texture->height, 0x87);
        D_800D60B0[index] = first;
        second = func_8002B280(texture->width * texture->height, 0x87);
        D_800D60C0[index] = second;
        if (D_800D60B0[index] == 0 || D_800D60C0[index] == 0) {
            D_800D60B0[index] = 0;
            D_800D60C0[index] = 0;
            return;
        }
        i = texture->width * texture->height;
        while (i--) {
            *first = 0;
            first++;
            *second = 0;
            second++;
        }
        func_800320F0((s32)&D_8007D47C[index]);
        if (D_8007D47C[index] != 0) {
            D_8007D47C[index](index, D_800D6098[index], 1);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004ADE8.s")
#endif
/* Workbench: structure-mismatch, 26 differing words, first mismatch +0x10. */
/* Candidate shape: 52 instructions/frame -0x38; D_800D60C0 is per-iteration, not exact. */
/* Remaining gap: saved-register order and loop-delay schedule; 18 structural words remain. */
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
            value1 = &D_800D60C0[i];
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
