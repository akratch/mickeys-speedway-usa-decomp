/*
 * Fast3D/RCP task and clear helpers -- ROM 0x2F400-0x30CD0.
 *
 * PROVENANCE -- the TU identity and descriptive names are adapted from Jet
 * Force Gemini's public decompilation, src/rcpFast3d.c. Mickey's exact
 * rcpFast3d/rcpInit skeleton anchors, ordered init helpers and RCP call graph
 * establish the boundary. Adapted C bodies are identified in docs/modules.md;
 * all remaining functions stay as Mickey GLOBAL_ASM.
 */

#include "PR/ultratypes.h"
#include "PR/os_message.h"
#include "game/gameVi.h"
#include "game/sched_internal.h"

typedef struct RcpCommand {
    u32 w0;
    u32 w1;
} RcpCommand;

#define RCP_DISPLAY_LIST(command, list) \
    { \
        RcpCommand *cmd = (command); \
        cmd->w0 = 0x06000000; \
        cmd->w1 = (u32) (list); \
    }

#define RCP_PIPE_SYNC(command) \
    { \
        RcpCommand *cmd = (command); \
        cmd->w0 = 0xE7000000; \
        cmd->w1 = 0; \
    }

#define RCP_SET_COLOR_IMAGE(command, width, address) \
    { \
        RcpCommand *cmd = (command); \
        cmd->w0 = 0xFF100000 | (((width) - 1) & 0xFFF); \
        cmd->w1 = (address); \
    }

#define RCP_SET_DEPTH_IMAGE(command, address) \
    { \
        RcpCommand *cmd = (command); \
        cmd->w0 = 0xFE000000; \
        cmd->w1 = (address); \
    }

extern u8 D_8007A3A0;
extern u8 D_8007A3A4;
extern u8 D_8007A3A8;
extern u32 D_8007A3B0;
extern u32 D_8007A3AC;
extern s32 D_8007A3C4;
extern s32 D_8007A3C8;
extern s32 D_8007A3EC;
extern s32 D_8007A410;
extern RcpCommand D_8007A438[];
extern RcpCommand D_8007A4B8[];
extern OSMesgQueue D_800D2880;
extern OSMesg D_800D2898;
extern OSMesgQueue D_800D28A0;
extern OSMesgQueue D_800D28B8;
extern OSMesg D_800D28D0[];
extern OSMesg D_800D28F0[];
extern OSMesgQueue *D_800D2C90;
extern OSMesgQueue D_800D2C98;
extern OSMesg D_800D2CB0[];
extern OSMesgQueue D_800D2CD0;
extern OSMesg D_800D2CE8[];
extern OSMesgQueue D_800D2D08;
extern OSMesg D_800D2D20[];
extern s32 D_800D2FAC;

OSMesgQueue *osScGetInterruptQ(OSSched *scheduler);
void TrapDanglingJump(void);

