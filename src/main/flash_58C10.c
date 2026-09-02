#include "PR/ultratypes.h"
#include "PR/os_gbpak.h"

extern s32 func_8006A5A0(OSMesgQueue *queue, OSPfs *pfs, s32 channel);
extern s32 func_8006AC60(OSPfs *pfs, s32 flag);
extern s32 func_8006B020(OSPfs *pfs, u16 flag, u16 address, u8 *buffer, u16 size);
extern OSMesgQueue *joyMessageQ(void);
extern OSPfs D_800D7830;
extern OSGbpakId D_800D77E0;
extern OSGbpakId *D_8007F7A0;
extern s8 D_8007F7A4;
extern s32 D_8007A1CC;
extern s32 D_800D789C;
extern s32 D_800D78A4;
extern s32 D_800D78AC;
extern s32 D_800D7898;
extern u8 D_800D789A[];
extern u8 *D_800D78A0[];
extern s32 D_800D78A8[];

void func_80058010(void) {
    u8 status[5];

    if (func_8006A5A0(joyMessageQ(), &D_800D7830, 0) == 0) {
        if (osGbpakCheckConnector(&D_800D7830, &status[3]) == 0) {
            D_8007F7A4 = 1;
            return;
        }
        func_8006AC60(&D_800D7830, 0);
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/flash_58C10/func_8005807C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/flash_58C10/func_800580F0.s")
void func_800581BC(void) {
    D_8007A1CC |= 0x04000000;
}
void func_800581D8(s32 arg0, s32 arg1, s32 arg2) {
    D_8007A1CC |= 0x02000000;
    D_800D789C = arg0;
    D_800D78A4 = arg1;
    D_800D78AC = arg2;
}
void func_8005820C(s32 arg0, s32 arg1, s32 arg2) {
    D_8007A1CC |= 0x01000000;
    D_800D7898 = arg0;
    D_800D78A0[0] = (u8 *)arg1;
    D_800D78A8[0] = arg2;
}

OSGbpakId *func_80058240(void) {
    return D_8007F7A0;
}
