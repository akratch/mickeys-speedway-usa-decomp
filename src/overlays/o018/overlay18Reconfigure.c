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
extern u8 *gOverlay18CurrentBuffer;
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

#ifdef NON_MATCHING
void overlay18Reconfigure(s32 mode) {
    s32 index;
    const s32 *scale10;
    const s32 *scale16;
    const s32 *scale64;
    const s32 *scale8;
    s32 value8;
    s32 value64;
    s32 value16;
    s32 size;
    void *token;
    u8 *buffer0;
    u8 *buffer1;
    s32 selected;
    Overlay18Command *command;

    if (mode != gOverlay18Mode) {
        index = mode;
        gOverlay18Mode = mode;
        token = overlay18BeginReconfigureReloc(mode);
        overlay18SelectReloc(0);

        scale16 = &gOverlay18ModeScale16[index];
        scale8 = &gOverlay18ModeScale8[index];
        scale64 = &gOverlay18ModeScale64[index];
        scale10 = &gOverlay18ModeScale10[index];
        size = (*scale16 * 16) + (*scale8 * 8) +
               (*scale10 * 10) + (*scale64 * 64);

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

        buffer0 = gOverlay18Buffers[0];
        buffer1 = gOverlay18Buffers[1];
        value8 = *scale8;
        value64 = *scale64;
        value16 = *scale16;
        gOverlay18Region8Slot0 = buffer0 + (value8 * 8);
        gOverlay18Region64Slot0 = gOverlay18Region8[0] + (value64 * 64);
        gOverlay18Region16Slot0 = gOverlay18Region64[0] + (value16 * 16);
        gOverlay18Region8Slot1[1] = buffer1 + (value8 * 8);
        gOverlay18Region64Slot1[1] = gOverlay18Region8[1] + (value64 * 64);
        gOverlay18Region16Slot1[1] = gOverlay18Region64[1] + (value16 * 16);

        gOverlay18ActiveScale8 = value8;
        gOverlay18ActiveScale64 = value64;
        gOverlay18ActiveScale16 = value16;
        gOverlay18ActiveScale10 = *scale10;
        overlay18FinishReconfigureReloc(token);
    }

    selected = gOverlay18SelectedBuffer;
    gOverlay18CurrentBuffer = gOverlay18Buffers[selected];
    gOverlay18CurrentRegion8 = gOverlay18Region8[selected];
    gOverlay18CurrentRegion64 = gOverlay18Region64[selected];
    gOverlay18CurrentRegion16 = gOverlay18Region16[selected];

    command = (Overlay18Command *)gOverlay18CurrentBuffer;
    gOverlay18CurrentBuffer += sizeof(*command);
    command->w1 = 0;
    command->w0 = 0xE9000000;
    command = (Overlay18Command *)gOverlay18CurrentBuffer;
    gOverlay18CurrentBuffer += sizeof(*command);
    command->w1 = 0;
    command->w0 = 0xB8000000;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o018/overlay18Reconfigure/func_overlay_018_F000024C_1874804.s")
#endif
