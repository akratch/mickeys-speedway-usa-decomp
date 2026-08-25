#include "PR/ultratypes.h"

typedef struct Overlay48Record {
    s32 handle;
    s16 lifetime;
    u8 pad06[2];
    s16 seed;
    u8 active;
    u8 pad0B[5];
} Overlay48Record;

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

/* DKR v77/v80 and JFG contain no exact donor for this initializer. */
/*
 * Plateau (2026-08-25, lane cx-ov-6-b-a-r4): a fresh 119-combination lattice
 * and ten typed header/seed/record variants improved the best masked result to
 * 47/57 differing words at 212/228 bytes; under -O2 -mips2 the first mismatch
 * is +0x0.  Direct seed loads reduce the register-color mismatch, but IDO then
 * folds the literal tail index and omits four address-formation words.  Sparse
 * header views, same-address seed aliases, and local BSS definitions either
 * materialize an extra base or move the frame setup.  The remaining blocker
 * is preserving the target's distinct first-seed and indexed-tail bases in
 * the 0x18-byte frame without spilling the index.
 */
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
