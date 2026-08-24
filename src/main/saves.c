/*
 * Save-device and rumble support -- ROM 0x2C8C0-0x2ECA0.
 *
 * PROVENANCE -- the translation-unit identity and the descriptive names used
 * below are adapted from Jet Force Gemini's public decompilation, src/saves.c.
 * Mickey's function order, sizes and call graph establish the correspondence;
 * adapted C bodies are identified in docs/modules.md; all remaining functions
 * stay as Mickey GLOBAL_ASM.
 */

#include "PR/ultratypes.h"

extern u8 D_8007A2F8;

#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002BCC0.s")
void rumbleRumbles(s32 value) {
    D_8007A2F8 = value;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/rumbleProcessing.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/rumbleStart.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/rumbleStop.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/rumbleKill.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/rumbleUpdate.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002BF54.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/rumbleTick.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C5F4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C60C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C69C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C70C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C788.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C790.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C79C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packCalculateGameChecksum.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C7EC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C8B4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C94C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002CB18.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002CCE4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002CD6C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packCalculateGlobalFlagsChecksum.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002CE54.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002CF0C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002CF6C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packOpen.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packClose.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packInit.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packIsPresent.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packDirectory.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packDirectoryFree.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packFreeSpace.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packDeleteFile.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packOpenFile.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packReadFile.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packWriteFile.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packFileSize.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/font_codes_to_string.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/string_to_font_codes.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002E020.s")
