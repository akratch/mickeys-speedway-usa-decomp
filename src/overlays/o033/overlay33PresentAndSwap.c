#include "PR/ultratypes.h"

extern volatile s32 gOverlay33BufferIndex;
extern u8 gOverlay33CommandStorage[];
extern void *gOverlay33DisplayLists[];
extern void *gOverlay33BufferRefs[];
extern void *gOverlay33ActiveBuffer;
extern s32 gOverlay33Ready;

extern void overlay33FlushReloc(void);
extern void overlay33SubmitReloc(void *commands, void *displayList, s32 mode,
                                 void *buffer);

/* Exact overlay 33 body at +0x66C. */
void overlay33PresentAndSwap(void) {
    s32 index;

    overlay33FlushReloc();
    index = gOverlay33BufferIndex;
    overlay33SubmitReloc(&gOverlay33CommandStorage[index * 0xC00],
                         gOverlay33DisplayLists[index], 4,
                         gOverlay33BufferRefs[index]);
    index = gOverlay33BufferIndex ^ 1;
    gOverlay33BufferIndex = index;
    gOverlay33ActiveBuffer = gOverlay33BufferRefs[index];
    gOverlay33Ready = 1;
}
