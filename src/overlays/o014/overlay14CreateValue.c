#include "PR/ultratypes.h"

typedef struct Overlay14ValueSlot {
    s32 key;
    void *volatile value;
} Overlay14ValueSlot;

extern Overlay14ValueSlot gOverlay14Slots28[];
extern Overlay14ValueSlot gOverlay14FreeSlots28[];
extern Overlay14ValueSlot gOverlay14ChosenSlots28[];
extern Overlay14ValueSlot gOverlay14SlotsEnd128[];
extern void *gOverlay14SlotsActive2C;
extern s32 gOverlay14SlotCountE8;

extern s32 overlay14SelectKind(void);
extern void *overlay14LoadRelocatedValue(s32 key, s32 kind);
extern void *overlay14LoadAsset(s32 key, s32 kind);

void *overlay14CreateValue(s32 key, s32 alternate) {
    Overlay14ValueSlot *slot;
    void *value;
    s32 index;
    s32 kind;
    Overlay14ValueSlot *volatile chosen;

    slot = gOverlay14Slots28;
scan_loop:
    value = slot->value;
    if ((value != 0) && (slot->key == key)) {
        return value;
    }
    slot++;
    if (slot < gOverlay14SlotsEnd128) {
        goto scan_loop;
    }

    index = 0;
    slot = gOverlay14FreeSlots28;
    if (gOverlay14SlotsActive2C != 0) {
        do {
            index++;
            if (index >= 32) {
                break;
            }
            slot = &gOverlay14FreeSlots28[index];
        } while (slot->value != 0);
    }
    if (index >= 32) {
        return 0;
    }
    chosen = &gOverlay14ChosenSlots28[index];

    switch (overlay14SelectKind()) {
        case 1:
            kind = 0xC;
            break;
        case 2:
            kind = 0xE;
            break;
        case 3:
            kind = 0x10;
            break;
        case 5:
            kind = 0x12;
            break;
        default:
            kind = 0xA;
            break;
    }

    slot = chosen;
    if (alternate != 1) {
        slot->value = overlay14LoadRelocatedValue(key, kind);
    } else {
        slot->value = overlay14LoadAsset(key, kind);
    }
    value = slot->value;
    if (value != 0) {
        slot->key = key;
        value = slot->value;
        gOverlay14SlotCountE8++;
    }
    return value;
}
