#include "PR/ultratypes.h"

extern s32 gOverlay101OrderCount;
extern void *gOverlay101Order[];
extern void overlay101DrawSlotReloc(void *context, s32 x, s32 y, void *slot);
extern void overlay101DrawMarkerReloc(void *context, s32 x, s32 y,
                                     s32 width, s32 height);

void overlay101DrawSlots(void *context, s32 x, s32 y) {
    s32 count;
    s32 first;
    void **slot;

    count = gOverlay101OrderCount;
    first = 1;
    if (count--) {
        slot = &gOverlay101Order[count];
        do {
            overlay101DrawSlotReloc(context, x, y, *slot);
            if (first != 0) {
                overlay101DrawMarkerReloc(context, x, y, 0x6E, 0x4D);
                first = 0;
            }
            slot--;
        } while (count--);
    }
}
