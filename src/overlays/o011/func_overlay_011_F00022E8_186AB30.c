#include "PR/ultratypes.h"

typedef struct O11StatusSlot {
    u8 mode;
    u8 pad01[7];
    s32 field8;
    u8 pad0C[0x1C];
} O11StatusSlot;

typedef struct O11ObjectSub {
    u8 pad0;
    s8 value1;
} O11ObjectSub;

typedef struct O11Object {
    u8 pad00[0x64];
    O11ObjectSub *sub64;
} O11Object;

extern s8 D_0[];
extern s8 D_0_reload_success[];
extern s8 D_0_reload_failure[];
extern u8 D_menuBase[];
extern s16 D_1B8;
extern s32 D_1BC;
extern s32 D_1C4;
extern void *D_1CC[5];
extern s32 D_204;

extern s32 D_state;
extern u8 D_stateFlag;
extern s8 D_cfgA;
extern s8 D_cfgB;
extern s32 D_lastMode;

extern O11StatusSlot *func_overlay_011_F0000000_1868848(void);
extern void func_overlay_011_F0001058_18698A0(s32 arg0);
extern void func_overlay_011_F0001130_1869978(s32 arg0);
extern void func_overlay_011_F0002A74_186B2BC(void);
extern void func_overlay_011_F0002AD8_186B320(void);
extern void func_overlay_011_F0002BF4_186B43C(void);
extern void func_80000F94(s32 soundId, void *handle);
extern u32 func_8002554C(s32 controller);
extern O11Object *func_80005820(s32 controller);
extern s32 func_8002675C(void);
extern void func_80028374(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                         s32 arg5);
extern void func_80028528(s32 arg0);
extern void func_800290AC(s32 arg0);
extern void func_800291D8(s32 arg0);
extern void func_800006BC(f32 arg0, s32 arg1);
extern void func_overlay_045_F0001BF4_188E04C(void *handle, s32 value);
extern void func_overlay_066_F0000000(void *arg0);

/* Plateau: -O2/-mips2 is exact-size, 227 masked (231 raw) words, first +0x4.
 * Case scoping saved two raw words, but the handle remains in s0; spill shapes miss size.
 * The full lattice and 40m MIPS2 permuter found no zero; best 1470 used dead/no-op syntax. */
#ifdef NON_MATCHING
void func_overlay_011_F00022E8_186AB30(s32 updateRate) {
    void **handle;
    s32 index;
    s8 direction;
    O11ObjectSub *sub;
    s16 value;
    O11StatusSlot *status;
    s32 state;
    s32 transition;
    volatile s32 *menuInput;

    status = func_overlay_011_F0000000_1868848();
    direction = D_0[D_1C4];
    if (direction < -32) {
        if (D_1BC < 5) {
            D_1BC++;
            func_80000F94(0x32C, 0);
            direction = D_0_reload_success[D_1C4];
        } else {
            func_80000F94(0x32D, 0);
            direction = D_0_reload_failure[D_1C4];
        }
    }
    if (direction >= 33) {
        if (D_1BC >= 2) {
            D_1BC--;
            func_80000F94(0x32C, 0);
        } else {
            func_80000F94(0x32D, 0);
        }
    }

    for (handle = D_1CC, index = 1; index < 6; handle++, index++) {
        value = (index == D_1BC) ? D_1B8 : 0;
        func_overlay_045_F0001BF4_188E04C(*handle, value);
    }

    menuInput = (volatile s32 *)(D_menuBase + 0x1C4);
    if ((func_8002554C(D_1C4) & 0x8000) || *menuInput != 0) {
        switch (D_1BC) {
        case 1:
            func_overlay_066_F0000000(0);
            func_800290AC(0);
            func_800291D8(0x1E);
            func_800006BC(0.5f, 0x7F);
            func_overlay_011_F0002BF4_186B43C();
            D_204 = 1;
            break;
        case 2: {
            s32 caseAction;

            caseAction = *menuInput;
            if (caseAction == 0) {
                func_overlay_011_F0001058_18698A0(6);
                return;
            }
            if (caseAction == -1) {
                func_overlay_011_F0001130_1869978(6);
                return;
            }
            if (caseAction == 1) {
                D_cfgA = 3;
                D_cfgB = 4;
                sub = func_80005820(D_1C4)->sub64;
                D_204 = 1;
                status[1].field8 = 0;
                status[0].field8 = 0;
                index = caseAction * 2;
                status[index + 1].field8 = 0;
                status[index + 2].field8 = 0;
                status[index + 3].field8 = 0;
                status[index].field8 = 0;
                transition = func_8002675C();
                func_80028374(transition, sub->value1, 0, 5, 1, 0);
            }
            break;
        }
        case 3: {
            s32 caseAction;

            caseAction = *menuInput;
            if (caseAction == 0) {
                func_overlay_011_F0001058_18698A0(6);
                return;
            }
            if (caseAction == -1) {
                func_overlay_011_F0001130_1869978(6);
                return;
            }
            if (caseAction == 1) {
                func_80028528(1);
                func_80028374(0x1D, 0, 0, 0xB, 1, 0);
                D_204 = 1;
            }
            break;
        }
        case 4: {
            s32 caseAction;

            caseAction = *menuInput;
            if (caseAction == 0) {
                func_overlay_011_F0001058_18698A0(6);
                return;
            }
            if (caseAction == -1) {
                func_overlay_011_F0001130_1869978(6);
                return;
            }
            if (caseAction == 1) {
                func_80028528(1);
                func_80028374(0xC, 0, 0, 0x12, 1, 0);
                D_204 = 1;
            }
            break;
        }
        case 5: {
            s32 caseAction;

            caseAction = *menuInput;
            if (caseAction == 0) {
                func_overlay_011_F0001058_18698A0(6);
                return;
            }
            if (caseAction == -1) {
                func_overlay_011_F0001130_1869978(6);
                return;
            }
            if (caseAction == 1) {
                func_overlay_011_F0002AD8_186B320();
                D_lastMode = 7;
                func_80028374(0xC, 0, 0, 0xC, 1, 0);
                D_204 = 1;
            }
            break;
        }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o011/func_overlay_011_F00022E8_186AB30/func_overlay_011_F00022E8_186AB30.s")
#endif
