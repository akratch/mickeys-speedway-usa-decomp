/*
 * PROVENANCE: bodies adapted from Jet Force Gemini's public decompilation,
 * src/math/math_arc.c. The lookup tables remain Mickey's extracted data;
 * Mickey's own linked bytes and relocations remain authoritative.
 */

#include "PR/ultratypes.h"

extern u16 D_800802C0[];
extern u16 D_8008033C[];
extern u16 D_800803BC[];

s32 tableval(s32 num) {
    s32 curTableVal;
    s32 nexTableVal;
    s32 tableIndex;
    s32 mask;
    s32 shift;
    u16 *table;

    if (num >= 0x7FE0) {
        mask = 7;
        shift = 3;
        table = D_800803BC;
        num -= 0x7FE0;
    } else if (num >= 0x7800) {
        mask = 0x1F;
        shift = 5;
        table = D_8008033C;
        num -= 0x7800;
    } else {
        mask = 0x1FF;
        shift = 9;
        table = D_800802C0;
    }
    tableIndex = num >> shift;
    curTableVal = table[tableIndex];
    nexTableVal = table[tableIndex + 1];
    curTableVal -= ((curTableVal - nexTableVal) * (num & mask)) >> shift;
    return curTableVal;
}

u16 acoss(s16 x) {
    s32 result;

    if (x >= 0) {
        result = x;
    } else {
        result = -x;
    }
    result = tableval(result);
    if (x < 0) {
        result = 0xFFFF - result;
    }
    return result;
}

s16 asins(s16 x) {
    s32 result;

    if (x >= 0) {
        result = x;
    } else {
        result = -x;
    }
    result = tableval(result);
    if (x >= 0) {
        result = 0x7FFF - result;
    } else {
        result -= 0x8000;
    }
    return result;
}
