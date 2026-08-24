#ifndef O045_RESOURCE_DESCRIPTOR_H
#define O045_RESOURCE_DESCRIPTOR_H

#include "ultra64.h"

/* Only fields whose offsets are proved by overlay 45/61 call and data flow. */
typedef struct Overlay45ResourceDescriptor {
    /* 0x00 */ void *elements;
    /* 0x04 */ void *elementEnd;
    /* 0x08 */ s32 flags;
    /* 0x0C */ s32 unk0C;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ s16 width;
    /* 0x1A */ s16 height;
    /* 0x1C */ u8 count;
    /* 0x1D */ u8 mode;
    /* 0x1E */ u8 unk1E;
    /* 0x1F */ u8 pad1F;
    /* 0x20 */ u8 unk20;
    /* 0x21 */ u8 unk21;
    /* 0x22 */ u8 unk22;
    /* 0x23 */ u8 pad23;
    /* 0x24 */ s16 optionalValue;
    /* 0x26 */ u8 pad26[2];
    /* 0x28 */ void *unk28;
    /* 0x2C */ void *allocation;
    /* 0x30 */ struct Overlay45ResourceDescriptor *next;
} Overlay45ResourceDescriptor;

#endif
