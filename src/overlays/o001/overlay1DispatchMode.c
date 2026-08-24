#include "PR/ultratypes.h"

typedef struct Overlay1ModeState {
    u8 pad00;
    s8 index;
    u8 pad02[0x198];
    u8 mode;
    u8 timer;
    u8 pad19C[4];
    s32 task;
    u8 pad1A4[0x1DA];
    u8 group;
    u8 pad37F[0x1D];
    f32 angle;
    u8 pad3A0[8];
    u8 status[1];
} Overlay1ModeState;

typedef struct Overlay1ModeObject {
    u8 pad00[0x64];
    Overlay1ModeState *state;
} Overlay1ModeObject;

#ifndef WORLD_GLOBAL_DECL
#define WORLD_GLOBAL_DECL extern Overlay1ModeState *D_1DA0_array[];
#define WORLD D_1DA0_array[0]
#endif
#ifndef CASE_END
#define CASE_END return 0
#endif
WORLD_GLOBAL_DECL
extern void *D_1D9C;
extern u8 D_6C[];
extern void overlay1ModeAction2(void *object, s32 arg);
extern s32 overlay1ModeRandom(s32 minimum, s32 maximum);
extern Overlay1ModeObject *overlay1ModeFind(f32 angle);
extern void overlay1ModeAction3(void *object);
extern void overlay1ModeAction4(void *object);
extern void overlay1ModeAction5(void *object);
extern void overlay1ModeAction6(void *object);
extern void overlay1ModeAction7(void *object);
extern void overlay1ModeAction8(void *object);
extern void overlay1ModeAction9(void *object);
extern f32 overlay1WrapOffset(f32 first, f32 second);

#ifdef NON_MATCHING
s32 overlay1DispatchMode(void) {
    Overlay1ModeState *world;
    Overlay1ModeObject *object;
    Overlay1ModeState *state;
    f32 difference;

    world = WORLD;
    switch (world->mode) {
        case 2:
            overlay1ModeAction2(D_1D9C, 1);
            WORLD->timer--;
            world = WORLD;
            if (world->timer == 0) {
                world->mode = 0xFF;
                WORLD->task = 0;
            }
            CASE_END;
        case 3:
            if (D_6C[WORLD->index] < overlay1ModeRandom(1, 100)) {
                object = overlay1ModeFind(WORLD->angle);
                if (object != 0) {
                    state = object->state;
                    if (WORLD->status[state->index] >= 3) {
                        overlay1ModeAction3(D_1D9C);
                    }
                }
            }
            CASE_END;
        case 4:
            if (D_6C[WORLD->index] < overlay1ModeRandom(1, 100)) {
                object = overlay1ModeFind(WORLD->angle);
                if (object != 0) {
                    state = object->state;
                    if (WORLD->status[state->index] >= 3) {
                        difference = overlay1WrapOffset(WORLD->angle,
                                                        object->state->angle);
                        if ((0.5f <= difference) && (difference <= 4.0f)) {
                            overlay1ModeAction4(D_1D9C);
                        }
                    }
                }
            }
            CASE_END;
        case 5:
            if (D_6C[WORLD->index] < overlay1ModeRandom(1, 100)) {
                object = overlay1ModeFind(WORLD->angle);
                if (object != 0) {
                    state = object->state;
                    if (WORLD->status[state->index] >= 3) {
                        overlay1ModeAction5(D_1D9C);
                    }
                }
            }
            CASE_END;
        case 6:
            object = overlay1ModeFind(WORLD->angle);
            if (object != 0) {
                state = object->state;
                difference = overlay1WrapOffset(WORLD->angle, state->angle);
                if ((difference <= 3.0f) &&
                    (WORLD->status[state->index] >= 3) &&
                    (state->group == WORLD->group)) {
                    overlay1ModeAction6(D_1D9C);
                }
            }
            CASE_END;
        case 7:
            overlay1ModeAction7(D_1D9C);
            CASE_END;
        case 8:
            overlay1ModeAction8(D_1D9C);
            CASE_END;
        case 9:
            overlay1ModeAction9(D_1D9C);
            CASE_END;
    }
    return 0;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1DispatchMode/func_overlay_001_F0005ED4_18522B4.s")
#endif
