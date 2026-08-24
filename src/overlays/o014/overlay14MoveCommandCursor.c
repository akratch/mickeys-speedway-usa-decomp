#include "PR/ultratypes.h"

typedef struct Overlay14SelectionEntry {
    s16 field0;
    s16 field2;
    s16 field4;
    s16 field6;
    s16 value;
    s16 fieldA;
    s16 fieldC;
    s16 fieldE;
} Overlay14SelectionEntry;

typedef struct Overlay14SelectionList {
    s16 count;
    s16 selected;
    s16 field4;
    s16 field6;
    s16 firstValue;
} Overlay14SelectionList;

extern Overlay14SelectionList *gOverlay14SelectionList;
extern s32 overlay14IsSelectable(s16 value);

#ifdef NON_MATCHING
s32 overlay14MoveCommandCursor(s32 direction) {
    Overlay14SelectionList *list;
    Overlay14SelectionEntry *entry;
    s32 index;
    s32 eligible;

    list = gOverlay14SelectionList;
    eligible = 0;
    if (list != 0) {
        index = 0;
        entry = (Overlay14SelectionEntry *)list;
        if (list->count > 0) {
            do {
                index++;
                entry++;
                if (overlay14IsSelectable(entry[-1].value)) {
                    eligible++;
                }
            } while (index < list->count);
        }

        if (eligible != 0) {
            index = list->selected;
            if (direction > 0) {
                do {
                    index++;
                    if (index >= list->count) {
                        break;
                    }
                } while (!overlay14IsSelectable(
                    ((Overlay14SelectionEntry *)list)[index].value));
            } else if (direction < 0) {
                index--;
backward_loop:
                if (index < 0) {
                    goto selection_done;
                }
                if (!overlay14IsSelectable(
                        ((Overlay14SelectionEntry *)list)[index].value)) {
                    index--;
                    goto backward_loop;
                }
                goto selection_done;
            } else if (!overlay14IsSelectable(
                           ((Overlay14SelectionEntry *)list)[index].value)) {
                index--;
zero_backward_loop:
                if (index < 0) {
                    goto scan_forward;
                }
                if (!overlay14IsSelectable(
                        ((Overlay14SelectionEntry *)list)[index].value)) {
                    index--;
                    goto zero_backward_loop;
                }
                goto selection_done;
scan_forward:
                index = list->selected;
                do {
                    index++;
                    if (index >= list->count) {
                        break;
                    }
                } while (!overlay14IsSelectable(
                    ((Overlay14SelectionEntry *)list)[index].value));
            }

selection_done:
            if ((index >= 0) && (index < list->count)) {
                list->selected = index;
            }
        }
    }
    return eligible;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o014/overlay14MoveCommandCursor/func_overlay_014_F0000578_186FE50.s")
#endif
