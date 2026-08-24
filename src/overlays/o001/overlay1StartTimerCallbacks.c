#include "PR/ultratypes.h"

typedef void (*Overlay1Callback)(void);

typedef struct Overlay1CallbackState {
    u8 pad000[0x382];
    u8 mode;
} Overlay1CallbackState;

typedef struct Overlay1CallbackObject {
    u8 pad00[0x64];
    Overlay1CallbackState *state;
} Overlay1CallbackObject;

typedef struct Overlay1CallbackEntry {
    Overlay1Callback callback;
    s32 pad4;
    u16 modeMask;
    u16 padA;
} Overlay1CallbackEntry;

/* Fresh pinned DKR v77/v80 and JFG scans found no Overlay 1 donor. */
extern s32 overlay1IsObjectActive(Overlay1CallbackObject *object);
extern Overlay1CallbackState *gOverlay1TimerState;
extern s32 gOverlay1TimerStep;
extern f32 gOverlay1CallbackStepFloat;
extern Overlay1CallbackEntry gOverlay1CallbackDescriptor[];
extern Overlay1CallbackEntry gOverlay1ModeCallbacks[];

void overlay1StartTimerCallbacks(Overlay1CallbackObject *object, s32 amount) {
    Overlay1CallbackEntry *entry;
    Overlay1Callback callback;
    s32 index;
    u8 mode;

    if (overlay1IsObjectActive(object) != 0) {
        gOverlay1TimerStep = amount;
        gOverlay1CallbackStepFloat = amount;
        entry = gOverlay1CallbackDescriptor;
        for (index = 5; index != 6; index++, entry++) {
            mode = gOverlay1TimerState->mode;
            if (index != mode) {
                callback = entry->callback;
                if (callback != NULL) {
                    if ((entry->modeMask & (1 << mode)) != 0) {
                        callback();
                    }
                }
            }
        }
        mode = gOverlay1TimerState->mode;
        callback = gOverlay1ModeCallbacks[mode].callback;
        if (callback != NULL) {
            callback();
        }
    }
}
