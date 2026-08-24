#include "PR/ultratypes.h"

extern s32 gO96EntryCountReloc;
extern s32 gO96EntriesReloc[16];

void overlay96Register(s32 value) {
    s32 index;

    index = gO96EntryCountReloc;
    while (index-- != 0) {
        if (gO96EntriesReloc[index] == value) {
            return;
        }
    }
    if (gO96EntryCountReloc < 16) {
        gO96EntriesReloc[gO96EntryCountReloc] = value;
        gO96EntryCountReloc++;
    }
}
