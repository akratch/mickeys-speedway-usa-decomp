#include "PR/ultratypes.h"

/* Audio-player construction; DKR has no exact body-level match. */
typedef struct Overlay5PlayerConfig {
    s32 arg0;
    s32 arg1;
    u8 channels;
    u8 pad9[3];
    void *heap;
    void *initQueue;
    void *frameCallback;
    void *eventQueue;
    u8 voiceCount;
} Overlay5PlayerConfig;

typedef struct Overlay5AudioState {
    u8 pad0[4];
    void *sequenceBank;
} Overlay5AudioState;

extern u8 gOverlay5AudioHeap[];
extern u8 gOverlay5InitQueue[];
extern u8 gOverlay5EventQueue[];
extern u8 gOverlay5FrameCallback[];
extern Overlay5AudioState *gOverlay5AudioState;

void *overlay5AllocPlayerReloc(s32 arg0, s32 arg1, void *heap, s32 arg3,
                               s32 size);
void overlay5InitPlayerReloc(void *player, Overlay5PlayerConfig *config);
void overlay5AttachBankReloc(void *player, void *bank);

void *overlay5CreatePlayer(s32 arg0, s32 arg1) {
    void *player;
    Overlay5PlayerConfig config;

    config.arg0 = arg0;
    config.arg1 = arg1;
    config.voiceCount = arg0;
    config.channels = 0x10;
    config.heap = gOverlay5AudioHeap;
    config.initQueue = gOverlay5InitQueue;
    config.eventQueue = gOverlay5EventQueue;
    config.frameCallback = gOverlay5FrameCallback;
    player = overlay5AllocPlayerReloc(0, 0, gOverlay5AudioHeap, 1, 0x90);
    overlay5InitPlayerReloc(player, &config);
    overlay5AttachBankReloc(player, gOverlay5AudioState->sequenceBank);
    return player;
}
