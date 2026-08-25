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
extern MemoryPool D_800D1C60[];
extern s32 D_800D1CA0;
extern MemoryPoolSlot *D_800D1C64;
extern s32 D_800D21AC;
extern s32 D_800D21B0;

#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/mmInit.s")

/* PROVENANCE: adapted from JFG src/memory.c:mmExtended. */
u8 mmExtended(void) {
    return D_8007A274;
}

/* JFG correspondence: mmAllocRegion (tier B; allocation then pool creation). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B154.s")

/* PROVENANCE: adapted from JFG src/memory.c:mempool_init. */
MemoryPoolSlot *func_8002B1A0(MemoryPoolSlot *slots, s32 poolSize, s32 numSlots) {
    MemoryPoolSlot *firstSlot;
    s32 poolCount;
    s32 i;
    s32 firstSlotSize;

    poolCount = ++D_800D1CA0;
    firstSlotSize = poolSize - (numSlots * sizeof(MemoryPoolSlot));
    D_800D1C60[poolCount].maxNumSlots = numSlots;
    D_800D1C60[poolCount].curNumSlots = 0;
    D_800D1C60[poolCount].slots = slots;
    D_800D1C60[poolCount].size = poolSize;
    D_800D1C60[poolCount].freeSize = firstSlotSize;
    firstSlot = slots;
    for (i = 0; i < D_800D1C60[poolCount].maxNumSlots; i++) {
        firstSlot->index = i;
        firstSlot++;
    }
    firstSlot = &D_800D1C60[poolCount].slots[0];
    slots += numSlots;
    if ((s32)slots & 0xF) {
        firstSlot->data = (u8 *)(((s32)slots & ~0xF) + 0x10);
    } else {
        firstSlot->data = (u8 *)slots;
    }
    firstSlot->size = firstSlotSize;
    firstSlot->flags = MEMORY_SLOT_FREE;
    firstSlot->colourTagIndex = 0x95;
    firstSlot->prevIndex = -1;
    firstSlot->nextIndex = -1;
    D_800D1C60[poolCount].curNumSlots++;
    if (poolCount == MEMORY_POOL_MAIN) {
        D_800D21B0 = firstSlotSize;
    }
    return D_800D1C60[poolCount].slots;
}

/* JFG correspondence: mmAlloc (tier B; main-pool allocation wrapper). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B280.s")

/* JFG correspondence: mmAlloc2 (tier B; duplicate allocation wrapper). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B314.s")

/* PROVENANCE: adapted from JFG src/memory.c:mempool_slot_find. */
s32 func_8002BB40(MemoryPoolIndex poolIndex, s32 slotIndex, s32 size,
                   s32 slotIsTaken, s32 newSlotIsTaken, u32 colourTag);

void *func_8002B3A8(MemoryPoolIndex poolIndex, s32 size, u32 colourTag) {
    s32 slotSize;
    MemoryPoolSlot *slot;
    volatile s32 pad;
    MemoryPool *pool;
    MemoryPoolSlot *slots;
    s16 nextIndex;
    s32 currIndex;

    pool = &D_800D1C60[poolIndex];
    if (pool->maxNumSlots == pool->curNumSlots + 1) {
        return NULL;
    }
    currIndex = -1;
    if (size & 0xF) {
        size = (size & ~0xF) + 0x10;
    }
    slotSize = 0x7FFFFFFF;
    slots = pool->slots;
    nextIndex = 0;
    do {
        slot = (MemoryPoolSlot *)((u8 *)slots + (nextIndex << 4) + (nextIndex << 2));
        if (slot->flags == MEMORY_SLOT_FREE) {
            if (slot->size >= size && slot->size < slotSize) {
                slotSize = slot->size;
                currIndex = nextIndex;
            }
        }
        nextIndex = slot->nextIndex;
    } while (nextIndex != -1);

    if (currIndex != -1) {
        func_8002BB40(poolIndex, currIndex, size, TRUE, FALSE, colourTag);
        return (currIndex + slots)->data;
    }
    return NULL;
}

/* PROVENANCE: adapted from JFG src/memory.c:mmAllocR. */
void *func_8002B3A8(MemoryPoolIndex poolIndex, s32 size, u32 colourTag);

void *func_8002B4C0(MemoryPoolSlot *slots, s32 size) {
    s32 i;

    for (i = D_800D1CA0; i != 0; i--) {
        if (slots == D_800D1C60[i].slots) {
            return func_8002B3A8(i, size, 0);
        }
    }
    return NULL;
}

