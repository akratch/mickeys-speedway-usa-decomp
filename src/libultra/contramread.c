/*
 * libultra: Controller Pak block read over the joybus.
 *
 * Byte-identical to Jet Force Gemini's built libultra `io/contramread.c` object, so
 * the file boundary is measured rather than guessed -- see the provenance note
 * in symbol_addrs.us.txt.
 *
 * NOT MATCHED, and close. At -O2 -g3 -mips2 -32 the SDK body reproduces every
 * word except five, all inside one four-times-unrolled byte-fill loop: the ROM
 * schedules `addiu s0, s0, 4` into the loop branch's delay slot and stores the
 * four bytes at 0(s0)..3(s0), while this compiler bumps the pointer early and
 * puts `sb zero, -4(s0)` in the slot. The instruction multiset is the same and
 * no other word differs. src/libultra/setglobalintmask.c is blocked on the
 * same kind of one-slot scheduling disagreement, which is the second
 * independent sign that this IDO's scheduler is not quite the one that built
 * Mickey.
 */

#pragma GLOBAL_ASM("asm/nonmatchings/libultra/contramread/__osContRamRead.s")
