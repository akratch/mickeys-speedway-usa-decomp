#include "PR/ultratypes.h"

typedef struct Overlay101Slot {
    u8 pad00[8];
    u8 state;
    u8 value;
    u8 pad0A[0x12];
} Overlay101Slot;

extern Overlay101Slot gOverlay101Slots[];
extern void overlay101PlaySoundReloc(s32 soundId, s32 arg1);

void overlay101ActivateSlot(s32 index) {
    Overlay101Slot *slot;
    slot = &gOverlay101Slots[index];
    if (slot->state == 0) {
        overlay101PlaySoundReloc(0x1F7, 0);
        slot->state = 1;
        slot->value = 0;
    } else if (slot->state == 3) {
        slot->state = 1;
    }
}
