#include "PR/ultratypes.h"

typedef struct Overlay7CheckEntry {
    s32 value;
    u8 pad04[4];
} Overlay7CheckEntry;

typedef struct Overlay7CheckState {
    u8 pad000[0x1A8];
    u16 flags1A8;
    u8 pad1AA[0x1D9];
    s8 field383;
    u8 pad384;
    u8 field385;
    u8 pad386[0x7A];
    s32 field400;
    s32 limits404[3];
} Overlay7CheckState;

typedef struct Overlay7CheckOwner {
    u8 pad00[0x64];
    Overlay7CheckState *state;
} Overlay7CheckOwner;

extern s32 D_8;
extern u8 *overlay7GetModeReloc(void);
extern void *overlay7GetCheckTableReloc(void);
extern void *overlay7GetCurrentReloc(void);
extern s32 overlay7GetCheckIndexReloc(void *current);
extern void overlay7RecordCheckReloc(s32 value);
extern void overlay7SetOwnerModeReloc(Overlay7CheckOwner *owner, s32 mode);

#ifdef NON_MATCHING
void overlay7UpdateOwnerMode(Overlay7CheckOwner *owner, s32 previous) {
    Overlay7CheckState *state;
    Overlay7CheckEntry *base;
    Overlay7CheckEntry *entries;
    s32 reference;
    s32 index;
    s32 failed;
    s32 i;

    state = owner->state;
    if (*overlay7GetModeReloc() == 1) {
        failed = 0;
        base = (Overlay7CheckEntry *)overlay7GetCheckTableReloc();
        index = overlay7GetCheckIndexReloc(overlay7GetCurrentReloc());

        if (D_8 < state->field383 && !(state->flags1A8 & 1)) {
            if (D_8 == 2) {
                entries = base + index * 4;
                reference = entries[3].value;
                for (i = 0; i < 3; i++) {
                    if (state->limits404[i] < reference || reference == 0 ||
                        state->field400 < entries[i].value ||
                        entries[i].value == 0) {
                        failed = 1;
                    }
                }
                if (failed) {
                    overlay7RecordCheckReloc(5);
                } else {
                    overlay7RecordCheckReloc(D_8 + 2);
                }
            } else {
                overlay7RecordCheckReloc(D_8 + 2);
            }
            D_8++;
        }

        if (previous == state->field383) {
            if (failed) {
                overlay7SetOwnerModeReloc(owner, 6);
            } else {
                overlay7SetOwnerModeReloc(owner, 12);
            }
        } else {
            overlay7SetOwnerModeReloc(owner, state->field385);
        }
    } else {
        if (D_8 < state->field383 && !(state->flags1A8 & 1)) {
            overlay7RecordCheckReloc(D_8 + 2);
            D_8++;
        }
        if (previous == state->field383) {
            overlay7SetOwnerModeReloc(owner, state->field385 + 7);
        } else {
            overlay7SetOwnerModeReloc(owner, state->field385);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o007/overlay7UpdateOwnerMode/func_overlay_007_F0000AA0_185C928.s")
#endif
