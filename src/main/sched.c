/*
 * Game scheduler -- ROM 0x30CD0-0x323A0.
 *
 * PROVENANCE -- the TU identity and descriptive names are adapted from Jet
 * Force Gemini's public decompilation, src/sched.c. Mickey has the same
 * ordered 21-function scheduler call graph, including the three pre-existing
 * accessor/string anchors. Adapted C bodies are identified in docs/modules.md;
 * all remaining functions stay as Mickey GLOBAL_ASM.
 */

#include "PR/ultratypes.h"
#include "PR/os.h"
#include "PR/os_internal.h"
#include "game/sched_internal.h"

typedef struct SchedGfx {
    u32 w0;
    u32 w1;
} SchedGfx;

#define OS_IM_NONE 1

extern s32 D_8007A640;
extern s32 D_8007A648;
extern s32 D_8007A650;
extern s32 D_8007A654;
extern s8 D_8007A658;
extern u64 D_8007A660;
extern char D_80082350[];
extern char D_80082354[];
extern char D_80082368[];
extern char D_8008237C[];
extern char D_80082390[];
extern char D_800823A4[];
extern char D_800823B8[];
extern char D_800823CC[];
extern u8 D_800CF520;
extern u8 D_800CF578;
extern u8 D_800CF590;
extern u8 D_800CF5A8;
extern s32 D_800D2D40;
extern s32 D_800D2D44;
extern u64 D_800D2D48;
extern u8 *D_800D2FA0;
extern u8 *D_800D2FA8;
extern u8 *D_800D2FAC;

void osViSetEvent(OSMesgQueue *mq, OSMesg msg, u32 retraceCount);
void osWritebackDCacheAll(void);
void osSpTaskLoad(OSTask *task);
void osSpTaskStartGo(OSTask *task);
void osSpTaskYield(void);
u64 osGetTime(void);
OSIntMask osSetIntMask(OSIntMask mask);
void *osViGetCurrentFramebuffer(void);
void *osViGetNextFramebuffer(void);
void rsp_segment(SchedGfx **dlist, s32 segment, void *base);
s32 diPrintf(const char *format, ...);
void diPrintfAll(SchedGfx **dlist);
void diPrintfSetXY(u16 x, u16 y);
char *osScGetTaskType(s32 taskID);
void func_800304E0(OSSched *sc);
void func_80030608(OSScTask *task);
SchedGfx *func_80030910(OSSched *sc, s32 *arg1, s32 *arg2, s32 *arg3,
                        s32 *arg4, s32 *arg5, s32 *arg6);
void __scAppendList(OSSched *sc, OSScTask *task);
s32 __scTaskComplete(OSSched *sc, OSScTask *task);
s32 __scSchedule(OSSched *sc, OSScTask **sp, OSScTask **dp, s32 state);
void __scExec(OSSched *sc, OSScTask *sp, OSScTask *dp);
s8 func_80001BE8(void);

