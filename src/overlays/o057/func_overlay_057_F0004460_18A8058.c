#include "PR/ultratypes.h"

typedef struct O57MiddlePoint {
    u8 pad00[0x0C];
    s16 x;
    s16 y;
} O57MiddlePoint;

typedef struct O57MiddleChoice {
    u8 pad00[0x28];
    s16 tableIndex;
    s8 enabled;
    u8 pad2B[9];
} O57MiddleChoice;

typedef struct O57MiddleOutput {
    u8 mode;
    u8 selection;
    u8 group;
    u8 kind;
    s8 value;
    u8 pad05[0x23];
} O57MiddleOutput;

typedef struct O57MiddleLocals {
    u8 pad00[0x10];
    s8 enabled[4];
    u8 pad14[4];
    s32 resultCount;
    u8 pad1C[8];
    u8 available[10];
} O57MiddleLocals;

extern O57MiddlePoint gO57MiddlePoint21C;
extern s16 gO57MiddleInputAxisReloc;
extern u8 gO57MiddleTableIndexReloc;
extern s32 gO57MiddleTableLimitReloc;
extern s32 gO57MiddleState144BaseReloc[];
extern s32 gO57MiddleSelectionChangedStartBaseReloc[];
extern s32 gO57MiddleSelectionChangedIncBaseReloc[];
extern s32 gO57MiddleSelectionChangedDecBaseReloc[];
extern u32 gO57MiddleInputFlagsReloc;
extern s32 gO57MiddleGate50C;
extern s32 gO57MiddleModeReloc;
extern s32 gO57MiddleResultCountReloc;
extern O57MiddleChoice gO57MiddleChoicesReloc[];
extern O57MiddleChoice gO57MiddleChoicesEndReloc[];
extern s16 gO57MiddleValueTable36C[];
extern O57MiddleOutput *gO57MiddleOutputs1B8;
extern u8 gO57MiddleFirstByteReloc;
extern u8 gO57MiddleSecondByteReloc;
extern u8 gO57MiddleThirdByteReloc;
extern u8 gO57MiddleFourthByteReloc;
extern s16 gO57MiddleShortReloc;
extern u8 gO57MiddleSixthByteReloc;
extern u8 gO57MiddleSubmitArgReloc;
extern u8 gO57MiddleConditionalByteReloc;
extern s32 gO57MiddleStateReloc;
extern u32 gO57MiddleFlagsReloc;
extern s16 gO57MiddleUnlockTableReloc[][8];
extern s32 gO57MiddlePositiveReloc;
extern s16 gO57MiddleIndexReloc;
extern s32 gO57MiddlePublishedValueReloc;
extern s32 gO57MiddlePublishedChoiceReloc;
extern s32 gO57MiddlePublishedModeReloc;
extern s32 gO57MiddlePublishedZeroReloc;

extern s32 gO57MiddlePrimaryIds134[];
extern s32 gO57MiddlePrimaryValues17C[];
extern s32 gO57MiddleSecondaryIds1A0[];
extern u8 gO57MiddleAlternateByte183;
extern s32 gO57MiddleTimer118;
extern s32 gO57MiddleState11C;

#define O57_SELECTION_START gO57MiddleSelectionChangedStartBaseReloc[0x130 / 4]
#define O57_SELECTION_INC gO57MiddleSelectionChangedIncBaseReloc[0x130 / 4]
#define O57_SELECTION_DEC gO57MiddleSelectionChangedDecBaseReloc[0x130 / 4]
#define O57_STATE144 gO57MiddleState144BaseReloc[0x144 / 4]

extern void overlay57ApplyTable(void);
extern void overlay57SetNodeValue(s32 id, s32 argument, f32 value);
extern void o57MiddleBeginReloc(s32 command, s32 argument);
extern s32 o57MiddleGetResultCountReloc(void);
extern void o57MiddleSelectModeReloc(s32 mode);
extern void o57MiddlePrepareSingleReloc(void);
extern void o57MiddleSubmitEnabledReloc(const s8 *enabled);
extern void o57MiddleSubmitModeReloc(u8 mode);
extern void o57MiddleApplyConditionalReloc(void);
extern void o57MiddleClearConditionalReloc(s32 value);
extern void o57MiddleMarkUnlockReloc(void);
extern void o57MiddleNotifyUnlockReloc(s32 value);
extern void o57MiddleEmitReloc(s32 kind, s32 value, s32 arg2, s32 arg3,
                               s32 arg4, s32 arg5);
