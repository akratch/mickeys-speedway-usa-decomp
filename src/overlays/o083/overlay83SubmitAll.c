#include "PR/ultratypes.h"

typedef struct Overlay83Command {
    u32 w0;
    u32 w1;
} Overlay83Command;

#define OVERLAY83_PACK_COLOR(red, green, blue, alpha) \
    (((red) << 24) | ((green) << 16) | ((blue) << 8) | (alpha))

typedef struct Overlay83Batch {
    u8 count;
    u8 pad01[3];
    u8 *records;
} Overlay83Batch;

typedef struct Overlay83Nested {
    u8 pad00[0xAC];
    void **resource;
} Overlay83Nested;

typedef struct Overlay83Context {
    u8 pad00[0x40];
    Overlay83Nested *nested;
    u8 pad44[0x20];
    Overlay83Batch *batch;
    void **setup;
} Overlay83Context;

extern void overlay83SubmitSetupReloc(Overlay83Command **displayList,
                                      void *data, s32 mode, s32 extra);
extern void overlay83DrawMeshReloc(Overlay83Command **displayList,
                                   void *record);
extern void overlay83DispatchLocalReloc(Overlay83Command **displayList,
                                        void *arg1, void *arg2, void *record,
                                        void *resource);

/* DKR v77/v80 and JFG contain no exact donor for this two-pass renderer. */
void overlay83SubmitAll(Overlay83Command **displayList, void *arg1, void *arg2,
                        Overlay83Context *context) {
    s32 count;
    u8 *record;
    Overlay83Batch *batch;

    batch = context->batch;
    overlay83SubmitSetupReloc(displayList, arg1, (s32)arg2, (s32)context);
    overlay83SubmitSetupReloc(displayList, *context->setup, 0x213, 0);

    count = batch->count;
    record = batch->records;
    while (count--) {
        u8 *currentRecord = record;
        record += 0x258;
        overlay83DrawMeshReloc(displayList, currentRecord);
    }

    count = batch->count;
    record = batch->records;
    while (count--) {
        u8 *currentRecord = record;
        record += 0x258;
        overlay83DispatchLocalReloc(displayList, arg1, arg2, currentRecord,
                                    *context->nested->resource);
    }

    {
        Overlay83Command *command;
        u32 color;

        color = OVERLAY83_PACK_COLOR(0xFF, 0xFF, 0xFF, 0xFF);
        command = *displayList;
        *displayList = command + 1;
        command->w1 = 0;
        command->w0 = 0xE7000000;

        command = *displayList;
        *displayList = command + 1;
        command->w1 = color;
        command->w0 = 0xFA000000;

        command = *displayList;
        *displayList = command + 1;
        command->w1 = color;
        command->w0 = 0xFB000000;
    }
}
