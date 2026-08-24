#include "PR/ultratypes.h"

typedef struct Overlay40Command { u32 w0; u32 w1; } Overlay40Command;
typedef struct Overlay40Position { s32 y; s32 x; s32 reserve[3]; } Overlay40Position;

extern s16 gO40TintAlphaReloc, gO40TintRedReloc;
extern s16 gO40TintGreenReloc, gO40TintBlueReloc;
extern Overlay40Command gO40TintDisplayListReloc[];
extern void o40TintCallReloc();

#define O40_SHIFTL(value, shift, width) \
    (((u32)(value) & ((1U << (width)) - 1U)) << (shift))

/* Overlay 40 text +0x534..+0x690. */
void overlay40DrawTintRectangle(Overlay40Command **displayList) {
    Overlay40Command *command;
    Overlay40Position position;
    s32 inverse, red, green, blue;

    if (gO40TintAlphaReloc != 0) {
        o40TintCallReloc(&position.x, &position.y, displayList);
        inverse = 0xFF - gO40TintAlphaReloc;
        red = (gO40TintRedReloc * inverse) >> 8;
        green = (gO40TintGreenReloc * inverse) >> 8;
        blue = (gO40TintBlueReloc * inverse) >> 8;
        command = (*displayList)++;
        command->w0 = 0x06000000;
        command->w1 = (u32)gO40TintDisplayListReloc;
        command = (*displayList)++;
        command->w0 = 0xFA000000;
        command->w1 = (red << 24) | ((green & 0xFF) << 16) |
                      ((blue & 0xFF) << 8) | (gO40TintAlphaReloc & 0xFF);
        command = (*displayList)++;
        command->w1 = 0;
        command->w0 = 0xF6000000 | O40_SHIFTL(position.x, 14, 10) |
                      O40_SHIFTL(position.y, 2, 10);
        o40TintCallReloc(displayList);
        command = (*displayList)++;
        command->w1 = 0xFFFFFFFF;
        command->w0 = 0xFA000000;
    }
}
