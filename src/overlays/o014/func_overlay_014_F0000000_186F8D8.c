#include "PR/ultratypes.h"
#ifndef INVALID_STORAGE
#define INVALID_STORAGE
#endif
typedef struct Slot { s32 key; void *value; } Slot;
extern Slot gOverlay14ValueSlots28[], gOverlay14ValueSlotsEnd128[];
extern s32 gOverlay14StateC0, gOverlay14StateC4, gOverlay14StateC8;
extern s32 gOverlay14PendingValueD0, gOverlay14PendingModeD4;
extern s32 gOverlay14TransitionD8, gOverlay14CursorDC, gOverlay14PointerE0;
extern s32 gOverlay14ValueCountE8, gOverlay14CommandCountEC;
extern void *gOverlay14AssetF0, *gOverlay14OffsetsF4;
extern s32 gOverlay14ResultF8, gOverlay14ResultFC;
extern s32 gOverlay14Current104, gOverlay14Next108, gOverlay14Context10C, gOverlay14Field114;
extern void overlay14InitializeReloc(void);
extern void overlay14FreeReloc(void *ptr);

/* Workbench: allocation-mismatch, 79 instructions/frame -40; 4 words, first +0x70.
 * Levers tried: direct-global count, comma setup, late invalid reassertion, flags and line probe.
 * Remains: target address setup orders slot/count/end differently; relocation identities remain diagnostic. */
#ifdef NON_MATCHING
void func_overlay_014_F0000000_186F8D8(void) {
    Slot *slot;
    Slot *end;
    INVALID_STORAGE s32 invalid;
    overlay14InitializeReloc();
    invalid = -1;
    gOverlay14Current104 = invalid; gOverlay14Next108 = invalid;
    gOverlay14Context10C = 0; gOverlay14Field114 = 0;
    if (gOverlay14AssetF0) overlay14FreeReloc(gOverlay14AssetF0);
    if (gOverlay14OffsetsF4) overlay14FreeReloc(gOverlay14OffsetsF4);
    slot = gOverlay14ValueSlots28, end = gOverlay14ValueSlotsEnd128;
    do {
        if ((gOverlay14ValueCountE8 != 0) && slot->value) overlay14FreeReloc(slot->value);
        slot++;
        slot[-1].value = 0;
        slot[-1].key = invalid;
    } while (slot != end);
#ifdef EARLY_PENDING
    gOverlay14PendingValueD0 = invalid; gOverlay14PendingModeD4 = invalid;
#endif
#ifndef COUNT_CLEAR
    gOverlay14ValueCountE8 = 0;
#else
    COUNT_CLEAR
#endif
    gOverlay14CommandCountEC = 0;
    gOverlay14StateC0 = 0; gOverlay14StateC4 = 0; gOverlay14StateC8 = 0;
#ifndef EARLY_PENDING
    invalid = -1;
    gOverlay14PendingValueD0 = invalid; gOverlay14PendingModeD4 = invalid;
#endif
    gOverlay14TransitionD8 = 0; gOverlay14CursorDC = 0; gOverlay14PointerE0 = 0;
    gOverlay14AssetF0 = 0; gOverlay14OffsetsF4 = 0;
    gOverlay14ResultF8 = 0; gOverlay14ResultFC = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o014/func_overlay_014_F0000000_186F8D8/func_overlay_014_F0000000_186F8D8.s")
#endif
