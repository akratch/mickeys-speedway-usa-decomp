/*
 * Rare sound player -- ROM 0x5C310-0x5E6B0
 * (VRAM 0x8005B710-0x8005DAB0).
 *
 * Tier A: the complete 0x23A0-byte text is byte-identical, under relocation
 * masking, to JFG's built src/gsSnd.c.o. The first split keeps every function
 * in generated assembly; matching bodies are promoted one at a time. A flag
 * lattice on gsSndpGetGlobalVolume found the TU's debug-shaped duplicate
 * epilogues exact only under bare -g (with -mips2 -32), not the game default
 * -O2, so the Makefile carries that measured per-file override.
 *
 * PROVENANCE: JFG's permitted src/gsSnd.c and src/gsSnd.h were read for the
 * function names, declarations and source candidates. Adapted bodies will be
 * identified at their point of use and remain subject to Mickey byte identity.
 */

#include "PR/ultratypes.h"

extern u32 D_8007FF50;
extern u16 *D_800D7D78;

void sndp_stop_with_flags(u8 flags);

typedef struct GsSndPriorityState {
    u8 pad0[0x40];
    u8 priority;
    u8 pad41[3];
    u8 state;
} GsSndPriorityState;

typedef struct GsSndPlayer {
    u8 pad0[0x14];
    u8 eventQueue[0x34];
    s32 maxSystemSoundChannels;
    u8 pad4C[0xC];
    s32 curTime;
} GsSndPlayer;

typedef struct GsSndKeyMap {
    u8 velocityMin;
    u8 velocityMax;
    u8 keyMin;
} GsSndKeyMap;

typedef struct GsSound {
    void *envelope;
    GsSndKeyMap *keyMap;
} GsSound;

typedef struct GsSoundStateLink {
    struct GsSoundStateLink *next;
    struct GsSoundStateLink *prev;
    GsSound *sound;
    u8 padC[0x37];
    u8 flags;
} GsSoundStateLink;

typedef struct GsSndEvent {
    s16 type;
    u16 pad2;
    GsSoundStateLink *state;
    s32 param;
    u8 padC[4];
} GsSndEvent;

extern GsSndPlayer *D_8007FF4C;
extern GsSoundStateLink *D_8007FF40;
extern GsSoundStateLink *D_8007FF44;
extern GsSoundStateLink *D_8007FF48;
extern const char D_800843CC[];
extern const char D_800843FC[];

u32 osSetIntMask(u32 mask);
void n_alEvtqPostEvent(void *eventQueue, void *event, s32 delta);
void n_alSynFreeVoice(void *voice);
void n_alSynStopVoice(void *voice);
void rmonPrintf(const char *format, ...);
void func_8005CE28(void *queue, GsSoundStateLink *state, u16 flags);
void func_8005D260(GsSoundStateLink *state);

