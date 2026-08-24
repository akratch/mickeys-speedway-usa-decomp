#include "overlays/overlay_005.h"

/* PROVENANCE: adapted from DKR us.v77, libultra/src/audio/bnkf.c::alSeqFileNew. */
void alSeqFileNew(ALSeqFile *file, u8 *base) {
    s32 offset = (s32)base;
    s32 i;

    for (i = 0; i < file->seqCount; i++) {
        file->seqArray[i].offset =
            (u8 *)((u8 *)file->seqArray[i].offset + offset);
    }
}
