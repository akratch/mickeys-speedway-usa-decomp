#include "overlays/overlay_045.h"

/*
 * Overlay 45, ADR 0006 consolidation: the leading matched-C range is one
 * translation unit in ROM order. The matched setters after the hand-written
 * middle remain in overlay_045_tail.c because a single object cannot span
 * that separately owned asm range.
 */

/* Pinned DKR v77/v80 and JFG scans contain no exact donor. */
void overlay45ResetState(void) {
    gOverlay45ResourceHead = NULL;
}

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

/*
 * Overlay 45 +0x270. Fresh DKR v77/v80 searches for linked-list removal via
 * offsets +0x30/+0x2C, and for analogous descriptor release loops, were
 * negative.
 */
void overlay45ReleaseDescriptor(Overlay45ResourceDescriptor *descriptor) {
    Overlay45ResourceDescriptor *current;
    Overlay45ResourceDescriptor *previous;
    Overlay45ResourceDescriptor *target;

    current = gOverlay45ResourceHead;
    target = descriptor;
    previous = NULL;
    if (current != NULL && descriptor != NULL) {
        do {
            if (current == target) {
                if (previous == NULL) {
                    gOverlay45ResourceHead = target->next;
                } else {
                    previous->next = target->next;
                }
                overlay45FreeReloc(target->allocation);
                target = NULL;
            } else {
                previous = current;
                current = current->next;
            }
        } while (current != NULL && target != NULL);
    }
}

/*
 * Overlay 45 text +0x0314..+0x0640.
 *
 * The two signed-halfword formals are widened immediately. Keeping x/y as
 * s16 work variables would introduce repeated truncation absent from the
 * target. The matched +0x00C caller already declares this physical ABI.
 */
void overlay45ConfigureLayout(
    Overlay45ResourceDescriptor *descriptor, s32 x, s32 y, s32 flags) {
    Overlay45Element *element;
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

/* DKR v77/v80 and JFG checks found no exact donor for this coordinate accessor. */
void overlay45ReadPair(Overlay45PairOwner *owner, s32 *x, s32 *y, s32 index) {
    Overlay45PairRecord *record = &owner->records[index];

    *x = record->x;
    *y = record->y;
}

void func_overlay_045_F000067C_188CAD4(
    Overlay45ResourceDescriptor *descriptor, s32 flags) {
    Overlay45Element *element;
    s32 i;

    element = descriptor->elements;
    descriptor->flags |= flags;
    if (flags & 0x20) {
        descriptor->flags &= ~0x40;
        descriptor->flags |= 0x80;
        i = 0;
        if (descriptor->count > 0) {
            do {
                element->offsetX = overlay45RandomRangeStoredReloc(-5, -1);
                if (overlay45RandomRangeStoredReloc(0, 1) == 1) {
                    element->offsetX = -element->offsetX;
                }
                element->offsetY = overlay45RandomRangeStoredReloc(-5, -1);
                if (overlay45RandomRangeStoredReloc(0, 1) == 1) {
                    element->offsetY = -element->offsetY;
                }
                i++;
                element++;
            } while (i < descriptor->count);
        }
    }
}
