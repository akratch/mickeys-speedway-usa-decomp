#ifndef MICKEY_OVERLAYS_O005_AUDIO_BANK_H
#define MICKEY_OVERLAYS_O005_AUDIO_BANK_H

#include "PR/ultratypes.h"

/*
 * Binary bank/sequence layouts used by overlay 5.  Field offsets are checked
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

void alSeqFileNew(ALSeqFile *file, u8 *base);
void alBnkfNew(ALBankFile *file, u8 *table);

#endif
