#include "PR/ultratypes.h"

typedef struct Overlay60Command {
    u32 w0;
    u32 w1;
} Overlay60Command;

typedef struct Overlay60Color {
    u8 pad00[8];
    u8 red;
    u8 green;
    u8 blue;
    u8 pad0B;
} Overlay60Color;

extern u8 gOverlay60LocalDataReloc[];
extern Overlay60Command *gOverlay60DisplayListReloc;
extern s16 gOverlay60InputAxisReloc;
extern u8 D_800000B0[];

extern s32 func_overlay_082_F00004A4_18CF624(void *object);
extern void func_80034554(Overlay60Command **commands, s32 resource, s32 mode,
                          s32 flags);
extern void func_80036600(Overlay60Color *color, s32 index);
extern void func_80036660(Overlay60Color *color, s32 amount);

#define OVERLAY60_SHIFTL(value, shift, width) \
    (((u32)(value) & ((1U << (width)) - 1U)) << (shift))

/*
 * Plateau: exact 0x378 size with -Wab,-r4300_mul; 84 of 222 words differ,
 * first mismatch +0x64. The remaining basin swaps the display-list and x
 * callee-saved registers, which then cascades through the packet temporaries.
 */
#ifdef NON_MATCHING
s32 func_overlay_060_F0002F54_18BCD2C(s32 left, s32 bottom, s32 width,
                                       s32 height, s32 progress, s32 alpha,
                                       s32 ticks) {
    Overlay60Command *command;
    s32 right;
    s32 x;
    s32 offset;
    s32 split;
    u32 top;
    Overlay60Color color;
    f32 widthFloat;
    f32 fraction;
    f32 topFloat;
    f32 verticalOffset;

    if (func_overlay_082_F00004A4_18CF624(
            *(void **)(gOverlay60LocalDataReloc + 0xA8)) != 0) {
        if (gOverlay60InputAxisReloc < -0x10) {
            progress -= ticks * 2;
            if (progress < 0) {
                progress = 0;
            }
        } else if (gOverlay60InputAxisReloc >= 0x11) {
            progress += ticks * 2;
            if (progress >= 0x101) {
                progress = 0x100;
            }
        }
    }

    right = left + width;
    func_80034554(&gOverlay60DisplayListReloc, 0, 0, 0);
    command = gOverlay60DisplayListReloc++;
    command->w0 = 0x07020010;
    command->w1 = (u32)D_800000B0;

    x = left;
    if (left < right) {
        widthFloat = (f32)width;
        split = (s32)((f32)left +
                      widthFloat * ((f32)progress * 0.00390625f));
        offset = x - left;
        do {
            fraction = (f32)offset / widthFloat;
            verticalOffset = (f32)height * fraction;
            func_80036600(&color, 0xB);
            func_80036660(&color, (s32)(fraction * 60.0f));
            topFloat = (f32)bottom - verticalOffset;

            if (x < split) {
                command = gOverlay60DisplayListReloc++;
                command->w0 = 0xFA000000;
                command->w1 = ((u32)color.red << 24) |
                              ((u32)color.green << 16) |
                              ((u32)color.blue << 8) | (alpha & 0xFF);
            } else {
                command = gOverlay60DisplayListReloc++;
                command->w0 = 0xFA000000;
                command->w1 = ((u32)((s32)color.red >> 1) << 24) |
                              (((u32)((s32)color.green >> 1) & 0xFF) << 16) |
                              (((u32)((s32)color.blue >> 1) & 0xFF) << 8) |
                              ((alpha >> 1) & 0xFF);
            }

            command = gOverlay60DisplayListReloc++;
            command->w0 = 0xF6000000 |
                          OVERLAY60_SHIFTL(x + 2, 14, 10) |
                          OVERLAY60_SHIFTL(bottom, 2, 10);
            top = (u32)topFloat;
            command->w1 = OVERLAY60_SHIFTL(x, 14, 10) |
                          OVERLAY60_SHIFTL(top, 2, 10);
            x += 4;
            offset += 4;
        } while (x < right);
    }
    return progress;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o060/func_overlay_060_F0002F54_18BCD2C/func_overlay_060_F0002F54_18BCD2C.s")
#endif
