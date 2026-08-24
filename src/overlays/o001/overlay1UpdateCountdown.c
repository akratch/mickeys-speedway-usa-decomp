#include "PR/ultratypes.h"

typedef struct Overlay1Countdown {
    u8 pad0[4];
    u16 value;
} Overlay1Countdown;

typedef struct Overlay1CountdownObject {
    u8 pad0[0x390];
    Overlay1Countdown countdown;
} Overlay1CountdownObject;

extern Overlay1CountdownObject *gOverlay1CountdownObject;
extern s32 gOverlay1CountdownAmount;
extern void *gOverlay1CountdownResource;
extern void overlay1CountdownReloc(void *, s32);

/* DKR v77/v80 and JFG contain no exact donor for this countdown update. */
void overlay1UpdateCountdown(void) {
    u8 *object;
    s32 amount;
    register s32 mode;
    u16 countdown;

    object = (u8 *)gOverlay1CountdownObject;
    amount = gOverlay1CountdownAmount;
    countdown = *(u16 *)(object + 0x394); mode = 0x78; object += 0x390;
    if ((((u32)object & mode) != 0) && (object == 0)) {
    }
    if (countdown <= amount) {
        overlay1CountdownReloc(gOverlay1CountdownResource, mode);
    } else {
        *(u16 *)(object + 4) = countdown - amount;
    }
}
