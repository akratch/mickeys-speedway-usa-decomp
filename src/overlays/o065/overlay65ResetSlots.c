#ifndef PERMUTER
#include "PR/ultratypes.h"
#else
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef short s16;
#endif

extern u8 gOverlay65Flag0;
extern u8 gOverlay65Flag1;
typedef struct Overlay65Slot {
    u8 pad000[0x7C];
    u8 flag07C;
    u8 pad07D[0x7F];
    u8 flag0FC;
    u8 pad0FD[0x7F];
    u8 flag17C;
    u8 pad17D[0x7F];
    u8 flag1FC;
    u8 pad1FD[3];
} Overlay65Slot;

extern Overlay65Slot gOverlay65Slots[12];
extern s16 gOverlay65Ready;

/* DKR v77/v80 and JFG contain no exact donor for this fixed-stride reset. */
void overlay65ResetSlots(void) {
    register s32 index;

    gOverlay65Flag0 = 0;
    for (index = 0, gOverlay65Flag1 = 0; index < 12; index++) {
        gOverlay65Slots[index].flag0FC = 0;
        gOverlay65Slots[index].flag17C = 0;
        gOverlay65Slots[index].flag1FC = 0;
        gOverlay65Slots[index].flag07C = 0;
    }
    gOverlay65Ready = 1;
}
