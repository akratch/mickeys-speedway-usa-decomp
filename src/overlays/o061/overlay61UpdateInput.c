#include "PR/ultratypes.h"

extern s32 gOverlay61RepeatXReloc;
extern s32 gOverlay61RepeatYReloc;

extern s32 overlay61ReadButtonsReloc(s32 controller);
extern s32 overlay61ReadXReloc(s32 controller);
extern s32 overlay61ReadYReloc(s32 controller);

void overlay61UpdateInput(s32 *xDirection, s32 *yDirection, s32 *confirm,
                          s32 *cancel) {
    s32 input;

    if ((overlay61ReadButtonsReloc(0) & 0x9000) != 0) {
        *confirm = 1;
    } else {
        *confirm = 0;
    }

    if ((overlay61ReadButtonsReloc(0) & 0x4000) != 0) {
        *cancel = 1;
    } else {
        *cancel = 0;
    }

    input = overlay61ReadXReloc(0);
    if (input >= 31) {
        if (gOverlay61RepeatXReloc > 0) {
            gOverlay61RepeatXReloc--;
            *xDirection = 0;
        } else {
            gOverlay61RepeatXReloc = 10;
            *xDirection = 1;
        }
    } else if (overlay61ReadXReloc(0) < -30) {
        if (gOverlay61RepeatXReloc < 0) {
            gOverlay61RepeatXReloc++;
            *xDirection = 0;
        } else {
            gOverlay61RepeatXReloc = -10;
            *xDirection = -1;
        }
    } else {
        gOverlay61RepeatXReloc = 0;
        *xDirection = 0;
    }

    input = overlay61ReadYReloc(0);
    if (input >= 31) {
        if (gOverlay61RepeatYReloc > 0) {
            gOverlay61RepeatYReloc--;
            *yDirection = 0;
        } else {
            gOverlay61RepeatYReloc = 10;
            *yDirection = 1;
        }
    } else if (overlay61ReadYReloc(0) < -30) {
        if (gOverlay61RepeatYReloc < 0) {
            gOverlay61RepeatYReloc++;
            *yDirection = 0;
        } else {
            gOverlay61RepeatYReloc = -10;
            *yDirection = -1;
        }
    } else {
        gOverlay61RepeatYReloc = 0;
        *yDirection = 0;
    }
}
