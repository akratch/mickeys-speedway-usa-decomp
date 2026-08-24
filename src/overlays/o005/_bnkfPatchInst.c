#include "overlays/o005/audio_bank.h"

/* Overlay 5 +0x188. Source crosswalk: DKR us.v77 libultra/src/audio/bnkf.c. */
void _bnkfPatchSound(ALSound *sound, s32 offset, s32 table);

void _bnkfPatchInst(ALInstrument *inst, s32 offset, s32 table) {
    s32 i;

    if (inst->flags != 0) {
        return;
    }
    inst->flags = 1;

    for (i = 0; i < inst->soundCount; i++) {
        inst->soundArray[i] = (ALSound *)((u8 *)inst->soundArray[i] + offset);
        _bnkfPatchSound(inst->soundArray[i], offset, table);
    }
}
