/*
 * Resident formatting and debug-text system -- ROM 0x43470-0x45760.
 *
 * PROVENANCE: The TU correspondence and names are adapted from Jet Force
 * Gemini's public src/diprint.c and its built diprint.c.o. Diddy Kong Racing's
 * public built unused_string.c.o/printf.c.o independently identify strcpy,
 * memset and sprintf byte-for-byte, and its public src/printf.c supplies the
 * debug_text_width name for the one routine absent from JFG. Mickey's bytes,
 * call graph and strings decide the mapping. Adapted bodies carry a
 * point-of-use PROVENANCE note.
 */

#include "PR/ultratypes.h"

/* PROVENANCE: body adapted from DKR src/unused_string.c:strcpy. */
char *strcpy(char *src, const char *dest) {
    char *ret = src;

    while ((*src++ = *dest++) != '\0') {}
    return ret;
}
/* PROVENANCE: body adapted from DKR src/unused_string.c:memset. */
void *memset(void *s, int c, size_t n) {
    unsigned char *ret = s;

    while (n-- > 0) {
        *ret++ = c;
    }
    return s;
}
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
