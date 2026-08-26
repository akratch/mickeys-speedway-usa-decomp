#include "PR/ultratypes.h"

typedef struct Overlay41QueueEntry {
    s16 timer;
    s16 value2;
    s16 value4;
    s16 value6;
    s8 value8;
    s8 value9;
    s8 valueA;
    u8 active;
} Overlay41QueueEntry;

extern u8 gOverlay41QueueActive0Read[];
extern u8 gOverlay41QueueTimer0Read[];
extern u8 gOverlay41QueueTimer0Write[];
extern u8 gOverlay41QueueValue2_0[];
extern u8 gOverlay41QueueValue4_0[];
extern u8 gOverlay41QueueValue6_0[];
extern u8 gOverlay41QueueValue8_0[];
extern u8 gOverlay41QueueValue9_0[];
extern u8 gOverlay41QueueValueA_0[];
extern u8 gOverlay41QueueActive0Write[];
extern u8 gOverlay41QueueEntries[];

#define entry ((volatile u8 *)cursor)

/* Workbench: mixed(structural:13, register:12, constant:3), exact 105 instructions/29 words, first +0x8.
 * Levers: cursor/+12 pointer and offset rewrites; all regressed, with the target's delayed cursor web intact.
 * Remains: target block order and one-cursor address routing. */
#ifdef NON_MATCHING
void func_overlay_041_F000195C_1888C94(s32 value2, s32 timer, s32 value4,
                                       s32 value6, s32 value8, s32 value9,
                                       s32 valueA) {
    u32 cursor;

    cursor = (u32)gOverlay41QueueEntries;
    if (gOverlay41QueueActive0Read[11] == 0 &&
        *(s16 *)(gOverlay41QueueTimer0Read + 0) <= 0) {
        *(s16 *)(gOverlay41QueueValue2_0 + 2) = value2;
        *(s16 *)(gOverlay41QueueTimer0Write + 0) = timer;
        *(s16 *)(gOverlay41QueueValue4_0 + 4) = value4;
        *(s16 *)(gOverlay41QueueValue6_0 + 6) = value6;
        gOverlay41QueueValue8_0[8] = value8;
        gOverlay41QueueValue9_0[9] = value9;
        gOverlay41QueueValueA_0[10] = valueA;
        gOverlay41QueueActive0Write[11] = 0;
        return;
    } else if (entry[11] == 0 && *(s16 *)(entry + 0) <= 0) {
        *(s16 *)(entry + 2) = value2;
        *(s16 *)(entry + 0) = timer;
        *(s16 *)(entry + 4) = value4;
        *(s16 *)(entry + 6) = value6;
        entry[8] = value8;
        entry[9] = value9;
        entry[11] = 0;
        entry[10] = valueA;
        return;
    }
    cursor += 12;
    if (entry[11] == 0 && *(s16 *)(entry + 0) <= 0) {
        *(s16 *)(entry + 2) = value2;
        *(s16 *)(entry + 0) = timer;
        *(s16 *)(entry + 4) = value4;
        *(s16 *)(entry + 6) = value6;
        entry[8] = value8;
        entry[9] = value9;
        entry[11] = 0;
        entry[10] = valueA;
        return;
    }
    cursor += 12;
    if (entry[11] == 0 && *(s16 *)(entry + 0) <= 0) {
        *(s16 *)(entry + 2) = value2;
        *(s16 *)(entry + 0) = timer;
        *(s16 *)(entry + 4) = value4;
        *(s16 *)(entry + 6) = value6;
        entry[8] = value8;
        entry[9] = value9;
        entry[11] = 0;
        entry[10] = valueA;
        return;
    }
    cursor += 12;
    if (entry[11] == 0 && *(s16 *)(entry + 0) <= 0) {
        *(s16 *)(entry + 2) = value2;
        *(s16 *)(entry + 0) = timer;
        *(s16 *)(entry + 4) = value4;
        *(s16 *)(entry + 6) = value6;
        entry[8] = value8;
        entry[9] = value9;
        entry[11] = 0;
        entry[10] = valueA;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o041/overlay41EnqueueTransition/func_overlay_041_F000195C_1888C94.s")
#endif
