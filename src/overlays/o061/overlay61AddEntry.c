#include "PR/ultratypes.h"

typedef struct Overlay61Entry {
    s16 field0;
    u8 field2;
    u8 field3;
    u8 field4;
    u8 field5;
    u8 field6;
    u8 field7;
    u8 text0[0x30];
    u8 text1[4];
    s32 field3C;
} Overlay61Entry;

extern s32 gOverlay61EntryCountReloc;
extern Overlay61Entry gOverlay61EntriesReloc[];
extern u8 gOverlay61Text0Reloc[];
extern u8 gOverlay61Text1Reloc[];
extern void overlay61CopyTextReloc(u8 *, u8 *, s32);

void overlay61AddEntry(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                       s32 arg5, s32 arg6, s32 arg7) {
    s32 hours;
    s32 minutes;
    s32 hundredths;
    Overlay61Entry *entry;

    if (gOverlay61EntryCountReloc < 20) {
        entry = &gOverlay61EntriesReloc[gOverlay61EntryCountReloc << 0];
        gOverlay61EntryCountReloc++;

        if (arg7 != -1) {
            hours = arg7 / 3600;
            minutes = (arg7 - (hours * 3600)) / 60;
            hundredths = (((arg7 - (hours * 3600)) - (minutes * 60)) * 100) / 60;
        }

        if ((arg5 < 0) || (arg5 >= 11)) {
            arg5 = 0;
        }

        if (arg2 != 0) {
            overlay61CopyTextReloc(entry->text0, gOverlay61Text0Reloc, arg2);
        } else {
            entry->text0[0] = 0;
        }

        if (arg3 != 0) {
            overlay61CopyTextReloc(entry->text1, gOverlay61Text1Reloc, arg3);
        } else {
            entry->text1[0] = 0;
        }

        entry->field0 = arg0;
        entry->field3 = arg5;
        entry->field2 = arg1;
        entry->field4 = arg6;
        entry->field5 = hours;
        entry->field6 = minutes;
        entry->field7 = hundredths;
        entry->field3C = arg4;
    }
}
