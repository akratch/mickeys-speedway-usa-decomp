#ifndef OVERLAY_005_H
#define OVERLAY_005_H

#include "PR/ultratypes.h"

/*
 * Binary bank/sequence layouts used by overlay 5. Field offsets are checked
 * against the overlay's loads and stores; the type names follow the matching
 * libaudio implementation in Diddy Kong Racing us.v77.
 */

#define AL_BANK_VERSION 0x4231
#define AL_ADPCM_WAVE 0
#define AL_RAW16_WAVE 1

typedef struct ALADPCMBook_s ALADPCMBook;
typedef struct ALADPCMloop_s ALADPCMloop;
typedef struct ALRawLoop_s ALRawLoop;
typedef struct ALEnvelope_s ALEnvelope;
typedef struct ALKeyMap_s ALKeyMap;

typedef struct {
    ALADPCMloop *loop;
    ALADPCMBook *book;
} ALADPCMWaveInfo;

typedef struct {
    ALRawLoop *loop;
} ALRAWWaveInfo;

typedef struct ALWaveTable_s {
    u8 *base;
    s32 len;
    u8 type;
    u8 flags;
    union {
        ALADPCMWaveInfo adpcmWave;
        ALRAWWaveInfo rawWave;
    } waveInfo;
} ALWaveTable;

typedef struct ALSound_s {
    ALEnvelope *envelope;
    ALKeyMap *keyMap;
    ALWaveTable *wavetable;
    u8 samplePan;
    u8 sampleVolume;
    u8 flags;
} ALSound;

typedef struct ALInstrument_s {
    u8 volume;
    u8 pan;
    u8 priority;
    u8 flags;
    u8 tremType;
    u8 tremRate;
    u8 tremDepth;
    u8 tremDelay;
    u8 vibType;
    u8 vibRate;
    u8 vibDepth;
    u8 vibDelay;
    s16 bendRange;
    s16 soundCount;
    ALSound *soundArray[1];
} ALInstrument;

typedef struct ALBank_s {
    s16 instCount;
    u8 flags;
    u8 pad;
    s32 sampleRate;
    ALInstrument *percussion;
    ALInstrument *instArray[1];
} ALBank;

typedef struct {
    s16 revision;
    s16 bankCount;
    ALBank *bankArray[1];
} ALBankFile;

typedef struct {
    u8 *offset;
    s32 len;
} ALSeqData;

typedef struct {
    s16 revision;
    s16 seqCount;
    ALSeqData seqArray[1];
} ALSeqFile;

typedef struct Overlay5PlayerConfig {
    s32 arg0;
    s32 arg1;
    u8 channels;
    u8 pad9[3];
    void *heap;
    void *initQueue;
    void *frameCallback;
    void *eventQueue;
    u8 voiceCount;
} Overlay5PlayerConfig;

typedef struct Overlay5AudioState {
    u8 pad0[4];
    void *sequenceBank;
} Overlay5AudioState;

typedef struct Overlay5SequenceHeader {
    s16 count;
    u8 pad2[2];
    s32 value;
    u8 pad8[8];
} Overlay5SequenceHeader;

typedef struct Overlay5Resource {
    u32 span0Start;
    u32 span0End;
    u32 span1Start;
    u32 span1End;
    u32 end;
} Overlay5Resource;

typedef struct Overlay5BankEntry {
    u32 value;
    u32 size;
} Overlay5BankEntry;

typedef struct Overlay5Bank {
    u8 pad00[2];
    s16 count;
    u8 pad04[4];
    Overlay5BankEntry entries[1];
} Overlay5Bank;

typedef struct Overlay5SoundConfig {
    void *field00;
    s32 field04;
    s32 field08;
    s32 field0C;
    s32 field10;
    void *field14;
    s32 field18;
    s8 field1C;
} Overlay5SoundConfig;

typedef struct Overlay5SequenceConfig {
    void *field00;
    s32 field04;
    s32 field08;
    void *field0C;
    s16 field10;
} Overlay5SequenceConfig;

extern u8 gOverlay5AudioHeap[];
extern u8 gOverlay5InitQueue[];
extern u8 gOverlay5EventQueue[];
extern u8 gOverlay5FrameCallback[];
extern Overlay5AudioState *gOverlay5AudioState;
extern u8 gOverlay5HeapState[];
extern u8 gOverlay5HeapMemory[];
extern void *gOverlay5AudioOwner;
extern void *gOverlay5Span0;
extern void *gOverlay5Span1;
extern void *gOverlay5Span2;
extern u32 gOverlay5Span0Size;
extern u32 gOverlay5Span1Size;
extern u32 gOverlay5ScaleValue;
extern Overlay5Bank *gOverlay5Bank;
extern u32 *gOverlay5EntryValues;
extern void *gOverlay5Player0;
extern void *gOverlay5Player1;
extern u8 gOverlay5SoundState[];
extern u8 gOverlay5MessageQueue[];
extern u8 gOverlay5MessageBuffer[];

extern void alHeapInit(void *heap, void *base, s32 length);
extern Overlay5Resource *func_8002E148(s32 resourceId);
extern void *func_8002B280(s32 size, s32 tag);
extern void func_8002E2E0(s32 resourceId, void *dst, const void *src,
                          s32 length);
extern void *func_8002E35C(s32 resourceId, const void *address);
extern void *alHeapDBAlloc(void *file, s32 line, void *heap, s32 count,
                           s32 size);
extern void func_80001740(Overlay5SoundConfig *config, s32 count,
                          void *context);
extern void gsSndpNew(Overlay5SequenceConfig *config);
extern void func_80001BA0(void);
extern void func_80000450(void *value);
extern void func_8002B768(Overlay5Resource *resource);
extern void func_800039F0(void);
extern void alSurround_OutputType(s32 outputType);
extern void alSurround_ReverbSetup(s32 arg0, s32 outputType);
extern void osCreateMesgQueue(void *queue, void *buffer, s32 count);
extern void n_alCSPSetMessageQ(void *player, void *queue);
extern void overlay5SequenceInitReloc();
extern void *overlay5AllocPlayerReloc(s32 arg0, s32 arg1, void *heap,
                                      s32 arg3, s32 size);
extern void overlay5InitPlayerReloc(void *player,
                                    Overlay5PlayerConfig *config);
extern void overlay5AttachBankReloc(void *player, void *bank);

void alSeqFileNew(ALSeqFile *file, u8 *base);
void alBnkfNew(ALBankFile *file, u8 *table);
void _bnkfPatchBank(ALBank *bank, s32 offset, s32 table);
void _bnkfPatchInst(ALInstrument *inst, s32 offset, s32 table);
void _bnkfPatchSound(ALSound *sound, s32 offset, s32 table);
void _bnkfPatchWaveTable(ALWaveTable *wave, s32 offset, s32 table);
void overlay5InitSequence(void *owner, s32 value);
void overlay5InitializeAudio(void *context);
void *overlay5CreatePlayer(s32 arg0, s32 arg1);

#endif
