#include "PR/ultratypes.h"

typedef struct Overlay71Command {
    u32 w0;
    u32 w1;
} Overlay71Command;

typedef struct Overlay71DrawState {
    u8 pad00[0xCC];
    u8 active;
    u8 vertexBank;
    u16 flags;
} Overlay71DrawState;

typedef struct Overlay71DrawObject {
    u8 pad00[0x64];
    Overlay71DrawState *state;
    s32 *resourceIndex;
} Overlay71DrawObject;

extern void *gOverlay71InitialResourceReloc;
extern u8 D_80000008[];
extern u8 D_80000028[];
extern u8 D_800000D8[];

extern void func_80032BF0(void *resource, s32 mode, s32 flags);
extern void func_8002409C(Overlay71Command **commands, s32 context,
                         Overlay71DrawObject *object, f32 scale, f32 extra);
extern void func_80034554(Overlay71Command **commands, s32 resource, s32 mode,
                         s32 flags);
extern void func_800241BC(Overlay71Command **commands);

/* DKR v77/v80 and JFG contain no exact donor for this renderer. */
#ifdef NON_MATCHING
void func_overlay_071_F0000870_18CA390(Overlay71Command **commands,
                                       s32 context,
                                       Overlay71DrawObject *object) {
    Overlay71Command *command;
    Overlay71DrawState *state;
    /* This unused identity preserves the retail IDO allocation basin. */
    volatile u32 stackShape;

    func_80032BF0(gOverlay71InitialResourceReloc, 2, 2);
    state = object->state;
    if (state->active != 0) {
        func_8002409C(commands, context, object, 1.0f, 0.0f);

        command = *commands;
        *commands = command + 1;
        command->w0 = 0xE7000000;
        command->w1 = 0;
        command = *commands;
        *commands = command + 1;
        command->w0 = 0xFB000000;
        command->w1 = 0xFFFFFFFF;

        if (state->flags & 1) {
            func_80034554(commands, 0, 0x17, 0);
            command = *commands;
            *commands = command + 1;
            command->w0 = 0xFA000000;
            command->w1 = 0xFFFFFFD0;
            command = *commands;
            *commands = command + 1;
            command->w0 =
                ((((((((u32)state + state->vertexBank * 0x50) +
                         0x80000000) & 6) | 0x20) & 0xFF) << 16) |
                 0x04000000) | 0x30;
            command->w1 =
                (u32)state + state->vertexBank * 0x50 + 0x80000000;
            command = *commands;
            *commands = command + 1;
            command->w0 = 0x05100020;
            command->w1 = (u32)D_80000008;
            command = *commands;
            *commands = command + 1;
            command->w0 = 0xE7000000;
            command->w1 = 0;
        }

        if (state->flags & 6) {
            func_80034554(commands, *object->resourceIndex, 0x17, 0);
            command = *commands;
            *commands = command + 1;
            command->w0 = 0xFA000000;
            command->w1 = 0xFFFFFFFF;
            command = *commands;
            *commands = command + 1;
            command->w0 =
                ((((((((u32)state + state->vertexBank * 0x50) +
                         0x80000000) & 6) | 0x40) & 0xFF) << 16) |
                 0x04000000) | 0x58;
            command->w1 =
                (u32)state + state->vertexBank * 0x50 + 0x80000000;
            if (state->flags & 4) {
                command = *commands;
                *commands = command + 1;
                command->w0 = 0x05710080;
                command->w1 = (u32)D_800000D8;
            }
            if (state->flags & 2) {
                command = *commands;
                *commands = command + 1;
                command->w0 = 0x05710080;
                command->w1 = (u32)D_80000028;
            }
            command = *commands;
            *commands = command + 1;
            command->w0 = 0xE7000000;
            command->w1 = 0;
        }
        command = *commands;
        *commands = command + 1;
        command->w0 = 0xFA000000;
        command->w1 = 0xFFFFFFFF;
        func_800241BC(commands);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o071/func_overlay_071_F0000870_18CA390/func_overlay_071_F0000870_18CA390.s")
#endif
