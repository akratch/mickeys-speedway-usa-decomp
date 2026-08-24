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
#ifdef NON_MATCHING
void overlay48InitializeState(void) {
    volatile s16 *initial;
    s32 index;

    gOverlay48HeaderLifetime = 0;
    gOverlay48HeaderActive = 0;
    gOverlay48HeaderSeed = gOverlay48InitialSeeds[0];
    initial = gOverlay48InitialSeeds;
    index = ((u32)initial > 0);
    initial = &initial[index];
    gOverlay48HeaderHandle = 0;
    gOverlay48Records[0].lifetime = 0;
    gOverlay48Records[0].active = 0;
    gOverlay48Records[1].active = 0;
    gOverlay48Records[1].lifetime = 0;
    gOverlay48Records[0].handle = 0;
    gOverlay48Records[0].seed = initial[0];
    gOverlay48Records[2].active = 0;
    gOverlay48Records[2].lifetime = 0;
    gOverlay48Records[1].handle = 0;
    gOverlay48Records[1].seed = initial[1];
    gOverlay48Records[3].active = 0;
    gOverlay48Records[3].lifetime = 0;
    gOverlay48Records[2].handle = 0;
    gOverlay48Records[2].seed = initial[2];
    gOverlay48Records[3].handle = 0;
    gOverlay48Records[3].seed = initial[3];
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
