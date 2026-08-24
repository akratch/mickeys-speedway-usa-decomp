#include "PR/ultratypes.h"

typedef struct Overlay1TimerState {
    u8 pad000[0x382];
    u8 active;
    u8 pad383[0xD];
    u16 timer;
} Overlay1TimerState;

/* Fresh pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern Overlay1TimerState *gOverlay1TimerState;
extern s32 gOverlay1TimerStep;

void overlay1ConsumeTimer(void) {
    if (gOverlay1TimerState->timer <= gOverlay1TimerStep) {
        gOverlay1TimerState->active = 0;
    } else {
        gOverlay1TimerState->timer -= gOverlay1TimerStep;
    }
}
