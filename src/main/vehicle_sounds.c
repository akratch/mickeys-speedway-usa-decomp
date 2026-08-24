/*
 * Racer/vehicle sound updater -- ROM 0x58E50-0x59B90
 * (VRAM 0x80058250-0x80058F90).
 *
 * The name is a Tier B/D description, not a borrowed JFG TU name. The large
 * updater walks active racer objects, owns two positional sound handles on
 * each one, and updates their position, volume and pitch from speed and
 * camera distance. No function in this boundary has an exact JFG skeleton
 * hit, so the existing splat boundary is intentionally not presented as a
 * measured cross-title file boundary.
 *
 * PROVENANCE: JFG's permitted src/audio_manager_36D0.c and audio.h were read
 * to identify the shared positional-sound API. No body is adapted from them;
 * all four functions remain Mickey's generated assembly.
 */

#pragma GLOBAL_ASM("asm/nonmatchings/main/vehicle_sounds/func_80058250.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/vehicle_sounds/func_800582A8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/vehicle_sounds/func_8005830C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/vehicle_sounds/func_80058EF4.s")
