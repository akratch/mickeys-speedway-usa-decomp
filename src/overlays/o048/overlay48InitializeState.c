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
 * Plateau (2026-08-25): this run's full 119-combination lattice retains the
 * natural -O2 source at 212/228 bytes; 51/57 instruction words differ and the
 * first mismatch is +0x0.  Its 0x18-byte frame is the right size, but IDO
 * schedules the frame setup at +0x98 instead of the target's +0x1C.  A
 * five-entry combined structure shortened the function by another 16 bytes;
 * separate first-seed aliases, header-tail packing, volatile qualification,
 * and pointer/indexed unrolled loops either moved the prologue to entry or
 * emitted repeated absolute accesses.  MIPS1/MIPS2 and the secondary lattice
 * flags are neutral.  The earlier bounded permuter result remains rejected
 * because its lower cost depended on synthetic conditions and arithmetic.
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
