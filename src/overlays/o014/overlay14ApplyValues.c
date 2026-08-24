#include "PR/ultratypes.h"

typedef struct Overlay14QueuedCommand {
    s16 value;
    s16 mode;
    s32 result;
} Overlay14QueuedCommand;

extern s32 gOverlay14StateC8;
extern s32 gOverlay14PendingValueD0;
extern s32 gOverlay14PendingModeD4;
extern s32 gOverlay14TransitionD8;
extern s32 gOverlay14CursorDC;
extern s32 gOverlay14PointerE0;
extern s32 gOverlay14ModeE4;
extern s32 gOverlay14CommandCountEC;
extern s32 gOverlay14ResultF8;
extern s32 gOverlay14ResultFC;
extern s32 gOverlay14Value108;
extern Overlay14QueuedCommand gOverlay14QueuedCommands128[];

extern s32 overlay14CreateValue(s32 value, s32 mode);
extern s32 overlay14MoveCommandCursor(s32 step);

s32 overlay14ApplyValues(s32 value, s32 mode) {
    s32 result;
    s32 index;
    Overlay14QueuedCommand *command;

    result = 0;
    if (gOverlay14StateC8 != 0) {
        gOverlay14PendingValueD0 = value;
        gOverlay14PendingModeD4 = mode;
        return 0;
    }

    gOverlay14PendingValueD0 = -1;
    gOverlay14PendingModeD4 = -1;
    if (gOverlay14CommandCountEC < 0x10) {
        if (mode == 2) {
            gOverlay14Value108 = value;
            gOverlay14StateC8 = 2;
        } else if (mode == 1) {
            gOverlay14ResultFC = overlay14CreateValue(value, 1);
            if (overlay14MoveCommandCursor(0) != 0) {
                gOverlay14ModeE4 = 1;
                result = gOverlay14ResultFC;
            }
        } else {
            gOverlay14ResultF8 = overlay14CreateValue(value, 4);
            gOverlay14TransitionD8 = 0;
            gOverlay14CursorDC = 0;
            gOverlay14PointerE0 = 0;
            gOverlay14ModeE4 = mode;
            result = gOverlay14ResultF8;
        }

        if (result != 0) {
            index = gOverlay14CommandCountEC;
            command = &gOverlay14QueuedCommands128[index];
            command->value = value;
            command->mode = mode;
            command->result = result;
            gOverlay14CommandCountEC = index + 1;
        }
    }
    return result;
}
