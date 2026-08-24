/*
 * Resident front-end menu -- ROM 0x39350-0x3B1A0 (VRAM 0x80038750).
 *
 * The translation-unit identity and function crosswalk come from Jet Force
 * Gemini's public decompilation of the same Rare engine. The boundary evidence
 * and provenance are recorded in docs/modules.md section 3.4. Functions stay
 * under GLOBAL_ASM until their C compiles to Mickey's bytes exactly.
 *
 * Flags: -O2 -mips2 -32, via the shared src/main rule.
 */

#include "PR/ultratypes.h"

#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80038750.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80038878.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_800389CC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80038BC4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80038DAC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80038E10.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80038E1C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039278.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039380.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/frontDrawRectangle.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_800395D4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003968C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039720.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039A40.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039A9C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039B88.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039BE4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039D50.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/setupFrontEndObject.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039E34.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A24C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A260.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/frontGetScreenMode.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A2C8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A348.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A35C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/frontGetLevelScreenMode.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A3D0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/frontSetWideAdjust.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A408.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A41C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A47C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A488.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A4C4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A4D0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A50C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A520.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A544.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A550.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A55C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A590.s")
