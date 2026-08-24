#include "PR/ultratypes.h"

typedef struct O13Record {
    u8 pad00[6];
    u8 state;
    u8 timer;
    f32 scale;
    f32 x;
    f32 z;
    f32 y;
    f32 floorZ;
    f32 velocityX;
    f32 velocityZ;
    f32 velocityY;
    f32 phase;
    s32 vertexBank;
    u8 vertices[0x50];
} O13Record;

typedef struct O13Command { u32 w0, w1; } O13Command;
typedef struct O13RenderState { u8 pad00[0xF7]; u8 r, g, b; } O13RenderState;

extern void *D_20;
extern u8 *D_24;
extern void *D_28;
extern f32 D_4;
extern u8 D_80000000[];

extern O13RenderState *o13GetRenderState(void);
extern void o13SetColor(u8, u8, u8, u8, u8, u8);
extern void o13DrawRecord(O13Command **, s32, s32, O13Record *, void *, s32, s32);
extern void o13FinishDraw(void);
extern void o13SetupRecord(O13Command **, void *, s32, s32);

#ifdef NON_MATCHING
void overlay13DrawRecord(
    O13Record *record, O13Command **commands, s32 arg2, s32 arg3) {
    O13RenderState *render;
    O13Command *cmd;
    f32 savedScale;

    if (((record->state == 1) && (D_20 != 0)) || record->state == 2) {
        render = o13GetRenderState();
        savedScale = record->scale;
        if (record->state != 2) {
            o13SetColor(0xFF, 0xFF, 0xFF, render->r, render->g, render->b);
            record->scale = savedScale;
            o13DrawRecord(commands, arg2, arg3, record, D_20, 6, 0xFF);
            o13FinishDraw();
        } else {
            o13SetupRecord(commands, D_28, 0xE, 0);
            if (record->timer < 0x20) {
                cmd = *commands;
                *commands = cmd + 1;
                cmd->w0 = 0xFA000000;
                cmd->w1 = ((record->timer * 8) & 0xFF) | 0xFFFFFF00;
            }

            cmd = *commands;
            *commands = cmd + 1;
            cmd->w0 = 0x04000030U |
                ((((((u32)record + record->vertexBank * 0x28 +
                    0x80000030U) & 6) | 0x20) & 0xFF) << 16);
            cmd->w1 = (u32)record + record->vertexBank * 0x28 + 0x80000030U;

            cmd = *commands;
            *commands = cmd + 1;
            cmd->w0 = 0x05110020;
            cmd->w1 = (u32)&D_80000000;

            if (D_24 != 0 && record->phase < (f32)(u32)*D_24) {
                record->scale = savedScale * D_4;
                cmd = *commands;
                *commands = cmd + 1;
                cmd->w0 = 0xFA000000;
                cmd->w1 = ((u32)render->r << 24) |
                          ((u32)render->g << 16) |
                          ((u32)render->b << 8) | 0xA0;
                cmd = *commands;
                *commands = cmd + 1;
                cmd->w0 = 0xFB000000;
                cmd->w1 = 0xFFFFFF00;
                o13DrawRecord(commands, arg2, arg3, record, D_24, 0xE, 0);
            }
        }

        record->scale = savedScale;
        cmd = *commands;
        *commands = cmd + 1;
        cmd->w0 = 0xE7000000;
        cmd->w1 = 0;
        cmd = *commands;
        *commands = cmd + 1;
        cmd->w0 = 0xFB000000;
        cmd->w1 = 0xFFFFFFFF;
        cmd = *commands;
        *commands = cmd + 1;
        cmd->w0 = 0xFA000000;
        cmd->w1 = 0xFFFFFFFF;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o013/overlay13DrawRecord/func_overlay_013_F0000580_186F098.s")
#endif
