/*
 * Resident gzip front end -- ROM 0x4E1E0-0x4EA60.
 *
 * DKR's published src/gzip.c identifies this source unit and supplies the
 * byteswap32 body below. Mickey's bytes remain decisive: the other five
 * functions retain their generated assembly until separately reconstructed.
 */

#include "PR/ultratypes.h"

#pragma GLOBAL_ASM("asm/nonmatchings/main/gzip/func_8004D5E0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gzip/func_8004D750.s")

/*
 * PROVENANCE: adapted from Diddy Kong Racing's published src/gzip.c.
 * Mickey's compiled and linked function is independently byte-identical.
 */
s32 byteswap32(u8 *arg0) {
    s32 value;

    value = *arg0++;
    value |= *arg0++ << 8;
    value |= *arg0++ << 16;
    value |= *arg0 << 24;
    return value;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/gzip/func_8004D7A8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gzip/func_8004D7E0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gzip/func_8004D840.s")
