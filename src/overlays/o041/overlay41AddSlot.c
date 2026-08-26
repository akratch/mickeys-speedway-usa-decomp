#include "PR/ultratypes.h"

typedef struct Overlay41Slot {
    u32 object;
    u8 color0;
    u8 value1;
    u8 color1;
    u8 value3;
    u8 color2;
    u8 value5;
    u8 color3;
    u8 value7;
    s16 amount;
    s16 previousAmount;
} Overlay41Slot;

extern Overlay41Slot gOverlay41Slots[];
extern f32 D_0[];

/* Workbench: register-permutation; 6/55 words remain, first +0x10.
 * Lever: D_0[0x15] constant-anchor audit and bounded 30m permutation; its best winner was semantically invalid, and a1/v1 plus at/sp stayed crossed.
 * Remains: one downstream forced-color allocation outcome; canonical assembly stays. */
#ifdef NON_MATCHING
void func_overlay_041_F0001650_1888988(void *object, volatile s32 value1,
                                       s32 value3, s32 value5, s32 value7,
                                       f32 amount, s32 alternateColors) {
    register s32 remaining;
    Overlay41Slot *slot;
    u8 *bytes;

    if (object == 0) {
        return;
    }

    slot = gOverlay41Slots;
    remaining = 11;
    do {
        if (slot->object == 0) {
            bytes = object;
            if (alternateColors != 0) {
                slot->object = (u32)object;
                slot->color0 = bytes[0x40];
                slot->color1 = bytes[0x41];
                slot->color2 = bytes[0x42];
                slot->color3 = bytes[0x43];
            } else {
                slot->object = (u32)object & 0x7FFFFFFF;
                slot->color0 = bytes[0x38];
                slot->color1 = bytes[0x39];
                slot->color2 = bytes[0x3A];
                slot->color3 = bytes[0x3B];
            }
            slot->value1 = value1;
            slot->value3 = value3;
            slot->value5 = value5;
            slot->value7 = value7;
            slot->amount = D_0[0x15] * amount;
            slot->previousAmount = slot->amount;
            return;
        }
        slot++;
    } while (remaining--);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o041/overlay41AddSlot/func_overlay_041_F0001650_1888988.s")
#endif
