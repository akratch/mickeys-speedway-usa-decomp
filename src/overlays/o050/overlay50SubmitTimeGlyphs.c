#include "PR/ultratypes.h"

typedef struct Overlay50GlyphRecord {
    s32 sharedWord0;
    s32 sharedWord1;
    s32 packedGlyph;
    s16 x;
    s16 y;
} Overlay50GlyphRecord;

extern s32 gOverlay50GlyphOffsets[9];
extern s32 gOverlay50GlyphWord0;
extern s32 gOverlay50GlyphWord1;
extern u8 gOverlay50GlyphContext[];

extern void overlay50SplitTimeReloc(s32 value, s32 *minutes, s32 *seconds,
                                    s32 *centiseconds);
extern void overlay50SubmitGlyphsReloc(void *context,
                                      Overlay50GlyphRecord *records,
                                      s32 arg2, s32 arg3, s32 red, s32 green,
                                      s32 blue, s32 alpha);

/* Exact DKR v77/v80 and JFG donor scans are negative for this formatter. */
void overlay50SubmitTimeGlyphs(s32 leadingGlyph, s32 x, s32 y,
                               s32 timeValue) {
    s32 i;
    s32 minutes;
    s32 seconds;
    s32 centiseconds;
    Overlay50GlyphRecord records[10];

    overlay50SplitTimeReloc(timeValue, &minutes, &seconds, &centiseconds);

    y -= 4;
    for (i = 0; i < 9; i++) {
        x += gOverlay50GlyphOffsets[i];
        records[i].x = x;
        records[i].y = y;
        records[i].sharedWord0 = gOverlay50GlyphWord0;
        records[i].sharedWord1 = gOverlay50GlyphWord1;
    }

    records[9].sharedWord0 = 0;
    records[9].sharedWord1 = 0;

    records[0].packedGlyph = leadingGlyph << 16;
    records[1].packedGlyph = (minutes / 10) << 16;
    records[2].packedGlyph = (minutes % 10) << 16;
    records[3].packedGlyph = 11 << 16;
    records[4].packedGlyph = (seconds / 10) << 16;
    records[5].packedGlyph = (seconds % 10) << 16;
    records[6].packedGlyph = 10 << 16;
    records[7].packedGlyph = (centiseconds / 10) << 16;
    records[8].packedGlyph = (centiseconds % 10) << 16;

    for (i = 1; i < 7; i++) {
        if ((records[i].packedGlyph >> 16) == 1) {
            if (!(i & 1)) {
                records[i].x++;
            } else {
                records[i].x--;
            }
        }
    }

    overlay50SubmitGlyphsReloc(gOverlay50GlyphContext, records, 0, 0,
                               255, 255, 255, 255);
}