#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/osCreateScheduler.s")
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:osScAddClient. */
void osScAddClient(OSSched *sc, OSScClient *client, OSMesgQueue *msgQ, u8 id) {
    OSIntMask mask;

    mask = osSetIntMask(OS_IM_NONE);
    client->msgQ = msgQ;
    client->next = sc->clientList;
    client->id = id;
    sc->clientList = client;
    osSetIntMask(mask);
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:osScRemoveClient. */
void osScRemoveClient(OSSched *sc, OSScClient *clientToRemove) {
    OSScClient *client = sc->clientList;
    OSScClient *previous = NULL;
    OSIntMask mask;

    mask = osSetIntMask(OS_IM_NONE);
    while (client != NULL) {
        if (client == clientToRemove) {
            if (previous != NULL) {
                previous->next = clientToRemove->next;
            } else {
                sc->clientList = clientToRemove->next;
            }
            break;
        }
        previous = client;
        client = client->next;
    }
    osSetIntMask(mask);
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/sched.c:osScGetCmdQ. */
OSMesgQueue *osScGetCmdQ(OSSched *scheduler) {
    return &scheduler->cmdQ;
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/sched.c:osScGetInterruptQ. */
OSMesgQueue *osScGetInterruptQ(OSSched *scheduler) {
    return &scheduler->interruptQ;
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/sched.c:osScGetAudioSPStats. */
void osScGetAudioSPStats(f32 *first, f32 *second, f32 *third) {
    *first = 0.0f;
    *second = 0.0f;
    *third = 0.0f;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/__scMain.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/func_800304E0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/osScGetTaskType.s")
void func_80030608(OSScTask *arg0) {
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/func_80030610.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/func_80030910.s")
#ifdef NON_MATCHING
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp, src/sched.c:__scHandleRetrace. */
void __scHandleRetrace(OSSched *sc) {
    OSScTask *rspTask = NULL;
    OSScClient *client;
    s32 state;
    OSScTask *sp = NULL;
    OSScTask *dp = NULL;
    u8 clearRSPTask = FALSE;
    struct {
        u8 pad[2];
        union {
            u8 normal;
            volatile u8 write;
        } value;
    } clearRDPTask;
    SchedGfx *spGfx;
    SchedGfx *dpGfx;
    s32 spC4;
    s32 spC0;
    s32 spBC;
    OSScTask *spB8;
    s32 spB4;
    s32 spB0;
    s32 spAC;
    s32 spA8;
    s32 spA4;
    s32 spA0;
    s32 sp9C;
    s32 sp98;
    OSScTask *unkTask;
    OSMesg intBuf[8];
    OSMesgQueue interruptQ;
    OSScTask *curTask;
    SchedGfx *dlist;
    s32 yPos;
    s32 pad;
    u16 diagnosticY;

    clearRDPTask.value.normal = FALSE;
    if (sc->curRSPTask != NULL) {
        D_8007A650++;
    }
    if (sc->curRDPTask != NULL) {
        D_8007A654++;
    }

    spGfx = NULL;
    dpGfx = NULL;
    if ((D_8007A650 > 10) && (sc->curRSPTask != NULL)) {
        if (D_800D2D40 != 0) {
            osScGetTaskType(sc->curRSPTask->taskID);
            func_80030608(sc->curRSPTask);
            if (sc->curRSPTask->list.t.type == 1) {
                spGfx = func_80030910(sc, &spB4, &spA4, &spC4, &spB0, &spA0, &spC0);
            }
            D_800D2D40 = 0;
        }
        D_8007A650 = 0;
        clearRSPTask = TRUE;
        __osSpSetStatus(0xAAAA82);
    } else if (sc->curRSPTask != NULL) {
        D_800D2D40 = 1;
    }

    if ((D_8007A654 > 10) && (sc->curRDPTask != NULL)) {
        if (sc->curRDPTask->unk68 == 0) {
            osSendMesg(sc->curRDPTask->msgQ, &D_8007A648, OS_MESG_BLOCK);
        }
        if (D_800D2D44 != 0) {
            osScGetTaskType(sc->curRDPTask->taskID);
            func_80030608(sc->curRDPTask);
            if (sc->curRDPTask->list.t.type == 1) {
                dpGfx = func_80030910(sc, &spAC, &sp9C, &spBC, &spA8, &sp98, (s32 *) &spB8);
            }
            D_800D2D44 = 0;
        }
        clearRDPTask.value.normal = TRUE;
        sc->frameCount = 0;
        D_8007A654 = 0;
        __osSpSetStatus(0xAAAA82);
        osDpSetStatus(0x1D6);
    } else if (sc->curRDPTask != NULL) {
        D_800D2D44 = 1;
    }

    if ((spGfx != NULL) || (dpGfx != NULL)) {
        osCreateMesgQueue(&interruptQ, intBuf, 8);
        osSetEventMesg(4, &interruptQ, (OSMesg) 667);
        osSetEventMesg(9, &interruptQ, (OSMesg) 668);
        osViSetEvent(&interruptQ, (OSMesg) 666, 1);
        if (sc->curRSPTask != NULL) {
            curTask = sc->curRSPTask;
        } else {
            curTask = sc->curRDPTask;
        }
        dlist = (SchedGfx *) curTask->list.t.data_ptr;
        rsp_segment(&dlist, 0, 0);
        rsp_segment(&dlist, 1, D_800D2FA8);
        rsp_segment(&dlist, 2, D_800D2FAC);
        rsp_segment(&dlist, 4, D_800D2FA0 - 0x500);
        if (spGfx != NULL) {
            diPrintfSetXY(30, 30);
            diPrintf(D_80082354, spGfx);
        }
        if (dpGfx != NULL) {
            diPrintfSetXY(30, 70);
            diPrintf(D_80082368, dpGfx);
        }
        yPos = 110;
        if (D_800CF520 != 0) {
            diPrintfSetXY(30, yPos);
            diPrintf(D_8008237C);
            yPos += 10;
        }
        if (D_800CF578 != 0) {
            diPrintfSetXY(30, yPos);
            diPrintf(D_80082390);
            yPos += 10;
        }
        if (D_800CF590 != 0) {
            diPrintfSetXY(30, yPos);
            diPrintf(D_800823A4);
            yPos += 10;
        }
        if (D_800CF5A8 != 0) {
            diPrintfSetXY(30, yPos);
            diPrintf(D_800823B8);
            yPos += 10;
        }
        diagnosticY = yPos + 10;
        clearRDPTask.value.write = FALSE;
        clearRDPTask.value.write = TRUE;
        spGfx = NULL;
        dpGfx = NULL;
        diPrintfSetXY(30, diagnosticY);
        diPrintf(D_800823CC, D_80082350);
        diPrintfAll(&dlist);
        __osSpSetStatus(0xAAAA82);
        osDpSetStatus(0x1D6);
        {
            SchedGfx *cmd = dlist++;
            cmd->w1 = 0;
            cmd->w0 = 0xE9000000;
        }
        {
            SchedGfx *cmd = dlist++;
            cmd->w1 = 0;
            cmd->w0 = 0xB8000000;
        }
        osWritebackDCacheAll();
        osSpTaskLoad(&curTask->list);
        osSpTaskStartGo(&curTask->list);
        while (TRUE) {
        }
    }

    if (clearRSPTask) {
        sc->curRSPTask = NULL;
    }
    if (clearRDPTask.value.normal) {
        sc->curRDPTask = NULL;
    }

    while (osRecvMesg(&sc->cmdQ, (OSMesg *) &rspTask, OS_MESG_NOBLOCK) != -1) {
        __scAppendList(sc, rspTask);
    }

    state = ((sc->curRSPTask == NULL) << 1) | (sc->curRDPTask == NULL);
    if (__scSchedule(sc, &sp, &dp, state) != state) {
        __scExec(sc, sp, dp);
    }

    D_8007A660++;
    sc->frameCount++;
    if ((sc->unkTask != NULL) && (sc->frameCount >= 2)) {
        unkTask = sc->unkTask;
        if (unkTask->msgQ != NULL) {
            if ((unkTask->unk68 != 0) || (unkTask->msg != NULL)) {
                osSendMesg(unkTask->msgQ, unkTask->msg, OS_MESG_BLOCK);
            } else {
                osSendMesg(unkTask->msgQ, &D_8007A640, OS_MESG_BLOCK);
            }
        }
        sc->frameCount = 0;
        sc->unkTask = NULL;
    }

    for (client = sc->clientList; client != NULL; client = client->next) {
        if (client->id == 1) {
            D_8007A658--;
            if (D_8007A658 <= 0) {
                osSendMesg(client->msgQ, sc, OS_MESG_NOBLOCK);
                if (sc->audioListHead != NULL) {
                    func_800304E0(sc);
                }
                D_8007A658 = func_80001BE8();
            }
        } else if (client->id == 2) {
            osSendMesg(client->msgQ, sc, OS_MESG_NOBLOCK);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/__scHandleRetrace.s")
#endif
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/__scHandleRSP.s")
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:__scHandleRDP. */
void __scHandleRDP(OSSched *sc) {
    OSScTask *task;
    OSScTask *sp = NULL;
    OSScTask *dp = NULL;
    s32 state;

    task = sc->curRDPTask;
    sc->curRDPTask = NULL;
    task->state &= ~1;
    __scTaskComplete(sc, task);
    state = ((sc->curRSPTask == NULL) << 1) | (sc->curRDPTask == NULL);
    if (__scSchedule(sc, &sp, &dp, state) != state) {
        __scExec(sc, sp, dp);
    }
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:__scTaskReady. */
OSScTask *__scTaskReady(OSScTask *task) {
    if (task != NULL) {
        if (osViGetCurrentFramebuffer() != osViGetNextFramebuffer()) {
            return NULL;
        }
        return task;
    }
    return NULL;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/__scTaskComplete.s")
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:__scAppendList. */
void __scAppendList(OSSched *sc, OSScTask *task) {
    s32 type = task->list.t.type;

    if (type == 2) {
        if (sc->audioListTail != NULL) {
            sc->audioListTail->next = task;
        } else {
            sc->audioListHead = task;
        }
        sc->audioListTail = task;
    } else {
        if (sc->gfxListTail != NULL) {
            sc->gfxListTail->next = task;
        } else {
            sc->gfxListHead = task;
        }
        sc->gfxListTail = task;
    }
    task->next = NULL;
    task->state = task->flags & 3;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/__scExec.s")
#ifdef NON_MATCHING
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp, src/sched.c:__scYield. */
void __scYield(OSSched *sc) {
    if (sc->curRSPTask->list.t.type == 1) {
        sc->curRSPTask->state |= 0x10;
        D_800D2D48 = osGetTime();
        osSpTaskYield();
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/__scYield.s")
#endif
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/__scSchedule.s")