/* JFG correspondence: mmAllocAtAddr (tier B; fixed-address allocation). */
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B524.s")

/* PROVENANCE: adapted from JFG src/memory.c:mmSetDelay. */
void mmSetDelay(s32 state) {
    D_800D21AC = state;
}

/* PROVENANCE: adapted from JFG src/memory.c:mmFlushFreeStack. */
extern void *D_800D1CA8[];
extern s8 D_800D20A8[];
extern s32 D_800D21A8;

void func_8002B8A8(u8 *address);

void func_8002B700(void) {
    while (D_800D21A8 > 0) {
        func_8002B8A8(D_800D1CA8[--D_800D21A8]);
    }
}

/* PROVENANCE: adapted from JFG src/memory.c:mmFree. */
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

/* PROVENANCE: adapted from JFG src/memory.c:mempool_free_addr. */
s32 func_8002B978(u8 *address);
void func_8002B9D0(MemoryPoolIndex poolIndex, s32 slotIndex);

void func_8002B8A8(u8 *address) {
    s16 slotIndex;
    s32 poolIndex;
    MemoryPoolSlot *slots;
    MemoryPoolSlot *slot;

    poolIndex = func_8002B978(address);
    slots = *(MemoryPoolSlot **)((u8 *)&D_800D1C64 + (poolIndex << 4));
    for (slotIndex = 0; slotIndex != -1; slotIndex = slot->nextIndex) {
        slot = (MemoryPoolSlot *)((u8 *)slots + (slotIndex << 4) + (slotIndex << 2));
        if (address == slot->data) {
            if (slot->flags == MEMORY_SLOT_USED || slot->flags == MEMORY_SLOT_SAFEGUARD) {
                func_8002B9D0(poolIndex, slotIndex);
            }
            break;
        }
    }
}

/* PROVENANCE: adapted from JFG src/memory.c:mempool_free_queue. */
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

/* PROVENANCE: adapted from JFG src/memory.c:mempool_slot_clear. */
void func_8002B9D0(MemoryPoolIndex poolIndex, s32 slotIndex) {
    s16 nextIndex;
    s16 prevIndex;
    s16 tempNextIndex;
    MemoryPoolSlot *slots;
    MemoryPoolSlot *slot;
    MemoryPoolSlot *nextSlot;
    MemoryPoolSlot *prevSlot;

    slots = D_800D1C60[poolIndex].slots;
    slot = (MemoryPoolSlot *)((u8 *)slots + (slotIndex << 4) + (slotIndex << 2));
    nextIndex = slot->nextIndex;
    prevIndex = slot->prevIndex;
    nextSlot = (MemoryPoolSlot *)((u8 *)slots + (nextIndex << 4) + (nextIndex << 2));
    prevSlot = (MemoryPoolSlot *)((u8 *)slots + (prevIndex << 4) + (prevIndex << 2));
    slot->flags = MEMORY_SLOT_FREE;
    if (poolIndex == MEMORY_POOL_MAIN) {
        D_800D21B0 += slot->size;
    }
    D_800D1C60[poolIndex].freeSize += slot->size;
    if (nextIndex != -1 && nextSlot->flags == MEMORY_SLOT_FREE) {
        slot->size += nextSlot->size;
        tempNextIndex = nextSlot->nextIndex;
        slot->nextIndex = tempNextIndex;
        if (tempNextIndex != -1) {
            ((MemoryPoolSlot *)((u8 *)slots + (tempNextIndex << 4) +
                                (tempNextIndex << 2)))->prevIndex = slotIndex;
        }
        D_800D1C60[poolIndex].curNumSlots--;
        slots[D_800D1C60[poolIndex].curNumSlots].index = nextIndex;
    }
    if (prevIndex != -1 && prevSlot->flags == MEMORY_SLOT_FREE) {
        prevSlot->size += slot->size;
        tempNextIndex = slot->nextIndex;
        prevSlot->nextIndex = tempNextIndex;
        if (tempNextIndex != -1) {
            ((MemoryPoolSlot *)((u8 *)slots + (tempNextIndex << 4) +
                                (tempNextIndex << 2)))->prevIndex = prevIndex;
        }
        D_800D1C60[poolIndex].curNumSlots--;
        slots[D_800D1C60[poolIndex].curNumSlots].index = slotIndex;
    }
}

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
