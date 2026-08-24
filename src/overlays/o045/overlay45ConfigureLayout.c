#include "overlays/o045/resource_descriptor.h"

/* Packet-local partial layouts. Only the accessed offsets are named. */
typedef struct Overlay45LayoutDefaults {
    u8 pad00[4];
    s16 x;
    s16 y;
    u8 pad08[0x18];
    s16 lineStartX;
} Overlay45LayoutDefaults;

typedef struct Overlay45LayoutElement {
    f32 x;
    f32 y;
    u8 pad08[0x10];
    s16 drawX;
    s16 drawY;
    u8 pad1C[8];
} Overlay45LayoutElement;

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

/* Packet-local semantic names for five globals used by seven address pairs. */
extern u8 *gOverlay45ControlStream;
extern u8 gOverlay45MetricsIndex;
extern Overlay45Metrics *gOverlay45Metrics;
extern u8 *gOverlay45AdvanceTables[];
extern u8 gOverlay45TightAdvance;

/* Packet-local semantic names for the four resident callees (five sites). */
extern void overlay45BuildControlStream(void *source, u8 *stream);
extern s32 overlay45MeasureStream(u8 *stream, u8 metricsIndex, s32 arg2);
extern Overlay45Glyph *overlay45GetGlyph(u8 code);
extern s32 overlay45RandomRangeReloc(s32 minimum, s32 maximum);

/*
 * Overlay 45 text +0x0314..+0x0640.
 *
 * The two signed-halfword formals are widened immediately. Keeping x/y as
 * s16 work variables would introduce repeated truncation absent from the
 * target. The matched +0x00C caller already declares this physical ABI.
 */
void overlay45ConfigureLayout(
    Overlay45ResourceDescriptor *descriptor, s32 x, s32 y, s32 flags) {
    Overlay45LayoutElement *element;
    Overlay45LayoutDefaults *defaults;
    Overlay45Metrics *metrics;
    Overlay45Glyph *glyph;
    u8 *stream;
    u8 code;
    s32 advance;
    s32 drawX;
    s32 drawY;
    s32 tabWidth;
    s32 randomize;
    s32 unused;

    overlay45BuildControlStream(descriptor->elementEnd,
                                gOverlay45ControlStream);
    stream = gOverlay45ControlStream;
    defaults = descriptor->unk28;
    gOverlay45MetricsIndex = descriptor->unk21;
    metrics = &gOverlay45Metrics[gOverlay45MetricsIndex];

    if (flags & 5) {
        s32 width;

        width = overlay45MeasureStream(stream, descriptor->unk21, 0);
        if (flags & 1) {
            x = x - width + 1;
        } else {
            x -= width >> 1;
        }
    }
    if (flags & 2) {
        y = y - metrics->lineHeight + 1;
    }
    if (flags & 8) {
        y -= metrics->lineHeight >> 1;
    }

    randomize = flags & 0x10;
    element = descriptor->elements;
    while (*stream != 0) {
        code = *stream++;
        if (code & 0x80) {
            code = *stream++;
            if (code == 0 || code == 0xF) {
                advance = metrics->advance;
            } else {
                drawX = defaults->x + x;
                drawY = defaults->y + y;
                advance = gOverlay45AdvanceTables[gOverlay45MetricsIndex][code];
                glyph = overlay45GetGlyph(code);
                if (glyph != NULL) {
                    advance = glyph->advance;
                    element->drawX = drawX;
                    element->drawY = drawY + glyph->yOffset;
                    descriptor->flags |= 0x40;
                    descriptor->flags &= ~0x80;
                }
            }
            if (gOverlay45TightAdvance != 0 && advance != 0) {
                advance--;
            }
            x += advance;
        } else {
            switch (code) {
            case 10:
                x = defaults->lineStartX;
                y += metrics->verticalAdvance;
                break;
            case 9:
                tabWidth = metrics->advance << 2;
                x = x + tabWidth -
                    ((x - defaults->lineStartX) % tabWidth);
                break;
            case 11:
                y += metrics->verticalAdvance;
                break;
            case 13:
                x = defaults->lineStartX;
                break;
            default:
                x += metrics->advance;
                break;
            }
        }

        if (randomize) {
            element->x = (f32)overlay45RandomRangeReloc(-0x20, 0x160);
            element->y = (f32)overlay45RandomRangeReloc(-0x20, 0xC0);
        } else if (!(flags & 0x100)) {
            element->x = (f32)element->drawX;
            element->y = (f32)element->drawY;
        }
        element++;
    }
}
