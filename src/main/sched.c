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
#include "game/sched_internal.h"

#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/osCreateScheduler.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/osScAddClient.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/osScRemoveClient.s")
OSMesgQueue *osScGetCmdQ(OSSched *scheduler) {
    return &scheduler->cmdQ;
}
OSMesgQueue *osScGetInterruptQ(OSSched *scheduler) {
    return &scheduler->interruptQ;
}
void osScGetAudioSPStats(f32 *first, f32 *second, f32 *third) {
    *first = 0.0f;
    *second = 0.0f;
    *third = 0.0f;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/__scMain.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/func_800304E0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/osScGetTaskType.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/func_80030608.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/func_80030610.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/func_80030910.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/__scHandleRetrace.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/__scHandleRSP.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/__scHandleRDP.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/__scTaskReady.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/__scTaskComplete.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/__scAppendList.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/__scExec.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/__scYield.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/__scSchedule.s")
