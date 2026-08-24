#include "PR/ultratypes.h"

typedef struct Overlay61RecordHeader {
    u32 magic;
    u8 pad4[4];
    s8 field8;
    u8 pad9;
    s16 fieldA;
    s16 fieldC;
} Overlay61RecordHeader;

extern u8 gOverlay61RecordPathReloc[];
extern s32 overlay61StorageReadyReloc(void);
extern s32 overlay61OpenReloc(s32 arg0, void *path, s32 arg2, void **handle);
extern s32 overlay61GetSizeReloc(s32 arg0, void *handle, s32 *size);
extern void *overlay61AllocReloc(s32 size, s32 tag);
extern s32 overlay61ReadReloc(s32 arg0, void *handle, void *data, s32 size);
extern void overlay61FreeReloc(void *data);
extern void overlay61CloseReloc(s32 arg0);

s32 overlay61ReadCharacter(
    s32 arg0, s32 arg1, s32 *out0, s32 *out1, s32 *out2) {
    void *handle;
    s32 size;
    Overlay61RecordHeader *buffer;
    s32 result;

    result = overlay61StorageReadyReloc();
    if (result == 0) {
        result = overlay61OpenReloc(
            arg0, gOverlay61RecordPathReloc, arg1, &handle);
        if (result == 0) {
            result = overlay61GetSizeReloc(arg0, handle, &size);
            if (size >= 0x40) {
                buffer = overlay61AllocReloc(0x40, 0x85);
                if (buffer != 0) {
                    result = overlay61ReadReloc(arg0, handle, buffer, 0x40);
                    if (result == 0) {
                        if (buffer->magic == 0x43484152) {
                            *out0 = buffer->field8;
                            *out1 = buffer->fieldA;
                            *out2 = buffer->fieldC;
                        } else {
                            result = 5;
                        }
                    }
                    overlay61FreeReloc(buffer);
                }
            } else {
                result = 6;
            }
        }
    }
    overlay61CloseReloc(arg0);
    return result;
}
