#include "PR/ultratypes.h"

extern s32 gOverlay14Transition;
extern s32 gOverlay14Cursor;
extern s32 gOverlay14PointerE0;
extern s32 gOverlay14Flag0;
extern s32 gOverlay14Flag4;
extern s32 gOverlay14ModeE4;
extern s32 gOverlay14ValueEC;
extern s32 gOverlay14StateCC;
extern s32 gOverlay14Value110;
extern s32 gOverlay14Value114;

extern s32 overlay14InitializeMode(s32 mode);
extern void overlay14ResetMode(void);
extern void overlay14ApplyValues(s32 value, s32 extra);

void overlay14AdvanceCommand(s32 context) {
    s32 divisor;
    s32 limit;
    s32 extra;

    divisor = overlay14InitializeMode(2);
    if (gOverlay14Transition != 0) {
        if (gOverlay14Transition < 0) {
            gOverlay14Transition += 1;
            gOverlay14Cursor -= 1;
        } else {
            gOverlay14Transition -= 1;
            gOverlay14Cursor += 1;
        }
    } else if ((gOverlay14PointerE0 != 0) && (gOverlay14Flag0 != 0)) {
        gOverlay14Transition = (0x58 / divisor) - 1;
    } else {
        limit = (0x58 / divisor) - 1;
        if ((gOverlay14Cursor >= limit) && (gOverlay14Flag4 != 0)) {
            gOverlay14Transition = -limit;
        }
    }

    if ((gOverlay14PointerE0 == 0) && (gOverlay14Flag0 != 0)) {
        if ((gOverlay14ModeE4 == 4) && (gOverlay14ValueEC >= 2)) {
            overlay14ResetMode();
            return;
        }
        if (gOverlay14ModeE4 == 3) {
            extra = gOverlay14Value114;
            if (extra != 0) {
                overlay14ApplyValues(gOverlay14Value110, extra);
                return;
            }
        }
        gOverlay14StateCC = 1;
        return;
    }
    if ((gOverlay14Flag4 != 0) && (gOverlay14ValueEC >= 2)) {
        overlay14ResetMode();
    }
}
