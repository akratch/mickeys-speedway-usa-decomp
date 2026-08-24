#include "overlays/overlay_005.h"

/*
 * PROVENANCE: adapted from DKR us.v77,
 * libultra/src/audio/bnkf.c::_bnkfPatchInst.
 */
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
