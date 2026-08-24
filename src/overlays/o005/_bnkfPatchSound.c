#include "overlays/o005/audio_bank.h"

/* Overlay 5 +0x220. Source crosswalk: DKR us.v77 libultra/src/audio/bnkf.c. */
void _bnkfPatchWaveTable(ALWaveTable *wave, s32 offset, s32 table);

void _bnkfPatchSound(ALSound *sound, s32 offset, s32 table) {
    if (sound->flags != 0) {
        return;
    }
    sound->flags = 1;

    sound->envelope = (ALEnvelope *)((u8 *)sound->envelope + offset);
    sound->keyMap = (ALKeyMap *)((u8 *)sound->keyMap + offset);
    sound->wavetable = (ALWaveTable *)((u8 *)sound->wavetable + offset);
    _bnkfPatchWaveTable(sound->wavetable, offset, table);
}
