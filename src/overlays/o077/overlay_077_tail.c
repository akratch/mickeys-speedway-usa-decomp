#include "PR/ultratypes.h"

/* Overlay 77 state tail; exact DKR and JFG scans are negative. */
extern volatile s32 gOverlay77Handle;
extern s32 gOverlay77Count;
extern s32 gOverlay77Selection;
extern void *gOverlay77CallbackArgument;

s32 overlay77RandomReloc(s32 minimum, s32 maximum);
void overlay77CallbackReloc(void *argument);

void overlay77EnsureSelection(void) {
    if (gOverlay77Handle == 0) {
        s32 count = gOverlay77Count;

        gOverlay77Handle = count;
        gOverlay77Selection = overlay77RandomReloc(0, count - 1);
    }
}

void overlay77RunCallback(void) {
    void *argument = gOverlay77CallbackArgument;

    if (argument != 0) {
        overlay77CallbackReloc(argument);
    }
}