extern void o57MiddleFinishPrimaryReloc(s32 value);
extern void o57MiddlePrepareAlternateReloc(void);
extern void o57MiddleApplyAlternateByteReloc(u8 value);
extern void o57MiddleSetAlternateReloc(s32 value);
extern void o57MiddleApplyPrimaryReloc(u8 id);
extern void o57MiddleApplySecondaryReloc(u8 id);
extern void o57MiddleFinishAlternateReloc(s32 value);

/* Overlay 57 text +0x4460..+0x4C18. */
/* Plateau: canonical -O2 -mips2 is 0x24 bytes larger than the 0x7B8-byte
 * target and differs in 359 of 494 masked words, first at +0x0. The original
 * 0x58-byte frame splits its two local byte arrays around a scalar spill and
 * keeps a broad imported-symbol register web; the best natural C uses a
 * 0x60-byte frame and resisted the bounded type, scope, and layout variants. */
#ifdef NON_MATCHING
void func_overlay_057_F0004460_18A8058(s32 updateRate) {
    O57MiddleLocals locals;
    register u32 inputFlags;
    register s32 i;
    register s32 value;
    register s32 *idPtr;

    O57_STATE144 = 0;

    {
        s32 easeIndex;

        easeIndex = 0;
        if (updateRate > 0) {
            do {
                gO57MiddlePoint21C.x =
                    (s16)(gO57MiddlePoint21C.x +
                          ((0x104 - gO57MiddlePoint21C.x) >> 3));
                gO57MiddlePoint21C.y =
                    (s16)(gO57MiddlePoint21C.y +
                          ((0xBE - gO57MiddlePoint21C.y) >> 3));
                easeIndex++;
            } while (easeIndex != updateRate);
        }
    }

    if ((gO57MiddleInputAxisReloc < -0x10) &&
        (gO57MiddleGate50C == 0)) {
        if (gO57MiddleTableIndexReloc < gO57MiddleTableLimitReloc) {
            gO57MiddleTableIndexReloc++;
            O57_SELECTION_INC = 1;
        }
    } else if ((gO57MiddleInputAxisReloc >= 0x11) &&
               (gO57MiddleGate50C == 0)) {
        if (gO57MiddleTableIndexReloc > 0) {
            gO57MiddleTableIndexReloc--;
            O57_SELECTION_DEC = 1;
        }
    }

    overlay57ApplyTable();
    inputFlags = gO57MiddleInputFlagsReloc;

    if (((inputFlags & 0x9000) != 0) &&
        (gO57MiddleGate50C == 0)) {
        o57MiddleBeginReloc(0xC, 0);
        locals.resultCount = o57MiddleGetResultCountReloc();
        o57MiddleSelectModeReloc(gO57MiddleModeReloc);

        if (gO57MiddleResultCountReloc == 1) {
            gO57MiddleOutputs1B8[0].mode = 0;
            gO57MiddleFirstByteReloc = 6;
            gO57MiddleSecondByteReloc = 5;
            gO57MiddleThirdByteReloc = 0;
            if (gO57MiddleTableIndexReloc == 3) {
                gO57MiddleFourthByteReloc = 2;
                gO57MiddleSixthByteReloc = 1;
            } else {
                gO57MiddleFourthByteReloc = gO57MiddleTableIndexReloc;
                gO57MiddleSixthByteReloc = 0;
            }
            gO57MiddleShortReloc = 0x3FC;
        } else {
            gO57MiddleOutputs1B8[0].mode = 3;
            if (((gO57MiddleModeReloc == 2) ||
                 (gO57MiddleModeReloc == 3)) &&
                (gO57MiddleConditionalByteReloc != 0)) {
                gO57MiddleFirstByteReloc = 4 - gO57MiddleModeReloc;
                gO57MiddleSecondByteReloc = 4;
            } else {
                gO57MiddleFirstByteReloc = 0;
                gO57MiddleSecondByteReloc = gO57MiddleModeReloc;
            }
            gO57MiddleThirdByteReloc = 1;
            gO57MiddleFourthByteReloc = 2;
            gO57MiddleShortReloc = gO57MiddleSubmitArgReloc;
            gO57MiddleSixthByteReloc =
                (gO57MiddleTableIndexReloc == 3) ? 1 : 0;
        }

        i = 0;
        do {
            locals.available[i] = 1;
            i++;
        } while (i != 10);

        if (gO57MiddleModeReloc == 1) {
            o57MiddlePrepareSingleReloc();
        }

        inputFlags = 0;
        i = 0;
        do {
            locals.enabled[i] = gO57MiddleChoicesReloc[i].enabled;
            if (gO57MiddleChoicesReloc[i].enabled != 0) {
                gO57MiddleOutputs1B8[inputFlags].value =
                    (s8)gO57MiddleValueTable36C[
                        gO57MiddleChoicesReloc[i].tableIndex];
                locals.available[gO57MiddleValueTable36C[
                    gO57MiddleChoicesReloc[i].tableIndex]] = 0;
                inputFlags++;
            }
            i++;
        } while (&gO57MiddleChoicesReloc[i] != gO57MiddleChoicesEndReloc);

        if (inputFlags < 6) {
            value = 0;
            inputFlags *= sizeof(O57MiddleOutput);
            i = 0;
            do {
                if (locals.available[i] == 0) {
                    do {
                        i++;
                        value++;
                    } while (locals.available[i] == 0);
                }
                gO57MiddleOutputs1B8[inputFlags /
                                    sizeof(O57MiddleOutput)].value = value;
                inputFlags += sizeof(O57MiddleOutput);
                value++;
                i++;
            } while (inputFlags < 6 * sizeof(O57MiddleOutput));
        }

        o57MiddleSubmitEnabledReloc(locals.enabled);
        gO57MiddleOutputs1B8[0].group = (u8)locals.resultCount;
        gO57MiddleOutputs1B8[0].selection = 0;
        gO57MiddleOutputs1B8[0].kind = 3;

        if ((gO57MiddleSubmitArgReloc == 0) ||
            ((gO57MiddleSubmitArgReloc != 0) &&
             (gO57MiddleConditionalByteReloc != 0))) {
            o57MiddleSubmitModeReloc(gO57MiddleSubmitArgReloc);
            if (gO57MiddleConditionalByteReloc != 0) {
                gO57MiddleConditionalByteReloc = 0;
            }
        }

        i = gO57MiddleStateReloc;
        gO57MiddlePublishedZeroReloc = 0;
        if (i == 1) {
            if ((gO57MiddleFlagsReloc & 0x20000) != 0) {
                gO57MiddleFlagsReloc &= ~0x20000;
                o57MiddleApplyConditionalReloc();
                o57MiddleClearConditionalReloc(0);
            }
            i = gO57MiddleUnlockTableReloc[gO57MiddleOutputs1B8[0].group]
                                                [gO57MiddleOutputs1B8[0].selection];
            if ((i != -1) &&
                ((gO57MiddleFlagsReloc & (1 << i)) == 0)) {
                gO57MiddleFlagsReloc |= 1 << i;
                o57MiddleMarkUnlockReloc();
                o57MiddleNotifyUnlockReloc(i + 0xE);
            }
        }

        if (gO57MiddlePositiveReloc > 0) {
            gO57MiddlePublishedValueReloc =
                gO57MiddleUnlockTableReloc[gO57MiddleOutputs1B8[0].group]
                                             [gO57MiddleOutputs1B8[0].selection];
            gO57MiddlePublishedChoiceReloc =
                gO57MiddleValueTable36C[gO57MiddleIndexReloc];
            gO57MiddlePublishedModeReloc = 5;
            gO57MiddlePublishedZeroReloc = 0;
            o57MiddleEmitReloc(0x12, 0, 0, 0xF, 1, 0);
            o57MiddleFinishPrimaryReloc(1);
        } else {
            o57MiddlePrepareAlternateReloc();
            o57MiddleEmitReloc(
                gO57MiddleUnlockTableReloc[gO57MiddleOutputs1B8[0].group]
                                              [gO57MiddleOutputs1B8[0].selection],
                gO57MiddleValueTable36C[gO57MiddleIndexReloc],
                0, 5, 1, 0);
        }
        gO57MiddleGate50C = 1;
        o57MiddleApplyConditionalReloc();
        return;
    }

    if (((inputFlags & 0x4000) != 0) &&
        (gO57MiddleGate50C == 0)) {
        o57MiddleBeginReloc(0xD, 0);
        o57MiddlePrepareAlternateReloc();
        o57MiddleApplyAlternateByteReloc(gO57MiddleAlternateByte183);
        gO57MiddleTimer118 = 7;
        gO57MiddleState11C = 0;
        o57MiddleSetAlternateReloc(3);

        idPtr = gO57MiddlePrimaryIds134;
        while (*idPtr != -1) {
            o57MiddleApplyPrimaryReloc((u8)*idPtr);
            overlay57SetNodeValue(*idPtr, gO57MiddlePrimaryValues17C[*idPtr],
                                  0.0070000002f);
            idPtr++;
        }

        idPtr = gO57MiddleSecondaryIds1A0;
        while (*idPtr != -1) {
            o57MiddleApplySecondaryReloc((u8)*idPtr);
            idPtr++;
        }
        o57MiddleFinishAlternateReloc(0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o057/func_overlay_057_F0004460_18A8058/func_overlay_057_F0004460_18A8058.s")
#endif
