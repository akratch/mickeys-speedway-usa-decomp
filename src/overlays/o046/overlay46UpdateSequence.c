#include "PR/ultratypes.h"

typedef struct Overlay46DisplayCommand {
    u32 w0;
    u32 w1;
} Overlay46DisplayCommand;

typedef struct Overlay46Progress {
    u8 pad00[3];
    u8 alpha;
} Overlay46Progress;

typedef struct Overlay46Counter {
    u8 pad00[4];
    s32 value;
} Overlay46Counter;

extern s32 gOverlay46Timer5C;
extern u8 D_60[];
extern u8 D_C0[];
extern u8 D_120[];
extern Overlay46Progress *gOverlay46Group54Render;
extern s32 gOverlay46Mode58;
extern s32 gOverlay46Value4;
extern s32 gOverlay46Value0;
extern s32 gOverlay46Value8;
extern Overlay46Counter *gOverlay46Group54Update;
extern void *gOverlay46Resource50;
extern void *gOverlay46Resource4C;
extern u8 *gOverlay46ExpectedName180;
extern s32 gOverlay46StatusFlags;
extern Overlay46DisplayCommand *gDisplayListHead;
extern Overlay46DisplayCommand *gDisplayListHeadB;
extern u8 gOverlay46MotionTarget[];
extern s32 D_80003634;
extern s32 D_80003638;

extern s32 func_80037664(void);
extern void func_80037414(s32 mode, f32 scale, f32 value, s32 arg3, s32 arg4,
                          s32 arg5, s32 arg6);
extern void func_80037658(void);
extern void func_overlay_099_F0000064_18D9614(s32 mode, void *resource,
                                               f32 scale, f32 value);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern void func_80000F94(s32 soundId, void *handle);
extern void func_overlay_044_F0000294_188BAF4(void *state, s32 updateRate);
extern void *func_80058240(void);
extern void func_800291B4(void);
extern void func_8003A680(s32 value);
extern void func_80001608(void);
extern void func_80028374(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                          s32 arg5);
extern void func_80028528(s32 value);
extern void func_80028D30(s32 value);
extern void func_80036F08(Overlay46DisplayCommand **commands, void *resource,
                          s32 arg2);
extern void func_overlay_044_F0000580_188BDE0(void *state, void *target,
                                              f32 scale);
extern void func_8004B0A4(s32 mode);
extern void func_8004B0DC(s32 red, s32 green, s32 blue, s32 alpha);
extern void func_8004B0B8(s32 red, s32 green, s32 blue, s32 alpha,
                          s32 intensity);
extern s32 func_overlay_046_F000069C_188EA94(void);
extern s32 func_overlay_046_F0000874_188EC6C();

#define O46_SHIFTL(value, shift, width) \
    (((u32)(value) & ((1U << (width)) - 1U)) << (shift))
