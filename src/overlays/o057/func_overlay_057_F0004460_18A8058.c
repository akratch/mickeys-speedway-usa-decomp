#include "PR/ultratypes.h"

typedef struct O57MenuTransition {
    u8 pad00[0xC];
    s16 horizontal;
    s16 vertical;
} O57MenuTransition;

typedef struct O57MenuChoice {
    u8 pad00[0x28];
    s16 tableIndex;
    s8 enabled;
    u8 pad2B[9];
} O57MenuChoice;

typedef struct O57MenuOutput {
    u8 type;
    u8 variant;
    u8 mode;
    u8 subtype;
    s8 controller;
    u8 pad05[0x23];
} O57MenuOutput;

typedef struct O57MenuLink {
    s32 index;
} O57MenuLink;

extern s32 gOverlay57ModeFlag;
extern O57MenuTransition gO57ModeSetup21C;
extern s16 gO57MenuInputXReloc;
extern u8 gO57MenuSelectionReloc;
extern s32 gO57MenuSelectionCountReloc;
extern s32 gOverlay57DistanceState;
extern s32 gOverlay57LayoutBusy;
extern u32 gO57ModeInputFlags;
extern s32 gO57MenuKindReloc;
extern s32 gO57MenuCameraReloc;
extern s32 gO57MenuModeReloc;
extern s32 gO57MenuUnlockCountReloc;
extern s32 gO57MenuTransitionFlagReloc;
extern O57MenuChoice gO57ModeChoices[4];
extern O57MenuChoice gO57ModeChoicesEnd[];
extern s16 gO57ModeValueTable[];
extern s16 gO57MenuLevelTableReloc[][4];
extern s16 gO57MenuCourseTableReloc[][4];
extern s16 gO57MenuEntranceTableReloc[];
extern s16 gO57MenuEntranceIndexReloc;
extern s32 gO57MenuLevelReloc;
extern s32 gO57MenuEntranceReloc;
extern s32 gO57MenuVehicleReloc;
extern s32 gO57MenuCutsceneReloc;
extern s32 gO57MenuPrimaryValues[];
extern O57MenuLink gO57ModePrimaryIds[];
extern O57MenuLink gO57ModeSecondaryIds[];
extern O57MenuOutput *gO57ModeOutputs;
extern u8 gO57ModeFirstByte;
extern u8 gO57ModeSecondByte;
extern u8 gO57ModeThirdByte;
extern u8 gO57ModeFourthByte;
extern s16 gO57ModeShortValue;
extern u8 gO57ModeSixthByte;
extern u8 gO57MenuMirrorReloc;
extern u8 gO57MenuMirrorEnabledReloc;
extern u8 gOverlay57ObjectId;
extern s32 gOverlay57Timer;
extern s32 gOverlay57State;

extern void amSndPlay(s32 soundId, void *handle);
extern s32 overlay84GetEnabledCurrent(void);
extern void overlay84ClearMode(void);
extern void overlay84ActivateCurrent(s32 kind);
extern void overlay84Mark(void);
extern void mainChangeCameras(s32 camera);
extern void mainChangeLevel(s32 level, s32 entrance, s32 vehicle,
                            s32 cutscene, s32 multiplayer, s32 arg5);
extern void mainSetAnimGroup(s32 group);
extern void mainSetMode(s32 mode);
extern void joyCreateMap(s8 *enabled);
extern void animseqStartPath(s32 pathId);
extern void animseqStopPath(s32 pathId);
extern void overlay57ApplyTable(void);
extern void overlay57SetNodeValue(s32 id, s32 argument, s32 valueBits);
extern void o57MenuPrepareReloc(void);
extern void o57MenuResetReloc(s32 value);
extern void o57MenuApplyCameraReloc(s32 camera);
extern void o57MenuClearUnlockReloc(void);
extern void o57MenuCommitUnlockReloc(s32 value);

/* Workbench p4: structure-mismatch; 449 positional/450 raw words differ,
 * 458/494 instructions, first +0x4, frame exact -88. Levers: selection/output
 * pointer lifetime and declaration order; remains saved-register web. */
