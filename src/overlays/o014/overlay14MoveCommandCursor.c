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

extern Overlay14SelectionList *gOverlay14CommandHeader;
extern s32 overlay14ReturnOneCallbackA(s16 value);

s32 overlay14MoveCommandCursor(s32 direction) {
    Overlay14SelectionList *list;
    Overlay14SelectionEntry *entry;
    s32 index;
    s32 eligible;
    s16 value;

    list = gOverlay14CommandHeader;
    eligible = 0;
    if (list != 0) {
        index = 0;
        entry = (Overlay14SelectionEntry *)list;
        if (list->count > 0) {
            do {
                value = entry->value;
                index++;
                entry++;
                if (overlay14ReturnOneCallbackA(value)) {
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
                } while (!overlay14ReturnOneCallbackA(
                    ((Overlay14SelectionEntry *)list)[index].value));
            } else if (direction < 0) {
                do {
                    index--;
                    if (index < 0) {
                        break;
                    }
                } while (!overlay14ReturnOneCallbackA(
                    ((Overlay14SelectionEntry *)list)[index].value));
            } else if (!overlay14ReturnOneCallbackA(
                           ((Overlay14SelectionEntry *)list)[index].value)) {
                do {
                    index--;
                    if (index < 0) {
                        break;
                    }
                } while (!overlay14ReturnOneCallbackA(
                    ((Overlay14SelectionEntry *)list)[index].value));
                if (index < 0) {
                    index = list->selected;
                    do {
                        index++;
                        if (index >= list->count) {
                            break;
                        }
                    } while (!overlay14ReturnOneCallbackA(
                        ((Overlay14SelectionEntry *)list)[index].value));
                }
            }

            if ((index >= 0) && (index < list->count)) {
                list->selected = index;
            }
        }
    }
    return eligible;
}
