#include "PR/ultratypes.h"

typedef struct Overlay7DispatchState {
    u8 pad00[1];
    s8 index;
    u8 pad02[0x45B];
    u8 field45D;
} Overlay7DispatchState;

typedef struct Overlay7DispatchOwner {
    u8 pad00[0x64];
    Overlay7DispatchState *state;
} Overlay7DispatchOwner;

extern u32 gOverlay7DispatchFlagsReloc;
extern u16 gOverlay7DispatchOverride[32];
extern u16 gOverlay7DispatchValues[][30];
extern u8 gOverlay7DispatchTypes[];
extern s8 gOverlay7DispatchMap[];
extern void overlay7CreateEntry(Overlay7DispatchOwner *owner, u16 value,
                                u8 type);
extern s32 overlay7QueryReloc(void);
extern void overlay7ApplyReloc(s32 arg0, s8 index, s8 value, u8 field);

#ifdef NON_MATCHING
void overlay7DispatchSelection(Overlay7DispatchOwner *owner, s32 selection) {
    Overlay7DispatchState *state;
    u16 *override;
    s8 mapped;

    state = owner->state;
    if ((s32)(gOverlay7DispatchFlagsReloc << 22) < 0) {
        if (selection >= 14 && selection < 17) {
            override = &gOverlay7DispatchOverride[state->index];
            if (*override == 0) {
                *override = 0x10E;
                goto create;
            }
        } else {
create:
            overlay7CreateEntry(owner,
                                gOverlay7DispatchValues[state->index][selection],
                                gOverlay7DispatchTypes[selection]);
            goto query;
        }
    } else {
query:
        if (overlay7QueryReloc() == 0) {
            mapped = gOverlay7DispatchMap[selection];
            if (mapped != -1) {
                overlay7ApplyReloc(0, state->index, mapped, state->field45D);
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o007/overlay7DispatchSelection/func_overlay_007_F0000CCC_185CB54.s")
#endif
