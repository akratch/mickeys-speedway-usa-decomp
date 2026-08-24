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

extern void func_800347A0(s32 linked);
extern void mmFree(void *ptr);
extern FxFlags D_800D5F5A[];
extern FxStatus D_800D5F59[];

#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80046E70.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80046EC4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004707C.s")
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/fxInit.s")
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004989C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_800498FC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80049A8C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80049B14.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80049E4C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004A0F0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004A10C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004A380.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004A4B0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004A51C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/fxSPDPRipple.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/fxQueueScreenEffect.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004A9CC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/fxScreenEffect.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004ACC4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004AD34.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004ADE8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_8004AF68.s")
