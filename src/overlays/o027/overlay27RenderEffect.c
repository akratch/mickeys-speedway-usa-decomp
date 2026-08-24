/*
 * Mickey-local reconstruction. Pinned DKR v77/v80 and JFG donor scans are
 * exact-negative; DKR billboard rendering was used only as structural
 * vocabulary. See the O27 +0624 GO packet for runtime-role provenance.
 */
#include "PR/ultratypes.h"

typedef struct O27Command {
    u32 w0;
    u32 w1;
} O27Command;

typedef struct O27Transform {
    s16 x;
    s16 y;
    s16 z;
    s16 pad06;
    f32 scale;
    f32 positionX;
    f32 positionY;
    f32 positionZ;
} O27Transform;

typedef struct O27Work {
    u8 scratchBefore[0xC];
    O27Transform transform;
    u8 scratchAfter[0xC];
} O27Work;

typedef struct O27Child {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x38];
    f32 *factor;
} O27Child;

typedef struct O27State {
    u8 pad00[0xA];
    s16 alpha;
    s16 drawPrimary;
    s16 drawSecondary;
    f32 y;
    u8 pad14[0xC];
    O27Child *child;
} O27State;

typedef struct O27Resource {
    u8 pad00[0xAC];
    void **displayList;
} O27Resource;

typedef struct O27Object {
    u8 pad00[8];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x21];
    s8 alpha;
    u8 pad3A[6];
    O27Resource *resource;
    u8 pad44[0x20];
    O27State *state;
} O27Object;

extern u8 D_80000000[];
extern u8 D_80000050[];
extern u8 D_80000118[];
extern u8 D_80000160[];

extern s16 *overlay27GetValue(void);
extern f32 overlay27GetChildScale(O27Child *child);
extern void overlay27Prepare(O27Command **commands, void *arg1,
                             O27Transform *transform, f32 arg3, f32 arg4);
extern void overlay27DrawPart(O27Command **commands, void *displayList,
                              s32 arg2, s32 arg3);
extern void overlay27SetMode(O27Command **commands, s32 arg1, s32 arg2,
                             s32 arg3);
extern void overlay27Finish(O27Command **commands, void *arg1);
extern void overlay27Finalize(O27Command **commands, void *arg1, s16 *arg2,
                              O27Object *object);

void func_overlay_027_F0000624_187BFFC(O27Command **commands, void *arg1,
                                       s16 *arg2, O27Object *object) {
    O27Work work;
    O27Command *command;
    O27State *state;
    O27Child *child;
    s16 *value;
    void *displayList;
    void *finishArg;
    f32 oldScale;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    s32 intensity;

    value = overlay27GetValue();
    state = object->state;
    child = state->child;
    if (child != 0) {
        scale = overlay27GetChildScale(child);
    } else {
        scale = 1.0f;
    }

    if (state->drawPrimary != 0) {
        if (child != 0) {
            work.transform.positionX = child->x;
            work.transform.positionY = child->y;
            work.transform.positionZ = child->z;
        } else {
            work.transform.positionX = object->x;
            work.transform.positionY = object->y;
            work.transform.positionZ = object->z;
        }

        work.transform.x = -*value;
        work.transform.y = 0;
        work.transform.z = 0;
        work.transform.scale = scale;
        intensity = 0x100;
        work.transform.positionY += 24.0f * scale;

        if (child != 0 && child->factor != 0) {
            intensity = (s32)(*child->factor * 256.0f);
        }

        displayList = *object->resource->displayList;
        overlay27Prepare(commands, arg1, &work.transform, 1.0f, 0.0f);
        overlay27DrawPart(commands, displayList, 0x214, 0);

        command = *commands;
        *commands = command + 1;
        command->w0 = 0xFA000000;
        command->w1 = (((intensity * 0x60) >> 8) << 24) |
                      ((((intensity * 0xE0) >> 8) & 0xFF) << 16) |
                      ((((intensity * 0xFF) >> 8) & 0xFF) << 8) |
                      (state->drawPrimary & 0xFF);

        command = *commands;
        *commands = command + 1;
        command->w0 = 0xFB000000;
        command->w1 = ((((intensity << 7) >> 8) & 0xFF) << 8) | 0xFF;

        command = *commands;
        *commands = command + 1;
        command->w0 = (((((u32)D_80000000 & 6) | 0x40) & 0xFF) << 16) |
                      0x04000058;
        command->w1 = (u32)D_80000000;

        command = *commands;
        *commands = command + 1;
        command->w0 = 0x059100A0;
        command->w1 = (u32)D_80000050;

        command = *commands;
        *commands = command + 1;
        command->w1 = 0;
        command->w0 = 0xE7000000;

        finishArg = 0;
        if (state->drawSecondary != 0) {
            overlay27SetMode(commands, 0, 5, 0);

            command = *commands;
            *commands = command + 1;
            command->w0 = 0xFA000000;
            command->w1 = (state->drawSecondary & 0xFF) | 0xFFFF0000;
            finishArg = D_80000118;

            command = *commands;
            *commands = command + 1;
            command->w0 = (((((u32)D_80000118 & 6) | 0x38) & 0xFF) << 16) |
                          0x0400004E;
            command->w1 = (u32)D_80000118;

            command = *commands;
            *commands = command + 1;
            command->w0 = 0x05400050;
            command->w1 = (u32)D_80000160;

            command = *commands;
            *commands = command + 1;
            command->w1 = 0;
            command->w0 = 0xE7000000;
        }

        command = *commands;
        *commands = command + 1;
        command->w1 = 0xFFFFFFFF;
        command->w0 = 0xFA000000;

        command = *commands;
        *commands = command + 1;
        command->w1 = 0xFFFFFF00;
        command->w0 = 0xFB000000;

        overlay27Finish(commands, finishArg);
    }

    oldScale = object->scale;
    if (child != 0) {
        object->y = child->y + (state->y * scale);
    }
    object->scale *= scale;
    object->alpha = state->alpha;
    overlay27Finalize(commands, arg1, arg2, object);
    object->scale = oldScale;
}
