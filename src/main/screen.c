/*
 * Compressed screen loading and drawing -- ROM 0x2F0D0-0x2F400.
 *
 * PROVENANCE -- the TU identity and names are adapted from Jet Force Gemini's
 * public decompilation, src/screen.c. Mickey's load/decompress and draw/VI
 * call graphs establish the two-function correspondence. The bodies remain
 * Mickey GLOBAL_ASM.
 */

#include "PR/ultratypes.h"

#pragma GLOBAL_ASM("asm/nonmatchings/main/screen/screenLoad.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/screen/screenDraw.s")
