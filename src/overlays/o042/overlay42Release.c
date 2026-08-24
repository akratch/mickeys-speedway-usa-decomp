#include "PR/ultratypes.h"

/* DKR thread3_main has the semantic double-display-list teardown, not an exact donor. */
extern void overlay42FreeReloc(void *allocation);
extern void *gOverlay42Buffers[2];
extern s32 gOverlay42State0;
extern s32 gOverlay42State1;
extern s32 gOverlay42Ready;
extern s32 gOverlay42Active;

void overlay42Release(void) {
    if (gOverlay42Buffers[0] != 0) {
        overlay42FreeReloc(gOverlay42Buffers[0]);
    }
    if (gOverlay42Buffers[1] != 0) {
        overlay42FreeReloc(gOverlay42Buffers[1]);
    }
    gOverlay42Buffers[0] = 0;
    gOverlay42Buffers[1] = 0;
    gOverlay42State0 = 0;
    gOverlay42State1 = 0;
    gOverlay42Ready = 0;
    gOverlay42Active = 0;
}
