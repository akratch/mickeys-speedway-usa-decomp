/*
 * Matrix and vector maths -- ROM 0x2B650-0x2BCD0 (VRAM 0x8002AA50).
 *
 * Six float-only functions with no rodata of their own: no jump tables and no
 * float literals, so the whole subsegment can become C without the .rodata
 * split that the rest of the static segment still needs. That is why this is
 * the first non-linker game TU here.
 *
 * Flags: -O2 -mips2 -32, the same src/main/ rule main/runlink.c established.
 *
 * NOTHING IN THIS FILE MATCHES, and the reason is not the source -- it is the
 * compiler. See the NONMATCHING-notes below.
 */

#include "PR/ultratypes.h"
#include "game/math.h"

#pragma GLOBAL_ASM("asm/nonmatchings/main/matrix/func_8002AA50.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/matrix/func_8002AB78.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/matrix/func_8002AC84.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/matrix/func_8002AE10.s")
/*
 * NONMATCHING-notes for this whole file: the toolchain cannot emit the ROM's
 * floating-point register allocation.
 *
 * The ROM's float code uses ODD single-precision FP registers -- $f5, $f7,
 * $f9, $f11 and $f17 all appear in the 53 instructions at 0x8002AF6C alone.
 * The IDO 5.3 in tools/ido/ never emits one. That is measured, not inferred:
 * a four-term dot-product test case compiled at every combination of
 * -mips1/-mips2/-mips3 with -O1/-O2/-O3 produced zero odd single-precision FP
 * registers in all nine builds. Every candidate below therefore comes out
 * using $f0/$f2/$f4/... and differs from the ROM in almost every FP register
 * name, with the instruction schedule following from that.
 *
 * ROM-wide the evidence is 1727 odd FP register operands across 9 of the
 * static segment's asm files, out of 24984 FP operands in total. The GNU
 * assembler notices too: assembling asm/18FF0.s prints "Warning: float
 * register should be even" once per occurrence.
 *
 * The most likely explanation is that Mickey's game code was built with IDO
 * 7.1 rather than 5.3, whose allocator does use the odd single-precision
 * registers. That is consistent with everything else measured in this task:
 * the integer-only functions in main/runlink.c match 5.3 byte for byte,
 * because for straight-line integer code the two versions agree, and not one
 * matched function so far touches the FPU. It is a hypothesis with one clean
 * test -- build either function below with IDO 7.1 -- and that test needs a
 * compiler this repository does not vendor.
 *
 * What IS believed correct is the C. Both bodies were derived from the ROM's
 * own multiply/add chains and reproduce the arithmetic exactly. The residual
 * measured with `decomp-workbench diagnose-dumps` is FP register names and
 * their consequences: MatrixMultiplyVec4 words=47 of which fp=46, after
 * hoisting the source components into locals to stop the dst-aliasing reloads
 * (words=63 before that). Do not rewrite these from scratch; re-test them the
 * day a 7.1 lands.
 */
#ifdef NON_MATCHING
/*
 * dst = m * src, treating src as a column vector:
 *   dst[i] = src[0]*m[i][0] + src[1]*m[i][1] + src[2]*m[i][2] + src[3]*m[i][3]
 *
 * The four src components are loaded once at the top and reused across all
 * four output rows; that is IDO's own common-subexpression elimination, not
 * something the source has to spell out.
 */
void MatrixMultiplyVec4(MtxF m, f32 *src, f32 *dst) {
    f32 x;
    f32 y;
    f32 z;
    f32 w;

    x = src[0];
    y = src[1];
    z = src[2];
    w = src[3];
    dst[0] = x * m[0][0] + y * m[0][1] + z * m[0][2] + w * m[0][3];
    dst[1] = x * m[1][0] + y * m[1][1] + z * m[1][2] + w * m[1][3];
    dst[2] = x * m[2][0] + y * m[2][1] + z * m[2][2] + w * m[2][3];
    dst[3] = x * m[3][0] + y * m[3][1] + z * m[3][2] + w * m[3][3];
}
/*
 * Rotate a direction by the matrix's upper 3x3, the other way round from
 * MatrixMultiplyVec4: the input scales whole *rows* rather than being dotted
 * with them, and the translation row is ignored.
 *
 *   *dstX = x*m[0][0] + y*m[1][0] + z*m[2][0]   (and likewise for Y, Z)
 *
 * The three scalars arrive in a1/a2/a3 as integers and are moved across with
 * mtc1, which is just o32: because the first argument is a pointer, no
 * floating-point argument register is used at all. The three destinations are
 * the stack arguments at 0x10/0x14/0x18(sp).
 */
void MatrixRotateVec3(MtxF m, f32 x, f32 y, f32 z, f32 *dstX, f32 *dstY, f32 *dstZ) {
    *dstX = x * m[0][0] + y * m[1][0] + z * m[2][0];
    *dstY = x * m[0][1] + y * m[1][1] + z * m[2][1];
    *dstZ = x * m[0][2] + y * m[1][2] + z * m[2][2];
}
#endif

#pragma GLOBAL_ASM("asm/nonmatchings/main/matrix/func_8002AF6C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/matrix/func_8002B040.s")
