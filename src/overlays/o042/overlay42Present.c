#include "PR/ultratypes.h"

/* DKR video.c::fb_swap is a semantic index-toggle relative, not an exact donor. */
extern void overlay42PresentReloc(void *buffer, s32 value, s32 mode, s32 index);
extern void *gOverlay42Buffer0;
extern s32 gOverlay42Buffer1;
extern s32 gOverlay42Buffer2;
extern s32 gOverlay42Active;
extern s32 gOverlay42BufferIndex;

void overlay42Present(void) {
    if (gOverlay42Buffer0 != 0) {
        overlay42PresentReloc(
            gOverlay42Buffer0, gOverlay42Buffer1, 5, gOverlay42Buffer2);
        gOverlay42Active = 1;
        gOverlay42BufferIndex ^= 1;
    }
}
