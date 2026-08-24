/*
 * Resident lights -- ROM 0x19310-0x1AE60 (VRAM 0x80018710).
 *
 * The boundary and function comparison map are documented in
 * docs/modules.md section 3.4. The preceding four-function shadows TU uses
 * odd single-precision FP registers and remains assembly by section 6.2;
 * none of the functions in this TU uses an odd FP register.
 *
 * PROVENANCE -- comparison names, declarations and future starting bodies
 * for this TU come from Jet Force Gemini's public decomp, src/lights.c and
 * src/lights.h. They are permitted published-decomp material under
 * docs/CLEANROOM.md. Mickey's own bytes decide every body and name adoption;
 * the unresolved functions below therefore retain their Mickey func_ names.
 *
 * Flags: -O2 -mips2 -32 (the resident game-code flag group).
 */

#include "PR/ultratypes.h"
#include "game/lights.h"

extern void initColourCycle(s32 arg0, s32 arg1);
extern s32 D_80079494;
extern void **D_80079498;

#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80018710.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001879C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_800188CC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001897C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80018BB4.s")
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void turnLightOff(UnkLight *light) {
    light->unk3 &= ~1;
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void turnLightOn(UnkLight *light) {
    light->unk3 |= 1;
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void toggleLight(UnkLight *light) {
    light->unk3 ^= 1;
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void changeLightColour(UnkLight *light, u8 red, u8 green, u8 blue) {
    light->red = red;
    light->green = green;
    light->blue = blue;
    light->unk2 |= 2;
    light->unk54 = 0;
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void changeLightColourCycle(s32 arg0, s32 arg1) {
    initColourCycle(arg0 + 0x48, arg1);
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void changeLightIntensity(UnkLight *light, u8 intensity) {
    light->unk43 = intensity;
    light->unk44 = intensity;
    if (light->unk6C != 0) {
        light->unk2 |= 2;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80018E7C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80018F08.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001923C.s")
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void **lightGetLights(s32 *count) {
    *count = D_80079494;
    return D_80079498;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019358.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019494.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001953C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019934.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019A24.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019AB8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019D98.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019DE8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019EE4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019F7C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001A008.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001A154.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001A23C.s")
