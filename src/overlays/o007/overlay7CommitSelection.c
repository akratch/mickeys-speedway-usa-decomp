#include "PR/ultratypes.h"

typedef struct Overlay7Pair {
    u16 key;
    u16 value;
} Overlay7Pair;

extern u8 gOverlay7DispatchModeReloc;
extern u8 gOverlay7DispatchData[];
extern void *gOverlay7DispatchObject;
extern void *gOverlay7Selected;
extern s32 overlay7LookupReloc(s32 arg0, u16 value);
extern void overlay7ObjectReloc(void *object);
/* The shipped runtime relocation table binds this local semantic role at load. */
extern void func_overlay_007_F0000CCC_185CB54(void *selected);
extern void overlay7CommitReloc(u16 value, void *argument);
extern u16 gOverlay7CommitArgument;

void overlay7CommitSelection(s32 selection) {
    Overlay7Pair *pair;
    s32 remaining;
    u16 value;

    if (gOverlay7DispatchModeReloc & 1) {
        switch (selection) {
        case 29:
            value = 0xCF;
            break;
        case 30:
            value = 0xF5;
            break;
        case 31:
            value = 0x116;
            break;
        default:
            value = *(u16 *)&gOverlay7DispatchData[
                0x754 + selection * 6 + overlay7LookupReloc(0, 2) * 2];
            break;
        }
        pair = (Overlay7Pair *)&gOverlay7DispatchData[0x8F4];
        remaining = 11;
        do {
            if (value == pair->key) {
                value = value + overlay7LookupReloc(0, pair->value);
                break;
            }
            pair++;
        } while (remaining--);
        if (value != 0) {
            if (gOverlay7DispatchObject != 0) {
                overlay7ObjectReloc(gOverlay7DispatchObject);
                func_overlay_007_F0000CCC_185CB54(gOverlay7Selected);
            }
            overlay7CommitReloc(value, &gOverlay7CommitArgument);
        }
    }
}
