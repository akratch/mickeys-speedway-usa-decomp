#include "PR/ultratypes.h"

extern u8 gOverlay55SetupBase[];
extern u8 gOverlay55SourceBase[];
extern u8 gOverlay55ExternalSetupTarget[];
extern u8 gOverlay55SetupSecondary[];
extern u8 gOverlay55RecordsA[];
extern u8 gOverlay55RecordsB[];
extern u8 gOverlay55SourceB[];
extern s8 gOverlay55Status[];
extern s32 gOverlay55StateWord;
extern f32 gOverlay55StateValue;
extern s16 gOverlay55Result;

extern void overlay55SetupFirstReloc(void *target);
extern void overlay55SetupSecondReloc(void *target);
extern void overlay55SetModeReloc(s32 mode);
extern void overlay55SetResourceReloc(s32 resource);
extern void overlay55PatchIndices(void *records);
extern void overlay55CopyOffsetRecords(
    void *records, void *source, s32 index, s32 offset);
extern void overlay56InitializeReloc(void);
extern s32 overlay55GetResultReloc(void);

/* NON_MATCHING plateau (2026-08-25): all 119 flag combinations and a bounded
 * ten-minute two-worker permutation pass retain two of 79 differing words,
 * first at +0x94. Retail schedules the gOverlay55Status low half before the
 * gOverlay55SourceBase low half; natural C emits those two in reverse order. */
#ifdef NON_MATCHING
void overlay55Initialize(void) {
    s32 index;
    u8 *sourceA;
    u8 *sourceB;
    s8 *status;

    overlay55SetupFirstReloc(gOverlay55SetupBase);
    overlay55SetupFirstReloc(gOverlay55ExternalSetupTarget);
    overlay55SetupSecondReloc(gOverlay55SetupSecondary);
    overlay55SetModeReloc(4);
    gOverlay55StateWord = 0x104;
    overlay55SetResourceReloc(11);
    overlay55PatchIndices(gOverlay55RecordsA);
    overlay55PatchIndices(gOverlay55RecordsB);

    sourceA = gOverlay55SourceBase;
    sourceB = gOverlay55SourceB;
    status = gOverlay55Status;
    index = 0;
    do {
        overlay55CopyOffsetRecords(gOverlay55RecordsA, sourceA, index, 0);
        overlay55CopyOffsetRecords(gOverlay55RecordsB, sourceB, index, 0);
        index++;
        sourceA += 0xA0;
        sourceB += 0x20;
        status++;
        status[-1] = -1;
    } while (index != 4);

    gOverlay55StateValue = -80.0f;
    overlay56InitializeReloc();
    gOverlay55Result = (s16)overlay55GetResultReloc();
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o055/overlay55Initialize/func_overlay_055_F0000000_18A1B18.s")
#endif
