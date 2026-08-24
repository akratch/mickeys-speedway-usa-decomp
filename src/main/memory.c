/*
 * Resident allocator -- ROM 0x2BCD0-0x2C8C0 (VRAM 0x8002B0D0).
 *
 * The linked pre-split object owns exactly 0xBF0 bytes. The last function ends
 * at ROM 0x2C8B4 and the final 0xC bytes are 16-byte alignment padding; the
 * next linked object begins at ROM 0x2C8C0.
 *
 * PROVENANCE: the candidate names recorded beside the still-unmatched stubs
 * were read from Jet Force Gemini's published src/memory.c and src/memory.h.
 * They are not adopted as Mickey symbols merely from source order. JFG is a
 * permitted public retail-derived decomp under docs/CLEANROOM.md.
 */

#include "game/memory.h"

#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/mmInit.s")

/* JFG correspondence: mmExtended (tier B; reads the flag consumed by mmInit). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B148.s")

/* JFG correspondence: mmAllocRegion (tier B; allocation then pool creation). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B154.s")

/* JFG correspondence: mempool_init (tier B; called by mmInit/mmAllocRegion). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B1A0.s")

/* JFG correspondence: mmAlloc (tier B; main-pool allocation wrapper). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B280.s")

/* JFG correspondence: mmAlloc2 (tier B; duplicate allocation wrapper). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B314.s")

/* JFG correspondence: mempool_slot_find (tier B; shared allocation worker). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B3A8.s")

/* JFG correspondence: mmAllocR (tier B; pool-selecting allocation wrapper). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B4C0.s")

/* JFG correspondence: mmAllocAtAddr (tier B; fixed-address allocation). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B524.s")

/* JFG correspondence: mmSetDelay (tier B; writes the deferred-free delay). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B6F4.s")

/* JFG correspondence: mmFlushFreeStack (tier B; drains deferred frees). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B700.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/mmFree.s")

/* JFG correspondence: mmFreeTick (tier B; services deferred frees). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B7AC.s")

/* JFG correspondence: mempool_free_addr (tier B; locates and clears a slot). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B8A8.s")

/* JFG correspondence: mempool_free_queue (tier B; enqueues a delayed free). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B93C.s")

/* JFG correspondence: mempool_get_pool (tier B; address-to-pool search). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B978.s")

/* JFG correspondence: mempool_slot_clear (tier B; frees/coalesces a slot). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B9D0.s")

/* JFG correspondence: mmGetSlotPtr (tier B; returns a pool's slot array). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002BB20.s")

/* JFG correspondence: mmGetDelay (tier B; returns deferred-free delay). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002BB34.s")

/* JFG correspondence: mempool_slot_assign (tier B; splits/assigns a slot). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002BB40.s")

/* PROVENANCE: adapted from JFG src/memory.c:mmAlign16. */
u8 *align16(u8 *address) {
    s32 remainder = (s32) address & 0xF;

    if (remainder > 0) {
        address = (u8 *) (((s32) address - remainder) + 16);
    }
    return address;
}

/* PROVENANCE: derived from JFG src/memory.c's mmAlign16/mmAlign4 family. */
u8 *align8(u8 *address) {
    s32 remainder = (s32) address & 7;

    if (remainder > 0) {
        address = (u8 *) (((s32) address - remainder) + 8);
    }
    return address;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/align4.s")
