#include "PR/ultratypes.h"

typedef struct Overlay49Object {
    u8 pad00[0x88];
    s32 mode;
} Overlay49Object;

extern Overlay49Object *gOverlay49Result;
extern s32 gOverlay49Finished;
extern s32 gOverlay49Timer;
extern s32 gOverlay49InputEnabled;
extern u8 D_8007BF08;
extern u8 D_8007BF04;
extern u8 D_800D0000[];
extern u8 D_800D0004[];
extern u32 func_800254FC(s32 index);
extern u32 func_8002554C(s32 index);
extern void func_800016EC(s32 mode);
extern void func_8003A754(void);
extern void overlay48InitializeReloc(void);
extern void func_80028374(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                         s32 arg5);
extern void overlay65UpdateReloc(void *arg0, void *arg1, s32 updateRate);

/* No exact updater donor exists in DKR v77/v80 or JFG. */
#ifdef NON_MATCHING
void overlay49Update(s32 updateRate) {
    s32 index;
    u32 inputA;
    u32 inputB;

    inputA = 0;
    inputB = 0;
    index = 3;
    do {
        inputA |= func_800254FC(index);
        inputB |= func_8002554C(index);
    } while (index-- != 0);
    if (gOverlay49Result != NULL) {
        gOverlay49Result->mode = 3;
    }
    if (gOverlay49Finished == 0) {
        gOverlay49Timer -= updateRate;
        if (gOverlay49Timer < 2520) {
            if (gOverlay49InputEnabled != 0 && (inputB & 0x9000) != 0) {
                gOverlay49Timer = 0;
            }
            if ((inputA & 0x820) == 0x820) {
                gOverlay49Timer = 0;
            }
        }
        if (gOverlay49Timer <= 0) {
            func_800016EC(9);
            if (D_8007BF08 == 0) {
                func_8003A754();
            }
            if (D_8007BF04 != 0) {
                overlay48InitializeReloc();
            } else {
                func_80028374(12, 0, 0, 12, 1, 0);
            }
            gOverlay49Finished = 1;
        }
    }
    overlay65UpdateReloc(D_800D0000, D_800D0004, updateRate);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o049/overlay49Update/func_overlay_049_F00001F4_1896604.s")
#endif
