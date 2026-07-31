#ifndef _GAME_SCHED_H_
#define _GAME_SCHED_H_

#include "PR/ultratypes.h"

/*
 * The game scheduler's task taxonomy.
 *
 * NOTHING IN THIS HEADER IS COMPILED INTO THE ROM YET. It records what the
 * disassembly of osScGetTaskType (0x80030580) proves, so that whoever
 * decompiles the scheduler at ROM 0x2F400-0x33FA0 inherits it instead of
 * re-deriving it. Nothing includes this file; that is deliberate, and it
 * should stop being true as soon as a scheduler TU exists.
 *
 * Field evidence. osScGetTaskType is a seven-arm jump table on `arg0 - 1`:
 *
 *   ROM 0x31180  addiu t6, a0, -0x1
 *   ROM 0x31184  sltiu at, t6, 0x7      <- so the valid range is 1..7
 *   ROM 0x31188  beqz  at, <default>
 *   ROM 0x31194  lui   at, %hi(jtbl_800823D8)
 *
 * and each arm returns the address of one string constant. Reading the arms in
 * table order gives the seven values below; the default arm returns
 * "(Unknown task type %d)" at 0x80082100, which is why 0 has no name.
 *
 * The same seven names, in the same order, appear as commented-out text inside
 * Jet Force Gemini's public decomp of its own osScGetTaskType -- which is what
 * the function's NAME is adopted from (see symbol_addrs.us.txt). The values
 * themselves come from Mickey's jump table, not from JFG.
 *
 * SC_TASK_CLONE is the interesting one for the module map: `clone` is also one
 * of the four module names the ROM's own __FILE__ strings carry, and the only
 * one whose path string lives in an overlay rather than the resident segment --
 * in overlay 43's .data, one of 107. See docs/modules.md.
 */
typedef enum ScTaskType {
    SC_TASK_AUDIO = 1,        /* "(Audio task)",           0x80082090 */
    SC_TASK_GAME = 2,         /* "(Game task)",            0x800820A0 */
    SC_TASK_DI = 3,           /* "(DI task)\n",            0x800820AC */
    SC_TASK_DI_BENCHMARK = 4, /* "(DI benchmark test)\n",  0x800820B8 */
    SC_TASK_CLONE = 5,        /* "(Clone task)\n",         0x800820D0 */
    SC_TASK_REFRACT = 6,      /* "(Refract task)\n",       0x800820E0 */
    SC_TASK_BLUR = 7          /* "(Blur task)\n",          0x800820F0 */
} ScTaskType;

/* Returns a printable name for `type`, or "(Unknown task type %d)". */
const char *osScGetTaskType(s32 type);

#endif /* _GAME_SCHED_H_ */
