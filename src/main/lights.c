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
 * Flags: -O2 -mips2 -32 -Wab,-r4300_mul (measured on the FP helpers).
 */

#include "PR/ultratypes.h"
#include "game/lights.h"

extern void initColourCycle();
extern void func_800188CC(UnkLight *light);
extern void func_80018F08(UnkLight *light, s32 updateRate);
extern u8 *func_8002679C(void);
extern s32 D_80079490;
extern s32 D_80079494;
extern void **D_80079498;

#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80018710.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001879C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_800188CC.s")
/* PROVENANCE: adapted from JFG's public decomp comparison and Mickey's own assembly. */
UnkLight *addRomdefLight(s32 arg0, RomdefLight *entry) {
    UnkLight *light;
    u8 *levelData;

    light = NULL;
    if (D_80079494 < D_80079490) {
        light = D_80079498[D_80079494];
        D_80079494++;
        light->unk0 = (entry->modeAndType >> 4) & 0xF;
        light->unk1 = entry->modeAndType & 0xF;
        if ((entry->flags & 0xE0) == 0x40) {
            light->unk0 = 3;
        }
        light->unk2 = 7;
        light->unk3 = ~entry->enabledFlags & 0x61;
        light->home = 0;
        light->index = -1;
        light->directionX = 0.0f;
        light->directionY = 0.0f;
        light->directionZ = 0.0f;
        light->owner = NULL;
        light->x = entry->x;
        light->y = entry->y;
        light->z = entry->z;
        light->radius = entry->radius;
        light->radius2 = light->radius;
        light->radius3 = light->radius;
        if (light->unk0 == 2) {
            light->radius2 = entry->radius2;
        }
        light->radiusSquare = light->radius * light->radius;
        light->radiusInverse = 1.0f / light->radius;
        light->red = entry->red;
        light->green = entry->green;
        light->blue = entry->blue;
        light->unk43 = entry->intensity;
        light->unk44 = (u32) entry->intensity;
        light->unk54 = 0;
        if (entry->colourCycleIndex < 7) {
            levelData = func_8002679C();
            if (*(s16 *)(levelData + 0x94 + (entry->colourCycleIndex * 2)) != -1) {
                initColourCycle(light->colourCycle, *(s16 *)(levelData + 0x94 + (entry->colourCycleIndex * 2)), entry);
            }
        }
        light->value58 = entry->value58;
        light->value5C = entry->value5C;
        light->value5A = entry->value5A;
        light->value5E = entry->value5E;
        func_800188CC(light);
        func_80018F08(light, 0);
    }
    return light;
}
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
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
f32 lightDirectionCalc(f32 arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5, f32 arg6) {
    f32 temp_f0;
    f32 var_f2;

    if (arg6 > 0.0f) {
        temp_f0 = 1.0f / arg6;
        var_f2 = (arg3 * temp_f0 * arg0) + (arg4 * temp_f0 * arg1) + (arg5 * temp_f0 * arg2);
        if (var_f2 < 0.0f) {
            var_f2 = 0.0f;
        }
    } else {
        var_f2 = 1.0f;
    }
    return var_f2;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019AB8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019D98.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019DE8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019EE4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019F7C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001A008.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001A154.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001A23C.s")
