#include "PR/ultratypes.h"

typedef struct Overlay48Record {
    s32 handle;
    s16 lifetime;
    u8 pad06[2];
    s16 seed;
    u8 active;
    u8 pad0B[5];
} Overlay48Record;

/* Overlay 48's BSS is one six-record owner: a header at +0x0 followed by
 * five runtime entries at +0x10..+0x5F. Historical D_* aliases and the
 * indexed timer/script tail resolve within this block at load time. */
Overlay48Record gOverlay48Entries[6];

extern volatile Overlay48Record D_10[4];
extern s16 D_274[];
extern u8 D_174[];
extern s16 gOverlay48HeaderLifetime;
extern s16 gOverlay48HeaderSeed;
extern u8 gOverlay48HeaderActive;
extern s32 gOverlay48HeaderHandle;
extern s16 gOverlay48Timer;
extern void *gOverlay48Script;
extern s16 gOverlay48ScriptIndex;
extern s16 gOverlay48Finished;
extern void func_overlay_048_F0000000_1895408();

/* Workbench: structure-mismatch (instruction shape), 47/57 masked words differ
 * from +0x0; next lever is structure-buckets. Ownership: this TU emits the
 * measured BSS +0x0..+0x60; initializer remains NON_MATCHING. */
#ifdef NON_MATCHING
void overlay48InitializeState(void) {
    register volatile s16 *initial;
    register s32 index;

    gOverlay48HeaderSeed = D_274[0];
    gOverlay48HeaderLifetime = 0;
    gOverlay48HeaderActive = 0;
    gOverlay48HeaderHandle = 0;

    index = 1;
    initial = &D_274[index];
    D_10[0].lifetime = 0;
    D_10[0].active = 0;
    D_10[1].active = 0;
    D_10[1].lifetime = 0;
    D_10[0].handle = 0;
    D_10[0].seed = initial[0];
    D_10[2].active = 0;
    D_10[2].lifetime = 0;
    D_10[1].handle = 0;
    D_10[1].seed = initial[1];
    D_10[3].active = 0;
    D_10[3].lifetime = 0;
    D_10[2].handle = 0;
    D_10[2].seed = initial[2];
    D_10[3].handle = 0;
    D_10[3].seed = initial[3];

    gOverlay48Timer = 0;
    gOverlay48ScriptIndex = 0;
    gOverlay48Finished = 0;
    gOverlay48Script = D_174;
    func_overlay_048_F0000000_1895408(0x16);
    func_overlay_048_F0000000_1895408();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o048/overlay48InitializeState/func_overlay_048_F0000060_1895468.s")
#endif
