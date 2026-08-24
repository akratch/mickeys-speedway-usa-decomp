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
    u8 keyMax;
    u8 keyBase;
    s8 detune;
} GsSndKeyMap;

typedef struct GsSndEnvelope {
    s32 attackTime;
    s32 decayTime;
    s32 releaseTime;
    u8 attackVolume;
    u8 decayVolume;
} GsSndEnvelope;

typedef struct GsSound {
    GsSndEnvelope *envelope;
    GsSndKeyMap *keyMap;
} GsSound;

typedef struct GsSoundStateLink {
    struct GsSoundStateLink *next;
    struct GsSoundStateLink *prev;
    GsSound *sound;
    u8 padC[0x1C];
    f32 slideMult;
    f32 pitch;
    struct GsSoundStateLink **userHandle;
    s32 unk34;
    s16 volume;
    u8 pad3A[6];
    u8 unk40;
    u8 unk41;
    u8 unk42;
    u8 flags;
    u8 state;
} GsSoundStateLink;

typedef struct GsSndEvent {
    u16 type;
    u16 pad2;
    GsSoundStateLink *state;
    s32 param;
    u8 padC[4];
} GsSndEvent;

typedef struct GsSndPitchEvent {
    s16 type;
    u16 pad2;
    GsSoundStateLink *state;
    f32 pitch;
} GsSndPitchEvent;

typedef struct GsSndEventItem {
    struct GsSndEventItem *next;
    struct GsSndEventItem *prev;
    s32 delta;
    GsSndEvent event;
} GsSndEventItem;

typedef struct GsSndEventQueue {
    GsSndEventItem *next;
    GsSndEventItem *prev;
    GsSndEventItem *allocHead;
    u8 padC[4];
    s16 allocCount;
} GsSndEventQueue;

extern GsSndPlayer *D_8007FF4C;
extern GsSoundStateLink *D_8007FF40;
extern GsSoundStateLink *D_8007FF44;
extern GsSoundStateLink *D_8007FF48;
extern s16 D_8007FF54;
extern const char D_800843CC[];
extern const char D_800843FC[];

u32 osSetIntMask(u32 mask);
f32 alCents2Ratio(s32 cents);
void alLink(void *element, void *after);
void alUnlink(void *element);
void n_alEvtqPostEvent(void *eventQueue, void *event, s32 delta);
void n_alSynFreeVoice(void *voice);
void n_alSynStopVoice(void *voice);
void rmonPrintf(const char *format, ...);
void func_8005CE28(GsSndEventQueue *queue, GsSoundStateLink *state, u16 flags);
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
    func_8005CE28((GsSndEventQueue *)D_8007FF4C->eventQueue, state, 0xFFFF);
}
/*
 * PROVENANCE: adapted from func_80243FE4 in Banjo-Kazooie's permitted
 * src/core1/code_5650.c and cross-checked against Perfect Dark's
 * sndp_apply_detune_pitch in src/lib/naudio/n_sndplayer.c.
 *
 * Plateau: bare -g -mips2 -32 is best across the flag lattice, but IDO emits
 * 30 instructions and a 0x28-byte frame instead of the target's 31 and 0x30.
 * The first mismatch is +0x2C. Ten event-union, declaration-order, typed-copy,
 * pointer, volatility and flag variants either produce the same object or
 * regress it. The surviving difference is the target's integer copy through
 * a stack address versus this toolchain's scalarized FP copy, consistent with
 * an original event-union/header-layout difference.
 */
#ifdef NON_MATCHING
void func_8005CDAC(GsSoundStateLink *state) {
    f32 pitch;
    GsSndPitchEvent event;

    pitch = alCents2Ratio(state->sound->keyMap->detune) * state->pitch;
    event.type = 0x10;
    event.state = state;
    event.pitch = pitch;
    n_alEvtqPostEvent(D_8007FF4C->eventQueue, &event, 0x8235);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005CDAC.s")
#endif
void func_8005CE28(GsSndEventQueue *queue, GsSoundStateLink *state, u16 typeMask) {
    GsSndEventItem *item;
    GsSndEventItem *next;
    GsSndEventItem *thisItem;
    GsSndEventItem *nextItem;
    GsSndEvent *event;
    u32 mask;

    mask = osSetIntMask(1);
    item = queue->allocHead;
    if (item != NULL) {
        do {
            next = item->next;
            thisItem = item;
            nextItem = next;
            event = &thisItem->event;
            if (event->state == state && (event->type & typeMask)) {
                if (nextItem != NULL) {
                    nextItem->delta += thisItem->delta;
                }
                alUnlink(item);
                queue->allocCount--;
                alLink(item, queue);
            }
            item = next;
        } while (item != NULL);
    }
    osSetIntMask(mask);
}
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
GsSoundStateLink *func_8005D030(s32 unused, GsSound *sound) {
    GsSoundStateLink *state;
    GsSndKeyMap *keyMap;
    s32 special;
    u32 mask;

    keyMap = sound->keyMap;
    mask = osSetIntMask(1);
    state = D_8007FF48;
    if (state != NULL) {
        D_8007FF48 = state->next;
        alUnlink(state);
        if (D_8007FF40 != NULL) {
            state->next = D_8007FF40;
            state->prev = NULL;
            D_8007FF40->prev = state;
            D_8007FF40 = state;
        } else {
            state->prev = NULL;
            state->next = state->prev;
            D_8007FF40 = state;
            D_8007FF44 = state;
        }
        osSetIntMask(mask);
        special = (sound->envelope->decayTime + 1) == 0;
        state->sound = sound;
        state->unk40 = special + 0x40;
        state->state = 5;
        state->pitch = 1.0f;
        state->unk34 = 2;
        state->flags = keyMap->keyMax & 0xF0;
        state->userHandle = NULL;
        if (state->flags & 0x20) {
            state->slideMult = alCents2Ratio((keyMap->keyBase * 100) - 6000);
        } else {
            state->slideMult = alCents2Ratio((keyMap->keyBase * 100 + keyMap->detune) - 6000);
        }
        if (special != 0) {
            state->flags |= 2;
        }
        state->unk42 = 0;
        state->unk41 = 0x40;
        state->volume = 0x7FFF;
    } else {
        osSetIntMask(mask);
    }
    return state;
}
void func_8005D260(GsSoundStateLink *state) {
    if (D_8007FF40 == state) {
        D_8007FF40 = state->next;
    }
    if (D_8007FF44 == state) {
        D_8007FF44 = state->prev;
    }
    alUnlink(state);
    if (D_8007FF48 != NULL) {
        state->next = D_8007FF48;
        state->prev = NULL;
        D_8007FF48->prev = state;
        D_8007FF48 = state;
    } else {
        state->prev = NULL;
        state->next = state->prev;
        D_8007FF48 = state;
    }
    if (state->flags & 4) {
        D_8007FF54--;
    }
    state->state = 0;
    if (state->userHandle != NULL) {
        if (*state->userHandle == state) {
            *state->userHandle = NULL;
        }
        state->userHandle = NULL;
    }
}
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
