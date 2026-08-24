/*
 * Resident formatting and debug-text system -- ROM 0x43470-0x45760.
 *
 * PROVENANCE: The TU correspondence and names are adapted from Jet Force
 * Gemini's public src/diprint.c and its built diprint.c.o. Diddy Kong Racing's
 * public built unused_string.c.o/printf.c.o independently identify strcpy,
 * memset and sprintf byte-for-byte, and its public src/printf.c supplies the
 * debug_text_width name for the one routine absent from JFG. Mickey's bytes,
 * call graph and strings decide the mapping. No external body has been adapted
 * while these entries remain GLOBAL_ASM.
 */

#include "PR/ultratypes.h"

#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/strcpy.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/memset.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/_itoa.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/sprintfSetSpacingCodes.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/sprintf.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/vsprintf.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/diPrintfInit.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/diPrintf.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/diPrintfAll.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/diPrintfSetCol.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/diPrintfSetBG.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/diPrintfSetXY.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/debug_text_width.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/debug_text_parse.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/debug_text_background.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/debug_text_character.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/debug_text_bounds.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/debug_text_origin.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/debug_text_newline.s")