#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpFast3d.s")
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/rcpFast3d.c:rcpWaitDP. */
s32 rcpWaitDP(void) {
    OSMesg message = NULL;
    OSMesg refractDoneMessage = NULL;
    OSMesg blurDoneMessage = NULL;

    if (D_8007A3C4 == 0) {
        return 0;
    }
    osRecvMesg(&D_800D28B8, &message, OS_MESG_BLOCK);
    if (D_8007A410 != 0) {
        osRecvMesg(&D_800D2D08, &blurDoneMessage, OS_MESG_BLOCK);
        D_8007A410 = 0;
    }
    if (D_8007A3EC != 0) {
        osRecvMesg(&D_800D2CD0, &refractDoneMessage, OS_MESG_BLOCK);
        D_8007A3EC = 0;
    }
    if (D_8007A3C8 != 0) {
        TrapDanglingJump();
        D_8007A3C8 = 0;
    }
    D_8007A3C4 = 0;
    return ((s32 *) message)[1];
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/rcpFast3d.c:rcpSetScreenColour. */
void rcpSetScreenColour(u8 red, u8 green, u8 blue) {
    D_8007A3A0 = red;
    D_8007A3A4 = green;
    D_8007A3A8 = blue;
}
/* PROVENANCE: body and name adapted from Diddy Kong Racing's public decomp, src/rcp_dkr.c:bgdraw_fillcolour. */
void bgdraw_fillcolour(s32 red, s32 green, s32 blue) {
    D_8007A3AC = ((red << 8) & 0xF800) | ((green << 3) & 0x7C0) | ((blue >> 2) & 0x3E) | 1;
    D_8007A3AC |= D_8007A3AC << 16;
}
void func_8002EBD4(u32 value) {
    D_8007A3B0 = value;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/func_8002EBE0.s")
#ifdef NON_MATCHING
/* PROVENANCE: command sequence adapted from Diddy Kong Racing's public
 * decomp, src/rcp_dkr.c:bgdraw_render, with Mickey's enable flag and bounds. */
void rcpClearZBuffer(RcpCommand **arg0, u32 arg1, u32 arg2, s32 arg3,
                     s32 arg4, s32 arg5, s32 arg6) {
    RcpCommand *dlist;

    if ((D_800D2FAC != 0) && (arg3 < arg5) && (arg4 < arg6)) {
        dlist = *arg0;
        dlist->w0 = 0xE7000000;
        dlist->w1 = 0;
        dlist++;
        dlist->w0 = 0xED000000;
        dlist->w1 = ((((s32) ((f32) (arg1 - 1) * 4.0f) & 0xFFF) << 12) |
                     ((s32) ((f32) (arg2 - 1) * 4.0f) & 0xFFF));
        dlist++;
        dlist->w0 = 0xEF30000F;
        dlist->w1 = 0;
        dlist++;
        dlist->w0 = 0xFF100000 | ((arg1 - 1) & 0xFFF);
        dlist->w1 = 0x02000000;
        dlist++;
        dlist->w0 = 0xF7000000;
        dlist->w1 = 0xFFFCFFFC;
        dlist++;
        dlist->w0 = 0xF6000000 |
                    ((((arg5 + 3) & ~3) & 0x3FF) << 14) |
                    ((arg6 & 0x3FF) << 2);
        dlist->w1 = (((arg3 & ~3) & 0x3FF) << 14) |
                    ((arg4 & 0x3FF) << 2);
        dlist++;
        dlist->w0 = 0xE7000000;
        dlist->w1 = 0;
        dlist++;
        dlist->w0 = 0xFF100000 | ((arg1 - 1) & 0xFFF);
        dlist->w1 = 0x01000000;
        dlist++;
        *arg0 = dlist;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpClearZBuffer.s")
#endif
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpClearScreen.s")
void rcpInitDp(RcpCommand **dlist) {
    s32 width;
    s32 height;

    viGetCurrentSize(&width, &height);
    RCP_PIPE_SYNC((*dlist)++);
    RCP_SET_COLOR_IMAGE((*dlist)++, width, 0x01000000);
    RCP_SET_DEPTH_IMAGE((*dlist)++, 0x02000000);
    RCP_DISPLAY_LIST((*dlist)++, D_8007A438);
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/rcpFast3d.c:rcpInitDpNoSize. */
void rcpInitDpNoSize(RcpCommand **dlist) {
    RCP_DISPLAY_LIST((*dlist)++, D_8007A438);
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/rcpFast3d.c:rcpInitSp. */
void rcpInitSp(RcpCommand **dlist) {
    RCP_DISPLAY_LIST((*dlist)++, D_8007A4B8);
}
/* Mickey-derived reconstruction; JFG supplies the name, prototype, and exact
 * object-skeleton anchor, while its public src/rcpFast3d.c retains assembly. */
void rcpInit(OSSched *scheduler) {
    D_800D2C90 = osScGetInterruptQ(scheduler);
    osCreateMesgQueue(&D_800D2880, &D_800D2898, 1);
    osCreateMesgQueue(&D_800D28A0, D_800D28D0, 8);
    osCreateMesgQueue(&D_800D28B8, D_800D28F0, 8);
    osCreateMesgQueue(&D_800D2C98, D_800D2CB0, 8);
    osCreateMesgQueue(&D_800D2CD0, D_800D2CE8, 8);
    osCreateMesgQueue(&D_800D2D08, D_800D2D20, 8);
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/func_8002F618.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/func_8002FB34.s")
