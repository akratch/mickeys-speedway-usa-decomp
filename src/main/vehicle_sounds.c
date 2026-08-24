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

#include "PR/ultratypes.h"

typedef struct VehicleSoundSlot {
    s32 handle;
    s32 pad4;
    f32 value;
    s32 secondaryHandle;
} VehicleSoundSlot;

extern s32 D_800D78B0;
extern s32 D_800D78BC;
extern s32 D_800D78C0;
extern s32 D_800D78CC;
extern s32 D_800D78D0;
extern s32 D_800D78DC;
extern s32 D_800D78E0;
extern s32 D_800D78EC;
extern s32 D_800D78F0;
extern f32 D_80084318;

void func_800031E8(s32 soundHandle);

#ifdef NON_MATCHING
/*
 * Plateau: the direct initialization is semantically exact, but separate
 * extern symbols emit four extra address materializations: 26 instructions
 * against the target's 22, first diverging at +0x8. The array-shaped spelling
 * removes too many instead. All 119 flag-lattice combinations retain the
 * target's otherwise-unusual two-address-materialization-per-slot schedule.
 */
void func_80058250(void) {
    D_800D78B0 = 0;
    D_800D78BC = 0;
    *((f32 *)&D_800D78BC - 1) = 0.0f;
    D_800D78C0 = 0;
    D_800D78CC = 0;
    *((f32 *)&D_800D78CC - 1) = 0.0f;
    D_800D78D0 = 0;
    D_800D78DC = 0;
    *((f32 *)&D_800D78DC - 1) = 0.0f;
    D_800D78E0 = 0;
    D_800D78EC = 0;
    *((f32 *)&D_800D78EC - 1) = 0.0f;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/vehicle_sounds/func_80058250.s")
#endif

void func_800582A8(void) {
    VehicleSoundSlot *slot = (VehicleSoundSlot *)&D_800D78B0, *end = (VehicleSoundSlot *)&D_800D78F0;
    do {
        if (slot->handle != 0) {
            func_800031E8(slot->handle);
        }
        if (slot->secondaryHandle != 0) {
            slot->secondaryHandle = 0;
        }
        slot++;
    } while (slot != end);
}

/*
 * Plateau: the 0xBE8-byte racer/audio update depends on several still-unknown
 * vehicle, listener and sound-state layouts. Its m2c draft exposes nested
 * field walks and calls but not trustworthy types, and the permitted JFG
 * audio-manager sources have no body with the same CFG. Inventing the missing
 * layouts would not be a semantics-preserving matching attempt, so the retail
 * body remains canonical pending a typed caller/field census.
 */
#pragma GLOBAL_ASM("asm/nonmatchings/main/vehicle_sounds/func_8005830C.s")

#ifdef NON_MATCHING
/*
 * Plateau: the best source has the target's arithmetic and relocation
 * surface, but emits 36 instructions against 39, first diverging at +0x4,
 * and swaps the two long-lived FP webs. All 119 flag combinations were swept;
 * `-Wab,-r4300_mul` is best at 13 differing words but is still structural.
 * The bounded permuter is unavailable in this checkout.
 */
f32 func_80058EF4(f32 arg0) {
    f32 one;
    f32 previous;
    f32 term;
    f32 result;
    s32 divisor;

    one = 1.0f;
    previous = -1.0f;
    result = 0.0f;
    divisor = 1;
    arg0 = (arg0 - one) / (one + arg0);
    term = arg0;
    if (D_80084318 < (result - previous)) {
        do {
            previous = result;
            result += term / divisor;
            divisor += 2;
            term *= arg0 * arg0;
        } while (D_80084318 < (result - previous));
    }
    result *= 2;
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/vehicle_sounds/func_80058EF4.s")
#endif
