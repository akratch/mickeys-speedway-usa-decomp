#include "PR/ultratypes.h"

typedef struct Overlay1ModeInput {
    u8 pad00[0xC];
    f32 x;
    u8 pad10[4];
    f32 y;
} Overlay1ModeInput;

typedef struct Overlay1ModeSource {
    u8 pad00[0xE];
    u16 value;
} Overlay1ModeSource;

typedef struct Overlay1ModeState {
    u8 pad00[0x37C];
    s16 value;
    u8 mode;
    u8 previousMode;
    u8 phase;
    u8 pad381[0x17];
    s32 selector;
} Overlay1ModeState;

extern Overlay1ModeState *D_1DA0;
extern Overlay1ModeSource *D_1D64;
extern Overlay1ModeInput *D_1D9C;
extern void func_overlay_001_F0000BD4_184CFB4(void);
extern u8 func_overlay_001_F0000614_184C9F4(
    f32 x, f32 y, Overlay1ModeSource *source, s32 selector);

void overlay1InitializeModeState(s32 value) {
    D_1DA0->value = value;
    D_1DA0->mode = 3;
    func_overlay_001_F0000BD4_184CFB4();
    D_1DA0->pad381[0x31] = (u8)D_1D64->value;
    D_1DA0->mode = func_overlay_001_F0000614_184C9F4(
        D_1D9C->x, D_1D9C->y, D_1D64, D_1DA0->selector);
    D_1DA0->previousMode = D_1DA0->mode;
    D_1DA0->phase = 0;
}
