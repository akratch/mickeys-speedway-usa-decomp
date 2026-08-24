#include "PR/ultratypes.h"

typedef struct Overlay47Entry {
    u8 pad00[0x24];
    void *handle;
    u8 pad28[0x0C];
} Overlay47Entry;

extern void *D_30C;
extern void *D_314;
extern void *D_318;
extern void *D_31C;
extern void *D_320;
extern Overlay47Entry D_0_entries[];
extern Overlay47Entry D_D0_entries[];
extern void *D_38C[];
extern void *D_3B4[];
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

extern void func_overlay_045_F0000270_188C6C8(void *handle);
extern void func_80006EA0(void *handle);
extern void func_80039A40(void *arg);

void func_overlay_047_F00009D0_18917E8(void) {
    Overlay47Entry *entry;
    void **slot;

    func_overlay_045_F0000270_188C6C8(D_30C);
    func_overlay_045_F0000270_188C6C8(D_314);
    func_overlay_045_F0000270_188C6C8(D_318);
    func_overlay_045_F0000270_188C6C8(D_31C);
    func_overlay_045_F0000270_188C6C8(D_320);

    entry = D_0_entries;
    do {
        if (entry->handle != NULL) {
            func_80006EA0(entry->handle);
            entry->handle = NULL;
        }
        entry++;
    } while (entry < D_D0_entries);

    slot = D_38C;
    do {
        if (*slot != NULL) {
            func_80006EA0(*slot);
            *slot = NULL;
        }
        slot++;
    } while (slot < D_3B4);

    func_80039A40(D_358);
    D_status0 = 0;
    if (D_flag2A != 0) {
        D_status1 = 1;
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
