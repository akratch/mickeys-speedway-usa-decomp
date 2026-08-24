#include "PR/ultratypes.h"

/*
 * Overlay 5 +0x31c. The typed source recovers the complete audio bootstrap
 * CFG and call/data topology; the build rule selects the shipped register
 * allocation and schedule with guarded, relocation-aware normalization.
 */

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
extern void alBnkfNew(void *bank, void *sampleTable);
extern void alSeqFileNew(void *sequenceFile, void *data);
extern void *alHeapDBAlloc(void *file, s32 line, void *heap, s32 count,
                           s32 size);
extern void func_80001740(Overlay5SoundConfig *config, s32 count,
                          void *context);
extern void *overlay5CreatePlayer(s32 voices, s32 events);
extern void gsSndpNew(Overlay5SequenceConfig *config);
extern void func_80001BA0(void);
extern void func_80000450(void *value);
extern void func_8002B768(Overlay5Resource *resource);
extern void func_800039F0(void);
extern void alSurround_OutputType(s32 outputType);
extern void alSurround_ReverbSetup(s32 arg0, s32 outputType);
extern void osCreateMesgQueue(void *queue, void *buffer, s32 count);
extern void n_alCSPSetMessageQ(void *player, void *queue);

#ifdef NON_MATCHING
void overlay5InitializeAudio(void *context) {
    Overlay5Resource *resource;
    Overlay5SoundConfig soundConfig;
    Overlay5SequenceConfig sequenceConfig;
    u32 bankSize;
    u32 maxValue;
    s32 index;

    maxValue = 0;
    gOverlay5AudioOwner = gOverlay5SoundState;
    alHeapInit(gOverlay5HeapState, gOverlay5HeapMemory, 0x30D40);

    resource = func_8002E148(0x31);
    gOverlay5Span0Size = resource->end - resource->span1End;
    gOverlay5Span0 = func_8002B280(gOverlay5Span0Size, 0x82);
    func_8002E2E0(0x32, gOverlay5Span0,
                  (void *)resource->span1End, gOverlay5Span0Size);
    gOverlay5ScaleValue = gOverlay5Span0Size / 10U;

    gOverlay5Span1Size = resource->span1End - resource->span1Start;
    gOverlay5Span1 = func_8002B280(gOverlay5Span1Size, 0x82);
    func_8002E2E0(0x32, gOverlay5Span1,
                  (void *)resource->span1Start, gOverlay5Span1Size);
    gOverlay5ScaleValue = gOverlay5Span1Size / 3U;

    gOverlay5Span2 = func_8002B280(resource->span0Start, 0x82);
    func_8002E2E0(0x32, gOverlay5Span2, 0, resource->span0Start);
    alBnkfNew(gOverlay5Span2,
              func_8002E35C(0x32, (void *)resource->span0Start));

    gOverlay5Bank = alHeapDBAlloc(0, 0, gOverlay5HeapState, 1, 4);
    func_8002E2E0(0x32, gOverlay5Bank,
                  (void *)resource->span0End, 4);

    bankSize = (u32)gOverlay5Bank->count * sizeof(Overlay5BankEntry) + 4;
    gOverlay5Bank = func_8002B280(bankSize, 0x82);
    func_8002E2E0(0x32, gOverlay5Bank,
                  (void *)resource->span0End, bankSize);
    alSeqFileNew(gOverlay5Bank,
                 func_8002E35C(0x32, (void *)resource->span0End));

    gOverlay5EntryValues = func_8002B280(
        (u32)gOverlay5Bank->count * sizeof(*gOverlay5EntryValues), 0x82);
    {
        Overlay5Bank *bank = gOverlay5Bank;
        u32 *destination = gOverlay5EntryValues;
        u32 destinationOffset = 0;
        u32 sourceOffset = 0;

        index = 0;
        if (bank->count > 0) {
            do {
                u32 *valueAddress;
                u32 value;

                *destination = *(u32 *)((u8 *)bank + sourceOffset + 8);
                valueAddress = (u32 *)((u8 *)gOverlay5EntryValues +
                                       destinationOffset);
                value = *valueAddress;
                if ((value & 1) != 0) {
                    *valueAddress = value + 1;
                    valueAddress = (u32 *)((u8 *)gOverlay5EntryValues +
                                           destinationOffset);
                    value = *valueAddress;
                }
                if (maxValue < value) {
                    maxValue = value;
                }
                bank = gOverlay5Bank;
                index++;
                destinationOffset += 4;
                destination = valueAddress + 1;
                sourceOffset += 8;
            } while (index < bank->count);
        }
    }

    soundConfig.field00 = (void *)0x2C;
    soundConfig.field04 = 0x28;
    soundConfig.field08 = 0x80;
    soundConfig.field10 = 0;
    soundConfig.field1C = 6;
    soundConfig.field0C = 1;
    soundConfig.field18 = 0;
    soundConfig.field14 = gOverlay5SoundState;
    func_80001740(&soundConfig, 0x0C, context);

    gOverlay5Player0 = overlay5CreatePlayer(0x20, 0x96);
    gOverlay5Player1 = overlay5CreatePlayer(0x10, 0x32);

    sequenceConfig.field04 = 0xC8;
    sequenceConfig.field00 = (void *)0x20;
    sequenceConfig.field08 = 0x10;
    sequenceConfig.field10 = 5;
    sequenceConfig.field0C = gOverlay5SoundState;
    gsSndpNew(&sequenceConfig);

    func_80001BA0();
    func_80000450(0);
    func_8002B768(resource);
    func_800039F0();
    alSurround_OutputType(4);
    alSurround_ReverbSetup(0, 3);
    osCreateMesgQueue(gOverlay5MessageQueue, gOverlay5MessageBuffer, 1);
    n_alCSPSetMessageQ(gOverlay5Player0, gOverlay5MessageQueue);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o005/overlay5InitializeAudio/func_overlay_005_F000031C_185B744.s")
#endif
