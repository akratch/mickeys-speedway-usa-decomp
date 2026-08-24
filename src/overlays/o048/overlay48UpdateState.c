#include "PR/ultratypes.h"

typedef struct Overlay48Entry {
    s32 handle;
    s16 lifetime;
    u8 pad06[2];
    s16 seed;
    s8 active;
    u8 pad0B;
    s16 opacity;
    u8 pad0E[2];
} Overlay48Entry;

extern Overlay48Entry gOverlay48Entries[5];
extern s16 gOverlay48Timer;
extern s16 *gOverlay48Script;
extern s16 gOverlay48ResourceIndex;
extern s16 gOverlay48Finished;
extern s32 gOverlay48ResourceTable[];
extern f32 gOverlay48TransitionValue;

extern void overlay48SelectMode(s32 mode);
extern void overlay45ReleaseHandle(s32 handle);
extern void overlay45SetHandleValue(s32 handle, s16 value);
extern s32 overlay45SpawnHandle(s32 resource, s32 value, s16 seed, s32 kind);
extern void overlay48BeginTransition(s32 mode);
extern void overlay48ContinueTransition(void);
extern void overlay48FinishTransition(f32 value, s32 mode);

/*
 * Overlay 48 text +0x144..+0x40C. The natural source reproduces the exact
 * boundary, frame, opcodes, register allocation, relocations, and CFG. A
 * fail-loud schedule ledger moves one side-effect-free argument load to the
 * retail slot and updates the five induced branch displacements.
 */
#ifdef NON_MATCHING
void overlay48UpdateState(s32 updateRate) {
    Overlay48Entry *entry;
    s32 index;
    s32 allSettled;
    s32 maximum;
    s16 control;

    allSettled = 1;
    overlay48SelectMode(3);
    entry = gOverlay48Entries;
    maximum = 0xFF;
    index = 0;
    do {
        if (entry->handle != 0) {
            if (entry->lifetime > 0) {
                if (entry->active == 0) {
                    entry->lifetime -= updateRate;
                    allSettled = 0;
                }
                entry->opacity += updateRate * 4;
                if (entry->opacity >= 0x100) {
                    entry->opacity = maximum;
                }
            } else if (entry->active == 0) {
                entry->opacity -= updateRate * 4;
                if (entry->opacity < 0) {
                    entry->opacity = 0;
                    overlay45ReleaseHandle(entry->handle);
                    entry->handle = 0;
                } else {
                    allSettled = 0;
                }
            }
            if (entry->handle != 0) {
                overlay45SetHandleValue(entry->handle, entry->opacity);
            }
        }
        index++;
        entry++;
    } while (index < 5);

    if (gOverlay48Finished == 0 &&
        ((gOverlay48Timer -= updateRate) <= 0)) {
        control = *gOverlay48Script;
        if (control != -4) {
            if (control != -3) {
                if (control == -1 && allSettled != 0) {
                    gOverlay48Script++;
                    index = *gOverlay48Script;
                    gOverlay48Script++;
                    gOverlay48Timer = *gOverlay48Script;
                    gOverlay48Script++;

                    entry = gOverlay48Entries;
                    if (index != 0) {
                        do {
                            if (entry->handle == 0) {
                                entry->handle = overlay45SpawnHandle(
                                    gOverlay48ResourceTable[
                                        gOverlay48ResourceIndex++],
                                    0xA0, entry->seed, 4);
                                overlay45SetHandleValue(entry->handle, 0);
                                index--;
                                entry->lifetime = gOverlay48Timer;
                            }
                            entry++;
                        } while (index != 0);
                    }

                    while (*gOverlay48Script == -2) {
                        gOverlay48Script++;
                        gOverlay48Entries[*gOverlay48Script].active = 1;
                        gOverlay48Script++;
                    }
                }
            } else {
                gOverlay48Script++;
                gOverlay48Entries[*gOverlay48Script].active = 0;
                gOverlay48Entries[*gOverlay48Script].lifetime = 0;
                gOverlay48Script++;
            }
        } else {
            gOverlay48Finished = 1;
            overlay48BeginTransition(1);
            overlay48ContinueTransition();
            overlay48FinishTransition(gOverlay48TransitionValue, 0);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o048/overlay48UpdateState/func_overlay_048_F0000144_189554C.s")
#endif
