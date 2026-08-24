#include "PR/ultratypes.h"

extern s32 overlay61RecordSizeReloc(void);
extern void *overlay61AllocReloc(s32 size, s32 tag);
extern s32 overlay61WriteRecordReloc(
    s32 arg0, s32 arg1, void *format, s32 arg3, void *data, s32 size);
extern void overlay61FreeReloc(void *data);
extern u8 gOverlay61RecordFormatReloc[];

#ifdef NON_MATCHING
s32 overlay61WriteCharacter(s32 *source, s32 destination, s32 arg2) {
    s32 size;
    s32 remainder;
    s32 words;
    s32 count;
    s32 *src;
    s32 *dst;
    s32 result;
    s32 *buffer;

    size = overlay61RecordSizeReloc() + 4;
    remainder = size % 32;
    if (remainder != 0) {
        size = (size - remainder) + 32;
    }

    buffer = overlay61AllocReloc(size, 0x85);
    if (buffer != 0) {
        buffer[0] = 0x43484152;
        words = (size - 1) >> 2;
        src = source;
        dst = buffer + 1;
        count = words - 1;
        if (words != 0) {
            do {
                *dst++ = *src++;
            } while (count--);
        }
        result = overlay61WriteRecordReloc(
            destination, -1, gOverlay61RecordFormatReloc, arg2, buffer, size);
        overlay61FreeReloc(buffer);
    } else {
        result = 6;
    }
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o061/overlay61WriteCharacter/func_overlay_061_F00017B8_18C0B80.s")
#endif
