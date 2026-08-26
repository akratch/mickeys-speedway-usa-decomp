/*
 * Display-list trace buffer -- ROM 0x45760-0x459C0 (VRAM 0x80044B60).
 *
 * PROVENANCE: the translation-unit identity and function identities are adapted
 * from Jet Force Gemini's public decompilation, src/diRcpTrace.c. Mickey's
 * own call graph, placement immediately before diRcp.c, and the four-function
 * order establish the correspondence; JFG is a permitted published decomp
 * under docs/CLEANROOM.md. Externally referenced functions retain their
 * Mickey address labels until their callers are decompiled. The bodies remain
 * Mickey's extracted assembly.
 */

#include "PR/ultratypes.h"

typedef struct Gfx {
    struct {
        u32 w0;
        u32 w1;
    } words;
} Gfx;

typedef struct DiRcpTraceEntry {
    Gfx *value0;
    char *value4;
    s32 value8;
} DiRcpTraceEntry;

extern s32 D_8007CFC8;
extern s32 D_800D4A90[];
extern s32 D_8007CFC0;
extern s32 D_8007CFC4;
extern s32 func_8002B280(s32 size, s32 tag);

#define DI_RCP_TRACE_BUFFERS ((DiRcpTraceEntry **)&D_8007CFC0)

void diRcpTraceInit(void) {
    D_8007CFC0 = func_8002B280(0x4B0, 0x8F);
    D_8007CFC4 = func_8002B280(0x4B0, 0x8F);
}
void func_80044B9C(void) {
    D_8007CFC8 = 1 - D_8007CFC8;
    D_800D4A90[D_8007CFC8] = 0;
}
/* Mickey-derived body; JFG's diRcpTrace implementation is assembly-only. */
void func_80044BC8(Gfx *value0, char *value4, s32 value8) {
    if (DI_RCP_TRACE_BUFFERS[D_8007CFC8] != 0) {
        if (D_800D4A90[D_8007CFC8] < 100) {
            DI_RCP_TRACE_BUFFERS[D_8007CFC8][D_800D4A90[D_8007CFC8]].value0 =
                value0;
            DI_RCP_TRACE_BUFFERS[D_8007CFC8][D_800D4A90[D_8007CFC8]].value4 =
                value4;
            DI_RCP_TRACE_BUFFERS[D_8007CFC8][D_800D4A90[D_8007CFC8]].value8 =
                value8;
            D_800D4A90[D_8007CFC8]++;
        }
    }
}
/*
 * Workbench: mixed(schedule:2, register:30), exact size/frame/75 instructions/32 words, first mismatch +0xC.
 * Levers: statement order/lines, pool-vs-temp routing, dead reads, and all 119 flag variants; no exact result.
 * Remains: target buffer-in-v1 coloring without extra dead-read instructions; the temporary phase follows.
 */
#ifdef NON_MATCHING
/* PROVENANCE: parameter and entry roles adapted from JFG src/sched.c and
 * src/diRcpTrace.c; the body is Mickey-derived and JFG's peer is assembly-only. */
void func_80044C94(Gfx *value, char **lowerValue4, s32 *lowerValue8,
                   Gfx **lowerValue0, char **upperValue4, s32 *upperValue8,
                   Gfx **upperValue0) {
    s32 buffer;
    s32 lowerOffset;
    s32 count;
    s32 offset;
    DiRcpTraceEntry *entries;
    DiRcpTraceEntry *entry;
    Gfx *entryValue;
    Gfx *lower;
    Gfx *upper;
    DiRcpTraceEntry *lowerEntry;
    s32 entrySize;
    DiRcpTraceEntry *upperEntry;

    entrySize = sizeof(DiRcpTraceEntry);
    buffer = 1 - D_8007CFC8;
    count = D_800D4A90[buffer];
    lower = 0;
    upper = (Gfx *)-1;
    lowerEntry = NULL;
    upperEntry = NULL;
    if (count > 0) {
        entries = DI_RCP_TRACE_BUFFERS[buffer];
        entry = entries;
        offset = 0;
        do {
            entryValue = entry->value0;
            if (value >= entryValue && lower < entryValue) {
                lower = entryValue;
                lowerOffset = offset;
                lowerEntry =
                    (DiRcpTraceEntry *)((u8 *)entries + lowerOffset);
            }
            if (value < entryValue && entryValue < upper) {
                upper = entryValue;
                upperEntry = (DiRcpTraceEntry *)((u8 *)entries + offset);
            }
            offset += sizeof(DiRcpTraceEntry);
            entry++;
        } while (offset < count * entrySize);
    }
    if (lowerEntry != NULL) {
        *lowerValue4 = lowerEntry->value4;
        *lowerValue8 = lowerEntry->value8;
        *lowerValue0 = lowerEntry->value0;
    } else {
        *lowerValue4 = 0;
    }
    if (upperEntry != NULL) {
        *upperValue4 = upperEntry->value4;
        *upperValue8 = upperEntry->value8;
        *upperValue0 = upperEntry->value0;
    } else {
        *upperValue4 = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcpTrace/func_80044C94.s")
#endif
