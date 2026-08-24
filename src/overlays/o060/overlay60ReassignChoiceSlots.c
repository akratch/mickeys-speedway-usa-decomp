#include "PR/ultratypes.h"

typedef struct Overlay60Choice {
    u8 pad0[0x28];
    s16 slot;
    u8 pad2A;
    s8 active;
    u8 pad2C[8];
} Overlay60Choice;

/* Separate pass aliases preserve the two address-materialization lifetimes. */
extern Overlay60Choice gOverlay60ChoicesPass1[];
extern Overlay60Choice gOverlay60ChoicesPass2[];
extern Overlay60Choice gOverlay60ChoicesPass1End[];
extern Overlay60Choice gOverlay60ChoicesPass2End[];

/* The pinned DKR v77/v80 and JFG object scans have no donor for this owner. */
#ifdef NON_MATCHING
void overlay60ReassignChoiceSlots(void) {
    u8 available[20];
    u8 *availableCursor;
    Overlay60Choice *choice;

    availableCursor = available;
    do {
        *availableCursor++ = 1;
    } while (availableCursor < available + 10);

    choice = gOverlay60ChoicesPass1;
    do {
        if (choice->active != 0) {
            available[choice->slot & 0xF] = 0;
        }
        choice++;
    } while (choice < gOverlay60ChoicesPass1End);

    choice = gOverlay60ChoicesPass2;
    do {
        if ((choice->active != 0) && (choice->slot >= 6)) {
            choice->slot = 0;
            while (available[choice->slot] == 0) {
                choice->slot++;
            }
            available[choice->slot] = 0;
        }
        choice++;
    } while (choice != gOverlay60ChoicesPass2End);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o060/overlay60ReassignChoiceSlots/func_overlay_060_F0003488_18BD260.s")
#endif
