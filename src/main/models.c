/*
 * Model animation loading and matrix generation -- ROM 0x5B300-0x5C310
 * (VRAM 0x8005A700-0x8005B710).
 *
 * The TU name is Tier B/D: its callers and data flow establish animation-table
 * loading, reference-counted animation allocation, frame selection and model
 * matrix construction. JFG models.c supplies the nearest non-exact skeletons
 * for the loader/free pair. camConvertMatrixList alone is Tier A against JFG
 * camera.c; that isolated helper does not turn the full range into camera.c.
 *
 * PROVENANCE: JFG's permitted src/models.c, models.h and camera.c were read
 * for names, layouts and comparison. No body is adapted from them here; all
 * functions remain Mickey's generated assembly.
 */

#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8005A700.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8005A764.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8005A770.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8005A7A0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8005A948.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8005AAC0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/camConvertMatrixList.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8005ABA8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8005AD64.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8005AF14.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8005B644.s")