#define O46_PRIM(packet, red, green, blue, alpha) { \
    Overlay46DisplayCommand *macroCommand = (Overlay46DisplayCommand *)(packet); \
    macroCommand->w0 = O46_SHIFTL(0xFA, 24, 8); \
    macroCommand->w1 = O46_SHIFTL(red, 24, 8) | \
        O46_SHIFTL(green, 16, 8) | O46_SHIFTL(blue, 8, 8) | \
        O46_SHIFTL(alpha, 0, 8); \
}

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
void func_overlay_046_F0000120_188E518(s32 amount) {
    s32 matched;
    s32 value;
    u8 *left;
    u8 *right;
    u8 current;
    u8 expected;
    Overlay46Counter *counter;

    switch (gOverlay46Mode58) {
    case 1:
        if (gOverlay46Timer5C == 0) {
            if (func_80037664() != 1) {
                gOverlay46Value0 = 0;
                gOverlay46Mode58 = 2;
                gOverlay46Timer5C = 100;
                gOverlay46Value4 = 255;
                func_80037414(1, 2.5f, 0.0f, 0, 0, 0, 0);
            }
        } else if (func_80037664() != 1) {
            value = (gOverlay46Timer5C -= amount);
            if (value <= 0) {
                gOverlay46Timer5C = 0;
                func_overlay_099_F0000064_18D9614(2, D_60, 2.5f, -1.0f);
                value = mathRnd(0x203, 0x204);
                func_80000F94(value & 0xFFFF, 0);
            }
        }
        break;
    case 2:
        if (gOverlay46Timer5C == 0) {
            if (func_80037664() != 1) {
                func_80037658();
                gOverlay46Value4 = 0;
                gOverlay46Mode58 = 3;
                gOverlay46Timer5C = 0;
            }
        } else if (func_80037664() != 1) {
            value = (gOverlay46Timer5C -= amount);
            if (value <= 0) {
                gOverlay46Timer5C = 0;
                func_overlay_099_F0000064_18D9614(2, D_C0, 2.5f, -1.0f);
                value = mathRnd(0x203, 0x204);
                func_80000F94(value & 0xFFFF, 0);
            }
        }
        break;
    case 3:
        func_overlay_044_F0000294_188BAF4(gOverlay46Group54Update, amount);
        counter = gOverlay46Group54Update;
        if (counter->value < 0x200) {
            gOverlay46Value8 = counter->value >> 1;
        } else {
            gOverlay46Value8 = 255;
        }
        if (counter->value >= 0x1E01) {
            if (gOverlay46Timer5C == 0) {
                func_overlay_099_F0000064_18D9614(2, D_120, 2.5f, -1.0f);
                value = mathRnd(0x203, 0x204);
                func_80000F94(value & 0xFFFF, 0);
                gOverlay46Timer5C = 1;
            } else if (func_80037664() != 1) {
                gOverlay46Value8 = 0;
                gOverlay46Mode58 = 4;
                func_overlay_046_F000069C_188EA94();
            }
        }
        break;
    case 4:
        if (func_overlay_046_F0000874_188EC6C(amount) == 0) {
            gOverlay46Mode58 = 5;
        }
        break;
    case 5:
        matched = 0;
        if ((gOverlay46StatusFlags << 4) >= 0) {
            right = func_80058240();
            if (right != 0) {
                left = gOverlay46ExpectedName180;
                right += 0x34;
                matched = 1;
                current = *left;
                if (current != 0) {
compare_name:
                    expected = *right;
                    left++;
                    right++;
                    if (current != expected) {
                        matched = 0;
                    } else {
                        current = *left;
                        if (current != 0) {
                            goto compare_name;
                        }
                    }
                }
            }
        }
        if (matched != 0) {
            *(u8 *)&gOverlay46StatusFlags |= 8;
            func_800291B4();
            D_80003634 = 0;
            func_8003A680(10);
            D_80003638 = -1;
            func_80001608();
            func_80028374(0x12, 0, 0, 0xF, 1, 0);
            func_80028528(1);
        } else {
            func_80028D30(0);
        }
        gOverlay46Mode58 = 6;
        break;
    }

    if (gOverlay46Value4 != 0) {
        O46_PRIM(gDisplayListHead++, gOverlay46Value4, gOverlay46Value4,
                 gOverlay46Value4, 0xFF);
        func_80036F08(&gDisplayListHead, gOverlay46Resource50, 0);
    }

    if (gOverlay46Value0 != 0) {
        O46_PRIM(gDisplayListHeadB++, gOverlay46Value0, gOverlay46Value0,
                 gOverlay46Value0, 0xFF);
        func_80036F08(&gDisplayListHeadB, gOverlay46Resource4C, 0);
    }

    value = gOverlay46Value8;
    if (value != 0) {
        gOverlay46Group54Render->alpha = value;
        func_overlay_044_F0000580_188BDE0(gOverlay46Group54Render,
                                          gOverlay46MotionTarget, 1.0f);
        func_8004B0A4(2);
        func_8004B0DC(0, 0, 0, 0);
        func_8004B0B8(255, 255, 255, 255, gOverlay46Value8);
    }
}
