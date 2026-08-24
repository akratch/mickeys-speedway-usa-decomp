/*
 * libultra system initialization and PI-domain speed-parameter setup.
 *
 * The complete 0x2E0-byte text object and its 0x10-byte initialized-data
 * section are byte-identical to Jet Force Gemini's built
 * `libultra/src/os/initialize.c.o` after relocation. Mickey's relocation
 * targets establish the fixed addresses used for the two PI handles and
 * __osFinalrom.
 *
 * Flags: -O2 -g3 -mips2 -32, BUILD_VERSION=VERSION_J, with RAREDIFFS.
 *
 * DEVIATION FROM THE REFERENCE: JFG defines __osFinalrom in this TU's BSS.
 * Mickey's reference lands at a fixed address inside the still-anonymous
 * whole-program data region, so it is declared extern here and assigned by
 * symbol_addrs.us.txt; no C object claims storage for it. The PI handles are
 * treated the same way because their BSS is not yet carved by TU.
 *
 * PROVENANCE: the bodies and SDK names are adapted from Jet Force Gemini's
 * `libultra/src/os/initialize.c`, a permitted published decomp source under
 * docs/CLEANROOM.md. Mickey's linked bytes decide all local declarations and
 * addresses.
 */

#include "PR/R4300.h"
#include "PR/os_internal.h"
#include "PR/os_pi.h"
#include "PR/os_version.h"
#include "PR/rcp.h"

typedef struct {
    u32 inst1;
    u32 inst2;
    u32 inst3;
    u32 inst4;
} __osExceptionVector;

extern __osExceptionVector __osExceptionPreamble[];
extern OSPiHandle __Dom1SpeedParam;
extern OSPiHandle __Dom2SpeedParam;
extern u32 __osFinalrom;
extern s32 osViClock;

OSTime osClockRate = OS_CLOCK_RATE;
u32 __osShutdown = 0;
u32 __OSGlobalIntMask = OS_IM_ALL;

static void createSpeedParam(void);

void osInitialize(void) {
    u32 pifdata;
    u32 clock = 0;

    __osFinalrom = TRUE;

    __osSetSR(__osGetSR() | SR_CU1);
    __osSetFpcCsr(FPCSR_FS | FPCSR_EV | FPCSR_RM_RN);

    while (__osSiRawReadIo(PIF_RAM_END - 3, &pifdata)) {
    }
    while (__osSiRawWriteIo(PIF_RAM_END - 3, pifdata | 8)) {
    }

    *(__osExceptionVector *)UT_VEC = *__osExceptionPreamble;
    *(__osExceptionVector *)XUT_VEC = *__osExceptionPreamble;
    *(__osExceptionVector *)ECC_VEC = *__osExceptionPreamble;
    *(__osExceptionVector *)E_VEC = *__osExceptionPreamble;
    osWritebackDCache((void *)UT_VEC, E_VEC - UT_VEC + sizeof(__osExceptionVector));
    osInvalICache((void *)UT_VEC, E_VEC - UT_VEC + sizeof(__osExceptionVector));

    createSpeedParam();
    osUnmapTLBAll();
    osMapTLBRdb();

    osClockRate = osClockRate * 3 / 4;

    if (osResetType == 0) {
        bzero(osAppNMIBuffer, OS_APP_NMI_BUFSIZE);
    }

    if (osTvType == OS_TV_PAL) {
        osViClock = VI_PAL_CLOCK;
    } else if (osTvType == OS_TV_MPAL) {
        osViClock = VI_MPAL_CLOCK;
    } else {
        osViClock = VI_NTSC_CLOCK;
    }

    if (__osGetCause() & CAUSE_IP5) {
        while (TRUE) {
        }
    }

    IO_WRITE(AI_CONTROL_REG, AI_CONTROL_DMA_ON);
    IO_WRITE(AI_DACRATE_REG, AI_MAX_DAC_RATE - 1);
    IO_WRITE(AI_BITRATE_REG, AI_MAX_BIT_RATE - 1);
}

static void createSpeedParam(void) {
    __Dom1SpeedParam.type = DEVICE_TYPE_INIT;
    __Dom1SpeedParam.latency = IO_READ(PI_BSD_DOM1_LAT_REG);
    __Dom1SpeedParam.pulse = IO_READ(PI_BSD_DOM1_PWD_REG);
    __Dom1SpeedParam.pageSize = IO_READ(PI_BSD_DOM1_PGS_REG);
    __Dom1SpeedParam.relDuration = IO_READ(PI_BSD_DOM1_RLS_REG);

    __Dom2SpeedParam.type = DEVICE_TYPE_INIT;
    __Dom2SpeedParam.latency = IO_READ(PI_BSD_DOM2_LAT_REG);
    __Dom2SpeedParam.pulse = IO_READ(PI_BSD_DOM2_PWD_REG);
    __Dom2SpeedParam.pageSize = IO_READ(PI_BSD_DOM2_PGS_REG);
    __Dom2SpeedParam.relDuration = IO_READ(PI_BSD_DOM2_RLS_REG);
}
