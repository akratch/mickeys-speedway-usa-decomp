#include "overlays/o045/resource_descriptor.h"

typedef struct Overlay45Element {
    f32 x;
    f32 y;
    u8 pad08[2];
    s16 unk0A;
    s16 unk0C;
    s16 unk0E;
    s16 unk10;
    u8 unk12;
    u8 unk13;
    f32 scale;
    u8 pad18[6];
    u8 unk1E;
    u8 unk1F;
    s16 unk20;
    s16 unk22;
} Overlay45Element;

extern u8 gOverlay45Defaults[];
extern Overlay45ResourceDescriptor *gOverlay45ResourceHead;
s32 overlay45StringLengthReloc(const char *text);
void *overlay45AllocReloc(s32 size, s32 tag);
s32 overlay45RandomRangeReloc(s32 minimum, s32 maximum);
s32 overlay45FormatReloc(char *dest, const char *format, ...);
void overlay45ConfigureReloc(Overlay45ResourceDescriptor *descriptor,
                             s16 width, s16 height, s32 flags);

/*
 * Overlay 45 +0x00C. Fresh DKR v77/v80 searches for the exact
 * strlen/count*0x45+0x35 allocation shape, allocation tag 0x87, and paired
 * 0x140/0xA0 random ranges were negative.
 */
Overlay45ResourceDescriptor *overlay45CreateDescriptor(
    const char *text, s16 width, s16 height, s32 flags) {
    s32 count;
    s32 i;
    Overlay45Element *element;
    Overlay45ResourceDescriptor *descriptor;
    Overlay45ResourceDescriptor *allocation;

    count = overlay45StringLengthReloc(text);
    allocation = overlay45AllocReloc((count << 5) +
                                     (count * sizeof(Overlay45Element)) +
                                     count + 0x35,
                                     0x87);
    if (allocation == NULL) {
        return NULL;
    }
    descriptor = (Overlay45ResourceDescriptor *)((u8 *)allocation + (count << 5));
    descriptor->allocation = allocation;
    descriptor->flags = flags;
    descriptor->unk0C = 0;
    descriptor->width = width;
    descriptor->height = height;
    descriptor->count = count;
    descriptor->elements = (u8 *)descriptor + 0x34;
    descriptor->elementEnd = (u8 *)descriptor->elements +
                             (count * sizeof(Overlay45Element));
    descriptor->unk28 = gOverlay45Defaults;
    descriptor->mode = gOverlay45Defaults[0x1C];
    descriptor->unk10 = (f32)0.0;
    descriptor->unk14 = 0.0f;
    descriptor->unk1E = 0;
    descriptor->unk20 = 0;
    descriptor->unk21 = gOverlay45Defaults[0x1D];
    descriptor->unk22 = 0;
    if (flags & 0x200) {
        descriptor->optionalValue = overlay45RandomRangeReloc(0x78, 0x258);
    }

    element = descriptor->elements;
    i = 0;
    if (count > 0) {
        do {
            element->unk0A = 0;
            element->x = (f32)width;
            element->y = (f32)height;
            element->x = (f32)overlay45RandomRangeReloc(0, 0x140);
            element->y = (f32)overlay45RandomRangeReloc(0, 0xA0);
            element->scale = 1.0f;
            element->unk0C = overlay45RandomRangeReloc(0, 0x8000);
            element->unk0E = overlay45RandomRangeReloc(0x600, 0xA00);
            element->unk10 = overlay45RandomRangeReloc(0x100, 0x300);
            if (overlay45RandomRangeReloc(0, 1) == 0) {
                element->unk10 = -element->unk10;
            }
            i++;
            element->unk13 = 0;
            element->unk1E = 0;
            element->unk1F = 0;
            element->unk20 = 0;
            element->unk22 = 0;
            element++;
        } while (i != count);
    }

    descriptor->next = gOverlay45ResourceHead;
    gOverlay45ResourceHead = descriptor;
    overlay45FormatReloc(descriptor->elementEnd,
                         (char *)gOverlay45Defaults + 0x20, text);
    overlay45ConfigureReloc(descriptor, width, height, flags);
    return descriptor;
}
