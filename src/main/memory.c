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

extern u8 D_8007A274;
extern MemoryPoolSlot *D_800D1C64;
extern s32 D_800D21AC;

#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/mmInit.s")

/* PROVENANCE: adapted from JFG src/memory.c:mmExtended. */
u8 mmExtended(void) {
    return D_8007A274;
}

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

/* PROVENANCE: adapted from JFG src/memory.c:mmSetDelay. */
void mmSetDelay(s32 state) {
    D_800D21AC = state;
}

/* JFG correspondence: mmFlushFreeStack (tier B; drains deferred frees). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B700.s")

/* PROVENANCE: adapted from JFG src/memory.c:mmFree. */
s32 func_8002B8A8(u8 *address);
void func_8002B93C(void *dataAddress);

void mmFree(void *data) {
    volatile s32 callerAddress = 0x666;

    if (D_800D21AC == 0) {
        func_8002B8A8(data);
    } else {
        func_8002B93C(data);
    }
}

/* JFG correspondence: mmFreeTick (tier B; services deferred frees). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B7AC.s")

/* JFG correspondence: mempool_free_addr (tier B; locates and clears a slot). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B8A8.s")

/* PROVENANCE: adapted from JFG src/memory.c:mempool_free_queue. */
extern void *D_800D1CA8[];
extern s8 D_800D20A8[];
extern s32 D_800D21A8;

void func_8002B93C(void *dataAddress) {
    D_800D1CA8[D_800D21A8] = dataAddress;
    D_800D20A8[D_800D21A8] = D_800D21AC;
    D_800D21A8++;
}

/* PROVENANCE: adapted from JFG src/memory.c:mempool_get_pool. */
extern MemoryPool D_800D1C60[];
extern s32 D_800D1CA0;

s32 func_8002B978(u8 *address) {
    s32 i;
    MemoryPool *pool;

    for (i = D_800D1CA0; i > 0; i--) {
        pool = &D_800D1C60[i];
        if ((u8 *)pool->slots >= address) {
            continue;
        }
        if (address < pool->size + (u8 *)pool->slots) {
            break;
        }
    }
    return i;
}

/* JFG correspondence: mempool_slot_clear (tier B; frees/coalesces a slot). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B9D0.s")

/* PROVENANCE: adapted from JFG src/memory.c:mmGetSlotPtr. */
MemoryPoolSlot *mmGetSlotPtr(MemoryPoolIndex poolIndex) {
    return *(MemoryPoolSlot **) ((u8 *) &D_800D1C64 + (poolIndex * sizeof(MemoryPool)));
}

/* PROVENANCE: adapted from JFG src/memory.c:mmGetDelay. */
s32 mmGetDelay(void) {
    return D_800D21AC;
}

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

/* PROVENANCE: adapted from JFG src/memory.c:mmAlign4. */
u8 *align4(u8 *address) {
    s32 remainder = (s32) address & 3;

    if (remainder > 0) {
        address = (u8 *) (((s32) address - remainder) + 4);
    }
    return address;
}
