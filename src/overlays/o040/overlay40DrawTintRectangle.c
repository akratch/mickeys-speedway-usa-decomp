#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

extern s16 gO40TintAlphaReloc, gO40TintRedReloc;
extern s16 gO40TintGreenReloc, gO40TintBlueReloc;
extern Gfx gO40TintDisplayListReloc[];
extern void o40TintCallReloc();

/* Overlay 40 text +0x534..+0x690. */
void overlay40DrawTintRectangle(Gfx **displayList) {
    s32 inverse, red, green, blue;
    s32 x;
    s32 y;

    if (gO40TintAlphaReloc != 0) {
        o40TintCallReloc(&x, &y, displayList);
        inverse = 0xFF - gO40TintAlphaReloc;
        red = (gO40TintRedReloc * inverse) >> 8;
        green = (gO40TintGreenReloc * inverse) >> 8;
        blue = (gO40TintBlueReloc * inverse) >> 8;
        gSPDisplayList((*displayList)++, gO40TintDisplayListReloc);
        gDPSetPrimColor((*displayList)++, 0, 0, red, green, blue,
                        gO40TintAlphaReloc);
        gDPFillRectangle((*displayList)++, 0, 0, x, y);
        o40TintCallReloc(displayList);
        gDPSetPrimColor((*displayList)++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    }
}
