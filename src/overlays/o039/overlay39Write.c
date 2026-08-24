#include "PR/ultratypes.h"

/* DKR v77/v80 exact-object and source-signature scans are negative. */
extern u8 *gOverlay39Buffer;
extern u8 gOverlay39State;

void overlay39BeginReloc(void);
void overlay39SendReloc(u8 *buffer, s32 size);
void overlay39SelectReloc(s32 channel, u8 *buffer);
void overlay39FinishReloc(void);

void overlay39Write(u8 *source, s32 unused) {
    /* Preserves the original local-slot allocation; emits no instructions. */
    volatile s32 reserved;
    u8 *buffer;
    u32 offset;
    u32 fill;

    buffer = gOverlay39Buffer;
    for (offset = 0, fill = 0xFF; ;) {
        buffer[offset++] = fill;
        if (offset >= 0x2E) {
            break;
        }
    }
    buffer[0x2E] = 0xF;
    buffer[0x2F] = 0xF;
    for (offset = 0; offset != 0x10; offset += 4) {
        buffer[offset + 0x30] = source[offset];
        buffer[offset + 0x31] = source[offset + 1];
        buffer[offset + 0x32] = source[offset + 2];
        buffer[offset + 0x33] = source[offset + 3];
    }
    overlay39BeginReloc();
    gOverlay39State = 0xFE;
    overlay39SendReloc(buffer, 0x40);
    overlay39SelectReloc(1, buffer);
    overlay39FinishReloc();
}