#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpNew.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005B978.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005BA40.s")
void func_8005CD3C(GsSoundStateLink *state) {
    if (state->flags & 4) {
        n_alSynStopVoice((u8 *)state + 0xC);
        n_alSynFreeVoice((u8 *)state + 0xC);
    }
    func_8005D260(state);
    func_8005CE28(D_8007FF4C->eventQueue, state, 0xFFFF);
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005CDAC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005CE28.s")
/* PROVENANCE: adapted from JFG src/gsSnd.c (getSoundStateCounts). */
u16 getSoundStateCounts(u16 *numFree, u16 *numAllocated) {
    u32 mask;
    u16 allocatedCounter;
    u16 freeCounter;
    u16 allocatedRevCounter;
    GsSoundStateLink *allocatedPtr;
    GsSoundStateLink *freePtr;
    GsSoundStateLink *allocatedRevPtr;

    mask = osSetIntMask(1);
    allocatedPtr = D_8007FF40;
    freePtr = D_8007FF48;
    allocatedRevPtr = D_8007FF44;

    for (allocatedCounter = 0; allocatedPtr != NULL;
         allocatedCounter++, allocatedPtr = allocatedPtr->next) {}
    for (freeCounter = 0; freePtr != NULL;
         freeCounter++, freePtr = freePtr->next) {}
    for (allocatedRevCounter = 0; allocatedRevPtr != NULL;
         allocatedRevCounter++, allocatedRevPtr = allocatedRevPtr->prev) {}

    *numFree = freeCounter;
    *numAllocated = allocatedCounter;

    osSetIntMask(mask);

    return allocatedRevCounter;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005D030.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005D260.s")
/* PROVENANCE: adapted from JFG src/gsSnd.c (gsSndpSetPriority). */
void gsSndpSetPriority(GsSndPriorityState *state, u8 priority) {
    if (state != NULL) {
        state->priority = (s16)priority;
    }
}
/* PROVENANCE: adapted from JFG src/gsSnd.c (gsSndpGetState). */
u8 gsSndpGetState(GsSndPriorityState *state) {
    if (state != NULL) {
        return state->state;
    } else {
        return 0;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/ad_sndp_play.s")
/* PROVENANCE: adapted from JFG src/gsSnd.c (gsSndpStop). */
void gsSndpStop(GsSoundStateLink *state) {
    GsSndEvent event;

    event.type = 0x400;
    event.state = state;
    if (state != NULL) {
        event.state->flags &= ~0x10;
        n_alEvtqPostEvent(D_8007FF4C->eventQueue, &event, 0);
    } else {
        rmonPrintf(D_800843CC);
    }
}
/* PROVENANCE: adapted from JFG src/gsSnd.c (sndp_stop_with_flags). */
void sndp_stop_with_flags(u8 eventFlags) {
    u32 mask;
    GsSndEvent event;
    GsSoundStateLink *state;

    mask = osSetIntMask(1);
    state = D_8007FF40;
    while (state != NULL) {
        event.type = 0x400;
        event.state = state;
        if ((state->flags & eventFlags) == eventFlags) {
            event.state->flags &= ~0x10;
            n_alEvtqPostEvent(D_8007FF4C->eventQueue, &event, 0);
        }
        state = state->next;
    }
    osSetIntMask(mask);
}
/* PROVENANCE: adapted from JFG src/gsSnd.c (gsSndpStopAll). */
void gsSndpStopAll(void) {
    sndp_stop_with_flags(1);
}
/* PROVENANCE: adapted from JFG src/gsSnd.c (gsSndpStopAllRetrigger). */
void gsSndpStopAllRetrigger(void) {
    sndp_stop_with_flags(0x11);
}
/* PROVENANCE: adapted from JFG src/gsSnd.c (gsSndpStopAllLooped). */
void gsSndpStopAllLooped(void) {
    sndp_stop_with_flags(3);
}
/* PROVENANCE: adapted from JFG src/gsSnd.c (gsSndpSetParam). */
void gsSndpSetParam(GsSoundStateLink *state, s16 type, u32 value) {
    GsSndEvent event;

    event.type = type;
    event.state = state;
    event.param = value;
    if (state != NULL) {
        n_alEvtqPostEvent(D_8007FF4C->eventQueue, &event, 0);
    } else {
        rmonPrintf(D_800843FC);
    }
}
/* PROVENANCE: adapted from JFG src/gsSnd.c (gsSndpGetMasterVolume). */
u16 gsSndpGetMasterVolume(u8 groupID) {
    return D_800D7D78[groupID];
}
/* PROVENANCE: adapted from JFG src/gsSnd.c (gsSndpSetMasterVolume). */
void gsSndpSetMasterVolume(u8 groupID, u16 volume) {
    u32 mask;
    GsSoundStateLink *state;
    s32 i;
    GsSndEvent event;

    mask = osSetIntMask(1);
    state = D_8007FF40;
    D_800D7D78[groupID] = volume;

    for (i = 0; state != NULL;) {
        if ((state->sound->keyMap->keyMin & 0x3F) == groupID) {
            event.type = 0x800;
            event.state = state;
            n_alEvtqPostEvent(D_8007FF4C->eventQueue, &event, 0);
        }
        i++, state = state->next;
    }

    osSetIntMask(mask);
}
/* PROVENANCE: adapted from JFG src/gsSnd.c (gsSndpSetGlobalVolume). */
void gsSndpSetGlobalVolume(u32 volume) {
    if (volume > 0x100) {
        volume = 0x100;
    }
    D_8007FF50 = volume;
}
/* PROVENANCE: adapted from JFG src/gsSnd.c (gsSndpGetGlobalVolume). */
u32 gsSndpGetGlobalVolume(void) {
    return D_8007FF50;
}
/* PROVENANCE: adapted from JFG src/gsSnd.c (gsSndpLimitVoices). */
void gsSndpLimitVoices(s32 limit) {
    if (D_8007FF4C->maxSystemSoundChannels >= limit) {
        D_8007FF4C->curTime = limit;
    } else {
        D_8007FF4C->curTime = D_8007FF4C->maxSystemSoundChannels;
    }
}
