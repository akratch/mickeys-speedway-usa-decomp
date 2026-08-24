#include "PR/ultratypes.h"

typedef struct Overlay7ModeRecord {
    u16 first;
    u16 second;
    u8 mode;
    u8 pad5;
} Overlay7ModeRecord;

typedef struct Overlay7ModeState {
    u8 pad000[1];
    s8 index;
    u8 pad002[0x381];
    u8 variant;
    u8 pad384[1];
    u8 alternate;
    u8 pad386[0x2D];
    u8 timer;
    u8 pad3B4[0x14];
    f32 height;
} Overlay7ModeState;

typedef struct Overlay7ModeOwner {
    u8 pad00[0x64];
    Overlay7ModeState *state;
} Overlay7ModeOwner;

extern Overlay7ModeRecord gOverlay7PrimaryModes[][10];
extern Overlay7ModeRecord gOverlay7AlternateModes[][10];
extern u32 gOverlay7DispatchFlagsReloc;
extern void overlay7CreateEntry(Overlay7ModeOwner *, u16, u8);
extern void overlay7AppendEntry(Overlay7ModeOwner *, u16, u8);
extern s32 overlay7LookupReloc(s32, s32);

#ifdef NON_MATCHING
void overlay7DispatchModes(Overlay7ModeOwner *first, Overlay7ModeOwner *second) {
    Overlay7ModeState *firstState;
    Overlay7ModeState *secondState;
    Overlay7ModeRecord *record;
    Overlay7ModeRecord (*modes)[10];

    secondState = second->state;
    firstState = first->state;
    modes = secondState->alternate == 0 ? gOverlay7AlternateModes
                                        : gOverlay7PrimaryModes;
    record = &modes[firstState->index][secondState->index];

    if ((s32)(gOverlay7DispatchFlagsReloc << 22) < 0) {
        secondState->timer = 100;
        secondState->height += 5.0f;
        switch (record->mode) {
        case 1:
            overlay7CreateEntry(first, record->first, 3);
            break;
        case 2:
            overlay7CreateEntry(first, record->first, 3);
            overlay7AppendEntry(second, record->second, 3);
            break;
        case 3:
            overlay7CreateEntry(second, record->second, 3);
            overlay7AppendEntry(first, record->first, 3);
            break;
        case 4:
            if (overlay7LookupReloc(1, 2) == 1) {
                overlay7CreateEntry(first, record->first, 3);
                overlay7AppendEntry(second, record->second, 3);
            } else {
                overlay7CreateEntry(second, record->second, 3);
                overlay7AppendEntry(first, record->first, 3);
            }
            break;
        case 0:
        case 5:
            overlay7CreateEntry(first, record->first, 3);
            overlay7CreateEntry(second, record->second, 3);
            break;
        case 6:
            if (overlay7LookupReloc(1, 2) == 1) {
                overlay7CreateEntry(first, record->first, 3);
                break;
            }
            overlay7CreateEntry(second, record->second, 3);
            break;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o007/overlay7DispatchModes/func_overlay_007_F0000894_185C71C.s")
#endif
