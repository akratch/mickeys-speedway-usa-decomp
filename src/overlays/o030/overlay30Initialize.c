#include "PR/ultratypes.h"

/* Module initialization reconstructed from Mickey; exact DKR/JFG scans are negative. */
extern void overlay30SetupReloc(s32, s32, s32, s32, s32);
extern void *overlay30InitPoolReloc(s32, s32);
extern void *overlay30AllocateReloc(s32, s32);
extern void *overlay30LoadResourceReloc(s32);
extern s32 overlay30ResourceSizeReloc(s32);
extern void overlay30ResetReloc(void);

extern void *gOverlay30WorkBuffer;
extern s32 gOverlay30WorkBufferSize;
extern s32 *gOverlay30TableResource;
extern s32 gOverlay30TableLength;
extern void *gOverlay30PixelResource;
extern s32 *gOverlay30PixelHeader;
extern s32 gOverlay30PixelHeaderLength;
extern f32 gOverlay30Scale;
extern void *gOverlay30WordTable;
extern s16 *gOverlay30HalfwordTable;
extern void *gOverlay30LargeBuffer;
extern void *gOverlay30Pool;
extern void *gOverlay30SmallBuffer;
extern s16 *gOverlay30SentinelResource;
extern s32 gOverlay30SentinelIndex;
extern void *gOverlay30Buffer320;
extern void *gOverlay30Buffer20;
extern void *gOverlay30Buffer80;

extern void overlay30TransposePixels(u8 *data, s32 length);

void overlay30Initialize(void) {
    s32 i;

    overlay30SetupReloc(0xAA, 0x55, 0, -0x2000, 0);
    gOverlay30Pool = overlay30InitPoolReloc(0x19000, 0x200);
    gOverlay30Buffer320 = overlay30AllocateReloc(0x320, 0x8B);
    gOverlay30Buffer20 = overlay30AllocateReloc(0x20, 0x8B);
    gOverlay30Buffer80 = overlay30AllocateReloc(0x80, 0x8B);

    gOverlay30SentinelResource = overlay30LoadResourceReloc(0x2E);
    gOverlay30SentinelIndex = (overlay30ResourceSizeReloc(0x2E) >> 1) - 1;
    while (gOverlay30SentinelResource[gOverlay30SentinelIndex] == 0) {
        gOverlay30SentinelIndex--;
    }

    gOverlay30WorkBuffer = overlay30AllocateReloc((gOverlay30WorkBufferSize = 0x2000), 0x8F);
    gOverlay30TableResource = overlay30LoadResourceReloc(0x2C);
    gOverlay30TableLength = 0;
    while (gOverlay30TableResource[gOverlay30TableLength] != -1) {
        gOverlay30TableLength++;
    }
    gOverlay30TableLength--;

    gOverlay30WordTable = overlay30AllocateReloc(gOverlay30TableLength * 4, 0x8B);
    gOverlay30HalfwordTable = overlay30AllocateReloc(gOverlay30TableLength * 2, 0x8B);
    for (i = 0; i < gOverlay30TableLength; i++) {
        gOverlay30HalfwordTable[i] = 0;
    }

    gOverlay30PixelResource = overlay30LoadResourceReloc(0x18);
    gOverlay30PixelHeader = overlay30LoadResourceReloc(0x19);
    gOverlay30PixelHeaderLength = 0;
    while (gOverlay30PixelHeader[gOverlay30PixelHeaderLength] != -1) {
        gOverlay30PixelHeaderLength++;
    }

    overlay30TransposePixels((u8 *)gOverlay30PixelResource + (gOverlay30PixelHeader[5] * 4),
                             (gOverlay30PixelHeader[6] - gOverlay30PixelHeader[5]) * 4);
    gOverlay30LargeBuffer = overlay30AllocateReloc(0x800, 0x8B);
    gOverlay30SmallBuffer = overlay30AllocateReloc(0x100, 0x8B);
    gOverlay30Scale = 2.0f;
    overlay30ResetReloc();
}
