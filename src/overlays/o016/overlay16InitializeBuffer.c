#include "PR/ultratypes.h"

extern u8 *gOverlay16Buffer;
extern s32 gOverlay16Phase;
extern s32 gOverlay16Mode;
extern void *overlay16AllocateReloc(s32 size, s32 tag);
extern void overlay16BuildGradientReloc(u8 *output, s32 red, s32 green,
                                        s32 blue, s32 endRed, s32 endGreen,
                                        s32 endBlue);

/* DKR v77/v80 have no donor; JFG only confirms the gradient helper. */
void overlay16InitializeBuffer(u8 *config) {
    gOverlay16Buffer = overlay16AllocateReloc(0x300, 0x87);
    if (gOverlay16Buffer != NULL) {
        overlay16BuildGradientReloc(gOverlay16Buffer, config[0x102], config[0x103],
                                    config[0x104], config[0xFF], config[0x100],
                                    config[0x101]);
        overlay16BuildGradientReloc(gOverlay16Buffer + 0xC0, config[0xFF], config[0x100],
                                    config[0x101], config[0xFC], config[0xFD],
                                    config[0xFE]);
        overlay16BuildGradientReloc(gOverlay16Buffer + 0x180, config[0xFC], config[0xFD],
                                    config[0xFE], config[0xFF], config[0x100],
                                    config[0x101]);
        overlay16BuildGradientReloc(gOverlay16Buffer + 0x240, config[0xFF], config[0x100],
                                    config[0x101], config[0x102], config[0x103],
                                    config[0x104]);
        gOverlay16Phase = 0;
        gOverlay16Mode = config[0xFB];
    }
}
