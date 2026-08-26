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

Overlay22Object *gOverlay22Nodes[12] = { 0 };
volatile s32 D_30 = 0;
static u8 overlay22DataPad34[0xC] = { 0 };
static f32 overlay22Constants[5] = { 14.4F, 14.4F, 0.8F, 0.03F, 0.707F };
static u8 overlay22DataTail[0xC] = { 0 };

extern Overlay22Object *gOverlay22NodesEnd[];
extern void func_overlay_022_F0000000_1878108();

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
/*
 * Workbench: allocation-mismatch, exact 91/-40 frame; 43 words, first +0x10; lane choices are ring-only.
 * Levers tried: flag lattice, declaration/type/global forms, 30-minute permuter, and a dead model read (reverted).
 * Remains: pool/temp FIFO placement across pointer/count/index webs; relocation identity differs diagnostically.
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
