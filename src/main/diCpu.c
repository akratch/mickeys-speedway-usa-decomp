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

#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/diCpuTraceInit.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/diCpuThread.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/stop_all_threads_except_main.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80045BBC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80045CAC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80045D34.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/diCpuReportWatchpoint.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80046504.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_8004650C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/render_epc_lock_up_display.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80046AA8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80046BCC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/cpuXYPrintf.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80046E00.s")
