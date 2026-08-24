#include "PR/ultratypes.h"

typedef struct Overlay91Gfx {
    u32 w0;
    u32 w1;
} Overlay91Gfx;

typedef struct Overlay91RenderObject {
    u8 pad00[6];
    s16 flags;
    u8 pad08[0x31];
    u8 alpha;
} Overlay91RenderObject;

extern s32 overlay91CanRenderReloc(void);
extern void overlay91GetDimensionsReloc(void **bufferOut, u32 *widthOut);
extern void overlay91DrawBandReloc(Overlay91Gfx **displayList, void *buffer,
                                   u32 width, void *optional, s32 left,
                                   void *bufferAgain, s32 right);
extern void overlay91BeginRenderReloc(Overlay91Gfx **displayList,
                                      void *renderContext);
extern void overlay91EndRenderReloc(Overlay91Gfx **displayList);
extern void overlay91RenderObjectReloc(Overlay91Gfx **displayList,
                                       void *renderContext, u32 renderArg,
                                       Overlay91RenderObject *object);

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
void overlay91Render(Overlay91Gfx **displayList, void *renderContext,
                     u32 renderArg, Overlay91RenderObject *object) {
    void *buffer;
    u32 width;
    u32 center;

    if (overlay91CanRenderReloc() == 0) {
        object->alpha = 0xFF;
        overlay91GetDimensionsReloc(&buffer, &width);
        center = width >> 1;
        overlay91DrawBandReloc(displayList, buffer, width, 0, center - 20,
                               buffer, center + 20);
        overlay91BeginRenderReloc(displayList, renderContext);
        overlay91EndRenderReloc(displayList);
        object->flags &= ~0x0400;
        overlay91RenderObjectReloc(displayList, renderContext, renderArg,
                                   object);
        object->flags |= 0x0400;
    }
}
