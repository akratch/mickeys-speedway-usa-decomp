#include "PR/ultratypes.h"

typedef struct Overlay61RecordHeader {
    u8 pad0[0xA];
    s16 count;
} Overlay61RecordHeader;

s32 overlay61RecordSize(Overlay61RecordHeader *record) {
    return (record->count * 10) + 0x10;
}
