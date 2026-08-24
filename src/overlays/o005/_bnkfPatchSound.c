#include "overlays/overlay_005.h"

/*
 * PROVENANCE: adapted from DKR us.v77,
 * libultra/src/audio/bnkf.c::_bnkfPatchSound.
 */
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
