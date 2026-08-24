#include "overlays/o005/audio_bank.h"

/*
 * Overlay 5 +0x040. Source crosswalk: DKR us.v77 libultra/src/audio/bnkf.c.
 * Kept as a separate TU because Mickey preserves the helper call while DKR's
 * whole-file -O3 build inlines the same helper graph.
 */
void _bnkfPatchBank(ALBank *bank, s32 offset, s32 table);

void alBnkfNew(ALBankFile *file, u8 *table) {
    s32 offset = (s32)file;
    s32 tableOffset = (s32)table;
    s32 i;

    if (file->revision != AL_BANK_VERSION) {
        return;
    }

    for (i = 0; i < file->bankCount; i++) {
        file->bankArray[i] = (ALBank *)((u8 *)file->bankArray[i] + offset);
        if (file->bankArray[i] != 0) {
            _bnkfPatchBank(file->bankArray[i], offset, tableOffset);
        }
    }
}