#ifdef NON_MATCHING
void func_overlay_057_F0004460_18A8058(s32 updateRate) {
    s8 activePlayers[10];
    s8 enabled[4];
    s32 i;
    s32 x;
    s32 y;
    s32 current;

    gOverlay57ModeFlag = 0;
    i = 0;
    if (updateRate > 0) {
        do {
            x = gO57ModeSetup21C.horizontal;
            y = gO57ModeSetup21C.vertical;
            i++;
            gO57ModeSetup21C.horizontal =
                (s16)(x + ((0x104 - x) >> 3));
            gO57ModeSetup21C.vertical =
                (s16)(y + ((0xBE - y) >> 3));
        } while (i != updateRate);
    }

    if ((gO57MenuInputXReloc < -16) &&
        (gOverlay57DistanceState == 0)) {
        if (gO57MenuSelectionReloc < gO57MenuSelectionCountReloc) {
            gO57MenuSelectionReloc++;
            gOverlay57LayoutBusy = 1;
        }
    } else if ((gO57MenuInputXReloc >= 17) &&
               (gOverlay57DistanceState == 0)) {
        if (gO57MenuSelectionReloc > 0) {
            gO57MenuSelectionReloc--;
            gOverlay57LayoutBusy = 1;
        }
    }

    overlay57ApplyTable();
    if (((gO57ModeInputFlags & 0x9000) != 0) &&
        (gOverlay57DistanceState == 0)) {
        amSndPlay(12, 0);
        current = overlay84GetEnabledCurrent();
        mainChangeCameras(gO57MenuCameraReloc);
        if (gO57MenuKindReloc == 1) {
            gO57ModeOutputs[0].type = 0;
            gO57ModeFirstByte = 6;
            gO57ModeSecondByte = 5;
            gO57ModeThirdByte = 0;
            if (gO57MenuSelectionReloc == 3) {
                gO57ModeFourthByte = 2;
                gO57ModeSixthByte = 1;
            } else {
                gO57ModeFourthByte = gO57MenuSelectionReloc;
                gO57ModeSixthByte = 0;
            }
            gO57ModeShortValue = 0x3FC;
        } else {
            gO57ModeOutputs[0].type = 3;
            if (((gO57MenuSelectionReloc == 2) ||
                 (gO57MenuSelectionReloc == 3)) &&
                (gO57MenuMirrorEnabledReloc != 0)) {
                gO57ModeFirstByte = 4;
                gO57ModeSecondByte = 4 - gO57MenuSelectionReloc;
            } else {
                gO57ModeFirstByte = gO57MenuSelectionReloc;
                gO57ModeSecondByte = 0;
            }
            gO57ModeThirdByte = 1;
            gO57ModeFourthByte = 2;
            gO57ModeShortValue = gO57MenuEntranceIndexReloc;
            gO57ModeSixthByte = (gO57MenuSelectionReloc == 3);
        }

        for (i = 0; i < 10; i++) {
            activePlayers[i] = 1;
        }
        if (gO57MenuKindReloc == 1) {
            o57MenuPrepareReloc();
        }

        x = 0;
        i = 0;
        do {
            enabled[i] = gO57ModeChoices[i].enabled;
            if (gO57ModeChoices[i].enabled != 0) {
                y = gO57ModeValueTable[gO57ModeChoices[i].tableIndex];
                gO57ModeOutputs[x++].controller = y;
                activePlayers[y] = 0;
            }
            i++;
        } while (&gO57ModeChoices[i] != gO57ModeChoicesEnd);

        y = 0;
        i = x * sizeof(O57MenuOutput);
        while (i < 6 * (s32)sizeof(O57MenuOutput)) {
            while (activePlayers[y] == 0) {
                y++;
            }
            ((O57MenuOutput *)((u8 *)gO57ModeOutputs + i))->controller = y++;
            i += sizeof(O57MenuOutput);
        }

        joyCreateMap(enabled);
        gO57ModeOutputs[0].mode = current;
        gO57ModeOutputs[0].variant = 0;
        gO57ModeOutputs[0].subtype = 3;
        if ((gO57MenuMirrorReloc == 0) ||
            (gO57MenuMirrorEnabledReloc != 0)) {
            o57MenuResetReloc(gO57MenuSelectionReloc);
            if (gO57MenuMirrorEnabledReloc != 0) {
                gO57MenuMirrorEnabledReloc = 0;
            }
        }

        gO57MenuTransitionFlagReloc = 0;
        if (gO57MenuKindReloc == 1) {
            o57MenuClearUnlockReloc();
            o57MenuCommitUnlockReloc(0);
        }

        if (gO57MenuUnlockCountReloc > 0) {
            gO57MenuLevelReloc =
                gO57MenuLevelTableReloc[gO57ModeOutputs[0].mode]
                                        [gO57ModeOutputs[0].variant];
            gO57MenuEntranceReloc =
                gO57ModeValueTable[gO57MenuEntranceIndexReloc];
            gO57MenuVehicleReloc = 5;
            gO57MenuCutsceneReloc = 0;
            mainChangeLevel(0x12, 0, 0, 0xF, 1, 0);
            mainSetAnimGroup(1);
        } else {
            mainSetMode(0);
            mainChangeLevel(
                gO57MenuCourseTableReloc[gO57ModeOutputs[0].mode]
                                         [gO57ModeOutputs[0].variant],
                gO57ModeValueTable[gO57MenuEntranceIndexReloc],
                0, 5, 1, 0);
        }
        gOverlay57DistanceState = 1;
        overlay84Mark();
        return;
    }

    if (((gO57ModeInputFlags & 0x4000) != 0) &&
        (gOverlay57DistanceState == 0)) {
        amSndPlay(13, 0);
        overlay84ClearMode();
        animseqStopPath(gOverlay57ObjectId);
        gOverlay57Timer = 7;
        gOverlay57State = 0;
        overlay84ActivateCurrent(3);

        i = 0;
        if (gO57ModePrimaryIds[i].index != -1) {
            do {
                x = gO57ModePrimaryIds[i].index;
                animseqStartPath(x & 0xFF);
                overlay57SetNodeValue(x, gO57MenuPrimaryValues[x],
                                      0x3BE56042);
                i++;
            } while (gO57ModePrimaryIds[i].index != -1);
        }
        i = 0;
        if (gO57ModeSecondaryIds[i].index != -1) {
            do {
                animseqStopPath(gO57ModeSecondaryIds[i].index & 0xFF);
                i++;
            } while (gO57ModeSecondaryIds[i].index != -1);
        }
        gOverlay57State = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o057/func_overlay_057_F0004460_18A8058/func_overlay_057_F0004460_18A8058.s")
#endif
