/*
 * CPU exception/debug monitor -- ROM 0x465B0-0x47A60 (VRAM 0x800459B0).
 *
 * PROVENANCE: the translation-unit identity and descriptive names are
 * adapted from Jet Force Gemini's public decompilation, src/diCpu.c. The
 * Tier-A diCpuTraceInit match, debug strings, direct callers and function
 * order establish the correspondence. JFG address-placeholder names are not
 * imported. The bodies remain Mickey's extracted assembly.
 */

#include "PR/ultratypes.h"
#include "PR/os_internal.h"
#include "PR/os_message.h"

extern s32 D_8007CFD8;
extern s32 D_8007CFDC;
extern f32 D_80083DBC;
extern OSMesgQueue D_800D5CD0;
extern OSMesg D_800D5CE8[8];
extern OSMesg D_800D5D08[8];
extern OSMesgQueue D_800D5D28;
extern void func_8004D5E0(s32 priority, OSMesgQueue *queue, OSMesg *messages,
                          s32 count);
extern void osViSetSpecialFeatures(u32 features);
extern void func_80045CAC(void);
void stop_all_threads_except_main(void);

#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/diCpuTraceInit.s")
#ifdef NON_MATCHING
/* PROVENANCE: body adapted from JFG src/diCpu.c::diCpuThread; Mickey's own
 * draft supplies its extra VI delay loop and exact event handling. */
void diCpuThread(void *unused) {
    OSMesg message;
    s32 events;
    register s32 i;
    register f32 divisor;
    register f32 sum;

    events = 0;
    osCreateMesgQueue(&D_800D5CD0, D_800D5CE8, 8);
    osSetEventMesg(12, &D_800D5CD0, (OSMesg)8);
    osSetEventMesg(10, &D_800D5CD0, (OSMesg)2);
    func_8004D5E0(150, &D_800D5D28, D_800D5D08, 8);
    divisor = D_80083DBC;
    while (1) {
        osRecvMesg(&D_800D5CD0, &message, OS_MESG_BLOCK);
        events |= (s32)message;
        if ((events & 8) || (events & 2)) {
            osViSetSpecialFeatures(0xA2);
            sum = 0.0f;
            i = 1000000;
            while (i--) {
                sum += (f32)i / divisor;
            }
            events &= ~8;
            stop_all_threads_except_main();
            func_80045CAC();
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/diCpuThread.s")
#endif
/* PROVENANCE: body adapted from JFG src/diCpu.c::stop_all_threads_except_main. */
void stop_all_threads_except_main(void) {
    OSThread *thread = __osGetActiveQueue();

    while (thread->priority != -1) {
        if (thread->priority > OS_PRIORITY_IDLE && thread->priority < 128) {
            osStopThread(thread);
        }
        thread = thread->tlnext;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80045BBC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80045CAC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80045D34.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/diCpuReportWatchpoint.s")
/* PROVENANCE: body adapted from JFG src/diCpu.c::diCpuTraceGetFault. */
s32 func_80046504(void) {
    return 0;
}

/* PROVENANCE: body adapted from JFG src/diCpu.c::diCpuTraceTick. */
void func_8004650C(s32 ticks) {
    D_8007CFDC += ticks;
    if (D_8007CFDC > 60) {
        D_8007CFDC = 0;
        D_8007CFD8++;
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/render_epc_lock_up_display.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80046AA8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80046BCC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/cpuXYPrintf.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80046E00.s")
