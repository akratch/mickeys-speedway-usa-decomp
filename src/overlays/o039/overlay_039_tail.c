#include "PR/ultratypes.h"

/* Overlay 39 tail exports; exact DKR and JFG scans are negative. */
extern u8 *gOverlay39Buffer;
extern u8 gOverlay39State;

void overlay39BeginReloc(void);
void overlay39SendReloc(u8 *buffer, s32 size);
void overlay39SelectReloc(s32 channel, u8 *buffer);
void overlay39FinishReloc(void);

void overlay39Reset(s32 unused) {
    overlay39BeginReloc();
    gOverlay39State = 0xFE;
    overlay39SendReloc(gOverlay39Buffer, 0x40);
    overlay39SelectReloc(0, gOverlay39Buffer);
    overlay39FinishReloc();
}

void overlay39Read(u8 *destination) {
    u8 *buffer = gOverlay39Buffer;
    s32 offset;

    for (offset = 0; offset != 0x10; offset += 4) {
        destination[offset] = buffer[offset + 0x30];
        destination[offset + 1] = buffer[offset + 0x31];
        destination[offset + 2] = buffer[offset + 0x32];
        destination[offset + 3] = buffer[offset + 0x33];
    }
}
