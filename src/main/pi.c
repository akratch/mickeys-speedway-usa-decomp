/*
 * Cartridge asset DMA -- ROM 0x2ECA0-0x2F0D0.
 *
 * PROVENANCE -- the TU identity and names are adapted from Jet Force Gemini's
 * public decompilation, src/pi.c. Mickey's seven-function order and PI/DMA
 * call graph establish the correspondence. The bodies are still Mickey
 * GLOBAL_ASM, not borrowed JFG C.
 */

#include "PR/ultratypes.h"

#pragma GLOBAL_ASM("asm/nonmatchings/main/pi/piInit.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/pi/piRomLoad.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/pi/piRomLoadCompressed.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/pi/piRomLoadSection.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/pi/piRomGetSectionPtr.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/pi/piRomGetFileSize.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/pi/romCopy.s")
