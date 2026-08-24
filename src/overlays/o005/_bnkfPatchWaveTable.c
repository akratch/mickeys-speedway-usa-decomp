#include "overlays/overlay_005.h"

/*
 * PROVENANCE: adapted from DKR us.v77,
 * libultra/src/audio/bnkf.c::_bnkfPatchWaveTable.
 */
void _bnkfPatchWaveTable(ALWaveTable *wave, s32 offset, s32 table) {
    if (wave->flags != 0) {
        return;
    }
    wave->flags = 1;
    wave->base += table;

    if (wave->type == AL_ADPCM_WAVE) {
        wave->waveInfo.adpcmWave.book =
            (ALADPCMBook *)((u8 *)wave->waveInfo.adpcmWave.book + offset);
        if (wave->waveInfo.adpcmWave.loop != 0) {
            wave->waveInfo.adpcmWave.loop =
                (ALADPCMloop *)((u8 *)wave->waveInfo.adpcmWave.loop + offset);
        }
    } else if (wave->type == AL_RAW16_WAVE) {
        if (wave->waveInfo.rawWave.loop != 0) {
            wave->waveInfo.rawWave.loop =
                (ALRawLoop *)((u8 *)wave->waveInfo.rawWave.loop + offset);
        }
    }
}
