#include "PR/ultratypes.h"

typedef struct Overlay68Record {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay68Record;

typedef struct Overlay68Entry {
    void *object;
    s8 kind;
    s8 field5;
    s16 generation;
    s16 timer;
    s16 recordCount;
    Overlay68Record *records;
} Overlay68Entry;

extern Overlay68Entry *gOverlay68PrimaryEntry;
extern Overlay68Entry *gOverlay68SecondaryEntry;
extern void *gOverlay68Tertiary;

extern void overlay68ClearNestedFlagPromoteReloc(void *entryOrHandle);
extern void overlay68FinishEntryPromoteReloc(void);

/*
 * Overlay 68 text +0x51C..+0x650. The natural object supplies the exact body,
 * opcodes, calls, data identities, copy loop, and effects. A complete decoded
 * ledger selects retail's private frame, owner precolor, and likely exit.
 */
void overlay68PromoteSecondary(void) {
    Overlay68Entry *primary;
    Overlay68Entry *secondary;
    Overlay68Record *source;
    Overlay68Record *destination;
    s32 remaining;

    secondary = gOverlay68SecondaryEntry;
    if (secondary == NULL) {
        return;
    }
    primary = gOverlay68PrimaryEntry;
    if (primary == NULL) {
        return;
    }

    overlay68ClearNestedFlagPromoteReloc(primary);
    overlay68ClearNestedFlagPromoteReloc(gOverlay68Tertiary);
    if (secondary->field5 != 0) {
        overlay68FinishEntryPromoteReloc();
    }

    if (secondary->timer != 0 && secondary->object == NULL &&
        (secondary->timer < primary->timer ||
         secondary->generation != primary->generation)) {
        primary->kind = secondary->kind;
        primary->generation = secondary->generation;
        primary->timer = secondary->timer;
        primary->recordCount = secondary->recordCount;

        remaining = secondary->recordCount;
        source = secondary->records;
        destination = primary->records;
        while (remaining--) {
            destination->red = source->red;
            destination->green = source->green;
            destination->blue = source->blue;
            destination->x = source->x;
            destination->y = source->y;
            destination->z = source->z;
            destination->alpha = source->alpha;
            source++;
            destination++;
        }
        secondary->timer = 0;
    }
}
