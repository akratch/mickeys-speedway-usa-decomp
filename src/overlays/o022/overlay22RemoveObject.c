#include "PR/ultratypes.h"

typedef struct Overlay22ModelState {
    u8 pad00[6];
    u16 flags06;
} Overlay22ModelState;

typedef struct Overlay22Object {
    u8 pad00[0xC];
    s32 x;
    s32 y;
    s32 z;
    u8 pad18[0x30];
    Overlay22ModelState *model48;
    u8 pad4C[0x34];
    u32 flags80;
} Overlay22Object;

extern Overlay22Object *gOverlay22Nodes[];
extern Overlay22Object *gOverlay22NodesEnd[];
extern volatile s32 D_30;
extern void func_overlay_022_F0000000_1878108();

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
/*
 * Plateau (2026-08-25, 10 attempts): the best canonical -O2 candidate has
 * the exact 91-word size, differs in 43 words, and first diverges at +0x10.
 * Its instruction and branch topology is otherwise exact; the remaining
 * mismatch is a cyclic temporary-register assignment across the model pointer,
 * node count, found index, loop index, and compaction tail.  Declaration,
 * scope, typed/raw-field, dead-assignment, and indexed-loop variants did not
 * improve it; the bounded permuter could not run because its import.py is
 * absent from this checkout.
 */
#ifdef NON_MATCHING
void func_overlay_022_F0000D30_1878E38(Overlay22Object *object, s32 flags) {
    s32 i;
    s32 found;
    s32 count;

    object->model48->flags06 &= ~1;
    count = D_30;
    found = -1;
    for (i = 0; i < count; i++) {
        if (gOverlay22Nodes[i] == object) {
            found = i;
            i = count;
        }
    }

    if (found != -1) {
        s32 last;

        last = count - 1;
        if (found < last) {
            Overlay22Object **current;
            Overlay22Object **end;

            end = &gOverlay22NodesEnd[last];
            current = &gOverlay22Nodes[found];
            do {
                *current = current[1];
                current++;
            } while (current < end);
        }
        gOverlay22Nodes[count] = 0;
        D_30 = last;
    }

    if (flags & 1) {
        object->flags80 |= 2;
        func_overlay_022_F0000000_1878108(object, 1);
    }
    if (flags & 2) {
        func_overlay_022_F0000000_1878108(0x22B, object->x, object->y,
                                          object->z, 4, 0);
    }
    if (flags & 4) {
        func_overlay_022_F0000000_1878108(0x279, object->x, object->y,
                                          object->z, 4, 0);
    }
    func_overlay_022_F0000000_1878108(object);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o022/overlay22RemoveObject/func_overlay_022_F0000D30_1878E38.s")
#endif
