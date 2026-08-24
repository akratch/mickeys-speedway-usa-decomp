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

extern void func_800347A0(s32 linked);
extern void func_8004ACC4(s32 index);
extern void mmFree(void *ptr);
extern FxFlags D_800D5F5A[];
extern FxStatus D_800D5F59[];
extern FxRecord D_800D5F58[];
extern s32 D_800D5F50;
extern s32 D_800D6038[];
extern s32 D_800D6040;
extern s32 D_8007D478;
extern FxScreenEffect D_800D6048[];
extern void fxScreenEffect(s32 arg0, s32 type, s32 value4, s32 value6,
                           s32 value8, s32 valueA, s32 valueC, s32 valueE,
                           s32 value10);

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
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_800475E8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_800479D4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80047CD8.s")
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004978C.s")
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004A380.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004A4B0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004A51C.s")
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004ACC4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004AD34.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004ADE8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004AF68.s")
