#include "PR/ultratypes.h"

extern u8 D_8007BF3C[];
extern s32 D_8007BF44;
extern void *func_80028F54(void);

typedef struct Menu3B1A0Record {
    u8 byte00;
    u8 byte01;
    u8 byte02;
    u8 byte03;
    u8 bytes04[3];
    u8 byte07;
    u8 bytes08[0x17];
    u8 byte1F;
    u8 bytes20[6];
    u16 half26;
} Menu3B1A0Record;

typedef struct Menu3B1A0ByteRecord {
    u8 bytes00[0x1F];
    u8 byte1F;
} Menu3B1A0ByteRecord;

typedef struct Menu3B1A0State {
    u8 bytes000[0x383];
    s8 count;
    u8 bytes384[0xD8];
    u8 flag45C;
} Menu3B1A0State;

typedef struct Menu3B1A0Object {
    u8 bytes00[0x64];
    Menu3B1A0State *state;
} Menu3B1A0Object;

typedef struct Menu3B1A0KeyGroup {
    s16 key00;
    s16 key02;
    s16 key04;
    s16 key06;
} Menu3B1A0KeyGroup;

extern s32 *D_8007C0B8;
extern u8 D_8007C0E8;
extern u8 D_8007C118;
extern s16 D_8007C11C[];
extern s16 D_8007C11E[];
extern s16 D_8007C120[];
extern s16 D_8007C122[];
extern u8 D_80082714[];

#pragma GLOBAL_ASM("asm/nonmatchings/main/menu_3B1A0/func_8003A5A0.s")

void func_8003A680(s32 arg0) {
    if (D_8007BF44 < 8) {
        D_8007BF3C[D_8007BF44] = arg0;
        D_8007BF44 += 1;
    }
}
s32 func_8003A6B0(u8 arg0) {
    s32 temp_t6 = arg0;
    s32 temp_v1 = temp_t6;

    if (temp_t6 == 0x21) {
        goto return_1B;
    }
    if (temp_t6 == 0x2A) {
        goto return_1C;
    }
    if (temp_t6 != 0x3F) {
        goto return_default;
    }
    return 0x1A;

return_1B:
    return 0x1B;
return_1C:
    return 0x1C;
return_default:
    return (temp_v1 - 0x41) & 0xFF;
}
s32 func_8003A700(u8 arg0) {
    s32 temp_t6 = arg0;
    s32 temp_v1 = temp_t6;

    switch (temp_t6) {
    case 0x1A:
        return 0x3F;
    case 0x1B:
        return 0x21;
    case 0x1C:
        return 0x2A;
    default:
        return (temp_v1 + 0x41) & 0xFF;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu_3B1A0/func_8003A754.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu_3B1A0/func_8003A7D0.s")
