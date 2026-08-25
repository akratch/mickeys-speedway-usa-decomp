#include "PR/ultratypes.h"

typedef struct Overlay47Entry {
    u8 pad00[0x24];
    void *handle;
    u8 pad28[0x0C];
} Overlay47Entry;

extern Overlay47Entry D_D0;
extern Overlay47Entry D_0_entries;
extern void *D_30C;
extern void *D_314;
extern void *D_318;
extern void *D_31C;
extern void *D_320;
extern void *D_38C;
extern void *D_3B4;
extern u8 D_358[];
extern u8 D_status0;
extern u8 D_status1;
extern u8 D_status2;
extern u8 D_status3;
extern u8 D_status4;
extern s8 D_flag2A;
extern s8 D_flag5E;
extern s8 D_flag92;
extern s8 D_flagC6;

extern void func_overlay_047_F0000000_1890E18(void *arg);

/*
 * Plateau (2026-08-25, renewed cap): the canonical -O2 -mips2 candidate is
 * size-exact with a 10-word masked instruction residual, first at +0x54. An
 * explicit boolean second-loop bound improved the prior 24-word residual, but
 * the first/second boundary low halves and the overlay-relative data addends
 * still need the original same-TU address model; the object has 30 relocation
 * metadata mismatches. All 119 flag variants and a bounded 10-minute permuter
 * batch were exhausted. Static calls use the extracted offset-zero carrier;
 * the shipped overlay relocation ledger retains their runtime identities.
 */
#ifdef NON_MATCHING
void func_overlay_047_F00009D0_18917E8(void) {
    Overlay47Entry *entry;
    void **slot;

    func_overlay_047_F0000000_1890E18(D_30C);
    func_overlay_047_F0000000_1890E18(D_314);
    func_overlay_047_F0000000_1890E18(D_318);
    func_overlay_047_F0000000_1890E18(D_31C);
    func_overlay_047_F0000000_1890E18(D_320);

    entry = &D_0_entries;
    do {
        if (entry->handle != NULL) {
            func_overlay_047_F0000000_1890E18(entry->handle);
            entry->handle = NULL;
        }
        entry++;
    } while (entry < &D_D0);

    slot = &D_38C;
    do {
        if (*slot != NULL) {
            func_overlay_047_F0000000_1890E18(*slot);
            *slot = NULL;
        }
        slot++;
    } while ((slot < &D_3B4) != 0);

    func_overlay_047_F0000000_1890E18(D_358);
    D_status0 = 0;
    if (D_flag2A != 0) {
        D_status1 = 1U;
    }
    if (D_flag5E != 0) {
        D_status2 = D_status0 | 2;
    }
    if (D_flag92 != 0) {
        D_status3 = D_status0 | 4;
    }
    if (D_flagC6 != 0) {
        D_status4 = D_status0 | 8;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o047/overlay47ReleaseResources/func_overlay_047_F00009D0_18917E8.s")
#endif
