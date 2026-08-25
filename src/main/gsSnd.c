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
#include "n_libaudio.h"

#define GS_SNDP_PLAY_EVT 0x1
#define GS_SNDP_RELEASE_EVT 0x2
#define GS_SNDP_PAN_EVT 0x4
#define GS_SNDP_VOL_EVT 0x8
#define GS_SNDP_PITCH_EVT 0x10
#define GS_SNDP_DECAY_EVT 0x40
#define GS_SNDP_END_EVT 0x80
#define GS_SNDP_FX_EVT 0x100
#define GS_SNDP_RETRIGGER_EVT 0x200
#define GS_SNDP_STOP_EVT 0x400
#define GS_SNDP_GROUP_VOL_EVT 0x800
#define GS_SNDP_RELEASE_NEXT_EVT 0x1000

#define GS_MIN(a, b) ((a) < (b) ? (a) : (b))
#define GS_MAX(a, b) ((a) > (b) ? (a) : (b))

extern u32 D_8007FF50;
extern u16 *D_800D7D78;

void sndp_stop_with_flags(u8 flags);

typedef struct GsSndPriorityState {
    u8 pad0[0x40];
    u8 priority;
    u8 pad41[3];
    u8 state;
} GsSndPriorityState;

struct GsSoundStateLink;

typedef struct GsSndPlayer {
    void *next;
    void *self;
    s32 (*handler)(struct GsSndPlayer *player);
    u8 padC[8];
    u8 eventQueue[0x18];
    s16 eventType;
    u8 eventPad2[0xE];
    void *driver;
    struct GsSoundStateLink *currentState;
    struct GsSoundStateLink *statePool;
    s32 maxSystemSoundChannels;
    s32 eventDelta;
    s32 nextDelta;
    s32 currentTime;
    s32 voiceLimit;
} GsSndPlayer;

typedef struct GsSndConfig {
    u32 maxSounds;
    s32 eventCount;
    s32 maxSystemSoundChannels;
    ALHeap *heap;
    u16 groupCount;
} GsSndConfig;

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
    ALWaveTable *wavetable;
    u8 samplePan;
    u8 sampleVolume;
    u8 flags;
} GsSound;

