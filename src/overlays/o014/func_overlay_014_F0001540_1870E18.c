#include "PR/ultratypes.h"

typedef struct Overlay14Entry {
    s16 count;
    s16 selected;
    u8 *text;
    s16 kind;
    u8 padA[6];
} Overlay14Entry;

extern s32 gOverlay14ValueC0;
extern s32 gOverlay14DataBase;
extern s32 gOverlay14Args34;
extern Overlay14Entry *gOverlay14Entries;
extern void overlay14BuildPanel(s32, void *, s32, s32, s32, s32, s32);
extern s32 overlay14Dispatch();
extern s32 overlay14ValidateEntry(s16);

#ifdef NON_MATCHING
void func_overlay_014_F0001540_1870E18(s32 context) {
    s32 index, entryOffset, opacity;
    void *drawArg;
    u8 *cursor;
    s32 cellWidth, remaining, x, y, first;
    u8 saved;
    Overlay14Entry *entry;
    overlay14BuildPanel(context, &gOverlay14DataBase, 0x5C, 0x14, 0xD0, 0x58,
                        (gOverlay14ValueC0 * 0xA0) >> 8);
    index = 0;
    cellWidth = overlay14Dispatch(2);
    remaining = (0x58 / cellWidth) - 1;
    overlay14Dispatch(2);
    overlay14Dispatch(0, 0, 0, 0);
    opacity = (gOverlay14ValueC0 * 0xFF) >> 8;
    y = ((0x58 - (remaining * cellWidth)) >> 1) + 0x14;
    if ((gOverlay14Entries->count > 0) && (remaining > 0)) {
        entryOffset = index * 0x10;
loop_entry:
        first = 1; x = 0x60;
        if (index == gOverlay14Entries->selected)
            overlay14Dispatch(0xFF, 0xFF, 0xFF, 0xFF, opacity);
        else
            overlay14Dispatch(0, 0xC0, 0xC0, 0xFF, opacity);
        entry = (Overlay14Entry *)((u8 *)gOverlay14Entries + entryOffset);
        cursor = entry->text;
        if (overlay14ValidateEntry(entry->kind) == 0) cursor = 0;
        if ((cursor != 0) && (remaining > 0)) {
            do {
                cursor = (u8 *)overlay14Dispatch(2, cursor, 0xC8, &drawArg, 0);
                if (cursor != 0) {
                    saved = *cursor; *cursor = 0;
                    overlay14Dispatch(context, x, y, drawArg, 0);
                    *cursor = saved; y += cellWidth;
                    if (first != 0) { x += 8; first = 0; }
                    remaining--;
                }
            } while ((cursor != 0) && (remaining > 0));
        }
        index++;
        entryOffset += 0x10;
        if ((index < gOverlay14Entries->count) && (remaining > 0)) goto loop_entry;
    }
    overlay14Dispatch(0, 0, 0, 0);
    overlay14Dispatch(0xFF, 0xC0, 0, 0xFF, opacity);
    if ((index < gOverlay14Entries->count) || (cursor != 0))
        overlay14Dispatch(context, 0xC4, 0x6C, &gOverlay14Args34, 0xC);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o014/func_overlay_014_F0001540_1870E18/func_overlay_014_F0001540_1870E18.s")
#endif
