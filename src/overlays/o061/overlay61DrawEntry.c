#include "PR/ultratypes.h"

typedef struct Overlay61Entry {
    s16 kind;
    u8 field2;
    u8 field3;
    s8 field4;
    u8 field5;
    u8 field6;
    u8 field7;
    u8 text0[0x30];
    u8 text1[4];
    s32 field3C;
} Overlay61Entry;

extern u8 gOverlay61Format58Reloc[];
extern u8 gOverlay61Format64Reloc[];
extern u8 gOverlay61Format70Reloc[];
extern u8 gOverlay61Format78Reloc[];
extern u8 gOverlay61Format90Reloc[];
extern u8 gOverlay61Format9CReloc[];
extern u8 gOverlay61FormatA8Reloc[];
extern u8 gOverlay61LabelB0Reloc[];
extern u8 gOverlay61FormatBCReloc[];
extern void *gOverlay61NamesReloc[];

extern void overlay61SetColorReloc(s32, s32, s32, s32, s32);
extern void overlay61DrawBoxReloc(s32, s32, s32, s32, s32, u32);
extern s32 overlay61FormatReloc(u8 *, u8 *, ...);
extern void overlay61DrawTextReloc(s32, s32, s32, void *, s32);
extern void *overlay61LookupReloc(s8);

void overlay61DrawEntry(s32 context, s32 y, Overlay61Entry *entry,
                        s32 selected) {
    u32 color;
    u8 text[128];

    if (entry == 0) {
        overlay61DrawBoxReloc(context, 0x1E, y - 2, 0x62, y + 0x14,
                             0x200000FF);
        overlay61DrawBoxReloc(context, 0x64, y - 2, 0x122, y + 0x14,
                             0x000020FF);
        return;
    }

    if (entry->kind == 5) {
        color = 0x00FF00FF;
    } else if ((entry->kind == 4) || (entry->kind == 6)) {
        color = 0x0000FFFF;
    } else {
        color = 0xFF0000FF;
    }

    if (selected) {
        overlay61SetColorReloc(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        overlay61DrawBoxReloc(context, 0x1E, y - 2, 0x62, y + 0x14,
                             0x800000FF);
        overlay61DrawBoxReloc(context, 0x64, y - 2, 0x122, y + 0x14,
                             color & 0x808080FF);
    } else {
        overlay61SetColorReloc(0xFF, 0xFF, 0, 0xFF, 0xFF);
        overlay61DrawBoxReloc(context, 0x1E, y - 2, 0x62, y + 0x14,
                             0x500000FF);
        overlay61DrawBoxReloc(context, 0x64, y - 2, 0x122, y + 0x14,
                             color & 0x505050FF);
    }

    switch (entry->kind) {
        case 4:
            overlay61FormatReloc(text, gOverlay61Format58Reloc, entry->field2);
            overlay61DrawTextReloc(context, 0x20, y, text, 0);
            /* fall through */
        case 5:
            overlay61FormatReloc(text, gOverlay61Format64Reloc, entry->field3C);
            overlay61DrawTextReloc(context, 0x20, y + 9, text, 0);
            overlay61FormatReloc(text, gOverlay61Format70Reloc,
                                 entry->text0, entry->text1);
            overlay61DrawTextReloc(context, 0x66, y, text, 0);
            overlay61DrawTextReloc(context, 0xD0, y,
                                   gOverlay61NamesReloc[entry->field3], 0);
            overlay61DrawTextReloc(context, 0x66, y + 9,
                                   overlay61LookupReloc(entry->field4), 0);
            overlay61FormatReloc(text, gOverlay61Format78Reloc,
                                 entry->field5, entry->field6, entry->field7);
            overlay61DrawTextReloc(context, 0xD0, y + 9, text, 0);
            return;

        case 6:
            overlay61FormatReloc(text, gOverlay61Format90Reloc, entry->field2);
            overlay61DrawTextReloc(context, 0x20, y, text, 0);
            overlay61FormatReloc(text, gOverlay61Format9CReloc, entry->field3C);
            overlay61DrawTextReloc(context, 0x20, y + 9, text, 0);
            overlay61FormatReloc(text, gOverlay61FormatA8Reloc,
                                 entry->text0, entry->text1);
            overlay61DrawTextReloc(context, 0x66, y + 4, text, 0);
            return;

        default:
            overlay61DrawTextReloc(context, 0x40, y, gOverlay61LabelB0Reloc, 4);
            overlay61FormatReloc(text, gOverlay61FormatBCReloc, entry->field3C);
            overlay61DrawTextReloc(context, 0x40, y + 9, text, 4);
            overlay61DrawTextReloc(context, 0x66, y + 4, entry->text0, 0);
            return;
    }
}
