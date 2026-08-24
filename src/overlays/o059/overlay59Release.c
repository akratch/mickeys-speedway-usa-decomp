#include "ultra64.h"

typedef struct {
    u8 pad0[0x20];
    void *owner;
    void *handle;
} Overlay59State;

extern void overlay59ReleaseReloc(void *handle);

/* DKR v77/v80 and JFG contain only generic fixed-handle cleanup relatives. */
void overlay59Release(Overlay59State *state) {
    s32 offset;
    Overlay59State *cursor;

    cursor = state;
    offset = 0;
    if (state->owner != NULL) {
        do {
            if (cursor->handle != NULL) {
                overlay59ReleaseReloc(cursor->handle);
                cursor->handle = NULL;
            }
            offset += 4;
            cursor = (Overlay59State *) ((u8 *) cursor + 4);
        } while (offset != 0x20);
        state->owner = NULL;
    }
}