typedef struct GsSoundStateLink {
    struct GsSoundStateLink *next;
    struct GsSoundStateLink *prev;
    GsSound *sound;
    N_ALVoice voice;
    f32 slideMult;
    f32 pitch;
    struct GsSoundStateLink **userHandle;
    s32 retries;
    s16 volume;
    s16 envelopeVolume;
    s32 endTime;
    u8 priority;
    u8 pan;
    u8 fxMix;
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

typedef struct GsSndPitchBitsEvent {
    s16 type;
    u16 pad2;
    GsSoundStateLink *state;
    u32 pitch;
    u8 padC[4];
} GsSndPitchBitsEvent;

typedef struct GsSndPitchEvent {
    s16 type;
    u16 pad2;
    GsSoundStateLink *state;
    f32 pitch;
} GsSndPitchEvent;

typedef struct GsSndRetriggerEvent {
    u16 type;
    u16 pad2;
    GsSoundStateLink *state;
    s32 soundId;
    void *bank;
} GsSndRetriggerEvent;

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
const char D_80084320[] =
    "Bad soundState: voices =%d, states free =%d, states busy =%d, type %d data %x\n";
const char D_80084370[] = "playing a playing sound\n";
const char D_8008438C[] = "Nonsense sndp event\n";
const char D_800843A4[] = "Sound state allocate failed - sndId %d\n";
const char D_800843CC[] = "WARNING: Attempt to stop NULL sound aborted\n";
const char D_800843FC[] = "WARNING: Attempt to modify NULL sound aborted\n";

u32 osSetIntMask(u32 mask);
f32 alCents2Ratio(s32 cents);
s32 func_8005B978(GsSndPlayer *player);
void func_8005BA40(GsSndEvent *event);
void rmonPrintf(const char *format, ...);
void func_8005CD3C(GsSoundStateLink *state);
void func_8005CDAC(GsSoundStateLink *state);
void func_8005CE28(GsSndEventQueue *queue, GsSoundStateLink *state, u16 flags);
u16 getSoundStateCounts(u16 *numFree, u16 *numAllocated);
GsSoundStateLink *ad_sndp_play(void *bank, s16 soundId, u16 volume, u8 pan,
                               f32 pitch, u8 fxMix,
                               GsSoundStateLink **handle);
void func_8005D260(GsSoundStateLink *state);

void gsSndpNew(GsSndConfig *config) {
    u32 i;
    void *allocation;
    GsSndEvent event;
    GsSoundStateLink *statePool;

    D_8007FF4C->maxSystemSoundChannels = config->maxSystemSoundChannels;
    D_8007FF4C->voiceLimit = config->maxSystemSoundChannels;
    D_8007FF4C->currentState = NULL;
    D_8007FF4C->eventDelta = 0x3E80;
    allocation = alHeapDBAlloc(NULL, 0, config->heap, 1,
                               config->maxSounds * sizeof(GsSoundStateLink));
    D_8007FF4C->statePool = allocation;
    allocation = alHeapDBAlloc(NULL, 0, config->heap, 1,
                               config->eventCount * sizeof(GsSndEventItem));
    n_alEvtqNew(D_8007FF4C->eventQueue, allocation, config->eventCount);
    D_8007FF48 = D_8007FF4C->statePool;
    for (i = 1; i < config->maxSounds; i++) {
        statePool = D_8007FF4C->statePool;
        alLink(&statePool[i], &statePool[i - 1]);
    }
    D_800D7D78 = alHeapDBAlloc(NULL, 0, config->heap, 2, config->groupCount);
    for (i = 0; i < config->groupCount; i++) {
        D_800D7D78[i] = 0x7FFF;
    }
    D_8007FF4C->next = NULL;
    D_8007FF4C->handler = func_8005B978;
    D_8007FF4C->self = D_8007FF4C;
    n_alSynAddSndPlayer(D_8007FF4C);
    event.type = 0x20;
    n_alEvtqPostEvent(D_8007FF4C->eventQueue, &event, D_8007FF4C->eventDelta);
    D_8007FF4C->nextDelta =
        n_alEvtqNextEvent(D_8007FF4C->eventQueue, &D_8007FF4C->eventType);
}
s32 func_8005B978(GsSndPlayer *playerArg) {
    GsSndPlayer *player;
    GsSndEvent event;

    player = playerArg;
    do {
        switch (player->eventType) {
            case 0x20:
                event.type = 0x20;
                n_alEvtqPostEvent(player->eventQueue, &event, player->eventDelta);
                break;
            default:
                func_8005BA40(&player->eventType);
                break;
        }
        player->nextDelta = n_alEvtqNextEvent(player->eventQueue, &player->eventType);
    } while (player->nextDelta == 0);
    player->currentTime += player->nextDelta;
    return player->nextDelta;
}
/*
 * PROVENANCE: adapted from Perfect Dark's permitted
 * src/lib/naudio/n_sndplayer.c (_n_handleEvent), with Mickey's event fields,
 * state layout, volume scaling, retrigger call and diagnostics reconstructed
 * from Mickey itself.
 */
void func_8005BA40(GsSndEvent *event) {
    ALVoiceConfig config;
    GsSound *sound;
    GsSndKeyMap *keyMap;
    u8 pan;
    GsSndEvent stateEvent;
    GsSndEvent nextStateEvent;
    s32 delta;
    s32 fxMix;
    u32 volume;
    s32 tempPan;
    s32 limitReached;
    s32 isSingleStateEvent;
    s32 done = TRUE;
    s32 hasVoice = FALSE;
    GsSoundStateLink *state = NULL;
    GsSoundStateLink *nextState = NULL;
    u16 numFree;
    u16 numAllocated;
    GsSoundStateLink *iterState;
    GsSndEvent interruptEvent;
    GsSoundStateLink *newSound;

    do {
        if (nextState != NULL) {
            nextStateEvent.state = state;
            nextStateEvent.type = event->type;
            nextStateEvent.param = event->param;
            event = &nextStateEvent;
        }

        state = event->state;
        sound = state->sound;

        if (sound == NULL) {
            getSoundStateCounts(&numFree, &numAllocated);
            rmonPrintf(D_80084320, D_8007FF54, numFree, numAllocated,
                       event->type, event->param);
            return;
        }

        keyMap = sound->keyMap;
        nextState = state->next;

        switch (event->type) {
            case GS_SNDP_PLAY_EVT:
                if (state->state != 5 && state->state != 4) {
                    return;
                }

                if (state->state == 1) {
                    rmonPrintf(D_80084370);
                }

                config.fxBus = 0;
                config.priority = state->priority;
                config.unityPitch = 0;
                limitReached = D_8007FF54 >= D_8007FF4C->voiceLimit;

                if (!limitReached || (state->flags & 0x10)) {
                    hasVoice = n_alSynAllocVoice(&state->voice, &config);
                }

                if (!hasVoice) {
                    if ((state->flags & 0x12) || state->retries > 0) {
                        state->state = 4;
                        state->retries--;
                        n_alEvtqPostEvent((ALEventQueue *)D_8007FF4C->eventQueue,
                                          (N_ALEvent *)event, 33333);
                    } else if (limitReached) {
                        iterState = D_8007FF44;

                        do {
                            if (iterState->priority <= state->priority &&
                                !(iterState->flags & 0x12) &&
                                (iterState->flags & 4) &&
                                iterState->state != 3) {
                                limitReached = FALSE;
                                interruptEvent.type = GS_SNDP_END_EVT;
                                interruptEvent.state = iterState;
                                iterState->state = 3;
                                n_alEvtqPostEvent(
                                    (ALEventQueue *)D_8007FF4C->eventQueue,
                                    (N_ALEvent *)&interruptEvent, 1000);
                                n_alSynSetVol(&iterState->voice, 0, 1000);
                            }

                            iterState = iterState->prev;
                        } while (limitReached && iterState != NULL);

                        if (!limitReached) {
                            state->retries = 2;
                            n_alEvtqPostEvent(
                                (ALEventQueue *)D_8007FF4C->eventQueue,
                                (N_ALEvent *)event, 1001);
                        } else {
                            func_8005CD3C(state);
                        }
                    } else {
                        func_8005CD3C(state);
                    }
                    return;
                }

                state->flags |= 4;
                state->envelopeVolume = sound->envelope->attackVolume;
                delta = sound->envelope->attackTime / state->pitch /
                        state->slideMult;
                state->endTime = D_8007FF4C->currentTime + delta;
                volume = GS_MAX(
                    0, ((s16 *)D_800D7D78)[keyMap->keyMin & 0x3F] *
                               (state->envelopeVolume * state->volume *
                                sound->sampleVolume / 16129) /
                               32767 -
                           1);
                volume = (u32)(volume * D_8007FF50) >> 8;
                tempPan = state->pan + sound->samplePan - 0x40;
                pan = GS_MIN(GS_MAX(tempPan, 0), 0x7F);
                fxMix = (state->fxMix & 0x7F) +
                        ((keyMap->keyMax & 0xF) * 8);
                fxMix = GS_MIN(0x7F, GS_MAX(0, fxMix));
                fxMix |= state->fxMix & 0x80;

                n_alSynStartVoiceParams(
                    &state->voice, sound->wavetable,
                    state->pitch * state->slideMult, volume, pan, fxMix, 0,
                    0.0f, 0, delta);
                state->state = 1;
                D_8007FF54++;

                if (!(state->flags & 2)) {
                    if (delta == 0) {
                        state->envelopeVolume =
                            sound->envelope->decayVolume;
                        volume = GS_MAX(
                            0, ((s16 *)D_800D7D78)[keyMap->keyMin & 0x3F] *
                                       (state->envelopeVolume * state->volume *
                                        sound->sampleVolume / 16129) /
                                       32767 -
                                   1);
                        volume = (u32)(volume * D_8007FF50) >> 8;
                        delta = sound->envelope->decayTime /
                                state->slideMult / state->pitch;
                        state->endTime = D_8007FF4C->currentTime + delta;
                        n_alSynSetVol(&state->voice, volume, delta);
                        stateEvent.type = GS_SNDP_RELEASE_EVT;
                        stateEvent.state = state;
                        n_alEvtqPostEvent(
                            (ALEventQueue *)D_8007FF4C->eventQueue,
                            (N_ALEvent *)&stateEvent, delta);
                        if (state->flags & 0x20) {
                            func_8005CDAC(state);
                        }
                    } else {
                        stateEvent.type = GS_SNDP_DECAY_EVT;
                        stateEvent.state = state;
                        delta = sound->envelope->attackTime / state->pitch /
                                state->slideMult;
                        n_alEvtqPostEvent(
                            (ALEventQueue *)D_8007FF4C->eventQueue,
                            (N_ALEvent *)&stateEvent, delta);
                    }
                }
                break;

            case GS_SNDP_RELEASE_EVT:
            case GS_SNDP_STOP_EVT:
            case GS_SNDP_RELEASE_NEXT_EVT:
                if (event->type != GS_SNDP_RELEASE_NEXT_EVT ||
                    (state->flags & 2)) {
                    switch (state->state) {
                        case 1:
                            func_8005CE28(
                                (GsSndEventQueue *)D_8007FF4C->eventQueue,
                                state, GS_SNDP_DECAY_EVT);
                            delta = sound->envelope->releaseTime /
                                    state->slideMult / state->pitch;
                            n_alSynSetVol(&state->voice, 0, delta);
                            if (delta != 0) {
                                stateEvent.type = GS_SNDP_END_EVT;
                                stateEvent.state = state;
                                n_alEvtqPostEvent(
                                    (ALEventQueue *)D_8007FF4C->eventQueue,
                                    (N_ALEvent *)&stateEvent, delta);
                                state->state = 2;
                            } else {
                                func_8005CD3C(state);
                            }
                            break;
                        case 4:
                        case 5:
                            func_8005CD3C(state);
                            break;
                        default:
                            break;
                    }

                    if (event->type == GS_SNDP_RELEASE_EVT) {
                        event->type = GS_SNDP_RELEASE_NEXT_EVT;
                    }
                }
                break;

            case GS_SNDP_PAN_EVT:
                state->pan = event->param;
                if (state->state == 1) {
                    tempPan = state->pan + sound->samplePan - 0x40;
                    pan = GS_MIN(GS_MAX(tempPan, 0), 0x7F);
                    n_alSynSetPan(&state->voice, pan);
                }
                break;

            case GS_SNDP_PITCH_EVT:
                state->pitch = ((GsSndPitchEvent *)event)->pitch;
                if (state->state == 1) {
                    n_alSynSetPitch(&state->voice,
                                    state->pitch * state->slideMult);
                    if (state->flags & 0x20) {
                        func_8005CDAC(state);
                    }
                }
                break;

            case GS_SNDP_FX_EVT:
                state->fxMix = event->param;
                if (state->state == 1) {
                    fxMix = (state->fxMix & 0x7F) +
                            ((keyMap->keyMax & 0xF) * 8);
                    fxMix = GS_MIN(0x7F, GS_MAX(0, fxMix));
                    fxMix |= state->fxMix & 0x80;
                    n_alSynSetFXMix(&state->voice, fxMix);
                }
                break;

            case GS_SNDP_VOL_EVT:
                state->volume = event->param;
                if (state->state == 1) {
                    volume = GS_MAX(
                        0, ((s16 *)D_800D7D78)[keyMap->keyMin & 0x3F] *
                                   (state->envelopeVolume * state->volume *
                                    sound->sampleVolume / 16129) /
                                   32767 -
                               1);
                    volume = (u32)(volume * D_8007FF50) >> 8;
                    n_alSynSetVol(
                        &state->voice, volume,
                        GS_MAX(1000,
                               state->endTime - D_8007FF4C->currentTime));
                }
                break;

            case GS_SNDP_GROUP_VOL_EVT:
                if (state->state == 1) {
                    delta = sound->envelope->releaseTime /
                            state->slideMult / state->pitch;
                    volume = GS_MAX(
                        0, ((s16 *)D_800D7D78)[keyMap->keyMin & 0x3F] *
                                   (state->envelopeVolume * state->volume *
                                    sound->sampleVolume / 16129) /
                                   32767 -
                               1);
                    volume = (u32)(volume * D_8007FF50) >> 8;
                    n_alSynSetVol(&state->voice, volume, delta);
                }
                break;

            case GS_SNDP_DECAY_EVT:
                if (!(state->flags & 2)) {
                    state->envelopeVolume = sound->envelope->decayVolume;
                    volume = GS_MAX(
                        0, ((s16 *)D_800D7D78)[keyMap->keyMin & 0x3F] *
                                   (state->envelopeVolume * state->volume *
                                    sound->sampleVolume / 16129) /
                                   32767 -
                               1);
                    volume = (u32)(volume * D_8007FF50) >> 8;
                    delta = sound->envelope->decayTime /
                            state->slideMult / state->pitch;
                    state->endTime = D_8007FF4C->currentTime + delta;
                    n_alSynSetVol(&state->voice, volume, delta);
                    stateEvent.type = GS_SNDP_RELEASE_EVT;
                    stateEvent.state = state;
                    n_alEvtqPostEvent(
                        (ALEventQueue *)D_8007FF4C->eventQueue,
                        (N_ALEvent *)&stateEvent, delta);
                    if (state->flags & 0x20) {
                        func_8005CDAC(state);
                    }
                }
                break;

            case GS_SNDP_END_EVT:
                func_8005CD3C(state);
                break;

            case GS_SNDP_RETRIGGER_EVT:
                if (state->flags & 0x10) {
                    newSound = ad_sndp_play(
                        ((GsSndRetriggerEvent *)event)->bank,
                        ((GsSndRetriggerEvent *)event)->soundId,
                        state->volume, state->pan, state->pitch, state->fxMix,
                        state->userHandle);
                }
                break;

            default:
                rmonPrintf(D_8008438C);
                break;
        }

        isSingleStateEvent = event->type &
                             (GS_SNDP_PLAY_EVT | GS_SNDP_PITCH_EVT |
                              GS_SNDP_DECAY_EVT | GS_SNDP_END_EVT |
                              GS_SNDP_RETRIGGER_EVT);

        if ((state = nextState) != NULL && !isSingleStateEvent) {
            done = state->flags & 1;
        }
    } while (!done && state != NULL && !isSingleStateEvent);
}
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
 * Exact under bare -g -mips2 -32. The 16-byte event footprint retains the
 * unused final word, and representing the pitch payload as raw bits preserves
 * the target's integer stack copy.
 */
void func_8005CDAC(GsSoundStateLink *state) {
    GsSndPitchBitsEvent event;
    f32 pitch;

    pitch = alCents2Ratio(state->sound->keyMap->detune) * state->pitch;
    event.type = 0x10;
    event.state = state;
    event.pitch = *(u32 *)&pitch;
    n_alEvtqPostEvent(D_8007FF4C->eventQueue, (N_ALEvent *)&event,
                      0x8235);
}
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
GsSoundStateLink *func_8005D030(void *unused, GsSound *sound) {
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
        state->priority = special + 0x40;
        state->state = 5;
        state->pitch = 1.0f;
        state->retries = 2;
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
        state->fxMix = 0;
        state->pan = 0x40;
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
/*
 * PROVENANCE: adapted from DKR src/audiosfx.c (sndp_play_with_priority) and
 * Perfect Dark src/lib/naudio/n_sndplayer.c (sndp_play_sound). Mickey's
 * extended volume/pan/pitch/fx parameters and bank lookup were reconstructed
 * from Mickey itself; the nested event lifetimes follow the permitted donors.
 */
GsSoundStateLink *ad_sndp_play(void *bank, s16 soundId, u16 volume, u8 pan,
                               f32 pitch, u8 fxMix,
                               GsSoundStateLink **handle) {
    GsSoundStateLink *state;
    GsSoundStateLink *lastState;
    GsSndKeyMap *keyMap;
    GsSound *sound;
    s16 retriggerSoundId;
    s32 retriggerDelta;
    s32 soundDelta;
    s32 sequenceDelta;
    s32 adjustedPan;

    lastState = NULL;
    retriggerSoundId = 0;
    sequenceDelta = 0;
    if (soundId != 0) {
        do {
            sound = (GsSound *) ((ALBank *) bank)->instArray[0]->soundArray[soundId - 1];
            state = func_8005D030(bank, sound);
            if (state != NULL) {
                GsSndEvent playEvent;

                D_8007FF4C->currentState = state;
                playEvent.type = 1;
                playEvent.state = state;
                adjustedPan = pan + state->pan - 0x40;
                if (adjustedPan >= 0x80) {
                    adjustedPan = 0x7F;
                } else if (adjustedPan < 0) {
                    adjustedPan = 0;
                }
                state->pan = adjustedPan;
                state->volume = (u32)(volume * state->volume) >> 15;
                state->pitch *= pitch;
                state->fxMix = fxMix;
                soundDelta = sound->keyMap->velocityMax * 33333;
                if (state->flags & 0x10) {
                    state->flags &= ~0x10;
                    n_alEvtqPostEvent(D_8007FF4C->eventQueue, &playEvent,
                                      sequenceDelta + 1);
                    retriggerDelta = soundDelta + 1;
                    retriggerSoundId = soundId;
                } else {
                    n_alEvtqPostEvent(D_8007FF4C->eventQueue, &playEvent,
                                      soundDelta + 1);
                }
                lastState = state;
            } else {
                rmonPrintf(D_800843A4, soundId);
            }
            sequenceDelta += soundDelta;
            keyMap = sound->keyMap;
            soundId = keyMap->velocityMin + ((keyMap->keyMin & 0xC0) << 2);
        } while (soundId != 0 && state != NULL);

        if (lastState != NULL) {
            lastState->flags |= 1;
            lastState->userHandle = handle;
            if (retriggerSoundId != 0) {
                GsSndRetriggerEvent retriggerEvent;

                lastState->flags |= 0x10;
                retriggerEvent.type = 0x200;
                retriggerEvent.state = lastState;
                retriggerEvent.soundId = retriggerSoundId;
                retriggerEvent.bank = bank;
                n_alEvtqPostEvent(D_8007FF4C->eventQueue, &retriggerEvent,
                                  retriggerDelta);
            }
        }
    }
    if (handle != NULL) {
        *handle = lastState;
    }
    return lastState;
}
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
        D_8007FF4C->voiceLimit = limit;
    } else {
        D_8007FF4C->voiceLimit = D_8007FF4C->maxSystemSoundChannels;
    }
}
