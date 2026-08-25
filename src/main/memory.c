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
MemoryPool D_800D1C60[MEMORY_POOL_COUNT];
s32 D_800D1CA0;
s32 sMemoryPoolCountPadding;
void *D_800D1CA8[256];
u8 D_800D20A8[256];
s32 D_800D21A8;
s32 D_800D21AC;
s32 D_800D21B0;
s32 D_800D21B4;
#define D_800D1C64 (D_800D1C60[0].slots)
extern u8 D_800D8750[];

MemoryPoolSlot *func_8002B1A0(MemoryPoolSlot *slots, s32 poolSize, s32 numSlots);

/* PROVENANCE: adapted from JFG src/memory.c:mmInit. */
void mmInit(void) {
    D_800D1CA0 = -1;
    if (D_8007A274) {
        D_800D21B4 = 0x80600000;
    } else {
        D_800D21B4 = 0x80400000;
    }
    func_8002B1A0((MemoryPoolSlot *)D_800D8750,
                  D_800D21B4 - (s32)D_800D8750, 0x640);
    mmSetDelay(2);
    D_800D21A8 = 0;
}

/* PROVENANCE: adapted from JFG src/memory.c:mmExtended. */
u8 mmExtended(void) {
    return D_8007A274;
}

/* PROVENANCE: adapted from JFG src/memory.c:mmAllocRegion. */
void *func_8002B280(s32 size, u32 colourTag);

MemoryPoolSlot *func_8002B154(s32 poolDataSize, s32 numSlots) {
    s32 size;
    MemoryPoolSlot *slots;
    s32 pad;
    MemoryPoolSlot *newPool;

    size = poolDataSize + (numSlots * sizeof(MemoryPoolSlot));
    slots = (MemoryPoolSlot *)func_8002B280(size, 0x95);
    newPool = func_8002B1A0(slots, size, numSlots);
    return newPool;
}

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

extern s32 D_8007A270;
extern s32 D_8007A278;
extern s32 D_8007A27C;

s32 runlinkGetAddressInfo(u32 address, s32 *moduleId, s32 *moduleAddress, u32 **symbolName);
void *func_8002B3A8(MemoryPoolIndex poolIndex, s32 size, u32 colourTag);

/* PROVENANCE: adapted from JFG src/memory.c:mmAlloc. */
void *func_8002B280(s32 size, u32 colourTag) {
    struct {
        volatile s32 address;
        s32 moduleAddress;
        s32 moduleId;
        s32 pad;
    } stack;

    stack.address = 0x666;
    D_8007A270 = colourTag;
    if (D_8007A278 != -1) {
        colourTag = D_8007A278 | 0xFF000000;
    } else if (D_8007A27C != -1) {
        colourTag = D_8007A27C | 0xFE000000;
    } else {
        runlinkGetAddressInfo(stack.address - 8, &stack.moduleId, &stack.moduleAddress, NULL);
        colourTag = (stack.moduleId << 24) | stack.moduleAddress;
    }
    return func_8002B3A8(MEMORY_POOL_MAIN, size, colourTag);
}

/* PROVENANCE: adapted from JFG src/memory.c:mmAlloc2. */
void *func_8002B314(s32 size, u32 colourTag) {
    struct {
        volatile s32 address;
        s32 moduleAddress;
        s32 moduleId;
        s32 pad;
    } stack;

    stack.address = 0x666;
    D_8007A270 = colourTag;
    if (D_8007A278 != -1) {
        colourTag = D_8007A278 | 0xFF000000;
    } else if (D_8007A27C != -1) {
        colourTag = D_8007A27C | 0xFE000000;
    } else {
        runlinkGetAddressInfo(stack.address - 8, &stack.moduleId, &stack.moduleAddress, NULL);
        colourTag = (stack.moduleId << 24) | stack.moduleAddress;
    }
    return func_8002B3A8(MEMORY_POOL_MAIN, size, colourTag);
}

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
void *func_8002B4C0(MemoryPoolSlot *slots, s32 size) {
    s32 i;

    for (i = D_800D1CA0; i != 0; i--) {
        if (slots == D_800D1C60[i].slots) {
            return func_8002B3A8(i, size, 0);
        }
    }
    return NULL;
}

/*
 * PROVENANCE: adapted from JFG src/memory.c:mmAllocAtAddr. Mickey's globals,
 * pool/slot layouts, absent diagnostic calls, and linked bytes are authoritative.
 * Workbench: mixed constant/structure/register, 116/116 words, 14 differences
 * from +0xE0 with the exact frame. Lever: constant-audit; owned BSS did not
 * alter the remaining slot/data-pointer allocation.
 */
