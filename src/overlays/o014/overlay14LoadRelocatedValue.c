#include "PR/ultratypes.h"

extern u32 *gOverlay14OffsetsF4;
extern void *overlay14AllocateReloc(s32 size, s32 tag);
extern void overlay14LoadReloc(s32 source, void *destination, s32 start, s32 size);

#ifdef NON_MATCHING
void *overlay14LoadRelocatedValue(s32 index, s32 source) {
    u8 *buffer;
    u8 *cursor;
    u8 *command;
    u32 encoded;
    u32 *offsets;
    s32 start;
    s32 size;

    offsets = gOverlay14OffsetsF4;
    start = offsets[index];
    size = offsets[index + 1] - start;
    buffer = overlay14AllocateReloc(size, 0x85);
    if (buffer != NULL) {
        overlay14LoadReloc(source, buffer, start, size);
        cursor = buffer;
        while (*cursor != 0) {
            command = cursor;
            switch (*cursor) {
                case 1:
                    cursor += 4;
                    break;
                case 2:
                    encoded = *(u32 *)cursor;
                    cursor += 4;
                    *(u32 *)command =
                        (u32)buffer + ((encoded & 0xFFFFFF) | 0x80000000);
                    *command = 2;
                    break;
                case 3:
                    encoded = *(u32 *)cursor;
                    cursor += 4;
                    *(u32 *)command =
                        (u32)buffer + ((encoded & 0xFFFFFF) | 0x80000000);
                    *command = 3;
                    break;
                case 4:
                    encoded = *(u32 *)cursor;
                    cursor += 4;
                    *(u32 *)command =
                        (u32)buffer + ((encoded & 0xFFFFFF) | 0x80000000);
                    *command = 4;
                    break;
                case 5:
                    encoded = *(u32 *)cursor;
                    cursor += 4;
                    *(u32 *)command =
                        (u32)buffer + ((encoded & 0xFFFFFF) | 0x80000000);
                    *command = 5;
                    break;
                case 6:
                    cursor += 8;
                    break;
                case 7:
                    cursor += 8;
                    break;
            }
        }
    }
    return buffer;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o014/overlay14LoadRelocatedValue/func_overlay_014_F000087C_1870154.s")
#endif
