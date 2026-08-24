#include "overlays/o045/resource_descriptor.h"

typedef struct Overlay45Element067C {
    u8 pad00[0x1C];
    s8 offsetX;
    s8 offsetY;
    u8 pad1E[6];
} Overlay45Element067C;

extern s32 overlay45RandomRangeReloc(s32 minimum, s32 maximum);

void func_overlay_045_F000067C_188CAD4(
    Overlay45ResourceDescriptor *descriptor, s32 flags) {
    Overlay45Element067C *element;
    s32 i;

    element = descriptor->elements;
    descriptor->flags |= flags;
    if (flags & 0x20) {
        descriptor->flags &= ~0x40;
        descriptor->flags |= 0x80;
        i = 0;
        if (descriptor->count > 0) {
            do {
                element->offsetX = overlay45RandomRangeReloc(-5, -1);
                if (overlay45RandomRangeReloc(0, 1) == 1) {
                    element->offsetX = -element->offsetX;
                }
                element->offsetY = overlay45RandomRangeReloc(-5, -1);
                if (overlay45RandomRangeReloc(0, 1) == 1) {
                    element->offsetY = -element->offsetY;
                }
                i++;
                element++;
            } while (i < descriptor->count);
        }
    }
}
