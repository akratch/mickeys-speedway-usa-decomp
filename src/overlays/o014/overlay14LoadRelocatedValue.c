#include "PR/ultratypes.h"

extern u32 *gOverlay14OffsetsF4;
extern void *overlay14AllocateReloc(s32 size, s32 tag);
extern void overlay14LoadReloc(s32 source, void *destination, s32 start, s32 size);

/* Workbench: allocation-mismatch, 53 raw differing words, first mismatch +0x04.
 * Exact 94-instruction jump-table CFG; target frame is -0x28 versus candidate -0x30.
 * Shape-exact/permuter-ready aside from frame/register scheduling; no CFG gap remains. */
#ifdef NON_MATCHING
void *overlay14LoadRelocatedValue(s32 index, s32 source) {
    u8 *buffer;
    u8 *cursor;
    s32 start;
    s32 size;

    start = gOverlay14OffsetsF4[index];
    size = gOverlay14OffsetsF4[index + 1] - start;
    buffer = overlay14AllocateReloc(size, 0x85);
    if (buffer != NULL) {
        overlay14LoadReloc(source, buffer, start, size);
        cursor = buffer;
        while (*cursor != 0) {
            u8 *command;

            command = cursor;
            switch (*cursor) {
                case 1:
                    cursor += 4;
                    break;
                case 2:
                    cursor += 4;
                    *(u32 *)command =
                        (u32)buffer + ((*(u32 *)command & 0xFFFFFF) | 0x80000000);
                    *command = 2;
                    break;
                case 3:
                    cursor += 4;
                    *(u32 *)command =
                        (u32)buffer + ((*(u32 *)command & 0xFFFFFF) | 0x80000000);
                    *command = 3;
                    break;
                case 4:
                    cursor += 4;
                    *(u32 *)command =
                        (u32)buffer + ((*(u32 *)command & 0xFFFFFF) | 0x80000000);
                    *command = 4;
                    break;
                case 5:
                    cursor += 4;
                    *(u32 *)command =
                        (u32)buffer + ((*(u32 *)command & 0xFFFFFF) | 0x80000000);
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
