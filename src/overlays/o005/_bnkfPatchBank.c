#include "overlays/o005/audio_bank.h"

/* Overlay 5 +0x0d0. Source crosswalk: DKR us.v77 libultra/src/audio/bnkf.c. */
void _bnkfPatchInst(ALInstrument *inst, s32 offset, s32 table);

void _bnkfPatchBank(ALBank *bank, s32 offset, s32 table) {
    s32 i;

    if (bank->flags != 0) {
        return;
    }
    bank->flags = 1;

    if (bank->percussion != 0) {
        bank->percussion = (ALInstrument *)((u8 *)bank->percussion + offset);
        _bnkfPatchInst(bank->percussion, offset, table);
    }

    for (i = 0; i < bank->instCount; i++) {
        bank->instArray[i] = (ALInstrument *)((u8 *)bank->instArray[i] + offset);
        if (bank->instArray[i] != 0) {
            _bnkfPatchInst(bank->instArray[i], offset, table);
        }
    }
}
