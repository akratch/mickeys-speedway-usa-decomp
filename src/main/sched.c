/*
 * Game scheduler -- ROM 0x30CD0-0x323A0.
 *
 * PROVENANCE -- the TU identity and descriptive names are adapted from Jet
 * Force Gemini's public decompilation, src/sched.c. Mickey has the same
 * ordered 21-function scheduler call graph, including the three pre-existing
 * accessor/string anchors. The bodies below remain Mickey GLOBAL_ASM.
 */

#include "PR/ultratypes.h"

#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/osCreateScheduler.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/osScAddClient.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/osScRemoveClient.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/osScGetCmdQ.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/osScGetInterruptQ.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/osScGetAudioSPStats.s")
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
