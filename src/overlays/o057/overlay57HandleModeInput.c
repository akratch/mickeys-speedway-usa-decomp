#include "PR/ultratypes.h"

typedef struct Overlay57ModeChoice {
    u8 pad00[0x28];
    s16 tableIndex;
    s8 enabled;
    u8 pad2B[9];
} Overlay57ModeChoice;

typedef struct Overlay57ModeOutput {
    u8 mode;
    u8 pad01[3];
    s8 value;
    u8 pad05[0x23];
} Overlay57ModeOutput;

extern u32 gO57ModeInputFlags;
extern s32 gOverlay57ModeFlag;
extern s32 gOverlay57State;
extern s32 gOverlay57DistanceState;
extern s32 gOverlay57Delay;
extern s32 gOverlay57Timer;
extern s16 gO57ModeValueTable[];
extern Overlay57ModeChoice gO57ModeChoices[4];
extern Overlay57ModeChoice gO57ModeChoicesEnd[];
extern Overlay57ModeOutput *gO57ModeOutputs;
extern u8 gO57ModeFirstByte;
extern u8 gO57ModeSecondByte;
extern u8 gO57ModeThirdByte;
extern u8 gO57ModeFourthByte;
extern s16 gO57ModeShortValue;
extern u8 gO57ModeSixthByte;
extern s32 gO57ModePrimaryIds[];
extern s32 gO57ModePrimaryValues[];
extern s32 gO57ModeSecondaryIds[];
extern u8 gO57ModeExternalByte;
extern u8 gO57ModeFinalArg0[];
extern u8 gO57ModeFinalArg1[];

extern void o57ModeBegin12Reloc(s32 choice, s32 value);
extern void o57ModeBegin13Reloc(s32 choice, s32 value);
extern void o57ModeSubmitEnabledReloc(const s8 values[4]);
extern void o57ModeSelectChoiceReloc(s32 choice);
extern void o57ModeEmitChoiceReloc(s32 kind, s32 value, s32 arg2, s32 arg3,
                                   s32 arg4, s32 arg5);
extern void o57ModeApplyTableReloc(void);
extern void o57ModePrepareAlternateReloc(void);
extern void o57ModeApplyExternalByteReloc(u8 value);
extern void o57ModeSetAlternateReloc(s32 value);
extern void o57ModeApplyPrimaryReloc(u8 id);
extern void o57ModeApplySecondaryReloc(u8 id);
extern void o57ModeFinishAlternateReloc(s32 value);
extern void overlay57SetNodeValue(s32 id, s32 argument, f32 value);
extern void o57ModeSubmitFinalReloc(void *object, void *state, f32 position,
                                   f32 extent, f32 scaleX, f32 scaleY,
                                   s32 index, s32 mode);

/* Overlay 57 text +0x4064..+0x43C8. */
/* Plateau: canonical -O2 -mips2 is exact-size at 0x364 and differs in 13
 * words, first at +0xE0.  Reusing inputFlags as the compacted output index
 * fixes the output-table carrier; the remaining initializer schedule and
 * private a1/a2 loop web resisted the bounded source variants. */
#ifdef NON_MATCHING
void overlay57HandleModeInput(s32 updateRate) {
    u32 inputFlags;
    s32 i;
    s32 *idPtr;
    s8 enabled[4];

    gOverlay57ModeFlag = 0;
    gOverlay57State = 3;
    inputFlags = gO57ModeInputFlags;

    if (((inputFlags & 0x9000) != 0) &&
        (gOverlay57DistanceState == 0)) {
        o57ModeBegin12Reloc(12, 0);

        gO57ModeOutputs[0].value =
            (s8) gO57ModeValueTable[gO57ModeChoices[0].tableIndex];
        gO57ModeOutputs[0].mode = 2;
        gO57ModeFirstByte = 1;
        gO57ModeSecondByte = 0;
        gO57ModeThirdByte = 0;
        gO57ModeFourthByte = 0;
        gO57ModeShortValue = 0x3FC;
        gO57ModeSixthByte = 0;

        inputFlags = 0;
        i = 0;
        do {
            enabled[i] = gO57ModeChoices[i].enabled;
            if (gO57ModeChoices[i].enabled != 0) {
                gO57ModeOutputs[inputFlags].value =
                    (s8) gO57ModeValueTable[
                        gO57ModeChoices[i].tableIndex];
                inputFlags++;
            }
            i++;
        } while (&gO57ModeChoices[i] != gO57ModeChoicesEnd);

        o57ModeSubmitEnabledReloc(enabled);
        o57ModeSelectChoiceReloc(0);
        o57ModeEmitChoiceReloc(0x1E,
                               gO57ModeValueTable[
                                   gO57ModeChoices[0].tableIndex],
                               0, 5, 1, 0);
        gOverlay57DistanceState = 1;
        o57ModeApplyTableReloc();
    } else if (((inputFlags & 0x4000) != 0) &&
               (gOverlay57DistanceState == 0)) {
        o57ModeBegin13Reloc(13, 0);
        o57ModePrepareAlternateReloc();
        o57ModeApplyExternalByteReloc(gO57ModeExternalByte);
        gOverlay57Timer = 7;
        gOverlay57State = 0;
        o57ModeSetAlternateReloc(2);

        idPtr = gO57ModePrimaryIds;
        while (*idPtr != -1) {
            o57ModeApplyPrimaryReloc((u8) *idPtr);
            overlay57SetNodeValue(*idPtr, gO57ModePrimaryValues[*idPtr],
                                  0.0070000002f);
            idPtr++;
        }

        idPtr = gO57ModeSecondaryIds;
        while (*idPtr != -1) {
            o57ModeApplySecondaryReloc((u8) *idPtr);
            idPtr++;
        }
        o57ModeFinishAlternateReloc(0);
    }

    for (i = 0; i < updateRate; i++) {
        gOverlay57Delay -= gOverlay57Delay >> 3;
    }

    o57ModeSubmitFinalReloc(gO57ModeFinalArg0, gO57ModeFinalArg1,
                            (f32) ((gOverlay57Delay >> 5) + 0x104), 184.0f,
                            1.0f, 1.0f, -2, 3);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o057/overlay57HandleModeInput/func_overlay_057_F0004064_18A7C5C.s")
#endif
