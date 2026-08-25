/*
 * Weather tail -- ROM 0x3D370-0x3D5F0.
 *
 * This is the C-owned tail of main/weather after its hand-written snow
 * assembly island.  The final helper remains descriptively unresolved.
 */

#include "PR/ultratypes.h"

extern s32 D_8007C854;
extern s32 D_8007C858;
extern s32 D_8007C85C;
extern s32 D_8007C860;
extern s32 D_8007C864;
extern s32 D_8007C868;

void func_8003C80C();

void func_8003C770(s32 arg0, s32 arg1) {
    s32 index;

    index = arg0;
    if ((arg0 < 0) || (arg0 >= 24)) {
        index = 0;
    }
    if (arg1 < 0) {
        arg1 = 0;
    }
    if (arg1 == 0) {
        D_8007C854 = 0;
        D_8007C858 = 255;
        D_8007C85C = 255;
        return;
    }
    D_8007C854 = 1;
    D_8007C860 = index >> 2;
    D_8007C864 = ((index & 3) * arg1) >> 2;
    D_8007C868 = arg1;
    func_8003C80C(0, arg1, index);
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather_tail/func_8003C80C.s")
