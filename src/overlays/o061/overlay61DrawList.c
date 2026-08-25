#include "PR/ultratypes.h"

extern s32 gOverlay61ItemCountReloc;
extern s32 gOverlay61SelectedIndexReloc;
extern s32 gOverlay61TopIndexReloc;
extern u8 gOverlay61ItemsReloc[];
extern u8 gOverlay61UpLabelReloc[];
extern u8 gOverlay61DownLabelReloc[];

extern void overlay61ClampIndexReloc(s32 mode, s32 *index);
extern void overlay61BeginListReloc(s32, s32, s32, s32);
extern void overlay61EndListReloc(s32, s32, s32, s32, s32);
extern void overlay61DrawScrollReloc(s32, s32, s32, void *, s32);
extern void func_overlay_061_F00003C0_18BF788(
    s32 context, s32 y, void *item, s32 selected);

/* Plateau: exact 0x1A4 and 102/105 words; first mismatch +0x94 schedules the
 * loop-limit li before two address addends. The 119-flag/access-shape pass
 * failed; a 2,401s permuter fixed that window but swapped initial v0/v1. */
#ifdef NON_MATCHING
void overlay61DrawList(s32 context) {
    s32 index;
    s32 y;
    s32 remaining;

    if (gOverlay61SelectedIndexReloc < gOverlay61TopIndexReloc) {
        gOverlay61TopIndexReloc = gOverlay61SelectedIndexReloc;
    } else if (gOverlay61SelectedIndexReloc >=
               gOverlay61TopIndexReloc + 7) {
        gOverlay61TopIndexReloc = gOverlay61SelectedIndexReloc - 6;
    }

    overlay61ClampIndexReloc(2, &gOverlay61TopIndexReloc);
    overlay61BeginListReloc(0, 0, 0, 0);

    index = gOverlay61TopIndexReloc;
    y = 0x32;
    remaining = 6;
    do {
        if (index >= gOverlay61ItemCountReloc) {
            func_overlay_061_F00003C0_18BF788(context, y, 0, 0);
        } else {
            func_overlay_061_F00003C0_18BF788(
                context, y, &gOverlay61ItemsReloc[index << 6],
                index++ == gOverlay61SelectedIndexReloc);
        }
        y += 0x18;
    } while (remaining--);

    overlay61EndListReloc(0xFF, 0xFF, 0, 0xFF, 0xFF);
    if (gOverlay61TopIndexReloc != 0) {
        overlay61DrawScrollReloc(
            context, 0xA0, 0x28, gOverlay61UpLabelReloc, 4);
    }
    if (gOverlay61TopIndexReloc + 7 < gOverlay61ItemCountReloc) {
        overlay61DrawScrollReloc(
            context, 0xA0, 0xD8, gOverlay61DownLabelReloc, 4);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o061/overlay61DrawList/func_overlay_061_F00007C4_18BFB8C.s")
#endif
