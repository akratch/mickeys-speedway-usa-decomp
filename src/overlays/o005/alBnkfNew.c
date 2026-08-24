#include "overlays/overlay_005.h"

/*
 * PROVENANCE: adapted from DKR us.v77,
 * libultra/src/audio/bnkf.c::alBnkfNew.
 */
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
