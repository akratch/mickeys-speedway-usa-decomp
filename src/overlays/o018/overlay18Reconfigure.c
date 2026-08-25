#include "PR/ultratypes.h"

typedef struct Overlay18Command {
    u32 w0;
    u32 w1;
} Overlay18Command;

extern const s32 gOverlay18ModeScale8[];
extern const s32 gOverlay18ModeScale10[];
extern const s32 gOverlay18ModeScale64[];
extern const s32 gOverlay18ModeScale16[];

extern s32 gOverlay18Mode;
extern s32 gOverlay18SelectedBuffer;
extern u8 *gOverlay18Buffers[2];
extern u8 *gOverlay18Region8[2];
extern u8 *gOverlay18Region64[2];
extern u8 *gOverlay18Region16[2];
extern u8 *gOverlay18Region8Slot0;
extern u8 *gOverlay18Region8Slot1[];
extern u8 *gOverlay18Region64Slot0;
extern u8 *gOverlay18Region64Slot1[];
extern u8 *gOverlay18Region16Slot0;
extern u8 *gOverlay18Region16Slot1[];
extern Overlay18Command *gOverlay18CurrentBuffer;
extern u8 *gOverlay18CurrentRegion8;
extern u8 *gOverlay18CurrentRegion64;
extern u8 *gOverlay18CurrentRegion16;
extern s32 gOverlay18ActiveScale8;
extern s32 gOverlay18ActiveScale10;
extern s32 gOverlay18ActiveScale64;
extern s32 gOverlay18ActiveScale16;

extern void *overlay18BeginReconfigureReloc(s32 mode);
extern void overlay18SelectReloc(s32 selector);
extern void overlay18ReleaseForResizeReloc(void *buffer);
extern void *overlay18ResizeReloc(s32 size, void *oldBuffer, s32 tag);
extern void overlay18ReleaseFailedReloc(void *buffer);
extern void overlay18FinishReconfigureReloc(void *token);
extern void overlay18InitializeBuffers(void);

/*
 * PROVENANCE: Diddy Kong Racing, src/thread3_main.c
 * (alloc_displaylist_heap); adapted allocation and cursor-update structure.
 */
void overlay18Reconfigure(s32 mode) {
    s32 index;
    s32 size;
    void *token;

    if (mode != gOverlay18Mode) {
        index = mode;
        gOverlay18Mode = mode;
        token = overlay18BeginReconfigureReloc(mode);
        overlay18SelectReloc(0);

        size = (gOverlay18ModeScale8[index] * 8) +
               (gOverlay18ModeScale64[index] * 64) +
               (gOverlay18ModeScale10[index] * 10) +
               (gOverlay18ModeScale16[index] * 16);

        overlay18ReleaseForResizeReloc(gOverlay18Buffers[0]);
        overlay18ReleaseForResizeReloc(gOverlay18Buffers[1]);
        gOverlay18Buffers[0] =
            overlay18ResizeReloc(size, gOverlay18Buffers[0], 0x87);
        gOverlay18Buffers[1] =
            overlay18ResizeReloc(size, gOverlay18Buffers[1], 0x87);

        if ((gOverlay18Buffers[0] == 0) ||
            (gOverlay18Buffers[1] == 0)) {
            if (gOverlay18Buffers[0] != 0) {
                overlay18ReleaseFailedReloc(gOverlay18Buffers[0]);
                gOverlay18Buffers[0] = 0;
            }
            if (gOverlay18Buffers[1] != 0) {
                overlay18ReleaseFailedReloc(gOverlay18Buffers[1]);
                gOverlay18Buffers[1] = 0;
            }
            overlay18InitializeBuffers();
        }

        gOverlay18Region8Slot0 = gOverlay18Buffers[0] +
                                (gOverlay18ModeScale8[index] * 8);
        gOverlay18Region64Slot0 = gOverlay18Region8[0] +
                                 (gOverlay18ModeScale64[index] * 64);
        gOverlay18Region16Slot0 = gOverlay18Region64[0] +
                                 (gOverlay18ModeScale16[index] * 16);
        gOverlay18Region8Slot1[1] = gOverlay18Buffers[1] +
                                    (gOverlay18ModeScale8[index] * 8);
        gOverlay18Region64Slot1[1] = gOverlay18Region8[1] +
                                     (gOverlay18ModeScale64[index] * 64);
        gOverlay18Region16Slot1[1] = gOverlay18Region64[1] +
                                     (gOverlay18ModeScale16[index] * 16);

        gOverlay18ActiveScale8 = gOverlay18ModeScale8[index];
        gOverlay18ActiveScale64 = gOverlay18ModeScale64[index];
        gOverlay18ActiveScale16 = gOverlay18ModeScale16[index];
        gOverlay18ActiveScale10 = gOverlay18ModeScale10[index];
        overlay18FinishReconfigureReloc(token);
    }

    gOverlay18CurrentBuffer =
        (Overlay18Command *)gOverlay18Buffers[gOverlay18SelectedBuffer];
    gOverlay18CurrentRegion8 = gOverlay18Region8[gOverlay18SelectedBuffer];
    gOverlay18CurrentRegion64 = gOverlay18Region64[gOverlay18SelectedBuffer];
    gOverlay18CurrentRegion16 = gOverlay18Region16[gOverlay18SelectedBuffer];

    {
        Overlay18Command *command;

        command = gOverlay18CurrentBuffer++;
        command->w1 = 0;
        command->w0 = 0xE9000000;
    }
    {
        Overlay18Command *command;

        command = gOverlay18CurrentBuffer++;
        command->w1 = 0;
        command->w0 = 0xB8000000;
    }
}
