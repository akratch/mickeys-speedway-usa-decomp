#include "PR/ultratypes.h"

extern s32 gO96EntryCountReloc;
extern s32 gO96EntriesReloc[16];

void overlay96Unregister(s32 value) {
    s32 count;
    s32 index;
    s32 *entry;
    s32 *end;

    count = gO96EntryCountReloc;
    index = count;
    if (index != 0) {
        index--;
        entry = &gO96EntriesReloc[index];
        do {
            if (value == *entry) {
                count--;
                gO96EntryCountReloc = count;
                if (index < count) {
                    end = &gO96EntriesReloc[count];
                    do {
                        *entry = entry[1];
                        entry++;
                    } while (entry < end);
                }
                return;
            }
            entry--;
        } while (index-- != 0);
    }
}
