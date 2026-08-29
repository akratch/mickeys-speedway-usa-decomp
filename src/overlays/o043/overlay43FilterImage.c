#include "PR/ultratypes.h"

typedef struct Overlay43Image {
    u8 pad0[0x20];
    u8 *pixels;
} Overlay43Image;

/* PROVENANCE: no code adapted. The pinned DKR/JFG exact-donor scan is negative.
 * A function-specific structural scan found JFG assembly-only
 * func_overlay_4_000015A8_1EF7898 as a 0.574 masked-skeleton relative; it
 * supplies no donor C. */
/* Bounded reproof 2026-08-29: the policy-clean configured C remains the exact
 * frameless 0xAC/43-instruction shape with no relocations, but only 8/43 words
 * match positionally, first +0x4. All 119 flag recipes were nonexact and the
 * canonical -O2 -mips2 recipe tied for best. A natural staged sum now emits
 * every pixel offset in target order without the historical empty guards,
 * inert comma, or physical-line packing; the remaining 35 words are one
 * counter/sum global-color rotation, temporary FIFO allocation, and the
 * +0x7C/+0x80 constant schedule. One instrumented UOPT trace identified the
 * four global webs and a forced-color oracle improved the residual to 19/43,
 * proving that global color is only part of the blocker. Direct-store, mask-
 * carrier, register-qualifier, association, and staged-sum forms were bounded
 * and nonexact. It owns overlay +0x1378..+0x1424 / ROM
 * 0x188B348..0x188B3F4; separate +0x1424..+0x1430 padding follows. Two local
 * JUMPs at +0x218/+0x24C are its only inbounds, with no export or outbound
 * relocation. Preserve the fallback; the next pass needs a source-faithful
 * temp-FIFO/web-coalescing lever, not more flags or generic permutation. */
#ifdef NON_MATCHING
/* PLATEAU-HANDOFF
 * symbol: overlay43FilterImage
 * score: 8/43 words
 * frame: frameless
 * relocations: 0
 * first-mismatch: +0x4
 * summary: All 119 flags were nonexact; trace and forced-color prove a remaining temp-FIFO and web-coalescing source blocker.
 */
void overlay43FilterImage(Overlay43Image *image) {
    u8 *pixel;
    u32 *word;
    s32 row;
    s32 column;
    u16 sum;

    pixel = image->pixels;
    row = 0x3D;
    do {
        column = 0x3D;
        do {
            sum = pixel[1] + pixel[0];
            sum += pixel[2];
            sum += pixel[0x40];
            sum += pixel[0x42];
            sum += pixel[0x80];
            sum += pixel[0x81];
            sum += pixel[0x82];
            pixel[0x41] = sum >> 3;
            pixel++;
        } while (column--);
        pixel += 2;
    } while (row--); word = (u32 *)image->pixels; row = 0x3FF; do {
        *word = (*word & 0xF0F0F0F0) >> 4;
        word++;
    } while (row--);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o043/overlay43FilterImage/func_overlay_043_F0001378_188B348.s")
#endif
