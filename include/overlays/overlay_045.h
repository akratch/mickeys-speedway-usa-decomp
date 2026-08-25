#ifndef OVERLAY_045_H
#define OVERLAY_045_H

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
    /* 0x1F */ s8 unk1F;
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

typedef struct Overlay45Element {
    f32 x;
    f32 y;
    u8 pad08[2];
    s16 unk0A;
    u16 unk0C;
    u16 unk0E;
    s16 unk10;
    s8 unk12;
    s8 unk13;
    f32 scale;
    s16 drawX;
    s16 drawY;
    s8 offsetX;
    s8 offsetY;
    s8 unk1E;
    s8 unk1F;
    u16 unk20;
    s16 unk22;
} Overlay45Element;

typedef struct Overlay45LayoutDefaults {
    u8 pad00[4];
    s16 x;
    s16 y;
    u8 pad08[0x18];
    s16 lineStartX;
} Overlay45LayoutDefaults;

typedef struct Overlay45Metrics {
    u8 pad00;
    u8 lineHeight;
    u8 advance;
    u8 verticalAdvance;
    u8 pad04[0x10];
} Overlay45Metrics;

typedef struct Overlay45Glyph {
    u8 pad00[0xB];
    u8 yOffset;
    u8 pad0C[2];
    u8 advance;
} Overlay45Glyph;

typedef struct Overlay45PairRecord {
    f32 x;
    f32 y;
    u8 pad8[0x1C];
} Overlay45PairRecord;

typedef struct Overlay45PairOwner {
    Overlay45PairRecord *records;
} Overlay45PairOwner;

extern u8 gOverlay45Defaults[];
extern Overlay45ResourceDescriptor *gOverlay45ResourceHead;
extern u8 *gOverlay45ControlStream;
extern u8 gOverlay45MetricsIndex;
extern Overlay45Metrics *gOverlay45Metrics;
extern u8 *gOverlay45AdvanceTables[];
extern u8 gOverlay45TightAdvance;

s32 overlay45StringLengthReloc(const char *text);
void *overlay45AllocReloc(s32 size, s32 tag);
s32 overlay45FreeReloc(void *allocation);
s32 overlay45RandomRangeReloc(s32 minimum, s32 maximum);
s32 overlay45RandomRangeStoredReloc(s32 minimum, s32 maximum);
s32 overlay45FormatReloc(char *dest, const char *format, ...);
void overlay45ConfigureReloc(Overlay45ResourceDescriptor *descriptor,
                             s16 width, s16 height, s32 flags);
void overlay45BuildControlStream(void *source, u8 *stream);
s32 overlay45MeasureStream(u8 *stream, u8 metricsIndex, s32 arg2);
Overlay45Glyph *overlay45GetGlyph(u8 code);

void overlay45ResetState(void);
Overlay45ResourceDescriptor *overlay45CreateDescriptor(
    const char *text, s16 width, s16 height, s32 flags);
void overlay45ReleaseDescriptor(Overlay45ResourceDescriptor *descriptor);
void overlay45ConfigureLayout(Overlay45ResourceDescriptor *descriptor,
                              s32 x, s32 y, s32 flags);
void overlay45ReadPair(Overlay45PairOwner *owner, s32 *x, s32 *y, s32 index);
void func_overlay_045_F000067C_188CAD4(
    Overlay45ResourceDescriptor *descriptor, s32 flags);
void overlay45SetMode(Overlay45ResourceDescriptor *descriptor, s32 mode);
void overlay45SetField22(Overlay45ResourceDescriptor *descriptor, s32 value);
void overlay45SetField20(Overlay45ResourceDescriptor *descriptor, s32 value);

#endif
