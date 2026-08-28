/*
 * Source provenance: body adapted from Jet Force Gemini's public decomp,
 * src/diCpuTraceCurrentStack.c::diCpuTraceCurrentStack; see
 * docs/CLEANROOM.md. Mickey's established name is retained.
 */

void *stack_pointer(void) {
    return (void *) __$sp;
}
