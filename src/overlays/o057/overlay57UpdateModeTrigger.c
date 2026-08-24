#include "PR/ultratypes.h"

typedef struct Overlay57ModeObject {
    s32 value;
} Overlay57ModeObject;

typedef struct Overlay57ModeResult {
    u8 pad00[0x16];
    u8 flags16;
} Overlay57ModeResult;

extern s32 gOverlay57Countdown;
extern s32 gOverlay57Timer;
extern s32 gOverlay57ModeFlag;
extern s32 gOverlay57SetupValues[6];
extern Overlay57ModeObject gOverlay57Object;
extern u8 gOverlay57ObjectId;
extern s32 gOverlay57SetupStatus;
extern s32 gOverlay57SetupDelay;
extern s32 gOverlay57TriggerLatched;
extern s32 gOverlay57ObjectStatus;

/* Physical a0 retains updateRate at this site; consumption is not proven. */
extern void *overlay57TailQueryModeReloc(void);
extern s32 overlay57TailQueryChoiceReloc(s32 mode);
extern void overlay57TailPrepareObjectReloc(u8 id);
extern void overlay57TailStartObjectReloc(u8 id);
extern Overlay57ModeResult *overlay57TailFindObjectReloc(u8 id);
extern void overlay57SetNodeValue(s32 id, s32 argument, f32 value);
extern void overlay57AdvanceReloc(s32 updateRate);

/* Pinned DKR v77/v80 and JFG scans found no exact Overlay 57 donor. */
#ifdef NON_MATCHING
void overlay57UpdateModeTrigger(s32 updateRate) {
    s32 trigger;

    gOverlay57ModeFlag = 1;

    if (gOverlay57Countdown > 0) {
        gOverlay57Countdown = gOverlay57Countdown - updateRate;
        if (gOverlay57Countdown <= 0) {
            trigger = 1;
        }
    }

    gOverlay57SetupStatus = 0;
    if (overlay57TailQueryModeReloc() == 0) {
        volatile s32 seed;
        s32 index;
        s32 base;
        Overlay57ModeResult *result;

        seed = 2;
        index = seed;
        base = ((index * 4) - index) << 4;

        gOverlay57SetupValues[1] = 0x30;
        gOverlay57SetupValues[0] = 0;
        gOverlay57SetupValues[index + 3] = base + 0x90;
        gOverlay57SetupValues[index + 2] = base + 0x60;
        gOverlay57SetupValues[index + 1] = base + 0x30;
        gOverlay57SetupValues[index] = base;

        trigger = 1;
        if (overlay57TailQueryChoiceReloc(index) == 5) {
            gOverlay57Timer = 10;
        }

        gOverlay57ObjectStatus = 0;
        gOverlay57Object.value = 80;
        overlay57TailPrepareObjectReloc(((u8 *)&gOverlay57Object)[3]);
        overlay57TailStartObjectReloc(gOverlay57ObjectId);
        result = overlay57TailFindObjectReloc(gOverlay57ObjectId);
        if (result != 0) {
            result->flags16 |= 2;
        }

        if (gOverlay57TriggerLatched != 0) {
            gOverlay57SetupDelay = 0;
        } else {
            gOverlay57SetupDelay = 60;
        }
    }

    /* Retail reads this deliberately uninitialized value when neither path
     * above assigns it. Initializing it changes the physical function. */
    if ((trigger != 0) && (gOverlay57TriggerLatched == 0)) {
        overlay57SetNodeValue(0x2F, 0, 0.012f);
        gOverlay57TriggerLatched = 1;
    }

    overlay57AdvanceReloc(updateRate);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o057/overlay57UpdateModeTrigger/func_overlay_057_F0004C18_18A8810.s")
#endif
