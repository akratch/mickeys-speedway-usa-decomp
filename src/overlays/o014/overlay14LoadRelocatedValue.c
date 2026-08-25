#include "PR/ultratypes.h"

extern u32 *gOverlay14OffsetsF4;
extern void *overlay14AllocateReloc(s32 size, s32 tag);
extern void overlay14LoadReloc(s32 source, void *destination, s32 start, s32 size);

/*
 * Plateau (2026-08-25): best -O2/-mips2 is size/opcode-exact with 53 raw words; first raw mismatch +0x4, frame 0x30 vs 0x28.
 * Direct offset-table reads and block-scoped command heal opening colors; alias/type/scope/early-return variants do not close the loop web.
 * A 40-minute one-worker permuter improved its -mips1 score 2230 to 1915 only with synthetic temporaries.
 */
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
