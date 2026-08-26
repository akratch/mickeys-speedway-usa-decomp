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

/* Plateau (near-miss p5): workbench mixed(constant:9, schedule:2, register:7), 10-word masked floor (18 raw) at 88 instructions/frame -0x20.
 * Levers: end-pointer/boolean forms and constant audit; data aggregate and relocation identities remain.
 * Remains: overlay aggregate ownership and relocation binding; assembly fallback stays canonical. */
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
