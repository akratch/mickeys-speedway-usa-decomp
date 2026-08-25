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

extern s32 D_8007CFD8;
extern s32 D_8007CFDC;

#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/diCpuTraceInit.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/diCpuThread.s")
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
