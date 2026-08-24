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
extern s8 D_800CB498[];
extern s16 D_800CB49C[];
extern s16 D_800CB4A2[];

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
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8001F45C.s")
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8001FBCC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8001FC50.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_80020184.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_80020278.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_800203E0.s")
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_80021010.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8002109C.s")
