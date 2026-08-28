/*
 * PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * src/audiomgr.c:amAudioMgrSetScheduleMode. Mickey's symbols, linked bytes,
 * and relocations remain authoritative.
 */

#include "PR/ultratypes.h"

extern s32 D_80078DDC;
extern s32 D_80078DE4;

void amAudioMgrSetScheduleMode(s32 mode) {
    mode = (D_80078DDC = mode);
    if (mode == 1) {
        D_80078DE4 = 0;
    }
}
