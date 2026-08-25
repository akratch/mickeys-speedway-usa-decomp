#include "PR/ultratypes.h"

typedef struct Overlay48Record {
    s32 handle;
    s16 lifetime;
    u8 pad06[2];
    s16 seed;
    u8 active;
    u8 pad0B[5];
} Overlay48Record;

extern s16 gOverlay48HeaderLifetime;
extern s16 gOverlay48HeaderSeed;
extern u8 gOverlay48HeaderActive;
extern s32 gOverlay48HeaderHandle;
extern volatile Overlay48Record gOverlay48Records[4];
extern s16 gOverlay48InitialSeeds[];
extern s16 gOverlay48Timer;
extern void *gOverlay48Script;
extern s16 gOverlay48ScriptIndex;
extern s16 gOverlay48Finished;
extern u8 gOverlay48ScriptData[];
extern void overlay48SetupMode(s32 mode);
extern void overlay48FinishSetup(void);

/* DKR v77/v80 and JFG contain no exact donor for this initializer. */
/*
 * Plateau (2026-08-25): the retained 57-word candidate has exact size and a
 * 73.07% object score; its first mismatch is +0x0.  Explicit seed temporaries
 * recover the target's unrolled load/store cadence, but the overlay-data alias
 * and prologue schedule remain different.  MIPS1 and MIPS2 are identical for
 * this candidate and secondary lattice flags were neutral.  A bounded
 * ten-minute permuter run reduced cost 1340 to 1075 only through synthetic
 * conditions, canceling arithmetic, and a widened temporary, so its result was
 * rejected.
 */
#ifdef NON_MATCHING
void overlay48InitializeState(void) {
    volatile s16 *initial;
    s16 seed;
    s16 seed1;
    s16 seed2;
    s16 seed3;
    s16 seed4;

    seed = gOverlay48InitialSeeds[0];
    gOverlay48HeaderLifetime = 0;
    gOverlay48HeaderActive = 0;
    gOverlay48HeaderSeed = seed;
    initial = gOverlay48InitialSeeds;
    initial++;
    gOverlay48HeaderHandle = 0;
    gOverlay48Records[0].lifetime = 0;
    gOverlay48Records[0].active = 0;
    seed1 = initial[0];
    gOverlay48Records[1].active = 0;
    gOverlay48Records[1].lifetime = 0;
    gOverlay48Records[0].handle = 0;
    gOverlay48Records[0].seed = seed1;
    seed2 = initial[1];
    gOverlay48Records[2].active = 0;
    gOverlay48Records[2].lifetime = 0;
    gOverlay48Records[1].handle = 0;
    gOverlay48Records[1].seed = seed2;
    seed3 = initial[2];
    gOverlay48Records[3].active = 0;
    gOverlay48Records[3].lifetime = 0;
    gOverlay48Records[2].handle = 0;
    gOverlay48Records[2].seed = seed3;
    seed4 = initial[3];
    gOverlay48Records[3].handle = 0;
    gOverlay48Records[3].seed = seed4;
    gOverlay48Timer = 0;
    gOverlay48ScriptIndex = 0;
    gOverlay48Finished = 0;
    gOverlay48Script = gOverlay48ScriptData;
    overlay48SetupMode(0x16);
    overlay48FinishSetup();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o048/overlay48InitializeState/func_overlay_048_F0000060_1895468.s")
#endif
