#include "PR/ultratypes.h"

extern void *overlay1AllocateRecordReloc(u32 *source);

/* DKR v77/v80 and JFG have generic copy loops, but no exact donor. */
#ifdef NON_MATCHING
void *overlay1CloneRecord(u32 *source) {
    u32 *destination;
    register u32 remaining;
    void *result;

    result = overlay1AllocateRecordReloc(source);
    if (result == 0) {
        return 0;
    }
    remaining = 50;
    {
        u32 *input;

        input = source;
        destination = result;
        do {
            *destination++ = *input++;
        } while (remaining--);
    }
    return result;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1CloneRecord/func_overlay_001_F0007344_1853724.s")
#endif
