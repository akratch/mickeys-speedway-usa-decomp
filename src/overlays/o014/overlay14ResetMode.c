#include "PR/ultratypes.h"

typedef struct Overlay14Command { s16 pad0; s16 type; s32 value; } Overlay14Command;

extern s32 gOverlay14CountEC;
extern s32 gOverlay14CurrentTypeE4;
extern s32 gOverlay14CurrentValueFC;
extern s32 gOverlay14ValueF8;
extern s32 gOverlay14ValueD8;
extern s32 gOverlay14ValueDC;
extern s32 gOverlay14ValueE0;
extern Overlay14Command gOverlay14Commands128[];
extern void overlay14Dispatch(void);
extern s32 overlay14Advance(s32);

void overlay14ResetMode(void) {
    Overlay14Command *command;
    do {
        if (gOverlay14CountEC <= 0) return;
        gOverlay14CountEC--;
        if (gOverlay14CountEC <= 0) return;
        overlay14Dispatch();
        command = &gOverlay14Commands128[gOverlay14CountEC - 1];
        gOverlay14CurrentTypeE4 = command->type;
        if (command->type != 1) {
            gOverlay14ValueF8 = command->value;
            gOverlay14ValueD8 = 0;
            gOverlay14ValueDC = 0;
            gOverlay14ValueE0 = 0;
            return;
        }
        gOverlay14CurrentValueFC = command->value;
    } while (overlay14Advance(0) == 0);
}
