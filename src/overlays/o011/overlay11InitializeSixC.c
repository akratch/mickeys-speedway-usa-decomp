#include "PR/ultratypes.h"

typedef struct {
    u8 pad000[0x50];
    void *resource50;
    u8 pad054[0x40];
    void *resource94;
    void *resource98;
    u8 pad09C[0x100];
    void *resource19C;
    u8 pad1A0[8];
    void *resource1A8;
    u8 pad1AC[0xD8];
    void *resource284;
} Overlay11Resources;

extern Overlay11Resources *gOverlay11Resources;
extern void *gOverlay11Created[6];
extern s32 gOverlay11CreatedActive;
extern void *overlay11CreateReloc(void *resource, s32 x, s32 y, s32 mode);

/* DKR v77/v80 and JFG have only generic fixed-resource creation idioms. */
void overlay11InitializeSixC(void) {
    gOverlay11Created[0] = overlay11CreateReloc(gOverlay11Resources->resource284, 160, 40, 4);
    gOverlay11Created[1] = overlay11CreateReloc(gOverlay11Resources->resource19C, 160, 80, 4);
    gOverlay11Created[2] = overlay11CreateReloc(gOverlay11Resources->resource1A8, 160, 110, 4);
    gOverlay11Created[3] = overlay11CreateReloc(gOverlay11Resources->resource94, 160, 140, 4);
    gOverlay11Created[4] = overlay11CreateReloc(gOverlay11Resources->resource98, 160, 170, 4);
    gOverlay11Created[5] = overlay11CreateReloc(gOverlay11Resources->resource50, 160, 200, 4);
    gOverlay11CreatedActive = 1;
}
