#include "overlays/o005/audio_bank.h"

/*
 * Overlay 5 +0x000. Exact DKR us.v77 donor: libultra/src/audio/bnkf.c.
 * Mickey and the donor both require IDO -O3 -mips2 for this 0x40-byte body.
 */
void alSeqFileNew(ALSeqFile *file, u8 *base) {
    s32 offset = (s32)base;
    s32 i;

    for (i = 0; i < file->seqCount; i++) {
        file->seqArray[i].offset =
            (u8 *)((u8 *)file->seqArray[i].offset + offset);
    }
}
