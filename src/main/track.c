/*
 * Resident track renderer, collision, and fog code.
 * ROM 0xC950-0x16140 (VRAM 0x8000BD50-0x80015540).
 *
 * PROVENANCE -- TU attribution and reference names come from Jet Force
 * Gemini's public decomp, `src/track.c` and its built `src/track.c.o`. The
 * 66-function Mickey block follows that TU's distinctive order from the
 * update/draw/sky routines through texture scrolling, track lights, collision
 * queries, and fog, ending with the same display-list helper. Mickey's own
 * strings, calls, function boundaries, and bytes decide every disagreement.
 * No JFG body has been adapted below yet; any body introduced later must keep
 * a PROVENANCE note at its point of use.
 *
 * Flags: -O2 -mips2 -32 (the resident game-code default).
 */

#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000BD50.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000BDB4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000C400.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000C540.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000C5E8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000C5F4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000CC78.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000CED0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D00C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D018.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D16C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D1B8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D3B8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D570.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D62C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D728.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D768.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D7F8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D820.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000D978.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000DB34.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000DDE4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000DFBC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000E5EC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000E920.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000F198.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000F57C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000F82C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FA2C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FAE0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FBD8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FCA4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FD68.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FEB4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FEEC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FF2C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80010178.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800103D4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80010654.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80010900.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80010B4C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800115E4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80011980.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80011CDC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80012234.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80012574.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80012658.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8001291C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800131AC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80013324.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800133FC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8001357C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8001398C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80013EB4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80013EC0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800140CC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80014430.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80014528.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/trackSetFogOff.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80014614.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800147A4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800148E0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80014BAC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80014DE4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80014EAC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80014ECC.s")
