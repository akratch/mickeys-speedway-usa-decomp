#include "PR/ultratypes.h"

/*
 * JFG overlay 27 has the unique full 0x8C-byte object match for this routine;
 * pinned DKR v77/v80 scans are negative. JFG retains it as assembly, so this
 * C reconstruction follows the shared instruction semantics conservatively.
 */
s32 overlay16BuildGradient(s8 *output, s32 first0, s32 first1, s32 first2,
                           s32 last0, s32 last1, s32 last2) {
    s32 remaining;

    last0 -= first0;
    last1 -= first1;
    last2 -= first2;
    first0 <<= 6;
    first1 <<= 6;
    /* The comma form preserves the retail compiler's v0/v1 loop coloring. */
    first2 <<= (0, 6);
    remaining = 0x3F;
    do {
        output[0] = first0 >> 6;
        output[1] = first1 >> 6;
        output[2] = first2 >> 6;
        first0 += last0;
        first1 += last1;
        first2 += last2;
        output += 3;
    } while (remaining--);
}
