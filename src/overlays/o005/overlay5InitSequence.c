#include "PR/ultratypes.h"

/* JFG o25 contains this unique 56-byte routine; DKR v77/v80 are negative. */
extern void overlay5SequenceInitReloc();

typedef struct Overlay5SequenceHeader {
    s16 count;
    u8 pad2[2];
    s32 value;
    u8 pad8[8];
} Overlay5SequenceHeader;

void overlay5InitSequence(void *owner, s32 value) {
    Overlay5SequenceHeader header;

    header.count = 14;
    header.value = value;
    overlay5SequenceInitReloc((u8 *)owner + 0x48, &header, 0);
}