#ifdef NON_MATCHING
void *func_8002B524(s32 size, u8 *address, u32 colourTag) {
    s32 slotIndex;
    MemoryPoolSlot *slot;
    MemoryPoolSlot *slots;
    s32 moduleId;
    s32 moduleAddress;
    volatile s32 callerAddress = 0x666;
    s32 pad;

    D_8007A270 = colourTag;
    if (D_8007A278 != -1) {
        colourTag = D_8007A278 | 0xFF000000;
    } else if (D_8007A27C != -1) {
        colourTag = D_8007A27C | 0xFE000000;
    } else {
        runlinkGetAddressInfo(callerAddress - 8, &moduleId, &moduleAddress, NULL);
        colourTag = (moduleId << 24) | moduleAddress;
    }

    if (D_800D1C60[MEMORY_POOL_MAIN].curNumSlots + 1 ==
        D_800D1C60[MEMORY_POOL_MAIN].maxNumSlots) {
        return NULL;
    }
    if (size & 0xF) {
        size = (size & ~0xF) + 0x10;
    }

    slots = D_800D1C60[MEMORY_POOL_MAIN].slots;
    for (slotIndex = 0; slotIndex != -1; slotIndex = slot->nextIndex) {
        slot = (MemoryPoolSlot *)((u8 *)slots + (slotIndex << 4) + (slotIndex << 2));
        if (slot->flags == MEMORY_SLOT_FREE) {
            if (address >= slot->data && address + size <= slot->data + slot->size) {
                if (address == slot->data) {
                    func_8002BB40(MEMORY_POOL_MAIN, slotIndex, size, TRUE, FALSE, colourTag);
                    return slot->data;
                }
                slotIndex = func_8002BB40(MEMORY_POOL_MAIN, slotIndex,
                                          address - slot->data, FALSE, TRUE, colourTag);
                func_8002BB40(MEMORY_POOL_MAIN, slotIndex, size, TRUE, FALSE, colourTag);
                return *(u8 **)((u8 *)slots + (slotIndex << 4) + (slotIndex << 2));
            }
        }
    }

    return NULL;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B524.s")
#endif

/* PROVENANCE: adapted from JFG src/memory.c:mmSetDelay. */
void mmSetDelay(s32 state) {
    D_800D21AC = state;
}

/* PROVENANCE: adapted from JFG src/memory.c:mmFlushFreeStack. */
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

/*
 * PROVENANCE: adapted from JFG src/memory.c:mmFreeTick. Mickey's low-memory
 * link-slot release and absent diagnostic print are authoritative differences.
 */
void ReleaseUnusedLinkSlots(void);

/* Workbench: mixed structure/register, 62/63 words, first +0x4; the target
 * keeps &D_800D21B0 in s0. Lever: structure-buckets; owned BSS leaves the
 * candidate's folded t6 base unchanged. */
#ifdef NON_MATCHING
void func_8002B7AC(void) {
    s32 i;

    if (D_800D21B0 < 0x14000) {
        ReleaseUnusedLinkSlots();
    }
    for (i = 0; i < D_800D21A8;) {
        D_800D20A8[i]--;
        if (D_800D20A8[i] == 0) {
            func_8002B8A8(D_800D1CA8[i]);
            D_800D1CA8[i] = D_800D1CA8[D_800D21A8 - 1];
            D_800D20A8[i] = D_800D20A8[D_800D21A8 - 1];
            D_800D21A8--;
        } else {
            i++;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002B7AC.s")
#endif

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

/*
 * PROVENANCE: adapted from JFG src/memory.c:mempool_slot_assign. Mickey's
 * pool accounting, byte-sized slot fields, globals, and bytes are authoritative.
 * Workbench: allocation-mismatch, exact 72 words, 30 register differences from +0x8C.
 * Lever: pool-position/temp-FIFO; owned BSS leaves the allocator web split unchanged.
 * Assembly fallback remains canonical.
 */
/* Workbench: allocation mismatch; exact 72-word size/opcode schedule, first +0x6C.
 * Levers: early colour scalar and scoped data local; 30-minute MIPS2 permuter scored 175.
 * Remaining: 26 register-only words from a pool/temp web-existence split. */
#ifdef NON_MATCHING
s32 func_8002BB40(MemoryPoolIndex poolIndex, s32 slotIndex, s32 size,
                   s32 slotIsTaken, s32 newSlotIsTaken, u32 colourTag) {
    MemoryPool *pool;
    MemoryPoolSlot *slots;
    MemoryPoolSlot *slot;
    MemoryPoolSlot *newSlot;
    volatile s32 *colourTagIndex;
    s32 index;
    s32 nextIndex;
    s32 slotSize;
    s32 colourIndex;

    colourTagIndex = &D_8007A270;
    if (slotIsTaken == TRUE) {
        if (poolIndex == MEMORY_POOL_MAIN) {
            D_800D21B0 -= size;
        }
        pool = (MemoryPool *)((u8 *)D_800D1C60 + (poolIndex << 4));
        pool->freeSize -= size;
    }

    pool = (MemoryPool *)((u8 *)D_800D1C60 + (poolIndex << 4));
    slots = pool->slots;
    slot = (MemoryPoolSlot *)((u8 *)slots + (slotIndex << 4) + (slotIndex << 2));
    slot->flags = slotIsTaken;
    slot->colourTagIndex = *colourTagIndex;
    slotSize = slot->size;
    slot->size = size;
    slot->colourTag = colourTag;
    if (size < slotSize) {
        index = ((MemoryPoolSlot *)((u8 *)slots +
                                    (((pool->curNumSlots << 2) + pool->curNumSlots) << 2)))->index;
        pool->curNumSlots++;
        ((MemoryPoolSlot *)((u8 *)slots + (index << 4) + (index << 2)))->data =
            slot->data + size;
        ((MemoryPoolSlot *)((u8 *)slots + (index << 4) + (index << 2)))->size =
            slotSize - size;
        ((MemoryPoolSlot *)((u8 *)slots + (index << 4) + (index << 2)))->flags =
            newSlotIsTaken;
        ((MemoryPoolSlot *)((u8 *)slots + (index << 4) +
                            (index << 2)))->colourTagIndex = *colourTagIndex;
        nextIndex = slot->nextIndex;
        ((MemoryPoolSlot *)((u8 *)slots + (index << 4) + (index << 2)))->prevIndex =
            slotIndex;
        ((MemoryPoolSlot *)((u8 *)slots + (index << 4) + (index << 2)))->nextIndex =
            nextIndex;
        slot->nextIndex = index;
        if (nextIndex != -1) {
            ((MemoryPoolSlot *)((u8 *)slots + (nextIndex << 4) +
                                (nextIndex << 2)))->prevIndex = index;
        }
        return index;
    }
    return slotIndex;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/memory/func_8002BB40.s")
#endif

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
