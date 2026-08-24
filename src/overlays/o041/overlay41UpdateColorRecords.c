#include "PR/ultratypes.h"

typedef struct Overlay41ColorRecord {
    s32 handle;
    u8 targetRed;
    u8 red;
    u8 targetGreen;
    u8 green;
    u8 targetBlue;
    u8 blue;
    u8 targetAlpha;
    u8 alpha;
    s16 remaining;
    s16 duration;
} Overlay41ColorRecord;

extern Overlay41ColorRecord gOverlay41ColorRecords[12];
extern void overlay41SetColor(s32 handle, s32 red, s32 green, s32 blue);
extern void overlay41SetAlpha(s32 handle, s32 alpha);

extern u8 D_80000038[];
extern u8 D_80000039[];
extern u8 D_8000003A[];
extern u8 D_8000003B[];
extern void func_overlay_041_F0000000_1887338(void);

#ifdef NON_MATCHING
/* Preserve runtime-only relocation identities in a removable private island. */
static void *const overlay41RuntimeSymbols[] = {
    D_80000038,
    D_80000039,
    D_8000003A,
    D_8000003B,
    func_overlay_041_F0000000_1887338,
};

void func_overlay_041_F0000124_188745C(s32 amount) {
    Overlay41ColorRecord *record;
    s32 i;
    s32 handle;
    s32 red;
    s32 green;
    s32 blue;
    s32 alpha;
    s32 initialRed;
    s32 initialGreen;
    s32 initialBlue;
    s32 initialAlpha;
    s32 factor;
    u8 *direct;

    record = gOverlay41ColorRecords;
    i = 11;
    do {
        handle = record->handle;
        if (handle != 0) {
            initialRed = record->red;
            initialGreen = record->green;
            initialBlue = record->blue;
            initialAlpha = record->alpha;
            red = initialRed;
            green = initialGreen;
            blue = initialBlue;
            alpha = initialAlpha;
            if (amount >= record->remaining) {
                record->handle = 0;
            } else {
                record->remaining -= amount;
                factor = (record->remaining << 16) / record->duration;
                red += ((record->targetRed - initialRed) * factor) >> 16;
                green += ((record->targetGreen - initialGreen) * factor) >> 16;
                blue += ((record->targetBlue - initialBlue) * factor) >> 16;
                alpha += ((record->targetAlpha - initialAlpha) * factor) >> 16;
            }
            if (handle < 0) {
                overlay41SetColor(handle, red & 0xFF, green & 0xFF, blue & 0xFF);
                overlay41SetAlpha(handle, alpha & 0xFF);
            } else {
                direct = (u8 *)((u32)handle | 0x80000000U);
                direct[0x38] = red;
                direct[0x39] = green;
                direct[0x3A] = blue;
                direct[0x3B] = alpha;
            }
        }
        record++;
    } while (i--);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o041/overlay41UpdateColorRecords/func_overlay_041_F0000124_188745C.s")
#endif
