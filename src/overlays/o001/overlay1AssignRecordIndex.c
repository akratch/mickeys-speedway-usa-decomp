#include "PR/ultratypes.h"

typedef struct O1VariableRecord { s16 type; u8 size; u8 pad03[9]; u16 index; } O1VariableRecord;
typedef struct O1RecordOwner { u8 pad00[0xC]; u16 index; } O1RecordOwner;
extern s32 D_1D8CRead;
extern void overlay1GetVariableRecords(O1VariableRecord **records, s32 *length,
                                       s32 enabled, O1RecordOwner *owner);

void overlay1AssignRecordIndex(s32 unused, O1RecordOwner *owner) {
    volatile s32 private;
    O1VariableRecord *records;
    O1VariableRecord *record;
    s32 length;
    s32 offset;
    s32 next;
    u8 size;

    if (owner->index == 0xFFFF) {
        overlay1GetVariableRecords(&records, &length, 1, owner);
        offset = 0;
        record = records;
        if (length > 0) {
            do {
                if (record->type == 0xCA) {
                    next = record->index + 1;
                    if (D_1D8CRead < next) *(s32 *)0x1D8C = next;
                }
                size = record->size;
                offset += size;
                record = (O1VariableRecord *)((u8 *)record + size);
            } while (offset < length);
        }
        owner->index = (u16)D_1D8CRead;
    }
}
