#include "PR/ultratypes.h"

extern u8 gOverlay61SavePathReloc[];
extern s32 overlay61StorageReadyReloc(s32);
extern s32 overlay61OpenReloc(s32, void *, s32 *, void **);
extern s32 overlay61GetSizeReloc(s32, void *, s32 *);
extern s32 *overlay61AllocReloc(s32, s32);
extern s32 overlay61ReadReloc(s32, void *, void *, s32);
extern s32 overlay68RecordSizeReloc(s32);
extern void overlay61FreeReloc(void *);
extern void overlay61CloseReloc(s32);

/* Workbench verdict: mixed(structural:3, schedule:10, register:10); 14/92 words, exact frame.
 * Lever: swapping src/dst declaration order was neutral; context/flag checks and prior branch probes stayed in the same basin.
 * Remaining: target a0/a1 pool color and branch-likely shape; assembly fallback stays canonical. */

#ifdef NON_MATCHING
s32 func_overlay_061_F0001648_18C0A10(s32 *output, s32 device, s32 *slot) {
    s32 copySize;
    s32 result;
    s32 size;
    void *handle;
    s32 *buffer;
    s32 *src;
    s32 *dst;

    result = overlay61StorageReadyReloc(device);
    if (result == 0) {
        result = overlay61OpenReloc(
            device, gOverlay61SavePathReloc, slot, &handle);
        if (result == 0) {
            result = overlay61GetSizeReloc(device, handle, &size);
            if (size >= 4) {
                buffer = overlay61AllocReloc(size, 0x85);
                if (buffer != 0) {
                    result = overlay61ReadReloc(device, handle, buffer, size);
                    if (result == 0) {
                        if (buffer[0] == 0x43484152) {
                            copySize = size - 4;
                            if (overlay68RecordSizeReloc(copySize) < copySize) {
                                copySize = overlay68RecordSizeReloc(copySize);
                            }
                            copySize = (copySize + 3) >> 2;
                            src = buffer + 1;
                            dst = output;
                            while (copySize--) {
                                *dst++ = *src++;
                            }
                            output[3] = (s32)(output + 4);
                        } else {
                            result = 5;
                        }
                    }
                    overlay61FreeReloc(buffer);
                }
            } else {
                result = 6;
            }
        }
    }
    overlay61CloseReloc(device);
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o061/func_overlay_061_F0001648_18C0A10/func_overlay_061_F0001648_18C0A10.s")
#endif
