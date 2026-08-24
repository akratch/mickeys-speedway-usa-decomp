#include "PR/ultratypes.h"

typedef struct Overlay20State {
    u8 pad00[0x84];
    s32 handle;
} Overlay20State;

void overlay20ReleaseReloc(void *handle);

/* DKR v77/v80 has the generic optional-release idiom, but no exact donor. */
void overlay20ReleaseHandle(Overlay20State *state) {
    if (state->handle != 0) {
        overlay20ReleaseReloc((void *)state->handle);
    }
}
