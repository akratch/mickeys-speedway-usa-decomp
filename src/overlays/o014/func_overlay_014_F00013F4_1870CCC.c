#include "PR/ultratypes.h"

extern s32 gOverlay14ValueC0;
extern s32 gOverlay14ValueDC;
extern s32 gOverlay14HandleE0;
extern s32 gOverlay14ModeE4;
extern s32 gOverlay14ValueEC;
extern s32 gOverlay14ValueF8;
extern s32 gOverlay14Value114;
extern s32 gOverlay14Args2C;
extern s32 gOverlay14Args30;
extern s32 gOverlay14DataBase;

extern void overlay14BuildPanel(s32 context, void *base, s32 width, s32 height,
                                s32 extra0, s32 extra1, s32 extra2);
extern s32 overlay14CreateHandle(s32 context, s32 valueF8, s32 valueDC);
extern void overlay14DrawPrimitive();

void func_overlay_014_F00013F4_1870CCC(s32 context) {
    s32 mode;

    overlay14BuildPanel(context, &gOverlay14DataBase, 0x5C, 0x14, 0xD0, 0x58,
                        (gOverlay14ValueC0 * 0xA0) >> 8);
    gOverlay14HandleE0 = overlay14CreateHandle(context, gOverlay14ValueF8,
                                               gOverlay14ValueDC);
    overlay14DrawPrimitive(0, 0, 0, 0);
    overlay14DrawPrimitive(0xFF, 0xC0, 0, 0xFF,
                           (gOverlay14ValueC0 * 0xFF) >> 8);
    if (gOverlay14ValueDC != 0) {
        overlay14DrawPrimitive(context, 0xC4, 0x14, &gOverlay14Args2C, 0xC);
    }
    if ((gOverlay14HandleE0 != 0) ||
        ((mode = gOverlay14ModeE4, mode == 4) &&
         (gOverlay14ValueEC >= 2)) ||
        ((mode == 3) && (gOverlay14Value114 != 0))) {
        overlay14DrawPrimitive(context, 0xC4, 0x6C, &gOverlay14Args30, 0xC);
    }
}
