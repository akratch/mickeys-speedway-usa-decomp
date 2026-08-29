#include "PR/ultratypes.h"

typedef struct Overlay99GridPoint {
    s16 reserved00;
    s16 reserved02;
    s16 height;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay99GridPoint;

typedef struct Overlay99Segment {
    u8 bytes[0x30];
} Overlay99Segment;

extern s32 gOverlay99CurrentGrid;
extern Overlay99GridPoint *gOverlay99Grids[];
extern s32 gOverlay99SegmentCount;
extern Overlay99Segment gOverlay99Segments[];
extern s32 gOverlay99GridWidth;
extern s32 gOverlay99GridHeight;
extern s32 gOverlay99WidthMinusOne;
extern s32 gOverlay99HeightMinusOne;
extern s32 gOverlay99Arg4;
extern s32 gOverlay99Arg5;
extern void overlay99ApplySegment(Overlay99Segment *segment, f32 scale);

/*
 * Bounded clean-source plateau (reviewed 2026-08-29): the owned target is
 * 114 words with a 0x28 frame. Configured -O2 -mips2 -32
 * -Wo,-loopunroll,0 emits 115 words, with 104 differing positions and first
 * mismatch +0x2C. It carries 29 static relocations, but only 8 of their
 * offset/type pairs align with the 29 shipped runtime records. All 119 flag
 * rows were attempted; the compilable O2/MIPS-II rows tie this result. A
 * codegen-faithful allocator trace colors the negative-magnitude web into v0
 * while target t4 is eligible at equal cost. Giving that magnitude a natural
 * block-local carrier regresses to 107 differing positions and swaps the main
 * v0/v1 pool. The assembly fallback remains the only exact linked output. */
#ifdef NON_MATCHING
/* PLATEAU-HANDOFF
 * symbol: overlay99BuildHeightGrid
 * score: 104 differing words
 * frame: 0x28
 * relocations: 29
 * first-mismatch: +0x2C
 * summary: clean V0 is 115/114 words; only 8/29 runtime offset/type pairs align; trace-led magnitude split regresses
 */
void overlay99BuildHeightGrid(f32 scale, void *unused, s32 widthMinusOne,
                              s32 heightMinusOne, s32 arg4, s32 arg5) {
    Overlay99GridPoint *point;
    s32 *widthPtr;
    s32 *heightPtr;
    s32 i;
    s32 value;

    point = gOverlay99Grids[gOverlay99CurrentGrid];
    if (point == 0) {
        return;
    }

    widthPtr = &gOverlay99GridWidth;
    heightPtr = &gOverlay99GridHeight;
    *widthPtr = widthMinusOne + 1;
    *heightPtr = heightMinusOne + 1;
    gOverlay99WidthMinusOne = widthMinusOne;
    gOverlay99HeightMinusOne = heightMinusOne;
    gOverlay99Arg4 = arg4;
    gOverlay99Arg5 = arg5;

    i = *heightPtr * *widthPtr;
    while (i--) {
        point->height = 5;
        point++;
    }

    i = gOverlay99SegmentCount;
    while (i--) {
        overlay99ApplySegment(&gOverlay99Segments[i], scale);
    }

    widthPtr = &gOverlay99GridWidth;
    heightPtr = &gOverlay99GridHeight;
    i = *heightPtr * *widthPtr;
    point = gOverlay99Grids[gOverlay99CurrentGrid];
    while (i--) {
        value = point->height - 5;
        if (value < 0) {
            if (value < -40) {
                value = -40;
            }
            value = -value;
            point->red = 0;
            point->green = 0;
            point->blue = 0;
            point->alpha = value;
        } else if (value > 0) {
            if (value > 40) {
                value = 40;
            }
            point->red = 0xFF;
            point->green = 0xFF;
            point->blue = 0xFF;
            point->alpha = value;
        } else {
            point->alpha = 0;
        }
        point++;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o099/overlay99BuildHeightGrid/func_overlay_099_F0000638_18D9BE8.s")
#endif
