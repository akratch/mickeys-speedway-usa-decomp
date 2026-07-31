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
 * At the project's flags the IDO 5.3 in tools/ido/ emits none: a four-term
 * dot-product test case compiled at every combination of -mips1/-mips2/-mips3
 * with -O1/-O2/-O3 produced zero odd single-precision FP registers in all nine
 * builds. Every candidate below therefore comes out using $f0/$f2/$f4/... and
 * differs from the ROM in almost every FP register name, with the instruction
 * schedule following from that. (It CAN emit them, under
 * -Wc,-mips3 -Wc,-fp32regs -- just not the ROM's ones. See below.)
 *
 * ROM-wide the evidence is 1727 odd FP register operands across 9 of the
 * static segment's asm files, out of 24984 FP operands in total. The GNU
 * assembler notices too: assembling asm/18FF0.s prints "Warning: float
 * register should be even" once per occurrence.
 *
 * WHAT IS SETTLED. No SGI IDO build can produce the ROM's allocation, and the
 * search for one is closed -- see docs/modules.md section 6.2 for the
 * mechanism. In short: this IDO's ugen does have SGI's -fp32regs, reachable as
 * `-Wc,-mips3 -Wc,-fp32regs` (the -Wo, form earlier sweeps used hands the flag
 * to uopt, which drops it), and with it MatrixMultiplyVec4 comes out at the
 * ROM's exact 53 instructions with odd registers -- but allocated across all
 * 32, including argument, return and unsaved callee-saved odd halves, where
 * the ROM confines itself to $f4-$f11 and $f16-$f18. That is structural: in
 * the matched decompilation of real IDO 7.1's ugen, -fp32regs is an
 * unconditional loop freeing all 16 odd registers, while the reservation logic
 * that protects live ones walks even register numbers only. Six IDO/MIPSpro
 * versions, 4.1 through 7.4.4, all choose the same all-32 set.
 *
 * So (a) is dead. Two explanations remain, and they are not exclusive:
 *
 *   (b) HAND-WRITTEN ASSEMBLY. Odd-register use is exactly what a human
 *       writing MIPS by hand produces, because the even-only constraint is a
 *       compiler convention rather than a hardware one here. This is NOT
 *       ruled out for these two functions and it is the cheapest thing to
 *       check first, because it would make them un-decompilable by design
 *       rather than blocked on a compiler. The per-file odd-operand density
 *       is NOT uniform, which is what a single-compiler story would predict:
 *
 *         61.3%  796/1299  asm/59DB0.s   <- rule this one out first
 *         50.0%   38/76    asm/4FC30.s
 *         43.5%   37/85    asm/59BF0.s
 *         40.1%  254/633   asm/nonmatchings/main/matrix/   <- THIS FILE
 *         24.2%  266/1100  asm/18FF0.s
 *         18.9%  252/1335  asm/2A250.s
 *         12.1%   44/365   asm/3B480.s
 *          2.9%    6/209   asm/33FA0.s
 *          2.7%   34/1257  asm/16140.s
 *
 *       A spread from 61% to 2.7% across nine files looks more like a mix of
 *       origins than like one allocator applied uniformly. Note where this
 *       file sits: 40%. That is high enough that hand-written assembly is a
 *       live explanation for the two functions below specifically, not just
 *       for the ROM in general -- and if it is the right one, they are
 *       un-decompilable by design and no compiler will fix them.
 *
 *   (c) A NON-IDO COMPILER for some or all of the game code. No positive
 *       evidence anywhere points at one; it is listed because nothing rules
 *       it out, not because anything suggests it.
 *
 * DO NOT re-sweep compiler flags or IDO versions for this file. The mechanism
 * above says in advance that every such sweep fails. What would reopen it is
 * named in docs/modules.md section 6.2, and neither item is something to wait
 * for.
 *
 * What IS believed correct is the C. Both bodies were derived from the ROM's
 * own multiply/add chains and reproduce the arithmetic exactly, and
 * MatrixMultiplyVec4 under -Wc,-mips3 -Wc,-fp32regs reproduces the ROM's
 * instruction count and kinds exactly, differing only in register names. Do
 * not rewrite them from scratch.
 *
 * func_8002B040 (MatrixRotateVec3) does not belong to this discussion at all:
 * it uses no odd registers. Its blocker is ugen's expression scheduling, and
 * with the stock toolchain at uopt -O3 -- reachable only through
 * tools/ido-phases.py, since `cc -O3` dies in uld -- and the m[i][j]*x operand
 * order it is 16 of 34 instructions from a match, with two systematic
 * residuals: the final add.s operand order (the ROM writes the accumulator
 * first) and the third mul.s's placement in rows 2 and 3.
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
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/matrix/func_8002AF6C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/matrix/func_8002B040.s")
#endif
